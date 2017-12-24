/*
 * my_time.h
 *
 *  Created on: 08 апр. 2016 г.
 *      Author: Jet <g.tanchin@yandex.ru>
 */

#ifndef MY_TIME_H_
#define MY_TIME_H_

#include <stdint.h>
#include "stm32f0xx.h"

#define ALARM_UPDATE_TOUT		100

typedef RTC_TimeTypeDef			tTime;
typedef RTC_DateTypeDef			tDate;

#define TIMEZONE_MSK			(+3)

	// DEF: standard signed format
	// UNDEF: non-standard unsigned format
	#define	_XT_SIGNED

#ifdef	_XT_SIGNED
	typedef	int32_t                           tXtime;
#else
	typedef	uint32                          tXtime;
#endif

extern volatile tXtime uxTime;

extern __IO uint32_t myTick;

#define TO_LOG_TOUT				60000				// Таймаут логирования датчиков температуры
#define TO_READ_TOUT			10000				// Таймаут считывания датчиков температуры
#define TO_MESG_TOUT			10000				// Таймаут передачи показаний датчиков температуры

// *********** Инициализация структуры ВРЕМЯ (сейчас - системное ) ************
void timeInit( void );
// Получение системного мремени
uint32_t getTick( void );
tXtime xtmtot( tDate *mdate, tTime *mtime );
void xttotm( tDate * mdate, tTime *mtime, tXtime secsarg);
void setRtcTime( tXtime xtime );
tXtime getRtcTime( void );
void timersProcess( void );

void timersHandler( void );
void myDelay( uint32_t del );

#endif /* UNIX_TIME_H_ */
