/*
 *
 * logger.c
 *  Created on: Jul 6, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */
#include <string.h>
#include "stm32f0xx_hal_def.h"
#include "eeprom.h"
#include "my_time.h"
#include "my_service.h"
#include "my_main.h"
#include "logger.h"

tLogBuf toLogBuff;
tLogBuf ddLogBuff;
uint8_t toEmptyFill;
uint8_t ddEmptyFill;

int8_t saveStateBuff( tLogBuf *buf ){
	// Сохраняем состояние данных буфера логгера
	if( sendEeprom( (buf->bufAddr - sizeof(tLogBuf)), (uint8_t *)buf, sizeof(tLogBuf) ) != HAL_OK ) {
		return -1;
	}
	return 0;
}

int8_t logWriteBuff( tLogBuf * buf, uint8_t * data ) {
	if ( (buf->end == buf->begin) && ( buf->full == 1) ){
		buf->begin ++;
	}
	if (buf->begin == buf->len ) {
		buf->begin = 0;
	}
//	*(buf->bufAddr + buf->end) = data;
	if( sendEeprom( (buf->bufAddr + buf->end * buf->size), data, buf->size )){
		return -1;
	}

	buf->end++;
	if (buf->end == buf->len ) {
		buf->end = 0;
	}
	if (buf->end == buf->begin) {
		buf->full = 1;
	}
	// Сохраняем состояние данных буфера логгера
	if( saveStateBuff( buf ) ) {
		return -1;
	}

	return 1;
}

int16_t logStartReadBuff( tLogBuf * buf, uint8_t * data ) {
	if ( (buf->begin == buf->end) && !buf->full) {
		return 0;
	}
	buf->full = 0;

//	ch =*(buf->bufAddr + buf->begin);
	if ( receiveEeprom( (buf->bufAddr + buf->begin * buf->size), data, buf->size ) != HAL_OK){
		return -1;
	}
	buf->readStart = 1;
	return 1;
}

int16_t logEndReadBuff( tLogBuf * buf ){
	buf->begin++;
	if (buf->begin == buf->len) {
		buf->begin = 0;
	}
	buf->readStart = 0;
	if( saveStateBuff( buf ) ){
		return -1;
	}
	return 0;
}
int8_t toLogWrite( void ) {
	uint8_t toLogUnit[sizeof(tXtime)+sizeof(uint64_t)];
#if (TO_DEV_NUM > 4)
#error "Данная функция написана для количества датчиков не превышающих 4"
#endif
	uint64_t *toPtr = (uint64_t *)(toLogUnit+sizeof(tXtime));

	// Записываем штамп времени
	*((tXtime *)toLogUnit) = uxTime;
	// Записываем показания датчиков
	*toPtr = 0;
	for( uint8_t i = 0; i < TO_DEV_NUM; i++ ){
		// Сохраняем значения температуры в следующем виде:
		//	0x44 0x43 0x33 0x22 0x21 0x11 ,
		//  где 4,3,2,1 - значения температуры соответствующих датчиков

		*toPtr |= ((uint64_t)(owToDev[i].temper & 0xFFF)) << (i * 12);
	}

	return logWriteBuff( &toLogBuff, (uint8_t *)&toLogUnit );
}

int8_t toLogRead( tToLogUnit * toLog ){
#if (TO_DEV_NUM > 4)
#error "Данная функция написана для количества датчиков не превышающих 4"
#endif
	uint8_t toBuf[sizeof(tXtime)+sizeof(uint64_t)];
	uint64_t *toDataSrc;// = (uint64_t *)(toLog+sizeof(tXtime));
	uint16_t *toDataDst;
	int8_t rd;

	switch( rd = logStartReadBuff( &toLogBuff, (uint8_t *)toBuf ) )  {
		case -1:
			// Ошибка чтения логов - на сервер будем отправлять значения, заполненные 0xFF
			memset( toLog, 0xFF, sizeof(tToLogUnit) );
			break;
		case 1:
			// Прочитана 1 запись
			toLog->xTime = ((tToLogUnit *)toBuf)->xTime;
			toDataSrc = (uint64_t *)((tToLogUnit *)toBuf)->toData;
			toDataDst = (uint16_t *)toLog->toData;
			for( uint8_t i=0; i < 4; i++ ){
				*toDataDst++ = (*toDataSrc) & 0xFFF;
				*toDataSrc >>=12;
			}
			break;
		default:
			// В логе нет непрочитанных записей - на сервер будем отправлять значения, заполненные 0x00
			// memset( toLog, 0x00, sizeof(tToLogUnit) );
			__NOP();
	}
	return rd;
}

int8_t ddLogWrite( void ) {
	tDdLogUnit ddLogUnit;
	uint8_t i;

	// Записываем штамп времени
	ddLogUnit.xTime = uxTime;
	// Записываем показания датчиков
	ddLogUnit.ddData = 0;
#if OW_DD
	for( i = 0; i < OW_DD_DEV_NUM; i++ ){
		// Сохраняем номер датчика в старшей тетраде и значение температуры в младших трех тетрадах:
		//		xxxx yyyy yyyy yyyy : x - номер датчика, y - 12-битное значение температуры этого датчика
		ddLogUnit.ddData |= (owDdDev[i].ddData[0] | (owDdDev[i].ddData[1]) << 1 ) << (i*2);
#else
	for( i = 0; i < DD_DEV_NUM; i++ ){
		ddLogUnit.ddData |= (ddDev[i].ddData & 0x1) << i;
#endif
	}
	if( i ) {
		return logWriteBuff( &ddLogBuff, (uint8_t *)&ddLogUnit );
	}
	return 0;
}

int8_t ddLogRead( tDdLogUnit * ddLog ) {
	int8_t rd;

	switch( rd = logStartReadBuff( &ddLogBuff, (uint8_t *)ddLog ) ){
		case -1:
			// Ошибка чтения логов - на сервер будем отправлять значения, заполненные 0xFF
			memset( ddLog, 0xFF, sizeof(tDdLogUnit) );
			break;
		case 0:
			// В логе нет непрочитанных записей - на сервер будем отправлять значения, заполненные 0x00
			memset( ddLog, 0x00, sizeof(tDdLogUnit) );
			break;
	}

	return rd;
}

int8_t alrmUpdate( uint8_t alrmId ) {
	uint8_t mask;

	if ( (mask = alrmId & blue.alrmId) ) {
		blue.alrmNewId |= mask;
		blue.alrmNoReadId |= mask;
		blue.alrmNewCount++;
		blue.alrmNoReadCount++;

		if ( blue.connected ) {
			alrmCharUpdate();
		}
		return 0;
	}
	return -1;
}

int8_t logSend( uint8_t toLogSendEn, uint8_t ddLogSendEn) {
	uint8_t logFill = FALSE;
	uint8_t tmpBuf[17];
	int8_t ret;

	memset( tmpBuf, 0x00, 17);

	if ( toLogSendEn ) {
		if ( blue.logStatus.toTxe ) {
			ret = toLogRead( (tToLogUnit *)tmpBuf );
			if ( ret < 0) {
				logFill = TRUE;
				alrmUpdate( ALARM_LOG_FAULT );
			}
			else if( ret > 0){
				logFill = TRUE;
				toEmptyFill = FALSE;
				blue.logStatus.toTxe = DISABLE;
			}
			else if( toEmptyFill == FALSE){
				logFill = TRUE;
				toEmptyFill = TRUE;
				blue.logStatus.toTxe = ENABLE;
			}
		}
	}
	else if( toEmptyFill == FALSE){
		logFill = TRUE;
		toEmptyFill = TRUE;
		blue.logStatus.toTxe = ENABLE;
	}

	if ( ddLogSendEn ) {
		if ( blue.logStatus.ddTxe ) {
			ret = ddLogRead( (tDdLogUnit *)&tmpBuf[12] );
			if ( ret < 0) {
				logFill = TRUE;
				alrmUpdate( ALARM_LOG_FAULT );
			}
			else if( ret > 0){
				logFill = TRUE;
				ddEmptyFill = FALSE;
				blue.logStatus.ddTxe = DISABLE;
			}
			else if( ddEmptyFill == FALSE){
				logFill = TRUE;
				ddEmptyFill = TRUE;
				blue.logStatus.ddTxe = ENABLE;
			}
		}
	}
	else if( ddEmptyFill == FALSE){
		logFill = TRUE;
		ddEmptyFill = TRUE;
		blue.logStatus.ddTxe = ENABLE;
	}

	if ( logFill ) {
		if( logCharUpdate( tmpBuf, 17) != BLE_STATUS_SUCCESS ){
			while(1)
			{}
		}
	}

	return 0;
}
