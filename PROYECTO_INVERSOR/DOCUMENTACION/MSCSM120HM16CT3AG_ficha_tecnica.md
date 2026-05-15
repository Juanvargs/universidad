# MSCSM120HM16CT3AG - ficha tecnica del puente H

Autor: Juan Pablo Vargas Cordoba  
Universidad Nacional de Colombia  
Fecha de organizacion: 2026-05-15  
Fuente principal: `C:\Users\juanv\Downloads\Microsemi_MSCSM120HM16CT3AG_Full_BridgeSiC_MOSFET_Power_Module_Rv1.0.pdf`  
Fuente web oficial: https://www.microchip.com/en-us/product/mscsm120hm16ct3ag-module

## 1. Proposito del documento

Este documento organiza las caracteristicas tecnicas del modulo `MSCSM120HM16CT3AG`. Su objetivo es servir como ficha de lectura rapida del puente H: que integra, cuales son sus limites, que significa cada parametro importante y que informacion debe tenerse presente antes de disenar el driver, la proteccion y la etapa de potencia.

Este documento no incluye aplicaciones ni implementacion del prototipo. Los calculos de corriente, seleccion de driver, fuente aislada y filtro de salida se presentan en el documento complementario de drivers y filtros.

## 2. Identificacion general del componente

| Caracteristica | Valor | Descripcion corta |
| --- | --- | --- |
| Fabricante | Microsemi / Microchip | Empresa responsable del componente y de la documentacion tecnica. |
| Referencia | `MSCSM120HM16CT3AG` | Codigo exacto del modulo. Es importante usarlo completo para no confundirlo con otros modulos SiC. |
| Tipo de modulo | Puente H completo SiC MOSFET | Integra cuatro MOSFETs de potencia formando una topologia de puente H. |
| Tecnologia | SiC, Silicon Carbide | Semiconductor de carburo de silicio. Permite conmutacion rapida, alta tension y menores perdidas que muchas soluciones de silicio. |
| Clase de tension | 1200 V | Tension maxima de bloqueo de los MOSFETs. Es un limite del componente, no una tension obligatoria de trabajo. |
| Familia mecanica | SP3F | Encapsulado/modulo de potencia con pines de potencia, pines de compuerta y base para montaje termico. |
| Revision del datasheet | Rev. 1.0, enero de 2020 | Version del documento usado como referencia. |

## 3. Estructura interna del puente H

Un puente H completo esta formado por cuatro interruptores de potencia. En este modulo esos interruptores son MOSFETs SiC.

| Parte interna | Descripcion | Que significa |
| --- | --- | --- |
| Rama A | Par de MOSFETs superior e inferior de un lado del puente. | Permite conectar un terminal de salida al bus positivo o al bus negativo. |
| Rama B | Par de MOSFETs superior e inferior del otro lado del puente. | Trabaja junto con la rama A para invertir la polaridad sobre la carga. |
| MOSFET superior, high-side | Transistor conectado hacia el bus DC positivo. | Su fuente no esta fija a tierra; por eso requiere driver flotante o aislado. |
| MOSFET inferior, low-side | Transistor conectado hacia el bus DC negativo o retorno. | Su referencia suele estar mas cerca del retorno de potencia. |
| Salidas del puente | Nodos centrales entre MOSFET superior e inferior de cada rama. | La carga se conecta entre estos dos nodos para obtener tension diferencial. |
| Diodos asociados | Diodos internos o antiparalelo asociados a los interruptores. | Proveen camino para corriente de recirculacion y transitorios cuando la carga es inductiva. |
| NTC interno | Sensor resistivo de temperatura. | Permite estimar la temperatura del modulo desde una etapa de control o proteccion. |

## 4. Limites electricos principales

Los limites absolutos indican valores que no deben superarse. No son valores recomendados de operacion continua.

| Simbolo | Parametro | Valor | Descripcion corta |
| --- | --- | --- | --- |
| `VDSS` | Tension drenador-fuente maxima | 1200 V | Maxima tension que un MOSFET puede bloquear cuando esta apagado. |
| `ID` | Corriente continua a `TC = 25 C` | 173 A | Corriente maxima con la carcasa mantenida a 25 C. |
| `ID` | Corriente continua a `TC = 80 C` | 138 A | Corriente maxima cuando la carcasa esta mas caliente. |
| `IDM` | Corriente pulsada | 350 A | Corriente admisible durante pulsos cortos bajo condiciones del fabricante. |
| `VGS` | Tension gate-source absoluta | -10 V a +25 V | Rango maximo entre compuerta y fuente. Superarlo puede danar el MOSFET. |
| `PD` | Potencia disipada a `TC = 25 C` | 745 W | Capacidad termica teorica con condiciones de refrigeracion especificas. |
| `TJ` | Temperatura de junta | -40 C a 175 C | Rango de temperatura interna del semiconductor. |

## 5. Caracteristicas de conduccion

Estas caracteristicas describen el comportamiento del MOSFET cuando esta encendido o bloqueando.

| Parametro | Condicion | Valor | Descripcion corta |
| --- | --- | --- | --- |
| `RDS(on)` tipica | `VGS = 20 V`, `ID = 80 A`, `TJ = 25 C` | 12.5 mohm | Resistencia equivalente del MOSFET encendido. Mientras menor sea, menor perdida por conduccion. |
| `RDS(on)` maxima | Misma condicion | 16 mohm | Valor conservador para calcular perdidas en conduccion. |
| `RDS(on)` caliente | `TJ = 175 C` | 20 mohm tipico | La resistencia aumenta con temperatura; por eso la disipacion tambien aumenta. |
| `VGS(th)` | `VGS = VDS`, `ID = 2 mA` | 1.8 V a 2.8 V | Tension donde el MOSFET apenas empieza a conducir. No es la tension correcta para manejarlo en potencia. |
| `IDSS` | `VGS = 0 V`, `VDS = 1200 V` | 20 uA tipico, 200 uA max | Corriente de fuga cuando el MOSFET esta apagado y bloqueando alta tension. |
| `IGSS` | `VGS = 20 V`, `VDS = 0 V` | 200 nA max | Corriente de fuga por la compuerta. La compuerta consume poca corriente DC, pero requiere pulsos de carga y descarga. |

## 6. Caracteristicas dinamicas de compuerta

Estas especificaciones indican que tan exigente es manejar la compuerta del MOSFET y que tan rapido puede conmutar.

| Parametro | Condicion | Valor | Descripcion corta |
| --- | --- | --- | --- |
| `Ciss` | `VGS = 0 V`, `VDS = 1000 V`, `f = 1 MHz` | 6040 pF | Capacitancia de entrada vista por el driver. A mayor capacitancia, mas energia debe mover el driver. |
| `Coss` | Misma condicion | 540 pF | Capacitancia drenador-fuente equivalente. Influye en transitorios del nodo de salida. |
| `Crss` | Misma condicion | 50 pF | Capacitancia Miller entre drenador y compuerta. Puede causar encendidos no deseados por alto `dv/dt`. |
| `Qg` | `VGS = -5 V / +20 V`, `VBus = 800 V`, `ID = 80 A` | 464 nC | Carga total de compuerta por conmutacion. Sirve para dimensionar el driver y la fuente aislada. |
| `Qgs` | Misma condicion | 82 nC | Parte de la carga asociada al tramo gate-source. |
| `Qgd` | Misma condicion | 100 nC | Carga Miller. Es critica durante el cambio de tension drenador-fuente. |
| `RGint` | Interna del modulo | 2.94 ohm | Resistencia interna de compuerta. Se suma a la resistencia externa del driver. |

## 7. Tiempos y energia de conmutacion

| Parametro | Condicion de referencia | Valor | Descripcion corta |
| --- | --- | --- | --- |
| `Td(on)` | `VGS = -5 V / +20 V`, `VBus = 600 V`, `ID = 100 A` | 30 ns | Retardo desde la orden de encendido hasta el inicio del encendido. |
| `Tr` | `RGon = 4 ohm`, `RGoff = 2.4 ohm` | 30 ns | Tiempo de subida durante el encendido. |
| `Td(off)` | Misma condicion | 50 ns | Retardo desde la orden de apagado hasta el inicio del apagado. |
| `Tf` | Misma condicion | 25 ns | Tiempo de caida durante el apagado. |
| `Eon` | `TJ = 150 C`, `VBus = 600 V`, `ID = 100 A` | 1.98 mJ | Energia perdida en cada encendido bajo las condiciones del datasheet. |
| `Eoff` | Misma condicion | 1.30 mJ | Energia perdida en cada apagado bajo las condiciones del datasheet. |

## 8. Diodo cuerpo por MOSFET

El diodo cuerpo es el camino de conduccion natural asociado al MOSFET cuando la corriente necesita circular en sentido inverso. En un puente H aparece durante recirculacion de corriente, cambios de polaridad o cargas inductivas.

| Parametro | Condicion | Valor | Descripcion corta |
| --- | --- | --- | --- |
| `VSD` | `VGS = 0 V`, `ISD = 80 A` | 4.0 V | Caida directa del diodo cuerpo. Si conduce durante mucho tiempo, produce perdida `P = VSD * I`. |
| `VSD` | `VGS = -5 V`, `ISD = 80 A` | 4.2 V | Caida directa con polarizacion negativa de compuerta. |
| `trr` | `ISD = 80 A`, `VR = 800 V`, `diF/dt = 2000 A/us` | 90 ns | Tiempo de recuperacion inversa. Mide cuanto tarda el diodo en dejar de conducir al cambiar de estado. |
| `Qrr` | Misma condicion | 1100 nC | Carga de recuperacion inversa. Menor carga implica menos perdida y menos ruido. |
| `Irr` | Misma condicion | 27 A | Pico de corriente durante la recuperacion inversa. |

## 9. Diodo SiC inverso por diodo

En el datasheet aparece una seccion para el diodo SiC inverso. Se refiere al diodo de carburo de silicio conectado en sentido inverso o antiparalelo respecto al camino principal del interruptor. Su funcion es permitir corriente cuando el MOSFET correspondiente no esta conduciendo directamente, especialmente en recirculacion y transitorios.

| Parametro | Condicion | Valor | Descripcion corta |
| --- | --- | --- | --- |
| `VRRM` | Tension inversa repetitiva pico | 1200 V | Maxima tension inversa repetitiva que el diodo puede bloquear. |
| `IRM` | `VR = 1200 V`, `TJ = 25 C` | 20 uA tipico, 400 uA max | Corriente de fuga cuando el diodo esta polarizado en inversa a temperatura ambiente. |
| `IRM` | `VR = 1200 V`, `TJ = 175 C` | 300 uA tipico | La fuga aumenta cuando el modulo esta caliente. |
| `IF` | `TC = 100 C` | 60 A | Corriente directa continua admisible del diodo. |
| `VF` | `IF = 60 A`, `TJ = 25 C` | 1.5 V tipico, 1.8 V max | Caida directa del diodo a temperatura ambiente. |
| `VF` | `IF = 60 A`, `TJ = 175 C` | 2.1 V tipico | Caida directa con el diodo caliente. |
| `Qc` | `VR = 600 V` | 260 nC | Carga capacitiva asociada al diodo. Afecta las perdidas de conmutacion. |
| `C` | `f = 1 MHz`, `VR = 400 V` | 282 pF | Capacitancia del diodo a 400 V. |
| `C` | `f = 1 MHz`, `VR = 800 V` | 210 pF | Capacitancia del diodo a 800 V. |
| `RthJC` | Junta a carcasa | 0.477 C/W | Resistencia termica del diodo hacia la carcasa. |

## 10. Caracteristicas termicas, aislamiento y mecanica

| Caracteristica | Valor | Descripcion corta |
| --- | --- | --- |
| `RthJC` MOSFET | 0.2 C/W | Resistencia termica desde la junta del MOSFET hasta la carcasa. |
| `RthJC` diodo | 0.477 C/W | Resistencia termica desde la junta del diodo hasta la carcasa. |
| Aislamiento terminal-carcasa | 4000 Vrms, 1 min, 50/60 Hz | Aislamiento electrico entre terminales y base/carcasa del modulo. |
| Temperatura de carcasa operativa | -40 C a 125 C | Rango permitido para la carcasa durante operacion. |
| Temperatura de almacenamiento | -40 C a 125 C | Rango permitido sin operacion. |
| Sustrato | AlN, nitruro de aluminio | Material con buena conductividad termica para transferir calor al disipador. |
| Paquete aislado | Si | Facilita el montaje en disipador con aislamiento interno del modulo. |
| Torque de montaje M4 | 2 N.m a 3 N.m | Torque recomendado para fijar el modulo al disipador. |
| Peso | 110 g | Masa aproximada del modulo. |

## 11. Sensor NTC interno

El NTC es una resistencia dependiente de la temperatura. Su resistencia disminuye cuando aumenta la temperatura.

| Parametro | Valor | Descripcion corta |
| --- | --- | --- |
| `R25` | 50 kohm | Resistencia nominal del NTC a 25 C. |
| Tolerancia `R25` | 5 % | Variacion esperada respecto al valor nominal. |
| `B25/85` | 3952 K | Constante beta usada para estimar temperatura entre 25 C y 85 C. |
| Tolerancia de `B` a `TC = 100 C` | 4 % | Error asociado a la constante beta. |

## 12. Lectura tecnica resumida

| Punto | Interpretacion |
| --- | --- |
| El modulo es de alta tension y alta corriente | Sus limites son mucho mayores que los de una prueba inicial de laboratorio. |
| La compuerta no puede manejarse directamente desde un microcontrolador | Requiere un driver de compuerta con corriente suficiente, aislamiento y protecciones. |
| La tecnologia SiC conmuta rapido | Esto reduce perdidas, pero aumenta la importancia del layout, el dead-time y la medicion correcta. |
| La tension de compuerta del datasheet es `+20 V / -5 V` | Esa es una condicion de caracterizacion; el driver real debe respetar sus propios limites. |
| Los diodos son importantes | Aunque el control ideal busque conducir con MOSFETs, los diodos aparecen en recirculacion y transitorios. |
| El NTC debe aprovecharse | Permite agregar proteccion termica y registrar comportamiento del modulo. |
