# Checklist de integracion y validacion

Este checklist organiza el paso de firmware y diseno hacia pruebas reales. La
idea es validar por capas, sin conectar bus alto antes de comprobar control,
fuentes, aislamiento y protecciones.

## 1. Antes de energizar potencia

- Confirmar que `DISEÑO` contiene el esquematico/BOM final correspondiente a la
  version que se va a probar.
- Confirmar referencias de sensores de corriente y voltaje.
- Confirmar escalas analogicas hacia ADC: ganancia, divisor, offset y limite de
  3.3 V de la MCU.
- Confirmar que `UCC_DISABLE` apaga realmente todos los drivers.
- Confirmar polaridad activa de `DISABLE` en `UCC21540DWR`.
- Confirmar que cada dominio aislado tiene nombre y retorno correcto.

## 2. Fuente aislada de gate

- Alimentar `SN6505BDBVR` solo desde `5 V`.
- Validar que `750342879` se use con topologia de rectificacion adecuada para
  salida alrededor de `18 V`.
- Medir salida sin carga.
- Medir salida con carga minima `10k`.
- Medir salida con carga aproximada de `30 mA`.
- Medir salida con carga aproximada de `50 mA`.
- Verificar rizado de salida.
- Verificar temperatura de `SN6505BDBVR`, transformador y diodos.
- Confirmar que el zener `MMSZ5250B` no regula permanentemente.

## 3. Driver UCC21540DWR

- Verificar `VCCI` logico.
- Verificar `VDDA/VSSA` y `VDDB/VSSB` por canal.
- Verificar desacoplos `100 nF + 4.7 uF` cerca del driver.
- Verificar `RDT = 20k` y `CDT = 1 nF`.
- Verificar que `INA/INB` correspondan al high-side y low-side de la misma rama.
- Verificar orientacion de red `RGon/RGoff`.
- Probar salida del driver sin bus DC alto.

## 4. PWM desde STM32F030C8T6

- Medir `PWM_A_H` en `PA8`.
- Medir `PWM_A_L` en `PA7`.
- Medir `PWM_B_H` en `PA9`.
- Medir `PWM_B_L` en `PB0`.
- Confirmar frecuencia de portadora cercana a `125 kHz`.
- Confirmar senal fundamental de referencia de `60 Hz`.
- Confirmar que los pares complementarios no se solapan.
- Confirmar efecto real del dead-time de `TIM1` mas el del `UCC21540`.
- Confirmar que el firmware inicia con `UCC_DISABLE` activo.

## 5. ADC y sensores

- Leer cada canal ADC con entrada conocida.
- Confirmar orden de canales:

```text
PA0 -> ADC_VDC_BUS_1
PA1 -> ADC_IDC_BUS
PA2 -> ADC_VOUT_1
PA3 -> ADC_IOUT_FILTER
PA4 -> ADC_IOUT
PA5 -> ADC_VDC_BUS_2
PA6 -> ADC_VOUT_2
```

- Determinar formula de conversion a voltios/amperios.
- Validar offset de sensores de corriente.
- Definir umbrales de proteccion.
- Probar apagado por falla usando `UCC_DISABLE`.

## 6. Prueba de potencia gradual

- Primera prueba sin modulo SiC conectado, solo senales.
- Segunda prueba con driver conectado y alimentaciones aisladas verificadas.
- Tercera prueba con bus DC bajo y corriente limitada.
- Usar carga controlada.
- Medir compuertas gate-source, no solo senales logicas.
- Medir salida diferencial.
- Revisar temperatura del modulo, drivers, transformadores y sensores.
- Aumentar tension/corriente solo si la etapa anterior fue estable.

## Criterio minimo para avanzar

```text
No subir bus DC si:
- UCC_DISABLE no apaga.
- Hay solape entre high-side y low-side.
- Una fuente aislada no regula alrededor de 18 V.
- El zener de fuente conduce continuamente.
- No estan definidas las escalas ADC de proteccion.
```
