# Resumen general del proyecto

## Proposito

El proyecto busca desarrollar un inversor monofasico controlado por SPWM. La
primera etapa fue una prueba de concepto de baja potencia usando un modulo
`L298N`. La segunda etapa es el inversor real, con puente H SiC, drivers
aislados, fuente auxiliar aislada y mediciones analogicas para control y
proteccion.

## Etapas de avance

| Etapa | Carpeta | Objetivo | Resultado |
| --- | --- | --- | --- |
| Prueba de concepto | `PRUEBA0_SPWM_L298N` | Validar generacion SPWM, puente H, salida diferencial y filtro LC. | Implementada y documentada con mediciones. |
| Proyecto real | `INVERSOR_SPWM_125K` | Escalar el control hacia un puente H SiC real con PWM complementario y sensores. | Firmware base creado; falta validar hardware completo. |
| Documentacion tecnica | `DOCUMENTACION` | Concentrar decisiones, componentes, calculos y checklists. | Organizada como indice de trabajo. |

## Arquitectura por etapas

### Prueba de concepto

```text
STM32 NUCLEO-G474RE
  -> SPWM en PC0/A5 hacia ENA
  -> PA8/D7 y PA9/D8 hacia IN1/IN2
  -> modulo L298N
  -> filtro LC y carga de prueba
  -> medicion diferencial con Analog Discovery 2
```

La prueba no pretendia manejar alta potencia. Su valor principal fue demostrar
que el control SPWM puede modular la energia promedio y que la polaridad del
puente H permite obtener una salida alterna diferencial.

### Proyecto real

```text
STM32F030C8T6
  -> TIM1 PWM complementario a 125 kHz
  -> UCC21540DWR por rama del puente H
  -> fuentes aisladas de gate con SN6505BDBVR + 750342879
  -> modulo SiC MSCSM120HM16CT3AG
  -> sensores de bus, salida y corriente hacia ADC
```

El proyecto real cambia el L298N por una etapa de potencia adecuada para mayor
tension y corriente. El microcontrolador ya no maneja un enable simple de un
puente integrado, sino senales complementarias para los drivers aislados de
compuerta.

## Reglas de continuidad

- Mantener la prueba `L298N` como referencia didactica, no como base de potencia.
- Usar `DOCUMENTACION` como fuente principal de lectura del proyecto.
- Mantener `INVERSOR_SPWM_125K` enfocado en firmware real.
- No modificar `DISEÑO` desde tareas de organizacion documental.
- Registrar cada decision electrica critica antes de pasar a pruebas con bus DC.

## Pendientes visibles

| Pendiente | Motivo |
| --- | --- |
| Confirmar referencias exactas de sensores de corriente y voltaje. | En firmware aparecen senales ADC, pero no part numbers. |
| Validar fuente aislada de `18 V`. | Es critica antes de conectar drivers y modulo SiC. |
| Validar polaridad y dead-time de cada salida PWM. | Evita conduccion simultanea en el puente H. |
| Confirmar escalas ADC y umbrales de proteccion. | Necesario para sobrecorriente, sobretension y control cerrado. |
| Definir procedimiento de energizacion gradual. | Reduce riesgo durante pruebas de potencia. |
