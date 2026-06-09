# Guia de ajuste y prueba del control PID

Esta guia define un procedimiento ordenado para probar y ajustar el control de RPM del motor DC. El objetivo es mantener una velocidad cercana a la referencia sin saltos bruscos de PWM.

## Condiciones previas

Antes de ajustar parametros de control se debe verificar:

- Tierra comun entre ESP32-C6, L293D, fuente del motor, sensor y OLED.
- Sensor TCRT5000 alimentado a 3.3 V.
- Salida `OUT` del sensor conectada a GPIO1.
- Marca negra/blanca alineada con el sensor.
- Potenciometro del sensor ajustado para que el LED cambie de estado al pasar la marca.
- Motor conectado al L293D y fuente externa activa.

Si la lectura de RPM cambia cuando el motor no esta cerca del sensor, primero se debe corregir cableado, ruido electrico o ajuste del TCRT5000.

## Variables principales

Los parametros se ajustan en:

```text
main/config.h
```

Parametros PID:

```c
#define PID_KP 0.12f
#define PID_KI 0.01f
#define PID_KD 0.00f
```

Parametros de cambio de duty:

```c
#define PWM_SLEW_STEP_UP 10.0f
#define PWM_SLEW_STEP_DOWN 35.0f
```

Parametros de base de PWM:

```c
#define PWM_FEEDFORWARD_REF_RPM 500.0f
#define PWM_RUN_FEEDFORWARD_DUTY 190.0f
#define PWM_MIN_EFFECTIVE_DUTY 190.0f
#define PWM_RECOVERY_DUTY 200.0f
```

Parametros de filtro:

```c
#define RPM_FILTER_ALPHA_RISE 0.12f
#define RPM_FILTER_ALPHA_FALL 0.50f
#define RPM_FILTER_ALPHA_FALL_NEAR 0.45f
#define RPM_PERIOD_FILTER_SAMPLES 5
```

## Que datos registrar

Para cada prueba se deben copiar entre 20 y 40 lineas consecutivas del monitor serial:

```text
RPM: 502.00 | REF: 500.00 | PIDOUT: ... | CMD: ... | DUTY: ... | MODE: RUNNING
```

Tambien se debe anotar:

- Referencia usada.
- Voltaje de la fuente del motor.
- Distancia aproximada entre sensor y marca.
- Estado de carga del motor.
- Si el motor arranco desde reposo o ya estaba girando.

## Prueba base a 500 RPM

1. Flashear el firmware.
2. Abrir monitor serial.
3. Enviar:

```text
rpm 500
```

4. Esperar el cambio de `STARTING` a `RUNNING`.
5. Registrar 10 segundos de datos.

El comportamiento esperado es:

- Arranque con duty alto.
- Entrada a `RUNNING` cuando hay lectura valida.
- RPM cercana a 500.
- Duty sin saltos grandes cuando la RPM cruza levemente por encima o por debajo de la referencia.

## Pruebas por rango

Se recomienda probar en este orden:

```text
rpm 500
rpm 1000
rpm 3000
rpm 6000
rpm 0
```

Cada valor debe registrarse por separado. El cambio entre referencias debe hacerse despues de que el sistema este estable en la referencia anterior.

## Interpretacion de resultados

### RPM se pasa mucho de la referencia

Posibles causas:

- `Kp` demasiado alto.
- `Ki` acumulando demasiada correccion.
- Sensor generando picos falsos.
- Duty base alto para el montaje fisico.

Acciones:

- Confirmar primero que el sensor no presenta pulsos falsos.
- Reducir `PID_KP` en pasos pequenos.
- Reducir `PID_KI` si la salida queda sostenida alta.

### RPM queda por debajo de la referencia

Posibles causas:

- Fuente con poca corriente.
- Duty base insuficiente.
- Motor con carga mecanica alta.
- Sensor mal alineado y lectura intermitente.

Acciones:

- Verificar fuente y tierra comun.
- Revisar que el motor alcance velocidad con duty alto.
- Ajustar `PWM_RUN_FEEDFORWARD_DUTY` solo si el problema se repite con lectura estable.

### Duty cambia demasiado brusco

Posibles causas:

- Error de RPM grande.
- Lectura del sensor con picos.
- Rampa de ayuda entrando en recuperacion.

Acciones:

- Revisar primero el log de RPM.
- Si el RPM salta pero el motor suena constante, el problema esta en medicion.
- Si el RPM realmente cae, el control esta intentando recuperar velocidad.

### RPM cae lento en pantalla

La caida visual depende del filtro:

```c
#define RPM_FILTER_ALPHA_FALL 0.50f
#define RPM_FILTER_ALPHA_FALL_NEAR 0.45f
```

Un valor mas alto hace que la lectura baje mas rapido, pero tambien puede volverla mas brusca. Un valor mas bajo hace que la lectura sea mas suave, pero mas lenta.

## Criterios para modificar parametros

Los cambios deben hacerse de uno en uno. Despues de cada cambio se debe compilar, flashear y registrar un nuevo log.

Orden recomendado:

1. Verificar sensor y cableado.
2. Verificar feedforward y duty base.
3. Ajustar `PWM_SLEW_STEP_UP` y `PWM_SLEW_STEP_DOWN` si hay golpes de duty.
4. Ajustar `PID_KP`.
5. Ajustar `PID_KI`.
6. Mantener `PID_KD` en cero salvo que la medicion sea muy limpia.

## Regla de decision

No se debe ajustar PID para corregir una medicion falsa. Si el monitor muestra RPM altas mientras el motor no esta frente al sensor, la causa esta en la senal del sensor, tierra comun, ruido electrico o cableado.

Si la medicion es coherente y el motor realmente cambia de velocidad, entonces si procede ajustar PID, feedforward o slew rate.

## Evidencia para presentacion

Para documentar el funcionamiento se recomienda incluir:

- Foto del montaje.
- Tabla de conexiones.
- Video corto del arranque y regulacion.
- Captura del monitor serial.
- Captura de la pagina web.
- Explicacion de `STARTING` y `RUNNING`.

El log mas importante debe mostrar:

```text
REF
RPM
PIDOUT
CMD
DUTY
MODE
```

Con esos datos se evidencia que el sistema mide, compara, calcula y actua sobre el motor.
