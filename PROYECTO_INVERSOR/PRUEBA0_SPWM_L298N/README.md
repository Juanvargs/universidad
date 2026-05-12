# Prueba de inversor SPWM con STM32G474RE y L298N

**Autor:** Juan Pablo Vargas Córdoba  
**Universidad:** Universidad Nacional de Colombia  
**Proyecto:** Prueba de generación SPWM para control de puente H con módulo L298N

## Resumen

En este proyecto se implementó una prueba experimental de un inversor monofásico
controlado mediante SPWM usando una tarjeta **STM32 NUCLEO-G474RE** y un módulo
**L298N**. La STM32 genera tres señales de control: una señal SPWM de alta
frecuencia para el pin `ENA` del L298N y dos señales digitales complementarias
para los pines `IN1` e `IN2`. El L298N usa estas señales para conmutar su puente
H interno y obtener una salida alterna diferencial entre `OUT1` y `OUT2`.

La salida se verificó con un **Analog Discovery 2** usando WaveForms. Primero se
midió la salida diferencial sin filtro físico, y luego se implementó un filtro
LC en protoboard para reducir la componente de alta frecuencia de la SPWM y
observar una forma de onda más cercana a una senoidal.

## Objetivos

- Configurar la STM32G474RE para generar una señal SPWM por hardware usando
  `TIM1_CH1`.
- Controlar el puente A del L298N usando `ENA`, `IN1` e `IN2`.
- Comprobar que el L298N invierte la polaridad de la salida mediante el puente H
  interno.
- Medir correctamente la salida diferencial del inversor como `OUT1 - OUT2`.
- Implementar un filtro LC físico para suavizar la señal SPWM de salida.
- Documentar el montaje, el código, las mediciones y el funcionamiento del
  sistema.

## Componentes utilizados

- STM32 NUCLEO-G474RE.
- Módulo L298N.
- Protoboard y jumpers.
- Fuente DC externa.
- Analog Discovery 2 con WaveForms.
- Resistencia de carga de `10 ohm / 10 W`.
- Inductor de `270 uH`.
- Capacitores electrolíticos conectados en pares espalda con espalda.

## Descripción general del sistema

El sistema se divide en tres bloques:

```text
STM32 NUCLEO-G474RE  ->  L298N puente H  ->  Filtro LC y carga
```

La STM32 no entrega potencia a la carga. Su función es generar las señales de
control. La potencia que recibe la carga proviene de la fuente externa conectada
al L298N.

```text
Fuente DC de potencia
        |
        v
     L298N
 puente H interno
        |
        v
  OUT1 - OUT2
 salida alterna diferencial
```

Las señales de control usadas son:

```text
A5 / PC0 -> ENA -> SPWM
D7 / PA8 -> IN1 -> selección de polaridad
D8 / PA9 -> IN2 -> selección de polaridad complementaria
```

## Conexiones principales

### Pines usados en la STM32

| Pin físico Nucleo | Pin MCU | Función |
| --- | --- | --- |
| `A5` | `PC0` | Señal SPWM hacia `ENA` del L298N |
| `D7` | `PA8` | Señal digital hacia `IN1` |
| `D8` | `PA9` | Señal digital hacia `IN2` |
| `GND` | GND | Tierra común del sistema |
| `5V` | 5 V | Alimentación lógica del módulo L298N durante la prueba |

### Pines usados en el L298N

| Pin del L298N | Conexión | Función |
| --- | --- | --- |
| `A Enable` / `ENA` | `A5 / PC0` | Habilita el puente A mediante SPWM |
| `IN1` | `D7 / PA8` | Selecciona una polaridad del puente |
| `IN2` | `D8 / PA9` | Selecciona la polaridad contraria |
| `OUT1` | Carga/filtro | Salida del puente A |
| `OUT2` | Carga/filtro | Salida complementaria del puente A |
| `+5V Power` | `5V` de la STM32 | Alimentación lógica del módulo |
| `Power GND` | GND común | Referencia común |
| `+12V Power` | Fuente externa | Entrada de potencia del puente H |

La imagen siguiente muestra la distribución de pines del módulo L298N utilizado.
En el proyecto se empleó solamente el puente A, es decir, `A Enable`, `IN1`,
`IN2`, `OUT1` y `OUT2`.

![Módulo L298N usado en la prueba](Imagenes/L298N.png)

Para que la STM32 pueda controlar `ENA`, se retiró el jumper de `A Enable`. Si
ese jumper queda instalado, el módulo mantiene `ENA` fijo en alto y la STM32 no
puede aplicar la SPWM sobre el pin de habilitación.

## Alimentación del L298N

El L298N requiere dos alimentaciones conceptualmente diferentes:

1. **Alimentación lógica (`VSS`)**

   Alimenta la electrónica interna de control del L298N. En el módulo aparece
   como `+5V Power`. En esta prueba se alimentó desde el pin `5V` de la Nucleo.
   Esta alimentación no mueve la carga; solo permite que el integrado interprete
   las señales `ENA`, `IN1` e `IN2`.

2. **Alimentación de potencia (`VS`)**

   Es la alimentación que el puente H conmuta hacia la carga. En el módulo
   aparece como `+12V Power`. Aunque el borne se llame `+12V`, durante la prueba
   se usó una fuente de `5 V` para reducir corriente y calentamiento.

Durante el montaje hubo dos tensiones de `5 V` con funciones distintas:

```text
5 V de la STM32  -> alimentación lógica del L298N
5 V de la fuente -> alimentación de potencia del puente H
```

Todas las partes del sistema deben compartir la misma referencia:

```text
GND STM32 ---- GND protoboard ---- Power GND L298N ---- GND Analog Discovery
```

## Fundamento de la SPWM

La SPWM, o modulación sinusoidal por ancho de pulso, consiste en generar pulsos
de amplitud fija pero con ancho variable. La altura instantánea del pulso la
determina la alimentación de potencia del puente H; lo que cambia es el tiempo
que ese pulso permanece encendido.

Para una frecuencia PWM de `20 kHz`, el periodo de cada pulso es:

```text
T_PWM = 1 / 20000 = 50 us
```

Si el duty cycle es `80%`, la salida permanece activa durante:

```text
t_on = 50 us * 0.80 = 40 us
```

y apagada durante:

```text
t_off = 50 us * 0.20 = 10 us
```

Si la alimentación de potencia ideal es `5 V`, el promedio ideal durante ese
periodo PWM es:

```text
V_promedio = Vdc * duty
V_promedio = 5 V * 0.80 = 4 V
```

Esto no significa que el pulso tenga una altura de `4 V`. El pulso sigue
intentando llegar a la tensión de la fuente de potencia. Lo que equivale a
`4 V` es el valor promedio durante el periodo PWM.

En el puente H, el signo del promedio depende de la polaridad seleccionada por
`IN1` e `IN2`:

```text
Vout_promedio = polaridad * Vdc * duty
```

donde:

```text
polaridad = +1 cuando IN1 = 1 e IN2 = 0
polaridad = -1 cuando IN1 = 0 e IN2 = 1
```

Por lo tanto, la SPWM no genera directamente una senoidal pura. Genera pulsos
rápidos cuyo valor promedio sigue una forma senoidal. El filtro LC se encarga de
atenuar la componente de alta frecuencia y dejar visible la componente de baja
frecuencia.

## Función de cada señal

Para entender el control del puente H es necesario separar dos funciones:

- `IN1` e `IN2` definen el sentido de la corriente por la carga.
- `ENA` define durante cuánto tiempo se deja pasar energía en ese sentido.

```text
IN1 e IN2 -> eligen la polaridad de la salida
ENA       -> habilita o deshabilita esa polaridad con SPWM
```

### Señal `A5 / PC0 -> ENA`

`A5` corresponde al pin `PC0` de la STM32. En CubeMX fue configurado como
`TIM1_CH1`, por lo que puede generar PWM por hardware.

Esta señal entra al pin `ENA` del L298N. `ENA` significa "Enable A", es decir,
habilitación del puente A.

```text
ENA = 1 -> el puente A puede conducir
ENA = 0 -> el puente A queda deshabilitado
```

En este proyecto `ENA` recibe la señal SPWM. Por eso el puente H se enciende y
se apaga muchas veces por segundo. Cuando `ENA` está en alto, la fuente DC se
aplica a la carga con la polaridad definida por `IN1` e `IN2`. Cuando `ENA` está
en bajo, el puente deja de entregar energía.

En resumen:

```text
A5 / PC0 / ENA = señal rápida SPWM que regula la energía promedio entregada.
```

### Señal `D7 / PA8 -> IN1`

`D7` corresponde al pin `PA8` de la STM32. Esta señal entra a `IN1` del L298N.

`IN1` no es la SPWM. Es una señal de dirección. Trabaja junto con `IN2` para
seleccionar qué par diagonal del puente H conduce.

En el código, `IN1` cambia cada `10 ms`:

```text
Primeros 10 ms   -> IN1 = 1
Siguientes 10 ms -> IN1 = 0
```

Como un periodo completo dura `20 ms`, la frecuencia fundamental de salida es:

```text
f = 1 / 20 ms = 50 Hz
```

En resumen:

```text
D7 / PA8 / IN1 = señal lenta que participa en la selección de polaridad.
```

### Señal `D8 / PA9 -> IN2`

`D8` corresponde al pin `PA9` de la STM32. Esta señal entra a `IN2` del L298N.

`IN2` es complementaria a `IN1`:

```text
Si IN1 = 1, entonces IN2 = 0
Si IN1 = 0, entonces IN2 = 1
```

Esta complementariedad permite que el puente H entregue primero una polaridad y
luego la polaridad opuesta.

En resumen:

```text
D8 / PA9 / IN2 = señal lenta complementaria a IN1.
```

### Operación conjunta

Durante el primer semiciclo:

```text
IN1 = 1
IN2 = 0
ENA = SPWM
```

El puente queda configurado para una polaridad. Cada vez que `ENA` está en alto,
la carga recibe un pulso de esa polaridad.

Durante el segundo semiciclo:

```text
IN1 = 0
IN2 = 1
ENA = SPWM
```

El puente queda configurado con la polaridad contraria. Cada vez que `ENA` está
en alto, la carga recibe un pulso de signo opuesto.

| Intervalo | IN1 / D7 | IN2 / D8 | ENA / A5 | Efecto sobre `OUT1 - OUT2` |
| --- | --- | --- | --- | --- |
| Primeros `10 ms` | Alto | Bajo | SPWM | Pulsos de una polaridad |
| Siguientes `10 ms` | Bajo | Alto | SPWM | Pulsos de polaridad contraria |

## Funcionamiento del puente H en el L298N

El L298N contiene dos puentes H internos. En esta prueba se utilizó el puente A,
formado por `IN1`, `IN2`, `ENA`, `OUT1` y `OUT2`.

![Diagrama de bloques interno del L298N](Imagenes/Diagrama_de_bloques_L298N.png)

En un puente H, la carga se conecta entre dos nodos de salida. La inversión de
polaridad se logra activando pares diagonales de transistores.

Para una polaridad:

```text
+Vdc -> transistor superior izquierdo -> OUT1 -> carga -> OUT2
     -> transistor inferior derecho -> GND
```

Para la polaridad contraria:

```text
+Vdc -> transistor superior derecho -> OUT2 -> carga -> OUT1
     -> transistor inferior izquierdo -> GND
```

Así, la corriente atraviesa la carga en un sentido durante un semiciclo y en el
sentido contrario durante el siguiente semiciclo.

La STM32 no controla directamente cada transistor interno. La STM32 entrega las
órdenes lógicas:

```text
IN1/IN2 -> seleccionan el par diagonal
ENA     -> habilita ese par con pulsos SPWM
```

## Salida diferencial `OUT1 - OUT2`

La salida útil del puente H no se mide como `OUT1` contra GND ni como `OUT2`
contra GND. La carga está conectada entre `OUT1` y `OUT2`, por lo que la tensión
real sobre la carga es:

```text
Vout = OUT1 - OUT2
```

Si se mide cada salida por separado respecto a GND, ambas señales se mueven
entre `0 V` y la tensión de la fuente de potencia. Por eso no se observa una
señal negativa clara en cada canal individual.

Ejemplo:

```text
OUT1 = +5 V
OUT2 = 0 V
Vout = OUT1 - OUT2 = +5 V
```

Luego el puente invierte:

```text
OUT1 = 0 V
OUT2 = +5 V
Vout = OUT1 - OUT2 = -5 V
```

Por esta razón, en WaveForms se utilizó:

```text
CH1 -> OUT1
CH2 -> OUT2
Math 1 = C1 - C2
```

`Math 1` representa la tensión que realmente recibe la carga.

## Diseño del código

El código propio del proyecto se encuentra principalmente en:

- `Core/Src/main.c`
- `Core/Src/l298n_spwm.c`
- `Core/Inc/l298n_spwm.h`

Los demás archivos corresponden principalmente a inicialización generada por
STM32CubeMX: reloj, GPIO, TIM1, BSP de la Nucleo y archivos HAL/CMSIS.

### Archivo `main.c`

En `main.c` se inicializan los periféricos:

```c
MX_GPIO_Init();
MX_TIM1_Init();
```

Luego se inicializa y arranca el módulo de control SPWM:

```c
L298N_SPWM_Init();
L298N_SPWM_SetModulation(L298N_MODULACION_SPWM);
L298N_SPWM_Start();
```

La modulación se definió como:

```c
#define L298N_MODULACION_SPWM       800U
```

Esto equivale a una modulación del `80%`.

Dentro del ciclo infinito se ejecuta:

```c
L298N_SPWM_Task();
```

Esta función actualiza el duty de la SPWM y cambia la polaridad cada `10 ms`.

### Archivo `l298n_spwm.c`

Este archivo contiene la lógica de generación SPWM.

Parámetros principales:

```c
#define L298N_PWM_TIMER              htim1
#define L298N_PWM_CHANNEL            TIM_CHANNEL_1
#define L298N_SPWM_TABLE_SIZE        100U
#define L298N_SPWM_UPDATE_US         200U
#define L298N_OUTPUT_PERIOD_US       20000U
#define L298N_HALF_PERIOD_US         10000U
#define L298N_DEFAULT_MODULATION     800U
```

Interpretación:

- `TIM1_CH1` genera el PWM en `PC0/A5`.
- La tabla SPWM contiene `100` muestras.
- Cada muestra dura `200 us`.
- `100 * 200 us = 20000 us = 20 ms`.
- Un periodo de `20 ms` equivale a `50 Hz`.
- Cada `10 ms` se invierte la polaridad del puente.

La tabla `spwm_abs_sine_table` contiene valores entre `0` y `1000`. Se usa una
tabla senoidal absoluta porque el PWM no maneja valores negativos. El signo de
la salida lo determinan `IN1` e `IN2`.

La función `SetBridgePolarity()` establece la polaridad:

```c
1U -> IN1 alto, IN2 bajo
0U -> IN1 bajo, IN2 alto
```

La función `SetDutyPermille()` actualiza el registro de comparación del timer:

```c
__HAL_TIM_SET_COMPARE(...)
```

La tarea `L298N_SPWM_Task()` realiza el proceso repetitivo:

1. Lee el tiempo actual.
2. Calcula la posición dentro del periodo de `20 ms`.
3. Selecciona la muestra correspondiente de la tabla.
4. Invierte `IN1/IN2` si se llega al siguiente semiciclo.
5. Aplica el nuevo duty al PWM de `ENA`.

## Medición con Analog Discovery 2

La medición se realizó conectando:

```text
AD2 GND -> GND común
CH1 1+  -> OUT1
CH2 2+  -> OUT2
```

En WaveForms se creó:

```text
Math 1 = C1 - C2
```

Después se agregó un filtro digital:

```text
Math 2 = filtro low-pass de Math 1
```

El filtro digital solo se utilizó para visualizar mejor la componente de baja
frecuencia. No modifica el circuito físico.

### Salida diferencial con filtro digital, sin filtro LC real

La siguiente captura muestra la salida diferencial `OUT1 - OUT2` con ayuda de
un filtro digital en WaveForms, antes de montar el filtro LC físico.

![Salida con filtro digital sin filtro LC real](Imagenes/PRUEBA1_CON_FILTRO.png)

### Salida con filtro LC físico y filtro digital

La siguiente captura muestra la salida después de implementar el filtro LC en la
protoboard. La señal medida ya presenta menor contenido de conmutación, y la
versión filtrada digitalmente permite observar mejor la componente senoidal.

![Salida con filtro LC real](Imagenes/PRUEBA1_CON_FILTRO_DOS.png)

## Filtro LC físico implementado

El filtro LC se conectó en la salida diferencial del puente H:

```text
OUT1 ---- L 270 uH ---- nodo filtrado ---- carga 10 ohm ---- OUT2
                              |
                              C equivalente
                              |
                            OUT2
```

El inductor se conectó en serie desde `OUT1`. La resistencia de carga y el
capacitor equivalente se conectaron entre el nodo filtrado y `OUT2`.

La configuración final de prueba fue:

```text
L = 270 uH
Carga = 10 ohm / 10 W
C = 47 uF + 47 uF en serie espalda con espalda
Ceq aproximado = 23.5 uF
Fuente de potencia = 5 V
```

Como los capacitores disponibles eran electrolíticos polarizados, se usaron dos
capacitores iguales en serie espalda con espalda para obtener un capacitor
equivalente no polarizado:

```text
nodo filtrado ---- +| |- ---- -| |+ ---- OUT2
```

Para dos capacitores iguales en serie:

```text
Ceq = C / 2
```

Por lo tanto, para dos capacitores de `47 uF`:

```text
Ceq = 47 uF / 2 = 23.5 uF
```

La frecuencia de corte aproximada del filtro LC es:

```text
fc = 1 / (2*pi*sqrt(L*C))
```

Con `L = 270 uH` y `C = 23.5 uF`, se obtiene aproximadamente:

```text
fc ~= 2.0 kHz
```

Esta frecuencia permite conservar la componente de `50 Hz` y atenuar parte de la
portadora PWM de aproximadamente `20 kHz`.

## Resultados

Con la configuración implementada se verificó que:

- La STM32 genera la señal SPWM en `PC0/A5`.
- `PA8/D7` y `PA9/D8` cambian de estado de forma complementaria cada `10 ms`.
- El L298N invierte la polaridad entre `OUT1` y `OUT2`.
- La salida útil debe medirse como `OUT1 - OUT2`.
- Sin filtro LC, la salida se observa como una señal bipolar compuesta por
  pulsos SPWM.
- Con el filtro LC, la salida presenta una forma más suave y cercana a una
  senoidal.

## Conclusiones

La prueba permitió comprobar experimentalmente el principio de operación de un
inversor controlado por SPWM. El pin `ENA` del L298N recibe una señal PWM de
ancho variable, mientras que `IN1` e `IN2` determinan la polaridad de la salida.
De esta forma, el puente H entrega pulsos positivos durante un semiciclo y
pulsos negativos durante el semiciclo contrario.

La medición diferencial `OUT1 - OUT2` fue esencial para observar la salida real
aplicada a la carga. Además, la implementación del filtro LC permitió reducir la
componente de alta frecuencia asociada a la conmutación y visualizar una forma
de onda más senoidal.

El montaje con L298N es adecuado como demostración didáctica del control SPWM y
del funcionamiento de un puente H. Para una etapa de potencia definitiva sería
necesario utilizar un driver y dispositivos de conmutación diseñados para mayor
eficiencia y capacidad de corriente.
