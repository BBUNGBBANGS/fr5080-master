#ifndef _BT_API_H
#define _BT_API_H

#include "pskeys.h"
#include "bt_error.h"
#include "hf_api.h"


#define BD_ADDR_SIZE    6

/*---------------------------------------------------------------------------
 * BtEventType type
 *
 *     All indications and confirmations are sent through a callback
 *     function. The event types are defined below.
 */
typedef uint8_t BtEventType;

/* Group: Global events sent to all handlers. These can be masked out if
 * desired.
 */

/** A remote device is found. "p.inqResult" is valid.  */
#define BTEVENT_INQUIRY_RESULT       1 

/** The Inquiry process is completed. "errCode" is valid. */
#define BTEVENT_INQUIRY_COMPLETE     2

/** The Inquiry process is canceled. */
#define BTEVENT_INQUIRY_CANCELED     3

/** An incoming ACL connection. "p.remDev" is valid. */
#define BTEVENT_LINK_CONNECT_IND     4

/** An incoming SCO connection. "p.scoConnect" is valid. */
//#define BTEVENT_SCO_CONNECT_IND      5

/** An ACL connection was disconnected. "p.disconnect" is valid, and
 * "errCode" may be valid (see documentation for the "disconnect" union for
 * details. */
#define BTEVENT_LINK_DISCONNECT      6

/* An outgoing ACL connection is up. "p.remDev" and "p.errCode" are valid. */
#define BTEVENT_LINK_CONNECT_CNF     7


/** Indicates the mode of the link changed. "p.modeChange" is valid */
#define BTEVENT_MODE_CHANGE          9

/** Indicates that an accessibility change is complete. "errCode" and
 *  "p.aMode" are valid. */
#define BTEVENT_ACCESSIBLE_CHANGE   10


/** Indicates that a role change has occurred */
#define BTEVENT_ROLE_CHANGE         14

/** SCO link has been disconnected. "p.scoConnect" is valid. */
//#define BTEVENT_SCO_DISCONNECT      15

/** An outgoing SCO link has been established. "p.scoConnect" is valid. */
//#define BTEVENT_SCO_CONNECT_CNF     16


/* Group: The following events are global events but cannot be masked */

/** Indicates that an ACL connection has received an internal data transmit
 *  request while it is in hold, park or sniff mode. The data will still be 
 *  passed to the radio in park and sniff modes. However, hold mode will
 *  block data transmit. It may be necessary to return the ACL to active
 *  mode to restore normal data transfer. "p.remDev" is valid.
 */
#define BTEVENT_ACL_DATA_NOT_ACTIVE 99

/** Indicates that the HCI failed to initialize. This implies that the 
 *  Bluetooth radio is having problems or a radio may not exist. 
 *  Events will be generated with an appropriate error code for any outstanding
 *  operations.  All pending operations that use pointers to memory supplied by 
 *  applications will have a corresponding event generated indicating the 
 *  failure of the operation. Applications should not modify memory passed as 
 *  part of operations until an event signaling the end of the operation
 *  has been received. The HCI and the radio will be reset. If the reset is 
 *  successful then the BTEVENT_HCI_INITIALIZED event will be sent to all 
 *  registered handlers.
 */
#define BTEVENT_HCI_INIT_ERROR      100

/** Indicates that the HCI initialization is successful. The ME will now
 *  accept commands again if an initialization error or fatal error
 *  has occurred. This event is sent whenever HCI is successfully initialized,
 *  including when the stack is first started.
 */
#define BTEVENT_HCI_INITIALIZED     101

/** Indicates that a fatal error has occurred in the radio or the HCI transport.
 *  The HCI and the radio will be reset. If the reset is successful then the 
 *  BTEVENT_HCI_INITIALIZED event will be sent to all registered handlers.
 */
#define BTEVENT_HCI_FATAL_ERROR     102

/** Indicates that the HCI has been deinitialized.  This can happen as the 
 *  result of a call to ME_RadioShutdown, or if some fatal error was reported
 *  by the radio or HCI transport. For any pending operations, events will 
 *  be received with proper error codes prior to receiving this event. 
 *  All pending commands that used memory supplied by the application will 
 *  result in an event, so the application should not modify that memory until 
 *  the event has been received. If the HCI was deinitialized as the result of
 *  a radio or transport error, the HCI will be reset. If the reset is 
 *  successful then the BTEVENT_HCI_INITIALIZED event will be sent to all 
 *  registered handlers.
 */
#define BTEVENT_HCI_DEINITIALIZED   103

/** Indicates that the HCI cannot be initialized. This happens when
 *  the HCI has been reset BT_HCI_NUM_INIT_RETRIES without success or
 *  if some unrecoverable error occurs.
 */
#define BTEVENT_HCI_FAILED          104

/** Indicates that an HCI command has been sent to the radio. This
 *  event is intended for global handlers that would like to be informed
 *  of when certain radio processes are started. "p.hciCmd" is valid.
 */
#define BTEVENT_HCI_COMMAND_SENT    105

/* An outgoing create link has been canceled. "p.remDev" is valid. */
#define BTEVENT_LINK_CREATE_CANCEL  106

/* End of BtEventType */

/*---------------------------------------------------------------------------
 * BtPageScanInfo structure
 */
typedef struct _BtPageScanInfo
{
    uint8_t   psRepMode;
    uint8_t   psMode;
    uint16_t  clockOffset;
} BtPageScanInfo;

/*---------------------------------------------------------------------------
 * BtInquiryMode type
 *
 * Describes the format of the inquiry responses that will be received 
 * from the controller.
 */
typedef uint8_t BtInquiryMode;

#define INQ_MODE_NORMAL    0 /* Normal Inquiry Response format           */
#define INQ_MODE_RSSI      1 /* RSSI Inquiry Response format             */
#define INQ_MODE_EXTENDED  2 /* Extended or RSSI Inquiry Response format */

/* End of BtInquiryMode */

typedef struct _BtInquiryResult
{
    BD_ADDR         bdAddr;           /* Device Address */
    BtPageScanInfo  psi;              /* Page scan info used for connecting */
    uint8_t              psPeriodMode;
    uint32_t classOfDevice;

    /* RSSI in dBm (-127 to +20). Only valid when controller reports RSSI with
     * in inquiry results (also see ME_SetInquiryMode). Otherwise it will be
     * set to BT_INVALID_RSSI.
     */
    int8_t              rssi;             

    /* Extended Inquiry response.  Only valid when controller reports an
     * extended inquiry (also see ME_SetInquiryMode).  Otherwise it will be
     * set to all 0's.
     */ 
    uint8_t              extInqResp[240];

    /* Describes the format of the current inquiry result */
    BtInquiryMode   inqMode;

} BtInquiryResult;

/*---------------------------------------------------------------------------
 * BtAccessibleMode type
 *
 *     Bluetooth Accessibility mode includes Discoverable and connectable
 *     modes.
 */
typedef uint8_t BtAccessibleMode;

#define BAM_NOT_ACCESSIBLE     0x00 /* Non-discoverable or connectable */
#define BAM_GENERAL_ACCESSIBLE 0x03 /* General discoverable and connectable */
#define BAM_LIMITED_ACCESSIBLE 0x13 /* Limited discoverable and connectable */
#define BAM_CONNECTABLE_ONLY   0x02 /* Connectable but not discoverable */
#define BAM_DISCOVERABLE_ONLY  0x01 /* Discoverable but not connectable */

/* End of BtAccessibleMode */

/*---------------------------------------------------------------------------
 * BtAccessModeInfo structure
 */
typedef struct _BtAccessModeInfo
{
    uint16_t inqInterval;    /* Inquiry scan interval */
    uint16_t inqWindow;      /* Inquiry scan Window */
    uint16_t pageInterval;   /* Page scan interval */
    uint16_t pageWindow;     /* Page scan window */
} BtAccessModeInfo;
/*---------------------------------------------------------------------------
 * BtLinkMode type
 */
typedef uint8_t BtLinkMode;

#define BLM_ACTIVE_MODE     0x00
#define BLM_HOLD_MODE       0x01
#define BLM_SNIFF_MODE      0x02
#define BLM_PARK_MODE       0x03
#define BLM_SCATTER_MODE    0x04

/* End of BtLinkMode */

/*---------------------------------------------------------------------------
 * BtConnectionRole type
 *
 *     Specifies the role of a ACL connection
 */
typedef uint8_t BtConnectionRole;

#define BCR_MASTER   0x00
#define BCR_SLAVE    0x01
#define BCR_ANY      0x02
#define BCR_UNKNOWN  0x03

/* End of BtConnectionRole */

/*---------------------------------------------------------------------------
 * BtIac type
 *     Bluetooth Inquiry Access Codes are used to specify which types of
 *     devices should respond to inquiries. Currently there are only two
 *     defined.
 */
typedef uint32_t BtIac;

#define BT_IAC_GIAC 0x9E8B33   /* General/Unlimited Inquiry Access Code */
#define BT_IAC_LIAC 0x9E8B00   /* Limited Dedicated Inquiry Access Code */

/* End of BtIac */

/*---------------------------------------------------------------------------
 * BtSniffInfo structure
 *
 *     Identifies the sniff requirements during a ME_StartSniff request.
 */
typedef struct _BtSniffInfo
{
    /* Maximum acceptable interval between each consecutive sniff period.
     * May be any even number between 0x0002 and 0xFFFE, but the mandatory
     * sniff interval range for controllers is between 0x0006 and 0x0540.
     * The value is expressed in 0.625 ms increments (0x0006 = 3.75 ms).
     *
     * The actual interval selected by the radio will be returned in
     * a BTEVENT_MODE_CHANGE event.
     */
    uint16_t maxInterval;
    
    /* Minimum acceptable interval between each consecutive sniff period.
     * Must be an even number between 0x0002 and 0xFFFE, and be less than
     * "maxInterval". Like maxInterval this value is expressed in
     * 0.625 ms increments.
     */
    uint16_t minInterval;

    /* The number of master-to-slave transmission slots during which
     * a device should listen for traffic (sniff attempt).
     * Expressed in 0.625 ms increments. May be between 0x0001 and 0x7FFF.
     */
    uint16_t attempt;

    /* The amount of time before a sniff timeout occurs. Expressed in
     * 1.25 ms increments. May be between 0x0000 and 0x7FFF, but the mandatory
     * range for controllers is 0x0000 to 0x0028.
     */
    uint16_t timeout;
    
} BtSniffInfo;


typedef struct{
    /* Event causing callback. Always valid.*/
    BtEventType type;
    /* Error code. See BtEventType for guidance on whether errCode is valid. */
    BtErrorCode   errCode;
    BD_ADDR *addr;
    void    *remDev;     /* Pointer to remote device */
    union{
        BtInquiryResult    *inqResult;  /* Inquiry result */
        
        BtAccessibleMode   aMode;      /* New access mode */

       /* Results for the BTEVENT_MODE_CHANGE event */
        struct {
            BtLinkMode      curMode;
            uint16_t             interval;
        } modeChange;

        BtConnectionRole   newRole;    /* New role */
    }param;
}me_event_t;


/* Device context */ 
typedef struct app_device_t {
    int             inUse;//bd addr is valid or not
    uint8_t         state;//bt state
    uint8_t         reconnecting;//linkloss,poweron,button
    uint8_t         reconnect_times;
    BD_ADDR         bd;
    uint8_t         conFlags;//avrcp,a2dp,hfp conn status
    uint8_t         prf_all_connected;
    uint8_t         responder;
    
    uint8_t         mode;//active or sniff
    
    //EvmTimer bt_send_battery_level_timer;
    HfCallSetupState setup_state;
    HfCallActiveState active;
    HfCallHeldState call_held;

    #if 0
    BtRemoteDevice *remDev;
    HfChannel *hf_chan;
    AvrcpChannel *rcp_chan;
    A2dpStream *pstream;
    HidChannel *hid_chan;
    #else
    void *remDev;
    void *hf_chan;
    void *rcp_chan;
    void *pstream;
    void *hid_chan;
    #endif
} APP_DEVICE;

#define NUM_DEVICES 2
/// Application environment structure
struct user_bt_env_tag
{
    uint8_t access_state;
    uint8_t charger_state;
    uint8_t last_active_dev_index;
    uint8_t current_playing_index;
    
    uint8_t current_hid_index;
    uint8_t testmode_enable;
    uint8_t power_off_onging;
    uint8_t button_avrcp_pending;

    APP_DEVICE dev[NUM_DEVICES];
};

enum a2dp_decorder_type_t{
    A2DP_DEC_TYPE_SBC_HW,
    A2DP_DEC_TYPE_SBC_DSP,
};

//typedef void(* app_bt_callback_func_t)(void * param);
typedef void(* me_callback_func_t)(me_event_t * event);


void bt_me_set_cb_func(me_callback_func_t func);


/*---------------------------------------------------------------------------
 * bt_get_current_mode()
 *
 *     Get the current mode of the link to the remote device. The
 *     value is meaningless if a connection does not exist to
 *     the remote device. This function does not check for a valid
 *     remDev pointer.
 *
 * Parameters:
 *     rm - pointer to remote device.
 *
 * Returns:
 *     The current mode if a link exists otherwise the value
 *     is meaningless.
 */
BtLinkMode bt_get_current_mode(void *rm);


/*---------------------------------------------------------------------------
 * bt_get_current_role()
 *
 *     Get the current role played by the local device. The
 *     value is meaningless if a connection does not exist to
 *     the remote device. This function does not check for a valid
 *     remDev pointer. When the role is currently being discovered,
 *     the role BCR_UNKNOWN will be returned. When the role discovery
 *     completes, the BTEVENT_ROLE_CHANGE event will be indicated.
 *
 * Parameters:
 *     rm - pointer to remote device.
 *
 * Returns:
 *     The current role if a link exists otherwise the value
 *     is meaningless.
 */
BtConnectionRole bt_get_current_role(void *rm);

/*---------------------------------------------------------------------------
 * bt_set_accessible_mode_nc()
 *
 *     Set the accessibility mode when the device is not 
 *     connected. If the mode is set to a value other than 
 *     BAM_NOT_ACCESSIBLE and there are no connections then the 
 *     Bluetooth radio will enter inquiry and/or page scan mode 
 *     using the information passed in info. If info is 0 or the
 *     values in info are set to defaults (BT_DEFAULT_SCAN_INTERVAL
 *     and BT_DEFAULT_SCAN_WINDOW) the radio module default values
 *     are used. It is assumed that the macro defaults
 *     match the radio defaults (see BT_DEFAULT_PAGE_SCAN_WINDOW
 *     documentation.) So, the first call to ME_SetAccessibleModeNC
 *     with info set to 0 will not change the settings as the radio has
 *     already been initialized to its default settings. If there is
 *     a connection or a connection is in the process  of being created
 *     then mode and info are saved and applied when all connections are 
 *     disconnected.  
 *
 *     To keep other devices from finding and connecting to this 
 *     device set the mode to BAM_NOT_ACCESSIBLE. The default mode when
 *     the stack is first loaded and initialized is controlled by
 *     BT_DEFAULT_ACCESS_MODE_NC.
 *
 *     In setting the values for info. Both "inqWindow" and
 *     "inqInterval" must be set to defaults or to legitimate
 *     values. The range for values is 0x0012 to 0x1000. The time
 *     calculated by taking the value * 0.625ms. It is an error
 *     if one is a default and the other is not. This is also true
 *     for "pageInterval" and "pageWindow".
 *
 *     Any time the scan interval or window is different from 
 *     the current settings in the radio, the radio will be 
 *     instructed to change to the new settings. This means that 
 *     if there are different settings for the connected state
 *     versus the non-connected state, the radio module will be 
 *     instructed to change the settings when the first connection 
 *     comes up and when the last connection goes down
 *     (automatically). This also means that if different values
 *     for window and interval are set when going from any setting
 *     of accessible to non-accessible then the radio will be 
 *     instructed to change. In most cases it is best to use
 *     the radio defaults. In this way the radio is never told
 *     to change the scan interval or window.
 *
 * Parameters:
 *     mode - desired accessibility mode
 *
 *     info - pointer to structure containing the inquiry and page
 *         scan interval and window to use. If info is 0 then the 
 *         defaults set by the radio module are used.
 *
 * Returns:
 *     BT_STATUS_PENDING - the mode is being set. All registered 
 *         global handlers with the BEM_ACCESSIBLE_CHANGE mask set will 
 *         receive BTEVENT_ACCESSIBLE_CHANGE event when the mode change 
 *         actually takes affect or an error has occurred. The "errCode"
 *         field of the BtEvent indicates the status of the operation. 
 *         If the operation is successful the "aMode" field of BtEvent
 *         indicates the new mode.  A BTEVENT_HCI_FATAL_ERROR event
 *         indicates a fatal radio or HCI transport error and that all
 *         pending operations have failed.
 *
 *     BT_STATUS_SUCCESS - Accessible mode is set. No event
 *         is sent out. This is returned if a connection exists and 
 *         the values are only saved or info already matches the current
 *         setting.
 *
 *     BT_STATUS_IN_PROGRESS - operation failed because a change
 *         is already in progress. Monitor the global events to 
 *         determine when the change has taken place.
 *
 *     BT_STATUS_INVALID_PARM - operation failed. The mode or info 
 *         parameter contains bad values (XA_ERROR_CHECK only)
 *
 *     BT_STATUS_HCI_INIT_ERR - operation failed because the HCI has
 *         an initialization error. Monitor the global events to
 *         be notified when the error has been corrected.
 */
BtStatus bt_set_accessible_mode_nc(BtAccessibleMode mode, const BtAccessModeInfo *info);

/*---------------------------------------------------------------------------
 * bt_set_accessible_mode_c()
 *
 *     Set the accessibility mode to be used when the device is 
 *     connected to one or more devices. If the mode is set to a 
 *     value other than BAM_NOT_ACCESSIBLE and there is a 
 *     connection to one or more devices then the Bluetooth radio 
 *     will enter inquiry and/or page scan mode using the 
 *     information passed in info. If info is 0 or the values in 
 *     info are set to defaults (BT_DEFAULT_SCAN_INTERVAL and 
 *     BT_DEFAULT_SCAN_WINDOW) the radio module default values 
 *     are used. It is assumed that the macro defaults
 *     match the radio defaults (see BT_DEFAULT_PAGE_SCAN_WINDOW
 *     documentation.) So, the first call to ME_SetAccessibleModeC
 *     with info set to 0 will not change the settings as the radio has
 *     already been initialized to its default settings. If there are
 *     no active connections then mode and info are saved and applied
 *     when the first connection comes up.  
 *
 *     To keep other devices from finding and connecting to this 
 *     device when connected set the mode to BAM_NOT_ACCESSIBLE. 
 *     The default mode when the stack is first loaded and initialized 
 *     is controlled by BT_DEFAULT_ACCESS_MODE_C.
 *
 *     In setting the values for info. Both "inqWindow" and
 *     "inqInterval" must be set to defaults or to legitimate
 *     values. The range for values is 0x0012 to 0x1000. The time
 *     is calculated by taking the value * 0.625ms. It is an error
 *     if one field (interval/window) is a default and the other is 
 *     not. This also true for "pageInterval" and "pageWindow".
 *
 *     Any time the scan interval or window is different from 
 *     the current settings in the radio, the radio will be 
 *     instructed to change to the new settings. This means that 
 *     if there are different settings for the connected state
 *     versus the non-connected state, the radio module will be 
 *     instructed to change the settings when the first connection 
 *     comes up and when the last connection goes down
 *     (automatically). This also means that if different values
 *     for window and interval are set when going from any setting
 *     of accessible to non-accessible then the radio will be 
 *     instructed to change. In most cases it is best to use
 *     the radio defaults. In this way the radio is never told
 *     to change the scan interval or window.
 *
 * Requires:
 *     BT_ALLOW_SCAN_WHILE_CON enabled.
 * 
 * Parameters:
 *     mode - desired accessibility mode
 *
 *     info - pointer to structure containing the inquiry and page
 *         scan interval and window to use. If info is 0 then the 
 *         defaults set by the radio module are used.
 *
 * Returns:
 *     BT_STATUS_PENDING - the mode is being set. All registered 
 *         global handlers with the BEM_ACCESSIBLE_CHANGE mask set will 
 *         receive BTEVENT_ACCESSIBLE_CHANGE event when the mode change 
 *         actually takes affect or an error has occurred. The "errCode"
 *         field of the BtEvent indicates the status of the operation. 
 *         If the operation is successful the "aMode" field of BtEvent
 *         indicates the new mode. A BTEVENT_HCI_FATAL_ERROR event
 *         indicates a fatal radio or HCI transport error and that all
 *         pending operations have failed.
 *
 *     BT_STATUS_SUCCESS - Accessible mode is set. No event
 *         is sent out. This is returned if no connections exist and 
 *         the values are only saved or info already matches the current
 *         setting.
 *
 *     BT_STATUS_IN_PROGRESS - operation failed because a change
 *         is already in progress. Monitor the global events to 
 *         determine when the change has taken place.
 *
 *     BT_STATUS_INVALID_PARM - operation failed. The mode or info 
 *         parameter contains bad values (XA_ERROR_CHECK only)
 *
 *     BT_STATUS_HCI_INIT_ERR - operation failed because the HCI has
 *         an initialization error. Monitor the global events to
 *         be notified when the error has been corrected.
 */
BtStatus bt_set_accessible_mode_c(BtAccessibleMode mode, const BtAccessModeInfo *info);

/*---------------------------------------------------------------------------
 * bt_start_inquiry()
 *
 *     Start a Bluetooth Inquiry procedure. If BT_STATUS_PENDING is
 *     returned then the results of the operation will be returned
 *     to all clients with registered global handlers. The following
 *     events will be sent to all registered handlers:
 *
 *     BTEVENT_INQUIRY_RESULT - indicates that a device was found. The
 *     "p.inqResult" field of the BtEvent contains the result.
 *
 *     BTEVENT_INQUIRY_COMPLETE - indicates that the inquiry process is
 *     complete. The "errCode" field of the BtEvent indicates whether 
 *     the operation completed without error.
 *
 *     BTEVENT_INQUIRY_CANCELED - this will be returned if the inquiry
 *     operation is canceled. BTEVENT_INQUIRY_COMPLETE will not be 
 *     returned.
 *
 *     BTEVENT_HCI_FATAL_ERROR - indicates fatal radio or HCI
 *     transport error. Assume all pending operations have failed.
 *     
 * Parameters:
 *
 *     length - Maximum amount of time before the Inquiry is
 *         halted. Range is 0x01 to 0x30. Time is length * 1.28 sec.
 *         The Generic Access profile specifies using the value
 *         BT_INQ_TIME_GAP100.
 *
 *     maxResp - The maximum number of responses. Specify zero to receive
 *         an unlimited number of responses.
 *
 * Returns:
 *     BT_STATUS_PENDING - The Inquiry process is started results
 *         will be sent via the handler. A BTEVENT_HCI_FATAL_ERROR event
 *         indicates a fatal radio or HCI transport error and that all
 *         pending operations have failed.
 *
 *     BT_STATUS_IN_PROGRESS - An inquiry process is already in 
 *         progress. Only one Inquiry can be in progress at a time.  
 *         Keep track of the general events to get the results from 
 *         the current Inquiry or to see when it ends. If it has just
 *         ended then a cancel is also in progress so wait for
 *         the cancel to complete to start another inquiry.
 *
 *     BT_STATUS_FAILED - The operation failed. 
 *
 *     BT_STATUS_HCI_INIT_ERR - operation failed because the HCI has
 *         an initialization error. Monitor the global events to
 *         be notified when the error has been corrected.
 */
BtStatus bt_start_inquiry(uint8_t Length, uint8_t maxResp);

/*---------------------------------------------------------------------------
 * bt_cancel_inquiry()
 *
 *     Cancel the current Inquiry process. When the inquiry
 *     operation is canceled all registered handlers will
 *     receive the BTEVENT_INQUIRY_CANCELED event. The "errCode"
 *     field of the BtEvent indicates the status of the
 *     operation.
 *
 * Returns:
 *     BT_STATUS_PENDING - The cancel operation was started
 *         successfully. The results will be sent to all clients
 *         with registered handlers. A BTEVENT_HCI_FATAL_ERROR event
 *         indicates a fatal radio or HCI transport error and that all
 *         pending operations have failed.
 *
 *     BT_STATUS_SUCCESS - The inquiry process was canceled
 *         immediately. It actually never was started. 
 *         BTEVENT_INQUIRY_CANCELED event will be sent to all handlers 
 *         before this function returns.
 *
 *     BT_STATUS_IN_PROGRESS - A cancel Inquiry is already in
 *         progress. Only one cancel can be in progress at a time.
 *         Keep track of the general events to see when the cancel
 *         is complete. 
 * 
 *     BT_STATUS_FAILED - The operation failed because an inquiry
 *         operation was not in progress.
 */
BtStatus bt_cancel_inquiry(void);

/*---------------------------------------------------------------------------
 * bt_start_sniff()
 *
 *     Start sniff mode for the ACL link specified by "remDev".
 *
 * Parameters:
 *     remDev - pointer to remote device
 *
 *     info - pointer to the sniff mode parameters. This structure may
 *            be freed after bt_start_sniff returns.
 *
 * Returns:
 *     BT_STATUS_PENDING - the operation was started successfully.
 *         All registered handlers and handlers bound to the remote device 
 *         will receive the BTEVENT_MODE_CHANGE event. The "errCode" field 
 *         of the BtEvent will indicate the success or failure of the mode 
 *         change event. The "p.modeChange" field indicates for which remote 
 *         Device the change has occurred along with the new mode and 
 *         interval. It is possible that link is disconnected before the
 *         mode change has occurred. In that case the handlers will not
 *         receive BTEVENT_MODE_CHANGE but instead will receive
 *         BTEVENT_LINK_DISCONNECT.
 *
 *     BT_STATUS_INVALID_PARM - Invalid parameter (XA_ERROR_CHECK only).
 *
 *     BT_STATUS_IN_PROGRESS - the operation failed because a mode
 *         change or disconnect operation is already in progress.
 *
 *     BT_STATUS_FAILED - the operation failed.
 *
 *     BT_STATUS_HCI_INIT_ERR - operation failed because the HCI has
 *         an initialization error. Monitor the global events to
 *         be notified when the error has been corrected.        
 */
BtStatus bt_start_sniff(void *remDev, const BtSniffInfo *info);

/*---------------------------------------------------------------------------
 * bt_stop_sniff()
 *
 *     Stop sniff mode and enter active mode for the ACL link
 *     specified by remDev. 
 *          
 * Parameters:
 *     remDev - pointer to remote device.
 *
 * Returns:
 *     BT_STATUS_PENDING - the operation was started successfully.
 *         All registered handlers and handlers bound to the remote device 
 *         will receive the BTEVENT_MODE_CHANGE event. The "errCode" field 
 *         of the BtEvent will indicate the success or failure of the mode 
 *         change event. The "p.modeChange" field indicates for which remote 
 *         Device the change has occurred along with the new mode and 
 *         interval. It is possible that link is disconnected before the
 *         mode change has occurred. In that case the handlers will not
 *         receive BTEVENT_MODE_CHANGE but instead will receive
 *         BTEVENT_LINK_DISCONNECT.
 *
 *     BT_STATUS_INVALID_PARM - the parameters are not valid
 *         (XA_ERROR_CHECK only).
 *
 *     BT_STATUS_IN_PROGRESS - the operation failed because a mode
 *         change or disconnect operation is already in progress.
 *
 *     BT_STATUS_FAILED - the operation failed. Device is not in
 *         sniff mode.
 *
 *     BT_STATUS_HCI_INIT_ERR - operation failed because the HCI has
 *         an initialization error. Monitor the global events to
 *         be notified when the error has been corrected.
 */
BtStatus bt_stop_sniff(void *remDev);

/*---------------------------------------------------------------------------
 * bt_switch_role()
 *
 *     Switch the current role the device is performing for the ACL link
 *     specified by remDev. If the current role is slave then switch to
 *     master. If the current role is master then switch to slave. The
 *     current role can be found via remDev.role. The result of the 
 *     operation will be returned via the BTEVENT_ROLE_CHANGE event.
 *
 * Parameters:
 *     remDev - pointer to remote device
 *
 * Returns:
 *     BT_STATUS_PENDING - the operation was started successfully.
 *         All registered handlers and handlers bound to the remote device 
 *         will receive the BTEVENT_ROLE_CHANGE event. The "errCode" field 
 *         of the BtEvent will indicate the success or failure of the role 
 *         change event. The "p.roleChange" field indicates for which remote 
 *         Device the change has occurred along with the new role.  It is 
 *         possible that link is disconnected before the role change has 
 *         occurred. In that case the handlers will not receive 
 *         BTEVENT_ROLE_CHANGE but instead will receive
 *         BTEVENT_LINK_DISCONNECT.
 *
 *     BT_STATUS_INVALID_PARM - the parameters are not valid
 *         (XA_ERROR_CHECK only).
 *
 *     BT_STATUS_IN_PROGRESS - the operation failed because a mode
 *         change or disconnect operation is already in progress.
 *
 *     BT_STATUS_FAILED - the operation failed. 
 *
 *     BT_STATUS_HCI_INIT_ERR - operation failed because the HCI has
 *         an initialization error. Monitor the global events to
 *         be notified when the error has been corrected.
 */
BtStatus bt_switch_role(void *remDev);

/*---------------------------------------------------------------------------
 * bt_get_last_device()
 *
 *     Called by the user to copy a BD_ADDR out of the database. The address
 *     is the last device saved in flash.
 *
 * Parameters:
  *     bdAddr - LAP used for the Inquiry (General or Limited)

 
 * Returns:
 *
 *     BT_STATUS_SUCCESS - Get the valid device successfully
 * 
 *     BT_STATUS_FAILED - No last device record in flash
 */
BtStatus bt_get_last_device(BD_ADDR *bdAddr);

/*---------------------------------------------------------------------------
 * bt_save_last_device()
 *
 *     Called by the user to add or replace the last device in flash.
 *     If the record indicates a BD_ADDR that is already in the
 *     database, this function should replace the existing device and
 *     increase the priority of the current device.
 *
 * Returns:
 *
 *     BT_STATUS_SUCCESS - The device is written to the flash successfully
 * 
 *     BT_STATUS_FAILED -  The device is failed to written to the flash
 */
BtStatus bt_save_last_device(const BD_ADDR *bdAddr);

/*---------------------------------------------------------------------------
 * bt_prevent_sniff_set()
 *
 *     Called by the user to prevent entering sniff mode 
 *
 * Parameters:
 *     flag - Bit to be set in the prevent sniff bit field
 *
 * Returns:
 *
 *     NULL
 */
void bt_prevent_sniff_set(uint8_t flag);

/*---------------------------------------------------------------------------
 * bt_prevent_sniff_clear()
 *
 *     Called by the user to allow entering sniff mode 
 *
 * Parameters:
 *     flag - Bit to be cleared in the prevent sniff bit field
 *
 * Returns:
 *
 *     NULL
 */
void bt_prevent_sniff_clear(uint8_t flag);


/*---------------------------------------------------------------------------
 * bt_remove_acl_link()
 *
 *     Called by the user to remove acl link,used in bt_disconnect 
 *
 * Parameters:
 *     NULL
 *
 * Returns:
 *
 *     NULL
 */
void bt_remove_acl_link(void *remdev);


/*---------------------------------------------------------------------------
 * bt_start_sniff_monitor()
 *
 *     Called by the user to start sniff timer when idle
 *
 * Parameters:
 *     active_to_sniff_time, uints:1s, monitor duration from active state to sniff state
 *
 * Returns:
 *
 *     NULL
 */
void bt_start_sniff_monitor(uint8_t active_to_sniff_time);

/*---------------------------------------------------------------------------
 * bt_stop_sniff_monitor()
 *
 *     Called by the user to stop sniff monitor
 *
 * Parameters:
 *     NULL
 *
 * Returns:
 *
 *     NULL
 */
void bt_stop_sniff_monitor(void);

/*---------------------------------------------------------------------------
 * bt_reset_controller()
 *
 *     Called by the user to reset bt controller
 *
 * Parameters:
 *     NULL
 *
 * Returns:
 *
 *     NULL
 */
void bt_reset_controller(void);

/*---------------------------------------------------------------------------
 * system_get_btdm_active_cnt()
 *
 *     Called by the user to get btdm cnt, it adds automatically when baseband running 
 *
 * Parameters:
 *     NULL
 *
 * Returns:
 *
 *     current counter value
 */
uint32_t system_get_btdm_active_cnt(void);

/*---------------------------------------------------------------------------
* system_set_btdm_active_cnt()
*
*     Called by the user to set btdm cnt, used to reset the cnt 
*
* Parameters:
*     cnt --- the count value to set
*
* Returns:
*
*     NULL
*/
void system_set_btdm_active_cnt(uint32_t cnt);


/*---------------------------------------------------------------------------
* bt_enter_bredr_testmode()
*
*     enter BREDR testmode,for bqb test only
*
* Parameters:
*     NULL
*
* Returns:
*
*     NULL
*/
void bt_enter_bredr_testmode(void);

/*---------------------------------------------------------------------------
* bt_enter_ble_testmode()
*
*     enter BLE testmode,for bqb test only
*
* Parameters:
*     uart_rx_io --- uart rx gpio(GPIO_PA0,GPIO_PA6,GPIO_PB6)
*     uart_tx_io --- uart tx gpio(GPIO_PA1,GPIO_PA7,GPIO_PB7)
*
* Returns:
*
*     NULL
*/
void bt_enter_ble_testmode(uint32_t uart_rx_io,uint32_t uart_tx_io);


/*---------------------------------------------------------------------------
* bt_enter_rx_sensitivity_testmode()
*
*     enter BREDR rx/tx testmode, only for sensitivity test
*
* Parameters:
*     NULL
*
* Returns:
*
*     NULL
*/
void bt_enter_rx_sensitivity_testmode(void);

/*---------------------------------------------------------------------------
* btdm_enter_tx_testmode()
*
*     enter ble tx testmode,
*
* Parameters:
*     type       0--- modulation tx, 1--- no modulation tx
*     channel    shall be even,in range [2,80]
*     pkt_type   tx packet type, 0---prbs9, other type refer to document
*     power      tx power in range [0,0x3f]
*
* Returns:
*
*     NULL
*/
void btdm_enter_tx_testmode(uint8_t type,uint8_t channel,uint8_t pkt_type,uint8_t power);

/*---------------------------------------------------------------------------
* btdm_exit_tx_testmode()
*
*     exit tx testmode
*
* Parameters:
*     NULL
*
* Returns:
*
*     NULL
*/
void btdm_exit_tx_testmode(void);

/*---------------------------------------------------------------------------
* bt_set_hf_source_mode()
*
*      set 5080 connecting mode, default mode: hfp&a2dp sink with phone, a2dp source with earphone
*      
*
* Parameters:
*     mode      0 --- hfp&a2dp sink with phone, a2dp source with earphone
*               1 --- hfp with phone, a2dp source with earphone, shall change a2dp type when switch connecting
*                     device type
*       
* Returns:
*
*     NULL
*/
void bt_set_hf_source_mode(uint8_t mode);

/*---------------------------------------------------------------------------
* bt_get_hf_source_mode()
*
*     get 5080 connecting mode
*      
*
* Parameters:
*     NULL
*       
* Returns:
*
*     mode
*/
uint8_t bt_get_hf_source_mode(void);

//read remote name
void bt_rd_rem_name(void);


/*---------------------------------------------------------------------------
* bt_set_a2dp_decorder_type()
*
*      set 5080 a2dp decorder type, default type: sbc hw decorder
*      
*
* Parameters:
*     type      0 --- a2dp hw sbc decorder
*               1 --- a2dp dsp sbc decorder
*       
* Returns:
*
*     NULL
*/
void bt_set_a2dp_decorder_type(uint8_t type);

/*---------------------------------------------------------------------------
* bt_get_a2dp_decorder_type()
*
*     get a2dp decorder type
*      
*
* Parameters:
*     NULL
*       
* Returns:
*
*     type
*/
uint8_t bt_get_a2dp_decorder_type(void);

typedef void(* bt_HID_check_conn)(unsigned char dev_num);

extern bt_HID_check_conn bt_hid_check_conn;

/*---------------------------------------------------------------------------
* bt_delete_device()
*
*     delete device from linkkey table
*      
*
* Parameters:
*     addr - device address to delete
*       
* Returns:
*
*     NULL
*/
void bt_delete_device(BD_ADDR *addr);

#endif

