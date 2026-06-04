# Documentacion organizada del inversor

Esta carpeta es el punto de entrada para estudiar el proyecto sin tener que
buscar informacion dispersa en el firmware, imagenes y documentos tecnicos.

## Indice principal

| Documento | Proposito |
| --- | --- |
| `00_resumen_general.md` | Vision completa del proyecto, etapas y carpetas. |
| `01_prueba_concepto_l298n.md` | Documento completo de la prueba con `STM32G474RE`, `L298N`, calculos, resultados y graficas de WaveForms. |
| `02_proyecto_real_inversor_125k.md` | Arquitectura del inversor real, firmware, pines y bloques de potencia. |
| `03_componentes_principales.md` | Componentes usados en la prueba de concepto y en el proyecto real. |
| `04_checklist_integracion_validacion.md` | Lista de verificacion para continuar el montaje y validarlo en banco. |
| `MSCSM120HM16CT3AG_ficha_tecnica.md` | Ficha tecnica organizada del modulo SiC de puente H. |
| `MSCSM120HM16CT3AG_drivers_filtros_referencias.md` | Driver de compuerta, fuente aislada y calculos de referencia. |
| `registro_cambios_diseno_inversor.md` | Cambios de diseno frente a la plantilla TIDA y decisiones tecnicas. |

## Estado de organizacion

- `DISEÑO` no fue modificado.
- El firmware no fue reestructurado ni editado.
- La informacion nueva se agrego como documentacion navegable.
- Los sensores de corriente y voltaje quedan documentados por sus senales ADC;
  las referencias exactas de parte deben confirmarse en el esquematico/BOM.

## Lectura rapida

Para una revision corta, leer primero `00_resumen_general.md` y
`03_componentes_principales.md`. Para continuar implementacion y pruebas,
seguir con `02_proyecto_real_inversor_125k.md` y
`04_checklist_integracion_validacion.md`.
