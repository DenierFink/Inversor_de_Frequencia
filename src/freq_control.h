/**
 * @file freq_control.h
 * @brief Controle de frequência para o inversor trifásico
 */

#ifndef __FREQ_CONTROL_H
#define __FREQ_CONTROL_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include <stdint.h>

/* Defines */
#define FREQ_MIN  0.1f   /* Minimum frequency in Hz */
#define FREQ_MAX  50.0f  /* Maximum frequency in Hz */

/* Public functions */
void FreqControl_Init(void);
uint8_t FreqControl_SetFrequency(float freqHz);
float FreqControl_GetFrequency(void);
uint8_t FreqControl_Start(void);
uint8_t FreqControl_Stop(void);
uint8_t FreqControl_IsRunning(void);
void FreqControl_Update(void);

#ifdef __cplusplus
}
#endif

#endif /* __FREQ_CONTROL_H */
