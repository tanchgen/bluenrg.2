/*
 *
 * eeprom.c
 *  Created on: Jul 8, 2016
 *      Author: Gennady Tanchin <g.tanchin@yndex.ru>
 */

#include <string.h>
#include "stm32f0xx.h"
#include 	"stm32f0xx_i2c.h"
#include "stm32xx_it.h"
#include "my_main.h"
#include "logger.h"
#include	"eeprom.h"

tEeprom eeprom;

int8_t eepromInit( void ){

  I2C_DeInit(EEPROM_I2C);
  /* I2C1 configuration ------------------------------------------------------*/

  i2cInit();

  /* Enable I2C1 Error interrupts */
  EEPROM_I2C->CR1 |= I2C_CR1_ERRIE;

  NVIC_SetPriority(I2C1_IRQn, 2);

  NVIC_EnableIRQ(I2C1_IRQn);

  eeprom.status = EPR_READY;
  return 0;
}

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

int8_t sendEeprom( uint32_t addr, uint8_t * data, uint16_t len) {
	/* TODO: Запись в EPROM данных
	 * addr - стартовый адрес в Eprom
	 * data - Указатель на буфер с данными
	 * len - длина записываемых данных в байта
	 *
	 * Организовать проверку на превышение границы памяти
	 */

	if(eeprom.status != EPR_READY) {
		return eeprom.status;
	}

	if ( (sendEepromAddr(addr)) != EPR_OK) {
			return eeprom.status;
	}
	if ( (eeprom.status = eepromSend( data, len)) == EPR_OK ){
	  eeprom.status = EPR_READY;
		return EPR_OK;
	}
	return eeprom.status;
}

eEprStatus receiveEeprom( uint32_t addr, uint8_t * data, uint16_t len) {
	/* Чтение данных из EPROM
	 * addr - стартовый адрес в Eprom
	 * data - Указатель на буфер, куда будут складываться данные
	 * len - длина записываемых данных в байта
	 *
	 * Организовать проверку на превышение границы памяти
	 */
	if(eeprom.status != EPR_READY) {
		return eeprom.status;
	}

	if ( (sendEepromAddr(addr)) != EPR_OK) {
			return eeprom.status;
	}
	if ( (eeprom.status = eepromRead( data, len)) == EPR_OK ){
	  eeprom.status = EPR_READY;
		return EPR_OK;
	}
	return eeprom.status;
}

int8_t sendEepromAddr( uint32_t addr ) {
	uint32_t tout;

	eeprom.status = EPR_BUSY;

	while( (EEPROM_I2C->ISR & I2C_ISR_ARLO) ){
		goto eprFault;
	}

	tout = myTick + I2C_TOUT;
	while( (EEPROM_I2C->ISR & I2C_ISR_BUSY) ){
		if ( myTick > tout){
			goto eprFault;
		}
	}

	// Стираем Slave-адрес, Перезагрузку, Автостоп, Чтение-НеЗапись, Старт, Стоп
	EEPROM_I2C->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | I2C_CR2_RD_WRN | I2C_CR2_START | I2C_CR2_STOP );
	// Младший бит адреса I2C - старший бит адреса памяти
	EEPROM_I2C->CR2 |= EEPROM_I2C_ADDR | ((addr >> 15) & 0x2);
	// Количество передаваемых байт
	EEPROM_I2C->CR2 &= ~I2C_CR2_NBYTES;
	EEPROM_I2C->CR2 |= 2<<16;
	EEPROM_I2C->CR2 |= I2C_CR2_AUTOEND;
	// Стартуем
	EEPROM_I2C->CR2 |= I2C_CR2_START;

	tout = myTick + I2C_TOUT;
	while ( !(EEPROM_I2C->ISR & I2C_ISR_TXIS) ){
		if ( myTick > tout ){
			goto eprFault;
		}
	}
	EEPROM_I2C->TXDR = (addr >> 8) & 0xFF;
	while ( !(EEPROM_I2C->ISR & I2C_ISR_TXIS) ){
		if ( myTick > tout ){
			goto eprFault;
		}
	}
	EEPROM_I2C->TXDR = addr & 0xFF;
	while ( !(EEPROM_I2C->ISR & I2C_ISR_STOPF) ){
		if ( myTick > tout ){
			goto eprFault;
		}
	}
  /* Clear STOP Flag */
  EEPROM_I2C->ICR |= I2C_ICR_STOPCF;
	return EPR_OK;

	eprFault:
		I2C_Cmd( EEPROM_I2C, DISABLE );
		NVIC_DisableIRQ( I2C1_IRQn );
		Error_Handler( LOG_ERR );
		return (eeprom.status = EPR_ERR);

}

int8_t eepromSend(  uint8_t * data, uint16_t len ){
	uint32_t tout;

	EEPROM_I2C->CR1 &= ~(I2C_IT_ERRI);

	// Количество передаваемых байт
	EEPROM_I2C->CR2 &= ~I2C_CR2_NBYTES;
	EEPROM_I2C->CR2 |= (len & 0xFF) << 16;
	// Без перезагрузки
	EEPROM_I2C->CR2 &= ~I2C_CR2_RELOAD;
	// С Автостопом
	EEPROM_I2C->CR2 |= I2C_CR2_AUTOEND;
	// Запись
	EEPROM_I2C->CR2 &= ~I2C_CR2_RD_WRN;
	// Стартуем
	EEPROM_I2C->CR2 |= I2C_CR2_START;

	tout = myTick + I2C_TOUT;
	for( uint16_t i = 0; i < len; i++){
		while ( !(EEPROM_I2C->ISR & I2C_ISR_TXIS) ){
			if ( myTick > tout ){
				goto eprFault;
			}
		}
		EEPROM_I2C->TXDR = *(data++);
	}
	tout = myTick + I2C_TOUT;
	while ( !(EEPROM_I2C->ISR & I2C_ISR_STOPF) ) {
		if ( myTick > tout ){
			return EPR_TIMEOUT;
		}
	}
  /* Clear STOP Flag */
  EEPROM_I2C->ICR |= I2C_ICR_STOPCF;
  /* Disable ERR, TC, STOP, NACK, TXI interrupt */
  EEPROM_I2C->CR1 &= ~(I2C_IT_ERRI);

  /* Clear Configuration Register 2 */
  EEPROM_I2C->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN | I2C_CR2_AUTOEND );

	return EPR_OK;

	eprFault:
		I2C_Cmd( EEPROM_I2C, DISABLE );
		NVIC_DisableIRQ( I2C1_IRQn );
		Error_Handler( LOG_ERR );
		return (eeprom.status = EPR_ERR);
}

int8_t eepromRead( uint8_t * data, uint16_t len ){

	eeprom.rxSize = len;
	eeprom.count = len;
	eeprom.rxPtr = data;

//	EEPROM_I2C->CR1 |= I2C_IT_ERRI | I2C_IT_TCI| I2C_IT_STOPI| I2C_IT_NACKI | I2C_IT_TXI;
        EEPROM_I2C->CR1 |= I2C_IT_ERRI;

	// Количество передаваемых байт
	EEPROM_I2C->CR2 &= ~I2C_CR2_NBYTES;
	EEPROM_I2C->CR2 |= (len & 0xFF) << 16;
	// Без перезагрузки
	EEPROM_I2C->CR2 &= ~I2C_CR2_RELOAD;
	// С Автостопом
	EEPROM_I2C->CR2 |= I2C_CR2_AUTOEND;
	// Запись
	EEPROM_I2C->CR2 |= I2C_CR2_RD_WRN;
	// Стартуем
	EEPROM_I2C->CR2 |= I2C_CR2_START;


	uint32_t tout = myTick + I2C_TOUT;
	while ( eeprom.count ) {
		while ( !(EEPROM_I2C->ISR & I2C_ISR_RXNE) ){
			if ( myTick > tout ){
				return EPR_TIMEOUT;
			}
		}
		*eeprom.rxPtr++ = EEPROM_I2C->RXDR;
		eeprom.count--;
		eeprom.rxSize++;
	}
	while ( !(EEPROM_I2C->ISR & I2C_ISR_STOPF) ) {
		if ( myTick > tout ){
			return EPR_TIMEOUT;
		}
	}
  /* Clear STOP Flag */
  EEPROM_I2C->ICR |= I2C_ICR_STOPCF;

  /* Disable ERR, TC, STOP, NACK, TXI interrupt */
  EEPROM_I2C->CR1 &= ~(I2C_IT_ERRI);

  /* Clear Configuration Register 2 */
  EEPROM_I2C->CR2 &= ~(I2C_CR2_SADD | I2C_CR2_HEAD10R | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_RD_WRN | I2C_CR2_AUTOEND );

	return EPR_OK;
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
      eeprom.status = EPR_ERR;
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

    eeprom.status = EPR_READY;

    eepromTxCpltCallback();

  }
  else if( EEPROM_I2C->ISR & I2C_ISR_NACKF )
  {
    EEPROM_I2C->ICR |= I2C_ICR_NACKCF;

    eeprom.status = EPR_ERR;
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

