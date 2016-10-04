/*
 *
 * eeprom.c
 *  Created on: Jul 8, 2016
 *      Author: Gennady Tanchin <g.tanchin@yndex.ru>
 */

#include <string.h>
#include "stm32f0xx.h"
#include "stm32xx_it.h"
#include "my_main.h"
#include "logger.h"
#include	"eeprom.h"
#include "stm32f0xx_hal_i2c.h"

#ifndef TIMING_CLEAR_MASK
#define TIMING_CLEAR_MASK       ((uint32_t)0xF0FFFFFF)  /*<! I2C TIMING clear register Mask */
#endif

tEeprom eeprom;
I2C_HandleTypeDef hepr;

static HAL_StatusTypeDef my_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout);
static HAL_StatusTypeDef I2C_WaitOnFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Flag, FlagStatus Status, uint32_t Timeout);

int8_t eepromInit( void ){

  /* Enable I2C1 reset state */
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, ENABLE);
  /* Release I2C1 from reset state */
  RCC_APB1PeriphResetCmd(RCC_APB1Periph_I2C1, DISABLE);
  /* I2C1 configuration ------------------------------------------------------*/

  hepr.Instance             = EEPROM_I2C;

  hepr.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
  hepr.Init.Timing          = I2C_TIMING;
  hepr.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hepr.Init.OwnAddress1     = 0;
  hepr.Init.OwnAddress2     = 0;
  hepr.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hepr.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;


  HAL_I2C_Init( &hepr );
//  i2cInit();

  /* Enable I2C1 Error interrupts */
  EEPROM_I2C->CR1 |= I2C_CR1_ERRIE;

  NVIC_SetPriority(I2C1_IRQn, 2);

  NVIC_EnableIRQ(I2C1_IRQn);

  return 0;
}

/*
void i2cInit( void ) {
	I2C_InitTypeDef I2C_InitStruct;

	if ( EEPROM_I2C == I2C1 ){
		// Для интерфейса I2C1
		RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	}
	else {
		while(1)		// Для других интерфейсов
		{}
	}

	i2cMspInit();

	// Set values
	I2C_InitStruct.I2C_Timing = 0x00B01A4B; // I2C_TIMING;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_OwnAddress1 = I2C_OWN_ADDR;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStruct.I2C_AnalogFilter = I2C_AnalogFilter_Enable;
  I2C_InitStruct.I2C_DigitalFilter = 4;
	// Disable I2C first
	EEPROM_I2C->CR1 &= ~I2C_CR1_PE;
	// Initialize I2C
	I2C_Init(EEPROM_I2C, &I2C_InitStruct);

	// Enable I2C
	EEPROM_I2C->CR1 |= I2C_CR1_PE;

}
*/


int8_t sendEeprom( uint32_t addr, uint8_t * data, uint16_t len) {
	/* TODO: Запись в EPROM данных
	 * addr - стартовый адрес в Eprom
	 * data - Указатель на буфер с данными
	 * len - длина записываемых данных в байта
	 *
	 * Организовать проверку на превышение границы памяти
	 */
	HAL_StatusTypeDef ret;
	uint32_t tout = myTick + I2C_TOUT;

	uint8_t eprAddr = EEPROM_I2C_ADDR | ((addr >> 16) & 0x2);
	addr = addr & 0xFFFF;

	if( (ret = HAL_I2C_Mem_Write( &hepr, eprAddr, addr, I2C_MEMADD_SIZE_16BIT, data, len, EEPROM_TOUT)) == HAL_OK){
		// Check if the EEPROM is ready for a new operation
		while ( (ret = my_I2C_IsDeviceReady(&hepr, eprAddr, 20, 300))== HAL_TIMEOUT){
			if( myTick > tout ){
				break;
			}
		}
	}
	return ret;
}

int8_t receiveEeprom( uint32_t addr, uint8_t * data, uint16_t len) {
	/* Чтение данных из EPROM
	 * addr - стартовый адрес в Eprom
	 * data - Указатель на буфер, куда будут складываться данные
	 * len - длина записываемых данных в байта
	 *
	 * Организовать проверку на превышение границы памяти
	 */
	HAL_StatusTypeDef ret;
	uint8_t eprAddr = EEPROM_I2C_ADDR | ((addr >> 16) & 0x2);
	addr = addr & 0xFFFF;

	ret = HAL_I2C_Mem_Read( & hepr, eprAddr, addr, I2C_MEMADD_SIZE_16BIT, data, len, EEPROM_TOUT );
	if( ret != HAL_OK ){
		while(1)
		{}
	}
	return ret;
}

void epprom_IRQHandler( void ) {
  if( EEPROM_I2C->ISR & I2C_FLAG_TXIS )
  {
    /* Write data to TXDR */
  	EEPROM_I2C->TXDR = *(eeprom.txPtr++);
    eeprom.count--;
  }
  else if( EEPROM_I2C->ISR & I2C_FLAG_TC )
  {
    if(eeprom.count == 0)
    {
      /* Generate Stop */
      EEPROM_I2C->CR2 |= I2C_CR2_STOP;
    }
    else
    {
      /* Wrong size Status regarding TCR flag event */
    	hepr.State = HAL_I2C_STATE_ERROR;
      eepromErrorCallBack();
    }
  }
  else if( EEPROM_I2C->ISR & I2C_FLAG_STOPF )
  {
    /* Disable ERR, TC, STOP, NACK, TXI interrupt */
    EEPROM_I2C->CR1 &= ~(I2C_IT_ERRI | I2C_IT_TCI| I2C_IT_STOPI| I2C_IT_NACKI | I2C_IT_TXI);

    /* Clear STOP Flag */
    EEPROM_I2C->ICR |= I2C_ICR_STOPCF;

    /* Clear Configuration Register 2 */
    EEPROM_I2C->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN);

    hepr.State = HAL_I2C_STATE_READY;

    eepromTxCpltCallback();

  }
  else if( EEPROM_I2C->ISR & I2C_ISR_NACKF )
  {
    EEPROM_I2C->ICR |= I2C_ICR_NACKCF;

    hepr.State = HAL_I2C_STATE_READY;
  }
  eepromErrorCallBack();
}

void i2cMspInit( void ){
	// Clock USART1 enable
	RCC->APB1ENR |= RCC_APB1ENR_I2C1EN;
	// Clock GPIOA enable
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;


	// EEPROM I2C CLK
	EEPROM_PORT->MODER |= 2 << (EEPROM_SCL_PIN_NUM*2);					// Выставляем в Alternate Function
	EEPROM_PORT->OTYPER &= ~(3 << (EEPROM_SCL_PIN_NUM*2));			// PullUp-PullDown
	EEPROM_PORT->OTYPER |= EEPROM_SCL_PIN;      			// PullUp-PullDown
	EEPROM_PORT->PUPDR &= ~(3 << (EEPROM_SCL_PIN_NUM*2));			// NoPULL
	EEPROM_PORT->PUPDR |= (1 << (EEPROM_SCL_PIN_NUM*2));			// NoPULL
	EEPROM_PORT->OSPEEDR |= (3 << (EEPROM_SCL_PIN_NUM*2));			// Low Speed
	EEPROM_PORT->AFR[EEPROM_SCL_PIN_NUM >> 0x3] |= 4 << ((EEPROM_SCL_PIN_NUM & 0x7) * 4);		// AF4

	// USART RX
	EEPROM_PORT->MODER |= 2 << (EEPROM_SDA_PIN_NUM*2);					// Выставляем в Alternate Function
	EEPROM_PORT->OTYPER &= ~(3 << (EEPROM_SDA_PIN_NUM*2));			// PullUp-PullDown
	EEPROM_PORT->OTYPER |= EEPROM_SDA_PIN;    	        		// PullUp-PullDown
	EEPROM_PORT->PUPDR &= ~(3 << (EEPROM_SDA_PIN_NUM*2));			// NoPULL
	EEPROM_PORT->PUPDR |= (1 << (EEPROM_SDA_PIN_NUM*2));			// NoPULL
	EEPROM_PORT->OSPEEDR |= (3 << (EEPROM_SDA_PIN_NUM*2));			// Low Speed
	EEPROM_PORT->AFR[EEPROM_SDA_PIN_NUM >> 0x3] |= 4 << ((EEPROM_SDA_PIN_NUM & 0x7) * 4);		// AF4

}

void eepromErrorCallBack( void ){
	EEPROM_I2C->ICR |= 	I2C_ICR_ALERTCF | \
											I2C_ICR_NACKCF | \
											I2C_ICR_STOPCF | \
											I2C_ICR_BERRCF | \
											I2C_ICR_ARLOCF | \
											I2C_ICR_OVRCF  | \
											I2C_ICR_PECCF | \
											I2C_ICR_TIMOUTCF | \
											I2C_ICR_ALERTCF;

}

void eepromTxCpltCallback( void ){

}

/**
  * @brief  Initializes the I2C according to the specified parameters
  *         in the I2C_InitTypeDef and initialize the associated handle.
  * @param  hi2c : Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c)
{
  /* Check the I2C handle allocation */
  if(hi2c == NULL)
  {
    return HAL_ERROR;
  }

  /* Check the parameters */
  assert_param(IS_I2C_ALL_INSTANCE(hi2c->Instance));
  assert_param(IS_I2C_OWN_ADDRESS1(hi2c->Init.OwnAddress1));
  assert_param(IS_I2C_ADDRESSING_MODE(hi2c->Init.AddressingMode));
  assert_param(IS_I2C_DUAL_ADDRESS(hi2c->Init.DualAddressMode));
  assert_param(IS_I2C_OWN_ADDRESS2(hi2c->Init.OwnAddress2));
  assert_param(IS_I2C_OWN_ADDRESS2_MASK(hi2c->Init.OwnAddress2Masks));
  assert_param(IS_I2C_GENERAL_CALL(hi2c->Init.GeneralCallMode));
  assert_param(IS_I2C_NO_STRETCH(hi2c->Init.NoStretchMode));

  if(hi2c->State == HAL_I2C_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hi2c->Lock = HAL_UNLOCKED;

    /* Init the low level hardware : GPIO, CLOCK, CORTEX...etc */
  	i2cMspInit();
  }

  hi2c->State = HAL_I2C_STATE_BUSY;

  /* Disable the selected I2C peripheral */
  __HAL_I2C_DISABLE(hi2c);

  /*---------------------------- I2Cx TIMINGR Configuration ------------------*/
  /* Configure I2Cx: Frequency range */
  hi2c->Instance->TIMINGR = hi2c->Init.Timing & TIMING_CLEAR_MASK;

  /*---------------------------- I2Cx OAR1 Configuration ---------------------*/
  /* Configure I2Cx: Own Address1 and ack own address1 mode */
  hi2c->Instance->OAR1 &= ~I2C_OAR1_OA1EN;
  if(hi2c->Init.OwnAddress1 != 0)
  {
    if(hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT)
    {
      hi2c->Instance->OAR1 = (I2C_OAR1_OA1EN | hi2c->Init.OwnAddress1);
    }
    else /* I2C_ADDRESSINGMODE_10BIT */
    {
      hi2c->Instance->OAR1 = (I2C_OAR1_OA1EN | I2C_OAR1_OA1MODE | hi2c->Init.OwnAddress1);
    }
  }

  /*---------------------------- I2Cx CR2 Configuration ----------------------*/
  /* Configure I2Cx: Addressing Master mode */
  if(hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_10BIT)
  {
    hi2c->Instance->CR2 = (I2C_CR2_ADD10);
  }
  /* Enable the AUTOEND by default, and enable NACK (should be disable only during Slave process */
  hi2c->Instance->CR2 |= (I2C_CR2_AUTOEND | I2C_CR2_NACK);

  /*---------------------------- I2Cx OAR2 Configuration ---------------------*/
  /* Configure I2Cx: Dual mode and Own Address2 */
  hi2c->Instance->OAR2 = (hi2c->Init.DualAddressMode | hi2c->Init.OwnAddress2 | (hi2c->Init.OwnAddress2Masks << 8));

  /*---------------------------- I2Cx CR1 Configuration ----------------------*/
  /* Configure I2Cx: Generalcall and NoStretch mode */
  hi2c->Instance->CR1 = (hi2c->Init.GeneralCallMode | hi2c->Init.NoStretchMode);

  /* Enable the selected I2C peripheral */
  __HAL_I2C_ENABLE(hi2c);

  hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
  hi2c->State = HAL_I2C_STATE_READY;

  return HAL_OK;
}

/**
  * @brief  Checks if target device is ready for communication.
  * @note   This function is used with Memory devices
  * @param  hi2c : Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @param  DevAddress: Target device address
  * @param  Trials: Number of trials
  * @param  Timeout: Timeout duration
  * @retval HAL status
  */
static HAL_StatusTypeDef my_I2C_IsDeviceReady(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint32_t Trials, uint32_t Timeout) {
  uint32_t tout;

  __IO uint32_t I2C_Trials = 0;

  if(hi2c->State == HAL_I2C_STATE_READY)
  {
    if(__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_BUSY) == SET)
    {
      return HAL_BUSY;
    }

    /* Process Locked */
    __HAL_LOCK(hi2c);

    hi2c->State = HAL_I2C_STATE_BUSY;
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;

    do
    {
      /* Generate Start */
      hi2c->Instance->CR2 = I2C_GENERATE_START(hi2c->Init.AddressingMode,DevAddress);

      /* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
      /* Wait until STOPF flag is set or a NACK flag is set*/
      tout = myTick + Timeout;
      while((__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_STOPF) == RESET) && (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_AF) == RESET) && (hi2c->State != HAL_I2C_STATE_TIMEOUT))
      {
        if(Timeout != HAL_MAX_DELAY)
        {
          if( (Timeout == 0) || (myTick > tout) )
          {
            /* Device is ready */
            hi2c->State = HAL_I2C_STATE_READY;
            /* Process Unlocked */
            __HAL_UNLOCK(hi2c);
            return HAL_TIMEOUT;
          }
        }
      }

      /* Check if the NACKF flag has not been set */
      if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_AF) == RESET)
      {
        /* Wait until STOPF flag is reset */
        if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_STOPF, RESET, Timeout) != HAL_OK)
        {
          return HAL_TIMEOUT;
        }

        /* Clear STOP Flag */
        __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);

        /* Device is ready */
        hi2c->State = HAL_I2C_STATE_READY;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_OK;
      }
      else
      {
        /* Wait until STOPF flag is reset */
        if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_STOPF, RESET, Timeout) != HAL_OK)
        {
          return HAL_TIMEOUT;
        }

        /* Clear NACK Flag */
        __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);

        /* Clear STOP Flag, auto generated with autoend*/
        __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);
      }

      /* Check if the maximum allowed number of trials has been reached */
      if (I2C_Trials++ == Trials)
      {
        /* Generate Stop */
        hi2c->Instance->CR2 |= I2C_CR2_STOP;

        /* Wait until STOPF flag is reset */
        if(I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_STOPF, RESET, Timeout) != HAL_OK)
        {
          return HAL_TIMEOUT;
        }

        /* Clear STOP Flag */
        __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);
      }
    }while(I2C_Trials < Trials);

    hi2c->State = HAL_I2C_STATE_READY;

    /* Process Unlocked */
    __HAL_UNLOCK(hi2c);

    return HAL_TIMEOUT;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @brief  This function handles I2C Communication Timeout.
  * @param  hi2c : Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @param  Flag: specifies the I2C flag to check.
  * @param  Status: The new Flag status (SET or RESET).
  * @param  Timeout: Timeout duration
  * @retval HAL status
  */
static HAL_StatusTypeDef I2C_WaitOnFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Flag, FlagStatus Status, uint32_t Timeout) {
  uint32_t tickstart = getTick();

  /* Wait until flag is set */
  if(Status == RESET)
  {
    while(__HAL_I2C_GET_FLAG(hi2c, Flag) == RESET)
    {
      /* Check for the Timeout */
      if(Timeout != HAL_MAX_DELAY)
      {
        if((Timeout == 0) || (( myTick - tickstart) > Timeout))
        {
          hi2c->State= HAL_I2C_STATE_READY;
          /* Process Unlocked */
          __HAL_UNLOCK(hi2c);
          return HAL_TIMEOUT;
        }
      }
    }
  }
  else
  {
    while(__HAL_I2C_GET_FLAG(hi2c, Flag) != RESET)
    {
      /* Check for the Timeout */
      if(Timeout != HAL_MAX_DELAY)
      {
        if((Timeout == 0) || (( myTick - tickstart) > Timeout))
        {
          hi2c->State= HAL_I2C_STATE_READY;
          /* Process Unlocked */
          __HAL_UNLOCK(hi2c);
          return HAL_TIMEOUT;
        }
      }
    }
  }
  return HAL_OK;
}
