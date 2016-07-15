/*
 * thermo.h
 *
 *  Created on: Jul 15, 2016
 *      Author: Gennady Tanchin <g.tanchin@yandex.ru>
 */

#ifndef THERMO_H_
#define THERMO_H_

#define MESUR_ACC				10	   	// Разрядность конвертации 10 бит (0.25 гр.С)
#define MESUR_TIME			190			// Время преобразования для 10 бит ( 9бит - 95мс, 11бит - 380мс, 12бит - 760мс )

void toSetConfig( tOwToDev * owToDev);
void toWriteEeprom( tOwToDev * owToDev);
void toReadTemperature( void );


#endif /* THERMO_H_ */
