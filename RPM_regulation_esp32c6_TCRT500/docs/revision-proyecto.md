# Memoria tecnica del proyecto

Este documento describe la estructura tecnica del firmware, el funcionamiento de cada modulo y las decisiones principales tomadas para estabilizar el control de RPM.

## Objetivo del sistema

Se implemento un sistema de control de velocidad para un motor DC usando un ESP32-C6. La velocidad se mide con un sensor optico TCRT5000, el motor se acciona mediante un L293D y la referencia de RPM puede configurarse por UART o por una pagina web local.

El sistema busca mantener la velocidad medida cerca de la referencia indicada en RPM, dentro del rango de 0 a 6000 RPM.

## Flujo funcional

1. `main.c` inicializa los modulos principales.
2. `motor.c` configura el PWM y los pines de direccion del L293D.
3. `rpm.c` mide el periodo entre pulsos del TCRT5000.
4. `control.c` ejecuta la logica de arranque, feedforward, PID y aplicacion del duty.
5. `oled.c` muestra RPM, referencia y duty en pantalla.
6. `uart_cmd.c` permite modificar la referencia desde el monitor serial.
7. `wifi_connect.c` conecta el ESP32-C6 a la red WiFi.
8. `http_server.c` publica la interfaz web y la API de control.

## Arquitectura por modulos

| Modulo | Responsabilidad |
| --- | --- |
| `main.c` | Secuencia de inicializacion y creacion de tareas. |
| `config.h` | Pines, limites, calibracion y parametros de control. |
| `motor.c` | PWM LEDC y direccion del motor. |
| `rpm.c` | Interrupcion del sensor, calculo y filtro de RPM. |
| `pid.c` | Calculo PID basico. |
| `control.c` | Control principal de velocidad. |
| `oled.c` | Visualizacion local en SSD1306. |
| `uart_cmd.c` | Comandos por puerto serial. |
| `wifi_connect.c` | Conexion WiFi en modo station. |
| `http_server.c` | Pagina web y endpoints HTTP. |

La separacion permite que el control del motor no dependa directamente de la web ni de la pantalla. La pantalla se actualiza en una tarea separada, y la web consulta un estado ya calculado por `control_get_status()`.

## Medicion de RPM

Se usa una interrupcion sobre el GPIO del sensor. Cada flanco valido permite medir el tiempo entre dos pulsos consecutivos.

La formula aplicada es:

```text
RPM = 60000000 / (periodo_us * PULSES_PER_REV)
```

Para este montaje se usa:

```c
#define PULSES_PER_REV 1
```

El uso de periodo entre pulsos se eligio porque una sola marca por vuelta genera pocos pulsos. Medir por conteo en una ventana fija de 100 ms producia resolucion baja: un pulso podia representar aproximadamente 600 RPM, dos pulsos 1200 RPM, etc. Por eso se implemento medicion por periodo.

## Rechazo de ruido del sensor

Se aplicaron varias protecciones:

- Antirrebote temporal.
- Rechazo de intervalos que impliquen mas de 6000 RPM.
- Comparacion contra el intervalo anterior para rechazar pulsos demasiado cercanos.
- Mediana de 5 periodos.
- Filtro exponencial con respuesta diferente para subida y bajada.

Estas medidas reducen picos falsos causados por vibracion, cambios de luz, mala alineacion o ruido electrico del motor.

## Control del motor

El control se divide en dos modos:

### Modo STARTING

Cuando la referencia es mayor que cero y el motor aun no tiene una lectura confiable, se aplica:

```text
DUTY = 255
```

Este arranque en lazo abierto permite vencer la friccion estatica del motor. El sistema permanece en este modo hasta detectar movimiento suficiente.

### Modo RUNNING

Cuando el sensor confirma movimiento, el sistema cierra el lazo de control. En este modo se calcula:

```text
CMD = duty_base_por_RPM + PIDOUT
```

Luego `CMD` se limita, se suaviza y se aplica como `DUTY`.

## Transferencia de arranque a control cerrado

Se midio que, para una referencia de 500 RPM, el sistema podia entrar a control alrededor de 425 RPM.

De esa relacion:

```text
425 / 500 = 0.85
```

se obtuvo la regla:

```text
RPM_handoff = max(80 RPM, referencia * 0.85)
```

Esto permite que el umbral de entrada a control cerrado sea proporcional a la referencia. Asi se evita usar un umbral fijo que pueda ser demasiado alto para referencias bajas.

## PID y feedforward

El PID no genera directamente todo el duty. Primero se calcula una base de PWM aproximada segun la referencia:

```text
500 RPM -> duty base 190
6000 RPM -> duty base 255
```

Sobre esa base se suma la correccion PID:

```text
PIDOUT = Kp * error + integral - Kd * cambio_medicion
CMD = duty_base + PIDOUT
```

Parametros finales:

```c
#define PID_KP 0.12f
#define PID_KI 0.01f
#define PID_KD 0.00f
```

Se mantuvo `Kd` en cero para evitar amplificacion del ruido del sensor optico.

## Reduccion de saltos de duty

Durante las pruebas se identifico que, cuando la velocidad medida quedaba apenas por debajo de la referencia, una proteccion de duty minimo podia entrar como escalon. Esto generaba cambios bruscos de potencia.

Se reemplazo el escalon por una rampa progresiva. La banda minima se calcula asi:

```text
banda = max(10 RPM, 2% de la referencia)
```

Si el error es pequeno, la ayuda es pequena. Si el error aumenta, la ayuda sube hasta la zona de recuperacion. Esta logica se aplica para cualquier referencia, no solo para 500 RPM.

## OLED

La pantalla SSD1306 se maneja por I2C a 400 kHz. La actualizacion se realiza desde `oled_task`, separada de `control_task`.

Esta decision evita que una falla o demora de I2C afecte directamente el periodo del control del motor.

## Interfaz HTTP

La pagina web esta embebida en `http_server.c`. El navegador consulta el estado del sistema cada 250 ms.

Endpoints:

| Metodo | Ruta | Funcion |
| --- | --- | --- |
| `GET` | `/` | Pagina web |
| `GET` | `/health` | Estado basico del servidor |
| `GET` | `/api/status` | RPM, referencia, PIDOUT, CMD, duty y modo |
| `GET` | `/api/setpoint` | Referencia configurada |
| `POST` | `/api/setpoint` | Cambio de referencia |

El cambio de referencia se valida contra los limites definidos en `config.h`.

## Validacion

Se verifico la compilacion del proyecto con ESP-IDF 6.0.1 para el objetivo `esp32c6`. La imagen generada fue:

```text
build/RPM.bin
```

Durante la compilacion aparece una advertencia de particion casi llena. La advertencia no impide compilar ni flashear, pero indica que el firmware se encuentra cerca del limite de la particion de aplicacion.

## Consideraciones de presentacion

Para una demostracion estable se recomienda:

- Mantener tierra comun entre ESP32-C6, L293D, fuente del motor, sensor y OLED.
- Ajustar el potenciometro del TCRT5000 antes de ejecutar pruebas.
- Usar una marca de alto contraste y borde limpio.
- Separar cables del motor de los cables del sensor.
- Probar primero `rpm 500` y luego subir progresivamente.
- Registrar el monitor serial para evidenciar `RPM`, `REF`, `CMD`, `DUTY` y `MODE`.
