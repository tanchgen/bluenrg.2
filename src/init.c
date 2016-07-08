/*
 *
 * init.c
 *  Created on: Jul 5, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 *
 */

#include <string.h>
#include "stm32xx_it.h"
#include "onewire.h"
#include "logger.h"
#include "eeprom.h"
#include "init.h"

uint32_t toLogTout;										// Таймаут для логгирования температуры
uint32_t toLogTimer;
uint32_t toReadTout;									// Таймаут для считывания температуры
uint32_t toReadTimer;
uint32_t toMesgTout;									// Таймаут для предачи температуры
uint32_t toMesgTimer;

uint32_t ddReadTout;										// Таймаут для считывания температуры
uint32_t ddReadTimer;

eOwStatus owInit( void ) {

	uint8_t termNum = 0;
	uint8_t ddNum = 0;

	owStatus = OW_OK;

	OW_Init();

  // Находим ow-устройства
	uint8_t tmpAddr[OW_DEV_NUM*8];
  owDevNum = OW_Scan( tmpAddr, OW_DEV_NUM );
  // Проверка на наличие необходимых устройств
  for ( uint8_t i =0; i < owDevNum; i++) {
  	uint64_t tmp = *((uint64_t *)(tmpAddr + i*8));
		if ( (tmp & 0xFF) == 0x28 ){
			owToDev[termNum++].addr = tmp;
		}
		else {
			owDdDev[ddNum++].addr = tmp;
		}
  }
	if ( (termNum < TO_DEV_NUM) || (ddNum < DD_DEV_NUM) ) {
		// Не хватает термометров или не хватает датчиков двери -
		// предполагаем, что проблема с проводом
		return OW_WIRE_ERR;
	}
	else if( (termNum != TO_DEV_NUM) || (ddNum != DD_DEV_NUM) ) {
 		// Количество термометров или всех устройств не соответствует указанным
 		// в "onewire.h"
 		return OW_DEV_ERR;
 	}

	// Установки для термометров
  for (uint8_t i = 0; i < TO_DEV_NUM; i ++ ){
  	owToDevInit( i );
  }
// Устанавливаем таймаут сбора информации
	toLogTout = TO_LOG_TOUT;
	toLogTimer = TO_LOG_TOUT;

	toReadTout = TO_READ_TOUT;
	toReadTimer = TO_READ_TOUT;

	toMesgTout = TO_MESG_TOUT;
	toMesgTimer = TO_MESG_TOUT;

// Установки для датчиков дверей
// Устанавливаем таймаут сбора информации
 	ddReadTout = DD_READ_TOUT;
 	ddReadTimer = DD_READ_TOUT;

  return OW_OK;
}

eOwStatus owToDevInit( uint8_t toDev ) {
	eOwStatus err = OW_OK;
	uint8_t data[9];

// Выбираем датчик
// Формируем массив с командой и адресом
	data[0] =MATCH_ROM;
	memcpy( &data[1], (uint8_t *)&(owToDev[toDev].addr), 8);
// Отправляем в шину
	if ((err = OW_Send(OW_SEND_RESET, data, 9, NULL, 0, OW_NO_READ)) == OW_DEV_ERR ) {
		owToDev[toDev].addr = 0;
		owToDev[toDev].devStatus = OW_DEV_ERR;
	}
	else {
// Устанавливаем точность измерения
// Формируем массив с командой и адресом
		data[0] = MEM_WRITE;
		data[1] = TERM_MAX;
		data[2] = TERM_MIN;
		data[3] = (0x1F | ((owToDev[toDev].mesurAcc-9) << 5)) ;
		OW_Send(OW_NO_RESET, data, 4, NULL, 0, OW_NO_READ);

// Сохраняем в ПЗУ
// Формируем массив с командой и адресом
		data[0] = RAM_TO_EEPROM;
		OW_Send(OW_NO_RESET, data, 1, NULL, 0, OW_NO_READ);
// При паразитном питании требуется дополнительная подтяжка провода шины к питанию на >10мс
// Включаем подтяжку шины 1-Wire к Vdd
		OW_PORT->MODER &= ~(3 << (OW_RX_PIN_NUM*2));			// Стираем Alternate Function
		OW_PORT->MODER |= 1 << (OW_RX_PIN_NUM*2);					// Выставляем в OUTPUT
		OW_PORT->BSRR |= OW_RX_PIN_NUM;										// Выставляем в 1
		myDelay(12);
// Восстанавливаем работу USART_RX - вывода
		OW_PORT->MODER &= ~(3 << (OW_RX_PIN_NUM*2));			// Стираем Alternate Function
		OW_PORT->MODER |= 2 << (OW_RX_PIN_NUM*2);					// Выставляем в Alternate Function
	}

	return err;
}

int8_t logInit( void ){
	// Инициализация Логирования TO
	toLogBuff.begin = 0;
	toLogBuff.end = 0;
	toLogBuff.full = 0;
	toLogBuff.size = TO_LOG_RECORD_SIZE;
	toLogBuff.bufAddr = TO_LOG_START_ADDR;
	toLogBuff.len = TO_LOG_RECORD_NUM;
	// Инициализация Логирования DD
	ddLogBuff.begin = 0;
	ddLogBuff.end = 0;
	ddLogBuff.full = 0;
	ddLogBuff.size = DD_LOG_RECORD_SIZE;
	ddLogBuff.bufAddr = DD_LOG_START_ADDR;
	ddLogBuff.len = DD_LOG_RECORD_NUM;

	// Иициализация EEPROM
	return eepromInit();
}

// Задержка в мс
void myDelay( uint32_t del ){
	uint32_t finish = myTick + del;
	while ( myTick < finish)
	{}
}
