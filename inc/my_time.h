/*
 * my_time.h
 *
 *  Created on: 08 апр. 2016 г.
 *      Author: Jet <g.tanchin@yandex.ru>
 */

#ifndef MY_TIME_H_
#define MY_TIME_H_

#include <stdint.h>

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

extern tXtime uxTime;

// *********** Инициализация структуры ВРЕМЯ (сейчас - системное ) ************
void timeInit( void );
// Получение системного мремени
uint32_t getTick( void );
tXtime xtmtot( tDate *mdate, tTime *mtime );
void xttotm( tDate * mdate, tTime *mtime, tXtime secsarg);
void setRtcTime( tXtime xtime );
tXtime getRtcTime( void );
void timersHandler( void );
void myDelay( uint32_t del );

#endif /* UNIX_TIME_H_ */
