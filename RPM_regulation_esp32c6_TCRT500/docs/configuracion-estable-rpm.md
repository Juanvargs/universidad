# Configuracion estable antes de filtro adaptativo

Esta configuracion fue la que el usuario reporto como estable antes de probar el filtro adaptativo cerca de la referencia.

Archivo principal: `main/config.h`

```c
#define DEFAULT_RPM_SETPOINT 500.0f
#define PID_KP 0.12f
#define PID_KI 0.01f
#define PID_KD 0.00f
#define PWM_SLEW_STEP_UP 10.0f
#define PWM_SLEW_STEP_DOWN 35.0f
#define PWM_RUN_FEEDFORWARD_DUTY 190.0f
#define PWM_MIN_EFFECTIVE_DUTY 190.0f
#define PWM_RECOVERY_DUTY 200.0f
#define RPM_CONTROL_DEADBAND 90.0f
#define RPM_FILTER_ALPHA_RISE 0.12f
#define RPM_FILTER_ALPHA_FALL 0.50f
#define RPM_PERIOD_FILTER_SAMPLES 5
```

Si el filtro adaptativo no funciona bien, volver a un solo valor fijo:

```c
#define RPM_FILTER_ALPHA_FALL 0.50f
```

Y en `main/rpm.c`, usar directamente:

```c
float alpha = (rpm < rpm_filtered) ? RPM_FILTER_ALPHA_FALL : RPM_FILTER_ALPHA_RISE;
```
