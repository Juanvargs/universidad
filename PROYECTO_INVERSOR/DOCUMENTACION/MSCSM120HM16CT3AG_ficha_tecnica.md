# MSCSM120HM16CT3AG - ficha tecnica organizada

Fecha de organizacion: 2026-05-14  
Fuente principal: `C:\Users\juanv\Downloads\Microsemi_MSCSM120HM16CT3AG_Full_BridgeSiC_MOSFET_Power_Module_Rv1.0.pdf`  
Fuente web oficial: https://www.microchip.com/en-us/product/mscsm120hm16ct3ag-module

## Identificacion

| Parametro | Valor |
| --- | --- |
| Fabricante | Microsemi / Microchip |
| Referencia | MSCSM120HM16CT3AG |
| Tipo | Modulo de potencia SiC MOSFET, puente H completo |
| Familia / encapsulado | SP3F |
| Estado en pagina Microchip | En produccion |
| Tension nominal | 1200 V |
| Corriente continua por MOSFET | 173 A a `TC = 25 C`, 138 A a `TC = 80 C` |
| `RDS(on)` | 12.5 mohm tipico, 16 mohm maximo a `VGS = 20 V`, `ID = 80 A`, `TJ = 25 C` |
| Revision del datasheet | 1.0, enero de 2020 |

## Caracteristicas principales del modulo

- Puente H completo con MOSFETs SiC y diodos SiC.
- Baja resistencia `RDS(on)`.
- Operacion adecuada a alta frecuencia respecto a soluciones de silicio.
- Diodo SiC Schottky con recuperacion inversa muy baja/casi nula.
- Baja inductancia parasita interna.
- Termistor NTC interno para monitoreo de temperatura.
- Sustrato de nitruro de aluminio, `AlN`, para mejorar conduccion termica.
- Encapsulado aislado para montaje directo en disipador.
- Terminales soldables de potencia y senal para montaje en PCB.

## Aplicaciones indicadas por el datasheet

- UPS / fuentes ininterrumpidas.
- Fuentes conmutadas.
- Traccion y motores electricos para EV.
- Convertidores de soldadura.

## Limites absolutos por MOSFET

| Simbolo | Parametro | Valor |
| --- | --- | --- |
| `VDSS` | Tension drenador-fuente | 1200 V |
| `ID` | Corriente continua, `TC = 25 C` | 173 A |
| `ID` | Corriente continua, `TC = 80 C` | 138 A |
| `IDM` | Corriente pulsada | 350 A |
| `VGS` | Tension compuerta-fuente | -10 V a +25 V |
| `RDS(on)` | Resistencia maxima de conduccion | 16 mohm |
| `PD` | Potencia disipada a `TC = 25 C` | 745 W |

Nota critica del datasheet: aunque el dado SiC permite esos valores, la corriente de salida debe limitarse por el tamano/capacidad de los conectores de potencia.

## Caracteristicas electricas por MOSFET

| Parametro | Condicion | Min | Tip | Max | Unidad |
| --- | --- | --- | --- | --- | --- |
| `IDSS` | `VGS = 0 V`, `VDS = 1200 V` | - | 20 | 200 | uA |
| `RDS(on)` | `VGS = 20 V`, `ID = 80 A`, `TJ = 25 C` | - | 12.5 | 16 | mohm |
| `RDS(on)` | `VGS = 20 V`, `ID = 80 A`, `TJ = 175 C` | - | 20 | - | mohm |
| `VGS(th)` | `VGS = VDS`, `ID = 2 mA` | 1.8 | - | 2.8 | V |
| `IGSS` | `VGS = 20 V`, `VDS = 0 V` | - | - | 200 | nA |

## Caracteristicas dinamicas por MOSFET

| Parametro | Condicion | Valor |
| --- | --- | --- |
| `Ciss` | `VGS = 0 V`, `VDS = 1000 V`, `f = 1 MHz` | 6040 pF |
| `Coss` | mismas condiciones | 540 pF |
| `Crss` | mismas condiciones | 50 pF |
| `Qg` | `VGS = -5 V / +20 V`, `VBus = 800 V`, `ID = 80 A` | 464 nC |
| `Qgs` | mismas condiciones | 82 nC |
| `Qgd` | mismas condiciones | 100 nC |
| `Td(on)` | `VGS = -5 V / +20 V`, `VBus = 600 V`, `ID = 100 A` | 30 ns |
| `Tr` | `RGon = 4 ohm`, `RGoff = 2.4 ohm` | 30 ns |
| `Td(off)` | mismas condiciones | 50 ns |
| `Tf` | mismas condiciones | 25 ns |
| `Eon` | `TJ = 150 C`, `VBus = 600 V`, `ID = 100 A` | 1.98 mJ |
| `Eoff` | mismas condiciones | 1.3 mJ |
| `RGint` | Resistencia interna de compuerta | 2.94 ohm |
| `RthJC` | MOSFET, junta a carcasa | 0.2 C/W |

Implicacion para el driver: el datasheet caracteriza el modulo con compuerta `+20 V` en encendido y `-5 V` en apagado. Para implementacion real no conviene manejarlo como un MOSFET comun de baja potencia; se requiere driver aislado de alta CMTI, fuente aislada para cada canal o para cada rama segun arquitectura, y protecciones de corto/desaturacion.

## Diodo cuerpo por MOSFET

| Parametro | Condicion | Valor |
| --- | --- | --- |
| `VSD` | `VGS = 0 V`, `ISD = 80 A` | 4.0 V |
| `VSD` | `VGS = -5 V`, `ISD = 80 A` | 4.2 V |
| `trr` | `ISD = 80 A`, `VGS = -5 V`, `VR = 800 V`, `diF/dt = 2000 A/us` | 90 ns |
| `Qrr` | mismas condiciones | 1100 nC |
| `Irr` | mismas condiciones | 27 A |

## Diodo SiC inverso por diodo

| Parametro | Condicion | Valor |
| --- | --- | --- |
| `VRRM` | Tension inversa repetitiva pico | 1200 V |
| `IRM` | `VR = 1200 V`, `TJ = 25 C` | 20 uA tipico, 400 uA max |
| `IRM` | `VR = 1200 V`, `TJ = 175 C` | 300 uA tipico |
| `IF` | `TC = 100 C` | 60 A |
| `VF` | `IF = 60 A`, `TJ = 25 C` | 1.5 V tip, 1.8 V max |
| `VF` | `IF = 60 A`, `TJ = 175 C` | 2.1 V tip |
| `Qc` | `VR = 600 V` | 260 nC |
| `C` | `f = 1 MHz`, `VR = 400 V` | 282 pF |
| `C` | `f = 1 MHz`, `VR = 800 V` | 210 pF |
| `RthJC` | Diodo, junta a carcasa | 0.477 C/W |

## Termico, aislamiento y mecanica

| Parametro | Valor |
| --- | --- |
| Aislamiento terminal-carcasa | 4000 Vrms, 1 min, 50/60 Hz |
| Rango de temperatura de junta | -40 C a 175 C |
| Temperatura de junta recomendada bajo conmutacion | -40 C a `TJmax - 25 C` |
| Temperatura de almacenamiento | -40 C a 125 C |
| Temperatura de carcasa operativa | -40 C a 125 C |
| Torque de montaje M4 al disipador | 2 N.m a 3 N.m |
| Peso | 110 g |

## Sensor NTC interno

| Parametro | Valor |
| --- | --- |
| `R25` | 50 kohm |
| Tolerancia `R25` | 5 % |
| `B25/85` | 3952 K |
| Tolerancia de `B` a `TC = 100 C` | 4 % |

## Puntos de diseno que se deben conservar

- Usar compuerta `+20 V / -5 V` como punto de partida, porque coincide con las pruebas dinamicas del datasheet.
- No conectar la STM32 directamente al modulo. La STM32 solo debe entregar PWM logico a un sistema de drivers aislados.
- Implementar dead-time por hardware en el temporizador y considerar bloqueo/validacion tambien en el driver.
- Usar proteccion DESAT o deteccion equivalente de corto, UVLO, soft shutdown y active Miller clamp.
- Separar resistencias de encendido y apagado (`RGon`, `RGoff`) para ajustar EMI, sobrepicos y perdidas.
- Disenar bus DC de baja inductancia, capacitores de DC-link muy cercanos al modulo y posible snubber.
- Integrar medicion del NTC para proteccion termica.
- Verificar pinout y distancias de aislamiento directamente en las figuras del datasheet antes de dibujar PCB.

## Datos pendientes antes de diseno final

- Pinout detallado del encapsulado, porque viene como figura en el PDF y no se extrajo de forma confiable como texto.
- Corriente objetivo real del prototipo.
- Tension de bus DC real.
- Potencia y tipo de carga.
- Frecuencia de conmutacion SPWM.
- Tipo de salida: aislada con transformador, carga resistiva/inductiva, motor, o salida AC filtrada.
- Restricciones de laboratorio: fuente HV disponible, disipador, sondas diferenciales, corriente maxima segura.
