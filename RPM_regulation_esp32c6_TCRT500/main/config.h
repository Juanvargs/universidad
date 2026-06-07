#pragma once

// Pines22
#define PIN_PWM     4
#define PIN_IN1     5
#define PIN_IN2     8
#define PIN_SENSOR  1

// OLED SSD1306 I2C
#define OLED_I2C_SDA      6
#define OLED_I2C_SCL      7
#define OLED_I2C_ADDR     0x3C
#define OLED_I2C_FREQ_HZ  400000

// PWM
#define PWM_FREQ    20000
#define PWM_RES     LEDC_TIMER_8_BIT

// Control
#define SAMPLE_TIME_MS 100
#define PULSES_PER_REV 1
#define DEFAULT_RPM_SETPOINT 500.0f
#define MIN_RPM_SETPOINT 0.0f
#define MAX_RPM_SETPOINT 6000.0f

// Initial PID tuning. These values must be calibrated on the real motor.
#define PID_KP 0.12f
#define PID_KI 0.01f
#define PID_KD 0.00f
#define PWM_MAX_DUTY 255.0f
#define PWM_SLEW_STEP_UP 10.0f
#define PWM_SLEW_STEP_DOWN 35.0f
// Feedforward calibration: duty needed around the known 500 RPM operating point.
#define PWM_FEEDFORWARD_REF_RPM 500.0f
#define PWM_RUN_FEEDFORWARD_DUTY 190.0f
#define PWM_MIN_EFFECTIVE_DUTY 190.0f
#define PWM_RECOVERY_DUTY 200.0f
#define PWM_KICKSTART_DUTY 255.0f
#define RPM_CONTROL_DEADBAND 90.0f

// Open-loop startup before closing the RPM feedback loop.
#define RPM_STARTUP_DUTY 255.0f
#define RPM_STARTUP_MIN_MS 500
#define RPM_STARTUP_MIN_RPM 80.0f
#define RPM_STARTUP_HANDOFF_CAL_RPM 500.0f
#define RPM_STARTUP_HANDOFF_CAL_DETECTED_RPM 425.0f

// RPM measurement limits and signal timeout.
#define RPM_SENSOR_MAX_VALID_RPM 6000.0f
#define RPM_MEASUREMENT_TIMEOUT_MS 1500
#define RPM_SIGNAL_TIMEOUT_MS 1500
#define RPM_FILTER_ALPHA_RISE 0.12f
#define RPM_FILTER_ALPHA_FALL 0.50f
#define RPM_FILTER_ALPHA_FALL_NEAR 0.45f
#define RPM_FILTER_NEAR_SETPOINT_BAND 100.0f
#define RPM_PERIOD_FILTER_SAMPLES 5
#define RPM_STALE_MIN_MS 250
#define RPM_STALE_PERIOD_MULTIPLIER 2.5f
#define RPM_MIN_INTERVAL_RATIO 0.45f
