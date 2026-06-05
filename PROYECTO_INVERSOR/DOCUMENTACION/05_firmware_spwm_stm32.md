# Firmware SPWM para STM32F030C8T6

Fecha: 2026-06-05  
Proyecto de firmware: `INVERSOR_SPWM_125K`  
MCU: `STM32F030C8T6`  
Objetivo: generar SPWM bipolar para un puente H monofasico usando `TIM1`

## 1. Objetivo del firmware

El firmware de la `STM32F030C8T6` se encarga de generar las senales de control
para el puente H del inversor. La salida deseada es una senal senoidal de `60 Hz`
obtenida mediante modulacion SPWM bipolar sobre una portadora PWM de `125 kHz`.
La MCU no maneja directamente potencia; solo entrega senales logicas hacia los
drivers `UCC21540`, que luego manejan las compuertas del modulo de potencia.

La funcion principal del firmware en esta etapa es:

```text
Generar PWM complementario con dead-time
Actualizar el duty con una referencia senoidal
Mantener los drivers deshabilitados durante el arranque
Preparar las entradas ADC para medicion y protecciones
```

## 2. Archivos principales del firmware

| Archivo | Funcion |
| --- | --- |
| `INVERSOR_SPWM_125K.ioc` | Configuracion CubeMX del microcontrolador. |
| `Src/main.c` | Inicializacion general del sistema y llamada al modulo SPWM. |
| `Src/tim.c` | Configuracion de `TIM1`, PWM complementario y dead-time. |
| `Src/adc.c` | Configuracion de los canales ADC disponibles. |
| `Src/gpio.c` | Configuracion de `UCC_DISABLE`. |
| `Src/spwm_inverter.c` | Logica propia de generacion SPWM. |
| `Inc/spwm_inverter.h` | Interfaz publica del modulo SPWM. |

El codigo propio del inversor se separa en `spwm_inverter.c/.h` para evitar
mezclar la logica de control con los archivos generados por CubeMX.

## 3. Pines usados

### 3.1 Senales PWM hacia los drivers

| Senal | Pin STM32 | Funcion timer | Uso |
| --- | --- | --- | --- |
| `PWM_A_H` | `PA8` | `TIM1_CH1` | High-side rama A. |
| `PWM_A_L` | `PA7` | `TIM1_CH1N` | Low-side rama A. |
| `PWM_B_H` | `PA9` | `TIM1_CH2` | High-side rama B. |
| `PWM_B_L` | `PB0` | `TIM1_CH2N` | Low-side rama B. |

`TIM1` es un timer avanzado. Por eso puede generar salidas complementarias
`CHx/CHxN` y aplicar dead-time por hardware. Esto es importante porque las
salidas superior e inferior de una misma rama no deben encenderse al mismo
tiempo.

### 3.2 Pin de habilitacion de drivers

| Senal | Pin STM32 | Tipo | Logica |
| --- | --- | --- | --- |
| `UCC_DISABLE` | `PA10` | GPIO salida | `1` apaga drivers, `0` habilita drivers. |

El `UCC21540` usa una entrada `DISABLE` activa en alto. Por seguridad, el
firmware inicia este pin en alto:

```text
PA10 = HIGH -> drivers apagados
PA10 = LOW  -> drivers habilitados
```

La generacion PWM puede probarse primero con osciloscopio manteniendo
`UCC_DISABLE` en alto. Solo despues de validar las cuatro senales PWM se debe
llamar a `SPWM_Inverter_EnablePowerStage()`.

## 4. Configuracion de TIM1

La configuracion actual de `TIM1` es:

| Parametro | Valor |
| --- | ---: |
| Reloj del timer | `48 MHz` |
| Prescaler | `0` |
| Periodo `ARR` | `383` |
| Cuentas por periodo | `384` |
| Frecuencia PWM | `125 kHz` |
| Pulse inicial | `192` |
| Duty inicial | `50 %` |
| Dead-time TIM1 | `10` cuentas de configuracion |

La frecuencia PWM se obtiene asi:

```text
fPWM = 48 MHz / (ARR + 1)
fPWM = 48 MHz / 384
fPWM = 125 kHz
```

El duty inicial de `50 %` se logra con:

```text
CCR = 192
ARR + 1 = 384
duty = 192 / 384 = 0.5
```

## 5. Principio de SPWM bipolar

Para un puente H bipolar se generan dos referencias opuestas, una para cada
rama:

```text
duty_A = 0.5 + 0.5 * m * sin(wt)
duty_B = 0.5 - 0.5 * m * sin(wt)
```

Donde:

| Simbolo | Significado |
| --- | --- |
| `m` | Indice de modulacion. |
| `w` | Velocidad angular de la fundamental. |
| `t` | Tiempo. |
| `duty_A` | Duty aplicado a la rama A. |
| `duty_B` | Duty aplicado a la rama B. |

Para `60 Hz`:

```text
w = 2 * pi * 60
```

El firmware no calcula `sin()` en tiempo real. En su lugar usa una tabla seno
de `256` muestras en formato Q15. Esto reduce carga de CPU y evita depender de
operaciones de punto flotante, algo importante en una `STM32F030` sin FPU.

## 6. Indice de modulacion actual

El firmware inicia con:

```c
#define SPWM_MODULATION_PERMILLE 800U
```

Esto equivale a:

```text
m = 0.8
```

Por tanto, el duty de cada rama queda entre:

```text
duty_min = 0.5 - 0.5 * 0.8 = 0.1 = 10 %
duty_mid = 0.5             = 50 %
duty_max = 0.5 + 0.5 * 0.8 = 0.9 = 90 %
```

Con `384` cuentas por periodo:

```text
10 % -> CCR aprox 38
50 % -> CCR aprox 192
90 % -> CCR aprox 346
```

El modulo limita la modulacion maxima a `950 permille` para no llegar a los
extremos absolutos del PWM.

## 7. Como se actualiza la senoide

La actualizacion se hace sincronizada con el evento de actualizacion de `TIM1`.
Cada vez que el timer completa un periodo PWM, entra al callback:

```c
HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
```

Si el timer que genero la interrupcion es `TIM1`, el firmware llama internamente
a la rutina que aplica la siguiente muestra SPWM:

```text
1. Lee la fase actual.
2. Convierte la fase en indice de tabla seno.
3. Calcula el delta respecto al centro de duty.
4. Escribe CCR1 para la rama A.
5. Escribe CCR2 para la rama B.
6. Avanza la fase.
```

La fase se maneja con un acumulador de `32 bits`, parecido a una tecnica DDS.
Esto permite obtener `60 Hz` a partir de una actualizacion de `125 kHz` sin
necesitar que el numero de muestras por ciclo sea entero.

La relacion aproximada es:

```text
125000 / 60 = 2083.33 actualizaciones por ciclo de 60 Hz
```

Como no es un numero entero, el acumulador de fase evita errores acumulados de
frecuencia.

## 8. Relacion entre CCR1 y CCR2

`CCR1` maneja la rama A:

```text
CCR1 -> TIM1_CH1  / TIM1_CH1N
CCR1 -> PWM_A_H   / PWM_A_L
```

`CCR2` maneja la rama B:

```text
CCR2 -> TIM1_CH2  / TIM1_CH2N
CCR2 -> PWM_B_H   / PWM_B_L
```

En cada muestra:

```text
CCR1 = centro + delta
CCR2 = centro - delta
```

Cuando la senoide es positiva, la rama A aumenta duty y la rama B lo reduce.  
Cuando la senoide es negativa, la rama A reduce duty y la rama B lo aumenta.
Despues del filtro de salida, la diferencia entre ambas ramas forma la senal AC.

## 9. Secuencia de arranque segura

La secuencia implementada es:

```text
1. Inicializar HAL y reloj del sistema.
2. Inicializar GPIO.
3. Inicializar TIM1.
4. Inicializar ADC.
5. Llamar SPWM_Inverter_Init().
6. Fijar modulacion inicial.
7. Arrancar PWM con SPWM_Inverter_Start().
8. Mantener UCC_DISABLE en alto.
```

El firmware arranca las senales PWM, pero no habilita automaticamente los
drivers. Esto es intencional para permitir pruebas de baja energia:

```text
Medir PA8, PA7, PA9 y PB0 con osciloscopio
Confirmar dead-time y polaridad
Confirmar que UCC_DISABLE permanece alto
Despues habilitar la etapa de potencia manualmente
```

La funcion que habilita los drivers es:

```c
SPWM_Inverter_EnablePowerStage();
```

No debe llamarse hasta validar las senales en banco.

## 10. Entradas ADC para sensores

Las entradas analogicas configuradas son:

| Senal | Pin STM32 | Canal ADC | Uso previsto |
| --- | --- | --- | --- |
| `ADC_VDC_BUS_1` | `PA0` | `ADC_IN0` | Medicion diferencial/aislada del bus DC, punto 1. |
| `ADC_IDC_BUS` | `PA1` | `ADC_IN1` | Corriente del bus DC. |
| `ADC_VOUT_1` | `PA2` | `ADC_IN2` | Medicion de salida, punto 1. |
| `ADC_IOUT_FILTER` | `PA3` | `ADC_IN3` | Corriente o senal filtrada de salida. |
| `ADC_IOUT` | `PA4` | `ADC_IN4` | Corriente de salida. |
| `ADC_VDC_BUS_2` | `PA5` | `ADC_IN5` | Medicion diferencial/aislada del bus DC, punto 2. |
| `ADC_VOUT_2` | `PA6` | `ADC_IN6` | Medicion de salida, punto 2. |

En esta version del firmware los canales ADC quedan configurados, pero todavia
no se implementa la tarea de lectura, conversion a unidades fisicas ni las
protecciones automaticas.

La interpretacion esperada para mediciones diferenciales es:

```text
VDC_BUS = ADC_VDC_BUS_1 - ADC_VDC_BUS_2
VOUT    = ADC_VOUT_1    - ADC_VOUT_2
```

Para sensores de corriente analogicos:

```text
IDC_BUS = funcion(ADC_IDC_BUS)
IOUT    = funcion(ADC_IOUT o ADC_IOUT_FILTER)
```

La funcion exacta depende del sensor, divisor, ganancia y offset usados en el
esquematico.

## 11. Protecciones pendientes

El siguiente paso de firmware sera usar las mediciones ADC para apagar el puente
ante condiciones peligrosas. Las protecciones previstas son:

| Proteccion | Senal base | Accion esperada |
| --- | --- | --- |
| Sobretension de bus DC | `ADC_VDC_BUS_1`, `ADC_VDC_BUS_2` | Poner `UCC_DISABLE` en alto. |
| Subtension de bus DC | `ADC_VDC_BUS_1`, `ADC_VDC_BUS_2` | No habilitar potencia. |
| Sobrecorriente de bus | `ADC_IDC_BUS` | Apagar drivers inmediatamente. |
| Sobrecorriente de salida | `ADC_IOUT`, `ADC_IOUT_FILTER` | Apagar drivers inmediatamente. |
| Error de salida | `ADC_VOUT_1`, `ADC_VOUT_2` | Registrar falla o reducir modulacion. |

La accion critica de seguridad sera:

```c
SPWM_Inverter_DisablePowerStage();
```

Esta funcion pone `UCC_DISABLE` en alto y apaga las salidas del `UCC21540`.

## 12. Diferencia con la prueba L298N

La prueba anterior con `L298N` usaba una senal PWM y dos GPIO para invertir
polaridad. En el inversor real no se usa esa tecnica porque el puente H de
potencia requiere control complementario con dead-time.

| Aspecto | Prueba L298N | Inversor real |
| --- | --- | --- |
| PWM usado | Un PWM hacia `ENA`. | Cuatro salidas derivadas de `TIM1`. |
| Polaridad | Dos GPIO `IN1/IN2`. | Dos ramas PWM opuestas. |
| Timer | PWM simple. | Timer avanzado con `CHx/CHxN`. |
| Dead-time | No critico en el L298N. | Critico para evitar shoot-through. |
| Fundamental | Tabla y cambio de polaridad. | Tabla seno bipolar con `CCR1/CCR2`. |

## 13. Pruebas recomendadas antes de conectar potencia

Antes de habilitar drivers o bus DC alto:

1. Programar la STM32 con `UCC_DISABLE` conectado.
2. Verificar que `PA10` inicia en alto.
3. Medir `PA8` y `PA7`; deben ser complementarias con dead-time.
4. Medir `PA9` y `PB0`; deben ser complementarias con dead-time.
5. Confirmar que la envolvente del duty cambia a `60 Hz`.
6. Confirmar que `PWM_A` y `PWM_B` estan en oposicion.
7. Medir primero sin alimentar el bus de potencia.
8. Luego alimentar drivers con bajo riesgo y mantener bus DC limitado.

No se debe llamar a `SPWM_Inverter_EnablePowerStage()` hasta completar esas
pruebas.

## 14. Pendientes de implementacion

- Agregar lectura ADC periodica.
- Definir escalas fisicas de cada sensor.
- Implementar conversion de cuentas ADC a voltios y amperios.
- Definir umbrales de proteccion.
- Implementar estado de falla bloqueante.
- Decidir si la lectura ADC se sincroniza con `TIM1` o con una tarea mas lenta.
- Agregar modo de arranque suave aumentando `m` progresivamente.
- Agregar opcion de cambiar frecuencia fundamental si se requiere `50 Hz`.

## 15. Resumen

El firmware actual genera una SPWM bipolar de `60 Hz` usando `TIM1` a `125 kHz`.
La rama A se controla con `CCR1` y la rama B con `CCR2`. El timer genera
automaticamente las salidas complementarias `CH1N` y `CH2N`, aplicando dead-time
por hardware. La modulacion inicial es `m = 0.8`, por lo que el duty oscila
aproximadamente entre `10 %` y `90 %`. Los ADC ya estan asignados para sensores,
pero aun falta implementar lectura, conversion y protecciones.
