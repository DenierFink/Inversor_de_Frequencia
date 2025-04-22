/**
 * @file serial_comm.h
 * @brief Interface de comunicação serial para controle do inversor
 */

#ifndef __SERIAL_COMM_H
#define __SERIAL_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes */
#include "stm32f0xx_hal.h"
#include <stdint.h>

/* Defines */
#define SERIAL_BUFFER_SIZE 64

/* Public functions */
void SerialComm_Init(UART_HandleTypeDef *huart);
void SerialComm_Process(void);
void SerialComm_SendResponse(const char* message);

#ifdef __cplusplus
}
#endif

#endif /* __SERIAL_COMM_H */
