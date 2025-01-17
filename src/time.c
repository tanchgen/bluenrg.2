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
#include "init.h"
#include "logger.h"
#include "thermo.h"
#include "my_service.h"
#include "stm32_bluenrg_ble.h"

#define HSE_CORRECT					0.99236217192
#define LSI_CORRECT					1.0

volatile tXtime uxTime;
uint8_t secondFlag = RESET;

extern volatile uint32_t resetCount; // Таймаут отсутствия активности на bluetoth.

// *********** Инициализация структуры ВРЕМЯ (сейчас - системное ) ************
void timeInit( void ) {
	uint32_t tempReg;
	RTC_InitTypeDef rtcInitStruct;
  RTC_DateTypeDef  sdatestructure;
  RTC_TimeTypeDef  stimestructure;
  uint32_t source;

	resetCount = NON_ACTIVE_TOUT*12;

// **************** RTC Clock configuration ***********************
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  PWR->CR |= PWR_CR_DBP;

  /* Wait for Backup domain Write protection disable */

  for( myTick = 0; !( PWR->CR & PWR_CR_DBP); myTick++) {
  	if ( myTick == HSE_STARTUP_TIMEOUT) {
  		Error_Handler( HW_ERR );
  	}
  }

	if( (RCC->CR & RCC_CR_HSERDY) == RCC_CR_HSERDY ){
		source = RCC_BDCR_RTCSEL_HSE;
		// Для HSE/32 (0x7F - PREDIV_A)
  	rtcInitStruct.RTC_SynchPrediv = (uint32_t)(HSE_VALUE/32/0x7F * HSE_CORRECT);
	}
	else {
  	RCC->CSR |= ((uint32_t)RCC_CSR_LSION);
  	for( myTick = 0; !(RCC->CSR & RCC_CSR_LSIRDY); myTick++) {
  		if ( myTick == HSE_STARTUP_TIMEOUT) {
  			Error_Handler( HW_ERR );
  		}
  	}
		// Для LSI = 40MHz
  	source = RCC_BDCR_RTCSEL_LSI;
  	rtcInitStruct.RTC_SynchPrediv = (uint32_t)((LSI_VALUE+0x3F)/0x7F  * LSI_CORRECT);
  }

  /* Store the content of BDCR register before the reset of Backup Domain */
  tempReg = (RCC->BDCR & ~(RCC_BDCR_RTCSEL));
  /* RTC Clock selection can be changed only if the Backup Domain is reset */
  RCC->BDCR |= RCC_BDCR_BDRST;
  RCC->BDCR &= ~RCC_BDCR_BDRST;
  /* Restore the Content of BDCR register */
  RCC->BDCR = tempReg | source;

  RCC->BDCR |= RCC_BDCR_RTCEN;

// ******************** RTC System configuration *************************

  rtcInitStruct.RTC_HourFormat = RTC_HourFormat_24;
  rtcInitStruct.RTC_AsynchPrediv = (uint32_t)0x7F;

  RTC_Init( &rtcInitStruct );

  /*##-1- Configure the Date #################################################*/
  /* Set Date: Wednesday June 1st 2016 */
  sdatestructure.RTC_Year = 16;
  sdatestructure.RTC_Month = RTC_Month_June;
  sdatestructure.RTC_Date = 1;
  sdatestructure.RTC_WeekDay = RTC_Weekday_Wednesday;

  if(RTC_SetDate( RTC_Format_BIN ,&sdatestructure ) != SUCCESS)
  {
    /* Initialization Error */
    Error_Handler( HW_ERR );
  }

  stimestructure.RTC_Hours = 0;
  stimestructure.RTC_Minutes = 0;
  stimestructure.RTC_Seconds = 0;
  stimestructure.RTC_H12 = 0;

  if(RTC_SetTime( rtcInitStruct.RTC_HourFormat ,&stimestructure ) != SUCCESS)
  {
    /* Initialization Error */
    Error_Handler( HW_ERR );
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

tXtime xTm2Utime( tDate *mdate, tTime *mtime ){
	/* convert time structure to scalar time */
int32_t		days;
int32_t		secs;
int32_t		mon, year;

	/* Calculate number of days. */
	mon = mdate->RTC_Month - 1;
	// Годы считаем от 1900г.
	year = (mdate->RTC_Year + 100) - _TBIAS_YEAR;
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

void xUtime2Tm( tDate * mdate, tTime *mtime, tXtime secsarg){
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

	mdate->RTC_WeekDay = (days + 1) % 7;

	/* determine year */
	for (year = days / 365; days < (i = Daysto32(year, 0) + 365*year); ) { --year; }
	days -= i;
	// Годы выставляем от эпохи 2000г., а не 1900г., как в UNIX Time
	mdate->RTC_Year = (year - 100) + _TBIAS_YEAR;

		/* determine month */
	pm = MONTAB(year);
	for (mon = 12; days < pm[--mon]; );
	mdate->RTC_Month = mon + 1;
	mdate->RTC_Date = days - pm[mon] + 1;
}

void setRtcTime( tXtime xtime ){
	tTime mtime;
	tDate mdate;

	xUtime2Tm( &mdate, &mtime, xtime);
	RTC_SetTime( RTC_Format_BIN, &mtime );
	RTC_SetDate( RTC_Format_BIN, &mdate );
}

tXtime getRtcTime( void ){
	tTime mtime;
	tDate mdate;

	RTC_GetTime( RTC_Format_BIN, &mtime );
	RTC_GetDate( RTC_Format_BIN, &mdate );

	return xTm2Utime( &mdate, &mtime );
}

void timersHandler( void ) {

	// Таймаут для логгирования температуры
	if ( toLogCount > 1) {
		toLogCount--;
	}
	// Таймаут для считывания температуры
	if ( toReadCount > 1) {
		toReadCount--;
	}

	// Таймаут для считывания датчиков двери
	if ( ddReadCount > 1) {
		ddReadCount--;
	}
	if ( !(myTick % 1000) ){
		secondFlag = SET;
	}
}

void timersProcess( void ) {

	// Таймаут для логгирования температуры
	if ( toLogCount == 1 ) {
		toLogCount += toLogTout;
		// Проверяем флаги ошибки TO
		for ( uint8_t i = 0; i < TO_DEV_NUM; i++ ) {
			if( owToDev[i].newErr ){
				alrmUpdate( ALARM_TO_FAULT );
				break;
			}
		}
#if OW_DD
		// Проверяем флаги ошибки DD
		for ( uint8_t i = 0; i < OW_DD_DEV_NUM; i++ ) {
			if( owDdDev[i].newErr ){
				alrmUpdate( ALARM_DD_FAULT );
				break;
			}
		}
#endif
		toLogWrite();
		while( toCurCharUpdate() != BLE_STATUS_SUCCESS ){
			bnrgFullRst();
			if( blue.bleStatus == BLE_STATUS_TIMEOUT ){
				break;
			}
		}
		while( minMaxCharUpdate() != BLE_STATUS_SUCCESS ){
			bnrgFullRst();
			if( blue.bleStatus == BLE_STATUS_TIMEOUT ){
				break;
			}
		}
	}
	// Таймаут для считывания датчиков двери
	if ( ddReadCount == 1) {
		ddReadCount += ddReadTout;
		ddReadDoor();
		while( ddCurCharUpdate() != BLE_STATUS_SUCCESS ){
			bnrgFullRst();
			if( blue.bleStatus == BLE_STATUS_TIMEOUT ){
				break;
			}
		}
	}
	// Таймаут для считывания температуры
	if ( toReadCount == 1 ) {
		toReadCount += TO_READ_TOUT;
		toReadTemperature();

	  while ( toCurCharUpdate() != BLE_STATUS_SUCCESS ){
			bnrgFullRst();
			if( blue.bleStatus == BLE_STATUS_TIMEOUT ){
				break;
			}
		}
	}
	if (secondFlag) {
		secondFlag = RESET;
		uxTime = getRtcTime();
		while( rtcCharUpdate( uxTime ) != BLE_STATUS_SUCCESS ){
			bnrgFullRst();
			if( blue.bleStatus == BLE_STATUS_TIMEOUT ){
				break;
			}
		}

		if( resetCount ) {
			resetCount--;
		}
		else {
			bnrgFullRst();
		}
	}
}

// Задержка в мс
void myDelay( uint32_t del ){
	uint32_t finish = myTick + del;
	while ( myTick < finish)
	{}
}
