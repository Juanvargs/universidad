# Driver y fuente aislada para el puente H MSCSM120HM16CT3AG

Fecha de revision: 2026-05-14  
Componentes seleccionados: `UCC21540DW / UCC21540DWR` y `SN6505BDBVR`  
Aplicacion real base: `TIDA-010933`, microinverter bidireccional de 1.6 kW de Texas Instruments.  
Punto de partida del prototipo: corriente limitada a `3 A`. No se usara el limite maximo de `1200 V` del modulo en la primera etapa.

## 1. Resumen ejecutivo

La combinacion `UCC21540` + `SN6505B` tiene sentido para una primera implementacion real del puente H `MSCSM120HM16CT3AG` trabajando a `3 A`. A esa corriente, el modulo SiC queda muy sobredimensionado en conduccion; por tanto, el riesgo principal no es la corriente nominal del MOSFET, sino la forma de manejar las compuertas, el aislamiento, el dead-time, las protecciones y el layout.

La conclusion actual es:

- Para un puente H completo se necesitan dos `UCC21540`, porque cada integrado tiene dos salidas y el modulo tiene cuatro MOSFETs.
- El `SN6505BDBVR` sirve para construir la fuente aislada de compuerta, pero no es una fuente aislada completa por si solo. Necesita transformador, rectificacion, filtrado y, si aplica, regulacion.
- Aunque el modulo es de `1200 V`, el diseno inicial debe tratarse como un prototipo de baja potencia/corriente limitada.
- El `UCC21540` no permite usar exactamente `+20 V / -5 V`, porque su alimentacion maxima de salida es `18 V` entre `VDDx` y `VSSx`.
- Para este prototipo se recomienda empezar con `+18 V / 0 V` o `+15 V / -3 V`.
- Para `3 A`, las perdidas de conduccion son pequenas, pero aun asi se debe mantener proteccion de sobrecorriente por hardware.

## 2. Referencia real: TIDA-010933

`TIDA-010933` es un diseno de referencia de Texas Instruments para un microinverter bidireccional de `1.6 kW` basado en GaN. Es relevante porque demuestra una arquitectura real con control digital, drivers aislados, fuentes aisladas auxiliares y convertidores de potencia compactos.

| Especificacion | Valor en TIDA-010933 | Que significa para este proyecto |
| --- | --- | --- |
| Tipo de aplicacion | Microinverter bidireccional | Es una aplicacion real cercana al objetivo de conversion DC/AC. |
| Potencia de referencia | 1.6 kW | Muestra que los componentes se usan en equipos de potencia, no solo en pruebas pequenas. |
| Producto resaltado | `UCC21540` | TI lo propone como driver aislado en un diseno real. |
| Alimentaciones aisladas | Se usan fuentes aisladas auxiliares | Confirma que el driver de potencia necesita dominios flotantes bien definidos. |
| Alta frecuencia | El diseno busca reducir tamano de filtros EMI | Es util como referencia para pensar en layout, EMI y fuentes auxiliares. |

Fuentes:

- TI TIDA-010933: https://www.ti.com/tool/TIDA-010933
- Guia de diseno TIDA-010933: https://www.ti.com/document-viewer/lit/html/TIDUF63
- Referencia Wurth para `SN6505` en TIDA-010933: https://www.we-online.com/en/components/icref/texas-instruments/SN6505-TIDA-010933-Inverter

## 3. Requisitos del puente H considerando una primera prueba de 3 A

| Especificacion | Valor del `MSCSM120HM16CT3AG` | Que significa | Decision para prototipo de 3 A |
| --- | --- | --- | --- |
| Topologia | Puente H completo | Hay cuatro MOSFETs que deben manejarse de forma coordinada. | Usar dos drivers duales `UCC21540`. |
| Tension maxima del modulo | 1200 V | Es el limite del modulo, no el valor obligatorio de operacion. | No disenar la primera prueba para 1200 V; usar bus DC bajo y limitado. |
| Corriente nominal | 173 A a `TC = 25 C`; 138 A a `TC = 80 C` | El modulo soporta mucha mas corriente que la prueba inicial. | A `3 A` hay mucho margen de conduccion. |
| Corriente inicial objetivo | 3 A | Este sera el limite de laboratorio para comenzar. | Dimensionar protecciones para cortar por encima de 3 A, por ejemplo 3.5 A a 5 A segun margen experimental. |
| `RDS(on)` | 12.5 mohm tipico, 16 mohm max a `VGS = 20 V`; 20 mohm tipico a `TJ = 175 C` | Define la perdida por conduccion `I^2 * R`. | A `3 A`, la perdida es muy baja. |
| `VGS` absoluto | -10 V a +25 V | No se debe superar este rango entre gate y source. | Con `+18/0` o `+15/-3` se permanece dentro del limite. |
| Manejo dinamico del datasheet | `+20 V / -5 V` | Es la condicion ideal usada por el fabricante para caracterizar switching. | No se puede replicar con `UCC21540`; usar 18 V totales. |
| `Qg` | 464 nC por MOSFET | Energia necesaria para cargar/descargar la compuerta. | La fuente aislada se dimensiona por `Qg` y `fsw`, no por los 3 A de carga. |
| Tiempos del MOSFET | `Td(on)=30 ns`, `Tr=30 ns`, `Td(off)=50 ns`, `Tf=25 ns` | El dispositivo puede conmutar rapido. | El driver y layout deben evitar retardos, ruido y disparos falsos. |
| Sensor termico | NTC interno 50 kohm | Permite medir temperatura del modulo. | Usarlo desde la primera placa aunque la corriente inicial sea baja. |

## 4. Calculos para corriente inicial de 3 A

### 4.1 Perdidas de conduccion

La perdida de conduccion aproximada de un MOSFET encendido es:

```text
Pcond_MOSFET = I^2 * RDS(on)
```

Para una corriente de `3 A`:

| Caso | Calculo | Resultado | Que significa |
| --- | --- | --- | --- |
| MOSFET, `RDS(on)` tipico 25 C | `3^2 * 0.0125` | 0.1125 W | Perdida instantanea por MOSFET cuando conduce. |
| MOSFET, `RDS(on)` max 25 C | `3^2 * 0.016` | 0.144 W | Caso mas conservador a temperatura baja. |
| MOSFET, `RDS(on)` tipico 175 C | `3^2 * 0.020` | 0.180 W | Perdida aproximada si el modulo esta caliente. |
| Camino de corriente con dos MOSFETs, tipico 25 C | `2 * 0.1125` | 0.225 W | En un puente H normalmente conducen dos MOSFETs en serie con la carga. |
| Camino de corriente con dos MOSFETs, max 25 C | `2 * 0.144` | 0.288 W | Perdida de conduccion muy baja para el modulo. |
| Camino de corriente con dos MOSFETs, tipico 175 C | `2 * 0.180` | 0.360 W | Incluso caliente, el modulo queda holgado para 3 A. |

Lectura practica: a `3 A`, las perdidas de conduccion no son el problema principal. El foco debe estar en no cometer errores de conmutacion, aislamiento, dead-time, medicion y proteccion.

### 4.2 Corriente pico de compuerta con UCC21540

Con el `UCC21540` se recomienda usar 18 V totales de manejo de compuerta. La corriente pico aproximada depende de la resistencia externa y de la resistencia interna de compuerta del modulo:

```text
Igate_on  = Vdrive / (RGon + RGint)
Igate_off = Vdrive / (RGoff + RGint)
```

Usando datos del datasheet del modulo:

| Especificacion | Valor | Calculo | Resultado | Que significa |
| --- | --- | --- | --- | --- |
| `Vdrive_total` | 18 V | - | 18 V | Tension maxima practica compatible con `UCC21540`. |
| `RGon` externa | 4 ohm | - | 4 ohm | Resistencia usada por el fabricante para encendido. |
| `RGoff` externa | 2.4 ohm | - | 2.4 ohm | Resistencia usada por el fabricante para apagado. |
| `RGint` | 2.94 ohm | - | 2.94 ohm | Resistencia interna de compuerta del modulo. |
| Corriente pico ON | `18 / (4 + 2.94)` | - | 2.59 A | Menor que los 4 A source del driver; cumple. |
| Corriente pico OFF | `18 / (2.4 + 2.94)` | - | 3.37 A | Menor que los 6 A sink del driver; cumple. |

Conclusion: para 18 V totales de compuerta, el `UCC21540` tiene margen de corriente de salida para manejar este modulo en la prueba inicial.

### 4.3 Energia de compuerta y fuente aislada

La potencia promedio de compuerta se estima como:

```text
Pgate = Qg * Vdrive_total * fsw
```

Usando `Qg = 464 nC` como valor conservador y `Vdrive_total = 18 V`:

| Frecuencia SPWM | Potencia por MOSFET | Potencia para 4 MOSFETs | Que significa |
| --- | --- | --- | --- |
| 20 kHz | 0.167 W | 0.668 W | Frecuencia razonable para primera prueba; fuente aislada moderada. |
| 50 kHz | 0.418 W | 1.672 W | Aun viable, pero exige mejor transformador y control de EMI. |
| 100 kHz | 0.835 W | 3.340 W | Posible teoricamente, pero no recomendable como primer arranque. |

Nota importante: esta potencia depende de `Qg` y frecuencia de conmutacion, no de que la carga sea de `3 A`. Aunque el modulo conduzca poca corriente, las compuertas siguen siendo grandes y necesitan una fuente auxiliar seria.

### 4.4 Perdidas de conmutacion estimadas a 3 A

El datasheet da:

```text
Eon = 1.98 mJ
Eoff = 1.30 mJ
Etotal = 3.28 mJ
Condicion: 600 V, 100 A, TJ = 150 C
```

Para estimar de forma preliminar a `3 A`, se puede escalar linealmente con corriente y tension de bus:

```text
Etotal_aprox = 3.28 mJ * (Vbus / 600 V) * (3 A / 100 A)
```

| Bus DC de prueba | Energia aprox por MOSFET y ciclo | Perdida por MOSFET a 20 kHz | Perdida por MOSFET a 50 kHz | Que significa |
| --- | --- | --- | --- | --- |
| 24 V | 3.94 uJ | 0.079 W | 0.197 W | Muy baja; ideal para primera validacion funcional. |
| 48 V | 7.87 uJ | 0.157 W | 0.394 W | Aun comodo; buen siguiente paso de laboratorio. |
| 100 V | 16.4 uJ | 0.328 W | 0.820 W | Empieza a exigir mas cuidado termico y EMI. |
| 200 V | 32.8 uJ | 0.656 W | 1.640 W | Ya requiere validacion seria de layout, disipacion y protecciones. |

Este calculo es aproximado. Las perdidas reales dependen del bus, corriente, temperatura, resistencias de compuerta, inductancias parasitas y forma de modulacion.

## 5. Ficha tecnica: UCC21540DW / UCC21540DWR

| Especificacion | Valor | Que significa para el proyecto |
| --- | --- | --- |
| Fabricante | Texas Instruments | Proveedor reconocido y con documentacion completa. |
| Tipo | Driver aislado de compuerta, doble canal | Permite manejar dos MOSFETs desde senales logicas aisladas. |
| Canales | 2 | Se necesitan dos integrados para un puente H de cuatro MOSFETs. |
| Configuracion | Dos low-side, dos high-side o half-bridge | Puede adaptarse a cada rama del puente H. |
| Aislacion entrada-salida | 5.7 kVrms por 1 minuto | Aisla la STM32/control de la etapa de potencia. |
| Aislacion reforzada | 8000 Vpk segun familia UCC2154x | Da margen de seguridad electrica frente a transitorios. |
| CMTI | Mayor que 100 V/ns; TI tambien lista 125 V/ns | Ayuda a evitar disparos falsos por alto `dv/dt`. |
| Corriente pico de salida | 4 A source, 6 A sink | Suficiente para los 2.59 A ON y 3.37 A OFF estimados con 18 V. |
| `VCCI` | 3 V a 5.5 V | Compatible con logica de microcontrolador de 3.3 V o 5 V. |
| `VDDA/B` | 9.2 V a 18 V | Limita el manejo de compuerta a 18 V totales. |
| UVLO | En todas las alimentaciones | Evita conmutar con alimentacion insuficiente. |
| Retardo de propagacion | 33 ns tipico; 40 ns max | Es rapido frente a frecuencias SPWM de 20 kHz a 50 kHz. |
| Matching de retardo | 5 ns max | Reduce diferencias entre canales y facilita dead-time. |
| Distorsion de ancho de pulso | 5.5 ns a 6 ns max | Mantiene precision temporal de la PWM. |
| Rise/fall time tipico | 5 ns / 6 ns | El integrado puede entregar flancos rapidos; la compuerta real sera mas lenta por `Qg` y resistencias. |
| Dead-time | Programable por resistencia | Da una segunda barrera contra conduccion cruzada. |
| Disable | Apaga simultaneamente ambas salidas | Puede conectarse a una proteccion de sobrecorriente por hardware. |
| Temperatura de operacion | -40 C a 125 C | Adecuado para ambiente de laboratorio y electronica de potencia. |
| Encapsulado | SOIC-16 `DW`; variantes `DWK` | Paquete soldable en PCB, con creepage/clearance definidos. |
| Estado de compra | `UCC21540DW` EOL/obsoleto; `UCC21540DWR` activo | Conviene comprar `UCC21540DWR` si se va a fabricar una placa nueva. |

Fuentes:

- TI `UCC21540DWR`: https://www.ti.com/product/UCC21540/part-details/UCC21540DWR
- Datasheet TI `UCC21540`: https://www.ti.com/lit/ds/symlink/ucc21540.pdf
- Mouser `UCC21540DW`: https://www.mouser.com/ProductDetail/Texas-Instruments/UCC21540DW?qs=byeeYqUIh0PZlHLK%252BqwDFA%3D%3D
- DigiKey `UCC21540DW`: https://www.digikey.com/en/products/detail/texas-instruments/UCC21540DW/9860890

### 5.1 Comparacion UCC21540 contra el MSCSM120HM16CT3AG a 3 A

| Punto de comparacion | Puente H `MSCSM120HM16CT3AG` | Driver `UCC21540` | Resultado | Que significa |
| --- | --- | --- | --- | --- |
| Numero de interruptores | 4 MOSFETs | 2 canales por integrado | Cumple con 2 integrados | Cada `UCC21540` puede manejar una rama del puente. |
| Aislacion requerida | Alta por etapa de potencia flotante | 5.7 kVrms entrada-salida | Cumple | Aunque no usemos 1200 V, la aislacion mejora seguridad y robustez. |
| Inmunidad `dv/dt` | SiC con conmutacion rapida | `>100 V/ns` o `125 V/ns` | Cumple | Reduce riesgo de pulsos falsos por transitorios. |
| Corriente pico ON | 2.59 A estimada con 18 V | 4 A source | Cumple | Hay margen para cargar la compuerta. |
| Corriente pico OFF | 3.37 A estimada con 18 V | 6 A sink | Cumple | Hay margen para apagar rapido. |
| Corriente de carga | 3 A inicial | No depende directamente del driver | Cumple | El driver maneja compuerta, no corriente de potencia. |
| Tension de compuerta ideal | `+20/-5 V` | Maximo 18 V totales | Parcial | Se debe aceptar `+18/0` o `+15/-3`. |
| Proteccion de corto | Necesaria aunque sea a 3 A | No tiene DESAT integrado | Falta proteccion externa | Se debe agregar comparador/sensor hacia `DISABLE` y/o `BKIN`. |
| Miller clamp | Deseable en SiC | No integrado | Requiere mitigacion externa | Usar bias negativo moderado, resistencias adecuadas o clamp externo. |
| Dead-time | Obligatorio | Programable y tambien desde STM32 | Cumple | Debe configurarse en ambos niveles con criterio. |

Conclusion: para una prueba inicial de `3 A`, el `UCC21540` es adecuado como driver si se limita la tension de compuerta a 18 V totales y se agregan protecciones externas.

## 6. Ficha tecnica: SN6505BDBVR

| Especificacion | Valor | Que significa para el proyecto |
| --- | --- | --- |
| Fabricante | Texas Instruments | Componente documentado y usado en referencias reales. |
| Tipo | Driver push-pull para transformador | No entrega aislamiento solo; excita un transformador. |
| Funcion | Generar fuente aislada con transformador externo | Permite alimentar cada lado flotante del driver. |
| Entrada | 2.25 V a 5.5 V | Puede alimentarse desde una linea auxiliar de 3.3 V o 5 V; se recomienda 5 V. |
| Corriente de salida primaria | 1 A a alimentacion de 5 V | Capacidad para excitar transformadores pequenos de fuente aislada. |
| Frecuencia interna | 420 kHz en `SN6505B` | Permite transformador mas pequeno que versiones de menor frecuencia. |
| `RDS(on)` interno | 0.25 ohm max a 4.5 V | Reduce perdida interna al manejar el transformador. |
| Limite de corriente | 1.7 A | Protege el driver ante sobrecarga o arranque dificil. |
| EMI | Slew-rate control y spread spectrum | Ayuda a reducir ruido en una placa con SiC. |
| Sincronizacion | Permite reloj externo | Util si se quiere controlar ubicacion de armonicos. |
| Protecciones | UVLO, limite de corriente, apagado termico, break-before-make, soft-start | Facilita arranque seguro de la fuente auxiliar. |
| Shutdown | Menor que 1 uA | Permite apagar la fuente auxiliar con bajo consumo. |
| Temperatura | -55 C a 125 C | Margen amplio para electronica cercana a potencia. |
| Encapsulado | SOT-23-6 `DBV` | Pequeno, pero exige buen layout. |
| Estado de compra | Activo en TI | Disponible para diseno nuevo. |

Fuentes:

- TI `SN6505BDBVR`: https://www.ti.com/product/SN6505B/part-details/SN6505BDBVR
- Datasheet TI `SN6505A/SN6505B`: https://www.ti.com/lit/ds/symlink/sn6505a.pdf
- Mouser `SN6505BDBVR`: https://www.mouser.com/ProductDetail/Texas-Instruments/SN6505BDBVR?qs=yajEpaT76uT%2FjUqvK%2B2rdw%3D%3D
- DigiKey `SN6505BDBVR`: https://www.digikey.com/en/products/detail/texas-instruments/SN6505BDBVR/5994596

### 6.1 Comparacion SN6505B contra la necesidad del puente H a 3 A

| Punto de comparacion | Necesidad del sistema | `SN6505B` | Resultado | Que significa |
| --- | --- | --- | --- | --- |
| Fuente flotante para high-side | Necesaria | La permite con transformador | Cumple | El high-side necesita una fuente referida a su source flotante. |
| Fuente para cuatro compuertas | 4 dominios o arquitectura equivalente | Depende del transformador y secundarios | Cumple si se disena bien | Se puede usar un transformador multisalida o varios SN6505B. |
| Potencia de compuerta | 0.668 W a 20 kHz; 1.672 W a 50 kHz para 4 MOSFETs | Capacidad depende del transformador | Viable con margen | No basta escoger el IC; hay que escoger el transformador correcto. |
| Aislacion | Necesaria para seguridad y nodos flotantes | La da el transformador | Depende del transformador | El transformador debe cumplir la aislacion requerida. |
| EMI | Importante por SiC | Incluye slew-rate control y spread spectrum | Favorable | Ayuda a que la fuente auxiliar no agregue ruido excesivo. |
| Relacion con 3 A | La carga inicial es baja | La fuente depende de compuerta, no de carga | No cambia mucho | Aunque se pruebe a 3 A, la compuerta del modulo sigue siendo grande. |

## 7. Fuente aislada: por que sigue siendo necesaria

Aunque no se vaya a usar el bus de `1200 V`, una fuente aislada sigue siendo conveniente porque:

| Motivo | Explicacion corta | Impacto en este proyecto |
| --- | --- | --- |
| High-side flotante | El source del MOSFET superior se mueve con el nodo de conmutacion. | La alimentacion del driver debe moverse con ese nodo. |
| Seguridad del control | La STM32 no debe compartir directamente la energia de potencia. | Reduce riesgo de dano al microcontrolador y al PC. |
| Ruido de modo comun | El puente SiC genera transitorios rapidos. | Aislar ayuda a evitar que el ruido vuelva al control. |
| Crecimiento futuro | El prototipo empieza a 3 A, pero puede subir tension/corriente. | La arquitectura no queda limitada a una prueba pequena. |

Implementacion recomendada:

```text
5 V auxiliar
  -> SN6505B
  -> transformador push-pull aislado
  -> rectificacion Schottky
  -> filtrado local
  -> regulacion/clamp
  -> alimentacion VDDA/VSSA y VDDB/VSSB del UCC21540
```

Opciones de salida:

| Opcion | Tension de compuerta | Ventaja | Riesgo / cuidado |
| --- | --- | --- | --- |
| Simple | `+18 V / 0 V` | Mas facil de implementar. | Menor inmunidad ante encendido parasitario por Miller. |
| Recomendada para SiC | `+15 V / -3 V` | Mejora apagado sin superar 18 V totales. | Requiere generar referencia negativa bien controlada. |
| No usar con UCC21540 | `+20 V / -5 V` | Coincide con datasheet del modulo. | Excede el maximo de 18 V del `UCC21540`. |

## 8. Arquitectura recomendada para el prototipo de 3 A

```text
STM32G474
  PWM_H1 / PWM_L1  -> UCC21540 #1 -> MOSFET alto/bajo rama A
  PWM_H2 / PWM_L2  -> UCC21540 #2 -> MOSFET alto/bajo rama B

5 V auxiliar -> SN6505B #1 + transformador -> fuente aislada driver rama A
             -> SN6505B #2 + transformador -> fuente aislada driver rama B
```

| Bloque | Cantidad | Que hace | Por que se usa |
| --- | --- | --- | --- |
| `UCC21540DWR` | 2 | Maneja las cuatro compuertas del puente H. | Cada integrado tiene dos canales aislados. |
| `SN6505BDBVR` | 2 recomendado | Genera fuentes aisladas para los drivers. | Un bloque por rama simplifica pruebas y layout. |
| Transformador push-pull | 2 recomendado | Entrega energia aislada al secundario. | Es el elemento que realmente da aislamiento en la fuente. |
| Rectificacion y filtrado | 2 o mas salidas | Convierte AC del transformador en DC estable. | El driver necesita alimentacion limpia. |
| Proteccion de sobrecorriente | 1 sistema minimo | Apaga el puente si se supera el limite. | A 3 A se puede cortar antes de que una falla crezca. |
| NTC del modulo | 1 medicion | Mide temperatura del modulo. | Permite detener pruebas si hay calentamiento anormal. |

## 9. Protecciones recomendadas para empezar a 3 A

| Proteccion | Valor / criterio inicial | Que significa |
| --- | --- | --- |
| Limite de corriente de laboratorio | 3 A | Valor objetivo de operacion inicial. |
| Umbral de corte rapido | 3.5 A a 5 A | Margen para tolerancias sin permitir fallas grandes. |
| Fusible o limitador de bus | Segun fuente DC usada | Barrera fisica si falla el control. |
| Entrada `DISABLE` del UCC21540 | Conectada a comparador de falla | Permite apagar driver sin esperar firmware. |
| Entrada `BKIN` de STM32 | Conectada a falla de corriente | Permite apagar PWM por hardware del temporizador. |
| TVS/Zener gate-source | Dentro de -10 V a +25 V | Protege la compuerta del modulo. |
| Dead-time inicial | Conservador, ajustar con osciloscopio | Evita que dos MOSFETs de la misma rama conduzcan a la vez. |
| DC-link cercano al modulo | Capacitores de baja inductancia | Reduce sobrepicos en conmutacion. |
| Medicion diferencial | Sonda diferencial / medicion segura | Evita cortos por tierra del osciloscopio. |

## 10. Precios publicos y disponibilidad

Precios consultados el 2026-05-14. No incluyen envio, impuestos, aranceles ni cambios de inventario.

| Componente | Distribuidor | Estado | Precio unitario aproximado | Que significa | Link |
| --- | --- | --- | --- | --- | --- |
| `UCC21540DW` | Mouser | 510 en stock; EOL programado | USD 4.48 a 1 unidad; USD 3.40 a 10 unidades | Se consigue, pero no conviene para diseno nuevo si hay alternativa activa. | https://www.mouser.com/ProductDetail/Texas-Instruments/UCC21540DW?qs=byeeYqUIh0PZlHLK%252BqwDFA%3D%3D |
| `UCC21540DW` | DigiKey | Obsolete; 171 en stock | USD 4.61 a 1 unidad; USD 3.503 a 10 unidades | Puede comprarse para pruebas, pero esta en salida de vida. | https://www.digikey.com/en/products/detail/texas-instruments/UCC21540DW/9860890 |
| `UCC21540DWR` | TI / distribuidores | Activo | DigiKey lo lista como sustituto recomendado alrededor de USD 2.42 | Mejor opcion de compra para PCB nueva. | https://www.ti.com/product/UCC21540/part-details/UCC21540DWR |
| `SN6505BDBVR` | Mouser | 12,388 en stock | USD 2.17 a 1 unidad; USD 1.60 a 10 unidades | Disponible y conveniente para comprar. | https://www.mouser.com/ProductDetail/Texas-Instruments/SN6505BDBVR?qs=yajEpaT76uT%2FjUqvK%2B2rdw%3D%3D |
| `SN6505BDBVR` | DigiKey | Sin stock; backorder | USD 2.23 a 1 unidad; USD 1.653 a 10 unidades | Precio similar, pero peor disponibilidad al momento de consulta. | https://www.digikey.com/en/products/detail/texas-instruments/SN6505BDBVR/5994596 |

Costo minimo estimado solo de ICs:

| Arquitectura | Cantidad | Costo aproximado | Que significa |
| --- | --- | --- | --- |
| 2 x `UCC21540DW` + 2 x `SN6505BDBVR` | 2 drivers duales + 2 drivers de transformador | USD 13.30 con precios Mouser a 1 unidad | Opcion posible, pero usa UCC en estado EOL. |
| 2 x `UCC21540DWR` + 2 x `SN6505BDBVR` | Empaque activo/recomendado | Alrededor de USD 9.04 usando sustituto DigiKey para UCC y Mouser para SN6505 | Mejor opcion de compra si se consigue inventario. |
| 2 x `UCC21540DW` + 4 x `SN6505BDBVR` | Fuente independiente por canal | USD 17.64 con precios Mouser a 1 unidad | Mas modular, pero no necesario para el primer prototipo. |

Faltan en el costo: transformadores, diodos Schottky, capacitores, resistencias de compuerta, TVS/Zener, conectores, sensores, comparadores y PCB.

## 11. Decision tecnica actual

Para un primer prototipo limitado a `3 A`, la seleccion queda aprobada con condiciones:

| Decision | Eleccion | Que significa |
| --- | --- | --- |
| Driver principal | Dos `UCC21540DWR` | Manejan las cuatro compuertas del puente H. |
| Fuente aislada | Dos `SN6505BDBVR` + transformadores | Alimentan las salidas flotantes de los drivers. |
| Tension de compuerta inicial | `+15 V / -3 V` preferida, o `+18 V / 0 V` simple | Ambas respetan el limite de 18 V del `UCC21540`. |
| Corriente inicial | 3 A | Mantiene bajas las perdidas y reduce riesgo en laboratorio. |
| Bus DC inicial | Bajo y limitado, no 1200 V | El modulo lo soporta, pero no es necesario ni prudente al inicio. |
| Frecuencia inicial | 20 kHz a 50 kHz | Balance entre filtro razonable, perdidas y dificultad de layout. |
| Proteccion minima | Sobrecorriente por hardware + `DISABLE` + `BKIN` | No depender solo del firmware. |
| Validacion | Osciloscopio con medicion segura | Verificar dead-time, VGS, ruido y temperatura antes de subir potencia. |

La idea central es correcta: usar componentes que ya aparecen en una aplicacion real (`TIDA-010933`), pero adaptarlos al `MSCSM120HM16CT3AG` como una etapa inicial de `3 A`. El modulo queda sobrado en corriente; el diseno debe concentrarse en que las compuertas se manejen de forma limpia, aislada y protegida.
