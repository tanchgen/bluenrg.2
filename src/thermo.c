/*
 *
 * thermo.c
 *  Created on: Jul 15, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#include <stddef.h>
#include "onewire.h"
#include "thermo.h"
#include "my_time.h"


void toSetConfig( tOwToDev * owToDev) {
	uint8_t sendBuf[4];
// Формируем массив с командой и адресом
		sendBuf[0] = MEM_WRITE;
		sendBuf[1] = TERM_MAX;
		sendBuf[2] = TERM_MIN;
		sendBuf[3] = (0x1F | ((owToDev->mesurAcc-9) << 5)) ;
		OW_Send(OW_NO_RESET, sendBuf, 4, NULL, 0, OW_NO_READ);
}

void toWriteEeprom( tOwToDev * owToDev) {
	uint8_t sendBuf[9];
	sendBuf[0] = MATCH_ROM;
	*((uint64_t *)&sendBuf[1]) = owToDev->addr;
	OW_Send(OW_SEND_RESET, sendBuf, 9, NULL, 0, OW_NO_READ);

// Формируем массив с командой и адресом

	sendBuf[0] = RAM_TO_EEPROM;
	OW_Send(OW_NO_RESET, sendBuf, 1, NULL, 0, OW_NO_READ);
// При паразитном питании требуется дополнительная подтяжка провода шины к питанию на >10мс
// Включаем подтяжку шины 1-Wire к Vdd
	OW_PORT->BSRR |= OW_RX_PIN;										// Выставляем в 1
	OW_PORT->MODER = tmpModerOut;
	myDelay(11);
// Восстанавливаем работу USART_RX - вывода
	OW_PORT->MODER = tmpModerAf;
	OW_PORT->BRR |= OW_RX_PIN;										// Выставляем в 0
}


void toReadTemperature( void ) {
	uint8_t sendBuf[11];
	uint8_t readBuf[2];

	sendBuf[0] = SCIP_ROM;
	sendBuf[1] = TERM_CONVERT;
	OW_Send(OW_SEND_RESET, (uint8_t *)sendBuf, 2, NULL, 0, OW_NO_READ);

	// При паразитном питании требуется дополнительная подтяжка провода шины к питанию на >10мс
	// Включаем подтяжку шины 1-Wire к Vdd
	OW_PORT->BSRR |= OW_RX_PIN;												// Выставляем в 1
	OW_PORT->MODER = tmpModerOut;											// Меняем Alternate Function на OUTPUT

	// Задержка на пересчет измерения
	myDelay( MESUR_TIME );

	// Восстанавливаем работу USART_RX - вывода
	OW_PORT->MODER = tmpModerAf;
	OW_PORT->BRR |= OW_RX_PIN;												// Выставляем в 0

	// Считываем показания датчиков
	for ( uint8_t i = 0; i < TO_DEV_NUM; i++ ){
	// Формируем массив с командой и адресом
		if (owToDev[i].addr){
			sendBuf[0] =MATCH_ROM;
			*((uint64_t *)&sendBuf[1]) = owToDev[i].addr;
			// Отправляем в шину
			if ((OW_Send(OW_SEND_RESET, sendBuf, 9, NULL, 0, OW_NO_READ)) == OW_DEV_ERR ) {
				owToDev[i].addr = 0;
				owToDev[i].devStatus = OW_DEV_ERR;
			}
			else {
				sendBuf[0] = MEM_READ;
				OW_Send(OW_NO_RESET, sendBuf, 3, readBuf, 2, 1);
				owToDev[i].temper = *((uint16_t *)readBuf);
			}
		}
	}
}
