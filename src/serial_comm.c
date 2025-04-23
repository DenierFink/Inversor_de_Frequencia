/**
 * @file serial_comm.c
 * @brief Implementação da comunicação serial para controle do inversor
 */

#include "serial_comm.h"
#include "freq_control.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h> // Para função sprintf
#include <ctype.h> // Para funções toupper/tolower

/* Private variables */
static UART_HandleTypeDef* uartHandle;
static uint8_t rxBuffer[SERIAL_BUFFER_SIZE];
static uint8_t cmdBuffer[SERIAL_BUFFER_SIZE];
static uint8_t rxIndex = 0;
static uint8_t rxComplete = 0;

/* Private function prototypes */
static void ProcessCommand(void);
static int str_case_compare(const char* s1, const char* s2);

/**
 * @brief Initializes the serial communication module
 * @param huart Pointer to UART handle
 * @retval None
 */
void SerialComm_Init(UART_HandleTypeDef *huart)
{
  uartHandle = huart;
  
  /* Start receiving data */
  HAL_UART_Receive_IT(uartHandle, &rxBuffer[0], 1);
}

/**
 * @brief Processes serial communication
 * @retval None
 */
void SerialComm_Process(void)
{
  /* Check if a complete command was received */
  if (rxComplete)
  {
    ProcessCommand();
    rxComplete = 0;
    rxIndex = 0;
  }
}

/**
 * @brief Sends response through serial port
 * @param message Message string to send
 * @retval None
 */
void SerialComm_SendResponse(const char* message)
{
  HAL_UART_Transmit(uartHandle, (uint8_t*)message, strlen(message), 100);
  HAL_UART_Transmit(uartHandle, (uint8_t*)"\r\n", 2, 10);
}

/**
 * @brief UART RX Complete callback
 * @param huart Pointer to UART handle
 * @retval None
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == uartHandle->Instance)
  {
    /* Check for newline character */
    if (rxBuffer[0] == '\r' || rxBuffer[0] == '\n')
    {
      /* Null-terminate the command string */
      cmdBuffer[rxIndex] = 0;
      rxComplete = 1;
    }
    else
    {
      /* Add character to command buffer */
      if (rxIndex < SERIAL_BUFFER_SIZE - 1)
      {
        cmdBuffer[rxIndex++] = rxBuffer[0];
      }
    }
    
    /* Continue receiving next character */
    HAL_UART_Receive_IT(uartHandle, &rxBuffer[0], 1);
  }
}

/**
 * @brief Compara duas strings sem diferenciar maiúsculas e minúsculas
 * @param s1 Primeira string
 * @param s2 Segunda string
 * @retval 0 se iguais, <0 se s1<s2, >0 se s1>s2
 */
static int str_case_compare(const char* s1, const char* s2)
{
  if (s1 == NULL || s2 == NULL)
    return s1 == s2 ? 0 : (s1 == NULL ? -1 : 1);
    
  while(*s1 && *s2) {
    int c1 = tolower((unsigned char)*s1);
    int c2 = tolower((unsigned char)*s2);
    if (c1 != c2)
      return c1 - c2;
    s1++;
    s2++;
  }
  
  return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}

static void ProcessCommand(void)
{
  char* token;
  char* rest = (char*)cmdBuffer;
    /* Get the command type */  token = strtok(rest, " ");
  
  if (token != NULL)
  {
    /* FREQ command - Set frequency */
    if (str_case_compare(token, "FREQ") == 0)
    {
      token = strtok(NULL, " ");
      if (token != NULL)
      {
        float frequency = atof(token);
        if (FreqControl_SetFrequency(frequency) == 0)
        {
          SerialComm_SendResponse("OK");
        }
        else
        {
          SerialComm_SendResponse("ERROR: Invalid frequency value");
        }
      }
      else
      {
        SerialComm_SendResponse("ERROR: Missing frequency value");
      }
    }
    /* START command - Start inverter */
    else if (str_case_compare(token, "START") == 0)
    {
      if (FreqControl_Start() == 0)
      {
        SerialComm_SendResponse("Inverter started");
      }
      else
      {
        SerialComm_SendResponse("ERROR: Cannot start inverter");
      }
    }
    /* STOP command - Stop inverter */
    else if (str_case_compare(token, "STOP") == 0)
    {
      if (FreqControl_Stop() == 0)
      {
        SerialComm_SendResponse("Inverter stopped");
      }
      else
      {
        SerialComm_SendResponse("ERROR: Cannot stop inverter");
      }
    }
    /* STATUS command - Get inverter status */
    else if (str_case_compare(token, "STATUS") == 0)
    {
      char statusMsg[64];
      float currFreq = FreqControl_GetFrequency();
      uint8_t isRunning = FreqControl_IsRunning();
      
      sprintf(statusMsg, "Status: %s, Frequency: %.1f Hz", 
              isRunning ? "Running" : "Stopped", currFreq);
      SerialComm_SendResponse(statusMsg);
    }
    /* HELP command - Show available commands */
    else if (str_case_compare(token, "HELP") == 0)
    {
      SerialComm_SendResponse("Available commands:");
      SerialComm_SendResponse("  FREQ <value> - Set frequency in Hz (0.1-50.0)");
      SerialComm_SendResponse("  START - Start inverter");
      SerialComm_SendResponse("  STOP - Stop inverter");
      SerialComm_SendResponse("  STATUS - Get inverter status");
      SerialComm_SendResponse("  HELP - Show this help");
    }
    else
    {
      SerialComm_SendResponse("Unknown command. Type HELP for available commands");
    }
  }
}

// Função para checar se um comando foi recebido
uint8_t SerialComm_HasReceivedCommand(void) {
    return rxComplete;
}
