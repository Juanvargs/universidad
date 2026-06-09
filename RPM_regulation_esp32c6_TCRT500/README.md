# Control de RPM con ESP32-C6, TCRT5000, L293D y OLED

Firmware para regular la velocidad de un motor DC mediante control PID sobre un ESP32-C6. El sistema mide RPM con un sensor optico TCRT5000, acciona el motor por PWM usando un L293D, muestra datos en una OLED SSD1306 y permite modificar la referencia de RPM por UART o por una pagina web local.

## Que hace el proyecto

El sistema compara una referencia de velocidad con la velocidad real del motor y ajusta automaticamente la potencia aplicada al driver. La referencia puede ir de 0 a 6000 RPM:

- `0 RPM`: pausa el motor.
- `> 0 RPM`: arranca el motor, mide RPM y regula la velocidad.

La version actual usa:

- Arranque en lazo abierto con duty maximo para vencer friccion estatica.
- Medicion de RPM por periodo entre pulsos.
- Filtro por mediana y filtro exponencial.
- PID como correccion sobre un duty base calculado por calibracion.
- Rampa de ayuda progresiva para evitar saltos bruscos de duty.

## Hardware requerido

| Elemento | Funcion |
| --- | --- |
| ESP32-C6 | Microcontrolador principal |
| TCRT5000 | Sensor optico para detectar pulsos de RPM |
| L293D | Driver puente H para el motor DC |
| Motor DC | Actuador controlado |
| OLED SSD1306 I2C | Visualizacion local de RPM y referencia |
| Fuente externa | Alimentacion del motor |
| Protoboard/cables | Conexion del montaje |

## Conexiones principales

| Funcion | ESP32-C6 | Conexion |
| --- | ---: | --- |
| PWM motor | GPIO4 | L293D pin 1, `EN1` |
| Direccion IN1 | GPIO5 | L293D pin 2, `IN1` |
| Direccion IN2 | GPIO10 | L293D pin 7, `IN2` |
| Sensor RPM | GPIO1 | TCRT5000 `OUT` |
| OLED SDA | GPIO6 | OLED `SDA` |
| OLED SCL | GPIO7 | OLED `SCL` |

Importante: todas las tierras deben estar en comun: ESP32-C6, L293D, fuente del motor, TCRT5000 y OLED.

## L293D

| Pin L293D | Nombre | Conexion |
| ---: | --- | --- |
| 1 | EN1 | ESP32-C6 GPIO4, PWM |
| 2 | IN1 | ESP32-C6 GPIO5 |
| 3 | OUT1 | Motor terminal 1 |
| 4 | GND | Tierra comun |
| 5 | GND | Tierra comun |
| 6 | OUT2 | Motor terminal 2 |
| 7 | IN2 | ESP32-C6 GPIO10 |
| 8 | VCC2 | Positivo de fuente externa del motor |
| 12 | GND | Tierra comun |
| 13 | GND | Tierra comun |
| 16 | VCC1 | 5 V logica del L293D |

El motor no debe alimentarse desde el pin de 3.3 V del ESP32-C6. Se usa una fuente externa para el motor y se comparte la tierra.

## Uso rapido

Desde la carpeta del proyecto:

```powershell
cd C:\Universidad\RPM_regulation_esp32c6_TCRT500
. C:\esp\v6.0.1\esp-idf\export.ps1
idf.py set-target esp32c6
idf.py build
idf.py -p COM13 flash monitor
```

Comandos por UART:

```text
rpm 500
set 1000
ref=3000
rpm 0
```

La pagina web queda disponible si se configura WiFi con:

```powershell
idf.py menuconfig
```

Luego se configuran `RPM_WIFI_SSID` y `RPM_WIFI_PASSWORD`.

## Interfaz web

La web embebida permite:

- Ver RPM medida.
- Ver referencia configurada.
- Ver `PIDOUT`, `CMD`, `DUTY` y modo de control.
- Aplicar una nueva referencia de RPM.
- Iniciar o pausar.
- Consultar una grafica historica local.

Endpoints:

| Metodo | Ruta | Funcion |
| --- | --- | --- |
| `GET` | `/` | Pagina web |
| `GET` | `/health` | Estado basico |
| `GET` | `/api/status` | RPM, referencia, PIDOUT, CMD, duty y modo |
| `GET` | `/api/setpoint` | Referencia actual |
| `POST` | `/api/setpoint` | Cambiar referencia |

## Organizacion del codigo

| Ruta | Funcion |
| --- | --- |
| `main/main.c` | Inicializacion del sistema y tareas |
| `main/config.h` | Pines, limites y parametros de control |
| `main/motor.c` | PWM LEDC y direccion del L293D |
| `main/rpm.c` | Lectura y filtro de RPM |
| `main/pid.c` | Calculo PID |
| `main/control.c` | Logica de arranque, PID, feedforward y duty final |
| `main/oled.c` | Interfaz OLED SSD1306 |
| `main/uart_cmd.c` | Comandos por monitor serial |
| `main/wifi_connect.c` | Conexion WiFi |
| `main/http_server.c` | Servidor web y API |

## Documentacion tecnica

La explicacion detallada esta separada por temas:

- [Resumen tecnico](docs/README.md)
- [Hardware y conexiones](docs/01-hardware-conexiones.md)
- [Medicion de RPM y sensor](docs/02-medicion-rpm-sensor.md)
- [Control PID, PWM y duty](docs/03-control-pid-pwm.md)
- [Interfaz web, UART y OLED](docs/04-interfaz-web-uart-oled.md)
- [Pruebas y calibracion](docs/05-pruebas-calibracion.md)

## Parametros principales

```c
#define MAX_RPM_SETPOINT 6000.0f
#define PID_KP 0.12f
#define PID_KI 0.01f
#define PID_KD 0.00f
#define PWM_RUN_FEEDFORWARD_DUTY 190.0f
#define PWM_SLEW_STEP_UP 10.0f
#define PWM_SLEW_STEP_DOWN 35.0f
```

Estos parametros corresponden a la configuracion validada del sistema. La estabilidad final tambien depende del cableado, la fuente, la alineacion del sensor y la calidad de la marca detectada por el TCRT5000.

## Notas de flasheo

Si aparece un error de modo de arranque al flashear, se recomienda desconectar temporalmente las senales del driver conectadas a GPIO4, GPIO5 y GPIO10, o usar los botones `BOOT` y `RESET` de la placa para entrar en modo descarga.
