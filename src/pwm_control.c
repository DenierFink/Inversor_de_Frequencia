/**
 * @file pwm_control.c
 * @brief Implementação do controle PWM para o inversor de frequência trifásico
 */

#include "pwm_control.h"

/* Private variables */
static TIM_HandleTypeDef* pwmTimer;
static uint8_t pwmEnabled = 0;

/**
 * @brief Initializes the PWM control module
 * @param htim Pointer to PWM timer handle
 * @retval None
 */
void PWMControl_Init(TIM_HandleTypeDef *htim)
{
  pwmTimer = htim;
  pwmEnabled = 0;
  
  /* Ensure PWM outputs are at zero */
  PWMControl_SetOutputs(0, 0, 0);
}

/**
 * @brief Sets the PWM duty cycle for all three phases
 * @param phaseU Phase U duty cycle value (0-1000)
 * @param phaseV Phase V duty cycle value (0-1000)
 * @param phaseW Phase W duty cycle value (0-1000)
 * @retval None
 */
void PWMControl_SetOutputs(uint16_t phaseU, uint16_t phaseV, uint16_t phaseW)
{
  /* Set PWM values for each phase */
  __HAL_TIM_SET_COMPARE(pwmTimer, TIM_CHANNEL_1, phaseU);
  __HAL_TIM_SET_COMPARE(pwmTimer, TIM_CHANNEL_2, phaseV);
  __HAL_TIM_SET_COMPARE(pwmTimer, TIM_CHANNEL_3, phaseW);
}

/**
 * @brief Sets the PWM carrier frequency
 * @param freqHz PWM carrier frequency in Hz
 * @retval None
 */
void PWMControl_SetCarrierFreq(uint32_t freqHz)
{
  uint32_t timerClock;
  uint32_t period;
  
  /* Limit the frequency to valid range */
  if (freqHz < PWM_MIN_CARRIER_FREQ)
    freqHz = PWM_MIN_CARRIER_FREQ;
  
  if (freqHz > PWM_MAX_CARRIER_FREQ)
    freqHz = PWM_MAX_CARRIER_FREQ;
  
  /* Get timer clock */
  timerClock = HAL_RCC_GetPCLK1Freq();
  
  /* Calculate timer period for desired frequency */
  /* In center-aligned mode, the frequency is timer_clock / (2 * period) */
  period = timerClock / (2 * freqHz);
  
  /* Update timer period */
  __HAL_TIM_SET_AUTORELOAD(pwmTimer, period);
}

/**
 * @brief Enables the PWM outputs
 * @retval None
 */
void PWMControl_Enable(void)
{
  if (!pwmEnabled)
  {
    /* Enable PWM outputs */
    HAL_TIM_PWM_Start(pwmTimer, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(pwmTimer, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(pwmTimer, TIM_CHANNEL_3);
    
    /* Enable timer main output */
    __HAL_TIM_MOE_ENABLE(pwmTimer);
    
    pwmEnabled = 1;
  }
}

/**
 * @brief Disables the PWM outputs
 * @retval None
 */
void PWMControl_Disable(void)
{
  if (pwmEnabled)
  {
    /* Disable PWM outputs */
    HAL_TIM_PWM_Stop(pwmTimer, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(pwmTimer, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop(pwmTimer, TIM_CHANNEL_3);
    
    /* Disable timer main output */
    __HAL_TIM_MOE_DISABLE(pwmTimer);
    
    pwmEnabled = 0;
  }
}
