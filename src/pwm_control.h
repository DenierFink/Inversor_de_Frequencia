/**
 * @file pwm_control.h
 * @brief Controle PWM para o inversor de frequência trifásico
 */

#ifndef __PWM_CONTROL_H
#define __PWM_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include "stm32f0xx_hal.h"
#include <stdint.h>

/* Defines */
#define PWM_MAX_CARRIER_FREQ  20000  /* 20kHz */
#define PWM_MIN_CARRIER_FREQ  4000   /* 4kHz */

/* Public functions */
void PWMControl_Init(TIM_HandleTypeDef *htim);
void PWMControl_SetOutputs(uint16_t phaseU, uint16_t phaseV, uint16_t phaseW);
void PWMControl_SetCarrierFreq(uint32_t freqHz);
void PWMControl_Enable(void);
void PWMControl_Disable(void);

#ifdef __cplusplus
}
#endif

#endif /* __PWM_CONTROL_H */
