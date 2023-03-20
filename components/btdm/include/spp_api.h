#ifndef _SPP_API_H
#define _SPP_API_H

/*---------------------------------------------------------------------------
 * SppEvent type
 *
 *     The application is notified of various indications and confirmations 
 *     through a callback function.  Depending on the event, different 
 *     elements of the SppCallbackParms "SppCallbackParms.p" union will be 
 *     valid.
 */
typedef uint16_t SppEvent;

/** A connection has been established with a remote device.  
 *

 */
#define SPP_EVENT_REMDEV_CONNECTED    0

/** A connection has been terminated for a remote device.  
 * 
 */
#define SPP_EVENT_REMDEV_DISCONNECTED 1


/**Data has been sent successfully, the "event.data" field point to the sent data 
 */
#define SPP_EVENT_SEND_COMPLETE       2

/**Data has been received, the "event.data" and "event.datalen" point to the received
 *data and datalen
 */
#define SPP_EVENT_DATA_IND            3

/* End of SppEvent */ 

typedef struct {
    SppEvent            event;   /* Event associated with the callback */ 

    BtStatus            status;  /* Status of the callback event       */ 

    uint16_t datalen;            /* received data length */
    
    void *data;                 /* received or sent data pointer*/
}spp_event_t;

typedef void(* spp_callback_func_t)(spp_event_t * event);
void bt_spp_set_cb_func(spp_callback_func_t func);

/*---------------------------------------------------------------------------
 * spp_connect()
 *
 *     Initiates a connection with a remote spp server. 
 *
 * Parameters:
 *
 *     addr -  The Bluetooth address of the device to which the connection 
 *             should be made. If 0, the connection manager is used 
 *             to select an appropriate device.  
 *
 * Returns:
 *
 *     BT_STATUS_PENDING - The connection process has been successfully 
 *         started. When the connection process is complete, the 
 *         application callback will receive either the AVRCP_EVENT_CONNECT 
 *         or AVRCP_EVENT_DISCONNECT event.
 *
 *     BT_STATUS_IN_USE - This channel is already connected or is in the 
 *         process of connecting.  
 *
 *     BT_STATUS_INVALID_PARM - The chnl parameter does not contain a 
 *         valid pointer. (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NOT_FOUND - The specified device was not found in the device 
 *         selector database.  The device must be discovered, paired, or 
 *         added manually using DS_AddDevice().  
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).  
 */
BtStatus spp_connect(BD_ADDR *addr);

/*---------------------------------------------------------------------------
 * spp_disconnect()
 *
 *     Terminates a connection with a remote spp server. 
 *
 * Parameters:
 *
 *     None
 *
 * Returns:
 *
 *     BT_STATUS_PENDING - The connection process has been successfully 
 *         started. When the connection process is complete, the 
 *         application callback will receive either the AVRCP_EVENT_CONNECT 
 *         or AVRCP_EVENT_DISCONNECT event.
 *
 *     BT_STATUS_IN_USE - This channel is already connected or is in the 
 *         process of connecting.  
 *
 *     BT_STATUS_INVALID_PARM - The chnl parameter does not contain a 
 *         valid pointer. (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NOT_FOUND - The specified device was not found in the device 
 *         selector database.  The device must be discovered, paired, or 
 *         added manually using DS_AddDevice().  
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).  
 */
BtStatus spp_disconnect(void);

/*---------------------------------------------------------------------------
 * spp_send_data()
 *
 *     Send data to a remote spp server. 
 *
 * Parameters:
 *
 *     data     the data pointer
 *     datalen  data length
 *
 * Returns:
 *
 *     BT_STATUS_PENDING - The connection process has been successfully 
 *         started. When the connection process is complete, the 
 *         application callback will receive either the AVRCP_EVENT_CONNECT 
 *         or AVRCP_EVENT_DISCONNECT event.
 *
 *     BT_STATUS_INVALID_PARM - The chnl parameter does not contain a 
 *         valid pointer. (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NOT_FOUND - The specified device was not found in the device 
 *         selector database.  The device must be discovered, paired, or 
 *         added manually using DS_AddDevice().  
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).  
 */
BtStatus spp_send_data(uint8_t *data, uint16_t datalen);


#endif
