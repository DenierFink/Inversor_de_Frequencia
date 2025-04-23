/**
 * Inversor de Frequência Trifásico com Controle Serial
 * 
 * Este projeto implementa um inversor de frequência trifásico
 * controlado através da porta serial do STM32F030R8.
 * 
 * Hardware: STM32F030R8 (Discovery)
 * 
 */

#include "main.h"
#include "serial_comm.h"
#include "pwm_control.h"
#include "freq_control.h"

#include <stdint.h>

// Protótipos das funções de inicialização
static void SystemClock_Config(void);
static void GPIO_Init(void);
static void UART2_Init(void);
static void TIM1_PWM_Init(void);

// Variáveis globais para os periféricos
UART_HandleTypeDef huart2;
TIM_HandleTypeDef htim1;

// Enum para os estados do sistema
typedef enum {
    SISTEMA_INICIALIZANDO = 0,
    SISTEMA_TESTE,
    SISTEMA_PRONTO,
    SISTEMA_RODANDO,
    SISTEMA_PARADO,
    SISTEMA_ERRO
} SistemaEstado_t;

// Variável global para o estado do sistema
static SistemaEstado_t sistemaEstado = SISTEMA_INICIALIZANDO;

// Controle de tempo para o LED
static uint32_t ledTick = 0;
static uint8_t ledOn = 0;

// Controle de tempo para o LED do microcontrolador
static uint32_t ledMCUTick = 0;
static uint8_t ledMCUOn = 0;

// Função para atualizar o LED conforme o estado do sistema
void AtualizaLedStatus(void) {
    uint32_t now = HAL_GetTick();
    uint32_t intervalo = 0;
    switch (sistemaEstado) {
        case SISTEMA_INICIALIZANDO:
            intervalo = 100; // Pisca rápido
            break;
        case SISTEMA_PRONTO:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET); // Aceso
            return;
        case SISTEMA_RODANDO:
            intervalo = 500; // Pisca lento
            break;
        case SISTEMA_PARADO:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET); // Apagado
            return;
        case SISTEMA_ERRO:
            intervalo = 200; // Pisca médio
            break;
        default:
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
            return;
    }
    if (now - ledTick >= intervalo) {
        ledTick = now;
        ledOn = !ledOn;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, ledOn ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void AtualizaLedMCU(void) {
    uint32_t now = HAL_GetTick();
    if (now - ledMCUTick >= 250) { // Pisca a cada 250ms
        ledMCUTick = now;
        ledMCUOn = !ledMCUOn;
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, ledMCUOn ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
  /* MCU Configuration */
  HAL_Init();
  SystemClock_Config();
  
  /* Initialize all configured peripherals */
  GPIO_Init();
  UART2_Init();
  TIM1_PWM_Init();
  
  /* Initialize modules */
  SerialComm_Init(&huart2);
  PWMControl_Init(&htim1);
  FreqControl_Init();

  sistemaEstado = SISTEMA_TESTE;
  uint32_t initTick = HAL_GetTick();
  float testFreq = 1.0f;
  uint8_t testUp = 1;

  /* Infinite loop */
  while (1)
  {
    if (sistemaEstado == SISTEMA_TESTE) {
        // Teste: varre frequência de 1 a 10 Hz e volta
        if (testUp) {
            testFreq += 0.05f;
            if (testFreq >= 10.0f) testUp = 0;
        } else {
            testFreq -= 0.05f;
            if (testFreq <= 1.0f) testUp = 1;
        }
        FreqControl_SetFrequency(testFreq);
        if (FreqControl_IsRunning() == 0) FreqControl_Start();
        // Se receber comando serial, sai do teste
        if (SerialComm_HasReceivedCommand()) {
            FreqControl_Stop();
            sistemaEstado = SISTEMA_PRONTO;
        }
    } else {
        /* Simula inicialização por 2 segundos se não estiver em teste */
        if (sistemaEstado == SISTEMA_INICIALIZANDO && (HAL_GetTick() - initTick > 2000)) {
            sistemaEstado = SISTEMA_PRONTO;
        }
        /* Atualiza estado conforme funcionamento do inversor */
        if (sistemaEstado != SISTEMA_INICIALIZANDO) {
            if (FreqControl_IsRunning()) {
                sistemaEstado = SISTEMA_RODANDO;
            } else {
                sistemaEstado = SISTEMA_PRONTO;
            }
        }
        // if (erro_detectado) sistemaEstado = SISTEMA_ERRO;
    }
    AtualizaLedStatus();
    AtualizaLedMCU();
    SerialComm_Process();
    FreqControl_Update();
    HAL_Delay(10);
  }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
static void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /* Configura apenas o HSI (8 MHz) como fonte de clock */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_OFF; // PLL desligado
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI; // Usa HSI direto
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief GPIO Initialization
 * @retval None
 */
static void GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  
  /* Configure LED de status (PA5) */
  GPIO_InitStruct.Pin = GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* Configure LED de funcionamento do MCU (PA6) */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/**
 * @brief UART2 Initialization
 * @retval None
 */
static void UART2_Init(void)
{
  /* Configure USART2 pins (PA2=TX, PA3=RX) */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  /* Enable GPIO clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_USART2_CLK_ENABLE();
  
  /* Configure USART2 pins (PA2=TX, PA3=RX) */
  GPIO_InitStruct.Pin = GPIO_PIN_2 | GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF1_USART2;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* Configure USART2 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
 * @brief TIM1 PWM Initialization
 * @retval None
 */
static void TIM1_PWM_Init(void)
{
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* Configuração dos pinos do Timer1 como Alternate Function */
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  /* Enable GPIO clock */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  
  /* Configure Timer1 pins: PA8(TIM1_CH1), PA9(TIM1_CH2), PA10(TIM1_CH3) */
  GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF2_TIM1;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  /* Enable TIM1 clock */
  __HAL_RCC_TIM1_CLK_ENABLE();

  /* Timer base configuration */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_CENTERALIGNED1;
  htim1.Init.Period = 1000;  // PWM carrier frequency = TIM1CLK/(2*Period)
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }

  /* Master configuration */
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* PWM configuration for phase U */
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  /* PWM configuration for phase V */
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_2) != HAL_OK)
  {
    Error_Handler();
  }

  /* PWM configuration for phase W */
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  /* Configure dead time */
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 100; // Dead time in timer ticks
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }

  /* Start PWM for all channels */
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
}

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    HAL_Delay(200);
  }
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  (void)file;
  (void)line;
}
#endif /* USE_FULL_ASSERT */
