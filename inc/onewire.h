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
#include "my_main.h"
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

// Скорость UART для работы с 1-Wire (рекомендуется 11520)
#define OW_BAUDRATE			103000;	// Вычисленно опытным путем

#define OW_TRANS_TOUT		500

#define OW_PORT					GPIOA
#define OW_TX_PIN				GPIO_Pin_2
#define OW_TX_PIN_NUM		2
#define OW_RX_PIN				GPIO_Pin_3
#define OW_RX_PIN_NUM		3

#endif //OW_USART1

#define MAX_TO_DEV_NUM	16

//#define TO_DEV_NUM			3				// Количество термометров
#if (MAX_TO_DEV_NUM < TO_DEV_NUM)
#error "Число датчиков температуры превышает максимальное для данной EEPROM (128кБ) для Логов"
#endif

#define DS1820_SERIAL		0x28

// ******************** Определения для Датчиков двери ********************

#define MAX_DD_DEV_NUM	16
// Датчики двери - на 1-Wire
//#define OW_DD						1
//#define DD_DEV_NUM			2				// Количество Датчиков Дверей (DD)


#define OW_DD_DEV_NUM	  DD_DEV_NUM				// Количество 1-wire контроллеров Датчиков Дверей (DD)
#define DS2413_SERIAL		0x3A

// Маски для датчиков дверей на одном 1-wire устройстве
#define DD_1				    0x1
#define DD_2            0x4

#define OW_DEV_NUM			16

typedef enum {
  DEV_NULL,
  DEV_DD,
  DEV_TO
} eDevType;

typedef struct {
	uint64_t addr;					//  Адрес устройства
  eErrStatus devStatus;
  uint8_t newErr;         //  Признак новой ошибки
  union{
    int16_t temper;         // Действующее значение температуры
    struct{
      uint8_t state;          // Действующее значение датчика двери
      uint8_t statePrev;
    } dd;
  } data;
#define toTemp  data.temper
#define ddState data.dd.state
#define ddStatePrev data.dd.statePrev
	uint8_t  mesurAcc;			//  Точность измерения ( 9-10-11-12 бит )
	int16_t tMin;						//  Допустимый максимум температуры
	int16_t tMax;						//  Допустимый минимум температуры
	eDevType devType;        //  Тип 1-Wire устройства
} __attribute__((packed)) tOwDev;

typedef struct {
  uint8_t cmd;
  union{
    uint64_t address;
    uint8_t ui8[8];
  } data;
  uint8_t empty[2];
#define adr    data.address
#define u_8     data.ui8
} __attribute__((packed)) tOwSendBuf;

// Запись одного датчика температуры в характеристиках
typedef struct {
  uint8_t toNumber;
  int16_t temp;
} __attribute__((packed)) tToRec;

// Запись одного датчика температуры в характеристиках
typedef struct {
  uint8_t owDevNumber;
  uint8_t owDevType;
  uint8_t err;
  uint64_t owId;
} __attribute__((packed)) tOwDevRec;

#define DD_READ_TOUT				500					// Таймаут считывания датчиков двери

// Команды 1-Wire
#define SEARCH_ROM			0xF0
#define READ_ROM				0x33
#define MATCH_ROM				0x55
#define SKIP_ROM				0xCC
#define ALARM_SEACH			0xEC
#define TERM_CONVERT		0x44
#define MEM_WRITE				0x4E
#define MEM_READ				0xBE
#define RAM_TO_EEPROM		0x48
#define EPPROM_TO_RAM		0xB8
#define PIO_READ				0xF5
#define PIO_WRITE				0x5A


// первый параметр функции OW_Send
#define OW_SEND_RESET		1
#define OW_NO_RESET			2

#define OW_NO_READ			0xff

#define OW_READ_SLOT		0xff

extern uint8_t toDevNum;
extern uint8_t ddDevNum;

extern int8_t owDevNum;

extern eErrStatus owStatus;
extern tOwDev owDev[]; 			// Массив структур устройств 1-Wire;

uint8_t OW_Init();
uint8_t OW_Send(uint8_t sendReset, uint8_t *command, uint8_t cLen, uint8_t *data, uint8_t dLen, uint8_t readStart);
uint8_t OW_Scan(uint8_t *buf, uint8_t num);

void ddReadDoor( void );

#endif /* ONEWIRE_H_ */
