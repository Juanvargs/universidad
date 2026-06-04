# Proyecto inversor SPWM

Este repositorio reune el avance del inversor SPWM desde la prueba de concepto
con `L298N` hasta el proyecto real con modulo SiC de potencia.

## Mapa de carpetas

| Carpeta | Contenido | Uso recomendado |
| --- | --- | --- |
| `PRUEBA0_SPWM_L298N` | Prueba de concepto con `STM32 NUCLEO-G474RE` y modulo `L298N`. | Referencia para entender la generacion SPWM, la medicion diferencial y el filtro LC inicial. |
| `INVERSOR_SPWM_125K` | Firmware del proyecto real con `STM32F030C8T6`, PWM complementario a `125 kHz`, ADC y control `UCC_DISABLE`. | Desarrollo activo de control para el puente H real. |
| `DOCUMENTACION` | Indices, fichas tecnicas, comparativas, arquitectura y checklists. | Punto de entrada principal para estudiar y continuar el proyecto. |
| `_tmp_pdf_images` | Imagenes extraidas de PDFs durante la preparacion de documentacion. | Material auxiliar; no es firmware ni diseno fuente. |
| `DISEÑO` | Archivos de diseno electronico. | No se modifica desde esta organizacion. |

## Ruta de lectura sugerida

1. `DOCUMENTACION/README.md`
2. `DOCUMENTACION/00_resumen_general.md`
3. `DOCUMENTACION/01_prueba_concepto_l298n.md`
4. `DOCUMENTACION/02_proyecto_real_inversor_125k.md`
5. `DOCUMENTACION/03_componentes_principales.md`
6. `DOCUMENTACION/04_checklist_integracion_validacion.md`

## Evolucion del proyecto

Primero se valido el concepto con una STM32 Nucleo y un modulo `L298N`. Esa
etapa demostro la generacion SPWM, la inversion de polaridad con puente H, la
medicion `OUT1 - OUT2` y el uso de un filtro LC para observar una salida mas
cercana a una senoidal.

Despues se paso al proyecto real: un puente H completo de potencia basado en el
modulo SiC `MSCSM120HM16CT3AG`, manejado con drivers aislados `UCC21540DWR`,
fuentes aisladas con `SN6505BDBVR` y transformador `750342879`, mas medicion de
bus, salida y corriente mediante entradas ADC de la MCU.
