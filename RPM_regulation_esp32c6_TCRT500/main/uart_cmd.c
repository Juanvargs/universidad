#include "uart_cmd.h"

#include "config.h"
#include "control.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define UART_CMD_PORT UART_NUM_0
#define UART_CMD_BAUD 115200
#define UART_CMD_BUF_SIZE 128

static const char *TAG = "UART_CMD";

static void trim_line(char *line) {
    size_t len = strlen(line);
    while (len > 0 && isspace((unsigned char)line[len - 1])) {
        line[--len] = '\0';
    }
    while (*line && isspace((unsigned char)*line)) {
        memmove(line, line + 1, strlen(line));
    }
}

static bool parse_rpm_command(const char *line, float *rpm) {
    char *end = NULL;
    if (strncasecmp(line, "rpm", 3) == 0 || strncasecmp(line, "set", 3) == 0 ||
        strncasecmp(line, "ref", 3) == 0) {
        line += 3;
    }

    while (*line && (*line == ':' || *line == '=' || isspace((unsigned char)*line))) {
        line++;
    }

    float value = strtof(line, &end);
    while (*end && isspace((unsigned char)*end)) {
        end++;
    }

    if (end == line || *end != '\0' || value < MIN_RPM_SETPOINT || value > MAX_RPM_SETPOINT) {
        return false;
    }

    *rpm = value;
    return true;
}

static void uart_cmd_task(void *arg) {
    char line[UART_CMD_BUF_SIZE];
    size_t len = 0;

    ESP_LOGI(TAG, "Use: rpm 1500, set 1500 or ref=1500. Rango: %.0f-%.0f RPM",
             MIN_RPM_SETPOINT, MAX_RPM_SETPOINT);
    while (true) {
        uint8_t ch;
        int read = uart_read_bytes(UART_CMD_PORT, &ch, 1, pdMS_TO_TICKS(100));
        if (read <= 0) {
            continue;
        }

        if (ch == '\r' || ch == '\n') {
            if (len == 0) {
                continue;
            }

            line[len] = '\0';
            trim_line(line);

            float rpm;
            if (parse_rpm_command(line, &rpm)) {
                control_set_setpoint(rpm);
                ESP_LOGI(TAG, "Referencia RPM actualizada: %.0f", rpm);
            } else {
                ESP_LOGW(TAG, "Comando invalido. Use: rpm 1500, rango %.0f-%.0f RPM",
                         MIN_RPM_SETPOINT, MAX_RPM_SETPOINT);
            }

            len = 0;
            continue;
        }

        if (len < sizeof(line) - 1) {
            line[len++] = (char)ch;
        } else {
            len = 0;
            ESP_LOGW(TAG, "Linea demasiado larga");
        }
    }
}

void uart_cmd_init(void) {
    uart_config_t config = {
        .baud_rate = UART_CMD_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(UART_CMD_PORT, UART_CMD_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(UART_CMD_PORT, &config));
    ESP_ERROR_CHECK(uart_set_pin(UART_CMD_PORT, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE,
                                 UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));

    xTaskCreate(uart_cmd_task, "uart_cmd_task", 3072, NULL, 4, NULL);
}
