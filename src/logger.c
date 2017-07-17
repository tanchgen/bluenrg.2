/*
 *
 * logger.c
 *  Created on: Jul 6, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */
#include <string.h>
#include "stm32_bluenrg_ble.h"
#include "stm32f0xx_hal_def.h"
#include "eeprom.h"
#include "my_time.h"
#include "my_service.h"
#include "my_main.h"
#include "logger.h"

tLogBuf toLogBuff;
tLogBuf ddLogBuff;
uint8_t toEmptyFill;
uint8_t ddEmptyFill;
uint8_t toCount;

int8_t saveStateBuff( tLogBuf *buf ){
	if( sendEeprom( (buf->bufAddr - sizeof(tLogBuf)), (uint8_t *)buf, sizeof(tLogBuf) ) != HAL_OK ) {
		return -1;
	}

	return 0;
}

// Получение длины следующей записи
int8_t recordLen( uint32_t addr ){
  int8_t sz;

  if ( receiveEeprom( (addr + 4), (uint8_t*)&sz, 1 ) != HAL_OK){
    Error_Handler( LOG_ERR );
    return -1;
  }
  sz *= 2;
  sz += 4;
  return sz;
}


int8_t ddLogWriteEE( tLogBuf * buf, uint8_t * data ) {
  if ( (buf->end == buf->begin) && ( buf->full == 1) ){
    buf->begin ++;
  }
  if (buf->begin == buf->len ) {
    buf->begin = 0;
  }
//  *(buf->bufAddr + buf->end) = data;
  if( sendEeprom( (buf->bufAddr + buf->end * buf->size), data, buf->size )){
    return -1;
  }

  buf->end++;
  if (buf->end == buf->len ) {
    buf->end = 0;
  }
  if (buf->end == buf->begin) {
    buf->full = 1;
  }
  // Сохраняем состояние данных буфера логгера
  if( saveStateBuff( buf ) ) {
    return -1;
  }

  return 1;
}

int16_t ddLogReadStart( tLogBuf * buf, uint8_t * data ) {
  if ( (buf->begin == buf->end) && !buf->full) {
    return 0;
  }
  buf->full = 0;

//  ch =*(buf->bufAddr + buf->begin);
  if ( receiveEeprom( (buf->bufAddr + buf->begin * buf->size), data, buf->size ) != HAL_OK){
    Error_Handler( LOG_ERR );
    return -1;
  }
  buf->b1 = 1;
  return 1;
}

int16_t ddLogReadEnd( tLogBuf * buf ){
  if( buf->b1){
    buf->begin++;
    if (buf->begin == buf->len) {
      buf->begin = 0;
    }
    buf->b1 = 0;
    if( saveStateBuff( buf ) ){
      return -1;
    }
  }
  return 0;
}

int8_t toLogWriteEE( tLogBuf * buf, uint8_t * data  ){
  uint8_t sz = *(data+4)*2+4;
  uint8_t sz1 = 0;
  uint32_t e0 = buf->end;
  uint32_t b0 = buf->begin;

  uint32_t e1 = e0 + sz;
  uint8_t ef = 0;
  uint8_t bf = 0;

  if( e1 >= buf->len ){
    ef = 1;
    e1 -= buf->len;
    sz1 = e1;
    sz -= sz1;
  }

  if( ef ){
    if( e0 < b0 ) {
      buf->full = 1;
      while( (bf == 0) || ((bf == 1) && (b0 < e1)) ){
        int8_t rc;
        if( (rc = recordLen( buf->bufAddr+b0)) < 0 ){
          return -1;
        }
        b0 = rc;
        b0 += buf->begin;
        if( b0 >= buf->len ){
          bf = 1;
          b0 -= buf->len;
        }
      }
    }
    else if( e1 > b0 ) {
      buf->full = 1;
      while( (b0 < e1) && (bf == 0) ){
        int8_t rc;
        if( (rc = recordLen( buf->bufAddr+b0)) < 0 ){
          return -1;
        }
        b0 = rc;
        b0 += buf->begin;
        if( b0 >= buf->len ){
          bf = 1;
          b0 -= buf->len;
        }
      }
    }
  }
  else if( (e0 < b0) && (e1 > b0) ){
    buf->full = 1;
    while( (b0 < e1) && (bf == 0) ){
      if( b0 >= buf->len ){
        bf = 1;
        b0 -= buf->len;
      }
    }
  }

  if( sendEeprom( (buf->bufAddr + buf->end), data, sz )){
    return -1;
  }

  if( sendEeprom( buf->bufAddr, data+sz, sz1 )){
    return -1;
  }

  buf->end = e1;
  buf->begin = b0;

  // Сохраняем состояние данных буфера логгера
  if( saveStateBuff( buf ) ) {
    return -1;
  }

  return 1;
}


int16_t toLogReadEEStart( tLogBuf * buf, uint8_t *data ){

  int8_t rc;
  uint8_t sz;
  uint8_t sz1 = 0;
  uint32_t b0 = buf->begin;
  uint32_t b1;

  if ( (buf->begin == buf->end) && !buf->full) {
    return 0;
  }

  if( (rc = recordLen(buf->bufAddr + b0)) < 0){
    return -1;
  }
  sz = rc;

  if( (b1 = b0+sz) >= buf->len ){
    b1 -= buf->len;
    sz1 = b1;
    sz -= sz1;
  }

  if ( receiveEeprom( (buf->bufAddr + b0), data, sz ) != HAL_OK){
    Error_Handler( LOG_ERR );
    return -1;
  }
  if( sz1 ){
    if ( receiveEeprom( buf->bufAddr, data+sz, sz1 ) != HAL_OK){
      Error_Handler( LOG_ERR );
      return -1;
    }
  }

  buf->b1 = b1+1;

  return 1;
}

int16_t toLogReadEEEnd( tLogBuf * buf ){
  if( buf->b1 ){
    buf->begin = buf->b1-1;
    buf->b1 = 0;
    buf->full = 0;
    if( saveStateBuff( buf ) ){
      return -1;
    }
  }
  return 0;
}


int8_t toLogWrite( void ) {
#if (TO_DEV_NUM > MAX_TO_DEV_NUM)
#error "Данная функция написана для количества датчиков не превышающих 4"
#endif

	tToLogUnit toLogUnit;

	uint8_t i;
	uint8_t j;

	// Записываем штамп времени
	toLogUnit.xTime = uxTime;
	toLogUnit.toNumber = toDevNum;
	// Записываем показания датчиков
	for( i = 0, j = 0; i < owDevNum; i++ ){
		// Сохраняем значения температуры в следующем виде:
	  if( owDev[i].devType == DEV_TO ){
	    toLogUnit.toData[j] = ((j+1) << 12) | ( owDev[i].toTemp & 0xFFF );
	    j++;
	  }
	}
	return toLogWriteEE( &toLogBuff, (uint8_t *)&toLogUnit );
}

int8_t toLogRead( tToLogUnit * toLog ){
#if (TO_DEV_NUM > 4)
#error "Данная функция написана для количества датчиков не превышающих 4"
#endif

  int8_t rd;


	if( (rd = toLogReadEEStart( &toLogBuff, (uint8_t *)toLog )) == -1 )  {
		// Ошибка чтения логов - на сервер будем отправлять значения, заполненные 0xFF
		memset( toLog, 0xFF, sizeof(tToLogUnit) );
		toLog->toNumber = 1;
	}

	return rd;
}

#if 0
int8_t ddLogWrite( void ) {
	tDdLogUnit ddLogUnit;
	uint8_t i, j;

	// Записываем штамп времени
	ddLogUnit.xTime = uxTime;
	// Записываем показания датчиков
	ddLogUnit.ddData = 0;
	for( j=0, i = 0; i < owDevNum; i++ ){
	  if( owDev[i].devType == DEV_DD ){
	    ddLogUnit.ddData |= owDev[i].ddState << j++;
	  }
	}
	if( j ) {
		return ddLogWriteEE( &ddLogBuff, (uint8_t *)&ddLogUnit );
	}
	return 0;
}
#endif

int8_t ddLogRead( uint8_t * ddLog ) {
	int8_t rd;

	switch( rd = ddLogReadStart( &ddLogBuff, ddLog ) ){
		case -1:
			// Ошибка чтения логов - на сервер будем отправлять значения, заполненные 0xFF
			memset( ddLog, 0xFF, sizeof(tDdLogUnit) );
			break;
		case 0:
			// В логе нет непрочитанных записей - на сервер будем отправлять значения, заполненные 0x00
			memset( ddLog, 0x00, sizeof(tDdLogUnit) );
			break;
	}

	return rd;
}

int8_t alrmUpdate( uint8_t alrmId ) {
	uint8_t mask;

	if ( (mask = alrmId & blue.alrmId) ) {
		blue.alrmNewId |= mask;
		blue.alrmNoReadId |= mask;
		blue.alrmNewCount++;
		blue.alrmNoReadCount++;

		if ( blue.connected ) {
			alrmCharUpdate();
		}
		return 0;
	}
	return -1;
}

uint8_t logBufFill( tToLogBuf * buf, tToLogUnit * log, uint8_t *toNum ){
  uint8_t fin = 0;

  uint8_t count;
  buf->xTime = log->xTime;
  count = log->toNumber - *toNum;
  if(count > 8){
    // Максимальное количество температур в характеристике = 8 (4Б + 8*2Б = 20Б)
    count = 8;
  }
  else {
    fin = 1;
  }
  for(uint8_t i = 0; i < count; i++){
    buf->toData[i] = log->toData[*toNum + i];
  }
  if(fin){
    *toNum = 0;
  }
  return count;
}

int8_t toLogSend( void ){
  tBleStatus bleRc;
	int8_t ret = 0;
	static tToLogUnit logBuf;
	tToLogBuf tmpbuf;
	uint8_t toLogFill = 0;  //Количество записанных значений температур в буфере лога

// Формируем подстроку Лога Температуры
	// Можно отправлять запись лога в Характеристику
	if(blue.logStatus.toTxe){
	  if( toCount == 0 ){
	    if( (ret = toLogRead( &logBuf )) ){
	      toLogFill = logBufFill( &tmpbuf, &logBuf, &toCount );
	      toEmptyFill = FALSE;
	      blue.logStatus.toTxe = FALSE;
	    }
	    else if ( toEmptyFill == FALSE ) {
	      // Заполняем нулями
	      if(logBuf.toNumber > 8){
	        toLogFill = 8;
	      }
	      else {
          toLogFill = logBuf.toNumber;
	      }
	      memset( (uint8_t *)&tmpbuf, 0, sizeof(tToLogBuf));
	      toEmptyFill = TRUE;
	      blue.logStatus.toTxe = TRUE;
	    }
	  }
	  else {
      toLogFill = 8;
      toEmptyFill = FALSE;
      blue.logStatus.toTxe = FALSE;
	  }
	}
	if ( toLogFill ) {
	  bleRc = toLogCharUpdate( (uint8_t *)&tmpbuf, 4 + toLogFill*2 );
		toLogFill = FALSE;
		if( bleRc != BLE_STATUS_SUCCESS ) {
			// Ошибка обновления Характеристики
			bnrgFullRst();
			toEmptyFill = TRUE;
			blue.logStatus.toTxe = TRUE;
		}
	}

	return ret;
}

int8_t ddLogSend( void ){
  int8_t ret = 0;
  uint8_t tmpBuf[6];
  uint8_t logFill = FALSE;

// Теперь формирует подстроку Лога Датчиков Дверей
    // Можно отправлять запись лога в Характеристику
  if(blue.logStatus.ddTxe){
    if( (ret = ddLogRead( tmpBuf )) ){
      logFill = TRUE;
      ddEmptyFill = FALSE;
      blue.logStatus.ddTxe = FALSE;
    }
    else if ( ddEmptyFill == FALSE ) {
      logFill = TRUE;
      ddEmptyFill = TRUE;
      blue.logStatus.ddTxe = TRUE;
      memset( tmpBuf, 0, 6 );
    }
  }
  if ( logFill ) {
    if( ddLogCharUpdate( tmpBuf, DD_LOG_LEN) != BLE_STATUS_SUCCESS ) {
      // Ошибка обновления Характеристики
      bnrgFullRst();
      toEmptyFill = TRUE;
      blue.logStatus.toTxe = TRUE;
      ddEmptyFill = TRUE;
      blue.logStatus.ddTxe = TRUE;
    }
  }

  return ret;
}
