# PRUEBA0_SPWM_L298N

Proyecto de prueba para generar una salida tipo inversor usando una STM32
NUCLEO-G474RE y un modulo L298N. La STM32 genera las senales de control y el
L298N actua como puente H de potencia para convertir una alimentacion DC en una
salida alterna diferencial entre `OUT1` y `OUT2`.

Este proyecto se hizo como una prueba de laboratorio antes de pasar a un
inversor real con driver y transistores de potencia dedicados.

## Objetivo

El objetivo de esta prueba fue comprobar todo el camino de control y potencia:

- Generar una senal SPWM desde la STM32.
- Usar dos senales digitales para invertir la polaridad del puente H.
- Controlar el modulo L298N con tres senales: `ENA`, `IN1` e `IN2`.
- Verificar en el osciloscopio que el puente H entrega una salida bipolar.
- Medir la salida real como `OUT1 - OUT2`.
- Agregar un filtro LC fisico para suavizar la salida y observar una forma mas
  senoidal.

## Componentes utilizados

- STM32 NUCLEO-G474RE.
- Modulo L298N.
- Protoboard.
- Jumpers.
- Fuente DC externa.
- Analog Discovery 2 con WaveForms.
- Resistencia de carga de `10 ohm / 10 W`.
- Inductor de `270 uH`.
- Capacitores electroliticos usados en pares espalda con espalda.

## Idea general del inversor

Un inversor toma una alimentacion DC y la conmuta mediante un puente H para
obtener una salida alterna. En esta prueba, la alimentacion DC entra al L298N
por su entrada de potencia y la salida alterna aparece entre `OUT1` y `OUT2`.

La STM32 no entrega potencia a la carga. La STM32 solo entrega senales de
control. La potencia sale de la fuente externa conectada al L298N.

El funcionamiento conceptual es:

```text
 Fuente DC de potencia
        |
        v
     L298N
  puente H interno
        |
        v
  OUT1 - OUT2
 salida alterna
```

Y el control es:

```text
 STM32
  |-- A5 / PC0 -> ENA -> PWM/SPWM
  |-- D7 / PA8 -> IN1 -> polaridad
  |-- D8 / PA9 -> IN2 -> polaridad inversa
```

## Pines usados

### STM32 NUCLEO-G474RE

| Pin fisico Nucleo | Pin MCU | Funcion en el proyecto |
| --- | --- | --- |
| `A5` | `PC0` | Salida PWM/SPWM hacia `ENA` del L298N |
| `D7` | `PA8` | Entrada `IN1` del L298N |
| `D8` | `PA9` | Entrada `IN2` del L298N |
| `GND` | GND | Tierra comun con el L298N y el Analog Discovery |
| `5V` | 5 V USB/Nucleo | Alimentacion logica del modulo L298N durante la prueba |

### Modulo L298N

| Pin del L298N | Conexion usada | Funcion |
| --- | --- | --- |
| `A Enable` / `ENA` | `A5 / PC0` | Habilita el puente A con PWM/SPWM |
| `IN1` | `D7 / PA8` | Selecciona una polaridad del puente |
| `IN2` | `D8 / PA9` | Selecciona la polaridad contraria |
| `OUT1` | Salida hacia carga/filtro | Nodo de salida del puente A |
| `OUT2` | Salida hacia carga/filtro | Nodo complementario de salida del puente A |
| `+5V Power` | 5 V de la STM32 en la prueba | Alimentacion logica del modulo |
| `Power GND` | GND comun | Referencia comun |
| `+12V Power` | Fuente externa de potencia | Entrada DC del puente H. En pruebas se uso 5 V |

## Alimentacion del L298N

El L298N tiene dos alimentaciones conceptuales diferentes:

1. Alimentacion logica (`VSS`):
   - Alimenta la parte interna de control del L298N.
   - En el modulo aparece como `+5V Power`.
   - En esta prueba se alimento desde el pin `5V` de la STM32/Nucleo.

2. Alimentacion de potencia (`VS`):
   - Es la energia que el puente H entrega a la carga.
   - En el modulo aparece como `+12V Power`.
   - El nombre `+12V` es el nombre del borne del modulo, pero durante las
     pruebas se uso una fuente menor, por ejemplo `5 V`, para no forzar la
     resistencia ni el L298N.

La conexion de tierra es obligatoria:

```text
GND STM32 ---- GND protoboard ---- Power GND L298N ---- GND Analog Discovery
```

Sin tierra comun, el L298N no puede interpretar correctamente las senales de la
STM32.

## Que hace cada senal

### A5 / PC0 -> ENA

`A5` corresponde al pin `PC0` de la STM32. En CubeMX fue configurado como
`TIM1_CH1`, por eso puede generar PWM por hardware.

Esta senal entra al pin `ENA` del L298N. `ENA` funciona como una compuerta
general del puente A:

```text
ENA = 1 -> el puente A puede conducir
ENA = 0 -> el puente A queda apagado
```

Como en `ENA` se aplica PWM/SPWM, esta senal decide cuanto tiempo conducen los
pares internos del puente seleccionados por `IN1` e `IN2`.

En otras palabras:

```text
ENA no decide la direccion.
ENA decide cuanto tiempo se deja pasar energia.
```

### D7 / PA8 -> IN1

`D7` corresponde al pin `PA8`. Esta senal entra a `IN1` del L298N.

`IN1` no genera el SPWM. Su funcion es seleccionar una polaridad del puente H.
En el proyecto cambia cada `10 ms`, es decir, medio periodo de una salida de
`50 Hz`.

### D8 / PA9 -> IN2

`D8` corresponde al pin `PA9`. Esta senal entra a `IN2` del L298N.

`IN2` es complementaria a `IN1`. Cuando `IN1` esta en alto, `IN2` esta en bajo.
Cuando `IN1` esta en bajo, `IN2` esta en alto.

Asi el puente H invierte la polaridad de la salida cada medio ciclo.

## Como invierte la senal el L298N

El L298N contiene internamente un puente H. En el puente A, `IN1` e `IN2`
seleccionan que par interno conduce.

Primer medio ciclo:

```text
IN1 = 1
IN2 = 0
ENA = SPWM
```

Durante los pulsos altos de `ENA`, el puente aplica una polaridad:

```text
OUT1 positivo respecto a OUT2
```

Segundo medio ciclo:

```text
IN1 = 0
IN2 = 1
ENA = SPWM
```

Durante los pulsos altos de `ENA`, el puente aplica la polaridad contraria:

```text
OUT2 positivo respecto a OUT1
```

Por eso, al medir la salida real de la carga, no se debe mirar solo `OUT1`
contra tierra ni solo `OUT2` contra tierra. La salida util del puente H es:

```text
Vout = OUT1 - OUT2
```

En WaveForms se midio asi:

```text
CH1 -> OUT1
CH2 -> OUT2
GND -> GND comun
Math 1 = C1 - C2
```

## Por que OUT1 y OUT2 por separado no muestran voltaje negativo

Cuando se mide cada salida contra GND, ambas senales se ven entre `0 V` y el
voltaje de la fuente de potencia. Eso es normal:

```text
OUT1 contra GND -> 0 V a +V
OUT2 contra GND -> 0 V a +V
```

La carga no esta conectada de una salida a GND. La carga esta conectada entre
`OUT1` y `OUT2`, entonces lo que importa es la diferencia:

```text
OUT1 = +V, OUT2 = 0 V  -> OUT1 - OUT2 = +V
OUT1 = 0 V,  OUT2 = +V -> OUT1 - OUT2 = -V
```

Por eso la salida bipolar aparece al usar `C1 - C2`.

## Diseno del codigo

El codigo propio del proyecto esta principalmente en:

- `Core/Src/main.c`
- `Core/Src/l298n_spwm.c`
- `Core/Inc/l298n_spwm.h`

Los archivos generados por CubeMX configuran reloj, GPIO, TIM1 y BSP de la
tarjeta Nucleo.

### main.c

En `main.c` se inicializan los perifericos generados por CubeMX:

```c
MX_GPIO_Init();
MX_TIM1_Init();
```

Luego se inicializa el modulo de control del L298N:

```c
L298N_SPWM_Init();
L298N_SPWM_SetModulation(L298N_MODULACION_SPWM);
L298N_SPWM_Start();
```

La modulacion se define asi:

```c
#define L298N_MODULACION_SPWM       800U
```

`800U` significa `80%` de la amplitud maxima de la tabla SPWM.

Dentro del ciclo infinito se llama:

```c
L298N_SPWM_Task();
```

Esta funcion actualiza periodicamente el duty del PWM y cambia la polaridad del
puente cada `10 ms`.

### l298n_spwm.c

Este archivo contiene la logica de generacion SPWM.

Parametros principales:

```c
#define L298N_PWM_TIMER              htim1
#define L298N_PWM_CHANNEL            TIM_CHANNEL_1
#define L298N_SPWM_TABLE_SIZE        100U
#define L298N_SPWM_UPDATE_US         200U
#define L298N_OUTPUT_PERIOD_US       20000U
#define L298N_HALF_PERIOD_US         10000U
#define L298N_DEFAULT_MODULATION     800U
```

Interpretacion:

- `TIM1_CH1` genera el PWM en `PC0/A5`.
- La tabla tiene `100` muestras.
- Cada muestra dura `200 us`.
- `100 * 200 us = 20000 us = 20 ms`.
- Un periodo de `20 ms` corresponde a `50 Hz`.
- Cada `10 ms` se invierte la polaridad del puente.

La tabla `spwm_abs_sine_table` contiene una senal senoidal absoluta entre `0` y
`1000`. Se usa absoluta porque la parte positiva y negativa no se generan con
valores negativos en el PWM. La polaridad la hacen `IN1` e `IN2`.

La funcion `SetBridgePolarity()` controla la direccion:

```c
1U -> IN1 alto, IN2 bajo
0U -> IN1 bajo, IN2 alto
```

La funcion `SetDutyPermille()` escribe el duty en el canal PWM de TIM1:

```c
__HAL_TIM_SET_COMPARE(...)
```

La funcion `L298N_SPWM_Task()` hace el trabajo repetitivo:

1. Calcula el tiempo actual.
2. Ubica ese tiempo dentro del periodo de `20 ms`.
3. Selecciona la muestra correspondiente de la tabla.
4. Cambia la polaridad si se pasa de los primeros `10 ms`.
5. Aplica el duty correspondiente al PWM de `ENA`.

## Configuracion de frecuencias

El PWM de `ENA` se configuro alrededor de `20 kHz`. Esta frecuencia es la
portadora. La salida fundamental del inversor es de `50 Hz`.

La relacion conceptual es:

```text
20 kHz -> frecuencia rapida de conmutacion PWM/SPWM
50 Hz  -> frecuencia lenta de salida alterna
```

Por eso en el osciloscopio no siempre se ven bonitas al mismo tiempo. Para ver
la salida del inversor se uso `OUT1 - OUT2`; para ver la componente suavizada se
uso filtro digital y despues filtro LC fisico.

## Medicion con Analog Discovery 2

Conexion usada:

```text
AD2 GND -> GND comun
CH1 1+  -> OUT1
CH2 2+  -> OUT2
```

En WaveForms:

```text
Math 1 = C1 - C2
```

`Math 1` representa la tension real sobre la carga.

Despues se agrego un filtro digital:

```text
Math 2 = filtro low-pass de Math 1
```

El filtro digital no cambia el circuito fisico. Solo ayuda a visualizar la
componente de baja frecuencia.

## Capturas de medicion

### Salida diferencial con filtro digital, sin filtro LC real

Esta captura corresponde a la salida del puente observada como `C1 - C2`, usando
filtro digital en WaveForms pero sin el filtro LC fisico montado en la salida.

![Salida con filtro digital sin filtro LC real](Imagenes/PRUEBA1_CON_FILTRO.png)

### Salida con filtro LC fisico y filtro digital

Esta captura corresponde a la salida despues de implementar el filtro LC real en
la protoboard. La senal naranja es la salida medida y la azul es la version
filtrada digitalmente en WaveForms.

![Salida con filtro LC real](Imagenes/PRUEBA1_CON_FILTRO_DOS.png)

## Filtro LC fisico implementado

El filtro se monto en la salida diferencial del puente H.

Conexion:

```text
OUT1 ---- L 270 uH ---- nodo filtrado ---- carga 10 ohm ---- OUT2
                              |
                              C equivalente
                              |
                            OUT2
```

Es decir:

- El inductor va en serie desde `OUT1`.
- La resistencia de carga va entre el nodo filtrado y `OUT2`.
- El capacitor equivalente va en paralelo con la carga.

La version final probada fue:

```text
L = 270 uH
Carga = 10 ohm / 10 W
C = 47 uF + 47 uF en serie espalda con espalda
Ceq aproximado = 23.5 uF
```

Como los capacitores disponibles eran electroliticos polarizados, se conectaron
dos iguales en serie espalda con espalda para formar un capacitor equivalente no
polarizado.

Conexion de los dos electroliticos:

```text
nodo filtrado ---- +| |- ---- -| |+ ---- OUT2
```

La capacitancia equivalente de dos capacitores iguales en serie es:

```text
Ceq = C / 2
```

Para dos capacitores de `47 uF`:

```text
Ceq = 47 uF / 2 = 23.5 uF
```

La frecuencia de corte aproximada del filtro LC es:

```text
fc = 1 / (2*pi*sqrt(L*C))
```

Con:

```text
L = 270 uH
C = 23.5 uF
```

se obtiene aproximadamente:

```text
fc ~= 2.0 kHz
```

Esta frecuencia deja pasar la componente de `50 Hz` y atenua parte importante
del PWM de `20 kHz`.

## Funcion del filtro

La salida del puente H con SPWM no es una senoidal pura. Es una senal bipolar
compuesta por pulsos rapidos. El filtro LC se usa para reducir la componente de
alta frecuencia y conservar la componente de baja frecuencia.

Antes del filtro:

```text
salida = pulsos SPWM positivos y negativos
```

Despues del filtro:

```text
salida = forma mas suave, cercana a una senoidal
```

El inductor se opone a cambios rapidos de corriente y el capacitor ofrece un
camino para las componentes rapidas de tension. Juntos atenuan la portadora
PWM y dejan visible la componente de 50 Hz.

## Carga utilizada

La carga usada fue:

```text
10 ohm / 10 W
```

Con `5 V`, la corriente ideal aproximada seria:

```text
I = V / R = 5 / 10 = 0.5 A
```

La potencia ideal aproximada seria:

```text
P = V^2 / R = 25 / 10 = 2.5 W
```

En la practica, el L298N tiene caida interna, por lo que la tension real sobre
la carga puede ser menor. El L298N puede calentarse porque no es un driver
eficiente para potencia alta.

## Precauciones importantes

- No conectar alto voltaje en esta etapa.
- No conectar la salida del puente H directamente a GND.
- La carga debe ir entre `OUT1` y `OUT2`.
- El Analog Discovery debe compartir GND con el circuito, pero sus entradas se
  conectan a `OUT1` y `OUT2` por separado.
- Para ver la salida bipolar se usa `C1 - C2`.
- No poner capacitores electroliticos polarizados directamente en una salida
  alterna bipolar.
- Si se usan electroliticos, usar dos iguales espalda con espalda.
- Vigilar temperatura del L298N, resistencia e inductores/capacitores.
- El L298N sirve para pruebas didacticas, pero no es ideal para un inversor de
  potencia final.

## Estado actual

El proyecto quedo en modo normal SPWM:

- `PC0/A5` genera PWM/SPWM hacia `ENA`.
- `PA8/D7` y `PA9/D8` invierten la polaridad cada `10 ms`.
- La salida real se mide como `OUT1 - OUT2`.
- El filtro LC fisico con `270 uH` y `47 uF + 47 uF` suaviza la salida.

La configuracion final probada para visualizar la senoidal fue:

```text
Fuente de potencia: 5 V
Carga: 10 ohm / 10 W
Inductor: 270 uH
Capacitores: 47 uF + 47 uF espalda con espalda
Medicion: Math 1 = C1 - C2
Filtro digital opcional: Math 2 = low-pass de Math 1
```
