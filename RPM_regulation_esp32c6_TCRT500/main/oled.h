#pragma once

#include "esp_err.h"

esp_err_t oled_init(void);
void oled_show_splash(void);
void oled_show_tachometer(float measured_rpm, float setpoint_rpm, float duty);
void oled_task(void *arg);
