# Driver y fuente aislada para el puente H MSCSM120HM16CT3AG

Autor: Juan Pablo Vargas Cordoba  
Universidad Nacional de Colombia  
Fecha de organizacion: 2026-05-15  
Componentes estudiados: `UCC21540DWR` y `SN6505BDBVR`  
Fuentes principales: datasheets oficiales de Texas Instruments para `UCC21540` y `SN6505B`

## 1. Objetivo del documento

Este documento presenta las especificaciones tecnicas del driver de compuerta `UCC21540DWR` y del controlador de transformador `SN6505BDBVR`. La idea es identificar, con valores numericos, que puede entregar cada componente, que corriente consume, que corriente puede suministrar en su salida y como se relacionan con el puente H `MSCSM120HM16CT3AG`.

La corriente de prueba del puente H se fija inicialmente en `3 A`. Esa corriente corresponde a la etapa de potencia. No debe confundirse con la corriente de compuerta ni con la corriente de alimentacion de los integrados.

## 2. Resumen electrico directo

| Componente | Alimentacion principal | Corriente de alimentacion | Corriente de salida | Que significa |
| --- | --- | --- | --- | --- |
| `UCC21540DWR` | `VCCI = 3 V a 5.5 V`; `VDDA/VDDB = 9.2 V a 18 V` para la version `UCC21540` | `IVCCI` reposo: 1.5 mA tip, 2.0 mA max. `IVDDA/IVDDB` reposo: 1.0 mA tip, 2.5 mA max por canal. | 4 A source y 6 A sink pico por salida. | Alimenta la logica y entrega pulsos fuertes de corriente para cargar/descargar compuertas. No entrega la corriente de la carga. |
| `SN6505BDBVR` | `VCC = 2.25 V a 5.5 V` | `I(VCC)` para `SN6505B`: 1.56 mA tip, 2.3 mA max con `RL = 50 ohm`. En apagado: 0.1 uA tip. | Corriente recomendada por interruptor `D1/D2`: 1 A si `2.8 V < VCC < 5.5 V`; 0.75 A si `2.25 V < VCC < 2.8 V`. | Excita el primario de un transformador. La corriente DC aislada final depende del transformador, rectificador, capacitores, regulador y carga. |

## 3. Diferencia entre corriente de potencia, corriente de compuerta y corriente de fuente

| Corriente | Donde circula | Componente relacionado | Descripcion |
| --- | --- | --- | --- |
| Corriente de potencia | Bus DC, MOSFETs del puente H y carga | `MSCSM120HM16CT3AG` | Es la corriente objetivo de la prueba: `3 A`. |
| Corriente de compuerta | Entre salida del driver y gate-source de cada MOSFET | `UCC21540DWR` | Son pulsos de corriente altos pero cortos. Sirven para cargar y descargar la capacitancia de compuerta. |
| Corriente de alimentacion del driver | Fuentes `VCCI`, `VDDA` y `VDDB` | `UCC21540DWR` | Mantiene funcionando la logica aislada y la etapa de salida del driver. |
| Corriente del primario del transformador | Pines `D1`, `D2`, `VCC` y `GND` | `SN6505BDBVR` | Es la corriente que conmuta el integrado para transferir energia por el transformador. |
| Corriente DC aislada disponible | Salida despues de transformador, diodos y capacitores | Fuente aislada completa | No la define solo el `SN6505BDBVR`; depende del diseno completo de la fuente. |

## 4. UCC21540DWR - identificacion

| Especificacion | Valor | Descripcion |
| --- | --- | --- |
| Fabricante | Texas Instruments | Proveedor del driver y del datasheet oficial. |
| Referencia usada | `UCC21540DWR` | Variante en empaque `DWR`, usada para compra en carrete. |
| Familia | `UCC21540`, `UCC21540A`, `UCC21541`, `UCC21542` | Familia de drivers aislados de dos canales. |
| Funcion | Driver aislado de compuerta | Convierte senales logicas en senales de compuerta con mayor corriente. |
| Numero de canales | 2 | Cada integrado puede manejar dos MOSFETs. |
| Cantidad para puente H completo | 2 integrados | Un puente H tiene cuatro MOSFETs. |
| Encapsulado | SOIC `DW` de 16 pines | Paquete ancho con aislamiento reforzado. |
| Tipo de aislamiento | Aislamiento reforzado | Separa electricamente el lado de control del lado de potencia. |
| Uso previsto | MOSFET, IGBT y GaN | En este proyecto se usa para MOSFETs SiC del modulo de potencia. |

## 5. UCC21540DWR - pines y funcion

| Pin / grupo | Funcion | Descripcion |
| --- | --- | --- |
| `VCCI` | Alimentacion del lado logico | Alimenta la entrada del driver, compatible con logica de microcontrolador. |
| `GND` | Referencia del lado logico | Tierra del lado de control. |
| `INA`, `INB` | Entradas logicas | Reciben las senales PWM desde la STM32. |
| `DISABLE` | Entrada de apagado | Deshabilita simultaneamente las salidas; util para proteccion por falla. |
| `DT` | Programacion de dead-time | Permite fijar tiempo muerto mediante resistencia. |
| `VDDA`, `VDDB` | Alimentacion de salida | Alimentan la etapa de salida de cada canal. |
| `VSSA`, `VSSB` | Referencia de salida | Referencia local de cada salida; en high-side puede ser flotante. |
| `OUTA`, `OUTB` | Salidas de compuerta | Entregan corriente de source/sink hacia la compuerta del MOSFET. |

## 6. UCC21540DWR - alimentacion y corrientes

| Parametro | Condicion | Min | Tip | Max | Unidad | Descripcion |
| --- | --- | --- | --- | --- | --- | --- |
| `VCCI` | Alimentacion logica recomendada | 3 | - | 5.5 | V | Alimenta el lado de entrada. Es compatible con logica de 3.3 V de la STM32. |
| `VDDA`, `VDDB` | Alimentacion de salida recomendada para `UCC21540` | 9.2 | - | 18 | V | Alimenta cada salida de compuerta. La version `UCC21540` tiene UVLO de 8 V. |
| `IVCCI` | Corriente de reposo, `INA = INB = 0 V` | - | 1.5 | 2.0 | mA | Corriente que consume el lado logico sin conmutar. |
| `IVDDA`, `IVDDB` | Corriente de reposo por canal, `INA = INB = 0 V` | - | 1.0 | 2.5 | mA | Corriente que consume cada lado de salida sin conmutar. |
| `IVCCI` | Corriente de operacion por canal, `f = 500 kHz`, `50 %` duty | - | 3.0 | 3.5 | mA | Corriente del lado logico cuando conmuta. |
| `IVDDA`, `IVDDB` | Corriente de operacion por canal, `f = 500 kHz`, `50 %` duty, `CL = 100 pF` | - | 2.5 | 4.2 | mA | Corriente del lado de salida con carga capacitiva pequena. |
| `PD` | Disipacion total, condiciones de prueba TI | - | - | 950 | mW | Potencia maxima de referencia para el encapsulado bajo condiciones especificas. |
| `PDI` | Disipacion lado transmisor | - | - | 50 | mW | Potencia asociada al lado logico. |
| `PDA`, `PDB` | Disipacion por lado driver | - | - | 450 | mW | Potencia asociada a cada canal de salida. |

Nota: la corriente real de `VDDA/VDDB` aumenta cuando se conecta una compuerta grande. Para calcular esa parte se usa la carga total de compuerta:

```text
Igate_promedio = Qg * fsw
Pgate = Qg * Vdrive * fsw
```

## 7. UCC21540DWR - salida de compuerta

| Parametro | Condicion | Valor | Unidad | Descripcion |
| --- | --- | --- | --- | --- |
| Corriente pico source `IOA+`, `IOB+` | `CVDD = 10 uF`, `CLOAD = 0.18 uF`, `f = 1 kHz` | 4 | A | Corriente pico que el driver puede entregar para cargar la compuerta. |
| Corriente pico sink `IOA-`, `IOB-` | Misma condicion | 6 | A | Corriente pico que el driver puede absorber para descargar la compuerta. |
| Resistencia de salida alta `ROH` | `IOUT = -10 mA` | 5 | ohm | Resistencia equivalente cuando la salida esta alta. No representa completamente la corriente pico. |
| Resistencia de salida baja `ROL` | `IOUT = 10 mA` | 0.55 | ohm | Resistencia equivalente cuando la salida esta baja. |
| `VOH` | `VDDA/VDDB = 15 V`, `IOUT = -10 mA` | 14.95 | V | Nivel alto de salida cercano a `VDD`. |
| `VOL` | `VDDA/VDDB = 15 V`, `IOUT = 10 mA` | 5.5 | mV | Nivel bajo de salida cercano a `VSS`. |
| Active pull-down | `VDDA/VDDB` sin alimentar, `IOUT = 200 mA` | 1.6 a 2 | V | Ayuda a mantener la compuerta descargada cuando el driver no esta alimentado. |

## 8. UCC21540DWR - entradas logicas y UVLO

| Parametro | Min | Tip | Max | Unidad | Descripcion |
| --- | --- | --- | --- | --- | --- |
| `VINH` para `INA`, `INB`, `DISABLE` | 1.2 | 1.8 | 2.0 | V | Tension reconocida como nivel alto. |
| `VINL` para `INA`, `INB`, `DISABLE` | 0.8 | 1.0 | 1.2 | V | Tension reconocida como nivel bajo. |
| Histeresis de entrada | - | 0.8 | - | V | Margen que evita cambios falsos por ruido. |
| `VCCI_ON` | 2.55 | 2.7 | 2.85 | V | Umbral de encendido del lado logico. |
| `VCCI_OFF` | 2.35 | 2.5 | 2.65 | V | Umbral de apagado del lado logico. |
| Histeresis `VCCI` | - | 0.2 | - | V | Diferencia entre encendido y apagado UVLO. |
| `VDD_ON` para `UCC21540` | 7.7 | 8.5 | 8.9 | V | Umbral de encendido de la alimentacion de salida. |
| `VDD_OFF` para `UCC21540` | 7.2 | 7.9 | 8.4 | V | Umbral de apagado de la alimentacion de salida. |
| Histeresis `VDD` | - | 0.6 | - | V | Evita oscilacion cerca del umbral UVLO. |

## 9. UCC21540DWR - tiempos de conmutacion

| Parametro | Condicion | Min | Tip | Max | Unidad | Descripcion |
| --- | --- | --- | --- | --- | --- | --- |
| `tRISE` | `COUT = 1.8 nF`, `VDD = 12 V` | - | 5 | 16 | ns | Tiempo de subida de la salida. |
| `tFALL` | `COUT = 1.8 nF`, `VDD = 12 V` | - | 6 | 12 | ns | Tiempo de bajada de la salida. |
| `tPWmin` | Pulso minimo que pasa a salida | - | 20 | - | ns | Pulsos menores pueden no reflejarse en la salida. |
| `tPDHL` | Propagacion en flanco de bajada | 26 | 33 | 45 | ns | Retardo entre entrada y salida al apagar. |
| `tPDLH` | Propagacion en flanco de subida | 26 | 33 | 45 | ns | Retardo entre entrada y salida al encender. |
| `tPWD` | Distorsion de ancho de pulso | - | - | 6 | ns | Diferencia entre retardo de subida y bajada. |
| `tDM` | Matching de retardo entre canales | - | - | 5 | ns | Diferencia de propagacion entre canales en rango normal. |
| `tVCCI+ to OUT` | Retardo desde UVLO de `VCCI` hasta salida | - | 50 | - | us | Tiempo de habilitacion despues de que sube `VCCI`. |
| `tVDD+ to OUT` | Retardo desde UVLO de `VDD` hasta salida | - | 10 | - | us | Tiempo de habilitacion despues de que sube `VDD`. |
| CMTI alto `CMH` | `VCM = 1000 V` | - | 125 | - | V/ns | Inmunidad ante transitorios de modo comun con salida alta. |
| CMTI bajo `CML` | `VCM = 1000 V` | - | 125 | - | V/ns | Inmunidad ante transitorios de modo comun con salida baja. |

## 10. UCC21540DWR - dead-time

| Configuracion `DT` | Dead-time minimo | Dead-time tipico | Dead-time maximo | Unidad | Descripcion |
| --- | --- | --- | --- | --- | --- |
| `DT` conectado a `VCCI` | - | Determinado por entradas | - | - | No se programa dead-time interno. |
| `RDT = 10 kOhm` | 80 | 100 | 120 | ns | Tiempo muerto corto. |
| `RDT = 20 kOhm` | 160 | 200 | 240 | ns | Tiempo muerto medio. |
| `RDT = 50 kOhm` | 400 | 500 | 600 | ns | Tiempo muerto largo. |

## 11. UCC21540DWR - aislamiento, termica y encapsulado

| Parametro | Valor | Unidad | Descripcion |
| --- | --- | --- | --- |
| Clearance externo | > 8 | mm | Distancia minima por aire entre pines de lados aislados. |
| Creepage externo | > 8 | mm | Distancia minima sobre la superficie del encapsulado. |
| `VIORM` | 1414 | Vpk | Tension repetitiva pico maxima de aislamiento. |
| `VIOWM` AC | 1000 | Vrms | Tension de trabajo AC maxima de aislamiento. |
| `VIOWM` DC | 1414 | Vdc | Tension de trabajo DC maxima de aislamiento. |
| `VIMP` | 7692 | Vpk | Tension de impulso maxima. |
| `VIOTM` | 8000 | Vpk | Tension transitoria maxima de aislamiento. |
| `VIOSM` | 10000 | Vpk | Tension de surge maxima. |
| `VISO` | 5700 | Vrms | Aislamiento soportado durante 60 s segun UL 1577. |
| Capacitancia de barrera `CIO` | 1.2 | pF | Capacitancia entre entrada y salida. Menor valor reduce acoplamiento de ruido. |
| Resistencia de aislamiento `RIO` a 25 C | > 10^12 | ohm | Resistencia entre entrada y salida. |
| `RthetaJA` para `DW` | 69.8 | C/W | Resistencia termica junta-ambiente del encapsulado `DW`. |
| `RthetaJC(top)` para `DW` | 33.1 | C/W | Resistencia termica junta-carcasa superior. |
| `RthetaJB` para `DW` | 36.9 | C/W | Resistencia termica junta-tarjeta. |
| Temperatura de junta recomendada | -40 a 150 | C | Rango funcional de junta. |

## 12. SN6505BDBVR - identificacion

| Especificacion | Valor | Descripcion |
| --- | --- | --- |
| Fabricante | Texas Instruments | Proveedor del integrado y del datasheet oficial. |
| Referencia | `SN6505BDBVR` | Version `SN6505B` en encapsulado `DBV`, empaque carrete. |
| Tipo | Driver push-pull para transformador | Conmuta dos MOSFETs internos para excitar un transformador con tap central. |
| Frecuencia nominal | 420 kHz | Frecuencia interna de la version `B`. |
| Funcion en el proyecto | Fuente aislada auxiliar | Genera, junto con transformador y rectificacion, la alimentacion aislada del driver de compuerta. |
| Encapsulado | SOT-23 `DBV` de 6 pines | Encapsulado pequeno para montaje superficial. |

## 13. SN6505BDBVR - pines y funcion

| Pin | Tipo | Funcion | Descripcion |
| --- | --- | --- | --- |
| `D1` | Salida | Drenador abierto del primer MOSFET interno | Se conecta a un extremo del primario del transformador. |
| `VCC` | Alimentacion | Entrada de alimentacion | Debe desacoplarse con capacitor de bajo ESR, minimo 4.7 uF recomendado. |
| `D2` | Salida | Drenador abierto del segundo MOSFET interno | Se conecta al otro extremo del primario del transformador. |
| `GND` | Potencia | Retorno de corriente | Debe conectarse con baja inductancia porque por ahi circula corriente de conmutacion. |
| `EN` | Entrada | Habilitacion | Si se pone en bajo o flotante, apaga el integrado. Si no se usa, conectarlo a `VCC`. |
| `CLK` | Entrada | Reloj externo opcional | Si no detecta reloj valido, usa el oscilador interno. |

## 14. SN6505BDBVR - alimentacion y corrientes

| Parametro | Condicion | Min | Tip | Max | Unidad | Descripcion |
| --- | --- | --- | --- | --- | --- | --- |
| `VCC` | Alimentacion recomendada | 2.25 | - | 5.5 | V | Alimentacion del integrado. Para este proyecto se recomienda usar 5 V. |
| `I(VCC)` | `SN6505B`, `2.8 V < VCC < 5.5 V`, `RL = 50 ohm` | - | 1.56 | 2.3 | mA | Corriente propia de alimentacion del integrado en operacion. |
| `IDIS` | `EN = 0` | - | 0.1 | - | uA | Corriente de alimentacion cuando el integrado esta apagado. |
| `IIH` | Fuga en `EN` y `CLK`, `EN/CLK = VCC` | - | 10 | 20 | uA | Corriente que entra por las entradas de control. |
| `ILKG(D1/D2)` | `EN = 0`, `D1/D2 = VCC` | - | 0.1 | - | uA | Fuga de las salidas cuando el integrado esta deshabilitado. |
| Corriente recomendada `D1/D2` | `2.8 V < VCC < 5.5 V` | - | - | 1 | A | Corriente de los interruptores internos en el primario del transformador. |
| Corriente recomendada `D1/D2` | `2.25 V < VCC < 2.8 V` | - | - | 0.75 | A | Corriente permitida cuando se alimenta con menor tension. |
| Corriente pico absoluta `D1/D2` | Valor absoluto maximo | - | - | 2.4 | A | Limite de esfuerzo; no debe usarse como corriente normal de diseno. |
| Limite de corriente `ILIM` | `2.8 V < VCC < 5.5 V` | 1.42 | 1.75 | 2.15 | A | Clamp interno de corriente de los MOSFETs de salida. |
| Limite de corriente `ILIM` | `2.25 V < VCC < 2.8 V` | - | 0.65 | 1.85 | A | Clamp cuando se alimenta con tension baja. |

## 15. SN6505BDBVR - etapa de salida y frecuencia

| Parametro | Condicion | Min | Tip | Max | Unidad | Descripcion |
| --- | --- | --- | --- | --- | --- | --- |
| `FSW` | `SN6505B`, `RL = 50 ohm` a `VCC` | 363 | 424 | 517 | kHz | Frecuencia promedio de conmutacion de `D1` y `D2`. |
| `FEXT` | Reloj externo para `SN6505B` | 100 | - | 1600 | kHz | Rango de frecuencia externa permitida en `CLK`. |
| `RON` | `VCC = 4.5 V`, `ID1/ID2 = 1 A` | - | 0.16 | 0.25 | ohm | Resistencia de los MOSFETs internos. |
| `RON` | `VCC = 2.8 V`, `ID1/ID2 = 1 A` | - | 0.19 | 0.31 | ohm | La resistencia aumenta al bajar la alimentacion. |
| `RON` | `VCC = 2.25 V`, `ID1/ID2 = 0.5 A` | - | 0.21 | 0.45 | ohm | Condicion de menor alimentacion. |
| `V(SLEWHF)` | Slew-rate de tension en `D1/D2`, `SN6505B` | - | 152 | - | V/us | Velocidad de cambio de tension de salida. |
| `I(SLEWHF)` | Slew-rate de corriente en `D1/D2`, `SN6505B` | - | 41 | - | A/us | Velocidad de cambio de corriente en el primario. |
| Mismatch promedio `D1/D2` | `RL = 50 ohm` | - | 0 | - | % | Diferencia promedio de tiempo activo entre las dos salidas. |

## 16. SN6505BDBVR - entradas, UVLO y protecciones

| Parametro | Min | Tip | Max | Unidad | Descripcion |
| --- | --- | --- | --- | --- | --- |
| `VCC+` UVLO | - | 2.25 | - | V | Umbral de encendido por alimentacion suficiente. |
| `VCC-` UVLO | - | 1.7 | - | V | Umbral de apagado por baja alimentacion. |
| Histeresis UVLO | - | 0.3 | - | V | Diferencia entre encendido y apagado. |
| `VIN(ON)` | - | 0.7 * `VCC` | - | V | Nivel alto para `EN` o `CLK`. |
| `VIN(OFF)` | - | 0.3 * `VCC` | - | V | Nivel bajo para `EN` o `CLK`. |
| Histeresis entrada | - | 0.2 * `VCC` | - | V | Evita disparos falsos en entradas. |
| Temperatura apagado `TSD+` | 154 | 168 | 181 | C | Temperatura donde se activa proteccion termica. |
| Temperatura rearme `TSD-` | 135 | 150 | 166 | C | Temperatura donde sale de apagado termico. |
| Histeresis termica | - | 17 | - | C | Separacion entre apagado y rearme termico. |

## 17. SN6505BDBVR - tiempos

| Parametro | Min | Tip / Nom | Max | Unidad | Descripcion |
| --- | --- | --- | --- | --- | --- |
| Cambio a reloj interno por reloj externo invalido | 10 | 25 | - | us | Tiempo despues del cual usa oscilador interno si `CLK` no es valido. |
| Break-before-make `SN6505B` | - | 90 | - | ns | Tiempo que evita que ambos MOSFETs internos conduzcan simultaneamente. |
| Soft-start `tSS` | 1 | 4.25 | 8 | ms | Tiempo de rampa de salida con transformador y carga definidos. |
| Retardo de soft-start `tSSdelay` | 3.5 | 8.5 | 18 | ms | Retardo desde energizacion hasta alcanzar la rampa de salida. |

## 18. SN6505BDBVR - limites absolutos y termica

| Parametro | Min | Max | Unidad | Descripcion |
| --- | --- | --- | --- | --- |
| `VCC` absoluto | -0.5 | 6 | V | Limite absoluto de alimentacion. |
| `EN`, `CLK` | -0.5 | `VCC + 0.5` | V | Limites absolutos de entradas logicas. |
| Tension en `D1`, `D2` | - | 16 | V | Limite absoluto de las salidas de drenador abierto. |
| Corriente pico `D1`, `D2` | - | 2.4 | A | Limite absoluto de corriente pico de interruptores internos. |
| Temperatura de junta | -55 | 150 | C | Rango absoluto de junta. |
| Temperatura de almacenamiento | -65 | 150 | C | Rango de almacenamiento. |
| `RthetaJA` | - | 137.7 | C/W | Resistencia termica junta-ambiente. |
| `RthetaJC(top)` | - | 57.7 | C/W | Resistencia termica junta-carcasa superior. |
| `RthetaJB` | - | 46.0 | C/W | Resistencia termica junta-tarjeta. |
| `psiJT` | - | 13.4 | C/W | Parametro de caracterizacion junta-top. |
| `psiJB` | - | 44.9 | C/W | Parametro de caracterizacion junta-board. |

## 19. Calculos para implementacion inicial con corriente de potencia de 3 A

### 19.1 Perdidas de conduccion en el modulo de potencia

La corriente de `3 A` circula por el puente H, no por el driver. Para un MOSFET del modulo:

```text
Pcond_MOSFET = I^2 * RDS(on)
```

| Caso | Calculo | Resultado | Descripcion |
| --- | --- | --- | --- |
| `RDS(on)` tipico a 25 C | `3^2 * 0.0125` | 0.1125 W | Perdida por un MOSFET conduciendo. |
| `RDS(on)` maximo a 25 C | `3^2 * 0.016` | 0.144 W | Caso conservador a temperatura baja. |
| `RDS(on)` tipico a 175 C | `3^2 * 0.020` | 0.180 W | Perdida aproximada con el modulo caliente. |
| Camino activo de dos MOSFETs, tipico 25 C | `2 * 0.1125` | 0.225 W | En un puente H normalmente conducen dos MOSFETs en serie con la carga. |

### 19.2 Corriente pico de compuerta usando UCC21540DWR

La corriente pico de salida depende del driver y de las resistencias de compuerta:

```text
Igate_on  = Vdrive / (RGon + RGint)
Igate_off = Vdrive / (RGoff + RGint)
```

Usando `Vdrive = 18 V`, `RGon = 4 ohm`, `RGoff = 2.4 ohm` y `RGint = 2.94 ohm`:

| Calculo | Resultado | Comparacion con driver |
| --- | --- | --- |
| `Igate_on = 18 / (4 + 2.94)` | 2.59 A | Menor que 4 A source del `UCC21540DWR`. |
| `Igate_off = 18 / (2.4 + 2.94)` | 3.37 A | Menor que 6 A sink del `UCC21540DWR`. |

### 19.3 Corriente promedio de compuerta

La corriente promedio que debe entregar la fuente de compuerta se aproxima con:

```text
Igate_promedio = Qg * fsw
Pgate = Qg * Vdrive * fsw
```

Con `Qg = 464 nC` por MOSFET y `Vdrive = 18 V`:

| Frecuencia | `Igate_promedio` por MOSFET | `Pgate` por MOSFET | `Pgate` para 4 MOSFETs |
| --- | --- | --- | --- |
| 20 kHz | 9.28 mA | 0.167 W | 0.668 W |
| 50 kHz | 23.2 mA | 0.418 W | 1.672 W |
| 100 kHz | 46.4 mA | 0.835 W | 3.340 W |

Esta corriente promedio se suma al consumo propio del `UCC21540DWR`. La corriente pico puede ser de amperios, pero durante tiempos muy cortos.

### 19.4 Corriente aproximada que debe tomar la fuente aislada desde 5 V

La corriente de entrada aproximada de la fuente aislada puede estimarse con:

```text
Iin_fuente = Pout / (Vin * eficiencia)
```

Ejemplo usando `Vin = 5 V` y una eficiencia estimada de `70 %`:

| Frecuencia | Potencia de compuerta total | Corriente aproximada desde 5 V | Descripcion |
| --- | --- | --- | --- |
| 20 kHz | 0.668 W | `0.668 / (5 * 0.70) = 0.191 A` | Valor base para primera prueba de conmutacion. |
| 50 kHz | 1.672 W | `1.672 / (5 * 0.70) = 0.478 A` | Exige mas margen en transformador y SN6505B. |
| 100 kHz | 3.340 W | `3.340 / (5 * 0.70) = 0.954 A` | Cerca del rango exigente para una fuente pequena. |

Este calculo no reemplaza el diseno completo de la fuente aislada. Sirve para estimar orden de magnitud.

## 20. Decision tecnica inicial

| Punto | Decision | Justificacion |
| --- | --- | --- |
| Driver de compuerta | 2 x `UCC21540DWR` | Se necesitan cuatro salidas de compuerta para el puente H completo. |
| Alimentacion logica del driver | 3.3 V o 5 V en `VCCI` | Compatible con la STM32; 3.3 V es suficiente para las entradas. |
| Alimentacion de salida del driver | `+18 V / 0 V` o `+15 V / -3 V` | El driver no debe superar 18 V entre `VDDx` y `VSSx`. |
| Fuente aislada | `SN6505BDBVR` + transformador + rectificacion + filtrado | El SN6505B solo excita el transformador; no es toda la fuente. |
| Corriente de potencia inicial | 3 A | Valor de prueba del puente H; no es la corriente del driver. |
| Corriente pico de compuerta calculada | 2.59 A ON, 3.37 A OFF | Esta dentro de la capacidad de salida del `UCC21540DWR`. |
| Frecuencia inicial sugerida | 20 kHz | Mantiene razonable la potencia de compuerta y la exigencia de la fuente aislada. |
| Proteccion minima | `DISABLE` del driver y corte por sobrecorriente | Una falla no debe depender solo del firmware. |

## 21. Fuentes consultadas

- Texas Instruments, `UCC21540, UCC21540A, UCC21541, UCC21542` datasheet: https://www.ti.com/lit/ds/symlink/ucc21540.pdf
- Texas Instruments, `SN6505A, SN6505B` datasheet: https://www.ti.com/lit/ds/symlink/sn6505b.pdf
