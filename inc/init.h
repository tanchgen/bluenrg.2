/*
 *
 * init.h
 *  Created on: Jul 5, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#ifndef INIT_H_
#define INIT_H_

#include "my_main.h"
#include "onewire.h"

eErrStatus toInit( void );
void ddInit( void );
eErrStatus owToDevInit( uint8_t toDev );
int8_t logInit( void );
int8_t alrmInit( void );

extern uint32_t toLogTout;										// Таймаут для логгирования температуры
extern uint32_t toLogCount;

extern uint32_t toReadCount;

extern uint32_t toMesgTout;									// Таймаут для предачи температуры

extern uint32_t ddReadTout;										// Таймаут для считывания температуры
extern uint32_t ddReadCount;

#endif /* INIT_H_ */
