# PRUEBA0_SPWM_L298N

Prueba de concepto del inversor SPWM con `STM32 NUCLEO-G474RE` y modulo
`L298N`. Esta etapa valida el principio de control antes de pasar al puente H
SiC del proyecto real.

## Que se valido

- Generacion SPWM por hardware en `TIM1_CH1`.
- Control de polaridad del puente H con `IN1` e `IN2`.
- Medicion diferencial `OUT1 - OUT2`.
- Filtro LC fisico para reducir la portadora PWM.
- Uso de `Analog Discovery 2` y WaveForms para observar la salida.

## Componentes principales

| Componente | Uso |
| --- | --- |
| `STM32 NUCLEO-G474RE` | Control SPWM. |
| Modulo `L298N` | Puente H de baja potencia. |
| Fuente DC externa | Alimentacion de potencia del puente. |
| `Analog Discovery 2` | Medicion de senales y diferencial. |
| Resistencia `10 ohm / 10 W` | Carga de prueba. |
| Inductor `270 uH` | Filtro de salida. |
| 2 x capacitores `47 uF` | Capacitor equivalente no polarizado de `23.5 uF`. |

## Pines usados

| Pin Nucleo | Pin MCU | L298N | Funcion |
| --- | --- | --- | --- |
| `A5` | `PC0` | `ENA` | SPWM. |
| `D7` | `PA8` | `IN1` | Polaridad. |
| `D8` | `PA9` | `IN2` | Polaridad complementaria. |
| `GND` | GND | `Power GND` | Referencia comun. |
| `5V` | 5 V | `+5V Power` | Alimentacion logica del L298N. |

## Documentacion relacionada

- `../DOCUMENTACION/01_prueba_concepto_l298n.md`: documento completo con calculos, resultados y graficas de WaveForms.
- `../DOCUMENTACION/03_componentes_principales.md`

Este archivo queda como entrada rapida a la prueba. La documentacion completa se
centralizo en `../DOCUMENTACION/01_prueba_concepto_l298n.md`.
