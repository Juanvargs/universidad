# MSCSM120HM16CT3AG - referencias de drivers, filtros y aplicaciones

Fecha de busqueda: 2026-05-14  
Objetivo: reunir referencias para pasar de la prueba con L298N a una implementacion real con el modulo SiC full bridge `MSCSM120HM16CT3AG`.

## Resultado rapido de la busqueda

No encontre una nota de aplicacion publica que use exactamente el part-number `MSCSM120HM16CT3AG` en un inversor completo con esquematico de driver y filtro. Si encontre referencias oficiales y comerciales muy cercanas:

- Pagina oficial Microchip del modulo: confirma que esta en produccion y lo identifica como `1200 V`, `12.5 mohm`, `SP3F`, full bridge mSiC MOSFET.
- Nota/placa Analog Devices con modulo Microsemi SiC de 1200 V: usa driver aislado `ADuM4135` o `ADuM4136` y fuente aislada `LT3999`.
- Tarjeta Taraz `GDC-2A4S1`: driver aislado de 4 canales para full bridge, compatible con SiC/IGBT/MOSFET de 1200 V.
- Cissoid `CMT-TIT8243`: referencia de driver SiC para modulos 1200 V de media rama, con `+20/-5 V`, DESAT, UVLO y Miller clamp.
- Simulacion ROHM de inversor full bridge SiC 15 kW: muestra valores de gate drive y filtro/inductor utiles como referencia de escala, no como diseno copiables directamente.

## Referencias encontradas

| Referencia | Aplicacion / topologia | Driver usado | Filtro / magneticos | Ideas utiles para nuestro proyecto |
| --- | --- | --- | --- | --- |
| Microchip `MSCSM120HM16CT3AG` | Modulo full bridge SiC 1200 V SP3F | No especifica driver en la pagina de producto | No especifica filtro | Es la referencia base. El datasheet caracteriza con `+20/-5 V`, `RGon = 4 ohm`, `RGoff = 2.4 ohm`. |
| ADI / Microsemi SiC evaluation board | Half-bridge 1200 V, 50 A, 200 kHz con modulo Microsemi SiC | `ADuM4135` + `LT3999`; la nota AN-2016 usa `ADuM4136` + `LT3999` | En AN-2016 el banco de prueba usa `Cdc = 1.2 mF` y `L = 38 uH` para pruebas de conmutacion/proteccion | Muy relevante para la arquitectura de driver: aislamiento, dead-time, DESAT, fuente aislada y medicion aislada de temperatura. |
| Taraz `GDC-2A4S1` | Driver aislado de 4 switches para full bridge, SMPS y PFC | 4 canales aislados, hasta 1200 Vdc, 3 kVac de aislamiento | No incluye filtro de potencia; se conecta al puente y el filtro se disena aparte | Es la forma mas directa para un puente H completo: 4 salidas de compuerta, dead-time configurable, fault latch, UVLO, Miller clamp. |
| Cissoid `CMT-TIT8243` | Driver half-bridge SiC 1200 V para modulos de potencia | Driver basado en chipset HADES, `+20/-5 V`, DESAT, UVLO, Miller clamp, CMTI alto | No es filtro de salida; enfoque en gate loop de baja inductancia | Buena referencia de requisitos minimos para una rama SiC: alta CMTI, baja inductancia de compuerta, proteccion de corto. Para full bridge se requieren dos ramas. |
| ROHM full-bridge PS inverter 15 kW | Full bridge SiC, `Vin = 400 Vdc`, `200 Vac`, `75 Aac`, `50 Hz`, `fsw = 50 kHz` | `Vd = +18 V`, `Vs = -4 V`, `Rsource = 1 ohm`, `Rsink = 1 ohm` | `L1 = 500 uH` en la salida de simulacion | Referencia de escala para filtro/inductor en un inversor full bridge. No se debe copiar sin recalcular por tension, corriente, carga y frecuencia propias. |
| Toshiba 3-phase inverter SiC 1200 V | Inversor trifasico para motor AC 440 V | Fotocoplador gate driver `TLP5774H`, 6 unidades, `IOP = +/-4 A`, aislamiento 5000 Vrms | No es referencia de filtro LC monofasico; aplica mas a motor | Confirma que en aplicaciones reales se usa driver aislado de varios amperios, no drivers bootstrap simples. |
| Imperix PEB-800-40 | Bloque half-bridge SiC integrado para prototipos | Gate drivers, protecciones, medicion y enfriamiento integrados | Indica que se requieren inductores o filtros externos para completar el convertidor | Refuerza una decision de arquitectura: el puente de potencia y el filtro son subsistemas separados. |

## Enlaces fuente

- Microchip, pagina oficial del modulo: https://www.microchip.com/en-us/product/mscsm120hm16ct3ag-module
- Microchip, datasheet PDF oficial: https://ww1.microchip.com/downloads/en/DeviceDoc/Microsemi_MSCSM120HM16CT3AG_Full_BridgeSiC_MOSFET_Power_Module_Rv1.0.pdf
- Microchip, AN3500 montaje SP1F/SP3F: https://www.microchip.com/en-us/application-notes/an3500
- Microchip, gate drivers mSiC: https://www.microchip.com/en-us/products/power-management/silicon-carbide/gate-drivers
- Analog Devices, Microsemi SiC gate driver eval board: https://wiki.analog.com/resources/eval/eval-microsemi-sic-module
- Analog Devices, AN-2016 con `ADuM4136` + `LT3999`: https://www.analog.com/en/resources/app-notes/an-2016.html
- Taraz `GDC-2A4S1`, 4-channel SiC gate driver: https://www.taraztechnologies.com/product/power-electronics-modules/gate-driver/full-bridge-sic-gate-driver/
- Cissoid `CMT-TIT8243`, 1200 V half-bridge SiC gate driver: https://www.cissoid.com/products/sic-gate-drivers-3/cmt-tit8243a-cmt-tit8243-3
- ROHM full-bridge PS inverter SiC simulation: https://fscdn.rohm.com/en/products/library/spice_circuit/application/pdf/sic_bsm120d12p2c005_dc-ac_full-bridge_ps_inverter_po%3D15kw.pdf
- Toshiba 3-phase inverter using SiC MOSFET: https://toshiba.semicon-storage.com/ap-en/semiconductor/design-development/referencedesign/detail.RD220.html
- Imperix SiC half-bridge module: https://imperix.com/products/power/sic-mosfet-module/

## Requisitos preliminares del driver para este modulo

Para el `MSCSM120HM16CT3AG`, el driver deberia cumplir como minimo:

| Requisito | Motivo |
| --- | --- |
| 4 canales aislados, o 2 drivers half-bridge aislados | El modulo es un puente H completo con cuatro MOSFETs. |
| Salida de compuerta `+20 V / -5 V` | Coincide con las condiciones dinamicas del datasheet. |
| Corriente de pico de varios amperios | `Qg = 464 nC` por MOSFET; conmutar rapido exige cargar/descargar compuerta con baja impedancia. |
| CMTI alto, idealmente `>= 100 kV/us` | SiC conmuta con alto `dv/dt`; se requiere inmunidad para evitar disparos falsos. |
| UVLO en alimentacion positiva y negativa | Evita operar el MOSFET en zona lineal por falta de tension de compuerta. |
| DESAT o proteccion de corto equivalente | El corto en modulos SiC debe despejarse muy rapido. |
| Soft shutdown | Reduce sobrepicos durante fallas. |
| Active Miller clamp | Evita encendido parasitario por `dv/dt`. |
| Dead-time configurable | Evita conduccion cruzada en cada rama. |
| Resistencias `RGon` y `RGoff` separadas | Permite balancear perdidas, EMI y sobrepicos. |
| Fuente aislada por canal o por switch flotante | Bootstrap simple no es buena primera opcion para una etapa SiC de alta tension y baja frecuencia de salida. |

## Filtro de salida: guia inicial

Para un inversor monofasico SPWM con puente H, el filtro tipico es:

```text
Vdc -> puente H SiC -> L serie -> nodo de salida -> C a retorno/carga -> carga
```

Tambien puede usarse un filtro `LCL` si se conecta a red o a un transformador, pero para una primera implementacion controlada en laboratorio conviene empezar con `LC` y carga conocida.

Variables que faltan para dimensionar el filtro:

- `Vdc`: tension del bus DC.
- `Vac_rms`: tension AC objetivo.
- `Pout`: potencia objetivo.
- `fout`: frecuencia fundamental, probablemente 50 Hz o 60 Hz.
- `fsw`: frecuencia SPWM.
- Rizado de corriente admisible.
- Tipo de carga y si habra transformador.
- Frecuencia de corte deseada del filtro.

Regla de partida:

- La frecuencia de corte del filtro debe estar bastante por encima de la fundamental y bastante por debajo de `fsw`.
- Para una salida de 50/60 Hz, una zona inicial comun de estudio es `fc` entre unas centenas de Hz y pocos kHz, segun carga y control.
- El valor de `L` se fija por rizado de corriente y capacidad de corriente.
- El valor de `C` se fija por `fc`, corriente reactiva permitida y estabilidad con la carga.

Referencia de escala, no valor final: la simulacion ROHM de un full bridge SiC de 15 kW usa `Vin = 400 Vdc`, `200 Vac`, `75 Aac`, `fsw = 50 kHz`, gate drive `+18/-4 V`, y `L1 = 500 uH`.

## Decision preliminar recomendada

Para avanzar desde la prueba L298N a la etapa real:

1. Mantener la STM32G474 como generador SPWM y dead-time por hardware.
2. Pasar las salidas de la STM32 a un sistema de drivers aislados de 4 canales.
3. Arrancar con bus DC bajo y corriente limitada, aunque el modulo sea de 1200 V.
4. Disenar primero una placa/interfaz de driver y sensado, no conectar el modulo directo al microcontrolador.
5. Usar `+20/-5 V` de compuerta, proteccion DESAT/UVLO/Miller clamp, NTC y fault hacia la STM32.
6. Dimensionar el filtro LC cuando se definan `Vdc`, `Vac`, `Pout`, `fsw` y carga.
