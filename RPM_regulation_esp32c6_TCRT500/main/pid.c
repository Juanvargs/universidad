#include "pid.h"

static float clampf_pid(float value, float min, float max) {
    if (value < min) {
        return min;
    }
    if (value > max) {
        return max;
    }
    return value;
}

void pid_init(pid_t *pid, float Kp, float Ki, float Kd) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->prev_measurement = 0.0f;
    pid->output = 0.0f;
    pid->out_min = 0.0f;
    pid->out_max = 255.0f;
}

void pid_set_output_limits(pid_t *pid, float min, float max) {
    if (min >= max) {
        return;
    }

    pid->out_min = min;
    pid->out_max = max;
    pid->output = clampf_pid(pid->output, min, max);
    pid->integral = clampf_pid(pid->integral, min, max);
}

float pid_compute(pid_t *pid, float setpoint, float measurement, float dt) {
    if (dt <= 0.0f) {
        return pid->output;
    }

    float error = setpoint - measurement;
    float d_input = measurement - pid->prev_measurement;

    pid->integral += pid->Ki * error * dt;
    pid->integral = clampf_pid(pid->integral, pid->out_min, pid->out_max);

    float output = pid->Kp * error +
                   pid->integral -
                   pid->Kd * (d_input / dt);

    output = clampf_pid(output, pid->out_min, pid->out_max);

    pid->prev_error = error;
    pid->prev_measurement = measurement;
    pid->output = output;
    return output;
}
