#ifndef __L298N_SPWM_H
#define __L298N_SPWM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

/* Inicializa el control del L298N y deja el puente apagado. */
void L298N_SPWM_Init(void);
/* Arranca el modo normal SPWM usado para la senal alterna de 50 Hz. */
void L298N_SPWM_Start(void);
/* Detiene el modo SPWM, apaga ENA y deja IN1/IN2 en bajo. */
void L298N_SPWM_Stop(void);
/* Actualiza periodicamente el duty senoidal y la polaridad del puente. */
void L298N_SPWM_Task(void);
/* Ajusta la amplitud de la SPWM: 1000U = 100%, 800U = 80%. */
void L298N_SPWM_SetModulation(uint16_t modulation_permille);

#ifdef __cplusplus
}
#endif

#endif /* __L298N_SPWM_H */
