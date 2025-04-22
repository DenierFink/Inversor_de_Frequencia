/**
 * @file freq_control.c
 * @brief Implementação do controle de frequência para o inversor trifásico
 */

#include "freq_control.h"
#include "pwm_control.h"
#include "main.h"
#include <math.h>

/* Private defines */
#define PI                  3.14159265359f
#define TWO_PI              (2.0f * PI)
#define PWM_MAX_VALUE       1000
#define PWM_CARRIER_FREQ    10000   /* 10kHz PWM carrier frequency */
#define SINE_TABLE_SIZE     360     /* One value per degree */
#define PHASE_120_DEGREES   120
#define PHASE_240_DEGREES   240

/* Private variables */
static float targetFrequency = 10.0f;  /* Default frequency 10 Hz */
static float currentAngle = 0.0f;
static float angleIncrement = 0.0f;
static uint8_t isRunning = 0;
static uint16_t sineTable[SINE_TABLE_SIZE];
static uint8_t voltageBoost = 0;       /* Voltage boost percentage (0-20) */

/* Private function prototypes */
static void UpdateAngleIncrement(void);
static void GenerateSineTable(void);
static uint16_t GetSineValue(float angle);

/**
 * @brief Initializes the frequency control module
 * @retval None
 */
void FreqControl_Init(void)
{
  /* Generate sine look-up table */
  GenerateSineTable();
  
  /* Initialize PWM carrier frequency */
  PWMControl_SetCarrierFreq(PWM_CARRIER_FREQ);
  
  /* Initial update of angle increment */
  UpdateAngleIncrement();
  
  /* Reset variables */
  currentAngle = 0.0f;
  isRunning = 0;
  voltageBoost = 10; /* 10% voltage boost at low frequencies */
}

/**
 * @brief Sets the output frequency
 * @param freqHz Frequency in Hz
 * @retval 0=success, 1=error (invalid frequency)
 */
uint8_t FreqControl_SetFrequency(float freqHz)
{
  /* Validate frequency */
  if (freqHz < FREQ_MIN || freqHz > FREQ_MAX)
  {
    return 1; /* Error: Invalid frequency */
  }
  
  /* Update target frequency */
  targetFrequency = freqHz;
  
  /* Update angle increment */
  UpdateAngleIncrement();
  
  return 0; /* Success */
}

/**
 * @brief Gets the current output frequency
 * @retval Current frequency in Hz
 */
float FreqControl_GetFrequency(void)
{
  return targetFrequency;
}

/**
 * @brief Starts the inverter
 * @retval 0=success, 1=error
 */
uint8_t FreqControl_Start(void)
{
  if (!isRunning)
  {
    /* Enable PWM outputs */
    PWMControl_Enable();
    isRunning = 1;
  }
  
  return 0; /* Success */
}

/**
 * @brief Stops the inverter
 * @retval 0=success, 1=error
 */
uint8_t FreqControl_Stop(void)
{
  if (isRunning)
  {
    /* Disable PWM outputs */
    PWMControl_Disable();
    isRunning = 0;
  }
  
  return 0; /* Success */
}

/**
 * @brief Checks if the inverter is running
 * @retval 1 if running, 0 if stopped
 */
uint8_t FreqControl_IsRunning(void)
{
  return isRunning;
}

/**
 * @brief Updates the inverter state (should be called periodically)
 * @retval None
 */
void FreqControl_Update(void)
{
  if (isRunning)
  {
    uint16_t phaseU, phaseV, phaseW;
    float angleU, angleV, angleW;
    
    /* Update current angle */
    currentAngle += angleIncrement;
    if (currentAngle >= TWO_PI)
    {
      currentAngle -= TWO_PI;
    }
    
    /* Calculate the three phase angles (120 degrees apart) */
    angleU = currentAngle;
    angleV = currentAngle + (PHASE_120_DEGREES * (PI / 180.0f));
    angleW = currentAngle + (PHASE_240_DEGREES * (PI / 180.0f));
    
    /* Wrap angles to 0-2PI */
    if (angleV >= TWO_PI) angleV -= TWO_PI;
    if (angleW >= TWO_PI) angleW -= TWO_PI;
    
    /* Get sine values for each phase */
    phaseU = GetSineValue(angleU);
    phaseV = GetSineValue(angleV);
    phaseW = GetSineValue(angleW);
    
    /* Update PWM outputs */
    PWMControl_SetOutputs(phaseU, phaseV, phaseW);
  }
}

/**
 * @brief Updates angle increment based on target frequency
 * @retval None
 */
static void UpdateAngleIncrement(void)
{
  /* Calculate angle increment per update cycle */
  /* angle_increment = 2π * frequency * update_period */
  /* For update_period = 10ms (100Hz), and frequency in Hz */
  angleIncrement = TWO_PI * targetFrequency * 0.01f;
}

/**
 * @brief Generates the sine look-up table
 * @retval None
 */
static void GenerateSineTable(void)
{
  float angle;
  float sinValue;
  uint16_t i;
  
  /* Generate sine values for 0 to 359 degrees */
  for (i = 0; i < SINE_TABLE_SIZE; i++)
  {
    /* Convert degree to radian */
    angle = i * (TWO_PI / SINE_TABLE_SIZE);
    
    /* Calculate sine value (-1 to +1) */
    sinValue = sinf(angle);
    
    /* Scale to PWM range (0 to PWM_MAX_VALUE) */
    /* First scale to 0-1 range by adding 1 and dividing by 2 */
    sinValue = (sinValue + 1.0f) / 2.0f;
    
    /* Then scale to PWM range */
    sineTable[i] = (uint16_t)(sinValue * PWM_MAX_VALUE);
  }
}

/**
 * @brief Gets the sine value for a specific angle
 * @param angle Angle in radians
 * @retval PWM value corresponding to the sine of the angle
 */
static uint16_t GetSineValue(float angle)
{
  uint16_t index;
  uint16_t sineVal;
  float voltageRatio;
  float boostFactor;
  
  /* Convert angle to index in sine table */
  index = (uint16_t)((angle * 180.0f / PI)) % SINE_TABLE_SIZE;
  
  /* Get base sine value */
  sineVal = sineTable[index];
  
  /* Apply voltage boost at low frequencies (V/f control) */
  if (targetFrequency < 10.0f && voltageBoost > 0)
  {
    /* Calculate voltage boost ratio */
    voltageRatio = targetFrequency / 10.0f;  /* 0-1 for 0-10Hz */
    
    /* Apply boost factor based on frequency */
    boostFactor = 1.0f + (voltageBoost / 100.0f) * (1.0f - voltageRatio);
    
    /* Apply boost but don't exceed PWM_MAX_VALUE */
    sineVal = (uint16_t)(sineVal * boostFactor);
    if (sineVal > PWM_MAX_VALUE)
    {
      sineVal = PWM_MAX_VALUE;
    }
  }
  
  return sineVal;
}
