/*
 *
 * logger.h
 *  Created on: Jul 6, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include "compiler.h"
#include "onewire.h"
#include "my_time.h"

#if (TO_DEV_NUM > MAX_TO_DEV_NUM)
#error "Maximum number of termosensors = 4"
#endif
#define LOG_TO_DATALEN    (MAX_TO_DEV_NUM * 2)

typedef __packed struct {
  tXtime xTime;
  uint16_t toData[6];
} __attribute__((packed)) tToLogBuf;

typedef __packed struct {
	tXtime xTime;
	uint8_t toNumber;
	uint16_t toData[MAX_TO_DEV_NUM];
} __attribute__((packed)) tToLogUnit;

typedef __packed struct {
	tXtime xTime;
	uint16_t ddData;
} __attribute__((packed)) tDdLogUnit;

typedef struct {
  uint32_t begin;
  uint32_t end;
  uint8_t full;
  uint32_t bufAddr;
  uint8_t size;
  uint32_t len;
  uint32_t b1;
} __attribute__((packed)) tLogBuf;

#define TO_LOG_LEN (sizeof(time_t) + (MAX_TO_DEV_NUM * 2))
#define DD_LOG_LEN (sizeof(tDdLogUnit))

//#define LOG_CHARACTERISTIC_LEN	((TO_LOG_LEN) + (DD_LOG_LEN))

#define EEPROM_SIZE           (uint32_t)128*1024

// #define TO_LOG_RECORD_NUM
#define TO_LOG_RECORD_SIZE		sizeof(tToLogUnit)
//#define TO_LOG_SAVE_ADDR			(uint32_t)16
#define TO_LOG_START_ADDR     (uint32_t)16
#define TO_LOG_END_ADDR				(uint32_t)((EEPROM_SIZE) - ((DD_LOG_RECORD_NUM) * (DD_LOG_LEN)) - sizeof(tLogBuf) - 1 )

#define DD_LOG_SAVE_ADDR      (TO_LOG_SAVE_ADDR + sizeof(tLogBuf))
#define DD_LOG_RECORD_NUM			5000
#define DD_LOG_RECORD_SIZE		sizeof(tDdLogUnit)
#define DD_LOG_START_ADDR			TO_LOG_END_ADDR + 1
//#define DD_LOG_END_ADDR				((DD_LOG_START_ADDR) + (DD_LOG_RECORD_NUM * DD_LOG_RECORD_SIZE) - 1)

#define TO_LOG_ENABLE					0x01
#define TO_LOG_TXE						0x02
#define TO_LOG_RNXE						0x04

#define DD_LOG_ENABLE					0x10
#define DD_LOG_TXE						0x20
#define DD_LOG_RNXE						0x40

extern tLogBuf toLogBuff;
extern tLogBuf ddLogBuff;
extern uint8_t toEmptyFill;
extern uint8_t ddEmptyFill;
extern uint8_t toCount;       // Счетчик значений температур из записи лога, отправленных в характеристику


int8_t saveStateBuff( tLogBuf *buf );
// Записывает байт в буфер
int8_t ddLogWriteEE( tLogBuf * buf, uint8_t * data );

// Начало считывания байт из буфера. Если буфер пуст - возвращает (-1)
int16_t toLogReadEEStart( tLogBuf * buf, uint8_t *data );
int16_t ddLogReadStart( tLogBuf * buf, uint8_t * data );

// Завершение считывания из буфера. В т.ч. и запись состояния буфера.
int16_t toLogReadEEEnd( tLogBuf * buf );
int16_t ddLogReadEnd( tLogBuf * buf );

int8_t toLogWrite( void );
int8_t toLogRead( tToLogUnit * toLog );
int8_t ddLogWrite( void );
int8_t ddLogRead( uint8_t * ddLog );
int8_t toLogSend( void );
int8_t ddLogSend( void );


#endif /* LOGGER_H_ */
