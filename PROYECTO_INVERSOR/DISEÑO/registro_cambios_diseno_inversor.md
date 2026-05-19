# Registro de cambios de diseno del inversor

Fecha: 2026-05-19

## 1. Para que sirve este documento

Este archivo es el registro maestro de decisiones del esquematico del inversor basado en el modulo SiC `MSCSM120HM16CT3AG`.

Cada cambio importante debe quedar documentado aqui con:

- que se cambio;
- por que se cambio;
- que dato de datasheet lo justifica;
- que calculo se uso;
- que se debe revisar en Altium y en banco.

La primera revision documenta la adaptacion de la plantilla del microinverter TIDA al puente H SiC usando:

- driver de compuerta `UCC21540DW`;
- fuente aislada con `SN6505B`;
- transformador `750342879`;
- rectificadores `SS16`;
- zener clamp `MMSZ5250B`;
- alimentacion de gate inicial de `+18 V / 0 V`.

## 2. Resumen ejecutivo de cambios

La plantilla TIDA no se copia literalmente. Se conserva la arquitectura general, pero se cambian valores y algunos criterios para que sirvan al modulo SiC.

| Bloque | Plantilla TIDA | Cambio para este inversor | Estado |
| --- | --- | --- | --- |
| Fuente de gate | `12 V` | `18 V` | Requerido |
| Transformador fuente | `PAD002-T764113S`, `1:1.3` | `750342879` | Requerido |
| Rectificador de fuente | topologia TIDA | Revisar topologia: `750342879` debe usarse como salida ~17.5 V, no como doblador TIDA | Correccion critica |
| Diodos rectificadores | `MBR0520`, `20 V` | `SS16`, `60 V`, `1 A` | Requerido |
| Zener clamp | `16 V` | `MMSZ5250B`, `20 V` | Requerido |
| Capacitores de salida fuente | `25 V` | `50 V` recomendado | Requerido |
| Driver | `UCC21540DW` | Se conserva | OK |
| `DT` | `20k` + capacitor | `20k` + `1 nF` | OK |
| Entrada PWM | filtro de TIDA | usar filtro mas pequeno: `33R/51R` + `33pF/47pF` | Recomendado |
| Red de gate | valores TIDA | `RGon = 4 ohm`, `RGoff = 2.4 ohm`, `RGS = 10k` | Requerido |
| `Cgs` gate-source | `220 pF` | dejar footprint, no montar inicialmente | Recomendado |

### 2.1 Orden de trabajo en Altium

Para no perderse, el diseno debe avanzar en este orden:

1. Corregir primero las fuentes aisladas de gate.
2. Confirmar que cada fuente entrega `18 V` entre su positivo aislado y su tierra aislada.
3. Colocar un `UCC21540DW` por cada rama del puente H.
4. Conectar `VDDA/VSSA` al MOSFET high-side de esa rama.
5. Conectar `VDDB/VSSB` al MOSFET low-side de esa rama.
6. Cambiar la red de gate a `RGon = 4 ohm`, `RGoff = 2.4 ohm`, `RGS = 10k` y `Cgs = DNP`.
7. Mantener la parte digital del TIDA casi igual, pero verificando que `DISABLE` sea activo alto.
8. Agregar puntos de prueba para medir cada `18V_x/GND_x` antes de conectar bus alto.

Regla principal:

```text
Primero se valida la fuente aislada.
Despues se valida el driver.
Al final se conecta el modulo SiC con bus DC bajo y corriente limitada.
```

## 3. Datos de datasheet que gobiernan el diseno

### 3.1 Modulo SiC MSCSM120HM16CT3AG

Datos relevantes por MOSFET:

| Parametro | Valor | Consecuencia de diseno |
| --- | ---: | --- |
| Tipo | SiC MOSFET, puente H completo | Se requieren 4 salidas de compuerta |
| `VDSS` | `1200 V` | Layout y aislamiento importan mucho |
| `VGS` absoluto | `-10 V` a `+25 V` | El gate nunca debe pasar esos limites |
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

### 3.2 Driver UCC21540DW

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

### 3.3 Driver de transformador SN6505B

Datos relevantes:

| Parametro | Valor | Consecuencia |
| --- | ---: | --- |
| `VCC` | `2.25 V` a `5.5 V` | Se alimenta desde `5 V`; no subir este pin |
| Frecuencia SN6505B | aprox `424 kHz` tipica | Transformador debe ser compatible |
| Corriente recomendada en switches | hasta `1 A` por `D1/D2` | Dimensiona potencia de fuente |
| Limite de corriente interno | aprox `1.75 A` tipico | No usar como corriente normal |
| Topologia | push-pull con primario center-tap | Requiere transformador compatible |

## 4. Arquitectura electrica del puente H

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

## 5. Cambio 1: fuente aislada de 12 V a 18 V

### 5.1 Objetivo

La plantilla TIDA alimenta el driver con `12 V`. Para el modulo SiC se requiere una tension de gate mayor, por lo que se define:

```text
Fuente aislada objetivo = 18 V
```

Esta fuente alimenta el lado de salida del `UCC21540`:

```text
VDDA - VSSA = 18 V
VDDB - VSSB = 18 V
```

### 5.2 Lo que NO se debe hacer

No se debe alimentar el `SN6505B` con mas de `5 V`.

```text
Correcto:  VCC_SN6505B = 5 V
Incorrecto: subir VCC_SN6505B a 18 V
```

La subida de tension se logra con el transformador y la rectificacion, no subiendo el pin `VCC` del `SN6505B`.

### 5.3 Correccion critica sobre el transformador 750342879

El transformador `750342879` esta pensado para obtener una salida alrededor de `17 V` a `17.5 V` desde `5 V` con `SN6505B`.

Por eso:

```text
750342879 + rectificacion estandar SN6505B -> salida ~17 V a 18 V
```

Pero si se copia exactamente el rectificador doblador del TIDA y ademas se usa `750342879`, la salida puede tender a ser mucho mayor que `18 V`. En ese caso el zener `MMSZ5250B` de `20 V` conduciria de forma continua, se calentaria y la fuente quedaria mal disenada.

Decision:

```text
Usar 750342879 con topologia de rectificacion adecuada para salida ~18 V.
No usarlo como si fuera el transformador 1:1.3 del TIDA en una etapa dobladora sin recalcular.
```

### 5.4 Topologia recomendada para la fuente de 18 V

Para usar el `750342879`, la topologia recomendada es la rectificacion estandar de secundario con center-tap:

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

Si en el esquematico actual el secundario todavia esta conectado como la red del TIDA con dos capacitores apilados, hay que corregirlo o cambiar el transformador por uno de menor relacion.

### 5.5 Componentes seleccionados para la fuente

| Funcion | Componente | Valor/rating | Revision |
| --- | --- | --- | --- |
| Driver transformador | `SN6505BDBVR` | `5 V`, push-pull, ~`420 kHz` | OK |
| Transformador | `750342879` | primario `5 V`, salida aprox `17 V`, aislamiento y pinout a confirmar con hoja del fabricante | Aprobado condicional: requiere topologia correcta |
| Diodos rectificadores | `SS16` | Schottky `60 V`, `1 A` | OK para salida 18 V |
| Clamp | `MMSZ5250B` | zener `20 V`, SOD-123 | OK como proteccion, no como regulador continuo |
| Capacitores salida | MLCC | `50 V` recomendado | OK |
| Carga minima opcional | `10k` | `1.8 mA`, `32 mW` | Recomendado |

### 5.6 Calculo de corriente de salida requerida

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

### 5.7 Carga minima

Para que la fuente no quede completamente en vacio:

```text
Rload_min = 10k
I = 18 V / 10k = 1.8 mA
P = 18 V * 1.8 mA = 32.4 mW
```

Usar resistencia `0603` de `0.1 W` minimo o mayor.

### 5.8 Verificacion obligatoria en banco

Antes de conectar al `UCC21540`, medir:

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

## 6. Cambio 2: alimentacion y desacoplo del UCC21540

### 6.1 Lado logico

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

Si la red se llama `ENABLE`, verificar si esta invertida.

### 6.2 Lado de potencia del driver

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

### 6.3 Calculo minimo del capacitor local de VDD

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
Usar 4.7 uF + 100 nF por cada VDDA/VSSA y VDDB/VSSB.
```

Esto deja margen frente a perdida de capacitancia por DC bias en MLCC.

## 7. Cambio 3: red de gate para el modulo SiC

La red de gate de TIDA no se copia con sus valores originales porque estaba ajustada para otros MOSFETs.

Valores iniciales para el modulo `MSCSM120HM16CT3AG`:

```text
RGon  = 4 ohm
RGoff = 2.4 ohm
RGS   = 10k
Cgs   = DNP inicial, dejar footprint
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

### 7.1 Calculo de corriente pico

Estimacion simple, sin incluir la resistencia interna completa del driver:

```text
Igate_on  = Vdrive / (RGon + RGint)
Igate_off = Vdrive / (RGoff + RGint)
```

Con:

```text
Vdrive = 18 V
RGon   = 4 ohm
RGoff  = 2.4 ohm
RGint  = 2.94 ohm
```

Resultados:

```text
Igate_on = 18 / (4 + 2.94) = 2.59 A
Igate_off = 18 / (2.4 + 2.94) = 3.37 A
```

Comparacion:

| Caso | Corriente estimada | Limite UCC21540 | Resultado |
| --- | ---: | ---: | --- |
| Encendido | `2.59 A` | `4 A source` | OK |
| Apagado | `3.37 A` | `6 A sink` | OK |

La corriente real sera menor por resistencia interna del driver y parasitos de layout, asi que estos valores son aceptables como punto inicial.

### 7.2 Calculo de potencia de gate

Por MOSFET:

```text
Pgate = Qg * Vdrive * fsw
```

Con:

```text
Qg = 464 nC
Vdrive = 18 V
```

| Frecuencia | Corriente promedio gate | Potencia por MOSFET | Potencia 4 MOSFETs |
| ---: | ---: | ---: | ---: |
| `20 kHz` | `9.28 mA` | `0.167 W` | `0.668 W` |
| `50 kHz` | `23.2 mA` | `0.418 W` | `1.672 W` |
| `100 kHz` | `46.4 mA` | `0.835 W` | `3.340 W` |

Decision inicial:

```text
fsw inicial sugerida = 20 kHz
```

## 8. Checklist para corregir el esquematico

### Fuente aislada

- No subir `VCC` del `SN6505B`; mantenerlo en `5 V`.
- Usar `750342879` solo con topologia que entregue alrededor de `18 V`.
- Si se conserva la topologia dobladora del TIDA, recalcular transformador; no usar `750342879` sin cambiar la rectificacion.
- Usar `SS16` o Schottky equivalente de al menos `60 V`.
- Usar `MMSZ5250B` con catodo al positivo y anodo a GND aislado.
- Usar capacitores de salida de `50 V`.
- Dejar carga minima opcional `10k`.

### Driver UCC21540

- Usar un `UCC21540DW` por rama.
- `INA` corresponde al high-side de la rama.
- `INB` corresponde al low-side de la rama.
- `DIS` es activo alto.
- `RDT = 20k`.
- `CDT = 1 nF`.
- `VCCI` con `100 nF` cerca del pin.
- `VDDA/VSSA` y `VDDB/VSSB` con `100 nF + 4.7 uF`.

### Red de gate

- Cambiar valores TIDA por `RGon = 4 ohm` y `RGoff = 2.4 ohm`.
- Mantener `RGS = 10k`.
- Dejar `Cgs` como DNP inicial.
- Verificar orientacion de los diodos de gate.
- Conectar `VSSx` al source de su MOSFET, idealmente al pin Kelvin/source de senal si el modulo lo ofrece.

## 9. Criterio de layout

El lazo critico es:

```text
capacitor VDDx-VSSx -> UCC21540 -> OUTx -> resistencia gate -> gate -> source -> VSSx -> capacitor
```

Prioridades:

1. `UCC21540` cerca del modulo.
2. Resistencias de gate cerca del gate o en ruta muy corta.
3. Capacitores `VDDx-VSSx` pegados al driver.
4. Retorno de source separado del retorno de potencia siempre que el modulo lo permita.
5. No colocar cobre bajo la barrera de aislamiento del driver.
6. Mantener distancia entre dominios high-side, low-side y logica.

## 10. Estado de decisiones

| Decision | Valor actual | Estado |
| --- | --- | --- |
| Gate drive inicial | `+18 V / 0 V` | Aprobado para primera version |
| Fuente aislada | `SN6505B + 750342879` | Aprobado condicional: corregir topologia y confirmar pinout |
| Rectificadores | `SS16` | Aprobado |
| Clamp | `MMSZ5250B`, `20 V` | Aprobado como proteccion |
| Driver | `UCC21540DW` | Aprobado |
| `RGon` | `4 ohm` | Aprobado inicial |
| `RGoff` | `2.4 ohm` | Aprobado inicial |
| `RGS` | `10k` | Aprobado |
| `Cgs` | DNP | Aprobado inicial |
| `RDT/CDT` | `20k / 1 nF` | Aprobado |
| Frecuencia inicial | `20 kHz` | Aprobado para pruebas |

## 11. Pendientes antes de fabricar PCB

- Corregir o confirmar topologia de la fuente de `18 V` con `750342879`.
- Confirmar pinout real del secundario del `750342879` en el simbolo de Altium.
- Confirmar footprint y polaridad de `MMSZ5250B`.
- Confirmar footprint y orientacion de `SS16`.
- Definir si las fuentes low-side seran compartidas o separadas por rama.
- Validar que el modulo tenga pines Kelvin/source de senal y usarlos para `VSSx`.
- Medir la fuente aislada en banco antes de conectar el driver.
- Probar el driver inicialmente sin bus alto, con fuente limitada y carga controlada.

## 12. Fuentes consultadas

- Datasheet local: `C:\Users\juanv\Downloads\Microsemi_MSCSM120HM16CT3AG_Full_BridgeSiC_MOSFET_Power_Module_Rv1.0.pdf`
- Datasheet local: `C:\Users\juanv\Downloads\ucc21540.pdf`
- Datasheet local: `C:\Users\juanv\Downloads\sn6505b.pdf`
- Esquematico TIDA local: `C:\Users\juanv\Downloads\TIDA-010933 Schematic.PDF`
- BOM TIDA local: `C:\Users\juanv\Downloads\TIDA-010933 BOM-PDF.PDF`
