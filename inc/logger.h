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
	uint8_t toData[TO_DEV_NUM*2];
} tToLogUnit;

typedef struct {
	tXtime xTime;
	uint8_t ddData;
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
#define TO_LOG_START_ADDR			(uint32_t)512
#define TO_LOG_END_ADDR				(TO_LOG_START_ADDR + (TO_LOG_RECORD_NUM * TO_LOG_RECORD_SIZE) - 1)

#define DD_LOG_RECORD_NUM			5000
#define DD_LOG_RECORD_SIZE		sizeof(tDdLogUnit)
#define DD_LOG_START_ADDR			(TO_LOG_END_ADDR + 1)
#define DD_LOG_END_ADDR				(DD_LOG_START_ADDR + (DD_LOG_RECORD_NUM * DD_LOG_RECORD_SIZE) - 1)

#define TO_LOG_ENABLE					0x01
#define TO_LOG_TXE						0x02
#define TO_LOG_RNXE						0x04

#define DD_LOG_ENABLE					0x10
#define DD_LOG_TXE						0x20
#define DD_LOG_RNXE						0x40

extern tLogBuf toLogBuff;
extern tLogBuf ddLogBuff;

// Записывает байт в буфер
int8_t logWriteBuff( tLogBuf * buf, uint8_t * data );
// Считывает байт из буфера. Если буфер пуст - возвращает (-1)
int16_t logReadBuff( tLogBuf * buf, uint8_t * data );

int8_t toLogWrite( void );
int8_t toLogRead( tToLogUnit * toLog );
int8_t ddLogWrite( void );
int8_t ddLogRead( tDdLogUnit * ddLog );

#endif /* LOGGER_H_ */
