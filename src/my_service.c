/* Includes ------------------------------------------------------------------*/
#include "my_service.h"
#include "connection_config.h"
#include "my_stm32l0xx_nucleo.h"
#include "my_main.h"
#include "osal.h"
#include <stdio.h>
#include "deviceid.h"

uint8_t relayStart( relayTypeDef rel );
uint8_t relayStatus( relayTypeDef rel );
tBleStatus setTokenStr();
void HardFault_Handler(void);

extern uint32_t Pin;
extern volatile uint32_t disconnCount; // Таймаут аутентификации по SHA1-хэш 120 сек.
volatile uint32_t resetCount; // Таймаут аутентификации по SHA1-хэш 120 сек.

/* Private variables ---------------------------------------------------------*/
volatile int connected = FALSE;
volatile uint8_t set_connectable = 1;
volatile uint16_t connection_handle = 0;
volatile uint8_t notification_enabled = FALSE; // Клиент готов получать данные
volatile uint8_t start_read_tx_char_handle = FALSE;
volatile uint8_t start_read_rx_char_handle = FALSE;
volatile uint8_t end_read_tx_char_handle = FALSE;
volatile uint8_t end_read_rx_char_handle = FALSE;

uint16_t tx_handle;
uint16_t rx_handle;

uint8_t authorized;             // признак совпадения токенов
uint8_t reqAuth;               // признак отправки запроса
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
uint16_t sampleServHandle, strCharHandle, respCharHandle, cmdCharHandle;

extern BLE_RoleTypeDef BLE_Role;
extern uint16_t uwTick, myWD;
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
  uint8_t bdaddr[BDADDR_SIZE] = { ((BDADDR>>8)&0xFF), (BDADDR&0xFF), 0x00, 0xE1, 0x80, 0x02 };
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

  ret = Add_Sample_Service();

  /* Set output power level */
  ret = aci_hal_set_tx_power_level(1,4);
  set_connectable = TRUE;

  myWD = MY_WD_TIME; // Запускаем програмный WatchDog
  return ret;
}

/* Добавляем сервис и 2 характеристики "RX" на прием от клиента, "TX" на передачу клиенту */
tBleStatus Add_Sample_Service(void)
{
  tBleStatus ret;

  /*
  UUIDs:
  D973F2E0-B19E-11E2-9E96-0800200C9A66
  D973F2E1-B19E-11E2-9E96-0800200C9A66
  D973F2E2-B19E-11E2-9E96-0800200C9A66
  */

  const uint8_t service_uuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe0,0xf2,0x73,0xd9};
  const uint8_t strCharUuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe1,0xf2,0x73,0xd9};
  const uint8_t respCharUuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe3,0xf2,0x73,0xd9};
  const uint8_t cmdCharUuid[16] = {0x66,0x9a,0x0c,0x20,0x00,0x08,0x96,0x9e,0xe2,0x11,0x9e,0xb1,0xe2,0xf2,0x73,0xd9};

  ret = aci_gatt_add_serv(UUID_TYPE_128, service_uuid, PRIMARY_SERVICE, 13, &sampleServHandle); /* original is 9?? */
  if (ret != BLE_STATUS_SUCCESS) goto fail;

  ret =  aci_gatt_add_char(sampleServHandle, UUID_TYPE_128, strCharUuid, 52, 
                           CHAR_PROP_NOTIFY|CHAR_PROP_READ, ATTR_PERMISSION_NONE,
                           0, 16, 1, &strCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;
  
  ret =  aci_gatt_add_char(sampleServHandle, UUID_TYPE_128, respCharUuid, 2, 
                           CHAR_PROP_NOTIFY|CHAR_PROP_READ, ATTR_PERMISSION_NONE, 
                           0, 16, 1, &respCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

  ret =  aci_gatt_add_char(sampleServHandle, UUID_TYPE_128, cmdCharUuid, 52,
                           CHAR_PROP_WRITE|CHAR_PROP_WRITE_WITHOUT_RESP|CHAR_PROP_READ, ATTR_PERMISSION_NONE,
                           GATT_NOTIFY_ATTRIBUTE_WRITE, 16, 1, &cmdCharHandle);
  if (ret != BLE_STATUS_SUCCESS) goto fail;

#ifdef TERMINAL
  printf("Sample Service added.\n\rTX Char Handle %04X, RX Char Handle %04X\n\r", strCharHandle, RXCharHandle);
#endif /* TERMINAL */

  return BLE_STATUS_SUCCESS;

fail:
#ifdef TERMINAL
  printf("Error while adding Sample Service.\n\r");
#endif /* TERMINAL */
  return BLE_STATUS_ERROR ;
}

void Make_Connection(void)
{
  const char local_name[] = {AD_TYPE_COMPLETE_LOCAL_NAME,'I','n','d','i','g','o','-','T','o','o','t','h'};

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

tBleStatus receiveCmd( uint8_t * buffer ){
  tBleStatus err;
  
  switch (buffer[0]) {
    case RELAY_ON_CMD:
      switch (buffer[1]) {
        case RELAY1:
        case RELAY2:
          err = relayStart( (relayTypeDef)buffer[1] );
          break;
        default: 
          err = REL_NUM_ERR;
          break;
      }
      break;
    case RELAY_STATUS_CMD:
      err = relayStatus( RELAY1 ) | ( relayStatus ( RELAY2 ) << 1);
      err = (err > 3) ? RELAY_STATUS_ERR : err ;
      break;
    default:
      err = RECEIVE_ERR;
  }
  return err;

  
}

tBleStatus sendResp( uint8_t resp )
{
  return aci_gatt_update_char_value(sampleServHandle, respCharHandle, 0, 1, &resp);
}

/* Вызывается, если у сервера есть изменение данных */
void receiveDataModified(uint8_t* data_buffer, uint8_t Nb_bytes)
{
  tBleStatus err = 0xFF;
  uint32_t i =0 , k;

  if (tokenPart <40 ){
    k = (tokenPart+Nb_bytes) <= 40 ? Nb_bytes : 40;
    for ( ; i < k; i++ ){
      if ( shaHash[i+tokenPart] != data_buffer[i] ) {
      // sha-хэш не совпадает - соединение обрываем
        err = TOKEN_ERR;
        HAL_Delay( 500 );
        aci_gap_terminate(connection_handle, SHA_TIMEOUT_REASON);
//        aci_gap_set_non_connectable( ADV_NONCONN_IND );
        GAP_DisconnectionComplete_CB();
        return;
      }
    }
  }
  if ((tokenPart+Nb_bytes) <= 40){
    tokenPart = tokenPart+Nb_bytes;   // Первая половина токена совпала полностью
    return;
  } else {
    disconnCount = 0;     // Остановка таймера разрыва связи
    authorized = TRUE;
    err = receiveCmd(&data_buffer[i]);
    sendResp( err );
    setTokenStr();
    err = TOKEN_SUCCESS;
    return;
  }
}

/**
 * @brief  This function is used to send data related to the sample service
 *         (to be sent over the air to the remote board).
 * @param  data_buffer : pointer to data to be sent
 * @param  Nb_bytes : number of bytes to send
 * @retval None
 */
tBleStatus sendData(uint8_t* data_buffer, uint8_t Nb_bytes)
{
#ifdef CLIENT_ROLE
    return aci_gatt_write_without_response(connection_handle, rx_handle+1, Nb_bytes, data_buffer);
#else /* SERVER_ROLE */
    return aci_gatt_update_char_value(sampleServHandle, strCharHandle, 0, Nb_bytes, data_buffer);
#endif /* CLIENT_ROLE */
}

/**
 * @brief  This function is called when an attribute gets modified
 * @param  handle : handle of the attribute
 * @param  data_length : size of the modified attribute data
 * @param  att_data : pointer to the modified attribute data
 * @retval None
 */
void Attribute_Modified_CB(uint16_t handle, uint8_t data_length, uint8_t *att_data)
{
  if(handle == cmdCharHandle + 1){
    /*  */
    /*  */
    if (( resetCount > 0 ) && ( (data_length == 2) && (att_data[0] == 0xA5) && (att_data[1] == 0x5A) )) {
         resetCount = 0;
         return;
      }
    if ( (data_length == 2) && (att_data[0] == 0x5A) && (att_data[1] == 0xA5) ) {
      resetCount = 5000;
      return;
    }

    /* Есть принятые данные */
    receiveDataModified(att_data, data_length);
  } else if (handle == strCharHandle + 2) {
    if((att_data[1] & 0x01) == 0x01) // Если установлен флаг в дескрипторе NOTIFY
      notification_enabled = TRUE;
#ifdef TERMINAL_ON  
      printf("Notification enabled\n\r");
#endif // TERMINAL_ON  

  }
}

/**
 * @brief  This function is called when there is a LE Connection Complete event.
 * @param  addr : Address of peer device
 * @param  handle : Connection handle
 * @retval None
 */
void GAP_ConnectionComplete_CB(uint8_t addr[6], uint16_t handle)
{
  connected = TRUE;
  connection_handle = handle;

#ifdef TERMINAL_ON  
  printf("Connected to device:");
  for(int i = 5; i > 0; i--){
    printf("%02X-", addr[i]);
  }
  printf("%02X\n\r", addr[0]);
#endif  // TERMINAL_ON  

#ifdef PAIRING_ON
  ret = aci_gap_slave_security_request(conn_handle, BONDING,
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
  connected = FALSE;
  authorized = FALSE;
  reqAuth = FALSE;
  tokenPart = 0;
  disconnCount =0;

#ifdef  TERMINAL_ON  
  printf("Disconnected\n\r");
#endif  // TERMINAL_ON  
  /* Make the device connectable again. */
  set_connectable = TRUE;
  notification_enabled = FALSE;
  for ( uint8_t i =0; i<40; i++ )
    shaHash[i] = 0x00;
  sendData( shaHash, 40 );
  HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, GPIO_PIN_RESET);
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


/**
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
          HAL_GPIO_WritePin(LED1_PORT, LED1_PIN, (GPIO_PinState)SET);
          setTokenStr();
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
            evt_gatt_attr_modified *evt = (evt_gatt_attr_modified*)blue_evt->data;
            Attribute_Modified_CB(evt->attr_handle, evt->data_length, evt->att_data);
          }
          break;

        case EVT_BLUE_GATT_WRITE_PERMIT_REQ:
          {
            evt_gatt_write_permit_req *evt = (evt_gatt_write_permit_req *)blue_evt->data;
            aci_gatt_write_response( connection_handle, evt->attr_handle, 
                                    BLE_STATUS_SUCCESS, RECEIVE_ERR, 
                                    evt->data_length, evt->data  );
            receiveDataModified(evt->data+1, evt->data_length-1);
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
            temp = grpr;
          }
          break;

      case EVT_BLUE_GATT_READ_MULTI_PERMIT_REQ:
          {
            evt_gatt_read_multi_permit_req * grmpr = (void *)blue_evt->data;
            temp = grmpr;
          }
          break;

//	default:
//	  HardFault_Handler();
      }
    }
    break;
  }
  pckt=temp;
}
