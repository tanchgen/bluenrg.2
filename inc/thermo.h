/*
 * thermo.h
 *
 *  Created on: Jul 15, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#ifndef THERMO_H_
#define THERMO_H_

#define TO_LOG_TOUT				60000				// Таймаут логирования датчиков температуры
#define TO_READ_TOUT			5000				// Таймаут считывания датчиков температуры
#define TO_MESG_TOUT			1000				// Таймаут передачи показаний датчиков температуры

#define TEMPER_MAX				0x7D0				// Максимальная температура
#define TEMPER_MIN				0xC90				// Минимальная температура

#define MESUR_ACC					10	   	// Кол-во разрядов преобразования (10 бит - 0.25C, 12бит - 0.0625)
#define MESUR_TIME_9			95			// Время преобразования для 9 бит ( 95мс )
#define MESUR_TIME_10			190			// Время преобразования для 10 бит ( 190мс )
#define MESUR_TIME_11			380			// Время преобразования для 11 бит ( 380мс )
#define MESUR_TIME_12			760			// Время преобразования для 12 бит ( 760мс )

void toSetConfig( tOwToDev * owToDev);
void toWriteEeprom( tOwToDev * owToDev);
void toReadTemperature( void );
void mesureDelay( void );

#endif /* THERMO_H_ */
