#ifndef __EVENTS_H
#define __EVENTS_H

#define EVT_CONN_COMPLETE		0x03
typedef __packed struct _evt_conn_complete{
  uint8_t  status;
  uint16_t handle;
  tBDAddr  bdaddr;
  uint8_t  link_type;
  uint8_t  encr_mode;
} PACKED evt_conn_complete;
#define EVT_CONN_COMPLETE_SIZE 13

#define EVT_CONN_REQUEST		0x04
typedef __packed struct _evt_conn_request{
  uint8_t  status;
  uint16_t handle;
  tBDAddr  bdaddr;
  uint8_t  device_Class[3];
  uint8_t  link_type;
} PACKED evt_conn_request;
#define EVT_CONN_REQUEST_SIZE 10

#define EVT_DISCONN_COMPLETE		0x05
typedef __packed struct _evt_disconn_complete{
  uint8_t  status;
  uint16_t handle;
  uint8_t  reason;
} PACKED evt_disconn_complete;
#define EVT_DISCONN_COMPLETE_SIZE 4

#define EVT_ENCRYPT_CHANGE		0x08
typedef __packed struct _evt_encrypt_change{
  uint8_t  status;
  uint16_t handle;
  uint8_t  encrypt;
} PACKED evt_encrypt_change;
#define EVT_ENCRYPT_CHANGE_SIZE 5

#define EVT_READ_REMOTE_VERSION_COMPLETE	0x0C

#define EVT_CMD_COMPLETE 		0x0E
typedef __packed struct _evt_cmd_complete{
  uint8_t  ncmd;
  uint16_t opcode;
} PACKED evt_cmd_complete;
#define EVT_CMD_COMPLETE_SIZE 3

#define EVT_CMD_STATUS 			0x0F
typedef __packed struct _evt_cmd_status{
  uint8_t  status;
  uint8_t  ncmd;
  uint16_t opcode;
} PACKED evt_cmd_status;
#define EVT_CMD_STATUS_SIZE 4

#define EVT_HARDWARE_ERROR		0x10
typedef __packed struct _evt_hardware_error{
  uint8_t code;
} PACKED evt_hardware_error;
#define EVT_HARDWARE_ERROR_SIZE 1

#define EVT_NUM_COMP_PKTS		0x13
typedef __packed struct _evt_num_comp_pkts{
  uint8_t num_hndl;
  /* variable length part */
} PACKED evt_num_comp_pkts;
#define EVT_NUM_COMP_PKTS_SIZE 1

/* variable length part of evt_num_comp_pkts. */
typedef __packed struct _evt_num_comp_pkts_param{
  uint16_t hndl;
  uint16_t num_comp_pkts;
} PACKED evt_num_comp_pkts_param;
#define EVT_NUM_COMP_PKTS_PARAM_SIZE 1

#define EVT_DATA_BUFFER_OVERFLOW		0x1A
typedef __packed struct _evt_data_buffer_overflow{
  uint8_t link_type;
} PACKED evt_data_buffer_overflow;
#define EVT_DATA_BUFFER_OVERFLOW_SIZE 1

#define EVT_ENCRYPTION_KEY_REFRESH_COMPLETE	0x30
typedef __packed struct _evt_encryption_key_refresh_complete{
  uint8_t  status;
  uint16_t handle;
} PACKED evt_encryption_key_refresh_complete;
#define EVT_ENCRYPTION_KEY_REFRESH_COMPLETE_SIZE 3

#define EVT_LE_META_EVENT	0x3E
typedef __packed struct _evt_le_meta_event{
  uint8_t subevent;
  uint8_t data[0];
} PACKED evt_le_meta_event;
#define EVT_LE_META_EVENT_SIZE 1

#define EVT_LE_CONN_COMPLETE	0x01
typedef __packed struct _evt_le_connection_complete{
  uint8_t  status;
  uint16_t handle;
  uint8_t  role;
  uint8_t  peer_bdaddr_type;
  tBDAddr  peer_bdaddr;
  uint16_t interval;
  uint16_t latency;
  uint16_t supervision_timeout;
  uint8_t  master_clock_accuracy;
} PACKED evt_le_connection_complete;
#define EVT_LE_CONN_COMPLETE_SIZE 18

#define EVT_LE_ADVERTISING_REPORT	0x02
typedef __packed struct _le_advertising_info{
  uint8_t evt_type;
  uint8_t bdaddr_type;
  tBDAddr bdaddr;
  uint8_t data_length;
  uint8_t data_RSSI[0]; // RSSI is last octect (signed integer).
} PACKED le_advertising_info;
#define LE_ADVERTISING_INFO_SIZE 9

#define EVT_LE_CONN_UPDATE_COMPLETE	0x03
typedef __packed struct _evt_le_connection_update_complete{
  uint8_t  status;
  uint16_t handle;
  uint16_t interval;
  uint16_t latency;
  uint16_t supervision_timeout;
} PACKED evt_le_connection_update_complete;
#define EVT_LE_CONN_UPDATE_COMPLETE_SIZE 9

#define EVT_LE_READ_REMOTE_USED_FEATURES_COMPLETE	0x04
typedef __packed struct _evt_le_read_remote_used_features_complete{
  uint8_t  status;
  uint16_t handle;
  uint8_t  features[8];
} PACKED evt_le_read_remote_used_features_complete;
#define EVT_LE_READ_REMOTE_USED_FEATURES_COMPLETE_SIZE 11

#define EVT_LE_LTK_REQUEST	0x05
typedef __packed struct _evt_le_long_term_key_request{
  uint16_t handle;
  uint8_t  random[8];
  uint16_t ediv;
} PACKED evt_le_long_term_key_request;
#define EVT_LE_LTK_REQUEST_SIZE 12

/**
* The event code in the @ref hci_event_pckt structure. If event code is EVT_VENDOR,
* application can use @ref evt_blue_aci structure to parse the packet.
*/
#define EVT_VENDOR	0xFF

/* ################# VENDOR VENTS ############################# */
#define EVT_BLUE_INITIALIZED                      (0x0001)
typedef __packed struct _evt_blue_initialized{
  uint8_t reason_code;
} PACKED evt_blue_initialized;

/* ################## GAP EVENTS ############################# */
#define EVT_BLUE_GAP_LIMITED_DISCOVERABLE     (0x0400)

/**
 * This event is generated when the pairing process has completed successfully
 * or a pairing procedure timeout has occurred or the pairing has failed.
 * This is to notify the application that we have paired with a remote device
 * so that it can take further actions or to notify that a timeout has occurred
 *  so that the upper layer can decide to disconnect the link. See @ref _evt_gap_pairing_cmplt.
 */
#define EVT_BLUE_GAP_PAIRING_CMPLT                (0x0401)
typedef __packed struct _evt_gap_pairing_cmplt{
  uint16_t conn_handle; /**< Connection handle on which the pairing procedure completed */
  /**
   * 0x00: Pairing Success. Pairing with a remote device was successful\n
   * 0x01: Pairing Timeout. The SMP timeout has elapsed and no further SMP commands will be processed until reconnection\n
   * 0x02: Pairing Failed. The pairing failed with the remote device.
   */
  uint8_t  status;
} PACKED evt_gap_pairing_cmplt;

/**
 * This event is generated by the Security manager to the application when a pass key is required for pairing.
 * When this event is received, the application has to respond with the aci_gap_pass_key_response() command.
 * See @ref _evt_gap_pass_key_req.
 */
#define EVT_BLUE_GAP_PASS_KEY_REQUEST             (0x0402)
typedef __packed struct _evt_gap_pass_key_req{
  uint16_t conn_handle; /**< Connection handle for which the passkey has been requested. */
} PACKED evt_gap_pass_key_req;


/**
 * This event is generated by the Security manager to the application when the application
 * has set that authorization is required for reading/writing of attributes. This event will
 * be generated as soon as the pairing is complete. When this event is received,
 * aci_gap_authorization_response() command should be used by the application.
 * See @ref _evt_gap_author_req.
 */
#define EVT_BLUE_GAP_AUTHORIZATION_REQUEST        (0x0403)
typedef __packed struct _evt_gap_author_req{
  uint16_t conn_handle; /**< Connection handle for which authorization has been requested. */
} PACKED evt_gap_author_req;

/**
 * This event is generated when the slave security request is successfully sent to the master.
 * No parameters for this event.
 */
#define EVT_BLUE_GAP_SLAVE_SECURITY_INITIATED     (0X0404)

/**
 * This event is generated when a pairing request is issued in response to a slave security
 * request from a master which has previously bonded with the slave. When this event is received,
 * the upper layer has to issue the command aci_gap_allow_rebond() in order to allow the slave
 * to continue the pairing process with the master. No parameters for this event
 */
#define EVT_BLUE_GAP_BOND_LOST                    (0X0405)

/**
 * The event is given by the GAP layer to the upper layers when a device is discovered during scanning
 * as a consequence of one of the GAP procedures started by the upper layers. See @ref _evt_gap_device_found.
 */
#define EVT_BLUE_GAP_DEVICE_FOUND                 (0x0406)
typedef __packed struct _evt_gap_device_found{
  	uint8_t		evt_type;     /**< Type of event (@ref ADV_IND, @ref ADV_DIRECT_IND, @ref ADV_SCAN_IND, @ref ADV_NONCONN_IND, @ref SCAN_RSP) */
	uint8_t		bdaddr_type;  /**< Type of the peer address (@ref PUBLIC_ADDR, @ref RANDOM_ADDR). */
	tBDAddr	    bdaddr;       /**< Address of the peer device found during scanning. */
	uint8_t		data_length;  /**< Length of advertising or scan response data. */
	uint8_t		data_RSSI[VARIABLE_SIZE]; /**< Advertising or scan response data + RSSI. RSSI is last octect (signed integer). */
} PACKED evt_gap_device_found;

/**
 * This event is sent by the GAP to the upper layers when a procedure previously started has been terminated
 * by the upper layer or has completed for any other reason. See @ref _evt_gap_procedure_complete.
 */
#define EVT_BLUE_GAP_PROCEDURE_COMPLETE           (0x0407)
typedef __packed struct _evt_gap_procedure_complete{
  uint8_t procedure_code; /**< Terminated procedure. See @ref gap_procedure_codes "GAP procedure codes". */
  /**
   * @ref BLE_STATUS_SUCCESS, @ref BLE_STATUS_FAILED or @ref ERR_AUTH_FAILURE (procedure failed
   * due to authentication requirements).
   */
  uint8_t status;
  /**
   * Procedure specific data.\n
   * @li For Name Discovery Procedure:\n
   * the name of the peer device if the procedure completed successfully.
   * @li For General Connection Establishment Procedure:\n
   * The reconnection address written to the peripheral device if the peripheral is privacy enabled
   */
  uint8_t data[VARIABLE_SIZE];
} PACKED evt_gap_procedure_complete;

#if BLUENRG_MS
///@cond BLUENRG_MS
/**
 * This event is sent only by a privacy enabled Peripheral. The event is sent to the upper layers when the peripheral
 * is not able to resolve the private address of the peer device after connecting to it.
 */
#define EVT_BLUE_GAP_ADDR_NOT_RESOLVED              (0x0408)
typedef __packed struct _evt_gap_addr_not_resolved{
  uint16_t conn_handle; /**< Connection handle for which the private address could not be resolved with any of the stored IRK's.  */
} PACKED evt_gap_addr_not_resolved;
///@endcond
#else
///@cond BLUENRG
/**
 * This event is raised when the reconnection address is generated during the general connection
 * establishment procedure. The same address is set into the peer device also as a part of the general
 * connection establishment procedure. In order to make use of the reconnection address the next time
 * while connecting to the bonded peripheral, the application needs to use this reconnection address
 * as its own address as well as the peer address to which it wants to connect. See aci_gap_start_general_conn_establish_proc()
 * and aci_gap_start_auto_conn_establish_proc().
 */
#define EVT_BLUE_GAP_RECONNECTION_ADDRESS           (0x0408)
typedef __packed struct _evt_gap_reconnection_addr{
  uint8_t reconnection_address[6]; /**< 6 bytes of reconnection address that has been generated */
} PACKED evt_gap_reconnection_addr;
///@endcond
#endif //BLUENRG_MS

/* ####################### L2CAP EVENTS ################################# */
/**
 * This event is generated when the master responds to the L2CAP connection update request packet.
 * For more info see CONNECTION PARAMETER UPDATE RESPONSE and COMMAND REJECT in Bluetooth Core v4.0 spec.
 */
#define EVT_BLUE_L2CAP_CONN_UPD_RESP		  (0x0800)
typedef __packed struct _evt_l2cap_conn_upd_resp{
  uint16_t conn_handle;         /**< The connection handle related to the event. */
  uint8_t  event_data_length;  /**< Length of following data. */
/**
 * @li 0x13 in case of valid L2CAP Connection Parameter Update Response packet.
 * @li 0x01 in case of Command Reject.
 */
  uint8_t  code;
  uint8_t  identifier;    /**< Identifier of the response. It is equal to the request. */
  uint16_t l2cap_length;  /**< Length of following data. It should always be 2 */
/**
 * Result code (parameters accepted or rejected) in case of Connection Parameter Update
 * Response (code=0x13) or reason code for rejection in case of Command Reject (code=0x01).
 */
  uint16_t result;
} PACKED evt_l2cap_conn_upd_resp;

/**
 * This event is generated when the master does not respond to the connection update request
 * within 30 seconds.
 */
#define EVT_BLUE_L2CAP_PROCEDURE_TIMEOUT      (0x0801)

/**
 * The event is given by the L2CAP layer when a connection update request is received from the slave.
 * The application has to respond by calling aci_l2cap_connection_parameter_update_response().
 */
#define EVT_BLUE_L2CAP_CONN_UPD_REQ		  	  (0x0802)
typedef __packed struct _evt_l2cap_conn_upd_req{
/**
 * Handle of the connection for which the connection update request has been received.
 * The same handle has to be returned while responding to the event with the command
 * aci_l2cap_connection_parameter_update_response().
 */
  uint16_t conn_handle; 
  uint8_t  event_data_length; /**< Length of following data. */
/**
 * This is the identifier which associates the request to the
 * response. The same identifier has to be returned by the upper
 * layer in the command aci_l2cap_connection_parameter_update_response().
 */
  uint8_t  identifier; 
  uint16_t l2cap_length;  /**< Length of the L2CAP connection update request. */
  uint16_t interval_min;  /**< Value as defined in Bluetooth 4.0 spec, Volume 3, Part A 4.20. */
  uint16_t interval_max;  /**< Value as defined in Bluetooth 4.0 spec, Volume 3, Part A 4.20. */
  uint16_t slave_latency; /**< Value as defined in Bluetooth 4.0 spec, Volume 3, Part A 4.20. */
  uint16_t timeout_mult;  /**< Value as defined in Bluetooth 4.0 spec, Volume 3, Part A 4.20. */
} PACKED evt_l2cap_conn_upd_req;

/* ####################### GATT EVENTS  ################################# */
/**
 * This event is raised to the application by the GATT server when a client modifies any attribute on the server,
 * if event is enabled (see @ref Gatt_Event_Mask "Gatt Event Mask"). See @ref _evt_gatt_attr_modified.
 */
#define EVT_BLUE_GATT_ATTRIBUTE_MODIFIED          (0x0C01)
typedef __packed struct _evt_gatt_attr_modified{
  uint16_t conn_handle; /**< The connection handle which modified the attribute. */
  uint16_t attr_handle; /**< Handle of the attribute that was modified. */
  uint8_t  data_length; /**< The length of the data */
#if BLUENRG_MS
///@cond BLUENRG_MS
  uint16_t  offset; /**< Offset from which the write has been performed by the peer device */
///@endcond
#endif
  uint8_t  att_data[VARIABLE_SIZE]; /**< The new value (length is data_length) */
} PACKED evt_gatt_attr_modified;

/**
 * This event is generated by the client/server to the application on a GATT timeout (30 seconds).
 * See @ref _evt_gatt_procedure_timeout.
 */
#define EVT_BLUE_GATT_PROCEDURE_TIMEOUT           (0x0C02)
typedef __packed struct _evt_gatt_procedure_timeout{
	uint16_t conn_handle; /**< The connection handle on which the GATT procedure has timed out */
} PACKED evt_gatt_procedure_timeout;

/**
 * This event is generated in response to an Exchange MTU request. See aci_gatt_exchange_configuration().
 * See @ref _evt_att_exchange_mtu_resp.
 */
#define EVT_BLUE_ATT_EXCHANGE_MTU_RESP		  (0x0C03)
typedef __packed struct _evt_att_exchange_mtu_resp{
  uint16_t conn_handle; /**< The connection handle related to the response */
  uint8_t  event_data_length; /**< Length of following data (always 1). */
  uint16_t server_rx_mtu; /**< Attribute server receive MTU size */
} PACKED evt_att_exchange_mtu_resp;

/**
 * This event is generated in response to a @a Find @a Information @a Request. See aci_att_find_information_req() and
 * Find Information Response in Bluetooth Core v4.0 spec. See @ref _evt_att_find_information_resp.
 */
#define EVT_BLUE_ATT_FIND_INFORMATION_RESP	  (0x0C04)
typedef __packed struct _evt_att_find_information_resp{
  uint16_t conn_handle;			/**< The connection handle related to the response */
  uint8_t  event_data_length;	/**< Length of following data. */
  uint8_t  format;				/**< The format of the handle_uuid_pair. @arg 1: 16-bit UUIDs @arg 2: 128-bit UUIDs */
  /**
   *  A sequence of handle-uuid pairs.\n
   *  @li if format=1, each pair is:\n
   *  [2 octets for handle, 2 octets for UUIDs] \n
   *  @li if format=2, each pair is:\n
   *  [2 octets for handle, 16 octets for UUIDs]
   */
  uint8_t  handle_uuid_pair[VARIABLE_SIZE];
} PACKED evt_att_find_information_resp;

/**
 * This event is generated in response to a @a Find @a By @a Type @a Value @a Request. See
 * Find By Type Value Response in Bluetooth Core v4.0 spec. See @ref _evt_att_find_by_type_val_resp.
 */
#define EVT_BLUE_ATT_FIND_BY_TYPE_VAL_RESP	  (0x0C05)
typedef __packed struct _evt_att_find_by_type_val_resp{
  uint16_t conn_handle;				/**< The connection handle related to the response */
  uint8_t  event_data_length;		/**< Length of following data. */
  /**
   *  Handles Information List as defined in Bluetooth Core v4.0 spec.
   *  A sequence of handle pairs: [2 octets for Found Attribute Handle, 2 octets for Group End Handle]
   */
  uint8_t  handles_info_list[VARIABLE_SIZE];
} PACKED evt_att_find_by_type_val_resp;

/**
 * This event is generated in response to a @a Read @a By @a Type @a Request. See aci_gatt_find_included_services() and
 * aci_gatt_disc_all_charac_of_serv().
 * For more info see Read By Type Response in Bluetooth Core v4.0 spec. See @ref _evt_att_read_by_type_resp.
 */
#define EVT_BLUE_ATT_READ_BY_TYPE_RESP		  (0x0C06)
typedef __packed struct _evt_att_read_by_type_resp{
  uint16_t conn_handle;				/**< The connection handle related to the response */
  uint8_t  event_data_length;		/**< Length of following data. */
  uint8_t  handle_value_pair_length; /**< The size of each attribute handle-value pair */
  /**
   *  Attribute Data List as defined in Bluetooth Core v4.0 spec.
   *  A sequence of handle-value pairs: [2 octets for Attribute Handle, (handle_value_pair_length - 2 octets) for Attribute Value]
   */
  uint8_t  handle_value_pair[VARIABLE_SIZE];
} PACKED evt_att_read_by_type_resp;

/**
 * This event is generated in response to a @a Read @a Request. See aci_gatt_read_charac_val().
 * For more info see Read Response in Bluetooth Core v4.0 spec. See @ref _evt_att_read_resp.
 */
#define EVT_BLUE_ATT_READ_RESP			  (0x0C07)
typedef __packed struct _evt_att_read_resp{
  uint16_t conn_handle;				/**< The connection handle related to the response. */
  uint8_t  event_data_length;		/**< Length of following data. */
  uint8_t  attribute_value[VARIABLE_SIZE]; /**< The value of the attribute. */
} PACKED evt_att_read_resp;

/**
 * This event is generated in response to a @a Read @a Blob @a Request. See aci_gatt_read_long_charac_val().
 * For more info see Read Blob Response in Bluetooth Core v4.0 spec. See @ref _evt_att_read_blob_resp.
 */
#define EVT_BLUE_ATT_READ_BLOB_RESP		  (0x0C08)
typedef __packed struct _evt_att_read_blob_resp{
  uint16_t conn_handle;				/**< The connection handle related to the response. */
  uint8_t  event_data_length;		/**< Length of following data. */
  uint8_t  part_attribute_value[VARIABLE_SIZE]; /**< Part of the attribute value. */
} PACKED evt_att_read_blob_resp;

/**
 * This event is generated in response to a @a Read @a Multiple @a Request.
 * For more info see Read Multiple Response in Bluetooth Core v4.0 spec. See @ref _evt_att_read_mult_resp.
 */
#define EVT_BLUE_ATT_READ_MULTIPLE_RESP		  (0x0C09)
typedef __packed struct _evt_att_read_mult_resp{
  uint16_t conn_handle;				/**< The connection handle related to the response. */
  uint8_t  event_data_length;		/**< Length of following data. */
  uint8_t  set_of_values[VARIABLE_SIZE]; /**< A set of two or more values.*/
} PACKED evt_att_read_mult_resp;

/**
 * This event is generated in response to a @a Read @a By @a Group @a Type @a Request. See aci_gatt_disc_all_prim_services().
 * For more info see Read By Group type Response in Bluetooth Core v4.0 spec. See @ref _evt_att_read_by_group_resp.
 */
#define EVT_BLUE_ATT_READ_BY_GROUP_TYPE_RESP           (0x0C0A)
typedef __packed struct _evt_att_read_by_group_resp{
  uint16_t conn_handle;				/**< The connection handle related to the response. */
  uint8_t  event_data_length;		/**< Length of following data. */
  uint8_t  attribute_data_length;   /**< The size of each Attribute Data. */
  /**
   *  A list of Attribute Data where the attribute data is composed by:
   *  @li 2 octets for Attribute Handle
   *  @li 2 octets for End Group Handle
   *  @li (attribute_data_length - 4) octets for Attribute Value
   */
  uint8_t  attribute_data_list[VARIABLE_SIZE];
} PACKED evt_att_read_by_group_resp;

/**
 * This event is generated in response to a @a Prepare @a Write @a Request.
 * For more info see Prepare Write Response in Bluetooth Core v4.0 spec. See @ref _evt_att_prepare_write_resp.
 */
#define EVT_BLUE_ATT_PREPARE_WRITE_RESP		  (0x0C0C)
typedef __packed struct _evt_att_prepare_write_resp{
  uint16_t conn_handle;				/**< The connection handle related to the response. */
  uint8_t  event_data_length;		/**< Length of following data. */
  uint16_t  attribute_handle;		/**< The handle of the attribute to be written. */
  uint16_t  offset;					/**< The offset of the first octet to be written. */
  uint8_t  part_attr_value[VARIABLE_SIZE]; /**< The value of the attribute to be written. */
} PACKED evt_att_prepare_write_resp;

/**
 * This event is generated in response to an @a Execute @a Write @a Request.
 * For more info see Execute Write Response in Bluetooth Core v4.0 spec. See @ref _evt_att_exec_write_resp.
 */
#define EVT_BLUE_ATT_EXEC_WRITE_RESP		  (0x0C0D)
typedef __packed struct _evt_att_exec_write_resp{
  uint16_t conn_handle;			/**< The connection handle related to the response. */
  uint8_t  event_data_length; 	/**< Always 0. */
} PACKED evt_att_exec_write_resp;

/**
 * This event is generated when an indication is received from the server.
 * For more info see Handle Value Indication in Bluetooth Core v4.0 spec. See @ref _evt_gatt_indication.
 */
#define EVT_BLUE_GATT_INDICATION		  (0x0C0E)
typedef __packed struct _evt_gatt_indication{
  uint16_t conn_handle;		  			/**< The connection handle related to the event. */
  uint8_t  event_data_length; 			/**< Length of following data. */
  uint16_t attr_handle;					/**< The handle of the attribute. */
  uint8_t  attr_value[VARIABLE_SIZE]; 	/**< The current value of the attribute. */
} PACKED evt_gatt_indication;

/**
 * This event is generated when a notification is received from the server.
 * For more info see Handle Value Notification in Bluetooth Core v4.0 spec. See @ref _evt_gatt_notification.
 */
#define EVT_BLUE_GATT_NOTIFICATION		  (0x0C0F)
typedef __packed struct _evt_gatt_notification{
  uint16_t conn_handle;					/**< The connection handle related to the event. */
  uint8_t  event_data_length; 			/**< Length of following data. */
  uint16_t attr_handle;					/**< The handle of the attribute. */
  uint8_t  attr_value[VARIABLE_SIZE]; 	/**< The current value of the attribute. */
} PACKED evt_gatt_attr_notification;

/**
 * This event is generated when a GATT client procedure completes either with error or successfully.
 * See @ref _evt_gatt_procedure_complete.
 */
#define EVT_BLUE_GATT_PROCEDURE_COMPLETE          (0x0C10)
typedef __packed struct _evt_gatt_procedure_complete{
  uint16_t conn_handle; /**< The connection handle on which the GATT procedure has completed */
  uint8_t  data_length; /**< Length of error_code field (always 1). */
  /**
   * Indicates whether the procedure completed with error (BLE_STATUS_FAILED) or was successful (BLE_STATUS_SUCCESS).
   */
  uint8_t  error_code;
} PACKED evt_gatt_procedure_complete;

/**
 * This event is generated when an Error Response is received from the server. The error response can be given
 * by the server at the end of one of the GATT discovery procedures. This does not mean that the procedure ended
 * with an error, but this error event is part of the procedure itself. See @ref _evt_gatt_error_resp.
 */
#define EVT_BLUE_GATT_ERROR_RESP                  (0x0C11)
typedef __packed struct _evt_gatt_error_resp{
  uint16_t conn_handle;			/**< The connection handle related to the event. */
  uint8_t  event_data_length;	/**< Length of following data. */
  uint8_t  req_opcode;			/**< The request that generated this error response. */
  uint16_t attr_handle;			/**< The attribute handle that generated this error response. */
  uint8_t  error_code;			/**< The reason why the request has generated an error response. See Error Response in Bluetooth Core v4.0 spec.  */
} PACKED evt_gatt_error_resp;

/**
 * This event can be generated during a "Discover Characteristics By UUID" procedure or a "Read using
 * Characteristic UUID" procedure.
 * The attribute value will be a service declaration as defined in Bluetooth Core v4.0 spec (vol.3, Part G, ch. 3.3.1),
 * when a "Discover Characteristics By UUID" has been started. It will be the value of the Characteristic if a
 * "Read using Characteristic UUID" has been performed. See @ref _evt_gatt_disc_read_char_by_uuid_resp.
 */
#define EVT_BLUE_GATT_DISC_READ_CHAR_BY_UUID_RESP (0x0C12)
typedef __packed struct _evt_gatt_disc_read_char_by_uuid_resp{
  uint16_t conn_handle;						/**< The connection handle related to the event. */
  uint8_t  event_data_length;				/**< Length of following data. */
  uint16_t attr_handle;						/**< The handle of the attribute. */
  /**
   * The attribute value will be a service declaration as defined in Bluetooth Core v4.0 spec (vol.3, Part G, ch. 3.3.1),
   * when a "Discover Characteristics By UUID" has been started. It will be the value of the Characteristic if a
   * "Read using Characteristic UUID" has been performed.
   */
  uint8_t  attr_value[VARIABLE_SIZE];
} PACKED evt_gatt_disc_read_char_by_uuid_resp;

/**
 * This event is given to the application when a write request, write command or signed write command
 * is received by the server from the client. This event will be given to the application only if the
 * event bit for this event generation is set when the characteristic was added.
 * When this event is received, the application has to check whether the value being requested for write
 * is allowed to be written and respond with the command aci_gatt_write_response().
 * If the write is rejected by the application, then the value of the attribute will not be modified.
 * In case of a write request, an error response will be sent to the client, with the error code as specified by the application.
 * In case of write/signed write commands, no response is sent to the client but the attribute is not modified.
 * See @ref evt_gatt_write_permit_req.
 */
#define EVT_BLUE_GATT_WRITE_PERMIT_REQ            (0x0C13)
typedef __packed struct _evt_gatt_write_permit_req{
  uint16_t conn_handle; /**< Handle of the connection on which there was the request to write the attribute. */
  uint16_t attr_handle; /**< The handle of the attribute for which the write request has been made by the client */
  uint8_t  data_length; /**< Length of data field. */
  uint8_t  data[VARIABLE_SIZE]; /**< The data that the client has requested to write */
} PACKED evt_gatt_write_permit_req;

/**
 * This event is given to the application when a read request or read blob request is received by the server
 * from the client. This event will be given to the application only if the event bit for this event generation
 * is set when the characteristic was added.
 * On receiving this event, the application can update the value of the handle if it desires and when done
 * it has to use the aci_gatt_allow_read() command to indicate to the stack that it can send the response to the client.
 * See @ref evt_gatt_read_permit_req.
 *
 */
#define EVT_BLUE_GATT_READ_PERMIT_REQ             (0x0C14)
typedef __packed struct _evt_gatt_read_permit_req{
  uint16_t conn_handle; /**< Handle of the connection on which there was the request to read the attribute. */
  uint16_t attr_handle; /**< The handle of the attribute for which the read request has been made by the client */
  uint8_t  data_length; /**< Length of offset field. (always 1). */
  uint8_t  offset;      /**< Contains the offset from which the read has been requested */
} PACKED evt_gatt_read_permit_req;

/**
 * This event is given to the application when a read multiple request or read by type request is received
 * by the server from the client. This event will be given to the application only if the event bit for this
 * event generation is set when the characteristic was added.
 * On receiving this event, the application can update the values of the handles if it desires and when done
 * it has to send the aci_gatt_allow_read command to indicate to the stack that it can send the response to the client.
 * See @ref evt_gatt_read_multi_permit_req.
 *
 */
#define EVT_BLUE_GATT_READ_MULTI_PERMIT_REQ       (0x0C15)
typedef __packed struct _evt_gatt_read_multi_permit_req{
  uint16_t conn_handle; /**< Handle of the connection on which there was the request to read the attribute. */
  uint8_t  data_length; /**< Length of data field. */
  uint8_t  data[VARIABLE_SIZE]; /**< The handles of the attributes that have been requested by the client for a read. */
} PACKED evt_gatt_read_multi_permit_req;

#endif // __EVENTS_H