/*
 *
 * init.c
 *  Created on: Jul 5, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 *
 */

#include <string.h>
#include "stm32f0xx_hal_def.h"
#include "stm32xx_it.h"
#include "onewire.h"
#include "thermo.h"
#include "logger.h"
#include "eeprom.h"
#include "my_service.h"
#include "init.h"
#include "my_main.h"

uint32_t toLogTout;										// Таймаут для логгирования температуры
uint32_t toLogCount;
uint32_t toReadCount;
uint32_t ddReadTout;										// Таймаут для считывания температуры
uint32_t ddReadCount;

void Error_Handler( eErrStatus err );

eErrStatus devInit( void ) {

	owStatus = OW_OK;

	OW_Init();

  // Находим ow-устройства
	uint8_t tmpAddr[OW_DEV_NUM*8];
	memset(tmpAddr, 0x00, OW_DEV_NUM*8);
  owDevNum = OW_Scan( tmpAddr, OW_DEV_NUM );
  if ( owDevNum <= 0 ) {
    // Не хватает термометров или не хватает датчиков двери -
    // предполагаем, что проблема с проводом
    Error_Handler(OW_WIRE_ERR);
    return OW_WIRE_ERR;
  }
  if( owDevNum > MAX_TO_DEV_NUM ) {
    owDevNum = MAX_TO_DEV_NUM;
  }

  // Проверка на наличие необходимых устройств
  for ( uint8_t i =0; i < owDevNum; i++) {
  	uint64_t tmp = *((uint64_t *)(tmpAddr + i*8));
    owDev[i].addr = tmp;
		if ( (tmp & 0xFF) == DS1820_SERIAL ){
			owDev[i].devType = DEV_TO;
			toDevNum++;
		  // Установки для термометров
	    owToInit( &owDev[i] );
		}
		if ( (tmp & 0xFF) == DS2413_SERIAL ){
			owDev[i].addr = tmp;
      owDev[i].devType = DEV_DD;
			ddDevNum++;
			owDdInit( &(owDev[i]) );
		}
  }
  // Устанавливаем таймаут сбора информации
	toReadCount = TO_READ_TOUT;
	toTempStart();

  return OW_OK;
}

eErrStatus owToInit( tOwDev * toDev ) {
	eErrStatus err = OW_OK;
	tOwSendBuf sendBuf;

	if ( !(toDev->addr) ) {
		toDev->newErr = TRUE;
		Error_Handler(OW_TO_DEV_ERR);
		return ( toDev->devStatus = OW_TO_DEV_ERR );
	}
// Выбираем датчик
// Формируем массив с командой и адресом
	sendBuf.cmd =MATCH_ROM;
	sendBuf.adr = toDev->addr;
// Отправляем в шину
	if ((err = OW_Send(OW_SEND_RESET, (uint8_t*)&sendBuf, 9, NULL, 0, OW_NO_READ)) == OW_ERR ) {
		toDev->newErr = TRUE;
		toDev->addr = 0;
		toDev->devStatus = OW_TO_DEV_ERR;
	}
	else {
		toDev->newErr = FALSE;
		toDev->devStatus = OW_DEV_OK;

// Устанавливаем точность измерения
		toDev->mesurAcc = MESUR_ACC;
// Выставляем допустимый Минимум температуры
		if( (toDev->tMin = TEMPER_MIN) & 0x800 ) {
			toDev->tMin |= 0xF000;
		}
		// Выставляем допустимый Максимум температуры
		if( (toDev->tMax = TEMPER_MAX) & 0x800 ) {
			toDev->tMax |= 0xF000;
		}

		toSetConfig( toDev );
// Сохраняем в ПЗУ
		toWriteEeprom( toDev->addr );
	}

	return err;
}

void owDdInit( tOwDev * pddDev ){
	// Установки для датчиков дверей
	if ( !pddDev->addr ) {
	  pddDev->newErr = TRUE;
		Error_Handler(OW_DD_DEV_ERR);
    pddDev->addr = 0;
		pddDev->devStatus = OW_DD_DEV_ERR;
	}
  else {
    pddDev->newErr = FALSE;
    pddDev->devStatus = OW_DEV_OK;
  }
	pddDev->ddState = 0x11;
	pddDev->ddStatePrev = 0x11;
	// Устанавливаем таймаут сбора информации
	ddReadTout = DD_READ_TOUT;
	ddReadCount = DD_READ_TOUT;
}

int8_t logInit( void ){
	int8_t err = HAL_ERROR;
	toLogBuff.bufAddr = 0;
	ddLogBuff.bufAddr = 0;

	// Иициализация EEPROM
	if ( !eepromInit() ){
// ************ Инициализация Логирования TO ******************

		// Восстанавливаем состояние данных буфера логгера
		err = receiveEeprom( TO_LOG_START_ADDR, (uint8_t *)&toLogBuff, sizeof(tLogBuf) );
		if ( (err != HAL_OK) || (toLogBuff.bufAddr != (TO_LOG_START_ADDR + sizeof(tLogBuf))) ) {
			toLogBuff.begin = 0;
			toLogBuff.end = 0;
			toLogBuff.full = 0;
			// Размер одной записи
			toLogBuff.size = sizeof(tXtime) + (2 * toDevNum) + 1;
			// Количество записей в буфере.
			toLogBuff.len = (TO_LOG_END_ADDR - TO_LOG_START_ADDR - sizeof(tLogBuf));
			toLogBuff.b1 = 0;
			// Оставляем место для сохранения состояния структуры toLogBuff
			toLogBuff.bufAddr = TO_LOG_START_ADDR + sizeof(tLogBuf);
		}

// ************ Инициализация Логирования DD ******************

		// Восстанавливаем состояние данных буфера логгера
		err = receiveEeprom( DD_LOG_START_ADDR, (uint8_t *)&ddLogBuff, sizeof(tLogBuf) );
		if ( (err != HAL_OK) || (ddLogBuff.bufAddr != (DD_LOG_START_ADDR + sizeof(tLogBuf))) ) {
			// Инициализация Логирования DD
			ddLogBuff.begin = 0;
			ddLogBuff.end = 0;
			ddLogBuff.full = 0;
			ddLogBuff.size = DD_LOG_RECORD_SIZE;
			ddLogBuff.len = DD_LOG_RECORD_NUM;
			ddLogBuff.b1 = 0;
			// Оставляем место для сохранения состояния структуры ddLogBuff
			ddLogBuff.bufAddr = DD_LOG_START_ADDR + sizeof(tLogBuf);
		}
	}

	blue.logStatus.toTxe = TRUE;
	blue.logStatus.ddTxe = TRUE;
	// Флаг - Запрос очередной записи Лога температуры
  blue.logStatus.toReq = DISABLE;
	// Флаг - Запрос очередной записи Лога двери
	blue.logStatus.ddReq = DISABLE;

	// Таймер для записи логов температуры
	toLogTout = TO_LOG_TOUT;
	toLogCount = TO_LOG_TOUT;

	return err;
}

int8_t alrmInit( void ){

	// Битовая маска Идентификаторов Тревог
 	blue.alrmId = ALARM_GENERIC	| \
	 								ALARM_DD_NEW_STATE | \
	 								ALARM_TO_MAX | \
	 								ALARM_TO_MIN | \
	 								ALARM_DD_FAULT | \
	 								ALARM_TO_FAULT | \
									ALARM_LOG_FAULT | \
									ALARM_HW_FAULT;
	blue.alrmNewId = 0;
	blue.alrmNewCount = 0;
	blue.alrmNoReadId = 0;
	blue.alrmNoReadCount = 0;
	blue.alrmSendId = 0;

	return 0;
}
