#include "oled.h"

#include "config.h"
#include "control.h"
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OLED_WIDTH 128
#define OLED_HEIGHT 64
#define OLED_PAGES 8
#define OLED_BUFFER_SIZE (OLED_WIDTH * OLED_PAGES)
#define OLED_PAGE_PACKET_SIZE (OLED_WIDTH + 1)
#define OLED_REFRESH_INTERVAL_US 100000

static const char *TAG = "OLED";
static i2c_master_bus_handle_t i2c_bus;
static i2c_master_dev_handle_t oled_dev;
static uint8_t framebuffer[OLED_BUFFER_SIZE];
static bool oled_ready;
static int64_t last_refresh_us;
static int64_t last_error_log_us;

static esp_err_t oled_write_command(uint8_t command) {
    uint8_t data[] = {0x00, command};
    return i2c_master_transmit(oled_dev, data, sizeof(data), pdMS_TO_TICKS(100));
}

static esp_err_t oled_write_commands(const uint8_t *commands, size_t len) {
    esp_err_t err = ESP_OK;
    for (size_t i = 0; i < len; i++) {
        err = oled_write_command(commands[i]);
        if (err != ESP_OK) {
            return err;
        }
    }
    return ESP_OK;
}

static esp_err_t oled_flush(void) {
    if (!oled_ready) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t packet[OLED_PAGE_PACKET_SIZE];
    packet[0] = 0x40;

    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        const uint8_t commands[] = {
            (uint8_t)(0xB0 | page),
            0x00,
            0x10,
        };
        esp_err_t err = oled_write_commands(commands, sizeof(commands));
        if (err != ESP_OK) {
            return err;
        }

        memcpy(&packet[1], &framebuffer[page * OLED_WIDTH], OLED_WIDTH);
        err = i2c_master_transmit(oled_dev, packet, sizeof(packet), pdMS_TO_TICKS(200));
        if (err != ESP_OK) {
            return err;
        }
    }

    return ESP_OK;
}

static void oled_log_flush_error(esp_err_t err) {
    int64_t now = esp_timer_get_time();
    if (now - last_error_log_us >= 1000000) {
        ESP_LOGW(TAG, "No se pudo actualizar la pantalla: %s", esp_err_to_name(err));
        last_error_log_us = now;
    }
}

static void oled_commit_frame(void) {
    esp_err_t err = oled_flush();
    last_refresh_us = esp_timer_get_time();
    if (err != ESP_OK) {
        oled_log_flush_error(err);
    }
}

static void oled_clear(void) {
    memset(framebuffer, 0, sizeof(framebuffer));
}

static void oled_set_pixel(int x, int y, bool on) {
    if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT) {
        return;
    }

    uint16_t index = x + (y / 8) * OLED_WIDTH;
    uint8_t mask = 1 << (y % 8);
    if (on) {
        framebuffer[index] |= mask;
    } else {
        framebuffer[index] &= ~mask;
    }
}

static void oled_draw_line(int x0, int y0, int x1, int y1) {
    int dx = abs(x1 - x0);
    int sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0);
    int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        oled_set_pixel(x0, y0, true);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

static void oled_fill_rect(int x, int y, int w, int h) {
    for (int yy = y; yy < y + h; yy++) {
        for (int xx = x; xx < x + w; xx++) {
            oled_set_pixel(xx, yy, true);
        }
    }
}

static const uint8_t *glyph_for(char c) {
    static const uint8_t space[5] = {0, 0, 0, 0, 0};
    static const uint8_t colon[5] = {0, 2, 0, 2, 0};
    static const uint8_t dot[5] = {0, 0, 0, 0, 2};
    static const uint8_t dash[5] = {0, 0, 7, 0, 0};
    static const uint8_t digits[10][5] = {
        {7, 5, 5, 5, 7}, {2, 6, 2, 2, 7}, {7, 1, 7, 4, 7}, {7, 1, 7, 1, 7}, {5, 5, 7, 1, 1},
        {7, 4, 7, 1, 7}, {7, 4, 7, 5, 7}, {7, 1, 1, 1, 1}, {7, 5, 7, 5, 7}, {7, 5, 7, 1, 7},
    };
    static const uint8_t letters[26][5] = {
        {7, 5, 7, 5, 5}, {6, 5, 6, 5, 6}, {7, 4, 4, 4, 7}, {6, 5, 5, 5, 6}, {7, 4, 7, 4, 7},
        {7, 4, 7, 4, 4}, {7, 4, 5, 5, 7}, {5, 5, 7, 5, 5}, {7, 2, 2, 2, 7}, {1, 1, 1, 5, 7},
        {5, 5, 6, 5, 5}, {4, 4, 4, 4, 7}, {5, 7, 7, 5, 5}, {5, 7, 7, 7, 5}, {7, 5, 5, 5, 7},
        {7, 5, 7, 4, 4}, {7, 5, 5, 7, 1}, {7, 5, 7, 6, 5}, {7, 4, 7, 1, 7}, {7, 2, 2, 2, 2},
        {5, 5, 5, 5, 7}, {5, 5, 5, 5, 2}, {5, 5, 7, 7, 5}, {5, 5, 2, 5, 5}, {5, 5, 2, 2, 2},
        {7, 1, 2, 4, 7},
    };

    if (c >= '0' && c <= '9') {
        return digits[c - '0'];
    }
    if (c >= 'a' && c <= 'z') {
        c -= 32;
    }
    if (c >= 'A' && c <= 'Z') {
        return letters[c - 'A'];
    }
    if (c == ':') {
        return colon;
    }
    if (c == '.') {
        return dot;
    }
    if (c == '-') {
        return dash;
    }
    return space;
}

static void oled_draw_char(int x, int y, char c, int scale) {
    const uint8_t *glyph = glyph_for(c);
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 3; col++) {
            if (glyph[row] & (1 << (2 - col))) {
                oled_fill_rect(x + col * scale, y + row * scale, scale, scale);
            }
        }
    }
}

static void oled_draw_text(int x, int y, const char *text, int scale) {
    int cursor = x;
    while (*text) {
        oled_draw_char(cursor, y, *text++, scale);
        cursor += 4 * scale;
    }
}

static void oled_draw_centered_text(int y, const char *text, int scale) {
    int width = (int)strlen(text) * 4 * scale - scale;
    oled_draw_text((OLED_WIDTH - width) / 2, y, text, scale);
}

static float clampf(float value, float min, float max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

static void oled_draw_tach_arc(float measured_rpm, float setpoint_rpm) {
    const int cx = 64;
    const int cy = 45;
    const int radius = 34;
    float max_rpm = fmaxf(setpoint_rpm * 1.5f, 1000.0f);
    float ratio = clampf(measured_rpm / max_rpm, 0.0f, 1.0f);

    for (int deg = 200; deg <= 340; deg += 2) {
        float rad = deg * (float)M_PI / 180.0f;
        oled_set_pixel(cx + (int)roundf(cosf(rad) * radius), cy + (int)roundf(sinf(rad) * radius), true);
    }

    for (int deg = 200; deg <= 340; deg += 20) {
        float rad = deg * (float)M_PI / 180.0f;
        int x0 = cx + (int)roundf(cosf(rad) * (radius - 4));
        int y0 = cy + (int)roundf(sinf(rad) * (radius - 4));
        int x1 = cx + (int)roundf(cosf(rad) * radius);
        int y1 = cy + (int)roundf(sinf(rad) * radius);
        oled_draw_line(x0, y0, x1, y1);
    }

    float target_ratio = clampf(setpoint_rpm / max_rpm, 0.0f, 1.0f);
    float target_rad = (200.0f + target_ratio * 140.0f) * (float)M_PI / 180.0f;
    oled_draw_line(cx + (int)roundf(cosf(target_rad) * 27), cy + (int)roundf(sinf(target_rad) * 27),
                   cx + (int)roundf(cosf(target_rad) * 34), cy + (int)roundf(sinf(target_rad) * 34));

    float needle_rad = (200.0f + ratio * 140.0f) * (float)M_PI / 180.0f;
    oled_draw_line(cx, cy, cx + (int)roundf(cosf(needle_rad) * 25), cy + (int)roundf(sinf(needle_rad) * 25));
    oled_fill_rect(cx - 1, cy - 1, 3, 3);
}

esp_err_t oled_init(void) {
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = OLED_I2C_SDA,
        .scl_io_num = OLED_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&bus_config, &i2c_bus), TAG, "bus");

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = OLED_I2C_ADDR,
        .scl_speed_hz = OLED_I2C_FREQ_HZ,
    };
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(i2c_bus, &dev_config, &oled_dev), TAG, "device");

    vTaskDelay(pdMS_TO_TICKS(100));
    const uint8_t init_sequence[] = {
        0xAE, 0x20, 0x00, 0xB0, 0xC8, 0x00, 0x10, 0x40,
        0x81, 0x7F, 0xA1, 0xA6, 0xA8, 0x3F, 0xA4, 0xD3,
        0x00, 0xD5, 0x80, 0xD9, 0xF1, 0xDA, 0x12, 0xDB,
        0x40, 0x8D, 0x14, 0xAF,
    };
    ESP_RETURN_ON_ERROR(oled_write_commands(init_sequence, sizeof(init_sequence)), TAG, "init");

    oled_ready = true;
    oled_clear();
    return oled_flush();
}

void oled_show_splash(void) {
    if (!oled_ready) {
        return;
    }

    oled_clear();
    oled_draw_centered_text(8, "CONTROL PID", 1);
    oled_draw_centered_text(24, "TACOMETRO", 1);
    oled_draw_centered_text(42, "OLED READY", 1);
    oled_commit_frame();
}

void oled_show_tachometer(float measured_rpm, float setpoint_rpm, float duty) {
    if (!oled_ready) {
        return;
    }

    int64_t now = esp_timer_get_time();
    if (last_refresh_us != 0 && now - last_refresh_us < OLED_REFRESH_INTERVAL_US) {
        return;
    }

    char measured[16];
    char bottom[24];

    oled_clear();
    oled_draw_centered_text(0, "RPM", 1);
    oled_draw_tach_arc(measured_rpm, setpoint_rpm);

    snprintf(measured, sizeof(measured), "%4.0f", measured_rpm);
    oled_draw_centered_text(21, measured, 2);

    snprintf(bottom, sizeof(bottom), "MED:%4.0f OBJ:%4.0f", measured_rpm, setpoint_rpm);
    oled_draw_text(0, 55, bottom, 1);

    int bar_w = (int)roundf(clampf(duty / 255.0f, 0.0f, 1.0f) * OLED_WIDTH);
    oled_fill_rect(0, 63, bar_w, 1);

    oled_commit_frame();
}

void oled_task(void *arg) {
    (void)arg;

    while (true) {
        control_status_t status;
        control_get_status(&status);
        oled_show_tachometer(status.rpm, status.setpoint, status.duty);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
