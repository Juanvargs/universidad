#include "l298n_spwm.h"
#include "tim.h"

/* Timer que genera el PWM en PC0/A5, conectado al pin ENA del L298N. */
#define L298N_PWM_TIMER              htim1
/* Canal del timer usado por PC0: TIM1_CH1. */
#define L298N_PWM_CHANNEL            TIM_CHANNEL_1
/* Cantidad de muestras usadas para dibujar un periodo completo de la senal. */
#define L298N_SPWM_TABLE_SIZE        100U
/* Tiempo entre una muestra y la siguiente: 200 us. */
#define L298N_SPWM_UPDATE_US         200U
/* Periodo completo de salida: 20000 us = 20 ms = 50 Hz. */
#define L298N_OUTPUT_PERIOD_US       20000U
/* Medio periodo: 10000 us = 10 ms. Aqui se invierten IN1 e IN2. */
#define L298N_HALF_PERIOD_US         10000U
/* Modulacion inicial: 800U significa 80% del duty maximo. */
#define L298N_DEFAULT_MODULATION     800U

/*
 * Tabla senoidal absoluta.
 *
 * Los valores van de 0 a 1000, donde 1000 representa 100% antes de aplicar
 * la modulacion. Se usa valor absoluto porque la polaridad la hacen IN1/IN2.
 */
static const uint16_t spwm_abs_sine_table[L298N_SPWM_TABLE_SIZE] = {
  0U, 63U, 125U, 187U, 249U, 309U, 368U, 426U, 482U, 536U,
  588U, 637U, 685U, 729U, 771U, 809U, 844U, 876U, 905U, 930U,
  951U, 969U, 982U, 992U, 998U, 1000U, 998U, 992U, 982U, 969U,
  951U, 930U, 905U, 876U, 844U, 809U, 771U, 729U, 685U, 637U,
  588U, 536U, 482U, 426U, 368U, 309U, 249U, 187U, 125U, 63U,
  0U, 63U, 125U, 187U, 249U, 309U, 368U, 426U, 482U, 536U,
  588U, 637U, 685U, 729U, 771U, 809U, 844U, 876U, 905U, 930U,
  951U, 969U, 982U, 992U, 998U, 1000U, 998U, 992U, 982U, 969U,
  951U, 930U, 905U, 876U, 844U, 809U, 771U, 729U, 685U, 637U,
  588U, 536U, 482U, 426U, 368U, 309U, 249U, 187U, 125U, 63U
};

/* Indica si el modo SPWM esta activo. */
static uint8_t spwm_enabled = 0U;
/* Porcentaje de modulacion en permil: 800U = 80%. */
static uint16_t spwm_modulation_permille = L298N_DEFAULT_MODULATION;
/* Guarda la ultima muestra aplicada para no repetir escrituras innecesarias. */
static uint16_t spwm_last_sample = L298N_SPWM_TABLE_SIZE;
/* Tiempo inicial del periodo SPWM, en microsegundos. */
static uint32_t spwm_start_us = 0U;
/* Estado actual del puente: 1 positivo, 0 negativo, 2 fuerza actualizacion. */
static uint8_t bridge_positive = 2U;

static void DWT_DelayCounter_Init(void)
{
  /* Habilita el contador de ciclos del Cortex-M para medir microsegundos. */
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

static uint32_t Micros(void)
{
  /* Convierte ciclos de CPU a microsegundos usando la frecuencia HCLK. */
  return DWT->CYCCNT / (HAL_RCC_GetHCLKFreq() / 1000000U);
}

static uint32_t PWM_PeriodCounts(void)
{
  /* Devuelve la cantidad de cuentas del periodo PWM configurado en TIM1. */
  return __HAL_TIM_GET_AUTORELOAD(&L298N_PWM_TIMER) + 1U;
}

static void SetBridgePolarity(uint8_t positive_half_cycle)
{
  /*
   * Cambia la polaridad del puente A:
   * 1U -> IN1 alto e IN2 bajo.
   * 0U -> IN1 bajo e IN2 alto.
   */
  if (bridge_positive == positive_half_cycle)
  {
    /* Si ya esta en esa polaridad, no hace nada. */
    return;
  }

  /* Guarda la nueva polaridad para evitar repetir la misma escritura. */
  bridge_positive = positive_half_cycle;

  if (positive_half_cycle != 0U)
  {
    /* Semiciclo positivo: OUT1 y OUT2 quedan en un sentido. */
    HAL_GPIO_WritePin(L298_IN1_GPIO_Port, L298_IN1_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(L298_IN2_GPIO_Port, L298_IN2_Pin, GPIO_PIN_RESET);
  }
  else
  {
    /* Semiciclo negativo: se invierte el sentido del puente. */
    HAL_GPIO_WritePin(L298_IN1_GPIO_Port, L298_IN1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(L298_IN2_GPIO_Port, L298_IN2_Pin, GPIO_PIN_SET);
  }
}

static void SetDutyPermille(uint16_t duty_permille)
{
  /* Limita el duty para que nunca pase de 1000U = 100%. */
  if (duty_permille > 1000U)
  {
    duty_permille = 1000U;
  }

  /* Escribe el duty en el registro CCR del PWM de TIM1. */
  __HAL_TIM_SET_COMPARE(&L298N_PWM_TIMER,
                        L298N_PWM_CHANNEL,
                        (PWM_PeriodCounts() * duty_permille) / 1000U);
}

void L298N_SPWM_Init(void)
{
  /* Prepara el contador de tiempo usado por la tarea SPWM. */
  DWT_DelayCounter_Init();
  /* Fuerza que la primera escritura de polaridad se haga aunque coincida. */
  bridge_positive = 2U;
  /* Arranca con ENA en 0% para que el L298N no entregue potencia. */
  SetDutyPermille(0U);
  /* Deja IN1 e IN2 apagados mientras se decide el modo de trabajo. */
  HAL_GPIO_WritePin(L298_IN1_GPIO_Port, L298_IN1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(L298_IN2_GPIO_Port, L298_IN2_Pin, GPIO_PIN_RESET);
}

void L298N_SPWM_Start(void)
{
  /*
   * MODO NORMAL SPWM:
   * Arranca la modulacion senoidal. IN1/IN2 se invierten cada 10 ms
   * dentro de L298N_SPWM_Task(), formando una salida de 50 Hz.
   */
  spwm_start_us = Micros();
  /* Obliga a aplicar la primera muestra de la tabla. */
  spwm_last_sample = L298N_SPWM_TABLE_SIZE;
  /* Fuerza la actualizacion inicial de IN1/IN2. */
  bridge_positive = 2U;
  /* Activa la tarea SPWM que corre dentro del while(1). */
  spwm_enabled = 1U;
  /* Inicia en el semiciclo positivo. */
  SetBridgePolarity(1U);
  /* Arranca fisicamente el PWM en PC0/A5 hacia ENA. */
  HAL_TIM_PWM_Start(&L298N_PWM_TIMER, L298N_PWM_CHANNEL);
}

void L298N_SPWM_Stop(void)
{
  /* Desactiva la tarea SPWM. */
  spwm_enabled = 0U;
  /* Fuerza que la proxima vez se pueda actualizar la polaridad. */
  bridge_positive = 2U;
  /* Pone ENA en 0% antes de detener el timer. */
  SetDutyPermille(0U);
  /* Apaga IN1 e IN2 para dejar el puente deshabilitado logicamente. */
  HAL_GPIO_WritePin(L298_IN1_GPIO_Port, L298_IN1_Pin, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(L298_IN2_GPIO_Port, L298_IN2_Pin, GPIO_PIN_RESET);
  /* Detiene la salida PWM del timer. */
  HAL_TIM_PWM_Stop(&L298N_PWM_TIMER, L298N_PWM_CHANNEL);
}

void L298N_SPWM_Task(void)
{
  uint16_t duty_permille;
  uint32_t now_us;
  uint32_t phase_us;
  uint16_t sample;

  if (spwm_enabled == 0U)
  {
    /* Si el modo SPWM no esta activo, no se actualiza nada. */
    return;
  }

  /* Calcula el tiempo actual desde el contador DWT. */
  now_us = Micros();
  /* Ubica el instante dentro del periodo de 20 ms. */
  phase_us = (uint32_t)(now_us - spwm_start_us) % L298N_OUTPUT_PERIOD_US;
  /* Convierte el tiempo del periodo en indice de la tabla senoidal. */
  sample = (uint16_t)(phase_us / L298N_SPWM_UPDATE_US);

  if (sample >= L298N_SPWM_TABLE_SIZE)
  {
    /* Proteccion por si el calculo cae justo al final de la tabla. */
    sample = L298N_SPWM_TABLE_SIZE - 1U;
  }

  /* En los primeros 10 ms usa una polaridad; en los otros 10 ms la invierte. */
  SetBridgePolarity(phase_us < L298N_HALF_PERIOD_US);

  if (sample == spwm_last_sample)
  {
    /* Si seguimos en la misma muestra, no cambia el duty. */
    return;
  }
  /* Guarda la nueva muestra aplicada. */
  spwm_last_sample = sample;

  /* Aplica el factor de modulacion a la muestra senoidal. */
  duty_permille = (spwm_abs_sine_table[sample] * spwm_modulation_permille) / 1000U;
  /* Actualiza el duty del PWM que va a ENA. */
  SetDutyPermille(duty_permille);
}

void L298N_SPWM_SetModulation(uint16_t modulation_permille)
{
  /* Limita la modulacion a 1000U = 100%. */
  if (modulation_permille > 1000U)
  {
    modulation_permille = 1000U;
  }

  /* Guarda la modulacion que usara L298N_SPWM_Task(). */
  spwm_modulation_permille = modulation_permille;
}
