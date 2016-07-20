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
#include "thermo.h"
#include "logger.h"
#include "eeprom.h"
#include "init.h"

uint32_t toLogTout;										// Таймаут для логгирования температуры
uint32_t toLogCount;
uint32_t toReadTout;									// Таймаут для считывания температуры
uint32_t toReadCount;
uint32_t toMesgTout;									// Таймаут для предачи температуры
uint32_t toMesgCount;

uint32_t ddReadTout;										// Таймаут для считывания температуры
uint32_t ddReadCount;

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
	toLogCount = TO_LOG_TOUT;

	toReadTout = TO_READ_TOUT;
	toReadCount = TO_READ_TOUT;

	toMesgTout = TO_MESG_TOUT;
	toMesgCount = TO_MESG_TOUT;

// Установки для датчиков дверей
// Устанавливаем таймаут сбора информации
 	ddReadTout = DD_READ_TOUT;
 	ddReadCount = DD_READ_TOUT;

  return OW_OK;
}

eOwStatus owToDevInit( uint8_t toDev ) {
	eOwStatus err = OW_OK;
	uint8_t sendBuf[9];

// Выбираем датчик
// Формируем массив с командой и адресом
	sendBuf[0] =MATCH_ROM;
	*((uint64_t *)&sendBuf[1]) = owToDev[toDev].addr;
// Отправляем в шину
	if ((err = OW_Send(OW_SEND_RESET, sendBuf, 9, NULL, 0, OW_NO_READ)) == OW_DEV_ERR ) {
		owToDev[toDev].addr = 0;
		owToDev[toDev].devStatus = OW_DEV_ERR;
	}
	else {

// Устанавливаем точность измерения
		owToDev[toDev].mesurAcc = MESUR_ACC;
		toSetConfig( &owToDev[toDev] );
// Сохраняем в ПЗУ
		toWriteEeprom( &owToDev[toDev] );
	}

	return err;
}

int8_t logInit( void ){
	int8_t err = EPR_ERR;
	toLogBuff.bufAddr = 0;
	ddLogBuff.bufAddr = 0;

	// Иициализация EEPROM
	if ( !eepromInit() ){
// ************ Инициализация Логирования TO ******************

		// Восстанавливаем состояние данных буфера логгера
		err = receiveEeprom( TO_LOG_START_ADDR, (uint8_t *)toLogBuff, sizeof(tLogBuf) );
		if ( (err != EPR_OK) || (toLogBuff.bufAddr != TO_LOG_START_ADDR + sizeof(tLogBuf)) ) {
			toLogBuff.begin = 0;
			toLogBuff.end = 0;
			toLogBuff.full = 0;
			toLogBuff.size = TO_LOG_RECORD_SIZE;
			toLogBuff.len = TO_LOG_RECORD_NUM;
			// Оставляем место для сохранения состояния структуры toLogBuff
			toLogBuff.bufAddr = TO_LOG_START_ADDR + sizeof(tLogBuf);
		}

// ************ Инициализация Логирования DD ******************

		// Восстанавливаем состояние данных буфера логгера
		err = receiveEeprom( DD_LOG_START_ADDR, (uint8_t *)ddLogBuff, sizeof(tLogBuf) );
		if ( (err != EPR_OK) || (ddLogBuff.bufAddr != DD_LOG_START_ADDR + sizeof(tLogBuf)) ) {
			// Инициализация Логирования DD
			ddLogBuff.begin = 0;
			ddLogBuff.end = 0;
			ddLogBuff.full = 0;
			ddLogBuff.size = DD_LOG_RECORD_SIZE;
			ddLogBuff.len = DD_LOG_RECORD_NUM;
			// Оставляем место для сохранения состояния структуры ddLogBuff
			ddLogBuff.bufAddr = DD_LOG_START_ADDR + sizeof(tLogBuf);
		}
	}
	return err;
}
