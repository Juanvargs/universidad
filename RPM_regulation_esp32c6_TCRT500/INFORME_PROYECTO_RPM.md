# Informe del proyecto: Control de RPM con ESP32-C6

## 1. Descripcion general

Este proyecto consiste en el desarrollo de un sistema de control de velocidad para un motor DC mediante un microcontrolador ESP32-C6. La velocidad del motor se mide con un sensor optico TCRT5000, se regula mediante una senal PWM aplicada a un driver L293D y se controla mediante un algoritmo PID.

El sistema permite establecer una referencia de RPM, medir la velocidad real del motor, comparar la medicion contra la referencia y modificar automaticamente el duty del PWM para mantener la velocidad lo mas estable posible.

Adicionalmente, el proyecto incluye una pantalla OLED SSD1306 para visualizacion local, comandos por UART para configuracion desde el monitor serial y una interfaz web local para monitoreo y ajuste de la referencia.

Repositorio del proyecto:

```text
https://github.com/Juanvargs/universidad/tree/main/RPM_regulation_esp32c6_TCRT500
```

Video de demostracion:

```text
https://youtu.be/YIFlS1HBjsg?si=SRZd096OrHsiWsi5
```

## 2. Objetivo general

Implementar un sistema embebido capaz de regular la velocidad de un motor DC usando realimentacion de RPM, control PID y modulacion por ancho de pulso PWM.

## 3. Objetivos especificos

- Medir la velocidad real del motor usando un sensor optico TCRT5000.
- Calcular las RPM a partir del periodo entre pulsos del sensor.
- Aplicar filtros para reducir errores de lectura y picos falsos.
- Implementar un control PID para corregir la diferencia entre RPM medida y RPM deseada.
- Accionar el motor mediante un driver L293D usando PWM.
- Mostrar informacion del sistema en una pantalla OLED.
- Permitir cambio de referencia por UART.
- Implementar una pagina web local para monitoreo y control.
- Documentar conexiones, parametros, calculos y funcionamiento del firmware.

## 4. Componentes utilizados

| Componente | Funcion |
| --- | --- |
| ESP32-C6 | Microcontrolador principal del sistema |
| TCRT5000 | Sensor optico para detectar vueltas del motor |
| L293D | Driver puente H para controlar el motor |
| Motor DC | Actuador cuya velocidad se regula |
| OLED SSD1306 | Pantalla para mostrar RPM, referencia y duty |
| Fuente externa | Alimentacion del motor |
| Protoboard y cables | Montaje y conexion de componentes |

## 5. Conexion del circuito

### 5.1 Pines principales del ESP32-C6

| Funcion | Pin ESP32-C6 | Conexion |
| --- | ---: | --- |
| PWM motor | GPIO4 | L293D pin 1, EN1 |
| Direccion IN1 | GPIO5 | L293D pin 2, IN1 |
| Direccion IN2 | GPIO10 | L293D pin 7, IN2 |
| Sensor RPM | GPIO1 | TCRT5000 OUT |
| OLED SDA | GPIO6 | OLED SDA |
| OLED SCL | GPIO7 | OLED SCL |

Estos pines se definen en el archivo:

```text
main/config.h
```

Codigo correspondiente:

```c
#define PIN_PWM     4
#define PIN_IN1     5
#define PIN_IN2     10
#define PIN_SENSOR  1

#define OLED_I2C_SDA 6
#define OLED_I2C_SCL 7
```

### 5.2 Conexion del L293D

| Pin L293D | Nombre | Conexion |
| ---: | --- | --- |
| 1 | EN1 | ESP32-C6 GPIO4 |
| 2 | IN1 | ESP32-C6 GPIO5 |
| 3 | OUT1 | Terminal 1 del motor |
| 4 | GND | Tierra comun |
| 5 | GND | Tierra comun |
| 6 | OUT2 | Terminal 2 del motor |
| 7 | IN2 | ESP32-C6 GPIO10 |
| 8 | VCC2 | Positivo de la fuente del motor |
| 12 | GND | Tierra comun |
| 13 | GND | Tierra comun |
| 16 | VCC1 | 5 V de logica |

La tierra comun es indispensable. El GND del ESP32-C6, el GND de la fuente externa, el GND del L293D, el GND del sensor y el GND de la OLED deben estar conectados entre si.

## 6. Funcionamiento general del sistema

El sistema opera de la siguiente forma:

1. El ESP32-C6 inicializa motor, sensor, OLED, UART, WiFi y servidor HTTP.
2. El motor recibe una senal PWM por el pin EN1 del L293D.
3. El sensor TCRT5000 detecta una marca en el eje o disco del motor.
4. Cada pulso del sensor permite calcular el periodo de giro.
5. A partir del periodo se calcula la velocidad en RPM.
6. La RPM medida se filtra para reducir ruido.
7. El control compara la RPM medida contra la referencia.
8. El PID calcula una correccion.
9. La correccion se suma a un duty base calculado por calibracion.
10. El duty final se aplica al motor mediante PWM.
11. La informacion se muestra en OLED, UART y pagina web.

## 7. Medicion de RPM

La medicion se realiza en:

```text
main/rpm.c
```

El sensor TCRT5000 genera un pulso cuando detecta el paso de la marca. El codigo mide el tiempo entre pulsos consecutivos y calcula las RPM.

La formula utilizada es:

```text
RPM = 60,000,000 / (periodo_us * pulsos_por_vuelta)
```

Donde:

- `periodo_us`: tiempo entre pulsos en microsegundos.
- `pulsos_por_vuelta`: numero de pulsos generados por cada vuelta.
- `60,000,000`: cantidad de microsegundos en un minuto.

En el proyecto se usa:

```c
#define PULSES_PER_REV 1
```

Ejemplo para 500 RPM:

```text
periodo = 120,000 us
RPM = 60,000,000 / (120,000 * 1)
RPM = 500
```

Ejemplo para 1000 RPM:

```text
periodo = 60,000 us
RPM = 60,000,000 / 60,000
RPM = 1000
```

## 8. Filtro de RPM

La lectura del sensor puede presentar ruido por vibracion, mala alineacion, cambios de luz o interferencia del motor. Por esta razon se aplicaron varias protecciones:

- Rechazo de pulsos demasiado rapidos.
- Comparacion contra el periodo anterior.
- Mediana de los ultimos periodos.
- Filtro exponencial de subida y bajada.

Parametros principales:

```c
#define RPM_SENSOR_MAX_VALID_RPM 6000.0f
#define RPM_PERIOD_FILTER_SAMPLES 5
#define RPM_FILTER_ALPHA_RISE 0.12f
#define RPM_FILTER_ALPHA_FALL 0.50f
#define RPM_FILTER_ALPHA_FALL_NEAR 0.45f
```

La mediana ayuda a rechazar valores aislados incorrectos. El filtro exponencial suaviza la lectura antes de entregarla al PID.

Formula del filtro:

```text
RPM_filtrada = alpha * RPM_nueva + (1 - alpha) * RPM_anterior
```

Ejemplo:

```text
RPM_anterior = 500
RPM_nueva = 900
alpha = 0.12

RPM_filtrada = 0.12 * 900 + 0.88 * 500
RPM_filtrada = 548
```

Aunque la medicion instantanea suba a 900 RPM, el valor entregado al control sube de forma moderada.

## 9. Control PID

El PID se implementa en:

```text
main/pid.c
```

El control PID calcula una correccion a partir del error entre la referencia y la medicion:

```text
error = RPM_referencia - RPM_medida
```

La formula implementada es:

```text
PIDOUT = Kp * error + integral - Kd * cambio_medicion
```

En el codigo:

```c
float error = setpoint - measurement;
float d_input = measurement - pid->prev_measurement;

pid->integral += pid->Ki * error * dt;

float output = pid->Kp * error +
               pid->integral -
               pid->Kd * (d_input / dt);
```

Parametros finales:

```c
#define PID_KP 0.12f
#define PID_KI 0.01f
#define PID_KD 0.00f
```

### 9.1 Parte proporcional

La parte proporcional responde al error actual.

Ejemplo:

```text
Referencia = 500 RPM
Medicion = 450 RPM
Error = 500 - 450 = 50 RPM
Kp = 0.12

P = 0.12 * 50
P = 6
```

### 9.2 Parte integral

La parte integral corrige errores que se mantienen en el tiempo.

Ejemplo con `dt = 0.1 s`:

```text
Ki = 0.01
Error = 50
dt = 0.1

I = 0.01 * 50 * 0.1
I = 0.05
```

### 9.3 Parte derivativa

La parte derivativa se dejo en cero:

```c
#define PID_KD 0.00f
```

Esto se hizo porque el sensor optico puede presentar picos o ruido. Un termino derivativo podria amplificar esas variaciones y volver el control mas inestable.

## 10. PWM, duty y potencia del motor

El PWM se configura en:

```text
main/motor.c
```

Parametros:

```c
#define PWM_FREQ 20000
#define PWM_RES LEDC_TIMER_8_BIT
#define PWM_MAX_DUTY 255.0f
```

La resolucion de 8 bits permite valores de duty entre 0 y 255:

```text
2^8 = 256 niveles
Rango = 0 a 255
```

Equivalencia aproximada:

| Duty | Porcentaje PWM |
| ---: | ---: |
| 0 | 0% |
| 64 | 25% |
| 127 | 50% |
| 190 | 74.5% |
| 255 | 100% |

El duty no es voltaje analogico directo. Es una senal digital que prende y apaga rapidamente. El motor recibe una potencia promedio relacionada con el porcentaje de duty.

## 11. Feedforward o duty base

El PID no trabaja solo. Se usa una base de duty calculada por calibracion, y el PID suma o resta sobre esa base.

La ecuacion general es:

```text
CMD = duty_base + PIDOUT
```

Parametros:

```c
#define PWM_FEEDFORWARD_REF_RPM 500.0f
#define PWM_RUN_FEEDFORWARD_DUTY 190.0f
#define PWM_MAX_DUTY 255.0f
#define MAX_RPM_SETPOINT 6000.0f
```

Se usa como referencia:

```text
500 RPM -> duty 190
6000 RPM -> duty 255
```

Ejemplo para 1000 RPM:

```text
rpm_span = 6000 - 500 = 5500
ratio = (1000 - 500) / 5500
ratio = 0.0909
duty = 190 + 0.0909 * (255 - 190)
duty = 195.91
```

Esto significa que para 1000 RPM el sistema parte aproximadamente desde duty 196 antes de sumar la correccion PID.

## 12. Arranque del motor

Un motor DC puede no arrancar con un duty bajo debido a la friccion estatica. Por eso el firmware usa un modo de arranque llamado `STARTING`.

Parametros:

```c
#define RPM_STARTUP_DUTY 255.0f
#define RPM_STARTUP_MIN_MS 500
#define RPM_STARTUP_MIN_RPM 80.0f
```

Cuando la referencia es mayor que cero, el motor inicia con duty 255. Despues, cuando el sensor detecta suficiente velocidad, se pasa al modo `RUNNING`.

## 13. Paso de STARTING a RUNNING

El paso a control cerrado se calcula con:

```text
RPM_handoff = max(80 RPM, referencia * 0.85)
```

La relacion 0.85 sale de una calibracion:

```text
425 / 500 = 0.85
```

Ejemplos:

| Referencia | Handoff aproximado |
| ---: | ---: |
| 100 RPM | 85 RPM |
| 500 RPM | 425 RPM |
| 1000 RPM | 850 RPM |
| 3000 RPM | 2550 RPM |

Esta logica evita usar un umbral fijo que pueda ser demasiado alto para referencias bajas.

## 14. Rampa progresiva de ayuda

Durante el desarrollo se encontro que aplicar un duty minimo de golpe podia causar saltos bruscos. Para evitarlo, se implemento una ayuda progresiva.

Parametros:

```c
#define RPM_CONTROL_DEADBAND 90.0f
#define RPM_MIN_EFFECTIVE_BAND_RATIO 0.02f
#define RPM_MIN_EFFECTIVE_BAND_MIN 10.0f
```

La banda se calcula asi:

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

Si el error es pequeno, la ayuda es pequena. Si el error aumenta, la ayuda sube progresivamente.

## 15. Slew rate del duty

El duty aplicado no cambia instantaneamente. Se limita con:

```c
#define PWM_SLEW_STEP_UP 10.0f
#define PWM_SLEW_STEP_DOWN 35.0f
```

Esto significa:

- El duty puede subir maximo 10 unidades por ciclo.
- El duty puede bajar maximo 35 unidades por ciclo.

Como el ciclo de control es de 100 ms, la subida es mas suave y la bajada es mas rapida cuando el motor se pasa de velocidad.

## 16. OLED

La pantalla OLED SSD1306 muestra informacion local del sistema. Se conecta por I2C:

```text
SDA -> GPIO6
SCL -> GPIO7
```

La pantalla se actualiza en una tarea separada:

```c
xTaskCreate(oled_task, "oled_task", 4096, NULL, 3, NULL);
```

Esto evita que la pantalla afecte directamente el tiempo del control principal.

## 17. UART

La interfaz UART permite modificar la referencia desde el monitor serial.

Comandos soportados:

```text
rpm 500
set 1000
ref=3000
rpm 0
```

El comando `rpm 0` pausa el motor.

## 18. Interfaz web

La interfaz web se implementa en:

```text
main/http_server.c
```

Permite:

- Ver RPM medida.
- Ver referencia.
- Ver PIDOUT.
- Ver CMD.
- Ver DUTY.
- Cambiar la referencia.
- Iniciar y pausar.

Endpoints:

| Metodo | Ruta | Funcion |
| --- | --- | --- |
| GET | `/` | Pagina web |
| GET | `/health` | Estado basico |
| GET | `/api/status` | Estado completo |
| GET | `/api/setpoint` | Referencia actual |
| POST | `/api/setpoint` | Cambiar referencia |

El navegador consulta el estado cada 250 ms.

## 19. Organizacion del codigo

| Archivo | Funcion |
| --- | --- |
| `main/main.c` | Inicializa el sistema |
| `main/config.h` | Define pines y parametros |
| `main/motor.c` | Configura PWM y direccion |
| `main/rpm.c` | Mide y filtra RPM |
| `main/pid.c` | Calcula PID |
| `main/control.c` | Ejecuta la logica principal de control |
| `main/oled.c` | Controla la pantalla OLED |
| `main/uart_cmd.c` | Procesa comandos seriales |
| `main/wifi_connect.c` | Conecta a WiFi |
| `main/http_server.c` | Sirve la pagina web y API |

## 20. Procedimiento de prueba

1. Conectar el circuito con tierra comun.
2. Verificar que el sensor cambie de estado con la marca.
3. Compilar el proyecto.
4. Flashear el ESP32-C6.
5. Abrir el monitor serial.
6. Enviar `rpm 500`.
7. Verificar el paso de `STARTING` a `RUNNING`.
8. Observar RPM, REF, PIDOUT, CMD y DUTY.
9. Probar otras referencias como 1000, 3000 y 6000 RPM.
10. Revisar la pagina web si WiFi esta configurado.

## 21. Comandos de compilacion

```powershell
cd C:\Universidad\RPM_regulation_esp32c6_TCRT500
. C:\esp\v6.0.1\esp-idf\export.ps1
idf.py set-target esp32c6
idf.py build
idf.py -p COM13 flash monitor
```

## 22. Resultados esperados

En el monitor serial se espera una salida similar a:

```text
RPM: 500.00 | REF: 500.00 | PIDOUT: ... | CMD: ... | DUTY: ... | MODE: RUNNING
```

Interpretacion:

- `RPM`: velocidad medida.
- `REF`: referencia deseada.
- `PIDOUT`: correccion calculada por el PID.
- `CMD`: comando calculado antes del suavizado final.
- `DUTY`: duty aplicado al PWM.
- `MODE`: estado del control.

## 23. Conclusiones

Se implemento un sistema completo de regulacion de velocidad para motor DC usando ESP32-C6. El sistema integra medicion optica, filtrado de RPM, control PID, PWM, driver L293D, OLED, UART y pagina web.

El uso de medicion por periodo permite una lectura mas adecuada para un sensor de una marca por vuelta. El feedforward reduce la carga del PID al partir de un duty base aproximado. El arranque en lazo abierto permite vencer la friccion estatica del motor. La rampa progresiva de ayuda reduce saltos bruscos cuando la velocidad cae cerca de la referencia.

El proyecto queda documentado y organizado para ser presentado como una solucion embebida de control de velocidad con realimentacion.

## 24. Recomendaciones

- Mantener todos los GND unidos.
- Separar cables del motor de cables del sensor.
- Usar una marca de alto contraste para el TCRT5000.
- Ajustar bien el potenciometro del sensor.
- No modificar el PID sin registrar datos del monitor serial.
- Probar primero en 500 RPM antes de subir la referencia.
- Si se requiere mayor precision, aumentar la cantidad de marcas por vuelta y actualizar `PULSES_PER_REV`.
