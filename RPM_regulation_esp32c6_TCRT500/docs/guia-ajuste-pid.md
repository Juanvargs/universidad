# Guia de ajuste PID del proyecto RPM

Esta guia define como recoger datos para ajustar el control de RPM del motor DC con ESP32-C6, L293D y sensor TCRT500.

## Objetivo

Lograr que el motor se acerque a la referencia de RPM sin ciclos bruscos de:

```text
DUTY alto -> RPM se pasa -> DUTY baja a 0 -> RPM cae -> DUTY vuelve alto
```

La meta es que el `DUTY` cambie de forma progresiva y que la `RPM` medida se estabilice cerca de la referencia.

## Valores a ajustar

Los valores estan en `main/config.h`:

```c
#define PID_KP 0.12f
#define PID_KI 0.01f
#define PID_KD 0.00f
#define PWM_SLEW_STEP 30.0f
#define PWM_FEEDFORWARD_REF_RPM 500.0f
#define PWM_RUN_FEEDFORWARD_DUTY 190.0f
```

- `PID_KP`: reaccion inmediata al error. Si es muy alto, oscila; si es muy bajo, responde lento.
- `PID_KI`: corrige error persistente. Si es muy alto, acumula y se pasa.
- `PID_KD`: frena cambios rapidos en la medicion. Puede ayudar, pero tambien amplifica ruido del sensor.
- `PWM_SLEW_STEP`: limite de cambio del PWM por ciclo. Evita golpes electricos y mecanicos.
- `PWM_FEEDFORWARD_REF_RPM`: punto de RPM donde se calibro el duty base conocido.
- `PWM_RUN_FEEDFORWARD_DUTY`: duty base usado alrededor de ese punto de calibracion.

## Datos que se deben registrar

Para cada prueba, copiar del monitor serial al menos 20 a 40 lineas consecutivas como estas:

```text
I (...) PID: RPM: 478.12 | REF: 500.00 | PIDOUT: 12.50 | CMD: 202.50 | DUTY: 202 | MODE: RUNNING
I (...) PID: RPM: 606.13 | REF: 500.00 | PIDOUT: -18.40 | CMD: 190.00 | DUTY: 190 | MODE: RUNNING
```

Tambien anotar:

- Voltaje de alimentacion del motor.
- Si el motor esta sin carga o con carga.
- Distancia aproximada del TCRT500 a la marca.
- Tipo de marca usada: blanco/negro, disco ranurado, cinta reflectiva, etc.
- Si el motor arranco desde parado o ya venia girando.

## Prueba 1: referencia baja

Usar referencia inicial de 500 RPM.

Registrar:

- Primeros 10 segundos despues de encender.
- 10 segundos cuando parezca estable.
- Si se cae a 0 RPM o si se pasa mucho de 500 RPM.

## Prueba 2: referencia media

Cambiar temporalmente en `main/config.h`:

```c
#define DEFAULT_RPM_SETPOINT 800.0f
```

Flashear y registrar de nuevo:

- Primeros 10 segundos.
- 10 segundos estable.

## Prueba 3: sensor sin control de carga

Si la RPM cae a 0 mientras el motor sigue girando, el problema principal no es PID: es lectura del sensor.

Revisar:

- Alineacion del TCRT500.
- Contraste de la marca.
- Vibracion de cables.
- Tierra comun.
- Ruido electrico del motor.

## Criterios de ajuste

Si `DUTY` llega frecuentemente a 255 y la RPM sube tarde:

- Bajar `PWM_SLEW_STEP` solo si el golpe de arranque es muy brusco.
- Subir un poco `PID_KP` si responde demasiado lento.

Si la RPM se pasa mucho de la referencia y luego cae:

- Bajar `PID_KP`.
- Bajar `PID_KI`.
- Mantener `PID_KD` en 0 hasta confirmar que la medicion del sensor es estable.

Si la RPM queda siempre por debajo de la referencia:

- Subir `PID_KI` muy poco.
- Confirmar que el motor tiene suficiente voltaje y corriente.

Si la RPM medida salta mucho aunque el motor suene constante:

- No ajustar PID todavia.
- Mejorar primero la lectura del sensor.

## Correccion de lectura del sensor

Se observo una caida de RPM con forma:

```text
360 -> 288 -> 230 -> 184 -> 147 -> 117 -> 94
```

Esa secuencia corresponde al filtro exponencial decayendo cuando dejan de entrar pulsos reales. Por eso, antes de seguir ajustando PID, se refuerza la configuracion del GPIO del sensor.

Cambio aplicado:

```c
gpio_config_t sensor_config = {
    .pin_bit_mask = 1ULL << PIN_SENSOR,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_NEGEDGE,
};
```

Fundamento:

- `gpio_config_t` es el patron de ESP-IDF para configurar modo, pull-up/pull-down e interrupcion de un GPIO en una sola llamada.
- `GPIO_PULLUP_ENABLE` ayuda a que la entrada no quede flotando si la salida digital del sensor o el cableado se vuelven debiles/ruidosos.
- Se mantiene `GPIO_INTR_NEGEDGE` porque el codigo cuenta un pulso por cambio de alto a bajo, compatible con la lectura digital del TCRT500 ajustada con potenciometro.
- Se agrego reporte de error si el servicio de interrupciones GPIO o el handler del sensor no quedan instalados.

## Proteccion por perdida de sensor y arranque

Problema observado:

Cuando el sensor se aleja del motor, el firmware deja de recibir pulsos y calcula:

```text
RPM: 0.00 | REF: 500.00
```

Esto es coherente para el PID: si la referencia es 500 RPM y la medicion es 0 RPM, el controlador aumenta `OUT` y `DUTY`. El riesgo es que una falla de sensor, cable suelto o mala alineacion pueda llevar el motor a maxima potencia aunque el problema no sea falta de velocidad real.

Cambio aplicado:

```c
#define PWM_RUN_FEEDFORWARD_DUTY 170.0f
#define PWM_MIN_EFFECTIVE_DUTY 175.0f
#define PWM_KICKSTART_DUTY 255.0f
#define PWM_KICKSTART_MS 1000
#define PWM_KICKSTART_PERIOD_MS 1500
#define RPM_CONTROL_DEADBAND 100.0f

#define RPM_STARTUP_DUTY 255.0f
#define RPM_STARTUP_MIN_MS 500
#define RPM_STARTUP_MIN_RPM 80.0f
#define RPM_STARTUP_HANDOFF_RATIO 0.85f
#define RPM_STARTUP_HANDOFF_MIN_RPM 350.0f
#define RPM_SIGNAL_TIMEOUT_MS 1000
```

La primera proteccion aplicada era incompleta porque trataba igual el arranque y la perdida real de sensor. Mantener el motor en `DUTY=80` despues de no detectar pulsos tampoco es adecuado para este montaje: si ese duty no vence la friccion, el motor nunca vuelve a arrancar por si solo.

Revision con log real:

```text
RPM: 92.30 | REF: 500.00 | OUT: 160.00 | DUTY: 72 | MODE: RUNNING
RPM: 373.84 | REF: 500.00 | OUT: 22.83 | DUTY: 54 | MODE: RUNNING
RPM: 779.07 | REF: 500.00 | OUT: 0.00 | DUTY: 36 | MODE: RUNNING
...
RPM: 68.56 | REF: 500.00 | OUT: 81.01 | DUTY: 81 | MODE: RUNNING
RPM: 17.97 | REF: 500.00 | OUT: 93.39 | DUTY: 93 | MODE: RUNNING
Modo control: RUNNING -> FAULT
```

Conclusion del log: el lazo se cerraba demasiado temprano, apenas aparecia una lectura de 92 RPM. Despues, cuando el motor caia, el PID mandaba `DUTY` entre 80 y 93, que ya se comprobo fisicamente que no arranca este motor. Por eso se agrega una compensacion de zona muerta: si el motor esta por debajo de la referencia y el PID pide algo menor que el duty minimo efectivo, el comando sube a `PWM_MIN_EFFECTIVE_DUTY`.

Revision posterior:

```text
RPM: 828.55 | REF: 500.00 | PIDOUT: 150.00 | CMD: 150.00 | DUTY: 36 | MODE: STARTING
RPM: 1071.16 | REF: 500.00 | PIDOUT: 150.00 | CMD: 150.00 | DUTY: 54 | MODE: STARTING
RPM: 1944.98 | REF: 500.00 | PIDOUT: 0.00 | CMD: 0.00 | DUTY: 36 | MODE: RUNNING
...
RPM: 445.93 | REF: 500.00 | PIDOUT: 9.79 | CMD: 110.00 | DUTY: 18 | MODE: RUNNING
```

Esto demuestra otro problema: la medicion por conteo en ventanas de 100 ms no sirve bien con `PULSES_PER_REV = 1`. Un solo pulso en 100 ms equivale aproximadamente a 600 RPM; dos pulsos equivalen a 1200 RPM; tres pulsos equivalen a 1800 RPM. Por eso el firmware creia ver velocidad maxima, cortaba PWM y dejaba al motor sin energia.

Cambio aplicado en `rpm.c`: la RPM ya no se calcula contando pulsos por ventana fija. Ahora se mide el periodo entre dos pulsos consecutivos:

```text
RPM = 60000000 / (periodo_us * PULSES_PER_REV)
```

Este metodo es mas adecuado para sensores de baja resolucion, como una sola marca blanca/negra por vuelta.

Revision adicional:

```text
RPM: 4221.27 | REF: 500.00 | PIDOUT: 0.00 | CMD: 0.00 | DUTY: 0 | MODE: RUNNING
RPM: 528.50  | REF: 500.00 | PIDOUT: 0.00 | CMD: 0.00 | DUTY: 0 | MODE: STARTING
RPM: 0.00    | REF: 500.00 | PIDOUT: 150.00 | CMD: 150.00 | DUTY: 150 | MODE: STARTING
```

Ese comportamiento indica dos fallas practicas:

- El sensor todavia puede entregar pulsos demasiado juntos por ruido, rebote o mala lectura de contraste. Esos pulsos implican RPM fisicamente incoherentes para esta prueba.
- El motor tiene zona muerta: si cae a 0, no siempre vuelve a arrancar con un duty medio. Necesita un golpe corto de arranque y luego un duty minimo efectivo.

Cambios aplicados:

```c
#define RPM_SENSOR_MAX_VALID_RPM 6000.0f
#define PWM_KICKSTART_DUTY 220.0f
#define PWM_KICKSTART_MS 250
#define PWM_MIN_EFFECTIVE_DUTY 150.0f
#define RPM_CONTROL_DEADBAND 100.0f
```

El sensor ahora ignora pulsos que implicarian mas de `RPM_SENSOR_MAX_VALID_RPM`. Para probar 6000 RPM con `PULSES_PER_REV = 1`, el periodo esperado entre pulsos es de 10 ms. El control de arranque aplica maxima potencia para vencer friccion estatica, y cuando la RPM esta por debajo de la referencia no permite que el comando caiga por debajo del duty minimo efectivo.

Revision con referencia de 500 RPM:

```text
RPM: 280-480 | REF: 500 | CMD: 160-175
```

Conclusion: el duty minimo anterior movia el motor, pero no alcanzaba para sostener 500 RPM. Se ajusto el control para que, si la RPM cae claramente por debajo de la referencia, use un duty de recuperacion:

```c
#define PWM_RUN_FEEDFORWARD_DUTY 190.0f
#define PWM_MIN_EFFECTIVE_DUTY 190.0f
#define PWM_RECOVERY_DUTY 210.0f
#define RPM_CONTROL_DEADBAND 60.0f
```

La OLED se cambio a I2C 400 kHz y refresco cada 100 ms para que la visualizacion sea mas rapida.

Revision actual:

El comportamiento corregido queda separado en dos modos:

1. `STARTING`: si la referencia es mayor que 0, el firmware intenta arrancar indefinidamente a `PWM_MAX_DUTY` / `RPM_STARTUP_DUTY = 255`. No existe apagado final por intentos agotados.
2. `RUNNING`: cuando ya hay pulsos recientes y la RPM supera el umbral de handoff, entra al control de velocidad. El PID ya no representa el duty completo; ahora es una correccion sobre `PWM_RUN_FEEDFORWARD_DUTY`.

Fundamento:

En un lazo cerrado, la medicion de RPM es la realimentacion. Si la realimentacion desaparece pero la referencia sigue activa, este montaje debe intentar recuperar movimiento, no quedar apagado. A la vez, el arranque desde reposo se trata como una fase especial porque al principio no hay pulsos aunque el sistema este funcionando correctamente.

En control real de motores se usa una fase de arranque en lazo abierto para llevar el motor a una velocidad minima antes de cerrar el lazo de velocidad. Tambien se considera la zona muerta del motor y del puente H. Un motor DC puede no moverse con PWM bajo porque la corriente promedio no vence friccion estatica, caidas internas del driver y carga mecanica.

Por eso el firmware ahora usa feedforward: `CMD = duty_base_por_RPM + PIDOUT`. La referencia `500` es RPM, no PWM; el PWM de este proyecto es de 8 bits y por tanto va de 0 a 255. El duty base es la aproximacion de potencia necesaria para sostener la velocidad, y el PID solo corrige arriba o abajo.

## Potencia entregada segun RPM

Antes el duty base de marcha era fijo: `PWM_RUN_FEEDFORWARD_DUTY = 190` para cualquier referencia. Eso permitia regular cerca de 500 RPM, pero no era coherente si se pedia 1000, 3000 o 6000 RPM, porque el sistema arrancaba el control desde la misma potencia base.

Cambio aplicado:

```text
500 RPM  -> duty base 190
6000 RPM -> duty base 255
```

Entre esos puntos el firmware interpola linealmente. Ejemplos aproximados:

```text
REF 500  -> base 190
REF 1000 -> base 196
REF 3000 -> base 220
REF 6000 -> base 255
```

Luego el PID corrige sobre esa base:

```text
CMD = duty_base_por_RPM + PIDOUT
```

Si la RPM medida esta por debajo de la referencia, `PIDOUT` tiende a subir `CMD`. Si la RPM medida esta por encima de la referencia, `PIDOUT` tiende a bajar `CMD`. Asi la potencia ya no depende solo del error instantaneo: tambien parte de una base proporcional a las RPM pedidas.

En el monitor serial debe aparecer al iniciar:

```text
Feedforward: base 500RPM=190 escala hasta 6000RPM=255 min=190 recovery=210 deadband=60 RPM
```

Para comprobarlo, cambiar la referencia por UART:

```text
rpm 500
rpm 1000
rpm 3000
rpm 6000
```

Con el motor ya en `MODE: RUNNING`, el `CMD` deberia quedar alrededor de la base esperada y moverse arriba o abajo segun la diferencia entre `RPM` y `REF`.

## Precision de lectura RPM

Problema observado:

```text
RPM cerca de 500 -> salto a 900/1200/1400 -> PID baja mucho el CMD -> motor cae -> control vuelve a recuperar
```

Con `PULSES_PER_REV = 1`, el sensor entrega una sola medida por vuelta. Si entra un pulso falso, un rebote o una lectura anticipada por vibracion, el calculo por periodo puede interpretar que el motor acelero mucho aunque no sea cierto.

Cambio aplicado:

```c
#define RPM_FILTER_ALPHA 0.12f
#define RPM_PERIOD_FILTER_SAMPLES 5
```

El firmware ahora guarda los ultimos 5 periodos entre pulsos y usa la mediana. La mediana no promedia directamente todos los valores: escoge el valor central, por eso resiste mejor un pulso aislado demasiado corto o demasiado largo. Despues se aplica un filtro exponencial mas suave antes de entregar la RPM al PID.

Resultado esperado:

- Menos saltos bruscos de RPM en el monitor.
- Menos bajadas agresivas de `CMD` por lecturas falsas altas.
- Respuesta un poco mas lenta, pero mas estable.

Si aun se ven saltos grandes, antes de subir o bajar PID conviene mejorar la parte fisica:

- Mantener distancia constante entre TCRT500 y marca.
- Usar una marca con alto contraste y borde limpio.
- Evitar brillo externo sobre el sensor.
- Separar cables del motor de los cables del sensor.
- Agregar mas marcas por vuelta y actualizar `PULSES_PER_REV`, porque eso aumenta la resolucion real.

Decision de arranque: en este montaje el motor necesita tomar inercia antes de que el sensor entregue pulsos confiables. Por eso, mientras `REF > 0` y el modo sea `STARTING`, el duty se mantiene en 255 hasta que el sensor confirme suficiente velocidad para cerrar el lazo.

Mensajes esperados en monitor:

```text
Modo control: STARTING -> RUNNING
Modo control: RUNNING -> STARTING
RPM: ... | REF: ... | PIDOUT: ... | CMD: ... | DUTY: ... | MODE: ...
```

## Registro de ajustes

### Iteracion 1

Valores:

```c
#define PID_KP 0.18f
#define PID_KI 0.04f
#define PID_KD 0.00f
#define PWM_SLEW_STEP 18.0f
```

Resultado observado con referencia de 500 RPM:

```text
Promedio estable aproximado: 568 RPM
Minimo estable aproximado: 418 RPM
Maximo estable aproximado: 767 RPM
Duty estable promedio aproximado: 225
```

Conclusion:

El control ya no queda pegado en 255 todo el tiempo, pero la RPM queda por encima de la referencia. El duty permanece alto aun con RPM mayor a 500, lo que indica que el termino integral esta sosteniendo demasiada salida.

Cambio para iteracion 2:

```c
#define PID_KI 0.01f
```

Se mantiene `PID_KP`, `PID_KD` y `PWM_SLEW_STEP` para aislar el efecto de bajar el integral.

## Notas de fundamento

La estructura del PID usa dos practicas comunes:

- Limitar la salida al rango real del actuador PWM, `0..255`.
- Limitar el acumulador integral para reducir windup cuando el actuador se satura.

Estas ideas estan alineadas con implementaciones conocidas como Arduino PID Library de Brett Beauregard y con la practica comun de anti-windup en control PID.
