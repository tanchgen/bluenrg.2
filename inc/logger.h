/*
 *
 * logger.h
 *  Created on: Jul 6, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include "onewire.h"

typedef struct {
	uint32_t time;
	uint16_t toData[TO_DEV_NUM];
};

typedef struct {
	uint8_t begin;
	uint8_t end;
	uint8_t full;
	uint32_t bufAddr;
	uint8_t size;								//
	uint32_t len;
} tLogBuf;

extern tLogBuf toLogBuff;
// Записывает байт в буфер
int8_t logWriteBuff( tLogBuf * buf, uint8_t * data );

// Считывает байт из буфера. Если буфер пуст - возвращает (-1)
int16_t logReadBuff( tLogBuf * buf, uint8_t * data, uint16_t len );

int8_t sendEprom( uint32_t addr, uint8_t * data, uint16_t len);
int8_t readEprom( uint32_t addr, uint8_t * data, uint16_t len);
int8_t toLogWrite( void );

#endif /* LOGGER_H_ */
