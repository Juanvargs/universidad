# Pruebas y calibracion

Este documento propone un procedimiento para validar el montaje y ajustar el sistema sin cambiar varios parametros a la vez.

## Prueba electrica previa

Antes de flashear:

1. Verificar tierra comun.
2. Verificar que el TCRT5000 este a 3.3 V.
3. Verificar que la OLED este a 3.3 V.
4. Verificar que el L293D tenga logica en 5 V y motor en fuente externa.
5. Verificar que `IN2` este en GPIO10.

## Prueba del sensor

Antes de usar PID:

1. Encender el ESP32-C6.
2. Acercar la marca blanca/negra al TCRT5000.
3. Ajustar el potenciometro hasta que el LED del modulo cambie.
4. Revisar que la lectura de RPM sea 0 cuando el motor no pasa por el sensor.

Si el RPM cambia sin que el sensor detecte la marca, no se debe ajustar PID todavia. Primero se revisa ruido, tierra comun, cableado o distancia.

## Prueba de arranque

Enviar por UART:

```text
rpm 500
```

Se espera ver:

```text
MODE: STARTING
```

Luego, cuando el sensor detecta suficiente velocidad:

```text
MODE: RUNNING
```

El arranque usa duty maximo. Esto es intencional para vencer friccion estatica.

## Prueba de regulacion

Registrar 20 a 40 lineas del monitor:

```text
RPM: ... | REF: ... | PIDOUT: ... | CMD: ... | DUTY: ... | MODE: ...
```

Datos que se deben anotar:

- Referencia usada.
- Voltaje de la fuente del motor.
- Distancia entre sensor y marca.
- Si el motor tiene carga o no.
- Si la marca es cinta, disco o pintura.

## Secuencia recomendada

```text
rpm 0
rpm 500
rpm 1000
rpm 3000
rpm 6000
rpm 0
```

Cada referencia debe probarse despues de que la anterior este estable.

## Interpretacion de fallas

### RPM sube sin motor frente al sensor

Causa probable:

- Entrada del sensor con ruido.
- Falta de tierra comun.
- Cable del sensor cerca del cable del motor.
- Potenciometro mal ajustado.

Accion:

- Corregir hardware antes de tocar PID.

### Motor no arranca

Causa probable:

- Fuente externa apagada.
- Tierra comun ausente.
- L293D mal cableado.
- Pin `EN1` no conectado a GPIO4.
- `IN2` conectado a un GPIO diferente al definido.

Accion:

- Revisar conexiones con `docs/01-hardware-conexiones.md`.

### RPM oscila alrededor de la referencia

Causa probable:

- Sensor sensible o inestable.
- `Kp` alto.
- `Ki` acumulando demasiado.
- Duty base no adecuado para el montaje.

Accion:

1. Revisar primero la lectura del sensor.
2. Si la lectura es real, bajar `PID_KP` en pasos pequenos.
3. Si el error se acumula y tarda en corregir, revisar `PID_KI`.

### Duty cambia con saltos fuertes

Causa probable:

- El control esta entrando a recuperacion por baja velocidad.
- La lectura de RPM cae de golpe.
- El error de RPM es grande.

Accion:

- Verificar si el salto de RPM es real o ruido del sensor.
- Revisar la rampa explicada en `docs/03-control-pid-pwm.md`.

## Ajuste de parametros

Los cambios deben hacerse de uno en uno.

Orden recomendado:

1. Sensor y cableado.
2. `PWM_RUN_FEEDFORWARD_DUTY`.
3. `PWM_SLEW_STEP_UP` y `PWM_SLEW_STEP_DOWN`.
4. `PID_KP`.
5. `PID_KI`.
6. `PID_KD` solo si la medicion es muy limpia.

## Evidencia para el informe

Para presentar el proyecto se recomienda incluir:

- Foto del circuito.
- Tabla de conexiones.
- Captura del monitor serial.
- Captura de la pagina web.
- Video corto del arranque y regulacion.
- Explicacion del modo `STARTING`.
- Explicacion del modo `RUNNING`.
- Tabla con referencia, RPM promedio y duty aproximado.

Tabla sugerida:

| Referencia | RPM promedio | Duty promedio | Observacion |
| ---: | ---: | ---: | --- |
| 500 | | | |
| 1000 | | | |
| 3000 | | | |
| 6000 | | | |

## Comando de compilacion

```powershell
cd C:\Universidad\RPM_regulation_esp32c6_TCRT500
. C:\esp\v6.0.1\esp-idf\export.ps1
idf.py build
```

Si la compilacion genera `build/RPM.bin`, el firmware fue construido correctamente.
