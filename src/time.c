/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id: time.c 4 2007-08-27 13:11:03Z xtimor $
 *
 * unix_time.c
 *      Author: Jet <g.tanchin@yandex.ru>
 *  Created on: 08 апр. 2016 г.
 */

#include "stm32f0xx.h"
#include "my_time.h"
#include "stm32xx_it.h"
#include "my_main.h"

tXtime uxTime;

// *********** Инициализация структуры ВРЕМЯ (сейчас - системное ) ************
void timeInit( void ) {
	RTC_InitTypeDef rtcInitStruct;
  RTC_DateTypeDef  sdatestructure;
  RTC_TimeTypeDef  stimestructure;

  RTC_StructInit( &rtcInitStruct );
  RTC_Init( &rtcInitStruct );
  /*##-1- Configure the Date #################################################*/
  /* Set Date: Tuesday February 18th 2014 */
  sdatestructure.RTC_Year = 16;
  sdatestructure.RTC_Month = RTC_Month_June;
  sdatestructure.RTC_Date = 1;
  sdatestructure.RTC_WeekDay = RTC_Weekday_Wednesday;

  if(RTC_SetDate( RTC_Format_BIN ,&sdatestructure ) != SUCCESS)
  {
    /* Initialization Error */
    Error_Handler();
  }

  stimestructure.RTC_Hours = 0;
  stimestructure.RTC_Minutes = 0;
  stimestructure.RTC_Seconds = 0;

  if(RTC_SetTime( rtcInitStruct.RTC_HourFormat ,&stimestructure ) != SUCCESS)
  {
    /* Initialization Error */
    Error_Handler();
  }

}

// Получение системного мремени
uint32_t getTick( void ) {
	// Возвращает количество тиков
	return myTick;
}

#define _TBIAS_DAYS		((70 * (uint32_t)365) + 17)
#define _TBIAS_SECS		(_TBIAS_DAYS * (uint32_t)86400)
#define	_TBIAS_YEAR		0
#define MONTAB(year)		((((year) & 03) || ((year) == 0)) ? mos : lmos)

const int16_t	lmos[] = {0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335};
const int16_t	mos[] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

#define	Daysto32(year, mon)	(((year - 1) / 4) + MONTAB(year)[mon])

/////////////////////////////////////////////////////////////////////

tXtime xtmtot( tDate *mdate, tTime *mtime ){
	/* convert time structure to scalar time */
int32_t		days;
int32_t		secs;
int32_t		mon, year;

	/* Calculate number of days. */
	mon = mdate->RTC_Month - 1;
	year = mdate->RTC_Year - _TBIAS_YEAR;
	days  = Daysto32(year, mon) - 1;
	days += 365 * year;
	days += mdate->RTC_Date;
	days -= _TBIAS_DAYS;

	/* Calculate number of seconds. */
	secs  = 3600 * mtime->RTC_Hours;
	secs += 60 * mtime->RTC_Minutes;
	secs += mtime->RTC_Seconds;

	secs += (days * (tXtime)86400);

	return (secs);
}

/////////////////////////////////////////////////////////////////////

void xttotm( tDate * mdate, tTime *mtime, tXtime secsarg){
	uint32_t		secs;
	int32_t		days;
	int32_t		mon;
	int32_t		year;
	int32_t		i;
	const int16_t *	pm;

	#ifdef	_XT_SIGNED
	if (secsarg >= 0) {
			secs = (uint32_t)secsarg;
			days = _TBIAS_DAYS;
		} else {
			secs = (uint32_t)secsarg + _TBIAS_SECS;
			days = 0;
		}
	#else
		secs = secsarg;
		days = _TBIAS_DAYS;
	#endif

		/* days, hour, min, sec */
	days += secs / 86400;
	secs = secs % 86400;
	mtime->RTC_Hours = secs / 3600;
	secs %= 3600;
	mtime->RTC_Minutes = secs / 60;
	mtime->RTC_Seconds = secs % 60;

	mdate->RTC_WeekDay = (days + 4) % 7;

	/* determine year */
	for (year = days / 365; days < (i = Daysto32(year, 0) + 365*year); ) { --year; }
	days -= i;
	mdate->RTC_Year = year + _TBIAS_YEAR;

		/* determine month */
	pm = MONTAB(year);
	for (mon = 12; days < pm[--mon]; );
	mdate->RTC_Month = mon + 1;
	mdate->RTC_Date = days - pm[mon] + 1;
}

void setRtcTime( tXtime xtime ){
	tTime mtime;
	tDate mdate;

	xttotm( &mdate, &mtime, xtime);
	RTC_SetTime( RTC_Format_BIN, &mtime );
	RTC_SetDate( RTC_Format_BIN, &mdate );
}

tXtime getRtcTime( void ){
	tTime mtime;
	tDate mdate;

	RTC_GetTime( RTC_Format_BIN, &mtime );
	RTC_GetDate( RTC_Format_BIN, &mdate );

	return xtmtot( &mdate, &mtime );
}
