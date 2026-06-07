#pragma once
#include <stdint.h>
void motor_init(void);
void motor_set_speed(uint8_t duty);
void motor_set_direction(int dir);