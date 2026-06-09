# Configuracion validada del control RPM

Este documento resume los parametros finales usados en la version validada del proyecto. La configuracion se definio a partir de pruebas con referencia de 500 RPM y posteriormente se ajusto para que la logica sea proporcional al rango completo de trabajo, de 0 a 6000 RPM.

## Archivo principal

Los parametros se encuentran en:

```text
main/config.h
```

## Parametros de referencia

```c
#define SAMPLE_TIME_MS 100
#define PULSES_PER_REV 1
#define DEFAULT_RPM_SETPOINT 500.0f
#define MIN_RPM_SETPOINT 0.0f
#define MAX_RPM_SETPOINT 6000.0f
```

El lazo de control se ejecuta cada 100 ms. La referencia permitida va desde 0 RPM hasta 6000 RPM. Una referencia de 0 RPM se usa como pausa del sistema.

## Parametros PID

```c
#define PID_KP 0.12f
#define PID_KI 0.01f
#define PID_KD 0.00f
```

Se mantuvo un control PI, con `Kd` en cero, debido a que la medicion por sensor optico de una marca por vuelta puede presentar ruido y picos. El termino derivativo puede amplificar esos cambios rapidos, por eso no se usa en la configuracion final.

## PWM y duty

```c
#define PWM_FREQ 20000
#define PWM_RES LEDC_TIMER_8_BIT
#define PWM_MAX_DUTY 255.0f
#define PWM_SLEW_STEP_UP 10.0f
#define PWM_SLEW_STEP_DOWN 35.0f
```

El PWM trabaja a 20 kHz con resolucion de 8 bits. Por esta razon el duty permitido esta entre 0 y 255.

El cambio del duty se limita para evitar golpes de potencia:

- Subida maxima por ciclo: 10 unidades.
- Bajada maxima por ciclo: 35 unidades.

Con esta relacion el sistema sube potencia de forma controlada, pero puede reducirla mas rapido si la medicion queda por encima de la referencia.

## Feedforward

```c
#define PWM_FEEDFORWARD_REF_RPM 500.0f
#define PWM_RUN_FEEDFORWARD_DUTY 190.0f
#define PWM_MIN_EFFECTIVE_DUTY 190.0f
#define PWM_RECOVERY_DUTY 200.0f
```

Se usa una base de duty antes de aplicar la correccion PID. El punto de calibracion usado fue:

```text
500 RPM -> duty base 190
```

Para referencias entre 0 y 500 RPM, el duty base se calcula proporcionalmente entre 0 y 190. Para referencias entre 500 y 6000 RPM, el duty base se interpola entre 190 y 255.

La expresion general es:

```text
CMD = duty_base_por_RPM + PIDOUT
```

## Arranque

```c
#define RPM_STARTUP_DUTY 255.0f
#define RPM_STARTUP_MIN_MS 500
#define RPM_STARTUP_MIN_RPM 80.0f
#define RPM_STARTUP_HANDOFF_CAL_RPM 500.0f
#define RPM_STARTUP_HANDOFF_CAL_DETECTED_RPM 425.0f
```

El motor arranca con duty 255 para vencer la friccion estatica. El sistema no cierra el lazo PID inmediatamente; primero espera un tiempo minimo y una lectura valida del sensor.

La transferencia a modo `RUNNING` se calcula con la relacion:

```text
425 / 500 = 0.85
```

Por tanto:

```text
RPM_handoff = max(80 RPM, referencia * 0.85)
```

## Banda progresiva de ayuda

```c
#define RPM_CONTROL_DEADBAND 90.0f
#define RPM_MIN_EFFECTIVE_BAND_RATIO 0.02f
#define RPM_MIN_EFFECTIVE_BAND_MIN 10.0f
```

La ayuda de duty minimo no se aplica como escalon. Se aplica como rampa cuando el RPM medido cae por debajo de la referencia.

La banda minima se calcula asi:

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

Con esta regla, un error pequeno no genera un salto fuerte de PWM. Si el error aumenta, la ayuda sube de forma progresiva.

## Filtro de RPM

```c
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
```

La medicion se realiza por periodo entre pulsos y no por conteo en ventana fija. Esto es mas adecuado para una sola marca por vuelta.

Tambien se usa una mediana de 5 periodos para rechazar pulsos aislados demasiado cortos o demasiado largos. Despues se aplica un filtro exponencial:

- Subida: `0.12`, para reducir picos altos falsos.
- Bajada lejos de la referencia: `0.50`, para que la lectura caiga con mayor rapidez.
- Bajada cerca de la referencia: `0.45`, para evitar saltos bruscos cerca del punto de trabajo.

## Estado esperado

Durante operacion normal el monitor serial presenta lineas de este tipo:

```text
RPM: 500.00 | REF: 500.00 | PIDOUT: ... | CMD: ... | DUTY: ... | MODE: RUNNING
```

El modo `STARTING` aparece durante arranque o recuperacion de movimiento. El modo `RUNNING` aparece cuando ya existe lectura valida del sensor y el control PID esta cerrado.
