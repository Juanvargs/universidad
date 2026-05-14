# Punto 1 UART: Juego de adivinar el numero con ESP32-C6 y ESP-IDF

## Que es y como funciona la interfaz de comunicacion serial asincrona UART

UART significa **Universal Asynchronous Receiver/Transmitter**. Es una interfaz de comunicacion serial que permite enviar y recibir datos bit a bit entre dos dispositivos.

Se llama **asincrona** porque no usa una señal de reloj compartida. En su lugar, ambos dispositivos deben trabajar con la misma velocidad de comunicacion, llamada **baud rate**. En este proyecto se usa:

```text
115200 baudios
```

UART transmite datos usando una linea de envio y una linea de recepcion:

```text
TX -> transmite datos
RX -> recibe datos
GND -> referencia comun
```

En este proyecto no se usan pines externos TX/RX. La ESP32-C6 se comunica con el PC mediante el cable USB y aparece como un puerto serial, por ejemplo `COM11`. Desde el punto de vista del usuario, la comunicacion se hace por el **Serial Monitor**.

```text
ESP32-C6 <==== USB ==== > PC / Serial Monitor
```

En ESP-IDF, el canal USB serial interno de la ESP32-C6 se maneja con el driver **USB Serial/JTAG**, pero aqui se usa solamente como comunicacion serial hacia el monitor del PC, no como depuracion JTAG.

## Objetivos del proyecto

- Implementar un juego de adivinar un numero en la ESP32-C6.
- Usar comunicacion serial por USB para enviar datos desde el PC hacia la placa.
- Pedir al usuario un rango minimo y maximo.
- Generar un numero secreto aleatorio dentro del rango.
- Pedir el numero maximo de intentos para cada ronda.
- Indicar si el numero secreto es mayor, menor o correcto.
- Reiniciar el juego al acertar o al agotar los intentos.
- Usar el boton BOOT para volver a mostrar mensajes de ayuda en el Serial Monitor.

## Componentes requeridos

| Componente | Cantidad | Descripcion |
| --- | ---: | --- |
| ESP32-C6 | 1 | Placa principal del proyecto |
| Cable USB | 1 | Programacion y comunicacion serial |
| PC con VS Code y ESP-IDF | 1 | Compilar, flashear y usar Serial Monitor |

## Conexion

No se necesitan conexiones externas.

```text
ESP32-C6 <==== USB ==== > PC / Serial Monitor
```

## Como usar el proyecto

1. Abrir el proyecto en VS Code con la extension ESP-IDF.
2. Seleccionar el target:

```text
esp32c6
```

3. Seleccionar el puerto de la placa, por ejemplo:

```text
COM11
```

4. Seleccionar metodo de flasheo:

```text
UART
```

5. Flashear la placa con **Flash Device**.
6. Cerrar cualquier monitor de ESP-IDF que tenga ocupado el puerto.
7. Abrir **Serial Monitor**.
8. Configurar:

```text
Port: COM11
Baud rate: 115200
Line ending: LF o CRLF
```

9. Presionar **Start**.
10. Presionar el boton **BOOT** para mostrar el mensaje inicial.
11. Enviar el rango:

```text
10 50
```

12. Enviar el numero maximo de intentos:

```text
4
```

13. Enviar intentos hasta acertar o agotar los intentos.

## Ejemplo de uso

```text
=== Juego de Adivinar el Numero por UART ===
ESP32-C6 con ESP-IDF
Ingrese dos numeros separados por espacio para definir el rango.
Ejemplo: 10 50
Luego ingrese el numero maximo de intentos para esa ronda.
Ejemplo intentos: 4

Escriba el rango minimo y maximo, luego presione Enter.
Rango minimo y maximo: 10 50

Escriba el numero maximo de intentos, luego presione Enter.
Numero de intentos: 4

Juego iniciado.
Adivine el numero entre 10 y 50.
Tiene 4 intentos para acertar.
Intento: 25
El numero secreto es mayor.
Intento: 40
El numero secreto es menor.
Intento: 33
Correcto. Adivinaste el numero en 3 intentos.
```

## Estructura general del codigo

El proyecto esta implementado en:

```text
main/main.c
```

Aunque todo esta en un solo archivo, el codigo esta organizado por funciones. La estructura general es:

```text
1. Librerias y constantes
2. Tipo de resultado de entrada
3. Mensajes y configuracion de hardware
4. Lectura de entrada por serial y BOOT
5. Validacion de datos escritos por el usuario
6. Generacion del numero secreto
7. Solicitud de rango e intentos
8. Ejecucion de la ronda
9. Funcion principal app_main()
```

## Grupos principales de funciones

### 1. Configuracion y mensajes

Estas funciones preparan la comunicacion serial, el boton BOOT y los mensajes iniciales.

| Funcion | Para que sirve |
| --- | --- |
| `print_welcome()` | Muestra el encabezado e instrucciones del juego. |
| `configure_usb_serial()` | Configura el canal serial USB de la ESP32-C6 para comunicarse con el PC. |
| `configure_boot_button()` | Configura el boton BOOT como entrada digital. |
| `boot_button_is_pressed()` | Revisa si el boton BOOT esta presionado. |
| `wait_for_boot_release()` | Espera a que el usuario suelte BOOT para evitar multiples lecturas. |
| `boot_press_confirmed()` | Confirma una pulsacion estable de BOOT usando antirrebote. |

### 2. Lectura de entrada

Estas funciones esperan datos desde el Serial Monitor y tambien detectan el boton BOOT.

| Funcion | Para que sirve |
| --- | --- |
| `read_start_line()` | Espera BOOT, Enter o una linea inicial antes de mostrar el mensaje principal. |
| `read_line()` | Lee una linea completa desde el Serial Monitor o detecta BOOT durante la espera. |

Secuencia de `read_start_line()`:

```text
Espera BOOT o texto serial.
Si llega BOOT -> inicia sin texto previo.
Si llega Enter vacio -> inicia sin texto previo.
Si llega texto + Enter -> guarda ese texto y lo usa como posible rango.
```

Secuencia de `read_line()`:

```text
Mientras espera entrada, revisa BOOT.
Si BOOT se presiona -> devuelve INPUT_BOOT_PRESSED.
Si llega texto + Enter -> guarda la linea y devuelve INPUT_LINE_RECEIVED.
Si llega Enter sin texto -> lo ignora y sigue esperando.
```

### 3. Validacion de datos

Estas funciones revisan que lo escrito por el usuario tenga el formato correcto.

| Funcion | Para que sirve |
| --- | --- |
| `parse_range()` | Convierte una linea como `10 50` en rango minimo y maximo. |
| `parse_guess()` | Convierte una linea como `25` en un intento numerico. |
| `parse_attempt_limit()` | Convierte una linea como `4` en el limite de intentos. |

### 4. Logica del juego

Estas funciones contienen el comportamiento principal del juego.

| Funcion | Para que sirve |
| --- | --- |
| `generate_secret_number()` | Genera el numero secreto dentro del rango, incluyendo minimo y maximo. |
| `request_valid_range()` | Pide el rango hasta recibir uno valido. |
| `request_valid_attempt_limit()` | Pide el numero maximo de intentos hasta recibir uno valido. |
| `play_round()` | Ejecuta una ronda completa del juego. |
| `app_main()` | Inicializa el sistema y mantiene el juego ejecutandose. |

## Funcion importante: generacion del numero secreto

La funcion:

```c
static int generate_secret_number(int range_min, int range_max)
```

genera el numero secreto usando esta idea:

```text
range_size = range_max - range_min + 1
random_offset = esp_random() % range_size
numero_secreto = range_min + random_offset
```

El `+ 1` es importante porque permite incluir el valor maximo del rango.

Ejemplo:

```text
Rango: 10 a 15
range_size = 15 - 10 + 1 = 6
random_offset puede ser 0, 1, 2, 3, 4 o 5
numero posible: 10, 11, 12, 13, 14 o 15
```

Por eso el numero secreto puede ser igual al minimo o igual al maximo.

## Flujo general del programa

```text
La ESP32-C6 configura el serial USB.
La ESP32-C6 configura el boton BOOT.
El programa espera BOOT, Enter o una linea inicial.
Muestra el mensaje de bienvenida.
Pide un rango minimo y maximo.
Valida el rango.
Pide el numero maximo de intentos.
Valida el numero de intentos.
Genera el numero secreto.
Pide intentos al usuario.
Responde si el numero secreto es mayor, menor o correcto.
Si el usuario acierta -> termina la ronda.
Si se acaban los intentos -> muestra el numero secreto.
Vuelve a pedir rango e intentos para una nueva ronda.
```

## Notas importantes

- El boton BOOT se usa para volver a mostrar mensajes cuando el Serial Monitor se abre de nuevo.
- No se debe mantener BOOT presionado mientras se reinicia la placa, porque puede entrar en modo bootloader.
- Si el Serial Monitor deja de responder despues de presionar RESET, cerrar y abrir nuevamente el puerto `COM11`.
- Para flashear desde VS Code, usar metodo **UART**, no **JTAG**.
- Aunque ESP-IDF llama internamente al canal como USB Serial/JTAG, en este proyecto se usa como comunicacion serial por USB hacia el PC.
