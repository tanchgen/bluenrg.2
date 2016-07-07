/*
 *
 * logger.c
 *  Created on: Jul 6, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */
#include "logger.h"
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
	if( sendEprom( (buf->bufAddr + buf->end * buf->size), data, buf->size ) < 0 ){
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

int16_t logReadBuff( tLogBuf * buf, uint8_t * data, uint16_t len ) {
	if ( (buf->begin == buf->end) && !buf->full) {
		return 0;
	}
	buf->full = 0;

//	ch =*(buf->bufAddr + buf->begin);
	if ( readEprom( (buf->bufAddr + buf->begin), data, len ) < 0){
		return -1;
	}
	buf->begin++;
	if (buf->begin == buf->len) {
		buf->begin = 0;
	}
	return 1;
}

int8_t sendEprom( uint32_t addr, uint8_t * data, uint16_t len) {
	/* TODO: Запись в EPROM данных
	 * addr - стартовый адрес в Eprom
	 * data - Указатель на буфер с данными
	 * len - длина записываемых данных в байта
	 *
	 * Организовать проверку на превышение границы памяти
	 */
	return len;
}

int8_t readEprom( uint32_t addr, uint8_t * data, uint16_t len) {
	/* TODO: Чтение данных из EPROM
	 * addr - стартовый адрес в Eprom
	 * data - Указатель на буфер, куда будут складываться данные
	 * len - длина записываемых данных в байта
	 *
	 * Организовать проверку на превышение границы памяти
	 */
	return len;
}

int8_t toLogWrite( void ) {
	uint8_t data[4+TO_DEV_NUM*2];

	// Записываем штамп времени
	*((uint32_t *)&data[0]) = (uint32_t)getRtcTime();
	// Записываем показания датчиков
	for( uint8_t i = 0; i < TO_DEV_NUM; i++ ){
		// Сохраняем номер датчика в старшей тетраде и значение температуры в младших трех тетрадах:
		//		xxxx yyyy yyyy yyyy : x - номер датчика, y - 12-битное значение температуры этого датчика
		*((uint16_t *)&data[i*2+4]) = owToDev[i].temper | ( i << 12);
	}

	return logWriteBuff( &toLogBuff, data );
}
