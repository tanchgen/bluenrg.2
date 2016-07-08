/*
 *
 * logger.h
 *  Created on: Jul 6, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include "onewire.h"
#include "my_time.h"

typedef struct {
	tXtime xTime;
	uint16_t toData[TO_DEV_NUM];
} tToLogUnit;

typedef struct {
	tXtime xTime;
	uint16_t ddData;
} tDdLogUnit;

typedef struct {
	uint8_t begin;
	uint8_t end;
	uint8_t full;
	uint32_t bufAddr;
	uint8_t size;								//
	uint32_t len;
} tLogBuf;

#define TO_LOG_RECORD_NUM			10080
#define TO_LOG_RECORD_SIZE		sizeof(tToLogUnit)
#define TO_LOG_START_ADDR			512
#define TO_LOG_END_ADDR				(TO_LOG_START_ADDR + (TO_LOG_RECORD_NUM * TO_LOG_RECORD_SIZE) - 1)

#define DD_LOG_RECORD_NUM			1600
#define DD_LOG_RECORD_SIZE		sizeof(tDdLogUnit)
#define DD_LOG_START_ADDR			(TO_LOG_END_ADDR + 1)
#define DD_LOG_END_ADDR				(DD_LOG_START_ADDR + (DD_LOG_RECORD_NUM * DD_LOG_RECORD_SIZE) - 1)

extern tLogBuf toLogBuff;
extern tLogBuf ddLogBuff;

// Записывает байт в буфер
int8_t logWriteBuff( tLogBuf * buf, uint8_t * data );

// Считывает байт из буфера. Если буфер пуст - возвращает (-1)
int16_t logReadBuff( tLogBuf * buf, uint8_t * data );

int8_t toLogWrite( void );

#endif /* LOGGER_H_ */
