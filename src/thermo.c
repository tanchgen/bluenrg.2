/*
 *
 * thermo.c
 *  Created on: Jul 15, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#include <string.h>
#include "onewire.h"
#include "thermo.h"
#include "my_time.h"

uint32_t toTickStart;

void toSetConfig( tOwDev * toDev) {
	tOwSendBuf sendBuf;
// Формируем массив с командой и адресом
		sendBuf.cmd = MEM_WRITE;
		sendBuf.u_8[0] = TEMPER_MAX >> 4;
		sendBuf.u_8[1] = TEMPER_MIN >> 4;
		sendBuf.u_8[2] = (0x1F | ((toDev->mesurAcc-9) << 5)) ;
		OW_Send(OW_NO_RESET, (uint8_t *)&sendBuf, 4, NULL, 0, OW_NO_READ);
}

void toWriteEeprom( uint64_t addr ) {
	tOwSendBuf sendBuf;
	sendBuf.cmd = MATCH_ROM;
	sendBuf.adr = addr;
	OW_Send(OW_SEND_RESET, (uint8_t *)&sendBuf, 9, NULL, 0, OW_NO_READ);

// Формируем массив с командой и адресом


	sendBuf.cmd = RAM_TO_EEPROM;
	OW_Send(OW_NO_RESET, (uint8_t *)&sendBuf, 1, NULL, 0, OW_NO_READ);

	myDelay(11);

}

void toTempStart(void){
  struct{
    uint8_t cmd;
    union{
      uint8_t u8[8];
      uint64_t addr;
    } _a;
    uint16_t empty;
  } __packed sendBuf;
//  uint8_t readBuf[11];

  // Заполняем пустое пространство
  sendBuf.empty = 0xFFFF;
  sendBuf.cmd = SKIP_ROM;
  sendBuf._a.u8[0] = TERM_CONVERT;
  OW_Send(OW_SEND_RESET, (uint8_t *)&sendBuf, 2, NULL, 0, OW_NO_READ);
  toTickStart = myTick;
}

void toReadTemperature( void ) {

	struct{
	  uint8_t cmd;
	  union{
	    uint8_t u8[8];
	    uint64_t addr;
	  } _a;
    uint16_t empty;
	} __packed sendBuf;
	uint8_t readBuf[11];
	uint32_t dtime;

	// Заполняем пустое пространство
	sendBuf.empty = 0xFFFF;

	// Задержка до окнчания измерения
	if((dtime = myTick-toTickStart) < 760 ){
	  mesureDelay( dtime );
	}

	// Считываем показания датчиков
	for ( uint8_t i = 0; i < owDevNum; i++ ){
	  if( owDev[i].devType != DEV_TO ){
	    continue;
	  }
	// По умолчанию - несуществующая температура 127.9375 гр.С
		owDev[i].data.temper = 0x7FF;
	// Формируем массив с командой и адресом
		if (owDev[i].addr){
			sendBuf.cmd =MATCH_ROM;
			sendBuf._a.addr = owDev[i].addr;
			// Отправляем в шину
			if ((OW_Send(OW_SEND_RESET, (uint8_t *)&sendBuf, 9, NULL, 0, OW_NO_READ)) == OW_ERR ) {
				if( owDev[i].devStatus == OW_DEV_OK ){
					// До этого был нормальный
					owDev[i].addr = 0;
					owDev[i].devStatus = OW_TO_DEV_ERR;
					owDev[i].newErr = TRUE;
				}
			}
			else {
				sendBuf._a.addr = 0xFFFFFFFFFFFFFFFF;
				sendBuf.cmd = MEM_READ;
				OW_Send(OW_NO_RESET, (uint8_t *)&sendBuf, 6, readBuf, 5, 1);
				if( (readBuf[4] & 0x80) == 0 ) {
					// Регистр конфигурации "правильный" - значит датчик откликнулся
					uint16_t mask;
					switch(owDev[i].mesurAcc){
					case 9:
						mask = ~(0x0007);
						break;
					case 10:
						mask = ~(0x0003);
						break;
					case 11:
						mask = ~(0x0001);
						break;
					default:
						mask = 0xFFFF;
						break;
					}
					owDev[i].data.temper = ((uint16_t)readBuf[0] | ((uint16_t)readBuf[1] << 8)) & mask;
				}
			}
		}
	}
	toTempStart();
}

void mesureDelay( uint32_t dt ){
  uint16_t dt2;
  uint8_t maxMesure = 0;

	for(uint8_t i = 0; i < owDevNum; i++){
	  if( owDev[i].devType == DEV_TO ){
	    if (owDev[i].mesurAcc > maxMesure) {
	      maxMesure = owDev[i].mesurAcc;
	    }
		}
	}

	dt2 = MESUR_TIME_9 << (maxMesure - 9);
	if( dt2 > dt ){
	  // Задержка для данной точности датчиков ( 95, 190, 380 или 760 мс )
	  myDelay( dt2 - dt );
	}
}
