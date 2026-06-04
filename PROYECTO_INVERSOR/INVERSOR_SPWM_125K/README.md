# INVERSOR_SPWM_125K

Firmware base del proyecto real del inversor SPWM. Esta carpeta contiene el
proyecto STM32CubeMX/CMake para una `STM32F030C8T6`.

## Resumen tecnico

| Elemento | Valor |
| --- | --- |
| MCU | `STM32F030C8T6` |
| Clock | `48 MHz` |
| PWM | `TIM1` con `CH1/CH1N` y `CH2/CH2N` |
| Frecuencia PWM | `125 kHz` |
| Fundamental SPWM | `60 Hz` |
| Modulacion inicial | `80 %` |
| Salida de seguridad | `UCC_DISABLE` en `PA10` |
| ADC | `PA0` a `PA6` |

## Archivos propios

| Archivo | Funcion |
| --- | --- |
| `Src/main.c` | Inicializa HAL, reloj, GPIO, TIM1, ADC y arranca SPWM con potencia deshabilitada. |
| `Src/spwm_inverter.c` | Genera SPWM complementaria mediante acumulador de fase y tabla Q15. |
| `Inc/spwm_inverter.h` | API del modulo de control SPWM. |
| `INVERSOR_SPWM_125K.ioc` | Configuracion de pines y perifericos. |

## Senales principales

| Señal | Pin | Funcion |
| --- | --- | --- |
| `PWM_A_H` | `PA8` | High-side rama A. |
| `PWM_A_L` | `PA7` | Low-side rama A. |
| `PWM_B_H` | `PA9` | High-side rama B. |
| `PWM_B_L` | `PB0` | Low-side rama B. |
| `UCC_DISABLE` | `PA10` | Deshabilitacion de drivers. |

## ADC

| Señal | Pin |
| --- | --- |
| `ADC_VDC_BUS_1` | `PA0` |
| `ADC_IDC_BUS` | `PA1` |
| `ADC_VOUT_1` | `PA2` |
| `ADC_IOUT_FILTER` | `PA3` |
| `ADC_IOUT` | `PA4` |
| `ADC_VDC_BUS_2` | `PA5` |
| `ADC_VOUT_2` | `PA6` |

## Documentacion relacionada

Leer primero:

- `../DOCUMENTACION/02_proyecto_real_inversor_125k.md`
- `../DOCUMENTACION/03_componentes_principales.md`
- `../DOCUMENTACION/04_checklist_integracion_validacion.md`

Nota de seguridad: el firmware arranca la PWM pero mantiene deshabilitada la
etapa de potencia. Validar con osciloscopio antes de habilitar drivers.
