#ifndef __SPWM_INVERTER_H
#define __SPWM_INVERTER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

void SPWM_Inverter_Init(void);
HAL_StatusTypeDef SPWM_Inverter_Start(void);
void SPWM_Inverter_Stop(void);
void SPWM_Inverter_EnablePowerStage(void);
void SPWM_Inverter_DisablePowerStage(void);
void SPWM_Inverter_SetModulationPermille(uint16_t modulation_permille);
uint16_t SPWM_Inverter_GetModulationPermille(void);
uint8_t SPWM_Inverter_IsRunning(void);

#ifdef __cplusplus
}
#endif

#endif /* __SPWM_INVERTER_H */
