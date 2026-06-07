#include "motor.h"
#include "rpm.h"
#include "control.h"
#include "oled.h"
#include "uart_cmd.h"
#include "wifi_connect.h"
#include "http_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdbool.h>

static const char *TAG = "APP";

void app_main(void) {
    bool oled_ok = false;

    motor_init();
    motor_set_direction(1);

    rpm_init();

    if (oled_init() == ESP_OK) {
        oled_ok = true;
        oled_show_splash();
        vTaskDelay(pdMS_TO_TICKS(1500));
    } else {
        ESP_LOGW(TAG, "No se pudo inicializar la OLED SSD1306");
    }

    uart_cmd_init();

    xTaskCreate(control_task, "control_task", 4096, NULL, 5, NULL);
    if (oled_ok) {
        xTaskCreate(oled_task, "oled_task", 4096, NULL, 3, NULL);
    }

    if (wifi_connect_sta() == ESP_OK) {
        ESP_ERROR_CHECK(rpm_http_server_start());
    } else {
        ESP_LOGW(TAG, "HTTP no iniciado; configure WiFi en menuconfig para habilitarlo");
    }
}
