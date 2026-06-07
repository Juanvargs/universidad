#pragma once

typedef struct {
    float Kp;
    float Ki;
    float Kd;

    float integral;
    float prev_error;
    float prev_measurement;
    float output;
    float out_min;
    float out_max;
} pid_t;

void pid_init(pid_t *pid, float Kp, float Ki, float Kd);
void pid_set_output_limits(pid_t *pid, float min, float max);
float pid_compute(pid_t *pid, float setpoint, float measurement, float dt);
