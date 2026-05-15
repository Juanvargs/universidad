# Driver, fuente aislada y filtro para el puente H MSCSM120HM16CT3AG

Autor: Juan Pablo Vargas Cordoba  
Universidad Nacional de Colombia  
Fecha de organizacion: 2026-05-15  
Componentes principales: `UCC21540DWR`, `SN6505BDBVR` y filtro LC de salida  
Punto de partida del prototipo: corriente limitada a `3 A`

## 1. Proposito del documento

Este documento organiza los componentes necesarios para implementar el puente H `MSCSM120HM16CT3AG` en una primera prueba controlada de `3 A`. Se explican los datos relevantes del driver de compuerta, de la fuente aislada y del filtro de salida. Al final se presentan calculos base para justificar la seleccion y definir valores iniciales de implementacion.

## 2. Arquitectura general de implementacion

```text
STM32G474
  -> senales PWM con dead-time
  -> drivers aislados UCC21540DWR
  -> compuertas del puente H MSCSM120HM16CT3AG
  -> filtro LC de salida
  -> carga

5 V auxiliar
  -> SN6505BDBVR
  -> transformador aislado
  -> rectificacion y filtrado
  -> alimentacion aislada de los drivers
```

| Bloque | Funcion | Descripcion corta |
| --- | --- | --- |
| STM32G474 | Generacion de PWM/SPWM | Produce las senales logicas de control y el dead-time. |
| `UCC21540DWR` | Driver de compuerta aislado | Convierte senales logicas en pulsos de compuerta con corriente suficiente. |
| `SN6505BDBVR` | Driver push-pull para transformador | Permite construir fuentes aisladas para alimentar los drivers. |
| Transformador aislado | Aislamiento galvanico de alimentacion | Es el elemento que separa electricamente el control de los nodos de potencia. |
| Puente H SiC | Etapa de potencia | Invierte la polaridad aplicada a la carga mediante cuatro MOSFETs. |
| Filtro LC | Suavizado de salida | Atenua la componente de conmutacion y deja pasar la componente fundamental. |

## 3. Driver de compuerta UCC21540DWR

El `UCC21540DWR` es un driver aislado de doble canal. No maneja la corriente de la carga; su trabajo es cargar y descargar rapidamente las compuertas de los MOSFETs.

| Caracteristica | Valor | Descripcion corta |
| --- | --- | --- |
| Fabricante | Texas Instruments | Proveedor del driver y del datasheet oficial. |
| Tipo | Driver aislado de compuerta, doble canal | Tiene dos salidas de compuerta aisladas respecto a la entrada logica. |
| Cantidad necesaria | 2 integrados | Un puente H tiene cuatro MOSFETs; cada integrado maneja dos compuertas. |
| Corriente pico de salida | 4 A source, 6 A sink | Capacidad para cargar y descargar rapidamente la compuerta. |
| Alimentacion logica `VCCI` | 3 V a 5.5 V | Compatible con senales logicas de microcontroladores como la STM32. |
| Alimentacion de salida `VDDA/B` | 9.2 V a 18 V | Limita la tension total disponible para manejar cada compuerta. |
| Aislamiento entrada-salida | 5.7 kVrms por 1 minuto | Separa el control de los dominios de potencia. |
| CMTI | Mayor que 100 V/ns | Inmunidad ante cambios rapidos de tension de modo comun. |
| Retardo de propagacion | 33 ns tipico, 40 ns max | Tiempo entre la senal de entrada y la respuesta de salida. |
| Matching de retardo | 5 ns max | Diferencia maxima entre canales; ayuda a definir dead-time. |
| Dead-time | Programable por resistencia | Permite agregar una proteccion temporal contra conduccion cruzada. |
| Entrada `DISABLE` | Apagado de salidas | Puede conectarse a una proteccion de sobrecorriente por hardware. |
| UVLO | Proteccion por baja tension | Evita activar compuertas si la alimentacion del driver no es suficiente. |
| Encapsulado recomendado | `DWR` | Variante activa para disenos nuevos. |

### 3.1 Relacion entre UCC21540DWR y el modulo SiC

| Necesidad del puente H | Dato del `UCC21540DWR` | Resultado |
| --- | --- | --- |
| Cuatro compuertas de MOSFET | Dos canales por integrado | Se usan dos drivers. |
| Separacion entre control y potencia | Aislamiento de 5.7 kVrms | Cumple para arquitectura aislada. |
| Conmutacion rapida de SiC | CMTI mayor que 100 V/ns | Adecuado para nodos con alto `dv/dt`. |
| Carga de compuerta grande | 4 A source y 6 A sink | Puede manejar corrientes pico calculadas para 18 V. |
| Manejo ideal del datasheet `+20 V / -5 V` | Salida maxima 18 V totales | No se puede usar `+20/-5`; se debe usar `+18/0` o `+15/-3`. |

## 4. Fuente aislada con SN6505BDBVR

El `SN6505BDBVR` no es una fuente aislada completa. Es un driver push-pull que excita un transformador. El aislamiento real lo entrega el transformador, y la salida DC se obtiene con rectificacion y filtrado.

| Caracteristica | Valor | Descripcion corta |
| --- | --- | --- |
| Fabricante | Texas Instruments | Proveedor del integrado y del datasheet oficial. |
| Tipo | Driver push-pull para transformador | Alterna corriente en el primario de un transformador pequeno. |
| Alimentacion de entrada | 2.25 V a 5.5 V | Puede alimentarse desde 3.3 V o 5 V; para potencia auxiliar conviene 5 V. |
| Corriente de salida primaria | 1 A a 5 V | Capacidad para excitar el transformador de la fuente aislada. |
| Frecuencia interna | 420 kHz en version `SN6505B` | Permite usar transformadores pequenos. |
| `RDS(on)` interno | 0.25 ohm max a 4.5 V | Menor resistencia interna implica menor perdida. |
| Limite de corriente | 1.7 A | Protege el integrado durante sobrecarga o arranque. |
| EMI | Slew-rate control y spread spectrum | Reduce ruido generado por la fuente auxiliar. |
| Protecciones | UVLO, limite de corriente, apagado termico, soft-start | Facilitan un arranque mas controlado. |
| Encapsulado | SOT-23-6 `DBV` | Pequeno; exige buen layout y capacitores cercanos. |

### 4.1 Bloque de alimentacion aislada

```text
5 V auxiliar
  -> SN6505BDBVR
  -> transformador push-pull
  -> diodos Schottky
  -> capacitores de filtrado
  -> regulacion o clamp
  -> VDDA/VSSA y VDDB/VSSB del UCC21540DWR
```

| Elemento | Funcion | Descripcion corta |
| --- | --- | --- |
| 5 V auxiliar | Entrada de energia | Alimenta el `SN6505BDBVR`. No debe confundirse con el bus DC de potencia. |
| Transformador | Aislamiento y transferencia de energia | Permite alimentar drivers high-side flotantes. |
| Rectificador | Conversion AC a DC | Convierte la senal del secundario en tension continua. |
| Capacitor de salida | Filtrado | Reduce rizado de la fuente aislada. |
| Regulador o clamp | Control de tension | Evita superar los 18 V permitidos por el driver. |

## 5. Filtro de salida LC

El filtro LC se ubica despues del puente H. Su objetivo es atenuar la frecuencia de conmutacion PWM y permitir que la componente fundamental de baja frecuencia llegue a la carga.

| Variable | Significado | Descripcion corta |
| --- | --- | --- |
| `L` | Inductancia serie | Se opone a cambios rapidos de corriente y reduce el rizado de alta frecuencia. |
| `C` | Capacitancia en paralelo con la carga | Se opone a cambios rapidos de tension y deriva componentes de alta frecuencia. |
| `Rload` | Carga equivalente | Define la corriente y amortigua el filtro. |
| `fsw` | Frecuencia de conmutacion | Frecuencia del PWM/SPWM. Debe ser mucho mayor que la fundamental. |
| `f0` | Frecuencia fundamental | Frecuencia deseada de salida, por ejemplo 50 Hz o 60 Hz. |
| `fc` | Frecuencia de corte del filtro | Debe quedar por encima de `f0` y por debajo de `fsw`. |

Formula de frecuencia de corte ideal:

```text
fc = 1 / (2 * pi * sqrt(L * C))
```

Para una primera prueba, una regla util es:

```text
f0 << fc << fsw
```

Esto significa que el filtro debe dejar pasar la senal fundamental, pero debe atenuar la conmutacion rapida.

## 6. Tabla de implementacion inicial a 3 A

La siguiente tabla resume valores iniciales para una primera etapa de validacion. La corriente se limita a `3 A`; el bus DC debe mantenerse bajo y con limite de corriente de fuente.

| Parte | Valor inicial | Calculo / criterio | Descripcion |
| --- | --- | --- | --- |
| Corriente de prueba | 3 A | Valor definido de laboratorio | Permite validar control, driver y filtro sin exigir el limite real del modulo. |
| Bus DC inicial | 24 V a 48 V | Fuente limitada en corriente | Rango bajo para pruebas funcionales. No usar alta tension en la primera etapa. |
| Frecuencia SPWM | 20 kHz a 50 kHz | Compromiso entre perdidas y filtro | Frecuencia suficientemente alta para filtrar, pero no extrema para el primer prototipo. |
| Fundamental de salida | 50 Hz o 60 Hz | Segun objetivo de la prueba | Frecuencia que debe quedar despues del filtro LC. |
| Driver de compuerta | 2 x `UCC21540DWR` | 4 MOSFETs / 2 canales por driver | Cada integrado maneja una rama o dos compuertas segun el esquema. |
| Fuente aislada | 2 x `SN6505BDBVR` + transformadores | Un bloque por rama como punto de partida | Simplifica dominios aislados y pruebas. |
| Tension de compuerta simple | `+18 V / 0 V` | Maximo total permitido por UCC21540 | Facil de implementar, pero menos robusta contra encendido Miller. |
| Tension de compuerta recomendada | `+15 V / -3 V` | Total = 18 V | Mejora apagado sin superar el limite del driver. |
| Proteccion de corriente | Corte entre 3.5 A y 5 A | Margen sobre 3 A nominal | Debe ir a `DISABLE` del driver y/o `BKIN` de la STM32. |
| Filtro LC inicial | Definir con `fc` entre fundamental y `fsw` | `fc = 1/(2*pi*sqrt(L*C))` | Se ajusta segun carga, bus DC, rizado permitido y componentes disponibles. |

## 7. Calculos para corriente de 3 A

### 7.1 Perdidas de conduccion

La perdida de conduccion de un MOSFET encendido se estima con:

```text
Pcond_MOSFET = I^2 * RDS(on)
```

En un puente H, durante un estado activo normalmente conducen dos MOSFETs en serie con la carga:

```text
Pcond_camino = 2 * Pcond_MOSFET
```

| Caso | Calculo | Resultado | Descripcion |
| --- | --- | --- | --- |
| MOSFET con `RDS(on)` tipico 25 C | `3^2 * 0.0125` | 0.1125 W | Perdida por un MOSFET conduciendo a 3 A. |
| MOSFET con `RDS(on)` max 25 C | `3^2 * 0.016` | 0.144 W | Caso conservador a temperatura baja. |
| MOSFET caliente, `RDS(on)` tipico 175 C | `3^2 * 0.020` | 0.180 W | Perdida estimada si la resistencia aumenta por temperatura. |
| Camino con dos MOSFETs, tipico 25 C | `2 * 0.1125` | 0.225 W | Perdida total aproximada del camino activo del puente. |
| Camino con dos MOSFETs, max 25 C | `2 * 0.144` | 0.288 W | Caso conservador del camino activo. |
| Camino con dos MOSFETs, caliente | `2 * 0.180` | 0.360 W | A 3 A sigue siendo una perdida baja para este modulo. |

Conclusion: a `3 A`, la conduccion no es el punto critico del modulo. El cuidado principal esta en compuerta, aislamiento, dead-time, protecciones y layout.

### 7.2 Corriente pico de compuerta

La corriente pico aproximada del driver se calcula con:

```text
Igate_on  = Vdrive / (RGon + RGint)
Igate_off = Vdrive / (RGoff + RGint)
```

Usando `Vdrive = 18 V`, `RGon = 4 ohm`, `RGoff = 2.4 ohm` y `RGint = 2.94 ohm`:

| Calculo | Resultado | Descripcion |
| --- | --- | --- |
| `Igate_on = 18 / (4 + 2.94)` | 2.59 A | Corriente pico de encendido. Esta por debajo de los 4 A source del UCC21540. |
| `Igate_off = 18 / (2.4 + 2.94)` | 3.37 A | Corriente pico de apagado. Esta por debajo de los 6 A sink del UCC21540. |

Conclusion: el `UCC21540DWR` tiene margen de corriente de salida para esta estrategia inicial de compuerta.

### 7.3 Potencia de compuerta

La potencia promedio necesaria para cargar y descargar compuertas se estima con:

```text
Pgate = Qg * Vdrive * fsw
```

Usando `Qg = 464 nC` y `Vdrive = 18 V`:

| `fsw` | Potencia por MOSFET | Potencia para 4 MOSFETs | Descripcion |
| --- | --- | --- | --- |
| 20 kHz | 0.167 W | 0.668 W | Valor razonable para una primera prueba. |
| 50 kHz | 0.418 W | 1.672 W | Exige mejor fuente aislada y mejor control de EMI. |
| 100 kHz | 0.835 W | 3.340 W | No recomendado como primera condicion de arranque. |

Esta potencia depende de la carga de compuerta y de la frecuencia de conmutacion, no directamente de la corriente de carga de `3 A`.

### 7.4 Perdidas de conmutacion estimadas

El datasheet reporta:

```text
Eon = 1.98 mJ
Eoff = 1.30 mJ
Etotal = 3.28 mJ
Condicion: 600 V, 100 A, TJ = 150 C
```

Una estimacion preliminar para `3 A` puede escalarse con tension y corriente:

```text
Etotal_aprox = 3.28 mJ * (Vbus / 600 V) * (3 A / 100 A)
Psw_aprox = Etotal_aprox * fsw
```

| Bus DC | `Etotal_aprox` por MOSFET | `Psw` a 20 kHz | `Psw` a 50 kHz | Descripcion |
| --- | --- | --- | --- | --- |
| 24 V | 3.94 uJ | 0.079 W | 0.197 W | Muy comodo para validacion inicial. |
| 48 V | 7.87 uJ | 0.157 W | 0.394 W | Aun adecuado para pruebas controladas. |
| 100 V | 16.4 uJ | 0.328 W | 0.820 W | Requiere mas cuidado termico y de medicion. |
| 200 V | 32.8 uJ | 0.656 W | 1.640 W | Ya exige layout, proteccion y disipacion bien validados. |

Este calculo es aproximado. Las perdidas reales dependen de tension, corriente, temperatura, resistencias de compuerta, inductancia parasita y forma de conmutacion.

### 7.5 Filtro LC de salida

Para seleccionar un filtro LC inicial:

```text
fc = 1 / (2 * pi * sqrt(L * C))
```

Si se conoce una inductancia disponible, se puede despejar la capacitancia:

```text
C = 1 / ((2 * pi * fc)^2 * L)
```

Si se conoce una capacitancia disponible, se puede despejar la inductancia:

```text
L = 1 / ((2 * pi * fc)^2 * C)
```

Ejemplo con `L = 270 uH`:

| Capacitancia | Frecuencia de corte | Descripcion |
| --- | --- | --- |
| `1 uF` | 9.69 kHz | Atenua poco la conmutacion si `fsw` es cercana a 20 kHz. |
| `11 uF` | 2.92 kHz | Mejor filtrado; valor equivalente de dos capacitores de 22 uF en serie. |
| `23.5 uF` | 2.00 kHz | Mejor suavizado; valor equivalente de dos capacitores de 47 uF en serie. |

Para una salida de 50 Hz con SPWM de 20 kHz, una `fc` alrededor de 2 kHz deja pasar la fundamental y atenua parte importante del rizado de conmutacion. El valor final debe validarse con osciloscopio, temperatura del inductor, corriente RMS del capacitor y comportamiento de la carga.

## 8. Recomendacion de implementacion inicial

| Elemento | Seleccion inicial | Motivo |
| --- | --- | --- |
| Driver | 2 x `UCC21540DWR` | Manejan cuatro compuertas con aislamiento y corriente suficiente. |
| Fuente aislada | 2 x `SN6505BDBVR` con transformadores adecuados | Permiten alimentar dominios flotantes de los drivers. |
| Tension de compuerta | `+15 V / -3 V` preferida | Mejora apagado y mantiene 18 V totales. |
| Corriente de prueba | 3 A | Valor seguro para primera validacion. |
| Bus DC | 24 V a 48 V inicialmente | Permite observar funcionamiento sin entrar en alta tension. |
| Frecuencia SPWM | 20 kHz al inicio | Reduce exigencia de compuerta frente a frecuencias mas altas. |
| Filtro | LC con `fc` entre 1 kHz y 3 kHz como punto de partida | Deja pasar 50 Hz y reduce rizado de PWM. |
| Proteccion minima | Sobrecorriente por hardware y apagado de driver | No depender solamente del firmware. |

## 9. Fuentes consultadas

- TI `UCC21540DWR`: https://www.ti.com/product/UCC21540/part-details/UCC21540DWR
- Datasheet TI `UCC21540`: https://www.ti.com/lit/ds/symlink/ucc21540.pdf
- TI `SN6505BDBVR`: https://www.ti.com/product/SN6505B/part-details/SN6505BDBVR
- Datasheet TI `SN6505A/SN6505B`: https://www.ti.com/lit/ds/symlink/sn6505a.pdf
- Referencia TI TIDA-010933: https://www.ti.com/tool/TIDA-010933
