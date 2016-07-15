/*
 * onewire.h
 *
 *  Version 1.0.1
 */

#ifndef ONEWIRE_H_
#define ONEWIRE_H_

// для разных процессоров потребуется проверить функцию OW_Init
// на предмет расположения ножек USART
#include "stm32f0xx.h"

// ****************** Установки для UART *****************************
// выбираем, на каком USART находится 1-wire
#define OW_USART1
//#define OW_USART2
//#define OW_USART3
//#define OW_USART4

#ifdef OW_USART1

#undef OW_USART2
#undef OW_USART3
#undef OW_USART4

#define OW_USART 				USART1
#define OW_DMA_CH_RX 		DMA1_Channel3
#define OW_DMA_CH_TX 		DMA1_Channel2
#define OW_DMA_RX_FLAG	DMA1_FLAG_TC3

#define OW_TRANS_TOUT		500

#define OW_PORT					GPIOA
#define OW_TX_PIN				GPIO_Pin_2
#define OW_TX_PIN_NUM		2
#define OW_RX_PIN				GPIO_Pin_3
#define OW_RX_PIN_NUM		3

#define TO_DEV_NUM			2				// Количество термометров
#define DD_DEV_NUM			0				// Количество Датчиков Дверей (DD)
#define OW_DEV_NUM			(TO_DEV_NUM + DD_DEV_NUM)

#endif


typedef enum {
	OW_OK,
	OW_WIRE_ERR,
	OW_DEV_ERR,
	OW_DEV_NUM_ERR,
	OW_ERR
} eOwStatus;

typedef struct {
	uint64_t addr;						//  Адрес устройства
	uint8_t  mesurAcc;				//  Точность измерения ( 9-10-11-12 бит )
	uint16_t tMin;						//  Допустимый максимум температуры
	uint16_t tMax;						//  Допустимый минимум температуры
	eOwStatus devStatus;
	int16_t	temper;						// Действующее значение температуры
} tOwToDev;

typedef struct {
	uint64_t addr;						//  Адрес устройства
	uint32_t readTout;
	uint32_t ddData;					// Действующее значение датчика двери
	eOwStatus devStatus;
} tOwDdDev;

#define DD_READ_TOUT				500					// Таймаут считывания датчиков двери

#define TO_LOG_TOUT					60000				// Таймаут логирования датчиков температуры
#define TO_READ_TOUT				1000				// Таймаут считывания датчиков температуры
#define TO_MESG_TOUT				1000				// Таймаут передачи показаний датчиков температуры
#define MESURE_ACCUR				10					// Кол-во разрядов преобразования (10 бит - 0.25C, 12бит - 0.0625)

#define TERM_MAX						0x7D				// Максимальная температура
#define TERM_MIN						0xC9				// Минимальная температура

// Команды 1-Wire
#define SEARCH_ROM			0xF0
#define READ_ROM				0x33
#define MATCH_ROM				0x55
#define SCIP_ROM				0xCC
#define ALARM_SEACH			0xEC
#define TERM_CONVERT		0x44
#define MEM_WRITE				0x4E
#define MEM_READ				0xBE
#define RAM_TO_EEPROM		0x48
#define EPPROM_TO_RAM		0xB8


// первый параметр функции OW_Send
#define OW_SEND_RESET		1
#define OW_NO_RESET			2

#define OW_NO_READ			0xff

#define OW_READ_SLOT		0xff

extern uint8_t owDevNum;
extern eOwStatus owStatus;
extern tOwToDev owToDev[]; 			// Массив структур устройств 1-Wire;
extern tOwDdDev owDdDev[]; 			// Массив структур Датчиков Двери 1-Wire;

extern uint32_t tmpModerOut, tmpModerAf;			 // Значения регистра MODER для UART и для подтяжки UART_RX к Vdd

uint8_t OW_Init();
uint8_t OW_Send(uint8_t sendReset, uint8_t *command, uint8_t cLen, uint8_t *data, uint8_t dLen, uint8_t readStart);
uint8_t OW_Scan(uint8_t *buf, uint8_t num);

void ddReadDoor( void );

#endif /* ONEWIRE_H_ */
