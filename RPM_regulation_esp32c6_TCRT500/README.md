# Control de RPM con ESP32-C6, TCRT5000, L293D y OLED

Este proyecto regula la velocidad de un motor DC usando un ESP32-C6, un sensor optico TCRT5000, un driver L293D, una pantalla OLED SSD1306 y una interfaz web. El objetivo es medir las revoluciones reales del motor, compararlas contra una referencia de RPM y ajustar el PWM enviado al driver para mantener la velocidad lo mas estable posible.

La version actual es la version final probada del control. El ajuste mas importante es que el motor arranca con maxima potencia y luego entra al control cerrado cuando el sensor ya detecta movimiento. Ademas, la ayuda de duty minimo entra como rampa progresiva para evitar saltos bruscos cuando el RPM cae un poco por debajo de la referencia.

## Hardware usado

- ESP32-C6.
- Sensor optico TCRT5000 con salida digital `OUT`.
- Driver de motor L293D.
- Motor DC.
- Pantalla OLED SSD1306 I2C, direccion `0x3C`.
- Fuente externa para el motor.
- Tierra comun entre ESP32-C6, L293D, sensor, OLED y fuente del motor.

## Pinout del ESP32-C6

| Funcion | GPIO ESP32-C6 | Conexion |
| --- | ---: | --- |
| PWM motor | GPIO4 | L293D pin 1, `EN1` |
| Direccion IN1 | GPIO5 | L293D pin 2, `IN1` |
| Direccion IN2 | GPIO8 | L293D pin 7, `IN2` |
| Sensor RPM | GPIO1 | TCRT5000 `OUT` |
| OLED SDA | GPIO6 | OLED `SDA` |
| OLED SCL | GPIO7 | OLED `SCL` |

Estos pines estan definidos en `main/config.h`.

## Conexion del L293D

| Pin L293D | Nombre | Conexion |
| ---: | --- | --- |
| 1 | EN1 | ESP32-C6 GPIO4, PWM |
| 2 | IN1 | ESP32-C6 GPIO5 |
| 3 | OUT1 | Terminal 1 del motor |
| 4 | GND | Tierra comun |
| 5 | GND | Tierra comun |
| 6 | OUT2 | Terminal 2 del motor |
| 7 | IN2 | ESP32-C6 GPIO8 |
| 8 | VCC2 | Positivo de la fuente externa del motor |
| 12 | GND | Tierra comun |
| 13 | GND | Tierra comun |
| 16 | VCC1 | 5 V de logica del L293D |

Importante: el negativo de la fuente externa del motor debe estar unido con el GND del ESP32-C6. Si no hay tierra comun, el driver puede no responder bien aunque el codigo este correcto.

## Conexion del sensor TCRT5000

| Pin sensor | Conexion |
| --- | --- |
| VCC | 3.3 V del ESP32-C6 |
| GND | Tierra comun |
| OUT | ESP32-C6 GPIO1 |

El codigo usa interrupcion por flanco de bajada en GPIO1. El sensor debe cambiar de estado cuando pasa de zona blanca a negra o de negra a blanca, segun el ajuste del potenciometro del modulo.

Para probarlo fisicamente, acerca una marca negra/blanca al sensor y verifica que el LED del modulo cambie. Si el LED no cambia, ajusta el potenciometro hasta que cambie justo cuando pasa la marca.

## Conexion de la OLED SSD1306

| Pin OLED | Conexion |
| --- | --- |
| VCC | 3.3 V del ESP32-C6 |
| GND | Tierra comun |
| SDA | ESP32-C6 GPIO6 |
| SCL | ESP32-C6 GPIO7 |

La pantalla se actualiza en una tarea separada para que un error I2C no bloquee directamente el control del motor.

## Funcionamiento general

1. El ESP32-C6 inicializa el PWM, la direccion del motor, el sensor RPM, la OLED, comandos por UART, WiFi y servidor HTTP.
2. El motor recibe PWM mediante el pin `EN1` del L293D.
3. El sensor TCRT5000 detecta pulsos cuando gira la marca del eje o disco.
4. El codigo mide el tiempo entre pulsos y calcula RPM.
5. La medicion se filtra para reducir picos falsos.
6. El control compara `RPM real` contra `RPM referencia`.
7. El PID calcula una correccion.
8. La correccion se suma a una base de PWM calculada por calibracion.
9. El duty final se limita, se suaviza y se envia al L293D.
10. La OLED y la pagina web muestran RPM, referencia, duty, comando y estado del control.

## Como se implementa el PWM

El PWM se configura en `main/motor.c` usando el periferico LEDC del ESP32-C6:

- Frecuencia: `20000 Hz`.
- Resolucion: 8 bits.
- Rango de duty: `0` a `255`.

Interpretacion del duty:

| Duty | Significado aproximado |
| ---: | --- |
| 0 | Motor apagado |
| 127 | 50% de PWM |
| 190 | Zona efectiva medida para mantener cerca de 500 RPM |
| 255 | Maxima potencia |

El PWM no es voltaje analogico puro. Es una senal digital que prende y apaga muy rapido. El motor recibe una potencia promedio proporcional al duty.

## Como se implementa el PID

El PID esta dividido en dos partes:

- `main/pid.c`: calcula la correccion PID pura.
- `main/control.c`: decide como usar esa correccion en el motor real.

La ecuacion basica es:

```text
error = RPM_referencia - RPM_medida
PIDOUT = Kp * error + integral - Kd * cambio_medicion
CMD = duty_base + PIDOUT
DUTY = CMD suavizado por slew rate
```

Parametros principales en `main/config.h`:

```c
#define PID_KP 0.12f
#define PID_KI 0.01f
#define PID_KD 0.00f
```

- `Kp`: reacciona al error actual.
- `Ki`: corrige errores pequenos acumulados en el tiempo.
- `Kd`: esta en cero porque con este sensor puede amplificar ruido y picos.

## Feedforward y duty base

El sistema no depende solo del PID. Tambien usa una base de duty calculada por calibracion.

Punto de calibracion probado:

```c
#define PWM_FEEDFORWARD_REF_RPM 500.0f
#define PWM_RUN_FEEDFORWARD_DUTY 190.0f
```

Eso significa que se observo que alrededor de 500 RPM el motor necesita una base cercana a 190 de duty. Para otras referencias, el codigo calcula una base proporcional:

- De 0 a 500 RPM, escala desde 0 hasta 190.
- De 500 a 6000 RPM, escala desde 190 hasta 255.

Luego el PID suma o resta sobre esa base.

## Arranque del motor

El motor no siempre arranca con un duty bajo porque debe vencer friccion estatica. Por eso el control tiene modo `STARTING`:

```c
#define RPM_STARTUP_DUTY 255.0f
#define RPM_STARTUP_MIN_MS 500
```

Cuando la referencia es mayor que cero, el motor arranca a maxima potencia. El control espera a que el sensor detecte movimiento y que el RPM llegue a una fraccion de la referencia antes de pasar a `RUNNING`.

La transferencia a control cerrado se calcula desde una medida real:

```c
#define RPM_STARTUP_HANDOFF_CAL_RPM 500.0f
#define RPM_STARTUP_HANDOFF_CAL_DETECTED_RPM 425.0f
```

La relacion es:

```text
425 / 500 = 0.85
```

Entonces el handoff se calcula asi:

```text
RPM_handoff = max(80 RPM, referencia * 0.85)
```

Ejemplos:

| Referencia | Handoff aproximado |
| ---: | ---: |
| 100 RPM | 85 RPM |
| 500 RPM | 425 RPM |
| 1000 RPM | 850 RPM |
| 3000 RPM | 2550 RPM |

Esto evita el problema de exigir siempre 350 RPM incluso cuando la referencia era baja.

## Proteccion contra saltos bruscos

En pruebas se vio que, cuando el RPM caia apenas por debajo de la referencia, el duty podia saltar demasiado fuerte. Para corregirlo, la ayuda de duty minimo ahora entra como rampa.

Parametros:

```c
#define RPM_CONTROL_DEADBAND 90.0f
#define RPM_MIN_EFFECTIVE_BAND_RATIO 0.02f
#define RPM_MIN_EFFECTIVE_BAND_MIN 10.0f
```

La banda minima se calcula asi:

```text
banda = max(10 RPM, 2% de la referencia)
```

Ejemplos:

| Referencia | Banda minima |
| ---: | ---: |
| 500 RPM | 10 RPM |
| 1000 RPM | 20 RPM |
| 3000 RPM | 60 RPM |
| 6000 RPM | 120 RPM |

Si el error es pequeno, el control corrige suave. Si el error crece, la ayuda aumenta progresivamente hasta llegar a recuperacion fuerte. Esto fue lo que elimino los golpes de duty cuando el motor estaba cerca de la referencia.

## Filtro de RPM

La medicion de RPM esta en `main/rpm.c`.

El sensor genera pulsos. El codigo mide el periodo entre pulsos y calcula:

```text
RPM = 60000000 / intervalo_us
```

Tambien se aplican protecciones:

- Antirrebote por tiempo minimo entre pulsos.
- Rechazo de intervalos imposibles para evitar picos falsos.
- Mediana de varios periodos para reducir ruido.
- Filtro diferente cuando el RPM sube o baja.

Parametros:

```c
#define RPM_FILTER_ALPHA_RISE 0.12f
#define RPM_FILTER_ALPHA_FALL 0.50f
#define RPM_FILTER_ALPHA_FALL_NEAR 0.45f
#define RPM_FILTER_NEAR_SETPOINT_BAND 100.0f
#define RPM_PERIOD_FILTER_SAMPLES 5
```

El filtro de subida es mas suave porque el sensor puede producir picos altos falsos. La bajada es mas rapida para que el sistema no se demore demasiado mostrando una velocidad que ya cayo.

## Suavizado del duty

El duty aplicado no salta directamente al comando calculado. Se suaviza con:

```c
#define PWM_SLEW_STEP_UP 10.0f
#define PWM_SLEW_STEP_DOWN 35.0f
```

Eso significa:

- El duty puede subir maximo 10 unidades por ciclo de control.
- El duty puede bajar maximo 35 unidades por ciclo de control.

Como el ciclo es de 100 ms, esto evita golpes bruscos al subir, pero permite bajar mas rapido cuando el motor se pasa de RPM.

## Interfaz por UART

Se puede cambiar la referencia desde el monitor serial a 115200 baudios.

Comandos validos:

```text
rpm 500
set 1000
ref=3000
rpm 0
```

`rpm 0` pausa el motor.

## Interfaz web

Si el WiFi esta configurado, el ESP32-C6 levanta un servidor HTTP. La pagina permite:

- Ver RPM actual.
- Ver referencia.
- Ver duty.
- Ver salida PID.
- Ver modo `STARTING` o `RUNNING`.
- Aplicar una nueva referencia de RPM.
- Iniciar y pausar.
- Usar botones rapidos de 0, 500, 1000, 3000 y 6000 RPM.

Endpoints principales:

| Ruta | Metodo | Funcion |
| --- | --- | --- |
| `/` | GET | Pagina web |
| `/api/status` | GET | Estado en JSON |
| `/api/setpoint` | GET | Referencia actual |
| `/api/setpoint` | POST | Cambiar referencia |
| `/health` | GET | Verificar servidor |

El navegador consulta `/api/status` cada 250 ms.

## Configuracion WiFi

En ESP-IDF:

```powershell
idf.py menuconfig
```

Luego buscar la configuracion del proyecto y definir:

- SSID WiFi.
- Password WiFi.
- Numero maximo de reintentos.

Si no se configura SSID, el proyecto funciona igual por UART y OLED, pero no inicia la pagina web.

## Organizacion del codigo

| Archivo | Funcion |
| --- | --- |
| `main/main.c` | Punto de entrada. Inicializa motor, sensor, OLED, UART, WiFi, HTTP y tareas. |
| `main/config.h` | Pines, parametros de PWM, PID, filtros, limites y calibracion. |
| `main/motor.c` / `main/motor.h` | Configura LEDC PWM y direccion del L293D. |
| `main/rpm.c` / `main/rpm.h` | Lee el sensor, calcula RPM y filtra la medicion. |
| `main/pid.c` / `main/pid.h` | Implementa el controlador PID basico. |
| `main/control.c` / `main/control.h` | Logica principal de control, arranque, feedforward, proteccion y duty final. |
| `main/oled.c` / `main/oled.h` | Controla la pantalla OLED SSD1306 por I2C. |
| `main/uart_cmd.c` / `main/uart_cmd.h` | Permite cambiar RPM desde el monitor serial. |
| `main/wifi_connect.c` / `main/wifi_connect.h` | Conecta el ESP32-C6 a una red WiFi. |
| `main/http_server.c` / `main/http_server.h` | Sirve la pagina web y la API HTTP. |
| `main/CMakeLists.txt` | Lista de fuentes que compila ESP-IDF. |
| `main/Kconfig.projbuild` | Opciones configurables desde `menuconfig`. |

## Como compilar y flashear

Desde la carpeta del proyecto:

```powershell
cd C:\Universidad\RPM_regulation_esp32c6_TCRT500
. C:\esp\v6.0.1\esp-idf\export.ps1
idf.py set-target esp32c6
idf.py build
idf.py -p COM13 flash monitor
```

Si aparece error de modo de arranque al flashear, desconecta temporalmente las senales conectadas a GPIO4, GPIO5 o GPIO8 del driver, o usa los botones `BOOT` y `RESET` de la placa para entrar en modo descarga.

## Notas importantes de uso

- Siempre conectar todas las tierras en comun.
- El motor debe alimentarse desde fuente externa, no desde el pin de 3.3 V del ESP32-C6.
- El TCRT5000 y la OLED se alimentan a 3.3 V.
- Ajustar el potenciometro del TCRT5000 hasta que el cambio negro/blanco sea claro.
- Para pausar el motor, enviar referencia `0`.
- Para arrancar, enviar una referencia mayor que `0`.
- No cambiar los parametros PID sin hacer pruebas con logs.

## Parametros finales de control

```c
#define SAMPLE_TIME_MS 100
#define MAX_RPM_SETPOINT 6000.0f
#define PID_KP 0.12f
#define PID_KI 0.01f
#define PID_KD 0.00f
#define PWM_SLEW_STEP_UP 10.0f
#define PWM_SLEW_STEP_DOWN 35.0f
#define PWM_FEEDFORWARD_REF_RPM 500.0f
#define PWM_RUN_FEEDFORWARD_DUTY 190.0f
#define RPM_FILTER_ALPHA_RISE 0.12f
#define RPM_FILTER_ALPHA_FALL 0.50f
#define RPM_FILTER_ALPHA_FALL_NEAR 0.45f
```

Estos valores corresponden a la version final probada. La estabilidad depende tambien de la distancia entre el sensor y la marca del motor, la calidad de la fuente, la tierra comun y la alineacion mecanica.
