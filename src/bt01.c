#include "stm32f0xx_hal_tim.h"
#include "bt01.h"
#include "stm32f030x6.h"

/* Определение преременных для управления реле */
GPIO_TypeDef* RELAY_PORT[RELAYn] = {RELAY1_PORT, RELAY2_PORT};
const uint16_t RELAY_PIN[RELAYn] = {RELAY1_PIN, RELAY2_PIN};

GPIO_TypeDef* LED_PORT[LEDn] = {LED1_PORT, LED2_PORT};
const uint16_t LED_PIN[LEDn] = {LED1_PIN, LED2_PIN};

uint32_t SpixTimeout = SPIx_TIMEOUT_MAX; /*<! Value of Timeout when SPI communication fails */
//static SPI_HandleTypeDef hSpi;
TIM_HandleTypeDef relayTimHandle[2];     // Хэндлы для таймеров 2-х реле
TIM_TypeDef *tim[] = { TIM16, TIM17 };

/**
  * @brief  Enables or disables the specified TIM peripheral.
  * @param  TIMx: where x can be 1 to 14 to select the TIMx peripheral.
  * @param  NewState: new state of the TIMx peripheral.
  *          This parameter can be: ENABLE or DISABLE.
  * @retval None
  */
void TIM_Cmd(TIM_TypeDef* TIMx, FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_TIM_ALL_PERIPH(TIMx));
  assert_param(IS_FUNCTIONAL_STATE(NewState));

  if (NewState != DISABLE)
  {
    /* Enable the TIM Counter */
    TIMx->CR1 |= TIM_CR1_CEN;
  }
  else
  {
    /* Disable the TIM Counter */
    TIMx->CR1 &= (uint16_t)~TIM_CR1_CEN;
  }
}

void relayInit( relayTypeDef rel )
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* Enable the GPIO_RELAY Clock */
  RELAYx_GPIO_CLK_ENABLE( rel );

  /* Configure the GPIO_RELAY1 pin */
  GPIO_InitStruct.Pin = RELAY_PIN[rel];
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM3;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

  HAL_GPIO_Init(RELAY_PORT[rel], &GPIO_InitStruct);
}

uint8_t relayStatus( relayTypeDef rel ){
  return HAL_GPIO_ReadPin( RELAY_PORT[rel], RELAY_PIN[rel] );
}
/* Включаем реле */
void relayOn( relayTypeDef rel )
{
  HAL_GPIO_WritePin(RELAY_PORT[rel], RELAY_PIN[rel], GPIO_PIN_SET);
}

/* Выключаем реле */
void relayOff( relayTypeDef rel )
{
  HAL_GPIO_WritePin(RELAY_PORT[rel], RELAY_PIN[rel], GPIO_PIN_RESET);
}

/* ПЕРЕКЛЮЧАЕМ реле: 0->1, 1->0 */
void relayToggle( relayTypeDef rel )
{
  HAL_GPIO_TogglePin(RELAY_PORT[rel], RELAY_PIN[rel]);
}

/* ================= Функции для LEDs ==============================*/
void ledInit( ledTypeDef led )
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* Enable the GPIO_RELAY Clock */
  LEDx_GPIO_CLK_ENABLE( led );

  /* Configure the GPIO_LED1 pin */
  GPIO_InitStruct.Pin = LED_PIN[led];
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

  HAL_GPIO_Init(LED_PORT[led], &GPIO_InitStruct);

}

uint8_t ledStatus( ledTypeDef led ){
  return HAL_GPIO_ReadPin( LED_PORT[led], LED_PIN[led] );
}
/* Включаем реле */
void ledOn( ledTypeDef led )
{
  HAL_GPIO_WritePin(LED_PORT[led], LED_PIN[led], GPIO_PIN_SET);
}

/* Выключаем реле */
void ledOff( ledTypeDef led )
{
  HAL_GPIO_WritePin(LED_PORT[led], LED_PIN[led], GPIO_PIN_RESET);
}

/* ПЕРЕКЛЮЧАЕМ реле: 0->1, 1->0 */
void ledToggle( ledTypeDef led )
{
  HAL_GPIO_TogglePin(LED_PORT[led], LED_PIN[led]);
}
