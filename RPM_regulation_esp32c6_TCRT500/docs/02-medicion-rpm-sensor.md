# Medicion de RPM y sensor

La velocidad se mide con el TCRT5000 usando una marca de contraste en el eje o disco del motor. Cada vuelta genera un pulso.

## Parametros usados

Archivo:

```text
main/config.h
```

Codigo:

```c
#define PULSES_PER_REV 1
#define RPM_SENSOR_MAX_VALID_RPM 6000.0f
#define RPM_MEASUREMENT_TIMEOUT_MS 1500
#define RPM_SIGNAL_TIMEOUT_MS 1500
#define RPM_PERIOD_FILTER_SAMPLES 5
#define RPM_MIN_INTERVAL_RATIO 0.45f
```

## Por que se mide por periodo

Con una sola marca por vuelta, contar pulsos en una ventana fija de 100 ms da poca resolucion:

```text
1 pulso en 100 ms -> 600 RPM
2 pulsos en 100 ms -> 1200 RPM
3 pulsos en 100 ms -> 1800 RPM
```

Por eso se usa el tiempo entre dos pulsos consecutivos.

## Formula de RPM

En `rpm.c`, dentro de `rpm_get()`, se calcula:

```c
rpm = 60000000.0f / ((float)interval_us * (float)PULSES_PER_REV);
```

La formula sale de:

```text
1 segundo = 1,000,000 us
1 minuto = 60 segundos
1 minuto = 60,000,000 us
```

Si se mide el periodo entre pulsos:

```text
RPM = 60,000,000 / (periodo_us * pulsos_por_vuelta)
```

Ejemplo con una marca por vuelta:

```text
periodo = 120,000 us
PULSES_PER_REV = 1
RPM = 60,000,000 / (120,000 * 1)
RPM = 500
```

Para 1000 RPM:

```text
periodo = 60,000 us
RPM = 60,000,000 / 60,000
RPM = 1000
```

Para 6000 RPM:

```text
periodo = 10,000 us
RPM = 60,000,000 / 10,000
RPM = 6000
```

## Interrupcion del sensor

El sensor se atiende en una ISR:

```c
static void IRAM_ATTR isr_handler(void *arg) {
    int64_t now = esp_timer_get_time();
    int64_t interval = now - last_pulse_time;
    ...
}
```

Cada pulso calcula el intervalo desde el pulso anterior. Si es el primer pulso o paso demasiado tiempo, se reinicia la medicion:

```c
if (last_pulse_time == 0 ||
    interval > ((int64_t)RPM_MEASUREMENT_TIMEOUT_MS * 1000)) {
    pulse_interval_us = 0;
    ...
    return;
}
```

Esto evita usar un periodo viejo cuando el motor se detuvo o cuando el sensor dejo de detectar.

## Rechazo de pulsos imposibles

Se define:

```c
#define MIN_VALID_INTERVAL_US ((int64_t)(60000000.0f / (RPM_SENSOR_MAX_VALID_RPM * PULSES_PER_REV)))
```

Con:

```text
RPM_SENSOR_MAX_VALID_RPM = 6000
PULSES_PER_REV = 1
```

El intervalo minimo valido es:

```text
MIN_VALID_INTERVAL_US = 60,000,000 / (6000 * 1)
MIN_VALID_INTERVAL_US = 10,000 us
```

Si llega un pulso antes de 10 ms, implicaria mas de 6000 RPM y se rechaza.

Tambien se rechazan pulsos demasiado cortos respecto al intervalo anterior:

```c
int64_t min_plausible_interval = (int64_t)((float)pulse_interval_us * RPM_MIN_INTERVAL_RATIO);
if (interval < min_plausible_interval) {
    interval_valid = false;
}
```

Con `RPM_MIN_INTERVAL_RATIO = 0.45`, si el periodo anterior fue 120,000 us, un nuevo periodo menor que:

```text
120,000 * 0.45 = 54,000 us
```

se considera sospechoso. Esto reduce picos falsos por ruido o rebote del sensor.

## Mediana de periodos

El codigo guarda varios periodos:

```c
static volatile int64_t pulse_intervals_us[RPM_PERIOD_FILTER_SAMPLES] = {0};
```

Luego usa:

```c
interval_us = median_interval_us(interval_samples, interval_count);
```

La mediana no promedia todos los valores; toma el valor central ordenado. Si hay un pulso falso aislado, la mediana lo ignora mejor que un promedio simple.

Ejemplo:

```text
Periodos medidos: 118000, 121000, 119000, 60000, 120000
Ordenados:        60000, 118000, 119000, 120000, 121000
Mediana:          119000
```

El valor falso de 60000 us no domina la medicion final.

## Filtro exponencial

Despues de calcular RPM se aplica:

```c
rpm_filtered = alpha * rpm + (1.0f - alpha) * rpm_filtered;
```

Parametros:

```c
#define RPM_FILTER_ALPHA_RISE 0.12f
#define RPM_FILTER_ALPHA_FALL 0.50f
#define RPM_FILTER_ALPHA_FALL_NEAR 0.45f
#define RPM_FILTER_NEAR_SETPOINT_BAND 100.0f
```

Si la RPM sube, se usa `0.12` para no reaccionar demasiado fuerte a picos altos. Si baja, se usa `0.50` o `0.45` para que la caida se vea mas rapido.

Ejemplo de subida:

```text
rpm_filtrada_anterior = 500
rpm_nueva = 900
alpha = 0.12
rpm_filtrada = 0.12*900 + 0.88*500
rpm_filtrada = 548
```

Aunque la lectura instantanea diga 900, el valor entregado al PID sube solo a 548.

Ejemplo de bajada:

```text
rpm_filtrada_anterior = 500
rpm_nueva = 300
alpha = 0.50
rpm_filtrada = 0.50*300 + 0.50*500
rpm_filtrada = 400
```

La bajada responde mas rapido que la subida.
