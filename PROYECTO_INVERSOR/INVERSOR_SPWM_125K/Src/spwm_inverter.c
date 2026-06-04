#include "spwm_inverter.h"
#include "tim.h"

#define SPWM_TIMER                         htim1
#define SPWM_PWM_FREQUENCY_HZ              125000UL
#define SPWM_OUTPUT_FREQUENCY_HZ           60UL
#define SPWM_DEFAULT_MODULATION_PERMILLE   800U
#define SPWM_MAX_MODULATION_PERMILLE       950U
#define SPWM_SINE_TABLE_BITS               8U
#define SPWM_SINE_TABLE_SIZE               (1U << SPWM_SINE_TABLE_BITS)
#define SPWM_PHASE_INDEX_SHIFT             (32U - SPWM_SINE_TABLE_BITS)
#define SPWM_Q15_FULL_SCALE                32767L
#define SPWM_COMPARE_GUARD_COUNTS          2U

static const int16_t spwm_sine_q15[SPWM_SINE_TABLE_SIZE] = {
       0,    804,   1608,   2410,   3212,   4011,   4808,   5602,
    6393,   7179,   7962,   8739,   9512,  10278,  11039,  11793,
   12539,  13279,  14010,  14732,  15446,  16151,  16846,  17530,
   18204,  18868,  19519,  20159,  20787,  21403,  22005,  22594,
   23170,  23731,  24279,  24811,  25329,  25832,  26319,  26790,
   27245,  27683,  28105,  28510,  28898,  29268,  29621,  29956,
   30273,  30571,  30852,  31113,  31356,  31580,  31785,  31971,
   32137,  32285,  32412,  32521,  32609,  32678,  32728,  32757,
   32767,  32757,  32728,  32678,  32609,  32521,  32412,  32285,
   32137,  31971,  31785,  31580,  31356,  31113,  30852,  30571,
   30273,  29956,  29621,  29268,  28898,  28510,  28105,  27683,
   27245,  26790,  26319,  25832,  25329,  24811,  24279,  23731,
   23170,  22594,  22005,  21403,  20787,  20159,  19519,  18868,
   18204,  17530,  16846,  16151,  15446,  14732,  14010,  13279,
   12539,  11793,  11039,  10278,   9512,   8739,   7962,   7179,
    6393,   5602,   4808,   4011,   3212,   2410,   1608,    804,
       0,   -804,  -1608,  -2410,  -3212,  -4011,  -4808,  -5602,
   -6393,  -7179,  -7962,  -8739,  -9512, -10278, -11039, -11793,
  -12539, -13279, -14010, -14732, -15446, -16151, -16846, -17530,
  -18204, -18868, -19519, -20159, -20787, -21403, -22005, -22594,
  -23170, -23731, -24279, -24811, -25329, -25832, -26319, -26790,
  -27245, -27683, -28105, -28510, -28898, -29268, -29621, -29956,
  -30273, -30571, -30852, -31113, -31356, -31580, -31785, -31971,
  -32137, -32285, -32412, -32521, -32609, -32678, -32728, -32757,
  -32767, -32757, -32728, -32678, -32609, -32521, -32412, -32285,
  -32137, -31971, -31785, -31580, -31356, -31113, -30852, -30571,
  -30273, -29956, -29621, -29268, -28898, -28510, -28105, -27683,
  -27245, -26790, -26319, -25832, -25329, -24811, -24279, -23731,
  -23170, -22594, -22005, -21403, -20787, -20159, -19519, -18868,
  -18204, -17530, -16846, -16151, -15446, -14732, -14010, -13279,
  -12539, -11793, -11039, -10278,  -9512,  -8739,  -7962,  -7179,
   -6393,  -5602,  -4808,  -4011,  -3212,  -2410,  -1608,   -804
};

static volatile uint8_t spwm_running = 0U;
static volatile uint32_t spwm_phase = 0UL;
static uint32_t spwm_phase_step = 0UL;
static uint16_t spwm_modulation_permille = SPWM_DEFAULT_MODULATION_PERMILLE;

static uint32_t CalculatePhaseStep(uint32_t output_frequency_hz)
{
  return (uint32_t)(((uint64_t)output_frequency_hz << 32) / SPWM_PWM_FREQUENCY_HZ);
}

static uint32_t ClampCompare(int32_t compare, uint32_t arr)
{
  const int32_t min_compare = (int32_t)SPWM_COMPARE_GUARD_COUNTS;
  const int32_t max_compare = (int32_t)arr - (int32_t)SPWM_COMPARE_GUARD_COUNTS;

  if (compare < min_compare)
  {
    return (uint32_t)min_compare;
  }

  if (compare > max_compare)
  {
    return (uint32_t)max_compare;
  }

  return (uint32_t)compare;
}

static void ApplyNeutralDuty(void)
{
  const uint32_t period_counts = __HAL_TIM_GET_AUTORELOAD(&SPWM_TIMER) + 1UL;
  const uint32_t center = period_counts / 2UL;

  __HAL_TIM_SET_COMPARE(&SPWM_TIMER, TIM_CHANNEL_1, center);
  __HAL_TIM_SET_COMPARE(&SPWM_TIMER, TIM_CHANNEL_2, center);
}

static void ApplyNextSpwmSample(void)
{
  const uint32_t arr = __HAL_TIM_GET_AUTORELOAD(&SPWM_TIMER);
  const uint32_t period_counts = arr + 1UL;
  const int32_t center = (int32_t)(period_counts / 2UL);
  const int32_t amplitude_limit = center - (int32_t)SPWM_COMPARE_GUARD_COUNTS;
  const uint32_t index = spwm_phase >> SPWM_PHASE_INDEX_SHIFT;
  const int32_t sine = (int32_t)spwm_sine_q15[index];
  const int32_t amplitude = (amplitude_limit * (int32_t)spwm_modulation_permille) / 1000L;
  const int32_t delta = (amplitude * sine) / SPWM_Q15_FULL_SCALE;

  __HAL_TIM_SET_COMPARE(&SPWM_TIMER, TIM_CHANNEL_1, ClampCompare(center + delta, arr));
  __HAL_TIM_SET_COMPARE(&SPWM_TIMER, TIM_CHANNEL_2, ClampCompare(center - delta, arr));

  spwm_phase += spwm_phase_step;
}

void SPWM_Inverter_Init(void)
{
  spwm_running = 0U;
  spwm_phase = 0UL;
  spwm_phase_step = CalculatePhaseStep(SPWM_OUTPUT_FREQUENCY_HZ);
  spwm_modulation_permille = SPWM_DEFAULT_MODULATION_PERMILLE;

  ApplyNeutralDuty();
  SPWM_Inverter_DisablePowerStage();
}

HAL_StatusTypeDef SPWM_Inverter_Start(void)
{
  HAL_StatusTypeDef status;

  SPWM_Inverter_DisablePowerStage();
  spwm_phase = 0UL;
  ApplyNeutralDuty();

  status = HAL_TIM_PWM_Start(&SPWM_TIMER, TIM_CHANNEL_1);
  if (status != HAL_OK)
  {
    return status;
  }

  status = HAL_TIMEx_PWMN_Start(&SPWM_TIMER, TIM_CHANNEL_1);
  if (status != HAL_OK)
  {
    return status;
  }

  status = HAL_TIM_PWM_Start(&SPWM_TIMER, TIM_CHANNEL_2);
  if (status != HAL_OK)
  {
    return status;
  }

  status = HAL_TIMEx_PWMN_Start(&SPWM_TIMER, TIM_CHANNEL_2);
  if (status != HAL_OK)
  {
    return status;
  }

  __HAL_TIM_CLEAR_FLAG(&SPWM_TIMER, TIM_FLAG_UPDATE);
  spwm_running = 1U;
  __HAL_TIM_ENABLE_IT(&SPWM_TIMER, TIM_IT_UPDATE);

  return HAL_OK;
}

void SPWM_Inverter_Stop(void)
{
  SPWM_Inverter_DisablePowerStage();
  spwm_running = 0U;
  __HAL_TIM_DISABLE_IT(&SPWM_TIMER, TIM_IT_UPDATE);
  __HAL_TIM_CLEAR_FLAG(&SPWM_TIMER, TIM_FLAG_UPDATE);
  ApplyNeutralDuty();

  (void)HAL_TIMEx_PWMN_Stop(&SPWM_TIMER, TIM_CHANNEL_2);
  (void)HAL_TIM_PWM_Stop(&SPWM_TIMER, TIM_CHANNEL_2);
  (void)HAL_TIMEx_PWMN_Stop(&SPWM_TIMER, TIM_CHANNEL_1);
  (void)HAL_TIM_PWM_Stop(&SPWM_TIMER, TIM_CHANNEL_1);
}

void SPWM_Inverter_EnablePowerStage(void)
{
  HAL_GPIO_WritePin(UCC_DISABLE_GPIO_Port, UCC_DISABLE_Pin, GPIO_PIN_RESET);
}

void SPWM_Inverter_DisablePowerStage(void)
{
  HAL_GPIO_WritePin(UCC_DISABLE_GPIO_Port, UCC_DISABLE_Pin, GPIO_PIN_SET);
}

void SPWM_Inverter_SetModulationPermille(uint16_t modulation_permille)
{
  if (modulation_permille > SPWM_MAX_MODULATION_PERMILLE)
  {
    modulation_permille = SPWM_MAX_MODULATION_PERMILLE;
  }

  spwm_modulation_permille = modulation_permille;
}

uint16_t SPWM_Inverter_GetModulationPermille(void)
{
  return spwm_modulation_permille;
}

uint8_t SPWM_Inverter_IsRunning(void)
{
  return spwm_running;
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if ((htim->Instance == TIM1) && (spwm_running != 0U))
  {
    ApplyNextSpwmSample();
  }
}
