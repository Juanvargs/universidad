# Componentes principales utilizados

## Prueba de concepto L298N

| Categoria | Componente | Funcion | Estado |
| --- | --- | --- | --- |
| MCU / tarjeta | `STM32 NUCLEO-G474RE` | Generacion SPWM y senales logicas. | Usado en prueba. |
| Puente H | Modulo `L298N` | Conmutacion de baja potencia. | Usado como prueba didactica. |
| Instrumentacion | `Analog Discovery 2` | Medicion de `OUT1`, `OUT2` y diferencial. | Usado en mediciones. |
| Software medicion | WaveForms | Visualizacion y filtro digital. | Usado en prueba. |
| Carga | `10 ohm / 10 W` | Carga resistiva. | Usada. |
| Filtro | Inductor `270 uH` | Filtrado de portadora. | Usado. |
| Filtro | 2 x capacitor `47 uF` espalda con espalda | Capacitor equivalente no polarizado de `23.5 uF`. | Usado. |
| Alimentacion | Fuente DC externa | Alimenta potencia del L298N. | Usada en baja tension. |
| Alimentacion logica | 5 V desde Nucleo | Alimenta logica del L298N. | Usada. |

## Proyecto real INVERSOR_SPWM_125K

| Categoria | Componente / senal | Funcion | Estado |
| --- | --- | --- | --- |
| MCU | `STM32F030C8T6` | Control SPWM, PWM complementario, ADC y proteccion. | Configurado en firmware. |
| Puente H de potencia | `MSCSM120HM16CT3AG` | Modulo SiC de puente H completo. | Seleccionado/documentado. |
| Driver de compuerta | `UCC21540DWR` | Driver aislado doble para high-side y low-side. | Seleccionado/documentado. |
| Driver fuente aislada | `SN6505BDBVR` | Excita transformador push-pull para fuentes aisladas. | Seleccionado/documentado. |
| Transformador fuente | `750342879` | Generacion de salida aislada alrededor de `17 V` a `18 V`. | Aprobado condicional a topologia y pinout. |
| Rectificadores fuente | `SS16` | Rectificacion Schottky de salida aislada. | Recomendado/documentado. |
| Clamp fuente | `MMSZ5250B` | Zener `20 V` de proteccion. | Recomendado/documentado. |
| Capacitores driver | `100 nF + 4.7 uF`, `50 V` | Desacoplo local de `VDDA/VSSA` y `VDDB/VSSB`. | Recomendado. |
| Red de gate | `RGon = 4 ohm`, `RGoff = 2.4 ohm`, `RGS = 10k`, `Cgs = DNP` | Control de velocidad de compuerta y descarga segura. | Punto inicial. |
| Sensor bus DC | `ADC_VDC_BUS_1`, `ADC_VDC_BUS_2` | Medicion de tension del bus DC. | Senales definidas; parte exacta por confirmar. |
| Sensor corriente bus | `ADC_IDC_BUS` | Medicion de corriente del bus. | Senal definida; parte exacta por confirmar. |
| Sensor salida | `ADC_VOUT_1`, `ADC_VOUT_2` | Medicion de tension diferencial de salida. | Senales definidas; parte exacta por confirmar. |
| Sensor corriente salida | `ADC_IOUT`, `ADC_IOUT_FILTER` | Medicion de corriente de salida/filtro. | Senales definidas; parte exacta por confirmar. |
| Habilitacion driver | `UCC_DISABLE` | Apagado global de drivers. | Implementado como GPIO. |

## Componentes por funcion

### Control

- `STM32G474RE`: usado para la prueba de concepto por disponibilidad de la
  tarjeta Nucleo.
- `STM32F030C8T6`: usado en el proyecto real, con pines dedicados a PWM
  complementario y medicion ADC.

### Potencia

- `L298N`: valido para demostracion de puente H de baja potencia.
- `MSCSM120HM16CT3AG`: componente de potencia real, puente H SiC completo,
  `1200 V`, alta corriente y NTC interno.

### Gate drive

- `UCC21540DWR`: driver aislado de dos canales, `4 A` source y `6 A` sink pico.
- `SN6505BDBVR`: driver push-pull para transformar 5 V en alimentaciones
  aisladas.
- `750342879`: transformador usado para obtener alrededor de `18 V` aislados.

### Medicion

La prueba de concepto midio con `Analog Discovery 2`. En el proyecto real las
mediciones pasan a la MCU mediante ADC:

```text
Bus DC:          ADC_VDC_BUS_1, ADC_VDC_BUS_2
Corriente bus:   ADC_IDC_BUS
Salida:          ADC_VOUT_1, ADC_VOUT_2
Corriente salida: ADC_IOUT, ADC_IOUT_FILTER
```

Las escalas, offset, ganancia y referencias exactas de sensores deben quedar
cerradas cuando se consulte el esquematico y la BOM final.
