# Referencias utilizadas en el proyecto

Fecha: 2026-06-05  
Proyecto: inversor monofasico SPWM con `STM32F030C8T6` y modulo SiC  
Alcance: referencias tecnicas, datasheets, notas de aplicacion, disenos guia,
componentes seleccionados y componentes evaluados durante el desarrollo.

## 1. Proposito del documento

Este documento concentra las referencias usadas para construir el esquematico,
seleccionar componentes, definir conexiones y justificar el firmware SPWM del
inversor. No reemplaza al BOM ni al esquematico; sirve como indice tecnico para
saber de donde salio cada decision importante.

Las referencias se dividen en:

```text
Disenos guia
Datasheets oficiales
Componentes seleccionados
Componentes evaluados
Firmware y herramientas
Referencias locales del proyecto
```

## 2. Disenos guia y notas de aplicacion

| Referencia | Fuente | Uso en el proyecto | Enlace |
| --- | --- | --- | --- |
| `TIDA-010938` | Texas Instruments | Referencia principal para arquitectura de inversor/PFC, fuentes aisladas, sensado y estructura de potencia. | https://www.ti.com/tool/TIDA-010938 |
| `TIDA-010938` en C2000Ware Digital Power SDK | Texas Instruments | Referencia de software y arquitectura de control DC/AC con H-Bridge, incluyendo modulacion bipolar/unipolar. | https://software-dl.ti.com/C2000/c2000_apps_public_sw/c2000ware_sdk/digitalpowersdk/5_03_00_00/html_guide/TIDA-010938.html |
| `TIDM-HV-1PH-DCAC` | Texas Instruments | Referencia de inversor monofasico DC/AC con filtro de salida y control digital. | https://software-dl.ti.com/C2000/c2000_apps_public_sw/c2000ware_sdk/digitalpowersdk/5_03_00_00/html_guide/TIDM-HV-1PH-DCAC.html |
| `TIDMCG4` BOM de TIDA-010938 | Texas Instruments | Revision del BOM original del TIDA para inductores PFC, ferritas, conectores y componentes de referencia. | https://www.ti.com/lit/zip/tidmcg4 |
| `AN4013` | STMicroelectronics | Referencia para timers STM32, PWM, salidas complementarias y dead-time. | https://www.st.com/resource/en/application_note/dm00042534-timers-and-pwm-generation-using-stm32-microcontrollers-stmicroelectronics.pdf |
| `AN4735` / ejemplos STM32CubeF0 | STMicroelectronics | Referencia de ejemplos de firmware STM32F0 y uso de perifericos HAL. | https://www.st.com/resource/en/application_note/dm00210690-stm32cube-firmware-examples-for-stm32f0-series-stmicroelectronics.pdf |

## 3. Microcontroladores y firmware

| Referencia | Fabricante | Uso | Enlace / ubicacion |
| --- | --- | --- | --- |
| `STM32F030C8T6` / `STM32F030C8T6TR` | STMicroelectronics | MCU del inversor real. Genera SPWM, lee ADC y controla `UCC_DISABLE`. | https://www.st.com/resource/en/datasheet/stm32f030c8.pdf |
| `STM32F030X4 (1).PDF` | STMicroelectronics | Datasheet local consultado para pines, alimentacion y funciones alternas. | `C:\Users\juanv\Downloads\STM32F030X4 (1).PDF` |
| `STM32 NUCLEO-G474RE` | STMicroelectronics | Tarjeta usada en la prueba de concepto con L298N. | https://www.st.com/en/evaluation-tools/nucleo-g474re.html |
| `STM32CubeMX` | STMicroelectronics | Configuracion de pines, `TIM1`, ADC y generacion base del firmware. | Proyecto local: `INVERSOR_SPWM_125K.ioc` |
| `STM32 HAL/CMSIS` | STMicroelectronics / Arm | Librerias generadas para GPIO, ADC, TIM y sistema. | Proyecto local: `INVERSOR_SPWM_125K/Drivers` |

### 3.1 Referencias de firmware creadas en el proyecto

| Archivo | Uso |
| --- | --- |
| `INVERSOR_SPWM_125K/Src/spwm_inverter.c` | Modulo propio para generar SPWM bipolar. |
| `INVERSOR_SPWM_125K/Inc/spwm_inverter.h` | Interfaz publica del modulo SPWM. |
| `DOCUMENTACION/05_firmware_spwm_stm32.md` | Explicacion del firmware SPWM, pines, duty y ADC. |
| `PRUEBA0_SPWM_L298N/Core/Src/l298n_spwm.c` | Referencia previa de estilo: tabla seno, modulo separado y funciones `Init/Start/Stop`. |

## 4. Modulo de potencia y drivers

| Referencia | Fabricante | Uso en el proyecto | Enlace / ubicacion |
| --- | --- | --- | --- |
| `MSCSM120HM16CT3AG` | Microchip / Microsemi | Modulo principal de potencia, puente H SiC. | Datasheet local: `C:\Users\juanv\Downloads\Microsemi_MSCSM120HM16CT3AG_Full_BridgeSiC_MOSFET_Power_Module_Rv1.0.pdf` |
| `MSCSM120HM16CT3AG` en Digi-Key | Digi-Key / Microchip | Referencia de compra registrada en Altium. | https://www.digikey.com/es/products/detail/microchip-technology/MSCSM120HM16CT3AG/12350281 |
| `UCC21540DW` / `UCC21540DWR` | Texas Instruments | Driver aislado doble para manejar high-side y low-side de cada rama. | https://www.ti.com/lit/ds/symlink/ucc21540.pdf |
| `SN6505BDBVR` | Texas Instruments | Driver push-pull para fuentes aisladas de gate. | https://www.ti.com/lit/ds/symlink/sn6505b.pdf |
| `750342879` | Wurth Elektronik | Transformador usado con `SN6505B` para generar fuentes aisladas. | https://www.we-online.com/components/products/datasheet/750342879.pdf |
| `VPT85BD-01A` | iNRCORE / Vanguard | Transformador/fuente aislada registrada en logs de Altium para las fuentes de gate. | Revisar datasheet del proveedor antes de liberar BOM. |

## 5. Sensores y medicion analogica

| Referencia | Fabricante | Uso | Enlace |
| --- | --- | --- | --- |
| `AMC1311BDWVR` | Texas Instruments | Amplificador aislado para medicion de tension/bus. Lados `VDD1/GND1` y `VDD2/GND2` aislados. | https://www.ti.com/lit/ds/symlink/amc1311.pdf |
| `AMC3330QDWERQ1` | Texas Instruments | Amplificador/modulador aislado evaluado para medicion de tension tipo TIDA. | https://www.ti.com/lit/ds/symlink/amc3330-q1.pdf |
| `ACS724LLCTR-20AB-T` | Allegro MicroSystems | Sensor Hall de corriente, rango `+/-20 A`, salida analogica. | https://www.allegromicro.com/-/media/files/datasheets/acs724-datasheet.pdf |
| `CC6937S8-5FB040` | CrossChip / proveedor LCSC | Sensor Hall de corriente evaluado/usado para corriente de bus. | Referencia LCSC: https://www.lcsc.com/product-detail/C41421928.html |

### 5.1 Senales ADC asociadas

| Senal | Referencia asociada |
| --- | --- |
| `ADC_VDC_BUS_1`, `ADC_VDC_BUS_2` | Salida diferencial del aislamiento de tension, por ejemplo `AMC1311`. |
| `ADC_IDC_BUS` | Corriente de bus, sensor Hall tipo `CC6937` o equivalente. |
| `ADC_IOUT`, `ADC_IOUT_FILTER` | Corriente de salida, sensor Hall tipo `ACS724`. |
| `ADC_VOUT_1`, `ADC_VOUT_2` | Medicion de salida AC o punto diferencial del filtro. |

## 6. Magneticos, inductores y filtro

| Referencia | Fabricante | Uso / decision | Enlace |
| --- | --- | --- | --- |
| `PH9455.405NL` | Pulse Electronics | Inductor PFC seleccionado por el usuario para el proyecto. | Revisar datasheet de Pulse antes de compra final. |
| `Bourns 145453` | Bourns | Inductor PFC original identificado en BOM AC/PFC del TIDA-010938: `87 uH`, `35 A`. No fue el seleccionado final. | BOM TI `TIDMCG4`: https://www.ti.com/lit/zip/tidmcg4 |
| `Bourns 145451` | Bourns | Inductor PFC identificado en BOM DC/DCE del TIDA: `120 uH`. Referencia, no seleccion final. | BOM TI `TIDMCG4`: https://www.ti.com/lit/zip/tidmcg4 |
| `FCH1365-101M` | FANGCHENG | Inductor de `100 uH` evaluado para lograr `200 uH` usando dos en serie. | Referencia LCSC registrada en busqueda del proyecto. |
| `TAI-TECH TMPC1206HP-101MG-D` | TAI-TECH | Alternativa recomendada de `100 uH` para dos en serie, por margen de saturacion. | https://www.lcsc.com/product-detail/Power-Inductors_TAI-TECH_C357256.html |
| `XRCD75-4R0K/201K` | Xiangru / LCSC | Componente evaluado y descartado para `200 uH > 3 A`; la seccion de `200 uH` no cumple corriente suficiente. | https://www.lcsc.com/product-image/C51484129.html |

## 7. Pasivos y protecciones destacadas

| Referencia / familia | Uso |
| --- | --- |
| `MMSZ5250B` | Zener de clamp para fuentes aisladas de gate alrededor de `20 V`. |
| `SS16` | Diodo Schottky `60 V`, `1 A`, usado en rectificacion de fuentes aisladas. |
| `MH1608-601Y` | Ferrita Bourns usada como referencia en el TIDA y en filtrado de alimentaciones/senales. |
| Capacitores film `630 V` o mayor | Usados/evaluados en filtro de potencia y bus, por ejemplo reemplazo de `1 uF` con varios capacitores en paralelo. |
| Resistencias de gate `RGon = 4 ohm`, `RGoff = 2.4 ohm` | Valores iniciales derivados del datasheet del modulo SiC y capacidad del `UCC21540`. |
| `RGS = 10k` | Pull-down gate-source para estado seguro del MOSFET. |
| `RDT = 20k`, `CDT = 1 nF` | Red de dead-time para `UCC21540`, objetivo aproximado `200 ns`. |

## 8. Referencias de compra registradas

Estas referencias aparecen en logs/ECO de Altium o fueron consultadas durante el
proyecto. Deben verificarse contra el BOM final antes de comprar.

| Componente | Distribuidor / referencia |
| --- | --- |
| `STM32F030C8T6TR` | LCSC `C23922`: https://www.lcsc.com/product-detail/ST-Microelectronics_STMicroelectronics-STM32F030C8T6_C23922.html |
| `SN6505BDBVR` | LCSC `C74518`: https://www.lcsc.com/product-detail/C74518.html |
| `CC6937S8-5FB040` | LCSC `C41421928`: https://www.lcsc.com/product-detail/C41421928.html |
| `AMC1311BDWVR` | LCSC `C783598`: https://www.lcsc.com/product-detail/C783598.html |
| `ACS724LLCTR-20AB-T` | LCSC `C99738`: https://www.lcsc.com/product-detail/Current-Sensors_Allegro-MicroSystems-LLC-ACS724LLCTR-20AB-T_C99738.html |
| `AMC3330QDWERQ1` | LCSC `C5214212`: https://www.lcsc.com/product-detail/C5214212.html |
| `UCC21540DW` | Digi-Key: https://www.digikey.com/es/products/detail/texas-instruments/UCC21540DW/9860890 |
| `MSCSM120HM16CT3AG` | Digi-Key `150-MSCSM120HM16CT3AG-ND`: https://www.digikey.com/es/products/detail/microchip-technology/MSCSM120HM16CT3AG/12350281 |

## 9. Referencias documentales locales

| Documento local | Contenido |
| --- | --- |
| `DOCUMENTACION/00_resumen_general.md` | Resumen del proyecto, bloques y estado. |
| `DOCUMENTACION/01_prueba_concepto_l298n.md` | Prueba de concepto con `STM32G474RE`, `L298N`, filtro LC y mediciones. |
| `DOCUMENTACION/02_proyecto_real_inversor_125k.md` | Arquitectura del inversor real, pines, ADC y potencia. |
| `DOCUMENTACION/03_componentes_principales.md` | Lista comentada de componentes principales. |
| `DOCUMENTACION/04_checklist_integracion_validacion.md` | Validacion en banco y checklist de integracion. |
| `DOCUMENTACION/05_firmware_spwm_stm32.md` | Firmware SPWM, `TIM1`, duty bipolar y futura lectura ADC. |
| `DOCUMENTACION/Microinverter_application_note.pdf` | Nota de aplicacion complementaria sobre microinversores. |
| `DOCUMENTACION/MSCSM120HM16CT3AG_ficha_tecnica.md` | Ficha tecnica organizada del modulo SiC. |
| `DOCUMENTACION/MSCSM120HM16CT3AG_drivers_filtros_referencias.md` | Driver, fuente aislada, calculos de gate y referencias TI. |
| `DOCUMENTACION/registro_cambios_diseno_inversor.md` | Cambios realizados frente a la plantilla TIDA. |

## 10. Referencias descartadas o solo evaluadas

| Referencia | Motivo |
| --- | --- |
| `STM32F030C8T6` para USB nativo | El micro no tiene periferico USB nativo; PA11/PA12 no se usan como USB en este proyecto. |
| `XRCD75-4R0K/201K` como inductor de `200 uH > 3 A` | La parte de `200 uH` no cumple la corriente requerida. |
| Una sola fuente aislada high-side para ambos brazos | No sirve para puente H completo porque `AC_A` y `AC_B` son nodos switch distintos. |
| Dos high-side compartiendo `GND_HS` | Conectaria indirectamente los nodos switch de ambas ramas; no es correcto. |
| `PA11` como `PWM_A_L` | No corresponde a `TIM1_CH1N`; se corrigio a `PA7`. |

## 11. Notas de uso

- Este documento es un indice tecnico, no un BOM congelado.
- Antes de comprar componentes, verificar disponibilidad, encapsulado, huella y
  rango electrico en el BOM final de Altium.
- Para componentes de potencia, confirmar datasheet oficial del fabricante,
  no solo pagina de distribuidor.
- Para sensores analogicos, confirmar escala real con el divisor, offset y
  alimentacion usados en el esquematico.
- Para firmware, la referencia actual es `05_firmware_spwm_stm32.md` y el codigo
  local `INVERSOR_SPWM_125K/Src/spwm_inverter.c`.
