# Prueba de concepto: STM32G474RE con L298N

## Objetivo de la prueba

Validar en baja potencia el principio del inversor SPWM: generar una senal PWM
con duty senoidal, invertir la polaridad de un puente H y medir la salida
diferencial filtrada.

## Bloques usados

| Bloque | Componente | Funcion |
| --- | --- | --- |
| Control | `STM32 NUCLEO-G474RE` | Genera SPWM y senales de polaridad. |
| Puente H | Modulo `L298N` | Conmuta la carga en baja potencia. |
| Medicion | `Analog Discovery 2` con WaveForms | Mide `OUT1`, `OUT2` y calcula `OUT1 - OUT2`. |
| Carga | Resistencia `10 ohm / 10 W` | Carga de prueba. |
| Filtro | Inductor `270 uH` y capacitores electroliticos espalda con espalda | Atenua la portadora PWM. |
| Alimentacion | Fuente DC externa y 5 V de Nucleo | Potencia y logica del modulo. |

## Firmware usado

| Archivo | Papel |
| --- | --- |
| `PRUEBA0_SPWM_L298N/Core/Src/main.c` | Inicializacion de perifericos y arranque de la tarea SPWM. |
| `PRUEBA0_SPWM_L298N/Core/Src/l298n_spwm.c` | Logica de tabla senoidal, duty y cambio de polaridad. |
| `PRUEBA0_SPWM_L298N/Core/Inc/l298n_spwm.h` | API del modulo SPWM. |
| `PRUEBA0_SPWM_L298N/PRUEBA0_SPWM_L298N.ioc` | Configuracion STM32CubeMX. |

## Pines principales

| Pin Nucleo | Pin MCU | Conexion | Funcion |
| --- | --- | --- | --- |
| `A5` | `PC0` | `ENA` del L298N | SPWM por `TIM1_CH1`. |
| `D7` | `PA8` | `IN1` del L298N | Polaridad del puente. |
| `D8` | `PA9` | `IN2` del L298N | Polaridad complementaria. |
| `GND` | GND | GND comun | Referencia comun para STM32, L298N, fuente y AD2. |
| `5V` | 5 V | Logica L298N | Alimentacion logica del modulo. |

## Parametros de control

| Parametro | Valor |
| --- | --- |
| Frecuencia PWM | Aproximadamente `20 kHz` |
| Frecuencia fundamental | `50 Hz` |
| Periodo fundamental | `20 ms` |
| Cambio de polaridad | Cada `10 ms` |
| Muestras de tabla | `100` |
| Tiempo por muestra | `200 us` |
| Modulacion inicial | `800 permille` = `80 %` |

## Funcionamiento

La STM32 envia la SPWM a `ENA`. Cuando `ENA` esta en alto, el L298N entrega
potencia con la polaridad seleccionada por `IN1` e `IN2`. Durante un semiciclo
`IN1 = 1` e `IN2 = 0`; durante el siguiente semiciclo se invierten.

La salida util se mide como:

```text
Vout = OUT1 - OUT2
```

Medir solo `OUT1` contra GND o `OUT2` contra GND no muestra directamente la
tension real sobre la carga, porque la carga esta entre ambas salidas.

## Filtro LC de prueba

```text
OUT1 ---- L 270 uH ---- nodo filtrado ---- carga 10 ohm ---- OUT2
                              |
                         C equivalente
                              |
                            OUT2
```

Se usaron dos capacitores electroliticos de `47 uF` en serie espalda con
espalda, obteniendo un equivalente aproximado de `23.5 uF`. Con `270 uH`, la
frecuencia de corte queda alrededor de `2 kHz`, suficiente para conservar la
componente de `50 Hz` y atenuar parte de la portadora de `20 kHz`.

## Resultado tecnico

La prueba valido:

- Generacion SPWM desde la STM32.
- Cambio de polaridad del puente H.
- Medicion diferencial correcta.
- Uso de filtro LC para observar una forma de onda mas suave.
- Separacion entre control logico y potencia de la carga.

## Valor para el proyecto real

Esta etapa deja demostrado el metodo de control, pero el `L298N` no es apto como
etapa final de potencia. El proyecto real reemplaza el puente integrado por
drivers aislados y un modulo SiC de puente H.
