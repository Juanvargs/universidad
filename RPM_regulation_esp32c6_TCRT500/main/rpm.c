#include "rpm.h"

#include "config.h"
#include "driver/gpio.h"
#include "esp_attr.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#define PULSE_DEBOUNCE_US 2000
#define MIN_VALID_INTERVAL_US ((int64_t)(60000000.0f / (RPM_SENSOR_MAX_VALID_RPM * PULSES_PER_REV)))

static const char *TAG = "RPM";
static float rpm_filtered = 0.0f;
static portMUX_TYPE pulse_lock = portMUX_INITIALIZER_UNLOCKED;
static volatile int64_t last_pulse_time = 0;
static volatile int64_t pulse_interval_us = 0;
static volatile int64_t pulse_intervals_us[RPM_PERIOD_FILTER_SAMPLES] = {0};
static volatile uint8_t pulse_interval_index = 0;
static volatile uint8_t pulse_interval_count = 0;
static float rpm = 0.0f;
static float rpm_filter_reference = DEFAULT_RPM_SETPOINT;

static float clampf_rpm(float value, float min, float max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static int64_t median_interval_us(const int64_t *values, uint8_t count) {
    int64_t sorted[RPM_PERIOD_FILTER_SAMPLES];

    for (uint8_t i = 0; i < count; ++i) {
        sorted[i] = values[i];
    }

    for (uint8_t i = 1; i < count; ++i) {
        int64_t value = sorted[i];
        int8_t j = (int8_t)i - 1;
        while (j >= 0 && sorted[j] > value) {
            sorted[j + 1] = sorted[j];
            --j;
        }
        sorted[j + 1] = value;
    }

    if ((count % 2) == 1) {
        return sorted[count / 2];
    }

    return (sorted[(count / 2) - 1] + sorted[count / 2]) / 2;
}

static void IRAM_ATTR isr_handler(void *arg) {
    (void)arg;

    int64_t now = esp_timer_get_time();
    portENTER_CRITICAL_ISR(&pulse_lock);
    int64_t interval = now - last_pulse_time;

    if (last_pulse_time == 0 ||
        interval > ((int64_t)RPM_MEASUREMENT_TIMEOUT_MS * 1000)) {
        pulse_interval_us = 0;
        pulse_interval_index = 0;
        pulse_interval_count = 0;
        last_pulse_time = now;
        portEXIT_CRITICAL_ISR(&pulse_lock);
        return;
    }

    bool interval_valid = interval > PULSE_DEBOUNCE_US && interval >= MIN_VALID_INTERVAL_US;
    if (interval_valid && pulse_interval_us > 0) {
        int64_t min_plausible_interval = (int64_t)((float)pulse_interval_us * RPM_MIN_INTERVAL_RATIO);
        if (interval < min_plausible_interval) {
            interval_valid = false;
        }
    }

    if (interval_valid) {
        pulse_interval_us = interval;
        pulse_intervals_us[pulse_interval_index] = interval;
        pulse_interval_index = (pulse_interval_index + 1) % RPM_PERIOD_FILTER_SAMPLES;
        if (pulse_interval_count < RPM_PERIOD_FILTER_SAMPLES) {
            pulse_interval_count++;
        }
        last_pulse_time = now;
    }
    portEXIT_CRITICAL_ISR(&pulse_lock);
}

void rpm_init(void) {
    const gpio_config_t sensor_config = {
        .pin_bit_mask = 1ULL << PIN_SENSOR,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };

    ESP_ERROR_CHECK(gpio_config(&sensor_config));

    esp_err_t err = gpio_install_isr_service(0);
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "No se pudo instalar servicio ISR GPIO: %s", esp_err_to_name(err));
        return;
    }

    err = gpio_isr_handler_add(PIN_SENSOR, isr_handler, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "No se pudo asociar ISR al sensor GPIO%d: %s", PIN_SENSOR, esp_err_to_name(err));
        return;
    }

    portENTER_CRITICAL(&pulse_lock);
    last_pulse_time = 0;
    pulse_interval_us = 0;
    pulse_interval_index = 0;
    pulse_interval_count = 0;
    for (uint8_t i = 0; i < RPM_PERIOD_FILTER_SAMPLES; ++i) {
        pulse_intervals_us[i] = 0;
    }
    portEXIT_CRITICAL(&pulse_lock);

    ESP_LOGI(TAG, "Sensor RPM GPIO%d por mediana de %d periodos, max valido %.0f RPM",
             PIN_SENSOR,
             RPM_PERIOD_FILTER_SAMPLES,
             RPM_SENSOR_MAX_VALID_RPM);
}

void rpm_set_filter_reference(float setpoint) {
    rpm_filter_reference = setpoint;
}

float rpm_get(void) {
    int64_t now = esp_timer_get_time();
    int64_t interval_us;
    int64_t interval_samples[RPM_PERIOD_FILTER_SAMPLES];
    uint8_t interval_count;
    int64_t pulse_time;

    portENTER_CRITICAL(&pulse_lock);
    interval_us = pulse_interval_us;
    interval_count = pulse_interval_count;
    for (uint8_t i = 0; i < interval_count; ++i) {
        interval_samples[i] = pulse_intervals_us[i];
    }
    pulse_time = last_pulse_time;
    portEXIT_CRITICAL(&pulse_lock);

    if (interval_count > 0) {
        interval_us = median_interval_us(interval_samples, interval_count);
    }

    int64_t pulse_age_us = now - pulse_time;
    int64_t stale_limit_us = (int64_t)((float)interval_us * RPM_STALE_PERIOD_MULTIPLIER);
    int64_t stale_min_us = (int64_t)RPM_STALE_MIN_MS * 1000;
    if (stale_limit_us < stale_min_us) {
        stale_limit_us = stale_min_us;
    }

    if (interval_us > 0 && pulse_time > 0 &&
        pulse_age_us <= stale_limit_us &&
        pulse_age_us <= ((int64_t)RPM_MEASUREMENT_TIMEOUT_MS * 1000)) {
        rpm = 60000000.0f / ((float)interval_us * (float)PULSES_PER_REV);
        rpm = clampf_rpm(rpm, 0.0f, MAX_RPM_SETPOINT);
    } else {
        rpm = 0.0f;
    }

    float alpha = RPM_FILTER_ALPHA_RISE;
    if (rpm < rpm_filtered) {
        bool near_setpoint = rpm_filter_reference > 0.0f &&
                             fabsf(rpm_filtered - rpm_filter_reference) <= RPM_FILTER_NEAR_SETPOINT_BAND;
        alpha = near_setpoint ? RPM_FILTER_ALPHA_FALL_NEAR : RPM_FILTER_ALPHA_FALL;
    }
    rpm_filtered = alpha * rpm + (1.0f - alpha) * rpm_filtered;
    if (rpm == 0.0f && rpm_filtered < 1.0f) {
        rpm_filtered = 0.0f;
    }

    return rpm_filtered;
}

bool rpm_signal_recent(uint32_t timeout_ms) {
    int64_t pulse_time;

    portENTER_CRITICAL(&pulse_lock);
    pulse_time = last_pulse_time;
    portEXIT_CRITICAL(&pulse_lock);

    if (pulse_time == 0) {
        return false;
    }

    int64_t age_us = esp_timer_get_time() - pulse_time;
    return age_us <= ((int64_t)timeout_ms * 1000);
}
