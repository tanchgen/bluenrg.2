/*
 *
 * eeprom.h
 *  Created on: Jul 8, 2016
 *      Author: Gennady Tanchin <g.tanchin@yndex.ru>
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include "stm32f0xx.h"


#define EEPROM_PORT					GPIOA
#define EEPROM_SCL_PIN			GPIO_Pin_9
#define EEPROM_SCL_PIN_NUM	9
#define EEPROM_SDA_PIN			GPIO_Pin_10
#define EEPROM_SDA_PIN_NUM	10

#define EEPROM_I2C						I2C1
#define I2C_FAST_SPEED				400000
#define I2C_TIMING						0x00B01A4B
#define I2C_OWN_ADDR					0x00
#define I2C_TOUT								100
#define EEPROM_I2C_ADDR				(0x50)

#define I2C_TRANSMITTER_MODE		0
#define I2C_RECEIVER_MODE				1
#define I2C_ACK_ENABLE         	1
#define I2C_ACK_DISABLE        	0
#define I2C_NO_RELOAD						0

typedef enum {
	I2C_OK,
	I2C_READY,
	I2C_BUSY,
	I2C_TIMEOUT,
	I2C_ERR
} eI2CStatus;

typedef struct {
	uint8_t txData[255];
	uint8_t *txPtr;
	uint8_t rxData[255];
	uint8_t *rxPtr;
	uint8_t txSize;
	uint8_t rxSize;
	uint8_t count;
	eI2CStatus status;
} tEeprom;

extern tEeprom eeprom;

int8_t eepromInit( void );
void i2cInit( void );
void i2cMspInit( void );

int8_t sendEeprom( uint32_t addr, uint8_t * data, uint16_t len);
int8_t receiveEeprom( uint32_t addr, uint8_t * data, uint16_t len);

int8_t sendEepromAddr( uint32_t addr );
int8_t eepromSendIT(  uint8_t * data, uint16_t len );
int8_t eepromRead( uint8_t * data, uint16_t len );

void eepromTxCpltCallback( void );
void eepromErrorCallBack( void );

void epprom_IRQHandler( void );

#endif /* EEPROM_H_ */
