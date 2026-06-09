# Control PID, PWM y duty

Este documento explica el control principal del proyecto. Es el tema central del sistema: mide RPM, compara con una referencia, calcula una correccion y aplica PWM al motor.

## Archivos involucrados

| Archivo | Funcion |
| --- | --- |
| `main/config.h` | Parametros del control |
| `main/pid.c` | Formula PID |
| `main/control.c` | Maquina de estados, feedforward, PID y duty final |
| `main/motor.c` | Aplicacion fisica del PWM |
| `main/rpm.c` | RPM medida usada como realimentacion |

## Que es un PID

PID significa:

```text
P = Proporcional
I = Integral
D = Derivativo
```

Un PID calcula una salida a partir del error:

```text
error = referencia - medicion
```

En este proyecto:

```text
error = RPM_referencia - RPM_medida
```

Si el motor gira por debajo de la referencia, el error es positivo y el control debe aumentar potencia. Si gira por encima, el error es negativo y el control debe reducir potencia.

## Formula implementada

Archivo:

```text
main/pid.c
```

Funcion:

```c
float pid_compute(pid_t *pid, float setpoint, float measurement, float dt)
```

Codigo usado:

```c
float error = setpoint - measurement;
float d_input = measurement - pid->prev_measurement;

pid->integral += pid->Ki * error * dt;

float output = pid->Kp * error +
               pid->integral -
               pid->Kd * (d_input / dt);
```

La salida se limita:

```c
output = clampf_pid(output, pid->out_min, pid->out_max);
```

## Parametros finales

Archivo:

```text
main/config.h
```

Codigo:

```c
#define PID_KP 0.12f
#define PID_KI 0.01f
#define PID_KD 0.00f
```

Interpretacion:

- `Kp = 0.12`: responde al error actual.
- `Ki = 0.01`: corrige error acumulado con el tiempo.
- `Kd = 0.00`: no se usa derivativo para no amplificar ruido del sensor optico.

## Ejemplo numerico del PID

Suposiciones:

```text
Referencia = 500 RPM
Medicion = 450 RPM
dt = 0.1 s
Kp = 0.12
Ki = 0.01
Kd = 0
integral_anterior = 0
```

Calculo:

```text
error = 500 - 450 = 50 RPM
P = 0.12 * 50 = 6
I = 0 + 0.01 * 50 * 0.1 = 0.05
D = 0
PIDOUT = 6 + 0.05 = 6.05
```

Ese `PIDOUT` no es el duty total. Es una correccion que se suma a una base de duty.

## Por que se usa feedforward

Un motor real necesita cierta potencia minima para sostener una velocidad. Si el PID parte desde cero, puede tardar mucho o entrar en ciclos bruscos.

Por eso se usa:

```text
CMD = duty_base_por_RPM + PIDOUT
```

Archivo:

```text
main/control.c
```

Funcion:

```c
static float duty_from_calibration(float setpoint, float duty_at_ref, float duty_at_max)
```

Parametros:

```c
#define PWM_FEEDFORWARD_REF_RPM 500.0f
#define PWM_RUN_FEEDFORWARD_DUTY 190.0f
#define PWM_MAX_DUTY 255.0f
#define MAX_RPM_SETPOINT 6000.0f
```

Se usa esta calibracion:

```text
500 RPM -> duty 190
6000 RPM -> duty 255
```

## Calculo de duty base

Para referencias de 0 a 500 RPM:

```c
float duty = duty_at_ref * (setpoint / PWM_FEEDFORWARD_REF_RPM);
```

Ejemplo a 250 RPM:

```text
duty = 190 * (250 / 500)
duty = 95
```

Para referencias de 500 a 6000 RPM:

```c
float ratio = (setpoint - PWM_FEEDFORWARD_REF_RPM) / rpm_span;
float duty = duty_at_ref + ratio * (duty_at_max - duty_at_ref);
```

Ejemplo a 1000 RPM:

```text
rpm_span = 6000 - 500 = 5500
ratio = (1000 - 500) / 5500 = 0.0909
duty = 190 + 0.0909 * (255 - 190)
duty = 190 + 5.91
duty = 195.91
```

Ejemplo a 3000 RPM:

```text
ratio = (3000 - 500) / 5500 = 0.4545
duty = 190 + 0.4545 * 65
duty = 219.54
```

## PWM y duty

Archivo:

```text
main/motor.c
```

Configuracion:

```c
ledc_timer_config_t timer = {
    .duty_resolution = PWM_RES,
    .freq_hz = PWM_FREQ
};
```

Parametros:

```c
#define PWM_FREQ 20000
#define PWM_RES LEDC_TIMER_8_BIT
#define PWM_MAX_DUTY 255.0f
```

Como la resolucion es de 8 bits:

```text
2^8 = 256 niveles
rango = 0 a 255
```

Equivalencia aproximada:

```text
duty 0   -> 0%
duty 64  -> 25%
duty 127 -> 50%
duty 190 -> 74.5%
duty 255 -> 100%
```

Porcentaje de duty 190:

```text
190 / 255 = 0.745
0.745 * 100 = 74.5%
```

## Arranque del motor

El motor puede no arrancar con duty bajo porque debe vencer friccion estatica. Por eso existe el modo `STARTING`.

Parametros:

```c
#define RPM_STARTUP_DUTY 255.0f
#define RPM_STARTUP_MIN_MS 500
#define RPM_STARTUP_MIN_RPM 80.0f
```

En `control.c`:

```c
if (mode == CONTROL_STARTING) {
    target_duty = startup_duty_for_elapsed(mode_ms);
    output = target_duty;
    immediate_kick = target_duty >= PWM_KICKSTART_DUTY;
}
```

Mientras esta arrancando, el motor recibe duty maximo hasta que el sensor indique suficiente movimiento.

## Handoff: paso de STARTING a RUNNING

El paso a control cerrado usa:

```c
float ratio = RPM_STARTUP_HANDOFF_CAL_DETECTED_RPM / RPM_STARTUP_HANDOFF_CAL_RPM;
float handoff = setpoint * ratio;
return maxf_local(RPM_STARTUP_MIN_RPM, handoff);
```

Parametros:

```c
#define RPM_STARTUP_HANDOFF_CAL_RPM 500.0f
#define RPM_STARTUP_HANDOFF_CAL_DETECTED_RPM 425.0f
```

Calculo:

```text
ratio = 425 / 500 = 0.85
handoff = referencia * 0.85
```

Ejemplos:

| Referencia | Handoff |
| ---: | ---: |
| 100 RPM | max(80, 85) = 85 RPM |
| 500 RPM | max(80, 425) = 425 RPM |
| 1000 RPM | max(80, 850) = 850 RPM |
| 3000 RPM | max(80, 2550) = 2550 RPM |

## Rampa progresiva contra saltos

Se detecto que una ayuda de duty minimo aplicada como escalon podia generar saltos fuertes. Por eso se usa una rampa progresiva.

Parametros:

```c
#define RPM_CONTROL_DEADBAND 90.0f
#define RPM_MIN_EFFECTIVE_BAND_RATIO 0.02f
#define RPM_MIN_EFFECTIVE_BAND_MIN 10.0f
```

Funcion:

```c
static float min_effective_band_for_setpoint(float setpoint) {
    float proportional_band = setpoint * RPM_MIN_EFFECTIVE_BAND_RATIO;
    return maxf_local(RPM_MIN_EFFECTIVE_BAND_MIN, proportional_band);
}
```

Calculo:

```text
banda = max(10 RPM, referencia * 0.02)
```

Ejemplos:

| Referencia | Banda minima |
| ---: | ---: |
| 500 RPM | 10 RPM |
| 1000 RPM | 20 RPM |
| 3000 RPM | 60 RPM |
| 6000 RPM | 120 RPM |

La rampa se aplica en `apply_motor_feedforward()`:

```c
float underspeed = setpoint - current_rpm;

if (underspeed > min_effective_band) {
    float assist_span = RPM_CONTROL_DEADBAND - min_effective_band;
    float assist_ratio = (underspeed - min_effective_band) / assist_span;
    float assist_floor = mixf_local(min_effective_duty, recovery_duty, assist_ratio);

    if (command < assist_floor) {
        command = mixf_local(command, assist_floor, assist_ratio);
    }
}
```

Ejemplo con referencia 1000 RPM:

```text
min_effective_band = max(10, 1000*0.02) = 20 RPM
RPM_CONTROL_DEADBAND = 90 RPM
```

Si la medicion es 980 RPM:

```text
underspeed = 1000 - 980 = 20 RPM
```

No se aplica ayuda fuerte porque apenas esta en la banda minima.

Si la medicion es 950 RPM:

```text
underspeed = 1000 - 950 = 50 RPM
assist_span = 90 - 20 = 70
assist_ratio = (50 - 20) / 70 = 0.428
```

La ayuda entra parcialmente, no de golpe.

## Slew rate del duty aplicado

Aunque `CMD` cambie, el duty aplicado al motor se limita:

```c
applied_duty = slew_toward(applied_duty, target_duty,
                           PWM_SLEW_STEP_UP,
                           PWM_SLEW_STEP_DOWN);
```

Parametros:

```c
#define PWM_SLEW_STEP_UP 10.0f
#define PWM_SLEW_STEP_DOWN 35.0f
```

Esto significa:

```text
Subida maxima por ciclo: 10
Bajada maxima por ciclo: 35
Periodo del ciclo: 100 ms
```

Ejemplo:

```text
duty actual = 120
CMD nuevo = 190
PWM_SLEW_STEP_UP = 10
duty aplicado = 130
```

En el siguiente ciclo podria subir a 140, luego 150, hasta alcanzar el comando.

## Resumen del calculo completo

Para cada ciclo de control:

```text
1. current_rpm = rpm_get()
2. error = setpoint - current_rpm
3. PIDOUT = pid_compute(...)
4. base_duty = feedforward_duty_for_setpoint(setpoint)
5. CMD = base_duty + PIDOUT
6. CMD se ajusta con rampa de ayuda si hay baja velocidad
7. DUTY = slew_toward(DUTY_anterior, CMD)
8. motor_set_speed(DUTY)
```

Esto aparece en el monitor como:

```text
RPM: ... | REF: ... | PIDOUT: ... | CMD: ... | DUTY: ... | MODE: ...
```
