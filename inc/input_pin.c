/*
######################################################################

Здесь собраны функции работы с РЕЛЕ и СВЕТОДИОДАМИ 

######################################################################### 
*/
#include "my_def.h"
#include "stm32f1xx_hal_gpio.h"

/* ================= Функции для LEDs ==============================*/
/* Функция получает номер светодиода и возвращает имя порта этого светодиода */
GPIO_TypeDef * getInPort( uint8_t in ){
  GPIO_TypeDef * ret;
  switch ( in ){
    case IN1:
      ret = IN1_PORT;
      break;
    case IN2:
      ret = IN2_PORT;
      break;
    case IN3:
      ret = IN3_PORT;
      break;
    case IN4:
      ret = IN4_PORT;
      break;
    case IN5:
      ret = IN5_PORT;
      break;
    case IN6:
      ret = IN6_PORT;
      break;
    case IN7:
      ret = IN7_PORT;
      break;
    case IN8:
      ret = IN8_PORT;
      break;
    case IN9:
      ret = IN9_PORT;
      break;
    case IN10:
      ret = IN10_PORT;
      break;
  }
  return ret;
}

/* Функция получает номер светодиода и возвращает имя порта этого светодиода */
uint16_t getInPin( uint8_t in ){
  uint16_t ret;
  switch ( in ){
    case IN1:
      ret = IN1_PIN;
      break;
    case IN2:
      ret = IN2_PIN;
      break;
    case IN3:
      ret = IN3_PIN;
      break;
    case IN4:
      ret = IN4_PIN;
      break;
    case IN5:
      ret = IN5_PIN;
      break;
    case IN6:
      ret = IN6_PIN;
      break;
    case IN7:
      ret = IN7_PIN;
      break;
    case IN8:
      ret = IN8_PIN;
      break;
    case IN9:
      ret = IN9_PIN;
      break;
    case IN10:
      ret = IN10_PIN;
      break;
  }
  return ret;
}

void inInit( inTypeDef in )
{
  GPIO_InitTypeDef  GPIO_InitStruct;
 
  /* Configure the GPIO_LED1 pin */
  GPIO_InitStruct.Pin = getInPin( in );
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
 
  HAL_GPIO_Init(getInPort( in ), &GPIO_InitStruct);
}

uint8_t inStatus( inTypeDef in ){
  return HAL_GPIO_ReadPin( getInPort( in ), getInPin( in ) );
}
