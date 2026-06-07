#pragma once

typedef struct {
    float rpm;
    float setpoint;
    float output;
    float command;
    float duty;
    float min_setpoint;
    float max_setpoint;
    const char *mode;
} control_status_t;

void control_task(void *arg);
void control_set_setpoint(float rpm);
float control_get_setpoint(void);
void control_get_status(control_status_t *status);
