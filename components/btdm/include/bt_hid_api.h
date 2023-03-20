#ifndef _BT_HID_API_H
#define _BT_HID_API_H

/*---------------------------------------------------------------------------
 * SdpErrorCode type
 *
 *     Indicates the type of error that caused the query to fail
 *     on the remote device. This is expressed as a parameter in
 *     a BSQR_ERROR_RESP response.
 */
typedef unsigned short SdpErrorCode;

#define BSEC_BAD_VERSION      0x0001 /* Invalid/unsupported SDP version */
#define BSEC_BAD_HANDLE       0x0002 /* Invalid service record handle */
#define BSEC_BAD_SYNTAX       0x0003 /* Invalid request syntax */
#define BSEC_BAD_PDU_SIZE     0x0004 /* Invalid PDU size */
#define BSEC_BAD_CONTINUATION 0x0005 /* Invalid ContinuationState */
#define BSEC_OUT_OF_RESOURCES 0x0006 /* Insufficient resources to satisfy
                                      * request */
/* End of SdpErrorCode */

/*---------------------------------------------------------------------------
 * HidEvent type
 *
 *     All event indications and confirmations are passed to a callback
 *     function of type HidCallback. The "event" field of the "HidCallbackParms"
 *     parameter will contain one of the event types below.  The "ptrs" field
 *     will contain information pertinent to each event.
 */
typedef uint8_t HidEvent;

/** A remote device (host or HID device) has requested a connection.  During the
 *  processing of this event, call HID_AcceptConnection or HID_RejectConnection
 *  to indicate whether the connection should be allowed.  The "ptrs.remDev"
 *  field contains a pointer to the remote device.
 */
#define HIDEVENT_OPEN_IND                1

/** A connection is open.  This event can be received as the result of a call to 
 *  HID_AcceptConnection or HID_OpenConnection.  When this event has been 
 *  received, the channel is available for sending transactions and interrupts.  
 *  When this event is received in the callback, the "ptrs.remDev" field 
 *  contains a pointer to the remote device.
 */
#define HIDEVENT_OPEN                    2

/** The remote device (host or HID device) is closing the connection.  Once the 
 *  connection is closed, an HIDEVENT_CLOSED event will be received.  When 
 *  this event is received in the callback, the "ptrs.remDev" field contains a 
 *  pointer to the remote device.
 */
#define HIDEVENT_CLOSE_IND               3

/** The connection is closed.  This can come as a result of calling 
 *  HID_CloseConnection, if the remote device has closed the connection, 
 *  or if an incoming connection is rejected.  Transactions and interrupts 
 *  cannot be sent or received in this state.  When this event is received, 
 *  the "ptrs.remDev" field may contain a pointer to the remote device.
 */
#define HIDEVENT_CLOSED                  4

/** A query response has been received from the HID device.  This event is 
 *  received as the result of a call to HID_HostQueryDevice.  The 
 *  "ptrs.queryRsp" field contains a pointer to the query response.  This
 *  structure contains important information about the device.  It is possible
 *  that the query will fail.  The "status" field contains a value of 
 *  BT_STATUS_SUCCESS when the query is successful.
 */
#define HIDEVENT_QUERY_CNF               5

/** A transaction has been received.  The "ptrs.trans" field contains a pointer
 *  to an "HidTransaction" structure.  The "type" field of this structure
 *  defines the type of transaction that has been received.  Based on the 
 *  value of the "type" field, the "parm" field of the transaction structure
 *  will point to the parameters of the transaction.  See HidTransactionType for
 *  more information about the transaction.  If the "status" field has a value
 *  of BT_STATUS_PENDING, then more data is pending on this event and 
 *  subsequent HIDEVENT_TRANSACTION_IND events will be received.  When "status" 
 *  has a value of BT_STATUS_SUCCESS, all data from the transaction has been
 *  received.
 */
#define HIDEVENT_TRANSACTION_IND         6

/** A transaction response has been received.  The "ptrs.trans" field contains
 *  a pointer to a "HidTransaction" structure.  The "type" field of the transaction
 *  structure defines the type of transaction.  Based on the value of the "type"
 *  field, the "parm" field of the transaction structure will point to the
 *  relevant parameters of the transaction.  See HidTransactionType for
 *  more information about the transaction.  If the "status" field has a value
 *  of BT_STATUS_PENDING, then more data is pending on this event and 
 *  subsequent HIDEVENT_TRANSACTION_RSP events will be received.  When "status" 
 *  has a value of BT_STATUS_SUCCESS, all data from the transaction has been
 *  received.
 */
#define HIDEVENT_TRANSACTION_RSP         7

/** A call to a transaction API has completed.  The "ptrs.trans" field contains
 *  a pointer to the transaction that was sent.  When this event is received,
 *  the transaction structure is now available for reuse.  The "status" field
 *  indicates whether the transaction was sent successfully. If successful, the
 *  "resultCode" field of the transaction structure is valid and contains the
 *  result of the transaction.
 */
#define HIDEVENT_TRANSACTION_COMPLETE    8

/** An interrupt was received from the remote Device.  The "ptrs.intrData" field
 *  contains the interrupt data.  Interrupt data should be handled in a timely
 *  manner, because it may be overwritten by subsequent interrupts.  If the
 *  "status" field has a value of BT_STATUS_PENDING, then more interrupt data
 *  is pending on this event and  subsequent HIDEVENT_INTERRUPT events will be
 *  received.  When "status" has a value of BT_STATUS_SUCCESS, all data from the
 *  interrupt has been received.
 */
#define HIDEVENT_INTERRUPT               9

/** A call to the HID_SendInterrupt API has been sent.  The "ptrs.intrData"
 *  field contains the interrupt data that was sent, and the structure is
 *  available for reuse.
 */
#define HIDEVENT_INTERRUPT_COMPLETE      10

/* End of HidEvent */

/*---------------------------------------------------------------------------
 * HidTransactionType type
 *
 *     When a transaction is received (request or response),  a transaction
 *     structure is passed to the application to indicate the type of
 *     transaction and its parameters.  The "Type" field of the transaction
 *     structure will be set to one of the following values, and will indicate
 *     which parameters are valid.
 */
typedef uint8_t HidTransactionType;

/** A control request has been received.  The "parm.control" field of the
 *  "HidTransaction" structure contains the current control operation.  Control
 *  operations should be executed by the receiving device.  The only valid
 *  operation that can be received by the Host is a
 *  HID_CTRL_VIRTUAL_CABLE_UNPLUG.  All operations can be received by the 
 *  HID Device.
 */
#define HID_TRANS_CONTROL           1

/** A request for a report has been received.  The "parm.reportReq" field
 *  contains the request.  The receiving device should respond to a  request by
 *  calling HID_DeviceGetReportRsp.  When an input report is requested, the
 *  device should respond with the instantaneous state of fields in the
 *  requested input report.  When an output report is requested, the device 
 *  should respond with the last output report received on the interrupt
 *  channel.  If no output report has been received, default values should be
 *  returned. If a feature report is requested, the device should return 
 *  default values or the instantaneous state of the feature report fields.
 */
#define HID_TRANS_GET_REPORT        2

/** A report has been received.  The "parm.report" field contains the report. 
 *  Multiple HIDEVENT_TRANSACTION_IND events may be received with report
 *  data.  If the "status" field of the "HidCallbackParms" structure contains
 *  a value of BT_STATUS_PENDING, more transaction events will be received.
 *  When a value of BT_STATUS_SUCCESS is received, all report data has been
 *  received.  When all report data has been received, the receiving device
 *  should respond to this request by calling HID_DeviceSetReportRsp.
 */
#define HID_TRANS_SET_REPORT        3

/** A request for the current protocol has been received.  The receiving device
 *  should respond to this request by calling HID_DeviceGetProtocolRsp.
 */
#define HID_TRANS_GET_PROTOCOL      4

/** A set protocol command has been received.  The "parm.protocol" field
 *  contains the current protocol.  The receiving device should respond to this
 *  request by calling HID_DeviceSetProtocolRsp.  If a successful response is
 *  sent, the device should begin to use the specified protocol.
 */
#define HID_TRANS_SET_PROTOCOL      5

/** A request for the current idle rate has been received.  The  receiving
 *  device should respond to this request by calling  HID_DeviceGetIdleRsp.
 *  No parameter is specified.
 */
#define HID_TRANS_GET_IDLE_RATE     6

/** A set idle request has been received.  The "parm.idleRate" field contains
 *  the current idle rate.  The device should respond to this request by calling
 *  HID_DeviceSetIdleRsp.  If a successful response is sent, the device
 *  should begin to use the specified idle rate.
 */
#define HID_TRANS_SET_IDLE_RATE     7

/** A response to a HID_TRANS_GET_REPORT transaction has been received.  
 *  If the "resultCode" field is set to HID_RESULT_SUCCESS,  then the
 *  "parm.report" field contains a pointer to  the report data.
 *  Multiple HIDEVENT_TRANSACTION_IND events may be received with report
 *  data.  If the "status" field of the "HidCallbackParms" structure contains
 *  a value of BT_STATUS_PENDING, more transaction events will be received.
 *  When a value of BT_STATUS_SUCCESS is received, all report data has been
 *  received.  
 */
#define HID_TRANS_GET_REPORT_RSP    8

/** A response to a HID_TRANS_SET_REPORT transaction has been received.  
 *  The "resultCode" field contains the results of the transaction.
 */
#define HID_TRANS_SET_REPORT_RSP    9

/** A response to a HID_TRANS_GET_PROTOCOL transaction has been received.
 *  If the "resultCode" field is set to HID_RESULT_SUCCESS, then the
 *  "parm.protocol" field contains the current protocol.
 */
#define HID_TRANS_GET_PROTOCOL_RSP  10

/** A response to a HID_TRANS_SET_PROTOCOL transaction has been received.
 *  The "resultCode" field contains the results of the transaction.
 */
#define HID_TRANS_SET_PROTOCOL_RSP  11

/** A response to a HID_TRANS_GET_IDLE_RATE transaction has been received.
 *  If the "resultCode" is set to HID_RESULT_SUCCESS, then the
 *  "parm.idleRate" field contains the current idle rate.
 */
#define HID_TRANS_GET_IDLE_RATE_RSP      12

/** A response to a HID_TRANS_SET_IDLE_RATE transaction has been received.  
 *  The "resultCode" field contains the results of the transaction.
 */
#define HID_TRANS_SET_IDLE_RATE_RSP      13

/* End of HidTransactionType */

/*---------------------------------------------------------------------------
 * HidResultCode type
 *
 * HID transactions return a status describing the success or failure of the
 * transaction.  This type describes all the possible result codes.
 */
typedef uint8_t HidResultCode;

#define HID_RESULT_SUCCESS              0
#define HID_RESULT_NOT_READY            1
#define HID_RESULT_INVALID_REPORT_ID    2
#define HID_RESULT_UNSUPPORTED_REQUEST  3
#define HID_RESULT_INVALID_PARAMETER    4
#define HID_RESULT_UNKNOWN              5
#define HID_RESULT_FATAL                6

/* End of HidResultCode */

/*---------------------------------------------------------------------------
 * HidIdleRate type
 *
 * This type defines the idle rate (required by keyboards).
 */
typedef uint8_t HidIdleRate;

/* End of HidIdleRate */

/*---------------------------------------------------------------------------
 * HidProtocol type
 *
 * This type defines the HID protocols.
 */
typedef uint8_t HidProtocol;

#define HID_PROTOCOL_REPORT  1
#define HID_PROTOCOL_BOOT    0

/* End of HidProtocol */

/*---------------------------------------------------------------------------
 * HidReportType type
 *
 * This defines the HID report type.
 */
typedef uint8_t HidReportType;

#define HID_REPORT_OTHER    0
#define HID_REPORT_INPUT    1
#define HID_REPORT_OUTPUT   2
#define HID_REPORT_FEATURE  3

/* End of HidReportType */

/*---------------------------------------------------------------------------
 * HidControl type
 *
 * This defines the control type.
 */
typedef uint8_t HidControl;

#define HID_CTRL_NOP                   0
#define HID_CTRL_HARD_RESET            1
#define HID_CTRL_SOFT_RESET            2
#define HID_CTRL_SUSPEND               3
#define HID_CTRL_EXIT_SUSPEND          4
#define HID_CTRL_VIRTUAL_CABLE_UNPLUG  5

/* End of HidControl */

/*---------------------------------------------------------------------------
 * HidQueryFlags type
 *
 * These flags are used to determine which values of the SDP query response
 * contain valid data (see HidQueryRsp).  Some fields may not be supported in
 * a particular device.  If no flags are set at all, then a valid query has
 * not been made.
 */
typedef uint16_t HidQueryFlags;

#define SDPQ_FLAG_DEVICE_RELEASE    0x0001    /* deviceRelease is valid */
#define SDPQ_FLAG_PARSER_VERSION    0x0002    /* parserVersion is valid */
#define SDPQ_FLAG_DEVICE_SUBCLASS   0x0004    /* deviceSubclass is valid */
#define SDPQ_FLAG_COUNTRY_CODE      0x0008    /* countryCode is valid */
#define SDPQ_FLAG_VIRTUAL_CABLE     0x0010    /* virtualCable is valid */
#define SDPQ_FLAG_RECONNECT_INIT    0x0020    /* reconnect is valid */
#define SDPQ_FLAG_DESCRIPTOR_LIST   0x0040    /* descriptorLen and descriptorList
                                               * are valid.
                                               */
#define SDPQ_FLAG_SDP_DISABLE       0x0080    /* sdpDisable is valid */
#define SDPQ_FLAG_BATTERY_POWER     0x0100    /* batteryPower is valid */
#define SDPQ_FLAG_REMOTE_WAKE       0x0200    /* remoteWakeup is valid */
#define SDPQ_FLAG_PROFILE_VERSION   0x0400    /* profileVersion is valid */
#define SDPQ_FLAG_SUPERV_TIMEOUT    0x0800    /* supervTimeout is valid */
#define SDPQ_FLAG_NORM_CONNECTABLE  0x1000    /* normConnectable is valid */
#define SDPQ_FLAG_BOOT_DEVICE       0x2000    /* bootDevice is valid */

/* End of HidQueryFlags */

/* HID roles */
typedef uint8_t HidRole;

#define HID_ROLE_DEVICE  1
#define HID_ROLE_HOST    2

/* Foreward References */
typedef struct _HidCallbackParms HidCallbackParms;
typedef struct _HidChannel HidChannel;
typedef struct _HidReport HidReport;
typedef struct _HidReportReq HidReportReq;
typedef struct _HidTransaction HidTransaction;
typedef struct _HidInterrupt HidInterrupt;


/*---------------------------------------------------------------------------
 * SdpDataElemType type
 *
 *     Specifies the type of a Data Element.
 *
 *     Data Elements begin with a single byte that contains both type and
 *     size information. To read the type from this byte, use the
 *     SDP_GetElemType macro.
 *
 *     To create the first byte of a Data Element, bitwise OR the
 *     SdpDataElemType and SdpDataElemSize values into a single byte.
 */
typedef unsigned char SdpDataElemType;

#define DETD_NIL  0x00 /* Specifies nil, the null type.
                        * Requires a size of DESD_1BYTE, which for this type
                        * means an actual size of 0.
                        */
#define DETD_UINT 0x08 /* Specifies an unsigned integer. Must use size
                        * DESD_1BYTE, DESD_2BYTES, DESD_4BYTES, DESD_8BYTES,
                        * or DESD_16BYTES.
                        */
#define DETD_SINT 0x10 /* Specifies a signed integer. May use size
                        * DESD_1BYTE, DESD_2BYTES, DESD_4BYTES, DESD_8BYTES,
                        * or DESD_16BYTES.
                        */
#define DETD_UUID 0x18 /* Specifies a Universally Unique Identifier (UUID).
                        * Must use size DESD_2BYTES, DESD_4BYTES, or
                        * DESD_16BYTES.
                        */
#define DETD_TEXT 0x20 /* Specifies a text string. Must use sizes
                        * DESD_ADD_8BITS, DESD_ADD_16BITS, or DESD_ADD32_BITS.
                        */
#define DETD_BOOL 0x28 /* Specifies a boolean value. Must use size
                        * DESD_1BYTE.
                        */
#define DETD_SEQ  0x30 /* Specifies a data element sequence. The data contains
                        * a sequence of Data Elements. Must use sizes
                        * DESD_ADD_8BITS, DESD_ADD_16BITS, or DESD_ADD_32BITS.
                        */
#define DETD_ALT  0x38 /* Specifies a data element alternative. The data contains
                        * a sequence of Data Elements. This type is sometimes
                        * used to distinguish between two possible sequences.
                        * Must use size DESD_ADD_8BITS, DESD_ADD_16BITS,
                        * or DESD_ADD_32BITS.
                        */
#define DETD_URL  0x40 /* Specifies a Uniform Resource Locator (URL).
                        * Must use size DESD_ADD_8BITS, DESD_ADD_16BITS,
                        * or DESD_ADD_32BITS.
                        */

#define DETD_MASK 0xf8 /* AND this value with the first byte of a Data
                        * Element to return the element's type.
                        */

/* End of SdpDataElemType */
                        
/*---------------------------------------------------------------------------
 * SdpDataElemSize type
 *
 *     Specifies the size of a Data Element.
 *
 *     Data Elements begin with a single byte that contains both type and
 *     size information. To read the size from this byte, use the
 *     SDP_GetElemSize macro.
 *
 *     To create the first byte of a Data Element, bitwise OR the
 *     SdpDataElemType and SdpDataElemSize values into a single byte.
 *     For example, a standard 16 bit unsigned integer with a value of 0x57
 *     could be encoded as follows:
 * 
 *     unsigned char val[3] = { DETD_UINT | DESD_2BYTES, 0x00, 0x57 };
 *
 *     The text string "hello" could be encoded as follows:
 *
 *     unsigned char str[7] = { DETD_TEXT | DESD_ADD_8BITS, 0x05, 'h','e','l','l','o' };
 */
typedef unsigned char SdpDataElemSize;

#define DESD_1BYTE      0x00 /* Specifies a 1-byte element. However, if
                              * type is DETD_NIL then the size is 0.
                              */
#define DESD_2BYTES     0x01 /* Specifies a 2-byte element. */
#define DESD_4BYTES     0x02 /* Specifies a 4-byte element. */
#define DESD_8BYTES     0x03 /* Specifies an 8-byte element. */
#define DESD_16BYTES    0x04 /* Specifies a 16-byte element. */
#define DESD_ADD_8BITS  0x05 /* The element's actual data size, in bytes,
                              * is contained in the next 8 bits.
                              */
#define DESD_ADD_16BITS 0x06 /* The element's actual data size, in bytes,
                              * is contained in the next 16 bits.
                              */
#define DESD_ADD_32BITS 0x07 /* The element's actual data size, in bytes,
                              * is contained in the next 32 bits.
                              */

#define DESD_MASK       0x07 /* AND this value with the first byte of a Data
                              * Element to return the element's size.
                              */

/* End of SdpDataElemSize */

#define AID_SERVICE_CLASS_ID_LIST               0x0001

/* Group: The following attributes are "universal" to all service records,
 * meaning that the same attribute IDs are always used. However, attributes
 * may or may not be present within a service record.
 *
 * See the Bluetooth Core specification, Service Discovery Protocol (SDP)
 * chapter, section 5.1 for more detailed explanations of these attributes.
 */

#define AID_SERVICE_RECORD_STATE                0x0002
#define AID_SERVICE_ID                          0x0003
#define AID_PROTOCOL_DESC_LIST                  0x0004
#define AID_BROWSE_GROUP_LIST                   0x0005
#define AID_LANG_BASE_ID_LIST                   0x0006
#define AID_SERVICE_INFO_TIME_TO_LIVE           0x0007
#define AID_SERVICE_AVAILABILITY                0x0008
#define AID_BT_PROFILE_DESC_LIST                0x0009
#define AID_DOC_URL                             0x000a
#define AID_CLIENT_EXEC_URL                     0x000b
#define AID_ICON_URL                            0x000c
#define AID_ADDITIONAL_PROT_DESC_LISTS          0x000d

/* Group: The following "universal" attribute IDs must be added to
 * the appropriate value from the AID_LANG_BASE_ID_LIST attribute (usually 
 * 0x0100).
 */
#define AID_SERVICE_NAME                        0x0000
#define AID_SERVICE_DESCRIPTION                 0x0001
#define AID_PROVIDER_NAME                       0x0002

/* Personal Area Networking Profile */
#define AID_IP_SUBNET                           0x0200

/* Group: The following attribute applies only to a service record that
 * corresponds to a BrowseGroupDescriptor service.
 */

/* A UUID used to locate services that are part of the browse group. */
#define AID_GROUP_ID                            0x0200

/* Group: The following attributes apply only to the service record that
 * corresponds to the Service Discovery Server itself. Therefore, they
 * are valid only when the AID_SERVICE_CLASS_ID_LIST contains
 * a UUID of SC_SERVICE_DISCOVERY_SERVER.
 */
#define AID_VERSION_NUMBER_LIST                 0x0200
#define AID_SERVICE_DATABASE_STATE              0x0201

/* Group: The following attributes are for use by specific profiles as
 * defined in the profile specification.
 */
#define AID_SERVICE_VERSION                     0x0300

/* Cordless Telephony Profile */
#define AID_EXTERNAL_NETWORK                    0x0301

/* Synchronization Profile */
#define AID_SUPPORTED_DATA_STORES_LIST          0x0301

/* Fax Class 1 */
#define AID_FAX_CLASS_1_SUPPORT                 0x0302

/* GAP Profile */
#define AID_REMOTE_AUDIO_VOL_CONTROL            0x0302

/* Fax Class 2.0 */
#define AID_FAX_CLASS_20_SUPPORT                0x0303

/* Object Push Profile */
#define AID_SUPPORTED_FORMATS_LIST              0x0303

/* Fax Service Class 2 - Manufacturer specific */
#define AID_FAX_CLASS_2_SUPPORT                 0x0304
#define AID_AUDIO_FEEDBACK_SUPPORT              0x0305

/* Bluetooth as WAP requirements */
#define AID_NETWORK_ADDRESS                     0x0306
#define AID_WAP_GATEWAY                         0x0307
#define AID_HOME_PAGE_URL                       0x0308
#define AID_WAP_STACK_TYPE                      0x0309

/* Personal Area Networking Profile */
#define AID_SECURITY_DESC                       0x030A
#define AID_NET_ACCESS_TYPE                     0x030B
#define AID_MAX_NET_ACCESS_RATE                 0x030C
#define AID_IPV4_SUBNET                         0x030D
#define AID_IPV6_SUBNET                         0x030E

/* Imaging Profile */
#define AID_SUPPORTED_CAPABILITIES              0x0310
#define AID_SUPPORTED_FEATURES                  0x0311
#define AID_SUPPORTED_FUNCTIONS                 0x0312
#define AID_TOTAL_IMAGE_DATA_CAPACITY           0x0313

/* Phonebook Access Profile */
#define AID_SUPPORTED_REPOSITORIES              0x0314

/* Message Access Profile */
#define AID_MAS_INSTANCE_ID                     0x0315
#define AID_SUPPORTED_MESSAGE_TYPES             0x0316

/* Basic Printing Profile */
#define AID_SUPPORTED_DOC_FORMATS               0x0350
#define AID_SUPPORTED_CHAR_REPERTOIRES          0x0352
#define AID_SUPPORTED_XHTML_IMAGE_FORMATS       0x0354
#define AID_COLOR_SUPPORTED                     0x0356
#define AID_PRINTER_1284ID                      0x0358
#define AID_DUPLEX_SUPPORTED                    0x035E
#define AID_SUPPORTED_MEDIA_TYPES               0x0360
#define AID_MAX_MEDIA_WIDTH                     0x0362
#define AID_MAX_MEDIA_LENGTH                    0x0364

/* End of SdpAttributeId */
/*---------------------------------------------------------------------------
 * SdpServiceClassUuid type
 *
 *     Represents the UUID associated with a specific service and
 *     profile.
 *
 *     Any number of these UUIDs may be present in the
 *     AID_SERVICE_CLASS_ID_LIST attribute of a service record, and may
 *     appear in the AID_BT_PROFILE_DESC_LIST.
 */
typedef unsigned short SdpServiceClassUuid;

/* Service Discovery Server service. */
#define SC_SERVICE_DISCOVERY_SERVER             0x1000

/* Browse Group Descriptor service. */
#define SC_BROWSE_GROUP_DESC                    0x1001

/* Public Browse Group service. */
#define SC_PUBLIC_BROWSE_GROUP                  0x1002

/* Serial Port service and profile. */
#define SC_SERIAL_PORT                          0x1101

/* LAN Access over PPP service. */
#define SC_LAN_ACCESS_PPP                       0x1102

/* Dial-up networking service and profile. */
#define SC_DIALUP_NETWORKING                    0x1103

/* IrMC Sync service and Synchronization profile. */
#define SC_IRMC_SYNC                            0x1104

/* OBEX Object Push service and Object Push profile. */
#define SC_OBEX_OBJECT_PUSH                     0x1105

/* OBEX File Transfer service and File Transfer profile. */
#define SC_OBEX_FILE_TRANSFER                   0x1106

/* IrMC Sync service and Synchronization profile
 * (Sync Command Scenario).
 */
#define SC_IRMC_SYNC_COMMAND                    0x1107

/* Headset service and profile. */
#define SC_HEADSET                              0x1108

/* Cordless telephony service and profile. */
#define SC_CORDLESS_TELEPHONY                   0x1109

/* Audio Source */
#define SC_AUDIO_SOURCE                         0x110A

/* Audio Sink */
#define SC_AUDIO_SINK                           0x110B

/* Audio/Video Remote Control Target */
#define SC_AV_REMOTE_CONTROL_TARGET             0x110C

/* Advanced Audio Distribution Profile */
#define SC_AUDIO_DISTRIBUTION                   0x110D

/* Audio/Video Remote Control */
#define SC_AV_REMOTE_CONTROL                    0x110E

/* Video Conferencing Profile */
#define SC_VIDEO_CONFERENCING                   0x110F

/* Intercom service and profile. */
#define SC_INTERCOM                             0x1110

/* Fax service and profile. */
#define SC_FAX                                  0x1111

/* Headset Audio Gateway */
#define SC_HEADSET_AUDIO_GATEWAY                0x1112

/* WAP service */
#define SC_WAP                                  0x1113

/* WAP client service */
#define SC_WAP_CLIENT                           0x1114

/* Personal Area Networking Profile */
#define SC_PANU                                 0x1115

/* Personal Area Networking Profile */
#define SC_NAP                                  0x1116

/* Personal Area Networking Profile */
#define SC_GN                                   0x1117

/* Basic Printing Profile */
#define SC_DIRECT_PRINTING                      0x1118

/* Basic Printing Profile */
#define SC_REFERENCE_PRINTING                   0x1119
    
/* Imaging Profile */
#define SC_IMAGING                              0x111A

/* Imaging Profile */
#define SC_IMAGING_RESPONDER                    0x111B

/* Imaging Profile */
#define SC_IMAGING_AUTOMATIC_ARCHIVE            0x111C

/* Imaging Profile */
#define SC_IMAGING_REFERENCED_OBJECTS           0x111D

/* Handsfree Profile */
#define SC_HANDSFREE                            0x111E

/* Handsfree Audio Gateway */
#define SC_HANDSFREE_AUDIO_GATEWAY              0x111F

/* Basic Printing Profile */
#define SC_DIRECT_PRINTING_REF_OBJECTS          0x1120

/* Basic Printing Profile */
#define SC_REFLECTED_UI                         0x1121

/* Basic Printing Profile */
#define SC_BASIC_PRINTING                       0x1122

/* Basic Printing Profile */
#define SC_PRINTING_STATUS                      0x1123

/* Human Interface Device Profile */
#define SC_HUMAN_INTERFACE_DEVICE               0x1124

/* Hardcopy Cable Replacement Profile */
#define SC_HCR                                  0x1125

/* Hardcopy Cable Replacement Profile */
#define SC_HCR_PRINT                            0x1126

/* Hardcopy Cable Replacement Profile */
#define SC_HCR_SCAN                             0x1127

/* Common ISDN Access / CAPI Message Transport Protocol */
#define SC_ISDN                                 0x1128

/* Video Conferencing Gateway */
#define SC_VIDEO_CONFERENCING_GW                0x1129

/* Unrestricted Digital Information Mobile Termination */
#define SC_UDI_MT                               0x112A

/* Unrestricted Digital Information Terminal Adapter */
#define SC_UDI_TA                               0x112B

/* Audio Video service */
#define SC_AUDIO_VIDEO                          0x112C

/* SIM Access Profile */
#define SC_SIM_ACCESS                           0x112D

/* Phonebook Access Client */
#define SC_PBAP_CLIENT                          0x112E

/* Phonebook Access Server */
#define SC_PBAP_SERVER                          0x112F

/* Phonebook Access Profile Id */
#define SC_PBAP_PROFILE                         0x1130

/* Message Access Server */
#define SC_MAP_SERVER                           0x1132

/* Message Access Notification Server */
#define SC_MAP_NOTIFY_SERVER                    0x1133

/* Message Access Profile */
#define SC_MAP_PROFILE                          0x1134

/* Plug-n-Play service */
#define SC_PNP_INFO                             0x1200

/* Generic Networking service. */
#define SC_GENERIC_NETWORKING                   0x1201

/* Generic File Transfer service. */
#define SC_GENERIC_FILE_TRANSFER                0x1202

/* Generic Audio service. */
#define SC_GENERIC_AUDIO                        0x1203

/* Generic Telephony service. */
#define SC_GENERIC_TELEPHONY                    0x1204

/* UPnP L2CAP based profile. */
#define SC_UPNP_SERVICE                         0x1205

/* UPnP IP based profile. */
#define SC_UPNP_IP_SERVICE                      0x1206

/* UPnP IP based solution using PAN */
#define SC_ESDP_UPNP_IP_PAN                     0x1300

/* UPnP IP based solution using LAP */
#define SC_ESDP_UPNP_IP_LAP                     0x1301

/* UPnP L2CAP based solution */
#define SC_ESDP_UPNP_L2CAP                      0x1302

/* Video Source */
#define SC_VIDEO_SOURCE                         0x1303

/* Video Sink */
#define SC_VIDEO_SINK                           0x1304

/* Video Sink */
#define SC_VIDEO_DISTRIBUTION                   0x1305

/* End of SdpServiceClassUuid */

/*---------------------------------------------------------------------------
 * SdpProtocolUuid type
 *
 *     Represents the UUID associated with a protocol.
 *
 *     Any number of these UUIDs may be present in the
 *     AID_SERVICE_CLASS_ID_LIST attribute of a service record, and may
 *     appear in the AID_BT_PROFILE_DESC_LIST.
 */
typedef unsigned short SdpProtocolUuid;

/* Service Discovery Protocol */
#define PROT_SDP                     0x0001

/* UDP Protocol */
#define PROT_UDP                     0x0002

/* RFCOMM Protocol */
#define PROT_RFCOMM                  0x0003

/* TCP Protocol */
#define PROT_TCP                     0x0004

/* TCS Binary Protocol */
#define PROT_TCS_BIN                 0x0005

/* TCS-AT Protocol */
#define PROT_TCS_AT                  0x0006

/* OBEX Protocol */
#define PROT_OBEX                    0x0008

/* IP Protocol */
#define PROT_IP                      0x0009

/* FTP Protocol */
#define PROT_FTP                     0x000A

/* HTTP Protocol */
#define PROT_HTTP                    0x000C

/* WSP Protocol */
#define PROT_WSP                     0x000E

/* BNEP Protocol */
#define PROT_BNEP                    0x000F

/* Universal Plug and Play */
#define PROT_UPNP                    0x0010

/* Human Interface Device Profile */
#define PROT_HIDP                    0x0011

/* Hardcopy Cable Replacement Control Channel */
#define PROT_HCR_CONTROL_CHANNEL     0x0012

/* Hardcopy Cable Replacement Data Channel */
#define PROT_HCR_DATA_CHANNEL        0x0014

/* Hardcopy Cable Replacement Notification*/
#define PROT_HCR_NOTIFICATION        0x0016

/* Audio/Video Control Transport Protocol */
#define PROT_AVCTP                   0x0017

/* Audio/Video Distribution Transport Protocol */
#define PROT_AVDTP                   0x0019

/* Audio/Video Control Transport Protocol Browsing Channel*/
#define PROT_AVCTP_BROWSING          0x001B

/* Unrestricted Digital Information Control Plane */
#define PROT_UDI_C                   0x001D

/* L2CAP Protocol */
#define PROT_L2CAP                   0x0100

/* End of SdpProtocolUuid */

/****************************************************************************
 * HID Specific Attribute IDs
 ****************************************************************************/

#define AID_HID_DEVICE_RELEASE        0x0200
#define AID_HID_PARSER_VERSION        0x0201
#define AID_HID_DEVICE_SUBCLASS       0x0202
#define AID_HID_COUNTRY_CODE          0x0203
#define AID_HID_VIRTUAL_CABLE         0x0204
#define AID_HID_RECONNECT_INIT        0x0205
#define AID_HID_DESCRIPTOR_LIST       0x0206
#define AID_HID_LANG_ID_BASE_LIST     0x0207
#define AID_HID_SDP_DISABLE           0x0208
#define AID_HID_BATTERY_POWER         0x0209
#define AID_HID_REMOTE_WAKE           0x020A
#define AID_HID_PROFILE_VERSION       0x020B
#define AID_HID_SUPERV_TIMEOUT        0x020C
#define AID_HID_NORM_CONNECTABLE      0x020D
#define AID_HID_BOOT_DEVICE           0x020E

/****************************************************************************
 * Device ID Specific Attribute IDs
 ****************************************************************************/

#define AID_DEVID_SPEC_ID             0x0200        /* UINT16 */
#define AID_DEVID_VENDOR_ID           0x0201        /* UINT16 */
#define AID_DEVID_PRODUCT_ID          0x0202        /* UINT16 */
#define AID_DEVID_VERSION             0x0203        /* UINT16 */
#define AID_DEVID_PRIMARY_RECORD      0x0204        /* BOOL */
#define AID_DEVID_VENDOR_ID_SRC       0x0205        /* UINT16 */


/*---------------------------------------------------------------------------
 * SDP_ATTRIBUTE()
 *
 *     Macro that formats an SdpAttribute structure using the supplied Attribute ID 
 *     and Attribute value. This macro is very useful when formatting the
 *     SdpAttribute structures used to form the attributes in an SDP Record.
 *
 * Parameters:
 *     attribId - SdpAttributeId value (see the AID_ values).
 *     attrib - Array containing the attribute value.
 */
#define SDP_ATTRIBUTE(attribId, attrib) \
          { attribId,           /* Attribute ID */          \
            sizeof(attrib),     /* Attribute Size */        \
            attrib,             /* Attribute Value */       \
            0x0000 }            /* Flag - For Internal Use */

/*---------------------------------------------------------------------------
 * SDP_ATTRIB_HEADER_8BIT()
 *
 *     Macro that forms a Data Element Sequence header using the supplied 8-bit
 *     size value.  Data Element Sequence headers are used in SDP Record Attributes
 *     and SDP Queries. Notice that this macro only forms the header portion
 *     of the Data Element Sequence. The actual Data Elements within this
 *     sequence will need to be formed using other SDP macros.
 *
 * Parameters:
 *     size - 8-bit size of the Data Element Sequence.
 */
#define SDP_ATTRIB_HEADER_8BIT(size) \
            DETD_SEQ + DESD_ADD_8BITS,      /* Type & size index 0x35 */ \
            size                            /* 8-bit size */

/*---------------------------------------------------------------------------
 * SDP_ATTRIB_HEADER_16BIT()
 *
 *     Macro that forms a Data Element Sequence header using the supplied 16-bit
 *     size value.  Data Element Sequence headers are used in SDP Record Attributes
 *     and SDP Queries. Notice that this macro only forms the header portion
 *     of the Data Element Sequence. The actual Data Elements within this
 *     sequence will need to be formed using other SDP macros.
 *
 * Parameters:
 *     size - 16-bit size of the Data Element Sequence.
 */
#define SDP_ATTRIB_HEADER_16BIT(size) \
            DETD_SEQ + DESD_ADD_16BITS,      /* Type & size index 0x36 */ \
            (unsigned char)(((size) & 0xff00) >> 8),    /* Bits[15:8] of size */     \
            (unsigned char)((size) & 0x00ff)            /* Bits[7:0] of size */

/*---------------------------------------------------------------------------
 * SDP_ATTRIB_HEADER_32BIT()
 *
 *     Macro that forms a Data Element Sequence header using the supplied 32-bit
 *     size value.  Data Element Sequence headers are used in SDP Record Attributes
 *     and SDP Queries. Notice that this macro only forms the header portion
 *     of the Data Element Sequence. The actual Data Elements within this
 *     sequence will need to be formed using other SDP macros.
 *
 * Parameters:
 *     size - 32-bit size of the Data Element Sequence.
 */
#define SDP_ATTRIB_HEADER_32BIT(size) \
            DETD_SEQ + DESD_ADD_32BITS,         /* Type & size index 0x37 */ \
            (unsigned char)(((size) & 0xff000000) >> 24),  /* Bits[32:24] of size */    \
            (unsigned char)(((size) & 0x00ff0000) >> 16),  /* Bits[23:16] of size */    \
            (unsigned char)(((size) & 0x0000ff00) >> 8),   /* Bits[15:8] of size */     \
            (unsigned char)((size) & 0x000000ff)           /* Bits[7:0] of size */

/*---------------------------------------------------------------------------
 * SDP_ATTRIB_HEADER_ALT_8BIT()
 *
 *     Macro that forms a Data Element Sequence Alternative header using the 
 *     supplied 8-bit size value.  Data Element Sequence Alternative headers 
 *     are used in SDP Record Attributes. Notice that this macro only forms 
 *     the header portion of the Data Element Sequence Alternative. The actual 
 *     Data Element Sequences within this alternative will need to be formed 
 *     using other SDP macros.
 *
 * Parameters:
 *     size - 8-bit size of the Data Element Sequence Alternative.
 */
#define SDP_ATTRIB_HEADER_ALT_8BIT(size) \
            DETD_ALT + DESD_ADD_8BITS,      /* Type & size index 0x35 */ \
            size                            /* 8-bit size */

/*---------------------------------------------------------------------------
 * SDP_ATTRIB_HEADER_ALT_16BIT()
 *
 *     Macro that forms a Data Element Sequence Alternative header using the 
 *     supplied 16-bit size value.  Data Element Sequence Alternative headers 
 *     are used in SDP Record Attributes. Notice that this macro only forms 
 *     the header portion of the Data Element Sequence Alternative. The actual 
 *     Data Element Sequences within this alternative will need to be formed 
 *     using other SDP macros.
 *
 * Parameters:
 *     size - 16-bit size of the Data Element Sequence Alternative.
 */
#define SDP_ATTRIB_HEADER_ALT_16BIT(size) \
            DETD_ALT + DESD_ADD_16BITS,      /* Type & size index 0x36 */ \
            (unsigned char)(((size) & 0xff00) >> 8),    /* Bits[15:8] of size */     \
            (unsigned char)((size) & 0x00ff)            /* Bits[7:0] of size */

/*---------------------------------------------------------------------------
 * SDP_ATTRIB_HEADER_ALT_32BIT()
 *
 *     Macro that forms a Data Element Sequence Alternative header using the 
 *     supplied 32-bit size value.  Data Element Sequence Alternative headers 
 *     are used in SDP Record Attributes. Notice that this macro only forms 
 *     the header portion of the Data Element Sequence Alternative. The actual 
 *     Data Element Sequences within this alternative will need to be formed 
 *     using other SDP macros.
 *
 * Parameters:
 *     size - 32-bit size of the Data Element Sequence Alternative.
 */
#define SDP_ATTRIB_HEADER_ALT_32BIT(size) \
            DETD_ALT + DESD_ADD_32BITS,         /* Type & size index 0x37 */ \
            (unsigned char)(((size) & 0xff000000) >> 24),  /* Bits[32:24] of size */    \
            (unsigned char)(((size) & 0x00ff0000) >> 16),  /* Bits[23:16] of size */    \
            (unsigned char)(((size) & 0x0000ff00) >> 8),   /* Bits[15:8] of size */     \
            (unsigned char)((size) & 0x000000ff)           /* Bits[7:0] of size */

/*---------------------------------------------------------------------------
 * SDP_UUID_16BIT()
 *
 *     Macro that forms a UUID Data Element from the supplied 16-bit UUID value. 
 *     UUID data elements are used in SDP Record Attributes and SDP Queries.
 *
 * Parameters:
 *     uuid - 16-bit UUID value (see the SC_ and PROT_ values).
 */
#define SDP_UUID_16BIT(uuid) \
            DETD_UUID + DESD_2BYTES,         /* Type & size index 0x19 */ \
            (unsigned char)(((uuid) & 0xff00) >> 8),    /* Bits[15:8] of UUID */     \
            (unsigned char)((uuid) & 0x00ff)            /* Bits[7:0] of UUID */

/*---------------------------------------------------------------------------
 * SDP_UUID_32BIT()
 *
 *     Macro that forms a UUID Data Element from the supplied 32-bit UUID value. 
 *     UUID data elements are used in SDP Record Attributes and SDP Queries.
 *
 * Parameters:
 *     uuid - 32-bit UUID value (see the SC_ and PROT_ values for 16-bit 
 *            values). 16-bit UUID values can be converted to 32-bit by 
 *            zero extending the 16-bit value.
 */
#define SDP_UUID_32BIT(uuid) \
            DETD_UUID + DESD_4BYTES,            /* Type & size index 0x1A */ \
            (unsigned char)(((uuid) & 0xff000000) >> 24),  /* Bits[32:24] of UUID */    \
            (unsigned char)(((uuid) & 0x00ff0000) >> 16),  /* Bits[23:16] of UUID */    \
            (unsigned char)(((uuid) & 0x0000ff00) >> 8),   /* Bits[15:8] of UUID */     \
            (unsigned char)((uuid) & 0x000000ff)           /* Bits[7:0] of UUID */

/*---------------------------------------------------------------------------
 * SDP_UUID_128BIT()
 *
 *     Macro that forms a UUID Data Element from the supplied 128-bit UUID value. 
 *     UUID data elements are used in SDP Record Attributes and SDP Queries.
 *
 * Parameters:
 *     uuid - 128-bit UUID value (see the SC_ and PROT_ values for 16-bit 
 *            values). 16-bit UUID values can be converted to 128-bit using 
 *            the following conversion: 128_bit_value = 16_bit_value * 2^96 +
 *            Bluetooth Base UUID.
 */
#define SDP_UUID_128BIT(uuid)                /* UUID must be a 16-byte array */ \
            DETD_UUID + DESD_16BYTES,        /* Type & size index 0x1C */ \
            uuid                             /* 128-bit UUID */

/*---------------------------------------------------------------------------
 * SDP_UINT_8BIT()
 *
 *     Macro that forms a UINT Data Element from the supplied 8-bit UINT value. 
 *     UINT data elements are used in SDP Record Attributes and SDP Queries.
 *
 * Parameters:
 *     uint - 8-bit UINT value.
 */
#define SDP_UINT_8BIT(uint) \
            DETD_UINT + DESD_1BYTE,          /* Type & size index 0x08 */ \
            (unsigned char)(uint)                       /* 8-bit UINT */

/*---------------------------------------------------------------------------
 * SDP_UINT_16BIT()
 *
 *     Macro that forms a UINT Data Element from the supplied 16-bit UINT value. 
 *     UINT data elements are used in SDP Record Attributes and SDP Queries.
 *
 * Parameters:
 *     uint - 16-bit UINT value.
 */
#define SDP_UINT_16BIT(uint) \
            DETD_UINT + DESD_2BYTES,         /* Type & size index 0x09 */ \
            (unsigned char)(((uint) & 0xff00) >> 8),    /* Bits[15:8] of UINT */     \
            (unsigned char)((uint) & 0x00ff)            /* Bits[7:0] of UINT */

/*---------------------------------------------------------------------------
 * SDP_UINT_32BIT()
 *
 *     Macro that forms a UINT Data Element from the supplied 32-bit UINT value. 
 *     UINT data elements are used in SDP Record Attributes and SDP Queries.
 *
 * Parameters:
 *     uint - 32-bit UINT value.
 */
#define SDP_UINT_32BIT(uint) \
            DETD_UINT + DESD_4BYTES,            /* Type & size index 0x0A */ \
            (unsigned char)(((uint) & 0xff000000) >> 24),  /* Bits[31:24] of UINT */    \
            (unsigned char)(((uint) & 0x00ff0000) >> 16),  /* Bits[23:16] of UINT */    \
            (unsigned char)(((uint) & 0x0000ff00) >> 8),   /* Bits[15:8] of UINT */     \
            (unsigned char)((uint) & 0x000000ff)           /* Bits[7:0] of UINT */

/*---------------------------------------------------------------------------
 * SDP_UINT_64BIT()
 *
 *     Macro that forms a UINT Data Element from the supplied 64-bit UINT value. 
 *     UINT data elements are used in SDP Record Attributes and SDP Queries.
 *
 * Parameters:
 *     uint - 64-bit UINT value.
 */
#define SDP_UINT_64BIT(uint)                    /* UINT must be an 8-byte array */ \
            DETD_UINT + DESD_8BYTES,            /* Type & size index 0x0B */ \
            uint                                /* 64-bit UINT */

/*---------------------------------------------------------------------------
 * SDP_UINT_128BIT()
 *
 *     Macro that forms a UINT Data Element from the supplied 128-bit UINT value. 
 *     UINT data elements are used in SDP Record Attributes and SDP Queries.
 *
 * Parameters:
 *     uint - 128-bit UINT value.
 */
#define SDP_UINT_128BIT(uint)                   /* UINT must be a 16-byte array */ \
            DETD_UINT + DESD_16BYTES,           /* Type & size index 0x0C */ \
            uint                                /* 128-bit UINT */

/*---------------------------------------------------------------------------
 * SDP_TEXT_8BIT()
 *
 *     Macro that forms a TEXT Data Element Header from the supplied 8-bit size 
 *     value. TEXT data elements are used in SDP Record Attributes and SDP Queries.
 *     Notice that this macro only forms the header portion of the TEXT Data 
 *     Element. The actual TEXT data within this data element will need to 
 *     be provided separately.
 *
 * Parameters:
 *     size - 8-bit size value.
 */
#define SDP_TEXT_8BIT(size) \
            DETD_TEXT + DESD_ADD_8BITS,      /* Type & size index 0x25 */ \
            (unsigned char)(size)                       /* 8-bit size */

/*---------------------------------------------------------------------------
 * SDP_TEXT_16BIT()
 *
 *     Macro that forms a TEXT Data Element Header from the supplied 16-bit size 
 *     value. TEXT data elements are used in SDP Record Attributes and SDP Queries.
 *     Notice that this macro only forms the header portion of the TEXT Data 
 *     Element. The actual TEXT data within this data element will need to 
 *     be provided separately.
 *
 * Parameters:
 *     size - 16-bit size value.
 */
#define SDP_TEXT_16BIT(size) \
            DETD_TEXT + DESD_ADD_16BITS,      /* Type & size index 0x26 */ \
            (unsigned char)(((size) & 0xff00) >> 8),     /* Bits[15:8] of size */     \
            (unsigned char)((size) & 0x00ff)             /* Bits[7:0] of size */

/*---------------------------------------------------------------------------
 * SDP_TEXT_32BIT()
 *
 *     Macro that forms a TEXT Data Element Header from the supplied 32-bit size 
 *     value. TEXT data elements are used in SDP Record Attributes and SDP Queries.
 *     Notice that this macro only forms the header portion of the TEXT Data 
 *     Element. The actual TEXT data within this data element will need to 
 *     be provided separately.
 *
 * Parameters:
 *     size - 32-bit size value.
 */
#define SDP_TEXT_32BIT(size) \
            DETD_TEXT + DESD_ADD_32BITS,        /* Type & size index 0x27 */ \
            (unsigned char)(((size) & 0xff000000) >> 24),  /* Bits[32:24] of size */    \
            (unsigned char)(((size) & 0x00ff0000) >> 16),  /* Bits[23:16] of size */    \
            (unsigned char)(((size) & 0x0000ff00) >> 8),   /* Bits[15:8] of size */     \
            (unsigned char)((size) & 0x000000ff)           /* Bits[7:0] of size */

/*---------------------------------------------------------------------------
 * SDP_BOOL()
 *
 *     Macro that forms a BOOL Data Element from the supplied 8-bit boolean value. 
 *     BOOL data elements are used in SDP Record Attributes and SDP Queries.
 *
 * Parameters:
 *     value - 8-bit boolean value.
 */
#define SDP_BOOL(value) \
            DETD_BOOL + DESD_1BYTE,          /* Type & size index 0x28 */ \
            (unsigned char)(value)                      /* Boolean value */


/*---------------------------------------------------------------------------
 * HID_DEVICE_RELEASE constant
 *     Vendor defined release version number for the device.  The default value
 *     is 0x0100 (1.0).  This information is stored in the SDP databases for the
 *     HID device and the DeviceId.
 */
#ifndef HID_DEVICE_RELEASE
#define HID_DEVICE_RELEASE 0x0100
#endif

/*---------------------------------------------------------------------------
 * HID_PARSER_VERSION constant
 *     Version number for the HID parser for which the device was designed.  
 *     The default value is 0x0111 (v1.11).  This information is stored in the
 *     SDP database of the HID device.
 */
#ifndef HID_PARSER_VERSION
#define HID_PARSER_VERSION 0x0111
#endif
      
/*---------------------------------------------------------------------------
 * HID_DEVICE_SUBCLASS constant
 *     Defines the type of device.  This is equivalent to the low order
 *     8 bytes of the Class of Device/Service Class field.  The default value
 *     is "Unspecified."  This information is stored in the SDP database of the 
 *     HID device.
 */
#ifndef HID_DEVICE_SUBCLASS
#define HID_DEVICE_SUBCLASS   0
#endif
      
/*---------------------------------------------------------------------------
 * HID_COUNTRY_CODE constant
 *     Country code.  The default value is 0x21 (USA).  This information is
 *     stored in the SDP database of the HID device.
 */
#ifndef HID_COUNTRY_CODE
#define HID_COUNTRY_CODE 0x21
#endif
      
/*---------------------------------------------------------------------------
 * HID_VIRTUAL_CABLE constant
 *     Indicates whether the device supports virtual connections.  Devices
 *     which set this value to TRUE indicate that they support a 1:1 bonding
 *     with the host and expect to automatically re-connect if the connection
 *     is dropped.  The default value is TRUE.  This information is stored in
 *     the SDP database of the HID device.
 */
#ifndef HID_VIRTUAL_CABLE
#define HID_VIRTUAL_CABLE 1
#endif
      
/*---------------------------------------------------------------------------
 * HID_RECONNECT_INITIATE constant
 *     Indicates that the device will attempt to re-connect to the host if
 *     the connection is dropped.  If set to TRUE, the device will connect
 *     to the host when the connection is dropped.  If set to FALSE, the
 *     device must be in a state to accept a connection from the host.
 *     Establishing a connection and setting the connectability modes is
 *     the responsibility of the application.  The default value is TRUE.  This
 *     information is stored in the SDP database of the HID device.
 */
#ifndef HID_RECONNECT_INITIATE
#define HID_RECONNECT_INITIATE 1
#endif
      
/*---------------------------------------------------------------------------
 * HID_DESCRIPTOR_TYPE constant
 *     Defines the type of descriptor defined by HID_DESCRIPTOR. The value
 *     can be either 0x22 (Report) or 0x23 (Physical Descriptor).
 *     A sample descriptor of type "Report" is provided.
 */
#ifndef HID_DESCRIPTOR_TYPE
#define HID_DESCRIPTOR_TYPE 0x22
#endif

/*---------------------------------------------------------------------------
 * HID_DESCRIPTOR_LEN constant
 *     Indicates the length of the descriptor defined in HID_DESCRIPTOR.  
 *     This length must match the number of elements defined in HID_DESCRIPTOR.
 *     A sample length and descriptor are provided.
 */
 #define HID_DESCRIPTOR_LEN 96
#ifndef HID_DESCRIPTOR_LEN
#define HID_DESCRIPTOR_LEN 96
#endif
	/**
		 *	--------------------------------------------------------------------------
		 *	Bit 	 |	 7	 |	 6	 |	 5	 |	 4	 |	 3	 |	 2	 |	 1	 |	 0	 |
		 *	--------------------------------------------------------------------------
		 *	Byte 0	 |				 Not Used				 | Middle| Right | Left  |
		 *	--------------------------------------------------------------------------
		 *	Byte 1	 |					   X Axis Relative Movement 				 |
		 *	--------------------------------------------------------------------------
		 *	Byte 2	 |					   Y Axis Relative Movement 				 |
		 *	--------------------------------------------------------------------------
		 *	Byte 3	 |					   Wheel Relative Movement					 |
		 *	--------------------------------------------------------------------------

#define HID_DESCRIPTOR \
        0x05, 0x01,     \
		0x09, 0x02,     \
		0xA1, 0x01, 	\
		0x85, 0x01, 	\
		0x09, 0x01, 	\
		0xA1, 0x00, 	\
		0x05, 0x09, 	\
		0x19, 0x01, 	\
		0x29, 0x08, 	\
		0x15, 0x00, 	\
		0x25, 0x01, 	\
		0x75, 0x01, 	\
		0x95, 0x08, 	\
		0x81, 0x02, 	\
		0x05, 0x01, 	\
		0x16, 0x08, 0xFF, \
		0x26, 0xFF, 0x00, \
		0x75, 0x08, 	\
		0x95, 0x02, 	\
		0x09, 0x30, 	\
		0x09, 0x31, 	\
		0x81, 0x06, 	\
	\
		0x15, 0x81, 	\
		0x25, 0x7F, 	\
		0x75, 0x08, 	\
		0x95, 0x01, 	\
		0x09, 0x38, 	\
		0x81, 0x06, 	\
	\
		0xC0,			\
		0xC0			

*/
#if 0
///mouse demo
#define HID_DESCRIPTOR \
            0x05, 0x01,  /* USAGE_PAGE(Generic Desktop)   */ \
            0x09, 0x02,  /* USAGE(Mouse)                  */ \
            0xA1, 0x01,  /* COLLECTION(Application)       */ \
            0x09, 0x01,  /*  USAGE(Pointer)               */ \
            0xA1, 0x00,  /*  COLLECTION(Physical)         */ \
            0x05, 0x01,  /*   USAGE_PAGE(Generic Desktop) */ \
            0x09, 0x30,  /*   USAGE(X)                    */ \
            0x09, 0x31,  /*   USAGE(Y)                    */ \
            0x15, 0x81,  /*   LOGICAL_MINIMUM(-127)       */ \
            0x25, 0x7F,  /*   LOGICAL_MAXIMUM(127)        */ \
            0x75, 0x08,  /*   REPORT_SIZE(8)              */ \
            0x95, 0x02,  /*   REPORT_COUNT(2)             */ \
            0x81, 0x06,  /*   INPUT(Data,Var,Rel)         */ \
            0xC0,        /*  END_COLLECTION               */ \
            0x05, 0x09,  /*  USAGE_PAGE(Button)           */ \
            0x19, 0x01,  /*  USAGE_MINIMUM(Button 1)      */ \
            0x29, 0x03,  /*  USAGE_MAXIMUM(Button 3)      */ \
            0x15, 0x00,  /*  LOGICAL_MINUMUM(0)           */ \
            0x25, 0x03,  /*  LOGICAL_MAXIMUM(1)           */ \
            0x95, 0x03,  /*  REPORT_COUNT(3)              */ \
            0x75, 0x01,  /*  REPORT_SIZE(1)               */ \
            0x81, 0x02,  /*  INPUT(Data,Var,Abs)          */ \
            0x95, 0x01,  /*  REPORT_COUNT(1)              */ \
            0x75, 0x05,  /*  REPORT_SIZE(5)               */ \
            0x81, 0x03,  /*  INPUT(Cnst,Var,Abs)          */ \
            0xC0         /* END_COLLECTION                */ 
#else
#define HID_DESCRIPTOR \
    0x05, 0x01,      /* UsagePage (Generic Desktop)*/ \
    0x09, 0x02,      /* Usage (Mouse)*/ \
    0xA1, 0x01,      /* Collection (Application)*/    \
    0x85, 0x01,     /* Report Id (1)*/      \
    0x09, 0x01,      /*   Usage (Pointer)*/\
    0xA1, 0x00,      /*  Collection (Physical)*/ \
    0x05, 0x09,      /*     Usage Page (Buttons)*/\
    0x19, 0x01,      /*     Usage Minimum (01) -Button 1*/\
    0x29, 0x03,      /*     Usage Maximum (03) -Button 3*/\
    0x15, 0x00,      /*     Logical Minimum (0)*/\
    0x25, 0x01,      /*     Logical Maximum (1)*/\
    0x75, 0x01,      /*     Report Size (1)*/\
    0x95, 0x03,      /*     Report Count (3)*/\
    0x81, 0x02,      /*     Input (Data, Variable,Absolute) - Button states*/\
    0x75, 0x05,      /*     Report Size (5)*/\
    0x95, 0x01,      /*     Report Count (1)*/\
    0x81, 0x01,      /*     Input (Constant) - Paddingor Reserved bits*/\
    0x05, 0x01,      /*     Usage Page (GenericDesktop)*/\
    0x09, 0x30,      /*     Usage (X)*/\
    0x09, 0x31,      /*     Usage (Y)*/\
    0x09, 0x38,      /*     Usage (Wheel)*/\
    0x15, 0x81,      /*     Logical Minimum (-127)*/\
    0x25, 0x7F,      /*     Logical Maximum (127)*/\
    0x75, 0x08,      /*     Report Size (8)*/\
    0x95, 0x03,      /*     Report Count (3)*/\
    0x81, 0x06,      /*    Input (Data, Variable,Relative) - X & Y coordinate*/ \
    0xC0,            /*   End Collection*/\
    0xC0,            /* End Collection*/\
    0x05, 0x0c,     \
    0x09, 0x01,     \
    0xa1, 0x01,     \
    0x85, 0x02,      /* Report Id (2)*/\
    0x15, 0x00,     \
    0x25, 0x01,     \
    0x75, 0x01,     \
    0x95, 0x01,     \
    0x09, 0xcd,     \
    0x81, 0x06,     \
    0x0a, 0x83, 0x01,\
    0x81, 0x06,     \
    0x09, 0xb5,     \
    0x81, 0x06,     \
    0x09, 0xb6,     \
    0x81, 0x06,     \
    0x09, 0xea,     \
    0x81, 0x06,     \
    0x09, 0xe9,     \
    0x81, 0x06,     \
    0xc0           



/*

///bt volume ctrl,play&pause
#define HID_DESCRIPTOR \
            0x05, 0x0c,     \
            0x09, 0x01,     \
            0xa1, 0x01,     \
            0x15, 0x00,     \
            0x25, 0x01,     \
            0x75, 0x01,     \
            0x95, 0x01,     \
            0x09, 0xcd,     \
            0x81, 0x06,     \
            0x0a, 0x83, 0x01,\
            0x81, 0x06,     \
            0x09, 0xb5,     \
            0x81, 0x06,     \
            0x09, 0xb6,     \
            0x81, 0x06,     \
            0x09, 0xea,     \
            0x81, 0x06,     \
            0x09, 0xe9,     \
            0x81, 0x06,     \
            0xc0


bit 0 (0x01)  :开始暂停

bit 1 (0x02 ) :一键启动应用

bit 2 (0x04 ) :下一首

bit 3 (0x08 ) :上一首

bit 4 (0x10 ) :音量 -

bit 5 (0x20 ) :音量 +
*/


#endif
/*---------------------------------------------------------------------------
 * HID_MAX_DESCRIPTOR_LEN constant
 *
 *     Defines maximum storage set aside on the host for each device's
 *     descriptor list. This value is only used when HID_HOST is enabled.
 */
#ifndef HID_MAX_DESCRIPTOR_LEN
#define HID_MAX_DESCRIPTOR_LEN 128
#endif

/*---------------------------------------------------------------------------
 * HID_BATTERY_POWER constant
 *     Indicates if the device is battery powered.  If set to TRUE, the device
 *     is battery powered.  If set to FALSE, the device has continuous power
 *     from a power supply.  The default value is FALSE.  This information is 
 *     stored in the SDP database of the HID device
 */
#ifndef HID_BATTERY_POWER
#define HID_BATTERY_POWER 0
#endif
      
/*---------------------------------------------------------------------------
 * HID_REMOTE_WAKE constant
 *     Indicates if the host can wake the device from suspend mode.  If
 *     this value is set to be TRUE, the host can wake the device from
 *     suspend mode by sending the appropriate control message.  If this
 *     value is set to FALSE, the host can exclude this device for the
 *     set of devices that it can wake up.  The default value is TRUE.  The
 *     ability to remotely wake is device (application) specific.  This
 *     information is stored in the SDP database of the HID device.
 */
#ifndef HID_REMOTE_WAKE
#define HID_REMOTE_WAKE 1
#endif
      
/*---------------------------------------------------------------------------
 * HID_SUPERVISION_TIMEOUT constant
 *     Defines the recommended supervision timeout for baseband connections.
 *     The default is the default value specified by the Bluetooth specification
 *     (0x7d00).  This information is stored in the SDP database of the HID 
 *     device.
 */
#ifndef HID_SUPERVISION_TIMEOUT
#define HID_SUPERVISION_TIMEOUT 0x7D00
#endif
      
/*---------------------------------------------------------------------------
 * HID_NORMALLY_CONNECTABLE constant
 *     Defines whether the device is normally in page scan mode.  If set to
 *     TRUE, the device is available to receive connections when there is
 *     no active connection.  The default value is TRUE.  Placing the device
 *     in the connectable mode is a function of the application.  This 
 *     information is stored in the SDP database of the HID device.
 */
#ifndef HID_NORMALLY_CONNECTABLE
#define HID_NORMALLY_CONNECTABLE 1
#endif
      
/*---------------------------------------------------------------------------
 * HID_BOOT_DEVICE constant
 *     Indicates whether the device supports the boot protocol.  If set to
 *     TRUE, the device supports the HID_TRANS_SET_PROTOCOL and 
 *     HID_TRANS_GET_PROTOCOL transaction.  The default value is TRUE.  Support
 *     for the boot protocol is application specific.  This information is
 *     stored in the SDP database of the HID device.
 */
#ifndef HID_BOOT_DEVICE
#define HID_BOOT_DEVICE 1
#endif
      
/*---------------------------------------------------------------------------
 * HID_DEVID_SPEC_ID constant
 *     Version number of the Bluetooth Device ID Profile supported by the 
 *     device.  The default value is 0x0103 (1.3).  This information is stored 
 *     in the SDP database for the DeviceId.
 */
#ifndef HID_DEVID_SPEC_ID
#define HID_DEVID_SPEC_ID 0x0103
#endif

/*---------------------------------------------------------------------------
 * HID_DEVID_VENDOR_ID constant
 *     Unique identifier for the vendor of the device. This value is used in 
 *     conjunction with the HID_DEVID_VENDOR_ID_SRC, which identifies the 
 *     organization that assigned the Vendor ID value. The example value 
 *     shown is 0x23A1, but should be changed for each vendor.  This 
 *     information is stored in the SDP database for the DeviceId.
 */
#ifndef HID_DEVID_VENDOR_ID
#define HID_DEVID_VENDOR_ID 0x23A1
#endif

/*---------------------------------------------------------------------------
 * HID_DEVID_VENDOR_ID_SRC constant
 *     Defines which organization assigned the VendorID value for the device. 
 *     This value is used in conjunction with the HID_DEVID_VENDOR_ID. The
 *     default value is 0x0001 which identifies the Bluetooth SIG as the
 *     organization assigning the Vendor ID value. This information is stored 
 *     in the SDP database for the DeviceId.
 */
#ifndef HID_DEVID_VENDOR_ID_SRC
#define HID_DEVID_VENDOR_ID_SRC 0x0001
#endif

/*---------------------------------------------------------------------------
 * HID_DEVID_PRODUCT_ID constant
 *     Unique identifier for each product used by the vendor. The example 
 *     value shown is 0x1234, but should be changed for each product issued 
 *     by a vendor.This information is stored in the SDP database for the 
 *     DeviceId.
 */
#ifndef HID_DEVID_PRODUCT_ID
#define HID_DEVID_PRODUCT_ID 0x1234
#endif



/*---------------------------------------------------------------------------
 * SdpAttribute structure
 *
 *     Defines an attribute's ID and value. SdpAttribute structures
 *     are stored in a SdpRecord prior to calling the SDP_AddRecord
 *     function. 
 */
typedef struct _SdpAttribute
{
    unsigned short   id;       /* Attribute ID. */

    unsigned short              len;      /* Length of the value buffer */
    
    const unsigned char        *value;    /* An array of bytes that contains the value
                                * of the attribute. The buffer is in
                                * Data Element form (see SdpDataElemType
                                * and SdpDataElemSize).
                                */
    
    /* Group: The following field is for internal use only */
    unsigned short              flags;

} SdpAttribute;

/*---------------------------------------------------------------------------
 * HidReport structure
 *
 *     This structure is used to identify an HID report.  The "reportType" 
 *     field describes the type of report pointed to by the "data" field.
 */
struct _HidReport {

    HidReportType reportType;    /* Report type (input, output, or feature) */
    uint8_t           *data;          /* Report data */
    uint16_t           dataLen;       /* Length of the report data */
};

/*---------------------------------------------------------------------------
 * HidReportReq structure
 *
 *     This structure is used to identify an HID report request.  The 
 *     "reportType" field describes the type of report pointed to by the "data" 
 *     field.
 */
struct _HidReportReq {

    HidReportType reportType;    /* Report type (input, output, or feature) */
    int          useId;         /* Set to TRUE if reportId should be used */
    uint8_t            reportId;      /* The report ID (optional) */

    /* Indicates the maximum amount of report data to return. If 0,
     * indicates a request to deliver all outstanding data.
     *
     * Note that in boot mode, this value must be increased by 1 to accomodate
     * the length of the Report ID field.
     *
     * A device that receives a report request with a non-zero buffer size
     * should respond with a report no longer than the specified bufferSize,
     * truncating if necessary.
     */
    uint16_t           bufferSize;    
};

/*---------------------------------------------------------------------------
 * HidTransaction structure
 *
 *     This structure is used to identify an HID transaction.
 */
struct _HidTransaction {
    /* Used internally by HID */
    void           *resv1;
    void           *resv2;

    /* Contains the HID Result Code */
    HidResultCode   resultCode;

    union {
        /* Contains report data */
        HidReport      *report;

        /* Contains a report data request */
        HidReportReq   *reportReq;

        /* Contains the current protocol */
        HidProtocol     protocol;

        /* Contains the idle rate */
        HidIdleRate     idleRate;

        /* Contains the control operation */
        HidControl      control;
    } parm;

    HidTransactionType  type;

    /* === Internal use only === */
    uint8_t                  flags;
    uint16_t                 offset;
};

/*---------------------------------------------------------------------------
 * HidInterrupt structure
 *
 *     This structure is used to identify an HID interrupt.
 */
struct _HidInterrupt {
    /* Used internally by HID */
    void           *resv1;
    void           *resv2;

    /* Contains a pointer to interrupt data */
    uint8_t           *data;

    /* Contains the length of interrupt data */
    uint16_t           dataLen;

    /* Report type (input, output, or feature) */
    HidReportType reportType;    

    /* === Internal use only === */
    uint8_t            flags;
    uint16_t           offset;
};

/*---------------------------------------------------------------------------
 * HidQueryRsp structure
 *
 *     This structure contains SDP query data from the device.  This structure
 *     only exists if HID_HOST is defined to be XA_ENABLE.  The Host has the
 *     ability to query a device's SDP entry for important HID information.
 *     During the query response, that data is passed to the application in
 *     this structure.  The data remains valid until the channel is deregistred.
 */
typedef struct _HidQueryRsp {
    HidQueryFlags     queryAttemptFlags;/* Defines which fields have been queried
                                         * for already.
                                         */

    HidQueryFlags     queryFlags;       /* Defines which query field contains
                                         * valid data.
                                         */
    uint16_t               deviceRelease;    /* Vendor specified device release 
                                         * version.
                                         */
    uint16_t               parserVersion;    /* HID parser version for which this
                                         * device is designed.
                                         */
    uint8_t                deviceSubclass;   /* Device subclass (minor Class of 
                                         * Device).
                                         */
    uint8_t                countryCode;      /* Country Code
                                         */
    uint8_t                virtualCable;     /* Virtual Cable relationship is 
                                         * supported.
                                         */
    uint8_t                reconnect;        /* Device initiates reconnect.
                                         */
    uint8_t                sdpDisable;       /* When TRUE, the device cannot accept
                                         * an SDP query when the control/interrupt
                                         * channels are connected.
                                         */
    uint8_t                batteryPower;     /* The device runs on battery power.
                                         */
    uint8_t                remoteWakeup;     /* The device can be awakened remotely.
                                         */
    uint16_t               profileVersion;   /* Version of the HID profile.
                                         */
    uint16_t               supervTimeout;    /* Suggested supervision timeout value
                                         * for LMP connections.
                                         */
    uint8_t                normConnectable;  /* The device is connectable when no
                                         * connection exists.
                                         */
    uint8_t                bootDevice;       /* Boot protocol support is provided.
                                         */
    uint16_t               descriptorLen;    /* Length of the HID descriptor list.
                                         */

    /* A list of HID descriptors (report or physical) associated with the
     * device. Each element of the list is an SDP data element sequence,
     * and therefore has a header of two bytes (0x35, len) which precedes
     * each descriptor.
     */
    uint8_t                descriptorList[HID_MAX_DESCRIPTOR_LEN];
} HidQueryRsp;


typedef struct{
    HidEvent            event;        /* Type of the HID transport event  */     
                                                                           
    BtStatus            status;       /* Communication status or error information */

    uint16_t                 len;          /* Length of the object pointed to 
                                       * by "ptrs"
                                       */

    union {
        /* During HIDEVENT_OPEN_IND, HIDEVENT_OPEN or HIDEVENT_CLOSED, 
         * events, contains the remote device structure.
         */
        void *remDev;

        /* During an HIDEVENT_TRANSACTION and HIDEVENT_TRANSACTION_RSP
         * events, contains the transaction type and parameters.
         */
        HidTransaction *trans;

        /* During HIDEVENT_INTERRUPT and HIDEVENT_INTERRUPT_RSP
         * events, contains the interrupt data.
         */
        HidInterrupt   *intr;

        /* During HIDEVENT_QUERY_CNF, contains the SDP query response data.
         */
        HidQueryRsp    *queryRsp;
    } ptrs;


}hid_event_t;

extern void *hid_sdp_attribute;
extern unsigned char hid_sdp_attribute_num; //maximum 24
extern void *device_id_sdp_attribute;
extern unsigned char device_id_sdp_attribute_num; //maximum 7

BtStatus hid_send_transaction(void *Channel, HidTransactionType TranType, 
                             HidTransaction *Trans);
/*---------------------------------------------------------------------------
 * HID_OpenConnection()
 *
 *     Attempts to establish a connection with a remote device (Host or HID
 *     Device).
 * 
 * Parameters:
 *     Channel - Identifies the Channel for this action.
 *
 *     Addr - Bluetooth device address of the remote device.
 *
 * Returns:
 *     BT_STATUS_FAILED - The request was invalid.
 *
 *     BT_STATUS_PENDING - The request to open the connection was sent.
 *         If the request is accepted by the remote device, a HIDEVENT_OPEN
 *         event will be sent to the application. If the connection is 
 *         rejected, a HIDEVENT_CLOSED event will be sent to the application.
 *
 *     BT_STATUS_BUSY - The connection is open or in the process of opening.
 *
 *     BT_STATUS_NOT_FOUND - The specified channel was not found (XA_ERROR_CHECK 
 *         only) or the BD_ADDR_T specified an unknown device.
 *
 *     BT_STATUS_NO_CONNECTION - No ACL connection exists.
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus hid_open_connection(void *Channel, BD_ADDR *Addr);

/*---------------------------------------------------------------------------
 * HID_AcceptConnection()
 *
 *     Accepts an incoming connection in response to an HIDEVENT_OPEN_IND 
 *     event.  This event occurs when a remote device (Host or HID Device) 
 *     attempts to connect to a registered Host or HID Device. Either this 
 *     function or HID_RejectConnection must be used to respond to the 
 *     connection request.
 *
 * Parameters:
 *     Channel - Identifies the channel that is accepting the connection.  This 
 *         channel is provided to the callback function as a parameter during 
 *         the HIDEVENT_OPEN_IND event.
 *
 * Returns:
 *     BT_STATUS_FAILED - The specified channel did not have a pending 
 *         connection request.
 *
 *     BT_STATUS_PENDING - The accept message will be sent. The application
 *         will receive an HIDEVENT_OPEN when the accept message has been 
 *         sent and the channel is open.
 *
 *     BT_STATUS_BUSY - A response is already in progress.
 *
 *     BT_STATUS_NOT_FOUND - The specified channel was not found (XA_ERROR_CHECK 
 *         only).
 *
 *     BT_STATUS_NO_CONNECTION - No ACL connection exists.
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus hid_accept_connection(void *Channel);

/*---------------------------------------------------------------------------
 * HID_RejectConnection()
 *
 *     Rejects an incoming connection in response to an HIDEVENT_OPEN_IND 
 *     event.  This event occurs when a remote device (Host or HID Device) 
 *     attempts to connect to a registered Host or HID Device. Either this 
 *     function or HID_AcceptConnection must be used to respond to the 
 *     connection request.
 *
 * Parameters:
 *     Channel - Identifies the channel to be rejected. This channel is
 *         provided to the callback function as a parameter during the
 *         HIDEVENT_OPEN_IND event.
 *
 * Returns:
 *     BT_STATUS_FAILED - The specified channel did not have a pending
 *         connection request.
 *
 *     BT_STATUS_PENDING - The rejection message has been sent. The application
 *         will receive an HIDEVENT_CLOSED event when the rejection is 
 *         complete.
 *
 *     BT_STATUS_BUSY - A response is already in progress.
 *
 *     BT_STATUS_NOT_FOUND - The specified channel was not found (XA_ERROR_CHECK 
 *         only).
 *
 *     BT_STATUS_NO_CONNECTION - No ACL connection exists.
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus hid_reject_connection(void *Channel);

/*---------------------------------------------------------------------------
 * HID_SendControl()
 *
 *     Sends a control operation to the remote device (Host or HID Device).  The
 *     "parm.control" field of the "Trans" parameter should be initialized with 
 *     the appropriate control operation. A HID device can only send the 
 *     HID_CTRL_VIRTUAL_CABLE_UNPLUG control operation.  A Host can send any 
 *     control operation.
 *
 * Parameters:
 *     Channel - Identifies the channel on which to send the request.
 *
 *     Trans - A pointer to the transaction, which describes the control
 *         operation.
 *
 * Returns:
 *     BT_STATUS_PENDING - The request has been queued. If sent successfully, 
 *         an HIDEVENT_TRANSACTION_COMPLETE event will arrive with a "status" of 
 *         BT_STATUS_SUCCESS.  If the transmission fails, the same event will be 
 *         received with a status specifying the reason.  The memory pointed
 *         to by the Trans parameter must not be modified until the 
 *         transaction is complete.
 *
 *     BT_STATUS_NO_CONNECTION - No connection exists for transmitting.
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus hid_send_control(void *Channel, HidTransaction *Trans);

/*---------------------------------------------------------------------------
 * HID_SendInterrupt()
 *
 *     Sends an interrupt (report) to the remote device (Host or HID Device).  
 *     The Interrupt parameter should be initialized with the appropriate 
 *     information.
 *
 * Parameters:
 *     Channel - Identifies the channel on which to send the interrupt.
 *
 *     Interrupt - A pointer to the interrupt structure which describes the 
 *         interrupt data.
 *
 * Returns:
 *     BT_STATUS_PENDING - The request has been queued. If sent successfully, 
 *         an HIDEVENT_INTERRUPT_COMPLETE event will arrive with a "status" of 
 *         BT_STATUS_SUCCESS.  If the transmission fails, the same event will be 
 *         received with a status specifying the reason.  The memory pointed
 *         to by the Interrupt parameter must not be modified until the 
 *         transaction is complete.  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel was not found (XA_ERROR_CHECK 
 *         only).
 *
 *     BT_STATUS_NO_CONNECTION - No connection exists for transmitting.
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     BT_STATUS_IN_USE - The channel is already in use (XA_ERROR_CHECK only).
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus hid_send_interrupt(void *Channel, HidInterrupt *Interrupt);

/*---------------------------------------------------------------------------
 * HID_CloseConnection()
 *
 *     Closes an HID connection between two devices.  When the connection
 *     is closed, the application will receive an HIDEVENT_CLOSED event.
 *
 *     If there are outstanding transactions or interrupts when a connection is 
 *     closed, an event will be received by the application for each one.  The 
 *     "status" field for these events will be set to BT_STATUS_NO_CONNECTION.
 *
 * Parameters:
 *     Channel - Identifies the channel connection to be closed. The 
 *         HIDEVENT_CLOSED event indicates that the connection is closed 
 *         and a new connection may be attempted.
 *
 * Returns:
 *     BT_STATUS_PENDING - The request to close the connection will be sent.
 *         The application will receive an HIDEVENT_CLOSED event when the
 *         connection is closed.
 *
 *     BT_STATUS_FAILED - The channel is invalid or could not be
 *         disconnected.
 *
 *     BT_STATUS_NOT_FOUND - The specified channel was not found (XA_ERROR_CHECK 
 *         only).
 *
 *     BT_STATUS_NO_CONNECTION - No ACL connection exists on this channel.
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only). 
 *
 *     BT_STATUS_IN_PROGRESS - The channel is already disconnecting
 *         (XA_ERROR_CHECK Only).    
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus hid_close_connection(void *Channel);

/*---------------------------------------------------------------------------
 * HID_HostQueryDevice()
 *
 *     Queries the Device for its SDP database entry.  The SDP database
 *     contains information about the device's capabilities.  The query
 *     information will be returned to the application with a
 *     HIDEVENT_QUERY_CNF event.  The query data is parsed and placed
 *     in memory allocated dynamically.  The application can save the
 *     pointer to the SDP data and reference it as long as the channel
 *     is still registered.  The data is no longer valid when the channel is
 *     deregistred.
 *
 *     It is possible that the query will fail, because some devices limited 
 *     in memory will not allow an SDP query while the HID channel is open.  
 *     It is suggested that the host query the device before opening a 
 *     connection.  After the callback, the "sdpDisable" field of the 
 *     "ptrs.queryRsp" structure tells whether sdp queries are enabled or 
 *     disabled when a HID channel is already open.
 *
 * Parameters:
 *     Channel - Identifies the Channel for this action.
 *
 *     Addr - Bluetooth device address of the remote device.
 *
 *     Parms - HidServiceSearchAttribReq array.
 *
 *     Len  - HidServiceSearchAttribReq array len.
 *
 * Returns:
 *     BT_STATUS_PENDING - The request to query the SDP entry was sent.
 *         When a response is received from the device, an HIDEVENT_QUERY_CNF
 *         event will be received.
 *
 *     BT_STATUS_BUSY - The connection is already in the process of opening.
 *
 *     BT_STATUS_NOT_FOUND - The specified channel was not found (XA_ERROR_CHECK 
 *         only).
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (SDP, L2CAP or Management Entity).
 */

BtStatus hid_host_query_device(void *Channel, BD_ADDR *Addr, const uint8_t *Parms, uint16_t Len);

/*---------------------------------------------------------------------------
 * HID_HostGetReport()
 *
 *     Sends an report request to the HID device.  The "parm.reportReq" field
 *     of the "Trans" parameter should be initialized with the appropriate 
 *     information.  Requesting an input report causes the device to respond
 *     with the instantaneous state of fields in the requested input report.
 *     Requesting an output report causes the device to respond with the last
 *     output report received on the interrupt channel.  If no output report
 *     has been received, default values will be returned. Requesting a feature
 *     report causes the device to return the default values or instantaneous
 *     state of the feature report fields.
 *
 * Parameters:
 *     Channel - Identifies the channel on which to send the request.
 *
 *     Trans - A pointer to the transaction structure which describes the
 *         request.
 *
 * Returns:
 *     BT_STATUS_PENDING - The request has been queued. If sent successfully, 
 *         at lease one HIDEVENT_TRANSACTION_RSP event will be receive with
 *         report data.  A HIDEVENT_TRANSACTION_COMPLETE event will arrive with
 *         a "status" of BT_STATUS_SUCCESS upon successful completion.
 *         If the transmission fails, the same event will be received with a
 *         status specifying the reason.  The memory pointed to by the Trans
 *         parameter must not be modified until the transaction is complete.
 *
 *     BT_STATUS_NOT_FOUND - The specified channel was not found (XA_ERROR_CHECK 
 *         only).
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus hid_host_get_report(void *Channel, HidTransaction *Trans);

/*---------------------------------------------------------------------------
 * HID_HostSetReport()
 *
 *     Sends a report to the HID device.  The "parm.report" field of the "Trans"
 *     parameter should be initialized with the appropriate report information.
 *
 * Parameters:
 *     Channel - Identifies the channel on which to send the request.
 *
 *     Trans - A pointer to the transaction, which describes the report.
 *
 * Returns:
 *     BT_STATUS_PENDING - The request has been queued. If sent successfully, 
 *         an HIDEVENT_TRANSACTION_COMPLETE event will arrive with a "status" of 
 *         BT_STATUS_SUCCESS.  If the transmission fails, the same event will be 
 *         received with a status specifying the reason.  The memory pointed to
 *         by the Trans parameter must not be modified until the transaction is
 *         complete.
 *
 *     BT_STATUS_NO_CONNECTION - No connection exists for transmitting.
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus hid_host_set_report(void *Channel, HidTransaction *Trans);

/*---------------------------------------------------------------------------
 * HID_HostGetProtocol()
 *
 *     Sends an protocol request to the HID device.  It is not necessary to
 *     initialize the "Trans" parameter.
 *
 * Parameters:
 *     Channel - Identifies the channel on which to send the request.
 *
 *     Trans - A pointer to the transaction structure.
 *
 * Returns:
 *     BT_STATUS_PENDING - The request has been queued. If sent successfully, 
 *         an HIDEVENT_TRANSACTION_RSP event will be receive with protocol data.
 *         An HIDEVENT_TRANSACTION_COMPLETE event will arrive with a "status" of 
 *         BT_STATUS_SUCCESS upon successful complete.  If the transaction
 *         fails, the same event will be received with a status specifying the
 *         reason.  The memory pointed to by the Trans parameter must not be
 *         modified until the transaction is complete.
 *
 *     BT_STATUS_NOT_FOUND - The specified channel was not found (XA_ERROR_CHECK 
 *         only).
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus hid_host_get_protocol(void *Channel, HidTransaction *Trans);

/*---------------------------------------------------------------------------
 * HID_HostSetProtocol()
 *
 *     Sends the current protocol to the HID device.  The "parm.protocol" field
 *     of the "Trans" parameter should be initialized with the appropriate 
 *     protocol.
 *
 * Parameters:
 *     Channel - Identifies the channel on which to send the request.
 *
 *     Trans - A pointer to the transaction, which describes the protocol.
 *
 * Returns:
 *     BT_STATUS_PENDING - The request has been queued. If sent successfully, 
 *         an HIDEVENT_TRANSACTION_COMPLETE event will arrive with a "status" of 
 *         BT_STATUS_SUCCESS.  If the transmission fails, the same event will be 
 *         received with a status specifying the reason.  The memory pointed to
 *         by the Trans parameter must not be modified until the transaction is
 *         complete.
 *
 *     BT_STATUS_NOT_FOUND - The specified channel was not found (XA_ERROR_CHECK 
 *         only).
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus hid_host_set_protocol(void *Channel, HidTransaction *Trans);

/*---------------------------------------------------------------------------
 * HID_DeviceSetReportRsp()
 *
 *     Sends a confirmation of a report to the HID host.  This function is
 *     called in response to an HIDEVENT_DEVICE_SET_REPORT event.  The 
 *     "resultCode" field of the "Trans" parameter should be initialized 
 *     with the appropriate response code. 
 *
 * Parameters:
 *     Channel - Identifies the channel on which to send the response.
 *
 *     Trans - A pointer to the transaction, which describes the result code.
 *
 * Returns:
 *     BT_STATUS_PENDING - The response has been queued.  
 *         A HIDEVENT_TRANSACTION_COMPLETE event will arrive with a "status" of
 *         BT_STATUS_SUCCESS upon successful transmission.  If the transmission 
 *         fails, the same event will be received with a status specifying the
 *         reason.  The memory pointed to by the Trans parameter must not be
 *         modified until the transaction is complete.
 *
 *     BT_STATUS_PENDING - The response has been queued. If sent successfully, 
 *         an HIDEVENT_SET_REPORT_RSP event will arrive with a "status" of 
 *         BT_STATUS_SUCCESS.  If the transmission fails, the same event will be 
 *         received with a status specifying the reason.
 *
 *     BT_STATUS_NOT_FOUND - The specified channel was not found (XA_ERROR_CHECK 
 *         only).
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus hid_device_set_report_rsp(void *Channel, HidTransaction *Trans);

/*---------------------------------------------------------------------------
 * HID_DeviceGetReportRsp()
 *
 *     Sends a report to the HID host.  This function is called in response to
 *     the HIDEVENT_DEVICE_GET_REPORT event.  The "resultCode"
 *     field of the "Trans" parameter should be initialized with the appropriate
 *     response code.  If "resultCode" is set to HID_RESULT_SUCCESS, then
 *     the "parm.report" field of the "Trans" parameter should be initialized 
 *     with the appropriate report data. 
 *
 * Parameters:
 *     Channel - Identifies the channel on which to send the report.
 *
 *     Trans - A pointer to the transaction, which describes the report.
 *
 * Returns:
 *     BT_STATUS_PENDING - The response has been queued.  
 *         A HIDEVENT_TRANSACTION_COMPLETE event will arrive with a "status" of
 *         BT_STATUS_SUCCESS upon successful transmission.  If the transmission 
 *         fails, the same event will be received with a status specifying the
 *         reason.  The memory pointed to by the Trans parameter must not be
 *         modified until the transaction is complete.
 *
 *     BT_STATUS_NOT_FOUND - The specified channel was not found (XA_ERROR_CHECK 
 *         only).
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus hid_device_get_report_rsp(void *Channel, HidTransaction *Trans);

/*---------------------------------------------------------------------------
 * HID_DeviceSetProtocolRsp()
 *
 *     Sends a confirmation of the protocol to the HID host.  This call is made
 *     in response to the HIDEVENT_DEVICE_SET_PROTOCOL event.  The 
 *     "resultCode" field of the "Trans" parameter should be initialized 
 *     with the appropriate response code.
 *
 * Parameters:
 *     Channel - Identifies the channel on which to send the response.
 *
 *     Trans - A pointer to the transaction, which describes the result code.
 *
 * Returns:
 *     BT_STATUS_PENDING - The response has been queued.  
 *         A HIDEVENT_TRANSACTION_COMPLETE event will arrive with a "status" of
 *         BT_STATUS_SUCCESS upon successful transmission.  If the transmission 
 *         fails, the same event will be received with a status specifying the
 *         reason.  The memory pointed to by the Trans parameter must not be
 *         modified until the transaction is complete.
 *
 *     BT_STATUS_NOT_FOUND - The specified channel was not found (XA_ERROR_CHECK 
 *         only).
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus hid_device_set_protocol_rsp(void *Channel, HidTransaction *Trans);

/*---------------------------------------------------------------------------
 * HID_DeviceGetProtocolRsp()
 *
 *     Sends a protocol response to the HID host.  This function is called in 
 *     response to the HIDEVENT_HOST_GET_PROTOCOL event.   The "resultCode"
 *     field of the "Trans" parameter should be initialized with the appropriate
 *     response code.  If "resultCode" is set to HID_RESULT_SUCCESS, then
 *     the "parm.protocol" field of the "Trans" parameter should be initialized
 *     with the appropriate protocol. 
 *
 * Parameters:
 *     Channel - Identifies the channel on which to send the response.
 *
 *     Trans - A pointer to the transaction, which describes the protocol.
 *
 * Returns:
 *     BT_STATUS_PENDING - The response has been queued.  
 *         A HIDEVENT_TRANSACTION_COMPLETE event will arrive with a "status" of
 *         BT_STATUS_SUCCESS upon successful transmission.  If the transmission 
 *         fails, the same event will be received with a status specifying the
 *         reason.  The memory pointed to by the Trans parameter must not be
 *         modified until the transaction is complete.
 *
 *     BT_STATUS_NOT_FOUND - The specified channel was not found (XA_ERROR_CHECK 
 *         only).
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus hid_device_get_protocol_rsp(HidChannel *Channel, HidTransaction *Trans);

#endif

