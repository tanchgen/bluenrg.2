/*
 * thermo.h
 *
 *  Created on: Jul 15, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#ifndef THERMO_H_
#define THERMO_H_

#define TEMPER_MAX				0x7D0				// Максимальная температура
#define TEMPER_MIN				0xC90				// Минимальная температура

#define MESUR_ACC					10	   	// Кол-во разрядов преобразования (10 бит - 0.25C, 12бит - 0.0625)
#define MESUR_TIME_9			95			// Время преобразования для 9 бит ( 95мс )
#define MESUR_TIME_10			190			// Время преобразования для 10 бит ( 190мс )
#define MESUR_TIME_11			380			// Время преобразования для 11 бит ( 380мс )
#define MESUR_TIME_12			760			// Время преобразования для 12 бит ( 760мс )

void toSetConfig( tOwDev * toDev);
void toWriteEeprom( uint64_t addr );
void toTempStart(void);
void toReadTemperature( void );
void mesureDelay( uint32_t dt );

#endif /* THERMO_H_ */
