/*
 *
 * init.h
 *  Created on: Jul 5, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#ifndef INIT_H_
#define INIT_H_

#include "onewire.h"

eOwStatus owInit( void );
eOwStatus owToDevInit( uint8_t toDev );
void logInit( void );
void myDelay( uint32_t del );

extern uint32_t toLogTout;										// Таймаут для логгирования температуры
extern uint32_t toReadTout;										// Таймаут для считывания температуры
extern uint32_t toMesgTout;										// Таймаут для предачи температуры

extern uint32_t ddReadTout;										// Таймаут для считывания температуры

#endif /* INIT_H_ */
