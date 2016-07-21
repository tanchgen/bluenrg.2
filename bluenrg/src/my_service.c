/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#include "connection_config.h"
#include "stm32_bluenrg_ble.h"
#include "my_main.h"
#include "my_time.h"
#include "osal.h"
#include "deviceid.h"
#include "onewire.h"
#include "logger.h"
#include "my_service.h"

struct _blue blue;

extern uint32_t Pin;
volatile uint32_t resetCount; // Таймаут аутентификации по SHA1-хэш 120 сек.

/* Private variables ---------------------------------------------------------*/
volatile uint16_t connection_handle = 0;
volatile uint8_t notification_enabled = FALSE; // Клиент готов получать данные
volatile uint8_t start_read_tx_char_handle = FALSE;
volatile uint8_t start_read_rx_char_handle = FALSE;
volatile uint8_t end_read_tx_char_handle = FALSE;
volatile uint8_t end_read_rx_char_handle = FALSE;

uint16_t tx_handle;
uint16_t rx_handle;

uint8_t tknStr[TOKEN_LEN+1];    // строка для вычесления токена
uint8_t shaHash[41];            // Длина SHA-hash - 40 байт
uint8_t tokenPart;


/*
#ifdef BLUENRG_MS
// Hardcoded values for X-NUCLEO-IDB05A1
uint16_t tx_handle = 0x000D;
uint16_t rx_handle = 0x0010;
#else
// Hardcoded values for X-NUCLEO-IDB04A1
uint16_t tx_handle = 0x0011;
uint16_t rx_handle = 0x0014;
#endif
*/

static uint16_t workServHandle;				// WORK Service Handle
static uint16_t timeCharHandle;				// Time charact handle
static uint16_t toMinMaxCharHandle;  	// Хэндл характеристики текущей температуры
static uint16_t toCurCharHandle;				// Хэндл характеристики текущей температуры
static uint16_t ddCurCharHandle;					// Хэндл характеристики текущего состояния датчикой двери

// uint16_t alrmServHandle;				// ALARM Service Handle
static uint16_t alrmIdCharHandle;			// Хэндл характеристики идентификаторов поддерживаемых тревог
static uint16_t alrmNewCharHandle;			// Хэндл характеристики новых тревог
static uint16_t alrmNoReadCharHandle;	// Хэндл характеристики непрочитанных тревог

// uint16_t logServHandle;					// LOG Service Handle
static uint16_t logCharHandle;					// Хэндл характеристики логов
static uint16_t logReqDescHandle;			// Хэндл Дескриптора запроса логов


//extern BLE_RoleTypeDef BLE_Role;
extern uint16_t myWD;
/**
 * @}
 */

/** @defgroup SAMPLE_SERVICE_Private_Macros
 * @{
 */
/* Private macros ------------------------------------------------------------*/
#define COPY_UUID_128(uuid_struct, uuid_15, uuid_14, uuid_13, uuid_12, uuid_11, uuid_10, uuid_9, uuid_8, uuid_7, uuid_6, uuid_5, uuid_4, uuid_3, uuid_2, uuid_1, uuid_0) \
  do {\
  	uuid_struct.uuid128[0] = uuid_0; uuid_struct.uuid128[1] = uuid_1; uuid_struct.uuid128[2] = uuid_2; uuid_struct.uuid128[3] = uuid_3; \
	uuid_struct.uuid128[4] = uuid_4; uuid_struct.uuid128[5] = uuid_5; uuid_struct.uuid128[6] = uuid_6; uuid_struct.uuid128[7] = uuid_7; \
	uuid_struct.uuid128[8] = uuid_8; uuid_struct.uuid128[9] = uuid_9; uuid_struct.uuid128[10] = uuid_10; uuid_struct.uuid128[11] = uuid_11; \
	uuid_struct.uuid128[12] = uuid_12; uuid_struct.uuid128[13] = uuid_13; uuid_struct.uuid128[14] = uuid_14; uuid_struct.uuid128[15] = uuid_15; \
	}while(0)

tBleStatus BlueNRG_Init( void )
{
  tBleStatus ret;
  //uint8_t SERVER_BDADDR[] = {0xfd, 0x00, 0x25, 0xec, 0x02, 0x04}; //BT address for HRM test
  uint8_t bdaddr[BDADDR_SIZE] = { (BDADDR&0xFF), ((BDADDR>>8)&0xFF), 0x00, 0xE1, 0x80, 0x02 };
  uint16_t service_handle, dev_name_char_handle, appearance_char_handle;
  
  /* Reset BlueNRG hardware */
  BlueNRG_RST();

#ifdef PAIRING_ON
  /* Установка случайных значений для вычисления ключей CSRK, LTK, IRK
     !!! Для каждого устройства - свой набор !!! */
  uint8_t DIV[2]={ 0x57, 0x30 };
  uint8_t ER[16]={ 0x4c,0x74,0x59,0x2c,0x4d,0x23,0x24,0x2f,0x64,0x6a,0x52,0x6b,0x52 };
  uint8_t IR[16]={ 0x5f,0x41,0x6b,0x39,0x75,0x44,0x65,0x29,0x3c,0x6a,0x6c,0x73,0x46 };


  /* Configure read root key DIV on BlueNRG, BlueNRG-MS device */
  ret = aci_hal_write_config_data(CONFIG_DATA_DIV_OFFSET,
                                  CONFIG_DATA_DIV_LEN,(uint8_t *) DIV);
  /* Configure read root key ER on BlueNRG, BlueNRG-MS device */
  ret = aci_hal_write_config_data(CONFIG_DATA_ER_OFFSET,
                                  CONFIG_DATA_ER_LEN,(uint8_t *) ER);
  /* Configure read root key IR on BlueNRG, BlueNRG-MS device */
  ret = aci_hal_write_config_data(CONFIG_DATA_IR_OFFSET,
                                  CONFIG_DATA_IR_LEN,(uint8_t *) IR);

  ret = aci_gap_set_io_capability(IO_CAP_DISPLAY_ONLY);
  if (ret != BLE_STATUS_SUCCESS)
    printf("Failure.\n");
#endif /* PAIRING_ON */
  //Osal_MemCpy(bdaddr, BDADDR, sizeof(BDADDR));
  ret = aci_hal_write_config_data(CONFIG_DATA_PUBADDR_OFFSET,
                                   CONFIG_DATA_PUBADDR_LEN,
                                   bdaddr);
  ret = aci_gatt_init();
  ret = aci_gap_init(GAP_PERIPHERAL_ROLE, &service_handle, &dev_name_char_handle, &appearance_char_handle);
  ret = aci_gap_set_auth_requirement(MITM_PROTECTION_REQUIRED,
                                     OOB_AUTH_DATA_ABSENT,
                                     NULL,
                                     7,
                                     16,
                                     USE_FIXED_PIN_FOR_PAIRING,
                                     Pin,
                                     BONDING);

  ret = addService();
//  ret = addAlrmService();
//  ret = addLogService();

  /* Set output power level */
  ret = aci_hal_set_tx_power_level(1,4);
  blue.connectable = TRUE;
  blue.connected = FALSE;
  blue.authorized = FALSE;
  blue.disconnCount = 0;

  myWD = MY_WD_TIME; // Запускаем програмный WatchDog
  return ret;
}

/* Добавляем сервис "РАБОЧИЕ ДАННЫЕ" и 4 характеристики */
static tBleStatus addService(void)
{
  tBleStatus ret;

  /*
  UUIDs:
  D973F230-B19E-11E2-9E96-0800200C9A66			- Work Service UUID
  D973F231-B19E-11E2-9E96-0800200C9A66			- Characteristic UUID
  D973F232-B19E-11E2-9E96-0800200C9A66
  D973F233-B19E-11E2-9E96-0800200C9A66
  D973F234-B19E-11E2-9E96-0800200C9A66
  */

  const uint8_t workServUuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0x30,0xf2,0x73,0xd9};
  const uint8_t timeCharUuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0x31,0xf2,0x73,0xd9};
  const uint8_t toMinMaxCharUuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0x32,0xf2,0x73,0xd9};
  const uint8_t toCurCharUuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0x33,0xf2,0x73,0xd9};
  const uint8_t ddCharUuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0x34,0xf2,0x73,0xd9};

  /*
  UUIDs:
  D973F241-B19E-11E2-9E96-0800200C9A66			- Characteristic UUID
  D973F242-B19E-11E2-9E96-0800200C9A66
  D973F243-B19E-11E2-9E96-0800200C9A66
  */
//  const uint8_t alrmServUuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0x40,0xf2,0x73,0xd9};
  const uint8_t alrmIdCharUuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0x41,0xf2,0x73,0xd9};
  const uint8_t alrmNewCharUuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0x42,0xf2,0x73,0xd9};
  const uint8_t alrmNoReadCharUuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0x43,0xf2,0x73,0xd9};

  /*
  UUIDs:
  D973F250-B19E-11E2-9E96-0800200C9A66			- Work Service UUID
  D973F251-B19E-11E2-9E96-0800200C9A66			- Characteristic UUID
  D973F252-B19E-11E2-9E96-0800200C9A66
  */
//  const uint8_t logServUuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0x50,0xf2,0x73,0xd9};
  const uint8_t logCharUuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0x52,0xf2,0x73,0xd9};
  const uint16_t logReqDescUuid = 0x29ff;


  ret = aci_gatt_add_serv(UUID_TYPE_128, workServUuid, PRIMARY_SERVICE, 24, &workServHandle); /* original is 9?? */
  if (ret != BLE_STATUS_SUCCESS) goto fail;

// Характеристика "Текущее Время"
  ret =  aci_gatt_add_char(workServHandle, UUID_TYPE_128, timeCharUuid, 4,
                           CHAR_PROP_READ|CHAR_PROP_WRITE_WITHOUT_RESP,
													 ATTR_PERMISSION_NONE,
													 GATT_NOTIFY_ATTRIBUTE_WRITE, 16, 0, &timeCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

// Характеристика "Температура Минимум-Максимум"
  ret =  aci_gatt_add_char(workServHandle, UUID_TYPE_128, toMinMaxCharUuid, 4,
                           CHAR_PROP_READ|CHAR_PROP_WRITE,
													 ATTR_PERMISSION_NONE,
 													 GATT_NOTIFY_ATTRIBUTE_WRITE, 16, 0, &toMinMaxCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

// Характеристика "Температура сейчас"
  ret =  aci_gatt_add_char(	workServHandle, UUID_TYPE_128, toCurCharUuid, TO_DEV_NUM * 2,
														CHAR_PROP_READ|CHAR_PROP_INDICATE,
														ATTR_PERMISSION_NONE,
														0, 16, 0, &toCurCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

// Характеристика "Датчик Двери"
  ret =  aci_gatt_add_char(	workServHandle, UUID_TYPE_128, ddCharUuid, 1,
														CHAR_PROP_READ|CHAR_PROP_INDICATE,
														ATTR_PERMISSION_NONE,
														0, 16, 0, &ddCurCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

// Добавляем сервис "ТРЕВОГА" и характеристики:

// Характеристика "Идентификаторы Поддерживаемых Тревог"
  ret =  aci_gatt_add_char( workServHandle, UUID_TYPE_128, alrmIdCharUuid, 1,
 														CHAR_PROP_READ,
														ATTR_PERMISSION_NONE,
														0, 16, 0, &alrmIdCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

// Характеристика "Новая тревога"
  ret =  aci_gatt_add_char( workServHandle, UUID_TYPE_128, alrmNewCharUuid, 4,
 														CHAR_PROP_READ|CHAR_PROP_NOTIFY,
														ATTR_PERMISSION_NONE,
														GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP, 16, 0, &alrmNewCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

// Характеристика "Непрочитанные тревоги"
  ret =  aci_gatt_add_char( workServHandle, UUID_TYPE_128, alrmNoReadCharUuid, 2,
  													CHAR_PROP_READ|CHAR_PROP_NOTIFY,
   												  ATTR_PERMISSION_NONE,
   													GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP, 16, 0, &alrmNoReadCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

// Битовая маска Идентификаторов Тревог
 	blue.alrmId = ALARM_GENERIC	| \
 								ALARM_DD_NEW_STATE | \
 								ALARM_TO_MAX | \
 								ALARM_TO_MIN | \
 								ALARM_DD_FAULT | \
 								ALARM_TO_FAULT | \
								ALARM_LOG_FAULT | \
								ALARM_HW_FAULT;
// Счетчики Новых Тревог (индекс соответствует номеру поля в alrmId )
 	blue.alrmNewCount = 0;
// Счетчики Непрочитанных Тревог (индекс соответствует номеру поля в alrmId)
 	blue.alrmNoReadCount = 0;

// Характеристика "Отправка логов"
  ret =  aci_gatt_add_char( workServHandle, UUID_TYPE_128, logCharUuid, 15,
													CHAR_PROP_READ|CHAR_PROP_NOTIFY,
 												  ATTR_PERMISSION_NONE,
 													GATT_NOTIFY_READ_REQ_AND_WAIT_FOR_APPL_RESP,
 													16, 0, &logCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

// Дескриптор "Запрос логов" к Характеристике "Отправка логов"
  uint8_t descVal = 0;
  aci_gatt_add_char_desc( workServHandle, logCharHandle, UUID_TYPE_16, (uint8_t *)&logReqDescUuid,
  												1, 1, &descVal,
													ATTR_PERMISSION_NONE, ATTR_ACCESS_WRITE_WITHOUT_RESPONSE,
 													GATT_NOTIFY_ATTRIBUTE_WRITE|GATT_NOTIFY_WRITE_REQ_AND_WAIT_FOR_APPL_RESP,
 													16, 0, &logReqDescHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

// Флаг - Запрос очередной записи Лога температуры
  blue.logStatus.toReq = DISABLE;
  blue.logStatus.toTxe = DISABLE;
// Флаг - Запрос очередной записи Лога температуры
	blue.logStatus.ddReq = DISABLE;
  blue.logStatus.ddTxe = DISABLE;

  return BLE_STATUS_SUCCESS;

fail:
  return BLE_STATUS_ERROR ;
}

void Make_Connection(void)
{
	// ОТображаемое имя устройства: "ITM-20.001"
  char local_name[16] = {	AD_TYPE_COMPLETE_LOCAL_NAME,'I','T','M','-',				// Indigo Thermo Meter
  														VER_MAJOR,VER_MINOR,'.'};

	char nameSuff[5] = { 0, 0, 0, 0, 0 };
	itoa( BDADDR, nameSuff, 10 );
	while ( nameSuff[2] == '\0') {
			nameSuff[2] = nameSuff[1];
			nameSuff[1] = nameSuff[0];
			nameSuff[0] = '0';
	}
	nameSuff[3] = DEV_SUFFIX;
	strcat(local_name, nameSuff);

  //hci_le_set_scan_resp_data(18,serviceUuidScan);
  /* disable scan response */
  hci_le_set_scan_resp_data(0,NULL);

  /*
  Advertising_Event_Type, Adv_Interval_Min, Adv_Interval_Max, Address_Type, Adv_Filter_Policy,
  Local_Name_Length, Local_Name, Service_Uuid_Length, Service_Uuid_List, Slave_Conn_Interval_Min,
  Slave_Conn_Interval_Max
  */
  aci_gap_set_discoverable(ADV_IND, 0, 0, PUBLIC_ADDR, NO_WHITE_LIST_USE,
                                 sizeof(local_name), local_name, 0, NULL, 0, 0);
}

/* Вызывается, если у сервера есть изменение данных */
static void writePermitReq(uint16_t handle, uint8_t* att_data, uint8_t dataLen)
{
	UNUSED(dataLen);

  if(handle == timeCharHandle + 1){
    // Текущее время
  	uxTime = *att_data;
  	setRtcTime( uxTime );
  }
  else if(handle == logReqDescHandle) {
  	if( att_data ) {
  		blue.logStatus.ddReq = ENABLE;
  		blue.logStatus.toReq = ENABLE;
  	}
  }
}

/**
 * @brief  This function is called when an attribute gets modified
 * @param  handle : handle of the attribute
 * @param  data_length : size of the modified attribute data
 * @param  att_data : pointer to the modified attribute data
 * @retval None
 */

void Attribute_Modified_CB(uint16_t handle, uint8_t dataLen, uint8_t *att_data)
{
	UNUSED(dataLen);

	/* Здесь обрабатываются данные:
	 * - Температура Макс	- toMinMaxCharHandle
	 * - Температута Мин	- toCurCharHandle
	 * - Запрос логов			- logReqDescHandle
	 * -
	 */
  if (handle == toMinMaxCharHandle + 1) {
  	// Минимум-Максимум температур
  	for( uint8_t i = 0; i < TO_DEV_NUM; i++ ) {
  		// Выставляем Минимум для всех датчиков одинаковую
  		owToDev[i].tMin = *((uint16_t *)att_data) & 0xFFF;
  		att_data += 2;
  		// Выставляем Максимум для всех датчиков одинаковую
  		owToDev[i].tMax = *((uint16_t *)att_data) & 0xFFF;
  	}
  }
  else if( handle == logReqDescHandle ) {
  	if( *att_data & 0x01 ) {
  		blue.logStatus.toReq = ENABLE;
  	}
  	if( *att_data & 0x02 ) {
  		blue.logStatus.ddReq = ENABLE;
  	}
  }
}

/* Здесь обрабатывается:
 * alrmNewCharHandle;			// Хэндл характеристики новых тревог
 * alrmNoReadCharHandle;	// Хэндл характеристики непрочитанных тревог
 * logCharHandle;
 */
static void readPermitRequest( uint16_t handle, uint8_t offset) {
	UNUSED(offset);

	handle--;

	if ( handle == logCharHandle ) {
		blue.logStatus.ddTxe = ENABLE;
		blue.logStatus.toTxe = ENABLE;
	}
	else 	if ( (handle == alrmNewCharHandle) || (handle == alrmNoReadCharHandle) ) {
		if ( (handle == alrmNewCharHandle) && blue.alrmSendId ) {
			blue.alrmNoReadId &= ~blue.alrmSendId;
			--blue.alrmNoReadCount;
			blue.alrmSendId = 0;
		}
		else {
			blue.alrmNoReadId = 0;
			blue.alrmNoReadCount = 0;
		}

		aci_gatt_update_char_value( workServHandle, alrmNewCharHandle, 0, 1, &blue.alrmNoReadId );
		aci_gatt_update_char_value( workServHandle, alrmNewCharHandle, 1, 1, &blue.alrmNoReadCount );
	}
	return;
}

/**
 * @brief  This function is called when there is a LE Connection Complete event.
 * @param  addr : Address of peer device
 * @param  handle : Connection handle
 * @retval None
 */
void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle)
{
	UNUSED(addr);
  blue.connected = TRUE;
  blue.connHandle = handle;

#ifdef PAIRING_ON
  ret = aci_gap_slave_security_request(blue.connHandle, BONDING,
                                       MITM_PROTECTION_REQUIRED );
  if (ret != BLE_STATUS_SUCCESS)
#ifdef  TERMINAL_ON  
    printf("Failure.\n")
#endif  // TERMINAL_ON  

#endif /* PPAIRING_ON */

}

/**
 * @brief  This function is called when the peer device get disconnected.
 * @param  None
 * @retval None
 */
void GAP_DisconnectionComplete_CB(void)
{
  blue.connected = FALSE;
  blue.authorized = FALSE;
  blue.disconnCount = 0;

#ifdef  TERMINAL_ON  
  printf("Disconnected\n\r");
#endif  // TERMINAL_ON  
  /* Make the device connectable again. */
  blue.connectable = TRUE;
  notification_enabled = FALSE;
  myWD = MY_WD_TIME; // Запускаем my WatchDog
}

#ifdef PAIRING_ON
/* Функция обработки события EVT_BLUE_GAP_PAIRING_CMPLT
  ==== при необходимости - дописать нужные действия =====
 */
tBleStatus pairingComplete( uint8_t status ) {
  tBleStatus ret;

  switch (status) {
    case SM_PAIRING_SUCCESS :
      ret = status;
      break;
    case SM_PAIRING_TIMEOUT :
      ret = status;
      break;
    case SM_PAIRING_FAILED  :
      ret = status;
      break;
  }
  return ret;
}
#endif /* PAIRING_ON */

void alarmCharUpdate( void ) {
	uint8_t i;

	aci_gatt_update_char_value( workServHandle, alrmIdCharHandle, 0, 1, &blue.alrmId );
	for( i = 0; i < 8; i++ ){
		if (blue.alrmNewId != 0){
			// Есть новая тревога - пишем ее в Характеристику
			aci_gatt_update_char_value( workServHandle, alrmNewCharHandle, 0, 1, &blue.alrmNewId );
			aci_gatt_update_char_value( workServHandle, alrmNewCharHandle, 1, 1, &blue.alrmNewCount );
			blue.alrmSendId = blue.alrmNewId;
			blue.alrmNewId = 0;
			break;
		}
	}
}

void toCurCharUpdate( void ) {
	uint16_t data[TO_DEV_NUM];
	uint8_t alrm = FALSE;

	for ( uint8_t i = 0; i < TO_DEV_NUM; i++) {
		data[i] = owToDev[i].temper;
		if ( owToDev[i].temper > owToDev[i].tMax ) {
			alrmUpdate( ALARM_TO_MAX );
		}
		else if ( owToDev[i].temper < owToDev[i].tMin ) {
			alrmUpdate( ALARM_TO_MAX );
		}
	}
	aci_gatt_update_char_value( workServHandle, toCurCharHandle, 0, 2*TO_DEV_NUM, (uint8_t *)data );
	if ( alrm ) {
		alarmCharUpdate();
	}
}

void ddCurCharUpdate( void ){
	uint8_t data;

// Обновляем Характеристику "Действующее значение состояния дверей"
	data = ddDev[0].ddData | (ddDev[1].ddData << 1);
	aci_gatt_update_char_value( workServHandle, ddCurCharHandle, 0, 1, (uint8_t *)&data );
// 	Изменилось ли состояние дверей - отправлять ли тревогу?
	if ( (ddDev[0].ddData != ddDev[0].ddDataPrev) || (ddDev[1].ddData != ddDev[1].ddDataPrev) ) {
		alrmUpdate( ALARM_DD_NEW_STATE );
	}
	ddDev[0].ddDataPrev = ddDev[0].ddData;
	ddDev[1].ddDataPrev = ddDev[1].ddData;
}

void logCharUpdate( uint8_t *data, uint8_t len) {
	aci_gatt_update_char_value( workServHandle, logCharHandle, 0, len, data );
}

/**
}
 * @brief  This function is called whenever there is an ACI event to be processed.
 * @note   Inside this function each event must be identified and correctly
 *         parsed.
 * @param  pckt  Pointer to the ACI packet
 * @retval None
 */
void HCI_Event_CB(void *pckt)
{
  hci_uart_pckt *hci_pckt = pckt;
  hci_event_pckt *event_pckt = (hci_event_pckt*)hci_pckt->data;
  void * temp;

  if(hci_pckt->type != HCI_EVENT_PKT)
    return;

  switch(event_pckt->evt){

    case EVT_CONN_COMPLETE:
      {
        evt_conn_complete * cuc = (void *)event_pckt->data;
        temp = cuc;
      }
      break;
    case EVT_CONN_REQUEST:
      {
        evt_conn_request * cr = (void *)event_pckt->data;
        temp = cr;
      }
      break;
    case EVT_ENCRYPT_CHANGE:
      {
        evt_encrypt_change * ec = (void *)event_pckt->data;
        temp = ec;
      }
      break;
    case EVT_READ_REMOTE_VERSION_COMPLETE:
      break;
    case EVT_CMD_COMPLETE:
      {
        evt_cmd_complete * cmdc = (void *)event_pckt->data;
        temp = cmdc;
      }
      break;
    case EVT_CMD_STATUS:
      {
        evt_cmd_status * cmds = (void *)event_pckt->data;
        temp = cmds;
      }
      break;
    case EVT_HARDWARE_ERROR:
      {
        evt_hardware_error * he = (void *)event_pckt->data;
        temp = he;
      }
      break;
    case EVT_NUM_COMP_PKTS:
      {
        evt_num_comp_pkts * ncp = (void *)event_pckt->data;
        temp = ncp;
      }
      break;
    case EVT_DATA_BUFFER_OVERFLOW:
      {
        evt_data_buffer_overflow * dbo = (void *)event_pckt->data;
        temp = dbo;
      }
      break;
    case EVT_ENCRYPTION_KEY_REFRESH_COMPLETE:
      {
        evt_encryption_key_refresh_complete * ekrc = (void *)event_pckt->data;
        temp = ekrc;
      }
      break;

  case EVT_DISCONN_COMPLETE:
    { 
      evt_disconn_complete * dc = (void *)event_pckt->data;
      temp = dc;
      GAP_DisconnectionComplete_CB();
      aci_gap_set_undirected_connectable(PUBLIC_ADDR, NO_WHITE_LIST_USE );
    }
    break;

  case EVT_LE_META_EVENT:
    {
      evt_le_meta_event *evt = (void *)event_pckt->data;

      switch(evt->subevent){
      case EVT_LE_CONN_COMPLETE:
        {
          myWD = 0; // Останавливаем програмный WatchDog
          evt_le_connection_complete *cc = (void *)evt->data;
/*  Проверка на бан
          if ( banTest( cc->peer_bdaddr ) ){       // Проверяем, не заванен ли
            GAP_ConnectionComplete_CB(cc->peer_bdaddr, cc->handle);
            HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, (GPIO_PinState)SET);
            setTokenStr();
          }
          else {
            aci_gap_terminate( cc->handle, VERY_EARLY_NEXT_ATTEMPT );
          }
*/
          GAP_ConnectionComplete_CB(cc->peer_bdaddr, cc->handle);
          alarmCharUpdate();
        }
        break;
      case EVT_LE_ADVERTISING_REPORT:
        {
          le_advertising_info *ar = (void *)evt->data;
          temp = ar;
        }
        break;
      case EVT_LE_CONN_UPDATE_COMPLETE:
        {
          evt_le_connection_update_complete * cuc = (void *)evt->data;
          temp = cuc;
        }
        break;
      case EVT_LE_READ_REMOTE_USED_FEATURES_COMPLETE:
        {
          evt_le_read_remote_used_features_complete * rrufc = (void *)evt->data;
          temp = rrufc;
        }
        break;
      case EVT_LE_LTK_REQUEST:
        {
          evt_le_long_term_key_request * ltkr= (void *)evt->data;
          temp = ltkr;
        }
        break;
        
      }
    }
    break;

  case EVT_VENDOR:
    {
      evt_blue_aci *blue_evt = (void*)event_pckt->data;
      switch(blue_evt->ecode){

        case EVT_BLUE_INITIALIZED:
          {
            evt_blue_initialized *evt = (evt_blue_initialized*)blue_evt->data;
            temp = (void *)evt;
          }
          break;
            
        case EVT_BLUE_GATT_ATTRIBUTE_MODIFIED:
          {
          	/* Здесь обрабатываются данные:
          	 * - Температура Макс
          	 * - Температута Мин
          	 * - Запрос Логов
          	 */
            evt_gatt_attr_modified *evt = (evt_gatt_attr_modified*)blue_evt->data;
            Attribute_Modified_CB(evt->attr_handle, evt->data_length, evt->att_data);
          }
          break;

        case EVT_BLUE_GATT_WRITE_PERMIT_REQ:
          {
          	/* Здесь обрабатываются данные:
          	 * - Текущее время
          	 */
            evt_gatt_write_permit_req *evt = (evt_gatt_write_permit_req *)blue_evt->data;
            aci_gatt_write_response( connection_handle, evt->attr_handle, 
                                    BLE_STATUS_SUCCESS, RECEIVE_ERR, 
                                    evt->data_length, evt->data  );
            writePermitReq(evt->attr_handle, evt->data+1, evt->data_length-1);
          }
          break;

      case EVT_BLUE_GATT_PROCEDURE_TIMEOUT:
          {
            evt_gatt_procedure_timeout * pt = (void *)blue_evt->data;
            temp = pt;
          }
          break;
      case EVT_BLUE_ATT_EXCHANGE_MTU_RESP:
          {
            evt_att_exchange_mtu_resp * aemr = (void *)blue_evt->data;
            temp = aemr;
          }
          break;

      case EVT_BLUE_ATT_FIND_INFORMATION_RESP:
          {
            evt_att_find_information_resp * afir = (void *)blue_evt->data;
            temp = afir;
          }
          break;

      case EVT_BLUE_ATT_FIND_BY_TYPE_VAL_RESP:
          {
            evt_att_find_by_type_val_resp * ftvr  = (void *)blue_evt->data;
            temp = ftvr;
          }
          break;

      case EVT_BLUE_ATT_READ_BY_TYPE_RESP:
          {
            evt_att_read_by_type_resp * artr = (void *)blue_evt->data;
            temp = artr;
          }
          break;

      case EVT_BLUE_ATT_READ_RESP:
          {
            evt_att_read_resp * arr = (void *)blue_evt->data;
            temp = arr;
          }
          break;

      case EVT_BLUE_ATT_READ_BLOB_RESP:
          {
            evt_att_read_blob_resp * arbr = (void *)blue_evt->data;
            temp = arbr;
          }
          break;

      case EVT_BLUE_ATT_READ_MULTIPLE_RESP:
          {
            evt_att_read_mult_resp * armr = (void *)blue_evt->data;
            temp = armr;
          }
          break;

      case EVT_BLUE_ATT_READ_BY_GROUP_TYPE_RESP:
          {
            evt_att_read_by_group_resp * argtr = (void *)blue_evt->data;
            temp = argtr;
          }
          break;

      case EVT_BLUE_ATT_PREPARE_WRITE_RESP:
          {
            evt_att_prepare_write_resp * apwr = (void *)blue_evt->data;
            temp = apwr;
          }
          break;

      case EVT_BLUE_ATT_EXEC_WRITE_RESP:
          {
            evt_att_exec_write_resp * aewr = (void *)blue_evt->data;
            temp = aewr;
          }
          break;

      case EVT_BLUE_GATT_INDICATION:
          {
            evt_gatt_indication * gi = (void *)blue_evt->data;
            temp = gi;
          }
          break;

      case EVT_BLUE_GATT_NOTIFICATION:
          {
            evt_gatt_attr_notification * gn = (void *)blue_evt->data;
            temp = gn;
          }
          break;

      case EVT_BLUE_GATT_PROCEDURE_COMPLETE:
          {
            evt_gatt_procedure_complete * apwr = (void *)blue_evt->data;
            temp = apwr;
          }
          break;

      case EVT_BLUE_GATT_ERROR_RESP:
          {
            evt_gatt_error_resp * ger = (void *)blue_evt->data;
            temp = ger;
          }
          break;

      case EVT_BLUE_GATT_DISC_READ_CHAR_BY_UUID_RESP:
          {
            evt_gatt_disc_read_char_by_uuid_resp * adrcur = (void *)blue_evt->data;
            temp = adrcur;
          }
          break;

      case EVT_BLUE_GATT_READ_PERMIT_REQ:
          {
            evt_gatt_read_permit_req * grpr = (void *)blue_evt->data;
            readPermitRequest( grpr->attr_handle, grpr->offset );
            temp = grpr;
          }
          break;

      case EVT_BLUE_GATT_READ_MULTI_PERMIT_REQ:
          {
            evt_gatt_read_multi_permit_req * grmpr = (void *)blue_evt->data;
            temp = grmpr;
          }
          break;
      }
    }
    break;
  }
  pckt=temp;
}
