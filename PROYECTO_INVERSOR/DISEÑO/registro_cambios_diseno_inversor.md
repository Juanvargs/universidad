# Registro de cambios de diseno del inversor

Fecha: 2026-05-19

Este documento plasma los cambios que se realizaron respecto al modelo del TIDA para adaptar la plantilla al inversor con modulo SiC `MSCSM120HM16CT3AG`.

## 1. Resumen ejecutivo de cambios

La plantilla TIDA no se copia literalmente. Se conserva la arquitectura general, pero se cambian valores y algunos criterios para que sirvan al modulo SiC.

| Bloque | Plantilla TIDA | Cambio para este inversor | Estado |
| --- | --- | --- | --- |
| Fuente de gate | `12 V` | `18 V` | Requerido |
| Transformador fuente | `PAD002-T764113S`, `1:1.3` | `750342879` | Requerido |
| Rectificador de fuente | topologia TIDA | Se revisa la topologia: `750342879` se usa como salida ~17.5 V, no como doblador TIDA | Correccion critica |
| Diodos rectificadores | `MBR0520`, `20 V` | `SS16`, `60 V`, `1 A` | Requerido |
| Zener clamp | `16 V` | `MMSZ5250B`, `20 V` | Requerido |
| Capacitores de salida fuente | `25 V` | `50 V` recomendado | Requerido |
| Driver | `UCC21540DW` | Se conserva | OK |
| `DT` | `20k` + capacitor | `20k` + `1 nF` | OK |
| Entrada PWM | filtro de TIDA | Se usa filtro mas pequeno: `33R/51R` + `33pF/47pF` | Recomendado |
| Red de gate | valores TIDA | `RGon = 4 ohm`, `RGoff = 2.4 ohm`, `RGS = 10k` | Requerido |
| `Cgs` gate-source | `220 pF` | Se deja footprint y no se monta inicialmente | Recomendado |

### 1.1 Orden de trabajo en Altium

El diseno avanza en este orden:

1. Primero se corrigen las fuentes aisladas de gate.
2. Se confirma que cada fuente entrega `18 V` entre su positivo aislado y su tierra aislada.
3. Se coloca un `UCC21540DW` por cada rama del puente H.
4. `VDDA/VSSA` se conectan al MOSFET high-side de esa rama.
5. `VDDB/VSSB` se conectan al MOSFET low-side de esa rama.
6. La red de gate se cambia a `RGon = 4 ohm`, `RGoff = 2.4 ohm`, `RGS = 10k` y `Cgs = DNP`.
7. La parte digital del TIDA se mantiene casi igual, verificando que `DISABLE` sea activo alto.
8. Se agregan puntos de prueba para medir cada `18V_x/GND_x` antes de conectar bus alto.

Regla principal:

```text
Primero se valida la fuente aislada.
Despues se valida el driver.
Al final se conecta el modulo SiC con bus DC bajo y corriente limitada.
```

## 2. Datos de datasheet que gobiernan el diseno

### 2.1 Modulo SiC MSCSM120HM16CT3AG

Datos relevantes por MOSFET:

| Parametro | Valor | Consecuencia de diseno |
| --- | ---: | --- |
| Tipo | SiC MOSFET, puente H completo | Se requieren 4 salidas de compuerta |
| `VDSS` | `1200 V` | Layout y aislamiento importan mucho |
| `VGS` absoluto | `-10 V` a `+25 V` | El gate no supera esos limites |
| `RDS(on)` | `12.5 mohm` tipico, `16 mohm` max a `VGS = 20 V` | El MOSFET esta caracterizado con gate alto |
| `VGS(th)` | `1.8 V` a `2.8 V` | No es tension de manejo, solo umbral |
| `Qg` | `464 nC` | Dimensiona fuente y driver |
| `Qgd` | `100 nC` | Importa para encendido Miller por `dv/dt` |
| `Ciss` | `6040 pF` | Carga capacitiva alta |
| `RGint` | `2.94 ohm` | Se suma a resistencia externa |
| Valores de prueba dinamica | `RGon = 4 ohm`, `RGoff = 2.4 ohm` | Punto inicial razonable |

El modulo fue caracterizado con `VGS = -5 V / +20 V`. En esta primera version no se hara fuente bipolar; se usara:

```text
VGS_ON  = +18 V
VGS_OFF = 0 V
```

Esto queda dentro del limite absoluto `-10 V` a `+25 V`. No es identico a la condicion de caracterizacion del datasheet, pero es una primera version simple y compatible con el `UCC21540DW`.

### 2.2 Driver UCC21540DW

Datos relevantes:

| Parametro | Valor | Consecuencia |
| --- | ---: | --- |
| Canales | 2 | Un driver puede manejar una rama high-side/low-side |
| Corriente source pico | `4 A` | Limite de carga de gate |
| Corriente sink pico | `6 A` | Limite de descarga de gate |
| `VDDA - VSSA`, `VDDB - VSSB` | `18 V` es valido | Compatible con fuente de gate de 18 V |
| UVLO salida version UCC21540 | alrededor de `8 V` | Si la fuente cae, apaga la salida |
| `DISABLE` | activo alto | `1` apaga, `0` habilita |
| `DT` | `tDT(ns) = 10 * RDT(kohm)` | `20k` produce aprox `200 ns` |
| Capacitor recomendado en `DT` | `1 nF` | Mejora inmunidad a ruido |

### 2.3 Driver de transformador SN6505B

Datos relevantes:

| Parametro | Valor | Consecuencia |
| --- | ---: | --- |
| `VCC` | `2.25 V` a `5.5 V` | Se alimenta desde `5 V`; no subir este pin |
| Frecuencia SN6505B | aprox `424 kHz` tipica | Transformador debe ser compatible |
| Corriente recomendada en switches | hasta `1 A` por `D1/D2` | Dimensiona potencia de fuente |
| Limite de corriente interno | aprox `1.75 A` tipico | No se usa como corriente normal |
| Topologia | push-pull con primario center-tap | Requiere transformador compatible |

## 3. Arquitectura electrica del puente H

El puente H tiene dos ramas. Cada rama se maneja con un `UCC21540DW`.

```text
Rama A:
  UCC21540_A OUTA -> gate high-side A
  UCC21540_A OUTB -> gate low-side A

Rama B:
  UCC21540_B OUTA -> gate high-side B
  UCC21540_B OUTB -> gate low-side B
```

Motivo: el dead-time interno del `UCC21540` actua entre `OUTA` y `OUTB`. Por eso esos dos canales deben corresponder a los MOSFETs superior e inferior de la misma rama.

### Alimentaciones aisladas necesarias

Minimo conceptual para un puente H:

```text
18V_HS_A / GND_HS_A -> high-side rama A
18V_HS_B / GND_HS_B -> high-side rama B
18V_LS   / GND_LS   -> low-sides, si ambos sources low-side comparten DC-
```

Opcion mas modular:

```text
18V_HS_A / GND_HS_A
18V_LS_A / GND_LS_A
18V_HS_B / GND_HS_B
18V_LS_B / GND_LS_B
```

Para esta etapa de diseno se mantiene la idea modular: cada dominio de gate debe tener una referencia clara. Lo mas importante es que cada `VSSx` del driver vaya a la referencia de source del MOSFET que maneja.

## 4. Cambio 1: fuente aislada de 12 V a 18 V

### 4.1 Objetivo

La plantilla TIDA alimenta el driver con `12 V`. Para el modulo SiC se requiere una tension de gate mayor, por lo que se define:

```text
Fuente aislada objetivo = 18 V
```

Esta fuente alimenta el lado de salida del `UCC21540`:

```text
VDDA - VSSA = 18 V
VDDB - VSSB = 18 V
```

### 4.2 Analisis: por que cambiar de 12 V a 18 V

La fuente original del TIDA era de `12 V`, pero el modulo `MSCSM120HM16CT3AG` no esta caracterizado como interruptor de potencia a `12 V`. Los datos importantes del datasheet son:

| Dato | Valor | Como se usa en el diseno |
| --- | ---: | --- |
| `VGS(th)` | `1.8 V` a `2.8 V` | Solo indica donde el MOSFET empieza a conducir poca corriente |
| `VGS` absoluto | `-10 V` a `+25 V` | Limite que no se supera |
| `RDS(on)` | `12.5 mohm` tipico, `16 mohm` max | Especificado con `VGS = 20 V` |
| `Qg` | `464 nC` | Medido con `VGS = -5 V / +20 V`; se usa conservadoramente para dimensionar fuente y driver |
| `RGint` | `2.94 ohm` | Se suma a la resistencia externa de gate |

Comparacion electrica usando:

```text
VGS(th)_max = 2.8 V
RGon  = 4 ohm
RGoff = 2.4 ohm
RGint = 2.94 ohm
Qg    = 464 nC
fsw   = 20 kHz
```

| Criterio | Formula | Caso 12 V | Caso 18 V | Lectura |
| --- | --- | ---: | ---: | --- |
| Margen sobre umbral | `Vdrive - VGS(th)_max` | `9.2 V` | `15.2 V` | `18 V` tiene `1.65x` mas margen de manejo |
| Corriente pico de encendido | `Vdrive / (RGon + RGint)` | `1.73 A` | `2.59 A` | Ambos caben en `4 A source` del driver |
| Corriente pico de apagado | `Vdrive / (RGoff + RGint)` | `2.25 A` | `3.37 A` | Ambos caben en `6 A sink` del driver |
| Corriente promedio de gate | `Qg * fsw` | `9.28 mA` | `9.28 mA` | Calculo conservador usando `Qg` de datasheet |
| Potencia de gate por MOSFET | `Qg * Vdrive * fsw` | `0.111 W` | `0.167 W` | `18 V` exige mas a la fuente, pero sigue siendo manejable |
| Potencia de gate, 4 MOSFETs | `4 * Pgate` | `0.445 W` | `0.668 W` | La diferencia total es aprox `0.223 W` |
| Capacitor local minimo | `(Qg + IVDD/fsw) / 0.5 V` | `1.18 uF` | `1.18 uF` | Se usan `4.7 uF + 100 nF` en ambos casos |

La corriente promedio de gate se calcula igual en la tabla porque se usa el `Qg = 464 nC` del datasheet como peor caso. En la practica, con `12 V` el `Qg` real puede ser menor, pero eso no soluciona el problema principal: el MOSFET queda menos mejorado y puede aumentar `RDS(on)`.

El datasheet entrega `RDS(on)` a `VGS = 20 V`, no entrega en la tabla principal el valor exacto a `VGS = 12 V`. Por eso `12 V` no se aprueba como condicion final de potencia sin curva o medicion. Para ver el impacto, se usa una estimacion de sensibilidad:

```text
RDS(on)_18V ~ 16 mohm   aproximacion conservadora cerca de condicion de datasheet
RDS(on)_12V ~ 24 a 27 mohm si sube 1.5x a 1.7x
Pcond = I^2 * RDS(on)
```

| Corriente por MOSFET | Perdida con 18 V, `16 mohm` | Perdida estimada con 12 V, `24-27 mohm` | Diferencia por MOSFET |
| ---: | ---: | ---: | ---: |
| `10 A` | `1.6 W` | `2.4 W` a `2.7 W` | `+0.8 W` a `+1.1 W` |
| `20 A` | `6.4 W` | `9.6 W` a `10.8 W` | `+3.2 W` a `+4.4 W` |
| `40 A` | `25.6 W` | `38.4 W` a `43.2 W` | `+12.8 W` a `+17.6 W` |

Conclusion de esta comparacion:

| Opcion | Ventaja | Desventaja | Decision |
| --- | --- | --- | --- |
| `12 V` | Menor consumo en gate y menor exigencia a la fuente | MOSFET menos mejorado, posible mayor `RDS(on)`, mas calor en potencia | Sirve para pruebas suaves, no como punto final |
| `18 V` | Mas cerca de `VGS = 20 V` de datasheet, menor perdida por conduccion esperada | Mayor consumo de gate y conmutacion mas fuerte | Punto inicial recomendado |
| `20 V` | Mas cercano a caracterizacion del datasheet | Menor margen contra limite absoluto `+25 V` | Posible mejora futura con buen control de transitorios |
| `+20 V / -5 V` | Mejor inmunidad contra encendido Miller | Requiere fuente bipolar mas compleja | Mejora futura si las pruebas lo exigen |

Decision del proyecto:

```text
Se usa +18 V / 0 V como primera version de gate drive.
No se usa 12 V como alimentacion final de potencia.
Se deja opcion futura de gate bipolar si aparece encendido falso por dv/dt.
```

### 4.3 Restriccion del SN6505B

El `SN6505B` no se alimenta con mas de `5 V`.

```text
Conexion correcta:    VCC_SN6505B = 5 V
Conexion incorrecta:  VCC_SN6505B = 18 V
```

La subida de tension se logra con el transformador y la rectificacion, no subiendo el pin `VCC` del `SN6505B`.

### 4.4 Correccion critica sobre el transformador 750342879

El transformador `750342879` esta pensado para obtener una salida alrededor de `17 V` a `17.5 V` desde `5 V` con `SN6505B`.

Por eso:

```text
750342879 + rectificacion estandar SN6505B -> salida ~17 V a 18 V
```

Pero si se copia exactamente el rectificador doblador del TIDA y ademas se usa `750342879`, la salida puede tender a ser mucho mayor que `18 V`. En ese caso el zener `MMSZ5250B` de `20 V` conduciria de forma continua, se calentaria y la fuente quedaria mal disenada.

Decision:

```text
Se usa 750342879 con topologia de rectificacion adecuada para salida ~18 V.
No se usa como si fuera el transformador 1:1.3 del TIDA en una etapa dobladora sin recalcular.
```

### 4.5 Topologia recomendada para la fuente de 18 V

Con el `750342879`, la topologia recomendada es la rectificacion estandar de secundario con center-tap:

```text
Secundario pin 1 -> anodo D1
Secundario pin 3 -> anodo D3
catodos D1/D3 unidos -> +18V_ISO
Secundario pin 2 -> GND_ISO
Cout entre +18V_ISO y GND_ISO
Zener 20 V entre +18V_ISO y GND_ISO
```

Conexion del zener:

```text
catodo MMSZ5250B -> +18V_ISO
anodo  MMSZ5250B -> GND_ISO
```

Si en el esquematico actual el secundario todavia esta conectado como la red del TIDA con dos capacitores apilados, se corrige esa topologia o se cambia el transformador por uno de menor relacion.

### 4.6 Componentes seleccionados para la fuente

| Funcion | Componente | Valor/rating | Revision |
| --- | --- | --- | --- |
| Driver transformador | `SN6505BDBVR` | `5 V`, push-pull, ~`420 kHz` | OK |
| Transformador | `750342879` | primario `5 V`, salida aprox `17 V`, aislamiento y pinout a confirmar con hoja del fabricante | Aprobado condicional: requiere topologia correcta |
| Diodos rectificadores | `SS16` | Schottky `60 V`, `1 A` | OK para salida 18 V |
| Clamp | `MMSZ5250B` | zener `20 V`, SOD-123 | OK como proteccion, no como regulador continuo |
| Capacitores salida | MLCC | `50 V` recomendado | OK |
| Carga minima opcional | `10k` | `1.8 mA`, `32 mW` | Recomendado |

### 4.7 Calculo de corriente de salida requerida

A `20 kHz`, por MOSFET:

```text
Igate_promedio = Qg * fsw
Igate_promedio = 464 nC * 20 kHz
Igate_promedio = 9.28 mA
```

Consumo adicional aproximado del canal de salida del `UCC21540`:

```text
IVDD_max_aprox = 2.5 mA
```

Corriente minima por dominio de gate:

```text
Imin = 9.28 mA + 2.5 mA
Imin = 11.78 mA
```

Criterio de diseno:

```text
Fuente por dominio de gate: minimo 30 mA
Margen recomendado: 50 mA
```

Si el transformador seleccionado esta especificado para alrededor de `100 mA`, hay margen suficiente para una primera version a `20 kHz`.

### 4.8 Carga minima

Para que la fuente no quede completamente en vacio:

```text
Rload_min = 10k
I = 18 V / 10k = 1.8 mA
P = 18 V * 1.8 mA = 32.4 mW
```

Se usa resistencia `0603` de `0.1 W` minimo o mayor.

### 4.9 Verificacion obligatoria en banco

Antes de conectar al `UCC21540`, se miden:

```text
Vout sin carga
Vout con 10k
Vout con 30 mA
Vout con 50 mA
rizado de salida
temperatura del SN6505B
temperatura del transformador
temperatura de diodos SS16
si el zener conduce continuamente
```

Condicion de aceptacion:

```text
Vout debe estar cerca de 18 V bajo carga.
El zener de 20 V no debe regular de forma permanente.
```

## 5. Cambio 2: alimentacion y desacoplo del UCC21540

### 5.1 Lado logico

Se conserva la parte digital de la plantilla TIDA:

```text
VCCI -> 3.3 V o 5 V
GND  -> tierra logica
INA  -> PWM high-side
INB  -> PWM low-side
DIS  -> senal de apagado/habilitacion
DT   -> resistencia de dead-time
```

Valores recomendados:

| Bloque | Valor |
| --- | --- |
| `VCCI` | `3.3 V` o `5 V` |
| capacitor `VCCI-GND` | `100 nF` cerca del pin |
| filtro PWM serie | `33 ohm` o `51 ohm` |
| filtro PWM a GND | `33 pF` o `47 pF` inicial |
| `DIS` capacitor | `1 nF` cerca del pin |
| `RDT` | `20k` |
| `CDT` | `1 nF` |

Nota:

```text
DIS = 1 -> salidas apagadas
DIS = 0 -> salidas habilitadas
```

Si la red se llama `ENABLE`, se verifica si esta invertida.

#### 5.1.1 Analisis de resistencias y capacitores del lado logico

La parte logica del TIDA se puede conservar como arquitectura, pero los valores se revisan para no filtrar demasiado las senales PWM.

| Bloque | Valor TIDA / referencia | Valor recomendado inicial | Calculo o criterio | Decision |
| --- | --- | --- | --- | --- |
| Desacoplo `VCCI` | `100 nF` | `100 nF` cerca de pines `3/8` a `GND` | capacitor local de alta frecuencia para el lado logico | Se conserva |
| Resistencia serie PWM | depende de plantilla | `33 ohm` a `51 ohm` | limita ringing y corriente de entrada sin crear retardo grande | Se cambia si el TIDA usa valores altos |
| Capacitor PWM a GND | `220 pF` en TIDA | `33 pF` a `47 pF` inicial | menor retardo y menor distorsion de PWM | Recomendado |
| `RDT` | `20k` | `20k` | `tDT(ns) = 10 * RDT(kohm)` | Se conserva |
| `CDT` | capacitor pequeno | `1 nF` | recomendado para inmunidad de ruido en `DT` | Se conserva |
| `DISABLE` pull/red RC | TIDA | Se revisa polaridad | `DIS = 1` apaga, `DIS = 0` habilita | Se verifica nombre `ENABLE` |

Calculo del filtro PWM recomendado:

```text
tau = R * C
fc = 1 / (2*pi*R*C)
```

| R serie | C a GND | `tau` | `fc` aproximada | Comentario |
| ---: | ---: | ---: | ---: | --- |
| `33 ohm` | `33 pF` | `1.09 ns` | `146 MHz` | Filtra ruido muy alto, casi no deforma PWM |
| `51 ohm` | `47 pF` | `2.40 ns` | `66 MHz` | Buen punto inicial |
| `51 ohm` | `220 pF` | `11.2 ns` | `14.2 MHz` | Mas lento; se usa solo si el ruido lo exige |
| `100 ohm` | `220 pF` | `22 ns` | `7.23 MHz` | Evitar si no hay razon de ruido |

Decision:

```text
Para PWM se usan inicialmente 33R/33pF o 51R/47pF.
No se usan capacitores grandes en PWM sin medir retardo y forma de onda.
```

Calculo de dead-time:

```text
tDT(ns) = 10 * RDT(kohm)
tDT = 10 * 20 = 200 ns
```

Resultado:

| `RDT` | `tDT` aproximado | Decision |
| ---: | ---: | --- |
| `20k` | `200 ns` | Valor inicial razonable para pruebas |

El dead-time final debe validarse con osciloscopio midiendo las compuertas reales. Si hay shoot-through, se aumenta. Si hay mucha conduccion por diodo/body diode, se optimiza.

### 5.2 Lado de potencia del driver

Por cada medio puente:

```text
VDDA -> +18V_HS
VSSA -> source del high-side

VDDB -> +18V_LS
VSSB -> source del low-side
```

Cada alimentacion debe tener desacoplo local junto al `UCC21540`, no solo en la fuente:

```text
100 nF + 4.7 uF entre VDDA y VSSA
100 nF + 4.7 uF entre VDDB y VSSB
```

Rating recomendado:

```text
50 V
```

### 5.3 Calculo minimo del capacitor local de VDD

La carga que debe entregar el capacitor en un evento de conmutacion es aproximadamente:

```text
Qtotal ~= Qg + (IVDD / fsw)
```

A `20 kHz`:

```text
Qg = 464 nC
IVDD = 2.5 mA
fsw = 20 kHz

IVDD/fsw = 2.5 mA / 20 kHz = 125 nC
Qtotal = 464 nC + 125 nC = 589 nC
```

Si se permite una caida local de `0.5 V`:

```text
Cmin = Qtotal / DeltaV
Cmin = 589 nC / 0.5 V
Cmin = 1.18 uF
```

Decision:

```text
Se usan 4.7 uF + 100 nF por cada VDDA/VSSA y VDDB/VSSB.
```

Esto deja margen frente a perdida de capacitancia por DC bias en MLCC.

Tabla de seleccion de capacitores:

| Capacitor | Funcion | Valor | Rating | Criterio |
| --- | --- | ---: | ---: | --- |
| `100 nF` | alta frecuencia, pico local del driver | `0.1 uF` | `50 V` | debe ir pegado a `VDDx/VSSx` |
| `4.7 uF` | reserva local de carga de gate | `4.7 uF` | `50 V` | mayor que `Cmin = 1.18 uF` |
| `10 uF` en fuente aislada | almacenamiento despues de rectificacion/filtro | `10 uF` | `50 V` | reduce rizado de la fuente |

Regla de layout:

```text
El capacitor local del UCC21540 no reemplaza al capacitor de la fuente.
La fuente necesita sus capacitores, y el driver necesita sus propios capacitores pegados al integrado.
```

## 6. Cambio 3: red de gate para el modulo SiC

La red de gate de TIDA no se copia con sus valores originales porque estaba ajustada para otros MOSFETs.

Valores iniciales para el modulo `MSCSM120HM16CT3AG`:

```text
RGon  = 4 ohm
RGoff = 2.4 ohm
RGS   = 10k
Cgs   = DNP inicial, con footprint disponible
```

Conexion por MOSFET:

```text
OUTx -> RGon -> Gate
Gate -> diodo + RGoff -> OUTx
Gate -> RGS -> Source
```

El diodo debe orientarse para que:

```text
encendido -> use RGon = 4 ohm
apagado   -> use RGoff = 2.4 ohm
```

### 6.1 Calculo de corriente pico: 12 V vs 18 V

Estimacion simple, sin incluir la resistencia interna completa del driver:

```text
Igate_on  = Vdrive / (RGon + RGint)
Igate_off = Vdrive / (RGoff + RGint)
```

Con:

```text
RGon   = 4 ohm
RGoff  = 2.4 ohm
RGint  = 2.94 ohm
```

Tabla de calculo:

| `Vdrive` | `Igate_on = V/(4 + 2.94)` | Limite source UCC21540 | `Igate_off = V/(2.4 + 2.94)` | Limite sink UCC21540 | Resultado |
| ---: | ---: | ---: | ---: | ---: | --- |
| `12 V` | `1.73 A` | `4 A` | `2.25 A` | `6 A` | OK, pero menor fuerza de manejo |
| `18 V` | `2.59 A` | `4 A` | `3.37 A` | `6 A` | OK, mejor manejo del SiC |

La corriente real sera menor por resistencia interna del driver y parasitos de layout, asi que estos valores son aceptables como punto inicial.

### 6.2 Calculo de potencia de gate: 12 V vs 18 V

Por MOSFET:

```text
Pgate = Qg * Vdrive * fsw
```

Con:

```text
Qg = 464 nC
```

| Frecuencia | `Igate_prom = Qg*fsw` | `Pgate` por MOSFET a `12 V` | `Pgate` 4 MOSFETs a `12 V` | `Pgate` por MOSFET a `18 V` | `Pgate` 4 MOSFETs a `18 V` |
| ---: | ---: | ---: | ---: | ---: | ---: |
| `20 kHz` | `9.28 mA` | `0.111 W` | `0.445 W` | `0.167 W` | `0.668 W` |
| `50 kHz` | `23.2 mA` | `0.278 W` | `1.114 W` | `0.418 W` | `1.672 W` |
| `100 kHz` | `46.4 mA` | `0.557 W` | `2.227 W` | `0.835 W` | `3.340 W` |

Lectura:

```text
18 V aumenta la potencia de gate 50 % frente a 12 V.
Ese aumento es pequeno comparado con la posible reduccion de perdida por conduccion del MOSFET.
```

Decision inicial:

```text
fsw inicial sugerida = 20 kHz
```

## 7. Checklist de correccion del esquematico

### Fuente aislada

- `VCC` del `SN6505B` se mantiene en `5 V`.
- `750342879` se usa solo con topologia que entregue alrededor de `18 V`.
- Si se conserva la topologia dobladora del TIDA, se recalcula el transformador; `750342879` no se usa sin cambiar la rectificacion.
- `SS16` se usa como Schottky equivalente de al menos `60 V`.
- `MMSZ5250B` se conecta con catodo al positivo y anodo a GND aislado.
- Los capacitores de salida se seleccionan de `50 V`.
- La carga minima opcional queda en `10k`.

### Driver UCC21540

- Se usa un `UCC21540DW` por rama.
- `INA` corresponde al high-side de la rama.
- `INB` corresponde al low-side de la rama.
- `DIS` es activo alto.
- `RDT = 20k`.
- `CDT = 1 nF`.
- Filtro PWM inicial: `33R/33pF` o `51R/47pF`.
- Evitar `220 pF` en PWM como valor por defecto si no se mide el retardo.
- `VCCI` con `100 nF` cerca del pin.
- `VDDA/VSSA` y `VDDB/VSSB` con `100 nF + 4.7 uF`.

### Red de gate

- Los valores TIDA se cambian por `RGon = 4 ohm` y `RGoff = 2.4 ohm`.
- `RGS` se mantiene en `10k`.
- `Cgs` queda como DNP inicial.
- Se verifica la orientacion de los diodos de gate.
- `VSSx` se conecta al source de su MOSFET, idealmente al pin Kelvin/source de senal si el modulo lo ofrece.

## 8. Criterio de layout

El lazo critico es:

```text
capacitor VDDx-VSSx -> UCC21540 -> OUTx -> resistencia gate -> gate -> source -> VSSx -> capacitor
```

Prioridades:

1. `UCC21540` cerca del modulo.
2. Resistencias de gate cerca del gate o en ruta muy corta.
3. Capacitores `VDDx-VSSx` pegados al driver.
4. Retorno de source separado del retorno de potencia siempre que el modulo lo permita.
5. No se coloca cobre bajo la barrera de aislamiento del driver.
6. Se mantiene distancia entre dominios high-side, low-side y logica.

## 9. Estado de decisiones

| Decision | Valor actual | Estado |
| --- | --- | --- |
| Gate drive inicial | `+18 V / 0 V` | Aprobado para primera version |
| Gate drive `12 V / 0 V` | Solo pruebas suaves | No aprobado como punto final de potencia |
| Fuente aislada | `SN6505B + 750342879` | Aprobado condicional: se corrige topologia y se confirma pinout |
| Rectificadores | `SS16` | Aprobado |
| Clamp | `MMSZ5250B`, `20 V` | Aprobado como proteccion |
| Driver | `UCC21540DW` | Aprobado |
| Filtro PWM | `33R/33pF` o `51R/47pF` | Aprobado inicial |
| `RGon` | `4 ohm` | Aprobado inicial |
| `RGoff` | `2.4 ohm` | Aprobado inicial |
| `RGS` | `10k` | Aprobado |
| `Cgs` | DNP | Aprobado inicial |
| `RDT/CDT` | `20k / 1 nF` | Aprobado |
| Frecuencia inicial | `20 kHz` | Aprobado para pruebas |

## 10. Pendientes antes de fabricar PCB

- Se corrige o confirma la topologia de la fuente de `18 V` con `750342879`.
- Se confirma el pinout real del secundario del `750342879` en el simbolo de Altium.
- Se confirma footprint y polaridad de `MMSZ5250B`.
- Se confirma footprint y orientacion de `SS16`.
- Definir si las fuentes low-side seran compartidas o separadas por rama.
- Validar que el modulo tenga pines Kelvin/source de senal y que estos se usen para `VSSx`.
- La fuente aislada se mide en banco antes de conectar el driver.
- Probar el driver inicialmente sin bus alto, con fuente limitada y carga controlada.

## 11. Fuentes consultadas

- Datasheet local: `C:\Users\juanv\Downloads\Microsemi_MSCSM120HM16CT3AG_Full_BridgeSiC_MOSFET_Power_Module_Rv1.0.pdf`
- Datasheet local: `C:\Users\juanv\Downloads\ucc21540.pdf`
- Datasheet local: `C:\Users\juanv\Downloads\sn6505b.pdf`
- Esquematico TIDA local: `C:\Users\juanv\Downloads\TIDA-010933 Schematic.PDF`
- BOM TIDA local: `C:\Users\juanv\Downloads\TIDA-010933 BOM-PDF.PDF`
