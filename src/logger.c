/*
 *
 * logger.c
 *  Created on: Jul 6, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */
#include "logger.h"
#include "eeprom.h"
#include "my_time.h"

tLogBuf toLogBuff;
tLogBuf ddLogBuff;

int8_t logWriteBuff( tLogBuf * buf, uint8_t * data ) {
	if ( (buf->end == buf->begin) && ( buf->full == 1) ){
		buf->begin ++;
	}
	if (buf->begin == buf->len ) {
		buf->begin = 0;
	}
//	*(buf->bufAddr + buf->end) = data;
	if( sendEeprom( (buf->bufAddr + buf->end * buf->size), data, buf->size ) < 0 ){
		return -1;
	}

	buf->end++;
	if (buf->end == buf->len ) {
		buf->end = 0;
	}
	if (buf->end == buf->begin) {
		buf->full = 1;
	}
	return 1;
}

int16_t logReadBuff( tLogBuf * buf, uint8_t * data ) {
	if ( (buf->begin == buf->end) && !buf->full) {
		return 0;
	}
	buf->full = 0;

//	ch =*(buf->bufAddr + buf->begin);
	if ( receiveEeprom( (buf->bufAddr + buf->begin * buf->size), data, buf->size ) < 0){
		return -1;
	}
	buf->begin++;
	if (buf->begin == buf->len) {
		buf->begin = 0;
	}
	return 1;
}

int8_t toLogWrite( void ) {
	tToLogUnit toLogUnit;

	// Записываем штамп времени
	toLogUnit.xTime = getRtcTime();
	// Записываем показания датчиков
	for( uint8_t i = 0; i < TO_DEV_NUM; i++ ){
		// Сохраняем номер датчика в старшей тетраде и значение температуры в младших трех тетрадах:
		//		xxxx yyyy yyyy yyyy : x - номер датчика, y - 12-битное значение температуры этого датчика
		toLogUnit.toData[i] = owToDev[i].temper | ( i << 12);
	}

	return logWriteBuff( &toLogBuff, (uint8_t *)&toLogUnit );
}

int8_t ddLogWrite( void ) {
	tDdLogUnit ddLogUnit;

	// Записываем штамп времени
	ddLogUnit.xTime = getRtcTime();
	// Записываем показания датчиков
	for( uint8_t i = 0; i < DD_DEV_NUM; i++ ){
		// Сохраняем номер датчика в старшей тетраде и значение температуры в младших трех тетрадах:
		//		xxxx yyyy yyyy yyyy : x - номер датчика, y - 12-битное значение температуры этого датчика
		ddLogUnit.ddData = owDdDev[i].ddData;
	}

	return logWriteBuff( &ddLogBuff, (uint8_t *)&ddLogUnit );
}


