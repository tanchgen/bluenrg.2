/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef _SAMPLE_SERVICE_H_
#define _SAMPLE_SERVICE_H_

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "cube_hal.h"
#include "bluenrg_gatt_server.h"
#include "bluenrg_gap.h"
#include "string.h"
#include "bluenrg_gap_aci.h"
#include "bluenrg_gatt_aci.h"
#include "hci_const.h"
#include "gp_timer.h"
#include "bluenrg_hal_aci.h"
#include "bluenrg_aci_const.h"
#include "hci.h"
#include "sm.h"
#include "debug.h"
#include "my_service.h"

/** 
* @brief Handle of TX Characteristic on the Server. The handle should be
*        discovered, but it is fixed only for this demo.
*/ 
#define NON_ACTIVE_TOUT 240

// Маска ошибок, отправляемых на сервер
#define ALARM_GENERIC					0x01
#define ALARM_DD_NEW_STATE		0x02
#define ALARM_TO_MAX					0x04
#define ALARM_TO_MIN					0x08
#define ALARM_DD_FAULT				0x10
#define ALARM_TO_FAULT				0x20
#define ALARM_LOG_FAULT				0x40
#define ALARM_HW_FAULT				0x80

struct _blue {
	uint8_t connectable;					// Флаг - Разрешено входящее подключение
	uint8_t connected;						// Флаг - Есть соединение
	uint16_t connHandle;
	uint32_t disconnCount;
	uint8_t authorized;
	uint8_t alrmId;								// Поддерживаемые Идентификаторов Тревог
	uint8_t alrmNewId;						// Флаги - Новых Тревог
	uint8_t alrmNewCount;					// Счетчики Новых Тревог (индекс соответствует alrmId )
	uint8_t alrmNoReadId;					// Флаги - Непрочитанных Тревог
	uint8_t alrmNoReadCount;			// Счетчики Непрочитанных Тревог (индекс соответствует alrmId)
	uint8_t alrmSendId;						// Флаги - Новых Тревог
	struct {
	// Состояние логов датчиков температуры, касательно bluetooth
		uint8_t toReq : 1;			// Есть запрос на отправку логов
		uint8_t toTxe	:	1;			// Очередной лог прочитан - можно отправлять следующие
		uint8_t toRxne:	1;			// Есть неотправленные записи логов - есть что отправлять
	// Состояние логов датчиков дверей, касательно bluetooth
		uint8_t ddReq : 1;			// Есть запрос на отправку логов
		uint8_t ddTxe	:	1;			// Очередной лог прочитан - можно отправлять следующие
		uint8_t ddRxne:	1;			// Есть неотправленные записи логов - есть что отправлять
	} logStatus;
	tBleStatus bleStatus;
};
/** 
* @brief Handle of RX Characteristic on the Client. The handle should be
*        discovered, but it is fixed only for this demo.
*/ 
//#define RX_HANDLE   0x0014

extern struct _blue blue;
/** @addtogroup SAMPLE_SERVICE_Exported_Functions
 *  @{
 */

void Make_Connection(void);
void receiveData(uint8_t* data_buffer, uint8_t Nb_bytes);
tBleStatus BlueNRG_Init( void );
tBleStatus sendData(uint8_t* data_buffer, uint8_t Nb_bytes);
void startReadTXCharHandle(void);
void startReadRXCharHandle(void);
void enableNotification(void);

int8_t alrmUpdate( uint8_t alrmId );

tBleStatus alrmCharUpdate( void );
tBleStatus logCharUpdate( uint8_t *data, uint8_t len);
tBleStatus toCurCharUpdate( void );
tBleStatus ddCurCharUpdate( void );
tBleStatus rtcCharUpdate(  uint32_t xTime  );
tBleStatus minMaxCharUpdate( void );

void Attribute_Modified_CB(uint16_t handle, uint8_t data_length,
                           uint8_t *att_data);
void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle);
void GAP_DisconnectionComplete_CB(void);
void GATT_Notification_CB(uint16_t attr_handle, uint8_t attr_len,
                          uint8_t *attr_value);
void HCI_Event_CB(void *pckt);
 
/* Остановка 2-х минутного таймера (2 минуты - время ожидания хаш-кода от клиента*/
void stopBreakTimer(void); 
/* Запуск 2-х минутного таймера (2 минуты - время ожидания хаш-кода от клиента*/
void startBreakTimer(void); 
/* Запуск 1секундного таймера (1 секунда - длительность вклучения реле) */
/* Закрытие соединения с клиентом */
void closeConnection(void);

#ifdef __cplusplus
}
#endif

#endif /* _SAMPLE_SERVICE_H_ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

