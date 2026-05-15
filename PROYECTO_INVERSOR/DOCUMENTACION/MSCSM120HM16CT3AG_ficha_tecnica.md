# MSCSM120HM16CT3AG - ficha tecnica explicada

Fecha de organizacion: 2026-05-14  
Fuente principal: `C:\Users\juanv\Downloads\Microsemi_MSCSM120HM16CT3AG_Full_BridgeSiC_MOSFET_Power_Module_Rv1.0.pdf`  
Fuente web oficial: https://www.microchip.com/en-us/product/mscsm120hm16ct3ag-module  
Contexto del proyecto: primera implementacion real con corriente limitada a `3 A`; no se usara inicialmente el limite maximo de `1200 V` del modulo.

## 1. Identificacion del componente

| Especificacion | Valor | Que significa | Importancia para el proyecto |
| --- | --- | --- | --- |
| Fabricante | Microsemi / Microchip | Empresa que fabrica y documenta el modulo. | Permite buscar datasheets, notas de aplicacion y soporte oficial. |
| Referencia | `MSCSM120HM16CT3AG` | Codigo exacto del modulo de potencia. | Evita confundirlo con otros modulos SiC parecidos. |
| Tipo | Modulo SiC MOSFET de puente H completo | Integra cuatro MOSFETs SiC formando un puente H. | Sirve para un inversor monofasico sin tener que armar el puente con transistores discretos. |
| Tecnologia | Silicon Carbide, SiC | Semiconductor de banda ancha con baja perdida y conmutacion rapida. | Permite trabajar a mayor frecuencia que soluciones de silicio, pero exige buen driver y layout. |
| Familia / encapsulado | SP3F | Formato mecanico/electrico del modulo. | Define montaje, pines, distancias, disipador y PCB. |
| Estado en pagina Microchip | En produccion | El fabricante aun lo lista como producto activo. | Es mejor para comprar y documentar que un componente descontinuado. |
| Tension nominal | 1200 V | Maxima clase de tension del modulo. | No obliga a usar 1200 V; para el prototipo se puede empezar con bus bajo y seguro. |
| Corriente continua por MOSFET | 173 A a `TC = 25 C`; 138 A a `TC = 80 C` | Corriente maxima por transistor bajo condiciones termicas especificas. | Nuestro objetivo inicial de `3 A` queda muy por debajo, asi que hay gran margen. |
| `RDS(on)` | 12.5 mohm tipico; 16 mohm max a `VGS = 20 V`, `ID = 80 A`, `TJ = 25 C` | Resistencia del MOSFET cuando esta encendido. | A menor `RDS(on)`, menor perdida por conduccion; a `3 A` la perdida sera muy baja. |
| Revision del datasheet | 1.0, enero de 2020 | Version del documento consultado. | Ayuda a saber de donde salen los datos y si en el futuro hay que comparar con una revision nueva. |

## 2. Caracteristicas principales del modulo

| Caracteristica | Que significa | Importancia para el proyecto |
| --- | --- | --- |
| MOSFETs de potencia SiC | Los interruptores principales son de carburo de silicio. | Permite conmutacion rapida, pero hace mas criticos el driver, el dead-time y el ruido. |
| Baja `RDS(on)` | Baja resistencia en conduccion. | Reduce calentamiento; en la prueba de `3 A` las perdidas de conduccion seran pequenas. |
| Alto rendimiento a temperatura | El SiC mantiene buen comportamiento a temperaturas elevadas. | Da margen termico, aunque igual se debe monitorear el NTC. |
| Diodos SiC Schottky | Diodos rapidos asociados al modulo. | Mejoran el comportamiento durante recirculacion de corriente y transitorios. |
| Recuperacion inversa muy baja | El diodo genera menos carga al cambiar de conduccion a bloqueo. | Reduce perdidas y picos de corriente durante la conmutacion. |
| Baja inductancia parasita | El encapsulado esta pensado para reducir inductancias internas. | Ayuda a reducir sobrepicos, pero el layout externo sigue siendo decisivo. |
| Termistor NTC interno | Sensor resistivo que cambia con temperatura. | Debe conectarse a la STM32 o a proteccion analogica para apagar por sobretemperatura. |
| Sustrato `AlN` | Nitruro de aluminio con buena conductividad termica. | Facilita extraer calor hacia el disipador. |
| Paquete aislado | El modulo puede montarse en disipador con aislamiento electrico interno. | Simplifica montaje mecanico y seguridad respecto al disipador. |
| Terminales soldables | Pines de potencia y senal preparados para PCB. | Se debe disenar una placa con buen cobre, baja inductancia y distancias adecuadas. |

## 3. Aplicaciones indicadas por el datasheet

| Aplicacion | Que significa | Relacion con el proyecto |
| --- | --- | --- |
| UPS / fuentes ininterrumpidas | Conversion DC/AC para respaldo de energia. | Muy cercano a un inversor monofasico con salida filtrada. |
| Fuentes conmutadas | Conversion eficiente de potencia usando alta frecuencia. | Refuerza que el modulo esta pensado para conmutacion rapida. |
| Traccion EV y motores | Control de motores de alta potencia. | Muestra que el modulo soporta entornos exigentes, aunque nuestro prototipo sera mucho menor. |
| Convertidores de soldadura | Convertidores de alta corriente y alta dinamica. | Indica robustez en aplicaciones de potencia pulsante. |

## 4. Limites absolutos por MOSFET

Estos son valores que no se deben superar. No son valores recomendados de operacion continua para el laboratorio.

| Simbolo | Especificacion | Valor | Que significa | Importancia para el proyecto |
| --- | --- | --- | --- | --- |
| `VDSS` | Tension drenador-fuente | 1200 V | Maxima tension que puede bloquear un MOSFET apagado. | Nuestro bus inicial debe ser mucho menor; el margen de tension sera amplio. |
| `ID` | Corriente continua a `TC = 25 C` | 173 A | Corriente maxima con carcasa a 25 C. | A `3 A` estamos a menos del 2 % de este valor. |
| `ID` | Corriente continua a `TC = 80 C` | 138 A | Corriente maxima cuando el modulo ya esta caliente. | Incluso caliente, `3 A` esta muy por debajo del limite. |
| `IDM` | Corriente pulsada | 350 A | Corriente maxima de pulsos cortos. | No debe usarse como objetivo; solo muestra capacidad ante transitorios. |
| `VGS` | Tension compuerta-fuente | -10 V a +25 V | Rango absoluto entre gate y source. | Nuestro driver debe mantenerse dentro de ese rango siempre. |
| `RDS(on)` | Resistencia maxima de conduccion | 16 mohm | Maximo esperado en condiciones del datasheet. | Sirve para calcular perdidas conservadoras. |
| `PD` | Potencia disipada a `TC = 25 C` | 745 W | Potencia termica maxima bajo condiciones ideales de carcasa. | No se debe tomar como potencia de trabajo; depende del disipador real. |

Nota critica del datasheet: aunque el dado SiC permite corrientes altas, la corriente de salida debe limitarse por el tamano/capacidad de los conectores de potencia. Para este proyecto el primer limite sera `3 A`, impuesto por seguridad y validacion.

## 5. Caracteristicas electricas por MOSFET

| Parametro | Condicion | Min | Tip | Max | Unidad | Que significa | Importancia para el proyecto |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `IDSS` | `VGS = 0 V`, `VDS = 1200 V` | - | 20 | 200 | uA | Corriente de fuga con el MOSFET apagado. | Mientras menor sea, menor perdida y calentamiento en bloqueo. |
| `RDS(on)` | `VGS = 20 V`, `ID = 80 A`, `TJ = 25 C` | - | 12.5 | 16 | mohm | Resistencia en conduccion a temperatura ambiente. | A `3 A`, un MOSFET disipa aprox. `0.1125 W` tipico. |
| `RDS(on)` | `VGS = 20 V`, `ID = 80 A`, `TJ = 175 C` | - | 20 | - | mohm | Resistencia en conduccion cuando el MOSFET esta caliente. | La resistencia sube con temperatura; aun asi a `3 A` la perdida sigue baja. |
| `VGS(th)` | `VGS = VDS`, `ID = 2 mA` | 1.8 | - | 2.8 | V | Tension donde el MOSFET apenas empieza a conducir. | No es tension de manejo; se requiere mucho mas que el umbral para conducir bien. |
| `IGSS` | `VGS = 20 V`, `VDS = 0 V` | - | - | 200 | nA | Fuga de corriente por la compuerta. | Es pequena; el driver gasta energia principalmente cargando `Qg`, no por fuga DC. |

## 6. Caracteristicas dinamicas por MOSFET

Estas especificaciones describen que tan dificil es manejar la compuerta y que tan rapido puede conmutar el modulo.

| Parametro | Condicion | Valor | Que significa | Importancia para el proyecto |
| --- | --- | --- | --- | --- |
| `Ciss` | `VGS = 0 V`, `VDS = 1000 V`, `f = 1 MHz` | 6040 pF | Capacitancia de entrada vista por el driver. | Mientras mayor sea, mas exigente es el driver de compuerta. |
| `Coss` | mismas condiciones | 540 pF | Capacitancia de salida drenador-source. | Influye en energia de conmutacion y transitorios del nodo de salida. |
| `Crss` | mismas condiciones | 50 pF | Capacitancia Miller entre drenador y gate. | Puede causar encendido parasitario; por eso conviene bias negativo o buen apagado. |
| `Qg` | `VGS = -5 V / +20 V`, `VBus = 800 V`, `ID = 80 A` | 464 nC | Carga total que debe mover el driver por ciclo. | Dimensiona la fuente aislada y la corriente del driver; no depende directamente de los `3 A`. |
| `Qgs` | mismas condiciones | 82 nC | Parte de la carga asociada al tramo gate-source. | Afecta el tiempo para llevar el MOSFET hacia conduccion. |
| `Qgd` | mismas condiciones | 100 nC | Carga Miller durante el cambio de tension drenador-source. | Es clave en velocidad de conmutacion y riesgo de disparos por `dv/dt`. |
| `Td(on)` | `VGS = -5 V / +20 V`, `VBus = 600 V`, `ID = 100 A` | 30 ns | Retardo entre ordenar encendido y empezar a encender. | Ayuda a estimar dead-time y sincronizacion. |
| `Tr` | `RGon = 4 ohm`, `RGoff = 2.4 ohm` | 30 ns | Tiempo de subida durante encendido. | Indica conmutacion rapida; exige layout de baja inductancia. |
| `Td(off)` | mismas condiciones | 50 ns | Retardo entre ordenar apagado y empezar a apagar. | Se usa para definir dead-time seguro. |
| `Tf` | mismas condiciones | 25 ns | Tiempo de caida durante apagado. | Apagado rapido reduce perdidas pero puede aumentar sobrepicos. |
| `Eon` | `TJ = 150 C`, `VBus = 600 V`, `ID = 100 A` | 1.98 mJ | Energia perdida en cada encendido. | A `3 A` y bus bajo sera menor, pero se debe validar experimentalmente. |
| `Eoff` | mismas condiciones | 1.3 mJ | Energia perdida en cada apagado. | Suma con `Eon` para estimar perdidas de conmutacion. |
| `RGint` | Resistencia interna de compuerta | 2.94 ohm | Resistencia propia dentro del modulo. | Se suma a `RGon/RGoff` externas para calcular corriente de compuerta. |
| `RthJC` | MOSFET, junta a carcasa | 0.2 C/W | Resistencia termica desde el chip hasta la carcasa. | Mientras menor sea, mas facil extraer calor hacia el disipador. |

Implicacion para el driver: el datasheet caracteriza el modulo con `+20 V` al encender y `-5 V` al apagar. Como el driver seleccionado `UCC21540` permite maximo `18 V` entre `VDDx` y `VSSx`, nuestra implementacion inicial debe usar `+18 V / 0 V` o preferiblemente `+15 V / -3 V`, no `+20 V / -5 V`.

## 7. Diodo cuerpo por MOSFET

| Parametro | Condicion | Valor | Que significa | Importancia para el proyecto |
| --- | --- | --- | --- | --- |
| `VSD` | `VGS = 0 V`, `ISD = 80 A` | 4.0 V | Caida directa del diodo cuerpo con gate en 0 V. | Si circula corriente por el diodo, hay perdida considerable. |
| `VSD` | `VGS = -5 V`, `ISD = 80 A` | 4.2 V | Caida directa con gate polarizado negativo. | El bias negativo mejora apagado, pero puede aumentar ligeramente conduccion por diodo. |
| `trr` | `ISD = 80 A`, `VGS = -5 V`, `VR = 800 V`, `diF/dt = 2000 A/us` | 90 ns | Tiempo de recuperacion inversa. | Influye en picos y perdidas cuando el diodo deja de conducir. |
| `Qrr` | mismas condiciones | 1100 nC | Carga de recuperacion inversa. | Menor `Qrr` reduce perdidas y ruido; aun asi debe considerarse. |
| `Irr` | mismas condiciones | 27 A | Corriente pico de recuperacion inversa. | Puede generar sobrepicos durante conmutacion. |

## 8. Diodo SiC inverso por diodo

| Parametro | Condicion | Valor | Que significa | Importancia para el proyecto |
| --- | --- | --- | --- | --- |
| `VRRM` | Tension inversa repetitiva pico | 1200 V | Maxima tension inversa repetitiva del diodo. | Da margen frente al bus DC y transitorios. |
| `IRM` | `VR = 1200 V`, `TJ = 25 C` | 20 uA tipico, 400 uA max | Corriente de fuga inversa a temperatura ambiente. | Fuga baja cuando el diodo esta bloqueando. |
| `IRM` | `VR = 1200 V`, `TJ = 175 C` | 300 uA tipico | Fuga inversa cuando el diodo esta caliente. | La fuga aumenta con temperatura; se debe vigilar calentamiento. |
| `IF` | `TC = 100 C` | 60 A | Corriente directa continua del diodo. | Nuestro prototipo de `3 A` queda muy por debajo. |
| `VF` | `IF = 60 A`, `TJ = 25 C` | 1.5 V tip, 1.8 V max | Caida directa del diodo a temperatura ambiente. | Si el diodo conduce, la perdida es `VF * I`. |
| `VF` | `IF = 60 A`, `TJ = 175 C` | 2.1 V tip | Caida directa del diodo caliente. | Ayuda a estimar perdidas en recirculacion. |
| `Qc` | `VR = 600 V` | 260 nC | Carga capacitiva del diodo. | Afecta perdidas y transitorios de conmutacion. |
| `C` | `f = 1 MHz`, `VR = 400 V` | 282 pF | Capacitancia del diodo a 400 V. | Influye en corriente capacitiva durante conmutacion. |
| `C` | `f = 1 MHz`, `VR = 800 V` | 210 pF | Capacitancia del diodo a 800 V. | La capacitancia baja al subir la tension inversa. |
| `RthJC` | Diodo, junta a carcasa | 0.477 C/W | Resistencia termica del diodo hacia la carcasa. | Sirve para estimar temperatura si el diodo conduce durante mucho tiempo. |

## 9. Termico, aislamiento y mecanica

| Especificacion | Valor | Que significa | Importancia para el proyecto |
| --- | --- | --- | --- |
| Aislamiento terminal-carcasa | 4000 Vrms, 1 min, 50/60 Hz | Aislamiento electrico entre pines y base/carcasa. | Mejora seguridad de montaje con disipador. |
| Rango de temperatura de junta | -40 C a 175 C | Temperatura interna maxima/minima del semiconductor. | No se debe acercar a 175 C en laboratorio; usar NTC y margen. |
| Temperatura de junta recomendada bajo conmutacion | -40 C a `TJmax - 25 C` | Recomendacion de operar con margen respecto al maximo. | Mantener margen termico evita degradacion y fallas. |
| Temperatura de almacenamiento | -40 C a 125 C | Rango permitido sin operar. | Importa para transporte y almacenamiento. |
| Temperatura de carcasa operativa | -40 C a 125 C | Rango de temperatura en la base/carcasa del modulo. | El disipador debe mantener la carcasa dentro de este rango. |
| Torque de montaje M4 al disipador | 2 N.m a 3 N.m | Fuerza de apriete recomendada para tornillos. | Torque incorrecto puede empeorar contacto termico o danar el modulo. |
| Peso | 110 g | Masa del modulo. | Ayuda a planear montaje mecanico y soporte. |

## 10. Sensor NTC interno

| Parametro | Valor | Que significa | Importancia para el proyecto |
| --- | --- | --- | --- |
| `R25` | 50 kohm | Resistencia del NTC a 25 C. | Punto base para convertir resistencia a temperatura. |
| Tolerancia `R25` | 5 % | Variacion permitida de la resistencia nominal. | La medicion de temperatura tendra tolerancia; no usar limites demasiado justos. |
| `B25/85` | 3952 K | Constante beta para calcular temperatura entre 25 C y 85 C. | Permite estimar temperatura con la ecuacion del NTC. |
| Tolerancia de `B` a `TC = 100 C` | 4 % | Variacion de la constante beta. | Agrega error a la medicion de temperatura. |

## 11. Calculos utiles para el prototipo de 3 A

| Calculo | Formula | Resultado | Que significa |
| --- | --- | --- | --- |
| Perdida por MOSFET con `RDS(on)` tipico 25 C | `3^2 * 0.0125` | 0.1125 W | Perdida de conduccion muy baja por transistor encendido. |
| Perdida por MOSFET con `RDS(on)` max 25 C | `3^2 * 0.016` | 0.144 W | Caso conservador a temperatura baja. |
| Perdida por MOSFET con `RDS(on)` tipico 175 C | `3^2 * 0.020` | 0.180 W | Caso caliente, aun bajo para el modulo. |
| Perdida en camino de dos MOSFETs, tipico 25 C | `2 * 0.1125` | 0.225 W | En puente H suelen conducir dos MOSFETs en serie con la carga. |
| Corriente pico gate ON con `UCC21540` a 18 V | `18 / (4 + 2.94)` | 2.59 A | Dentro del limite `4 A source` del driver. |
| Corriente pico gate OFF con `UCC21540` a 18 V | `18 / (2.4 + 2.94)` | 3.37 A | Dentro del limite `6 A sink` del driver. |

## 12. Puntos de diseno que se deben conservar

| Punto de diseno | Explicacion | Decision para este proyecto |
| --- | --- | --- |
| No conectar STM32 directo al modulo | La compuerta requiere corriente, aislamiento y proteccion. | Usar `UCC21540` como driver aislado. |
| Usar fuente aislada de compuerta | Los high-side son nodos flotantes y ruidosos. | Usar `SN6505BDBVR + transformador + rectificacion + filtrado`. |
| Respetar limite de `VGS` | El modulo permite -10 V a +25 V, pero el driver solo 18 V totales. | Usar `+15/-3 V` preferido o `+18/0 V` simple. |
| Implementar dead-time | Evita que dos MOSFETs de la misma rama conduzcan simultaneamente. | Configurarlo en STM32 y validar con osciloscopio. |
| Usar proteccion de sobrecorriente | Una falla puede crecer muy rapido aunque el objetivo sea `3 A`. | Comparador hardware hacia `DISABLE` del driver y `BKIN` de STM32. |
| Separar `RGon` y `RGoff` | Permite ajustar encendido y apagado de forma independiente. | Empezar cerca de valores del datasheet y ajustar experimentalmente. |
| Mantener bus DC de baja inductancia | Reduce sobrepicos por conmutacion rapida. | Capacitores DC-link cerca del modulo y rutas cortas. |
| Medir NTC | Permite detectar calentamiento anormal. | Conectarlo a ADC/proteccion desde la primera version. |
| Verificar pinout del PDF | El pinout esta en figuras, no en texto confiable. | Revisar visualmente antes de disenar PCB. |

## 13. Datos pendientes antes de diseno final

| Dato pendiente | Por que falta | Para que se necesita |
| --- | --- | --- |
| Pinout detallado desde figura | La extraccion de texto del PDF no recupera bien las figuras. | Dibujar correctamente el esquematico y PCB. |
| Tension de bus DC inicial | Aun no se definio el valor de laboratorio. | Calcular filtros, protecciones y perdidas de conmutacion. |
| Potencia y tipo de carga | Puede ser resistiva, inductiva, motor, transformador o filtro AC. | Define filtro, proteccion y estrategia de control. |
| Frecuencia SPWM final | Afecta perdidas, filtro y fuente de compuerta. | Dimensionar LC, disipacion y fuente aislada. |
| Filtro de salida | Depende de bus, carga, frecuencia y objetivo de onda. | Obtener salida senoidal segura y estable. |
| Disipador y montaje | El modulo necesita ruta termica real. | Validar temperatura aun con baja corriente. |
| Instrumentacion disponible | Sondas, fuente, osciloscopio y medicion diferencial. | Probar sin danar equipo ni crear cortos por tierra. |
