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
//#define TX_HANDLE 0x0011
#define ALARM_GENERAL				0x00
#define ALARM_DD_NEW_STATE	0x01
#define ALARM_TO_MAX				0x02
#define ALARM_TOM_MIN				0x04
#define ALARM_DD_FAULT			0x08
#define ALARM_TO_FAULT			0x10


struct _blue {
	uint8_t connectable;					// Флаг - Разрешено входящее подключение
	uint8_t connected;						// Флаг - Есть соединение
	uint16_t connHandle;
	uint32_t disconnCount;
	uint8_t authorized;
	uint8_t alrmId;								// Битовая маска Идентификаторов Тревог
	uint8_t alrmNew[8];						// Флаги - Новых Тревог (индекс соответствует номеру поля в alrmId )
	uint8_t alrmNewCount[8];			// Счетчики Новых Тревог (индекс соответствует номеру поля в alrmId )
	uint8_t alrmNoReadCount[8];		// Счетчики Непрочитанных Тревог (индекс соответствует номеру поля в alrmId)
	uint8_t toLogReq;							// Флаг - Запрос очередной записи Лога температуры
	uint8_t ddLogReq;							// Флаг - Запрос очередной записи Лога температуры
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
tBleStatus Add_Sample_Service(void);
void Make_Connection(void);
void receiveData(uint8_t* data_buffer, uint8_t Nb_bytes);
tBleStatus BlueNRG_Init( void );
tBleStatus sendData(uint8_t* data_buffer, uint8_t Nb_bytes);
void startReadTXCharHandle(void);
void startReadRXCharHandle(void);
void enableNotification(void);
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

