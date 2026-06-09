# Hardware y conexiones

Este documento explica los componentes usados y la funcion de cada pin en el montaje.

## Componentes

| Componente | Funcion en el sistema |
| --- | --- |
| ESP32-C6 | Ejecuta el firmware, mide RPM y genera PWM |
| TCRT5000 | Detecta cambios de blanco/negro para contar vueltas |
| L293D | Etapa de potencia para alimentar el motor DC |
| Motor DC | Elemento mecanico que se regula |
| OLED SSD1306 | Muestra RPM, referencia y duty |
| Fuente externa | Entrega corriente al motor |

## Pinout definido en el codigo

Archivo usado:

```text
main/config.h
```

Codigo:

```c
#define PIN_PWM     4
#define PIN_IN1     5
#define PIN_IN2     10
#define PIN_SENSOR  1

#define OLED_I2C_SDA 6
#define OLED_I2C_SCL 7
```

Tabla de conexion:

| Funcion | GPIO ESP32-C6 | Se conecta a |
| --- | ---: | --- |
| PWM del motor | GPIO4 | L293D pin 1, `EN1` |
| Direccion 1 | GPIO5 | L293D pin 2, `IN1` |
| Direccion 2 | GPIO10 | L293D pin 7, `IN2` |
| Sensor RPM | GPIO1 | TCRT5000 `OUT` |
| I2C SDA | GPIO6 | OLED `SDA` |
| I2C SCL | GPIO7 | OLED `SCL` |

## L293D

El L293D permite manejar el motor con una fuente externa, ya que el ESP32-C6 no puede entregar la corriente requerida por el motor.

| Pin L293D | Nombre | Conexion |
| ---: | --- | --- |
| 1 | EN1 | GPIO4, PWM |
| 2 | IN1 | GPIO5 |
| 3 | OUT1 | Terminal 1 del motor |
| 4 | GND | Tierra comun |
| 5 | GND | Tierra comun |
| 6 | OUT2 | Terminal 2 del motor |
| 7 | IN2 | GPIO10 |
| 8 | VCC2 | Positivo de la fuente del motor |
| 12 | GND | Tierra comun |
| 13 | GND | Tierra comun |
| 16 | VCC1 | 5 V logica |

La direccion se fija en `motor.c`:

```c
void motor_set_direction(int dir) {
    if (dir > 0) {
        gpio_set_level(PIN_IN1, 1);
        gpio_set_level(PIN_IN2, 0);
    } else {
        gpio_set_level(PIN_IN1, 0);
        gpio_set_level(PIN_IN2, 1);
    }
}
```

En `control.c` se llama siempre:

```c
motor_set_direction(1);
```

Por eso el motor gira en una direccion fija durante la regulacion.

## Sensor TCRT5000

| Pin sensor | Conexion |
| --- | --- |
| VCC | 3.3 V |
| GND | Tierra comun |
| OUT | GPIO1 |

El sensor se configura en `rpm.c`:

```c
gpio_config_t sensor_config = {
    .pin_bit_mask = 1ULL << PIN_SENSOR,
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_NEGEDGE,
};
```

Se usa `GPIO_INTR_NEGEDGE`, es decir, se cuenta el flanco de alto a bajo. El potenciometro del modulo debe ajustarse para que la salida digital cambie limpiamente cuando pasa la marca blanca/negra.

## OLED SSD1306

| Pin OLED | Conexion |
| --- | --- |
| VCC | 3.3 V |
| GND | Tierra comun |
| SDA | GPIO6 |
| SCL | GPIO7 |

Parametros:

```c
#define OLED_I2C_ADDR     0x3C
#define OLED_I2C_FREQ_HZ  400000
```

La pantalla se inicializa en `main.c`. Si falla, el control del motor sigue funcionando y solo se deshabilita la visualizacion OLED.

## Tierra comun

La tierra comun es obligatoria. Si la fuente del motor, el L293D y el ESP32-C6 no comparten GND, las senales de control pueden quedar sin referencia electrica y el motor puede no arrancar o comportarse de forma erratica.

Conexion minima de tierras:

```text
GND ESP32-C6
GND TCRT5000
GND OLED
GND L293D pines 4, 5, 12, 13
Negativo de fuente externa del motor
```
