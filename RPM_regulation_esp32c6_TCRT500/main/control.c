#include "control.h"
#include "pid.h"
#include "motor.h"
#include "rpm.h"
#include "config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_timer.h"
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

static portMUX_TYPE setpoint_lock = portMUX_INITIALIZER_UNLOCKED;
static float rpm_setpoint = DEFAULT_RPM_SETPOINT;
static float last_rpm = 0.0f;
static float last_output = 0.0f;
static float last_command = 0.0f;
static float last_duty = 0.0f;
static const char *last_mode = "STARTING";

void control_set_setpoint(float rpm) {
    if (rpm < MIN_RPM_SETPOINT) {
        rpm = MIN_RPM_SETPOINT;
    }
    if (rpm > MAX_RPM_SETPOINT) {
        rpm = MAX_RPM_SETPOINT;
    }

    portENTER_CRITICAL(&setpoint_lock);
    rpm_setpoint = rpm;
    portEXIT_CRITICAL(&setpoint_lock);
}

float control_get_setpoint(void) {
    float rpm;
    portENTER_CRITICAL(&setpoint_lock);
    rpm = rpm_setpoint;
    portEXIT_CRITICAL(&setpoint_lock);
    return rpm;
}

void control_get_status(control_status_t *status) {
    if (status == NULL) {
        return;
    }

    portENTER_CRITICAL(&setpoint_lock);
    status->rpm = last_rpm;
    status->setpoint = rpm_setpoint;
    status->output = last_output;
    status->command = last_command;
    status->duty = last_duty;
    status->mode = last_mode;
    portEXIT_CRITICAL(&setpoint_lock);

    status->min_setpoint = MIN_RPM_SETPOINT;
    status->max_setpoint = MAX_RPM_SETPOINT;
}

static float clampf_local(float value, float min, float max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static float slew_toward(float current, float target, float step_up, float step_down) {
    float delta = target - current;
    if (delta > step_up) {
        return current + step_up;
    }
    if (delta < -step_down) {
        return current - step_down;
    }
    return target;
}

static float maxf_local(float a, float b) {
    return (a > b) ? a : b;
}

static float startup_handoff_rpm(float setpoint) {
    if (setpoint <= 0.0f || RPM_STARTUP_HANDOFF_CAL_RPM <= 0.0f) {
        return 0.0f;
    }

    float ratio = RPM_STARTUP_HANDOFF_CAL_DETECTED_RPM / RPM_STARTUP_HANDOFF_CAL_RPM;
    float handoff = setpoint * ratio;
    return maxf_local(RPM_STARTUP_MIN_RPM, handoff);
}

static float startup_duty_for_elapsed(int64_t mode_ms) {
    (void)mode_ms;
    return RPM_STARTUP_DUTY;
}

static float duty_from_calibration(float setpoint, float duty_at_ref, float duty_at_max) {
    if (setpoint <= 0.0f || PWM_FEEDFORWARD_REF_RPM <= 0.0f) {
        return 0.0f;
    }

    if (setpoint <= PWM_FEEDFORWARD_REF_RPM) {
        float duty = duty_at_ref * (setpoint / PWM_FEEDFORWARD_REF_RPM);
        return clampf_local(duty, 0.0f, PWM_MAX_DUTY);
    }

    float rpm_span = MAX_RPM_SETPOINT - PWM_FEEDFORWARD_REF_RPM;
    if (rpm_span <= 0.0f) {
        return clampf_local(duty_at_ref, 0.0f, PWM_MAX_DUTY);
    }

    float ratio = (setpoint - PWM_FEEDFORWARD_REF_RPM) / rpm_span;
    float duty = duty_at_ref + ratio * (duty_at_max - duty_at_ref);
    return clampf_local(duty, 0.0f, PWM_MAX_DUTY);
}

static float feedforward_duty_for_setpoint(float setpoint) {
    return duty_from_calibration(setpoint, PWM_RUN_FEEDFORWARD_DUTY, PWM_MAX_DUTY);
}

static float apply_motor_feedforward(float pid_correction, float setpoint, float current_rpm) {
    if (setpoint <= 0.0f) {
        return 0.0f;
    }

    float base_duty = feedforward_duty_for_setpoint(setpoint);
    float min_effective_duty = duty_from_calibration(setpoint, PWM_MIN_EFFECTIVE_DUTY, PWM_MAX_DUTY);
    float recovery_duty = duty_from_calibration(setpoint, PWM_RECOVERY_DUTY, PWM_MAX_DUTY);
    float command = base_duty + pid_correction;

    if (current_rpm < (setpoint - RPM_CONTROL_DEADBAND) &&
        command < recovery_duty) {
        command = recovery_duty;
    } else if (current_rpm < setpoint && command < min_effective_duty) {
        command = min_effective_duty;
    }

    return clampf_local(command, 0.0f, PWM_MAX_DUTY);
}

typedef enum {
    CONTROL_STARTING,
    CONTROL_RUNNING,
} control_mode_t;

static const char *mode_name(control_mode_t mode) {
    switch (mode) {
    case CONTROL_STARTING:
        return "STARTING";
    case CONTROL_RUNNING:
        return "RUNNING";
    default:
        return "UNKNOWN";
    }
}

void control_task(void *arg) {
    (void)arg;

    pid_t pid;
    pid_init(&pid, PID_KP, PID_KI, PID_KD);
    pid_set_output_limits(&pid, -PWM_RUN_FEEDFORWARD_DUTY, PWM_MAX_DUTY - PWM_RUN_FEEDFORWARD_DUTY);
    float applied_duty = 0.0f;
    control_mode_t mode = CONTROL_STARTING;
    int64_t mode_since_us = esp_timer_get_time();

    static const char *TAG = "PID";
    ESP_LOGI(TAG, "Tuning activo: Kp=%.3f Ki=%.3f Kd=%.3f SlewUp=%.1f SlewDown=%.1f DutyMax=%.0f",
             PID_KP, PID_KI, PID_KD, PWM_SLEW_STEP_UP, PWM_SLEW_STEP_DOWN, PWM_MAX_DUTY);
    ESP_LOGI(TAG, "Arranque infinito a maxima potencia: duty=%.0f hasta detectar handoff=max(%.0f, %.0f%% ref)",
             PWM_MAX_DUTY,
             RPM_STARTUP_MIN_RPM,
             (RPM_STARTUP_HANDOFF_CAL_DETECTED_RPM / RPM_STARTUP_HANDOFF_CAL_RPM) * 100.0f);
    ESP_LOGI(TAG, "Feedforward: base %.0fRPM=%.0f escala hasta %.0fRPM=%.0f min=%.0f recovery=%.0f deadband=%.0f RPM",
             PWM_FEEDFORWARD_REF_RPM,
             PWM_RUN_FEEDFORWARD_DUTY,
             MAX_RPM_SETPOINT,
             PWM_MAX_DUTY,
             PWM_MIN_EFFECTIVE_DUTY,
             PWM_RECOVERY_DUTY,
             RPM_CONTROL_DEADBAND);

    while (1) {
        float setpoint = control_get_setpoint();
        rpm_set_filter_reference(setpoint);
        float current_rpm = rpm_get();
        bool signal_recent = rpm_signal_recent(RPM_SIGNAL_TIMEOUT_MS);
        int64_t now_us = esp_timer_get_time();
        float output = 0.0f;
        float target_duty = 0.0f;
        bool immediate_kick = false;

        if (setpoint <= 0.0f) {
            applied_duty = 0.0f;
            target_duty = 0.0f;
            mode = CONTROL_STARTING;
            mode_since_us = now_us;
            pid_init(&pid, PID_KP, PID_KI, PID_KD);
            pid_set_output_limits(&pid, -PWM_MAX_DUTY, PWM_MAX_DUTY);
        } else {
            int64_t mode_ms = (now_us - mode_since_us) / 1000;
            control_mode_t previous_mode = mode;
            float handoff_rpm = startup_handoff_rpm(setpoint);

            if (mode == CONTROL_STARTING) {
                target_duty = startup_duty_for_elapsed(mode_ms);
                output = target_duty;
                immediate_kick = target_duty >= PWM_KICKSTART_DUTY;

                if (mode_ms >= RPM_STARTUP_MIN_MS && signal_recent && current_rpm >= handoff_rpm) {
                    mode = CONTROL_RUNNING;
                    mode_since_us = now_us;
                    pid_init(&pid, PID_KP, PID_KI, PID_KD);
                    float base_duty = feedforward_duty_for_setpoint(setpoint);
                    pid_set_output_limits(&pid, -base_duty, PWM_MAX_DUTY - base_duty);
                }
            } else if (mode == CONTROL_RUNNING) {
                if (!signal_recent || current_rpm < RPM_STARTUP_MIN_RPM) {
                    mode = CONTROL_STARTING;
                    mode_since_us = now_us;
                    target_duty = PWM_KICKSTART_DUTY;
                    output = target_duty;
                    immediate_kick = true;
                } else {
                    float base_duty = feedforward_duty_for_setpoint(setpoint);
                    pid_set_output_limits(&pid, -base_duty, PWM_MAX_DUTY - base_duty);
                    output = pid_compute(&pid, setpoint, current_rpm, SAMPLE_TIME_MS / 1000.0f);
                    target_duty = apply_motor_feedforward(output, setpoint, current_rpm);
                }
            }

            if (mode != previous_mode) {
                ESP_LOGW(TAG, "Modo control: %s -> %s", mode_name(previous_mode), mode_name(mode));
            }

            if (immediate_kick && target_duty > applied_duty) {
                applied_duty = target_duty;
            } else {
                applied_duty = slew_toward(applied_duty, target_duty, PWM_SLEW_STEP_UP, PWM_SLEW_STEP_DOWN);
            }
        }

        float duty_f = clampf_local(fabsf(applied_duty), 0.0f, PWM_MAX_DUTY);

        motor_set_direction(1);
        motor_set_speed((uint8_t)duty_f);

        portENTER_CRITICAL(&setpoint_lock);
        last_rpm = current_rpm;
        last_output = output;
        last_command = target_duty;
        last_duty = duty_f;
        last_mode = mode_name(mode);
        portEXIT_CRITICAL(&setpoint_lock);

        ESP_LOGI(TAG, "RPM: %.2f | REF: %.2f | PIDOUT: %.2f | CMD: %.2f | DUTY: %d | MODE: %s",
                 current_rpm,
                 setpoint,
                 output,
                 target_duty,
                 (int)duty_f,
                 mode_name(mode));

        vTaskDelay(pdMS_TO_TICKS(SAMPLE_TIME_MS));
    }
}
