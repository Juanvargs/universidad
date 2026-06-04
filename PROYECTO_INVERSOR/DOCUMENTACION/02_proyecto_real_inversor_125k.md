# Proyecto real: INVERSOR_SPWM_125K

## Objetivo

Implementar el control del inversor real con una `STM32F030C8T6`, PWM
complementario, drivers aislados de compuerta, modulo SiC de puente H y entradas
ADC para medicion de tension y corriente.

## Bloques principales

```text
STM32F030C8T6
  -> TIM1 CH1/CH1N y CH2/CH2N
  -> UCC21540DWR rama A y rama B
  -> red de gate del modulo SiC
  -> MSCSM120HM16CT3AG

STM32F030C8T6
  <- ADC de bus DC, salida e intensidad

Fuente auxiliar
  -> SN6505BDBVR + transformador 750342879
  -> 18 V aislados para drivers
```

## MCU y firmware

| Elemento | Valor |
| --- | --- |
| MCU | `STM32F030C8T6` |
| Familia | `STM32F0` |
| Clock del sistema | `48 MHz` |
| Proyecto CubeMX | `INVERSOR_SPWM_125K.ioc` |
| Firmware propio | `Src/spwm_inverter.c` y `Inc/spwm_inverter.h` |
| Timer de PWM | `TIM1` |
| Frecuencia PWM | `125 kHz` |
| Frecuencia fundamental | `60 Hz` |
| Tabla SPWM | `256` muestras Q15 |
| Modulacion inicial | `800 permille` |
| Modulacion maxima en firmware | `950 permille` |

El control usa acumulador de fase de 32 bits. En cada evento de actualizacion de
`TIM1`, se calcula una muestra senoidal y se aplican comparadores
complementarios:

```text
TIM1_CH1  -> PWM_A_H
TIM1_CH1N -> PWM_A_L
TIM1_CH2  -> PWM_B_H
TIM1_CH2N -> PWM_B_L
```

## Pines de control

| Senal | Pin MCU | Funcion |
| --- | --- | --- |
| `PWM_A_H` | `PA8` | PWM high-side rama A, `TIM1_CH1`. |
| `PWM_A_L` | `PA7` | PWM low-side rama A, `TIM1_CH1N`. |
| `PWM_B_H` | `PA9` | PWM high-side rama B, `TIM1_CH2`. |
| `PWM_B_L` | `PB0` | PWM low-side rama B, `TIM1_CH2N`. |
| `UCC_DISABLE` | `PA10` | Deshabilita drivers; en firmware inicia en alto. |
| `SWDIO` | `PA13` | Programacion/debug. |
| `SWCLK` | `PA14` | Programacion/debug. |

`UCC_DISABLE` se maneja como proteccion inicial:

```text
GPIO_PIN_SET   -> drivers deshabilitados
GPIO_PIN_RESET -> drivers habilitados
```

El firmware arranca la PWM con los drivers deshabilitados. La llamada a
`SPWM_Inverter_EnablePowerStage()` queda intencionalmente fuera del arranque
automatico hasta validar las senales con osciloscopio.

## Entradas ADC

| Senal | Pin MCU | Canal ADC | Uso esperado |
| --- | --- | --- | --- |
| `ADC_VDC_BUS_1` | `PA0` | `ADC_IN0` | Medicion del bus DC, punto 1. |
| `ADC_IDC_BUS` | `PA1` | `ADC_IN1` | Corriente del bus DC. |
| `ADC_VOUT_1` | `PA2` | `ADC_IN2` | Tension de salida, punto 1. |
| `ADC_IOUT_FILTER` | `PA3` | `ADC_IN3` | Corriente despues del filtro o rama filtrada. |
| `ADC_IOUT` | `PA4` | `ADC_IN4` | Corriente de salida. |
| `ADC_VDC_BUS_2` | `PA5` | `ADC_IN5` | Medicion del bus DC, punto 2. |
| `ADC_VOUT_2` | `PA6` | `ADC_IN6` | Tension de salida, punto 2. |

Nota: los nombres de sensores estan definidos en el firmware, pero las
referencias exactas de parte de los sensores de corriente y voltaje no aparecen
en los archivos revisados fuera de la carpeta `DISEÑO`. Deben confirmarse en el
esquematico/BOM.

## Etapa de potencia

| Bloque | Componente | Decision actual |
| --- | --- | --- |
| Puente H SiC | `MSCSM120HM16CT3AG` | Modulo principal de potencia. |
| Driver de compuerta | `UCC21540DWR` | Un driver por rama del puente H. |
| Alimentacion aislada de gate | `SN6505BDBVR` + `750342879` | Fuente auxiliar objetivo alrededor de `18 V`. |
| Rectificadores fuente | `SS16` | Schottky `60 V`, `1 A`. |
| Clamp fuente | `MMSZ5250B` | Zener `20 V` como proteccion, no regulador permanente. |
| Red de gate | `RGon = 4 ohm`, `RGoff = 2.4 ohm`, `RGS = 10k`, `Cgs = DNP` | Punto inicial segun datasheet y calculos. |
| Dead-time driver | `RDT = 20k`, `CDT = 1 nF` | Aproximadamente `200 ns` en `UCC21540`. |

## Diferencia frente a la prueba L298N

| Aspecto | Prueba L298N | Proyecto real |
| --- | --- | --- |
| Puente H | Integrado `L298N` | Modulo SiC `MSCSM120HM16CT3AG`. |
| Control de puente | `ENA`, `IN1`, `IN2` | Cuatro PWM complementarios. |
| MCU | `STM32G474RE` Nucleo | `STM32F030C8T6` en proyecto real. |
| Frecuencia fundamental | `50 Hz` | `60 Hz`. |
| Portadora | `20 kHz` aprox. | `125 kHz`. |
| Medicion | Analog Discovery externo | Entradas ADC preparadas en firmware. |
| Potencia | Baja potencia didactica | Etapa de potencia real con aislamiento y protecciones. |

## Pendientes de firmware

- Implementar lectura periodica ADC.
- Convertir cuentas ADC a unidades fisicas segun escalas de sensores.
- Definir umbrales de sobrecorriente, sobretension y subtension.
- Integrar apagado por falla usando `UCC_DISABLE`.
- Registrar estado de fallas y condiciones de arranque.
- Confirmar si el dead-time de `TIM1` y el dead-time del `UCC21540` se suman de
  forma aceptable para el hardware final.
