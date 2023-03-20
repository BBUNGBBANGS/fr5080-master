#ifndef _AVRCP_API_H
#define _AVRCP_API_H

/* The number of enumeration elements 
 * defined for the AvrcpMediaAttrId type.  
 */ 
#define AVRCP_NUM_MEDIA_ATTRIBUTES        7

/*---------------------------------------------------------------------------
 * AvrcpEvent type
 *      
 */
typedef uint8_t AvrcpEvent;

/* Group: Connection events for establishing and releasing the AVRCP control
 * channel.  The control channel is used for the exchange of basic AV/C
 * commands/responses and for the exchange of AV/C commands directed to the
 * panel subunit (which includes advanced Bluetooth specific commands).
 */

/** The transport layer is connected and commands/responses can now
 *  be exchanged.
 * 
 *  During this callback, the 'p.remDev' parameter is valid.
 */
#define AVRCP_EVENT_CONNECT             1

/** The application will receive this event when a lower layer connection
 *  (L2CAP) has been disconnected.  Both the target and controller of the
 *  connection are notified.
 *  
 *  During this callback, the 'p.remDev' parameter is valid.
 */
#define AVRCP_EVENT_DISCONNECT          3

/** A remote device is attempting to connect the transport layer.
 *  Only the acceptor of the connection is notified.  The acceptor may
 *  call AVRCP_ConnectRsp() to either accept or reject the connection.
 * 
 *  During this callback, the 'p.remDev' parameter is valid.
 */
#define AVRCP_EVENT_CONNECT_IND         2

/* Group: Events for the exchange of basic AV/C commands that are not routed
 * to the panel subunit.
 */

/** A AV/C command was received from the remote device (controller). This
 *  event is received for commands not routed to the panel subunit.
 * 
 *  During this callback, the 'p.cmdFrame' parameter is valid. It contains the
 *  the AVRCP command header information, including operands. If the "more"
 *  value is TRUE then this event only signals the first part of the operands.
 *  Subsequent AVRCP_EVENT_OPERANDS events will follow this event with
 *  additional operand data.
 *
 *  Note that the AVRCP specification requires that target devices respond to
 *  commands within 100ms after receiving the command.
 */
#define AVRCP_EVENT_COMMAND             4

/** A AV/C response was received from the remote device (target). This event
 *  is received for responses not routed to the panel subunit.
 * 
 * During this callback, the 'p.rspFrame' parameter is valid. It contains the
 * the AVRCP response header information, including operands. If the "more"
 * value is TRUE then this event only signals the first part of the operands.
 * Subsequent AVRCP_EVENT_OPERANDS events will follow this event with
 * additional operand data.
 */
#define AVRCP_EVENT_RESPONSE            5

/** The remote device (target) rejected the AV/C command.  This event is
 *  received for responses not routed to the panel subunit.
 * 
 * During this callback, the 'p.rspFrame' parameter is valid. It contains the
 * the AVRCP reject header information, including operands. If the "more"
 * value is TRUE then this event only signals the first part of the operands.
 * Subsequent AVRCP_EVENT_OPERANDS events will follow this event with 
 * additional operand data. 
 */
#define AVRCP_EVENT_REJECT              6

/** A command (see AVRCP_SendCommand) or response (see AVRCP_SendResponse)
 * has been sent. Memory allocated for the operation can be freed or reused
 * after receiving this event.  This event is received for commands or 
 * responses not routed to the panel subunit. 
 * 
 * During this callback, the 'p.cmdFrame' or 'p.rspFrame' parameter associated
 * with the sent command or response is valid. They contain a pointer to the 
 * AV/C comand or response that was sent.  In addition, "status" will be set 
 * to "BT_STATUS_SUCCESS" or "BT_STATUS_FAILED" to indicate whether 
 * the event was properly delivered. 
 */
#define AVRCP_EVENT_TX_DONE             8

/** Additional operand data has been received for the previous
 *  AVRCP_EVENT_COMMAND or AVRCP_EVENT_RESPONSE.  This event is received for
 *  commands or responses not routed to the panel subunit.
 * 
 * During this callback, the 'p.cmdFrame' or 'p.rspFrame' parameter associated
 * with the received command or response is valid. The "operands" and
 * "operandLen" fields indicate the chunk of operands being received for
 * the command or response. If the "more" field is set to TRUE, the full
 * operand buffer will be received in multiple _OPERANDS events and the
 * last operand buffer indicated with the "more" field set to FALSE.
 */
#define AVRCP_EVENT_OPERANDS            9

/** An AV/C command has timed out.  Memory allocated for the command can be
 * freed or reused after receiving this event.  This event is received for 
 * commands not routed to the panel subunit. 
 * 
 * During this callback, the 'p.cmdFrame' parameter is valid. It contains a 
 * pointer to the the AV/C command that was sent. 
 */
#define AVRCP_EVENT_CMD_TIMEOUT         (10)

/* Group: Events for the exchange of standard AV/C panel subunit commands.
 */ 

/** The key corresponding to a panel operation has been pressed on the
 * remote controller device. See p.panelInd for information about the
 * operation.
 *
 * AVRCP_RejectPanelOperation may be used to reject an unsupported or
 * reserved command. If the operation is not rejected during the callback
 * it is implicitly accepted. If accepted, the next panel-related event
 * for the operation will be AVRCP_EVENT_PANEL_HOLD (if the controller key
 * is held down for at least AVRCP_PANEL_PRESSHOLD_TIME) or
 * AVRCP_EVENT_PANEL_RELEASE (if the controller key is released more quickly).
 */
#define AVRCP_EVENT_PANEL_PRESS        (11)

/** The key corresponding to a panel operation has been held down on the
 * remote controller device for at least AVRCP_PANEL_PRESSHOLD_TIME.
 * See p.panelInd for information about the operation.
 *
 * A target receiving this event should act as if the key was held down until
 * the AVRCP_EVENT_PANEL_RELEASE event is received.
 */
#define AVRCP_EVENT_PANEL_HOLD         (12)

/** The key corresponding to a panel operation has been released on the
 * remote controller device.  See p.panelInd for information about the
 * operation.
 */
#define AVRCP_EVENT_PANEL_RELEASE      (13)

/** A panel response has been received from the remote target device.
 * All fields of "p.panelCnf" are valid.
 */
#define AVRCP_EVENT_PANEL_CNF          (14)

/* Group: Events for the exchange of Bluetooth specific advanced AV/C
 * commands routed through the panel subunit.  These event are used primarily
 * for the exchange of media information (metadata) and player settings.
 */ 

/** Advanced status information has been received from the controller. During
 *  this callback, "advOp" is set to the operation type that was received
 *  from the controller, and "p.adv.info" contains the relevant information
 *  based on the operation type.  A response was automatically sent to the
 *  controller.
 */
#define AVRCP_EVENT_ADV_INFO           (15)

/** An advanced request was received and requires a response by the
 *  application.  During this callback, "advOp" is set to the operation type
 *  that was received from the controller, and "p.adv.req" contains relevant
 *  information based on the operation type.  A response should be sent
 *  quickly to avoid a timeout on the controller (see
 *  AVRCP_TgSendAdvResponse()).
 */
#define AVRCP_EVENT_ADV_REQ            (16)

/** The notification of a registered event has been received.
 *  During this callback, "p.adv.notify" contains the relevant notification
 *  information.  If "errorCode" indicates that no error occured, the 
 *  "p.adv.notify.event" field contains the type of notification, 
 *  and "p.adv.notify.value" contains the value of the notification.
 */
#define AVRCP_EVENT_ADV_NOTIFY         (17)

/** An Advanced command was sent successfully to the target and a response
 *  was received.  During this callback, "advOp" is set to the operation type
 *  that was sent to the target, and "p.adv.rsp" contains the relevant
 *  response information based on the operation type.
 */
#define AVRCP_EVENT_ADV_RESPONSE       (18)

/** No response was received for an Advanced command and it has timed out.
 *  This event is received for advanced commands routed to the panel subunit.
 * 
 *  During this callback, the 'p.cmdFrame' parameter is valid. It contains
 *  the the Advanced command that was sent. 
 */
#define AVRCP_EVENT_ADV_CMD_TIMEOUT    (19)

/** An Advanced PDU was sent successfully.  The memory used by the PDU can be
 *  freed or reused. During this callback, "p.advTxPdu" points to the memory
 *  used by the PDU.  The operation associated with the PDU is contained in
 *  "p.advTxPdu->op", and the associated parameters are in
 *  "p.advTxPdu->parms".  The transmitted PDU may have been sent in either an
 *  advanced command or response.  The "status" field contains the results of
 *  the transmission.
 */
#define AVRCP_EVENT_ADV_TX_DONE        (20)

#if 0
/* Group: Connection events for establishing and releasing the AVRCP browsing
 * channel.  The browsing channel is used for the exchange of AVCTP-based
 * commands/responses which contain Bluetooth specific media
 * operations.
 */
/** The transport layer is connected and commands/responses can now
 *  be exchanged on the browsing channel.
 * 
 *  During this callback, the 'p.remDev' parameter is valid.
 */
#define AVRCP_EVENT_BROWSE_CONNECT     (AVCTP_EVENT_LAST + 12)

/** The application will receive this event when a lower layer connection
 *  (L2CAP) has been disconnected.  Both the target and controller of the
 *  connection are notified.
 *  
 *  During this callback, the 'p.remDev' parameter is valid.
 */
#define AVRCP_EVENT_BROWSE_DISCONNECT  (AVCTP_EVENT_LAST + 13)

/** A remote device is attempting to connect the browsing channel. Only the
 *  acceptor of the connection is notified.  The acceptor may call
 *  AVRCP_BrowseConnectRsp() to either accept or reject the connection.
 * 
 *  During this callback, the 'p.remDev' parameter is valid.
 */
#define AVRCP_EVENT_BROWSE_CONNECT_IND (AVCTP_EVENT_LAST + 14)

/* Group: Events for the exchange of browsing channel commands.  The browsing
 * channel is used for the exchange of AVCTP-based commands/responses which
 * contain Bluetooth specific media operations.
 */

/** An browsing request was received and requires a response by the
 *  application. During this callback, "advOp" is set to the operation type
 *  that was received from the controller, and "p.adv.browseReq" contains
 *  relevant information based on the operation type.  A response should be
 *  sent quickly to avoid a timeout on the controller.
 */
#define AVRCP_EVENT_BROWSE_REQ         (AVCTP_EVENT_LAST + 15)

/** Advanced status information has been received from the controller. During
 *  this callback, "advOp" is set to the operation type that was received
 *  from the controller, and "p.adv.browseInfo" contains the relevant
 *  information based on the operation type.  A response was automatically
 *  sent to the controller.
 */
#define AVRCP_EVENT_BROWSE_INFO        (AVCTP_EVENT_LAST + 16)

/** An browsing command was sent successfully to the target and a response
 *  was received.  During this callback, "advOp" is set to the operation type
 *  that was sent to the target, and "p.adv.browseRsp" contains the relevant
 *  response information based on the operation type.
 */
#define AVRCP_EVENT_BROWSE_RESPONSE    (AVCTP_EVENT_LAST + 17)

/** No response was received for a Browsing command and it has timed out.
 *  This event is received for browsing commands only.
 * 
 *  During this callback, the 'p.cmdFrame' parameter is valid. It contains
 *  the the Browsing command that was sent. 
 */
#define AVRCP_EVENT_BROWSE_CMD_TIMEOUT (AVCTP_EVENT_LAST + 18)

/** A browsing PDU was sent successfully.  The memory used by the PDU can be
 *  freed or reused. During this callback, "p.advTxPdu" points to the memory
 *  used by the PDU.  The transmitted PDU may have been sent in either an
 *  browsing command or response.  The "status" field contains the results of
 *  the transmission.
 */
#define AVRCP_EVENT_BROWSE_TX_DONE     (AVCTP_EVENT_LAST + 19)
#endif

#define AVRCP_EVENT_LAST               (28)

/* End of AvrcpEvent */ 


/*---------------------------------------------------------------------------
 * AvrcpOperation type
 *
 *     AVRCP Bluetooth specific operations that may be sent/received by a
 *     controller or target.  Some operations are Bluetooth specific AV/C
 *     commands sent over the control channel and routed through the panel
 *     subunit, and others are browsing commands sent over the browsing
 *     channel.
 * 
 *     Each operation is associated with an API function and/or event. The
 *     function name and the events and parameters used in the callback
 *     function are described below.
 */
typedef uint8_t AvrcpOperation;

/* Associated Function:  AVRCP_CtGetCapabilities()
 * 
 * When 'advOp' is set to AVRCP_OP_GET_CAPABILITIES during the
 * AVRCP_EVENT_ADV_RESPONSE event, the 'p.adv.rsp.capability' field contains
 * the results of the operation.
 */ 
#define AVRCP_OP_GET_CAPABILITIES               0x10

#if 0
/* Associated Function:  AVRCP_CtListPlayerSettingAttrs()
 * 
 * When 'advOp' is set to AVRCP_OP_LIST_PLAYER_SETTING_ATTRIBS during the
 * AVRCP_EVENT_ADV_RESPONSE event, the 'p.adv.rsp.attrMask' field contains
 * the results of the operation.
 */ 
#define AVRCP_OP_LIST_PLAYER_SETTING_ATTRIBS    0x11

/* Associated Function:  AVRCP_CtListPlayerSettingValues() 
 * 
 * When 'advOp' is set to AVRCP_OP_LIST_PLAYER_SETTING_VALUES during the 
 * AVRCP_EVENT_ADV_RESPONSE event, the 'p.adv.rsp.attrValues' field contains
 * the results of the operation.
 */ 
#define AVRCP_OP_LIST_PLAYER_SETTING_VALUES     0x12

/* Associated Function:  AVRCP_CtGetPlayerSettingValues()
 * 
 * When 'advOp' is set to AVRCP_OP_GET_PLAYER_SETTING_VALUE during the
 * AVRCP_EVENT_ADV_RESPONSE event, the 'p.adv.rsp.setting' field contains the
 * results of the operation.
 */ 
#define AVRCP_OP_GET_PLAYER_SETTING_VALUE       0x13

/* Associated Function:  AVRCP_CtSetPlayerSettingValues()
 * 
 * When 'advOp' is set to AVRCP_OP_SET_PLAYER_SETTING_VALUE during the
 * AVRCP_EVENT_ADV_INFO event, the 'p.adv.info.setting' field contains the
 * results of the operation.
 */ 
#define AVRCP_OP_SET_PLAYER_SETTING_VALUE       0x14

/* Associated Function:  AVRCP_CtGetPlayerSettingAttrTxt()
 * 
 * When 'advOp' is set to AVRCP_OP_GET_PLAYER_SETTING_ATTR_TEXT during the
 * AVRCP_EVENT_ADV_RESPONSE event, the 'p.adv.rsp.attrStrings' field contains
 * the results of the operation.
 */ 
#define AVRCP_OP_GET_PLAYER_SETTING_ATTR_TEXT   0x15

/* Associated Function:  AVRCP_CtGetPlayerSettingValueTxt()
 * 
 * When 'advOp' is set to AVRCP_OP_GET_PLAYER_SETTING_VALUE_TEXT during the
 * AVRCP_EVENT_ADV_RESPONSE event, the 'p.adv.rsp.settingStrings' field
 * contains the results of the operation.
 */ 
#define AVRCP_OP_GET_PLAYER_SETTING_VALUE_TEXT  0x16

/* Associated Function:  AVRCP_CtInformCharset()
 * 
 * When 'advOp' is set to AVRCP_OP_INFORM_DISP_CHAR_SET during the 
 * AVRCP_EVENT_ADV_INFO event, the 'p.adv.info.charSet' field contains the
 * results of the operation.
 */ 
#define AVRCP_OP_INFORM_DISP_CHAR_SET           0x17

#endif

/* Associated Function:  AVRCP_CtGetMediaInfo()
 * 
 * When 'advOp' is set to AVRCP_OP_GET_MEDIA_INFO during the
 * AVRCP_EVENT_ADV_RESPONSE event, the 'p.adv.rsp.element' field contains the
 * results of the operation.
 */ 
#define AVRCP_OP_GET_MEDIA_INFO                 0x20



/* Associated Function:  AVRCP_CtGetPlayStatus()
 * 
 * When 'advOp' is set to AVRCP_OP_GET_PLAY_STATUS during the
 * AVRCP_EVENT_ADV_INFO event, the 'p.adv.info.charSet' field contains the
 * results of the operation.
 */ 
#define AVRCP_OP_GET_PLAY_STATUS                0x30

/* Associated Function:  AVRCP_CtRegisterNotification()
 * 
 * When 'advOp' is set to AVRCP_OP_REGISTER_NOTIFY during the
 * AVRCP_EVENT_ADV_RESPONSE event, or when the AVRCP_EVENT_ADV_NOTIFY event
 * is received, the 'p.adv.notify' field contains the response or parameters
 * for the operation.
 */ 
#define AVRCP_OP_REGISTER_NOTIFY                0x31


/* Associated Function:  AVRCP_CtSetAbsoluteVolume()
 * 
 * When 'advOp' is set to AVRCP_OP_SET_ABSOLUTE_VOLUME during the
 * AVRCP_EVENT_ADV_INFO event, the 'p.info.volume' field contains the
 * results of the operation.
 */ 
#define AVRCP_OP_SET_ABSOLUTE_VOLUME            0x50


#if 0
/* Associated Function:  AVRCP_CtSetAddressedPlayer()
 * 
 * When 'advOp' is set to AVRCP_OP_SET_ADDRESSED_PLAYER during the
 * AVRCP_EVENT_ADV_INFO event, the 'p.info.addrPlayer' field contains the
 * results of the operation.
 */ 
#define AVRCP_OP_SET_ADDRESSED_PLAYER           0x60


/* Associated Function:  AVRCP_CtPlayItem()
 * 
 * When 'advOp' is set to AVRCP_OP_PLAY_ITEM during the AVRCP_EVENT_ADV_REQ
 * event, the 'p.adv.req.item' field contains the browsing request (a
 * response is expected).
 */ 
#define AVRCP_OP_PLAY_ITEM                      0x74


/* Associated Function:  AVRCP_CtAddToNowPlaying()
 * 
 * When 'advOp' is set to AVRCP_OP_ADD_TO_NOW_PLAYING during the
 * AVRCP_EVENT_ADV_REQ event, the 'p.adv.req.item' field contains the
 * browsing request (a response is expected).
 */ 
#define AVRCP_OP_ADD_TO_NOW_PLAYING             0x90
#endif
/* End of AvrcpOperation */ 

/*---------------------------------------------------------------------------
 * AvrcpEventMask
 *
 * Bitmask of supported AVRCP events.  By default, only 
 * AVRCP_ENABLE_PLAY_STATUS_CHANGED and AVRCP_ENABLE_TRACK_CHANGED are 
 * enabled when a channel is registered.  The application must explicitly
 * enable any other supported events.

 */
typedef uint16_t AvrcpEventMask;

#define AVRCP_ENABLE_PLAY_STATUS_CHANGED      0x0001  /* Change in playback 
                                                       * status 
                                                       */ 
#define AVRCP_ENABLE_MEDIA_STATUS_CHANGED     0x0001  /* Alias */
#define AVRCP_ENABLE_TRACK_CHANGED            0x0002  /* Current track changed */ 
#define AVRCP_ENABLE_TRACK_END                0x0004  /* Reached end of track  */ 
#define AVRCP_ENABLE_TRACK_START              0x0008  /* Reached track start   */ 
#define AVRCP_ENABLE_PLAY_POS_CHANGED         0x0010  /* Change in playback 
                                                       * position 
                                                       */ 
#define AVRCP_ENABLE_BATT_STATUS_CHANGED      0x0020  /* Change in battery 
                                                       * status 
                                                       */ 
#define AVRCP_ENABLE_SYS_STATUS_CHANGED       0x0040  /* Change in system status */ 
#define AVRCP_ENABLE_APP_SETTING_CHANGED      0x0080  /* Change in player 
                                                       * application setting
                                                       */ 


#define AVRCP_ENABLE_NOW_PLAYING_CHANGED      0x0100  /* Change in the now 
                                                       * playing list 
                                                       */
#define AVRCP_ENABLE_AVAIL_PLAYERS_CHANGED    0x0200  /* Available players
                                                       * changed 
                                                       */
#define AVRCP_ENABLE_ADDRESSED_PLAYER_CHANGED 0x0400  /* Addressed player changed */
#define AVRCP_ENABLE_UIDS_CHANGED             0x0800  /* UIDS changed */
#define AVRCP_ENABLE_VOLUME_CHANGED           0x1000  /* Volume Changed */

/* End of AvrcpEventMask */ 

/*---------------------------------------------------------------------------
 * AvrcpMediaStatus type
 *
 * Defines play status of the currently playing media.
 */
typedef uint8_t AvrcpMediaStatus;

#define AVRCP_MEDIA_STOPPED       0x00
#define AVRCP_MEDIA_PLAYING       0x01
#define AVRCP_MEDIA_PAUSED        0x02
#define AVRCP_MEDIA_FWD_SEEK      0x03
#define AVRCP_MEDIA_REV_SEEK      0x04
#define AVRCP_MEDIA_ERROR         0xFF

/* End of AvrcpMediaStatus */ 

/*---------------------------------------------------------------------------
 * AvrcpCapabilityId type
 *
 * Defines the capability ID for the AVRCP_OP_GET_CAPABILITIES operation.
 */
typedef uint8_t AvrcpCapabilityId;

#define AVRCP_CAPABILITY_COMPANY_ID        2
#define AVRCP_CAPABILITY_EVENTS_SUPPORTED  3

/* End of AvrcpCapabilityId */ 

/*---------------------------------------------------------------------------
 * AvrcpMediaAttr structure
 * 
 * Used to describe media info attributes.
 */
typedef struct _AvrcpMediaAttr {
    uint32_t         attrId;
    uint16_t         charSet;
    uint16_t         length;
    const char *string;
} AvrcpMediaAttr;

/*---------------------------------------------------------------------------
 * AvrcpRspParms structure
 * 
 * Defines the callback parameters received during the
 * AVRCP_EVENT_ADV_RESPONSE event, not including AVRCP_OP_REGISTER_NOTIFY
 * operation (see AvrcpNotifyParms).
 */
typedef union _AvrcpAdvRspParms {
    /* The capabilities of the target.  
     * This is valid when "advOp" is set to AVRCP_OP_GET_CAPABILITIES.
     */ 
    struct {

        /* The type of capability. */ 
        AvrcpCapabilityId type;

        /* The capability info. */ 
        union {

            /* The list of company IDs.  
             * (type == AVRCP_CAPABILITY_COMPANY_ID) 
             */ 
            struct {

                /* The number of supported company IDs. */ 
                uint8_t  numIds;

                /* An array of company IDs (3 bytes each). */ 
                uint8_t *ids;

            } companyId;

            /* A bitmask of the supported events. 
             * (type == AVRCP_CAPABILITY_EVENTS_SUPPORTED)
             */ 
            uint16_t  eventMask;

        } info;

    } capability;
    
    /* The list of element values for the current track on the 
     * target.  This is valid when "advOp" is set to AVRCP_OP_GET_MEDIA_INFO.
     */ 
    struct {

        /* The number of elements returned */ 
        uint8_t numIds;

        /* An array of element value text information */ 
        AvrcpMediaAttr txt[AVRCP_NUM_MEDIA_ATTRIBUTES];

    } element;

    /* The playback status of the current track.
     * This is valid when "advOp" is set to AVRCP_OP_GET_PLAY_STATUS.
     */ 
    struct {
        uint32_t               length;
        uint32_t               position;
        AvrcpMediaStatus  mediaStatus;
    } playStatus;

    /* The Absolute Volume 
     * This is valid when "advOp" is set to AVRCP_OP_SET_ABSOLUTE_VOLUME.
     */
    uint8_t volume;
} AvrcpAdvRspParms; 

/*---------------------------------------------------------------------------
 * AvrcpEventId
 *
 *     AVRCP events for the AVRCP_REGISTER_NOTIFY operation.
 */
typedef uint8_t AvrcpEventId;

#define AVRCP_EID_MEDIA_STATUS_CHANGED        0x01  /* Change in media status */ 
                                             
#define AVRCP_EID_TRACK_CHANGED               0x02  /* Current track changed */ 
                                             
#define AVRCP_EID_TRACK_END                   0x03  /* Reached end of track */ 
                                             
#define AVRCP_EID_TRACK_START                 0x04  /* Reached track start */ 
                                             
#define AVRCP_EID_PLAY_POS_CHANGED            0x05  /* Change in playback position.  
                                                     * Returned after the specified 
                                                     * playback notification change 
                                                     * notification interval. 
                                                     */ 
                                             
#define AVRCP_EID_BATT_STATUS_CHANGED         0x06  /* Change in battery status   */ 
#define AVRCP_EID_SYS_STATUS_CHANGED          0x07  /* Change in system status    */ 
#define AVRCP_EID_APP_SETTING_CHANGED         0x08  /* Change in player 
                                                     * application setting
                                                     */

#define AVRCP_EID_NOW_PLAYING_CONTENT_CHANGED 0x09 /* Contents of the now playing 
                                                    * list have changed 
                                                    */


#define AVRCP_EID_AVAILABLE_PLAYERS_CHANGED   0x0A /* The available players have
                                                    * changed 
                                                    */

#define AVRCP_EID_ADDRESSED_PLAYER_CHANGED    0x0B /* The addressed player has 
                                                    * changed 
                                                    */

#define AVRCP_EID_UIDS_CHANGED                0x0C /* The UIDS have changed */

#define AVRCP_EID_VOLUME_CHANGED              0x0D /* The volume was changed */

#define AVRCP_EID_FLAG_INTERIM                0x80 /* Used Internally */

/* End of AvrcpEventId */

/*---------------------------------------------------------------------------
 * AvrcpPanelOperation type
 *
 *     Panel subunit operations that may be sent (by a controller) or
 *     received (by a target). These codes are defined by the 1394
 *     AV/C Panel Subunit Specification (version 1.1).
 */
typedef uint16_t AvrcpPanelOperation;

#define AVRCP_POP_SELECT            0x0000
#define AVRCP_POP_UP                0x0001
#define AVRCP_POP_DOWN              0x0002
#define AVRCP_POP_LEFT              0x0003
#define AVRCP_POP_RIGHT             0x0004
#define AVRCP_POP_RIGHT_UP          0x0005
#define AVRCP_POP_RIGHT_DOWN        0x0006
#define AVRCP_POP_LEFT_UP           0x0007
#define AVRCP_POP_LEFT_DOWN         0x0008
#define AVRCP_POP_ROOT_MENU         0x0009
#define AVRCP_POP_SETUP_MENU        0x000A
#define AVRCP_POP_CONTENTS_MENU     0x000B
#define AVRCP_POP_FAVORITE_MENU     0x000C
#define AVRCP_POP_EXIT              0x000D

#define AVRCP_POP_0                 0x0020
#define AVRCP_POP_1                 0x0021
#define AVRCP_POP_2                 0x0022
#define AVRCP_POP_3                 0x0023
#define AVRCP_POP_4                 0x0024
#define AVRCP_POP_5                 0x0025
#define AVRCP_POP_6                 0x0026
#define AVRCP_POP_7                 0x0027
#define AVRCP_POP_8                 0x0028
#define AVRCP_POP_9                 0x0029
#define AVRCP_POP_DOT               0x002A
#define AVRCP_POP_ENTER             0x002B
#define AVRCP_POP_CLEAR             0x002C

#define AVRCP_POP_CHANNEL_UP        0x0030
#define AVRCP_POP_CHANNEL_DOWN      0x0031
#define AVRCP_POP_PREVIOUS_CHANNEL  0x0032
#define AVRCP_POP_SOUND_SELECT      0x0033
#define AVRCP_POP_INPUT_SELECT      0x0034
#define AVRCP_POP_DISPLAY_INFO      0x0035
#define AVRCP_POP_HELP              0x0036
#define AVRCP_POP_PAGE_UP           0x0037
#define AVRCP_POP_PAGE_DOWN         0x0038

#define AVRCP_POP_POWER             0x0040
#define AVRCP_POP_VOLUME_UP         0x0041
#define AVRCP_POP_VOLUME_DOWN       0x0042
#define AVRCP_POP_MUTE              0x0043
#define AVRCP_POP_PLAY              0x0044
#define AVRCP_POP_STOP              0x0045
#define AVRCP_POP_PAUSE             0x0046
#define AVRCP_POP_RECORD            0x0047
#define AVRCP_POP_REWIND            0x0048
#define AVRCP_POP_FAST_FORWARD      0x0049
#define AVRCP_POP_EJECT             0x004A
#define AVRCP_POP_FORWARD           0x004B
#define AVRCP_POP_BACKWARD          0x004C

#define AVRCP_POP_ANGLE             0x0050
#define AVRCP_POP_SUBPICTURE        0x0051

#define AVRCP_POP_F1                0x0071
#define AVRCP_POP_F2                0x0072
#define AVRCP_POP_F3                0x0073
#define AVRCP_POP_F4                0x0074
#define AVRCP_POP_F5                0x0075

#define AVRCP_POP_VENDOR_UNIQUE     0x007E

#define AVRCP_POP_NEXT_GROUP        0x017E
#define AVRCP_POP_PREV_GROUP        0x027E

#define AVRCP_POP_RESERVED          0x007F

/* End of AvrcpPanelOperation */


/*---------------------------------------------------------------------------
 * AvrcpResponse type
 *
 * This type defines the AV/C response codes.
 */
typedef uint8_t AvrcpResponse;

#define AVRCP_RESPONSE_NOT_IMPLEMENTED    0x08   
#define AVRCP_RESPONSE_ACCEPTED           0x09          
#define AVRCP_RESPONSE_REJECTED           0x0a          
#define AVRCP_RESPONSE_IN_TRANSITION      0x0b     
#define AVRCP_RESPONSE_IMPLEMENTED_STABLE 0x0c
#define AVRCP_RESPONSE_CHANGED            0x0d           
#define AVRCP_RESPONSE_INTERIM            0x0f


/* This code, when received in an AVRCP_EVENT_PANEL_CNF event, indicates
 * that a "release" command was not actually delivered to the target because
 * the original "press" command was rejected.
 *
 * This value is NOT legal for use in functions that accept AvrcpResponse
 * as a parameter.
 */
#define AVRCP_RESPONSE_SKIPPED            0xF0

/* This code, when received in an AVRCP_EVENT_PANEL_CNF event, indicates
 * that the expected response message from the target was not received
 * within the expected time frame. The application may proceed normally
 * as if the command was accepted, or take some other action.
 *
 * This value is NOT legal for use in functions that accept AvrcpResponse
 * as a parameter.
 */
#define AVRCP_RESPONSE_TIMEOUT            0xF1
/* End of AvrcpResponse */ 

/*---------------------------------------------------------------------------
 * AvrcpErrorCode
 *
 *     Error code for AVRCP specific operations.
 */
typedef uint8_t AvrcpErrorCode;

#define AVRCP_ERR_INVALID_CMD         0x00
#define AVRCP_ERR_INVALID_PARM        0x01
#define AVRCP_ERR_PARM_NOT_FOUND      0x02
#define AVRCP_ERR_INTERNAL_ERROR      0x03
#define AVRCP_ERR_NO_ERROR            0x04
#define AVRCP_ERR_UIDS_CHANGED        0x05
#define AVRCP_ERR_UNKNOWN_ERROR       0x06
#define AVRCP_ERR_INVALID_DIRECTION   0x07
#define AVRCP_ERR_NON_DIRECTORY       0x08
#define AVRCP_ERR_DOES_NOT_EXIST      0x09
#define AVRCP_ERR_INVALID_SCOPE       0x0A
#define AVRCP_ERR_OUT_OF_BOUNDS       0x0B
#define AVRCP_ERR_IS_DIRECTORY        0x0C
#define AVRCP_ERR_MEDIA_IN_USE        0x0D
#define AVRCP_ERR_NOW_PLAYING_FULL    0x0E
#define AVRCP_ERR_NO_SEARCH_SUPPORT   0x0F
#define AVRCP_ERR_SEARCH_IN_PROGRESS  0x10
#define AVRCP_ERR_INVALID_PLAYER_ID   0x11
#define AVRCP_ERR_NOT_BROWSABLE       0x12
#define AVRCP_ERR_NOT_ADDRESSED       0x13
#define AVRCP_ERR_NO_SEARCH_RESULTS   0x14
#define AVRCP_ERR_NO_AVAIL_PLAYERS    0x15
#define AVRCP_ERR_ADDR_PLAYER_CHANGED 0x16

/* End of AvrcpErrorCode */ 

/*---------------------------------------------------------------------------
 * AvrcpMediaAttrIdMask type
 *
 * Defines supported values for the media attribute ID.  By default, only
 * AVRCP_ENABLE_MEDIA_ATTR_TITLE is enabled when a channel is registered.
 * The application must explicitly enable any supported attributes and
 * set the appropriate values.
 */
typedef uint8_t AvrcpMediaAttrIdMask;

#define AVRCP_ENABLE_MEDIA_ATTR_TITLE       0x01
#define AVRCP_ENABLE_MEDIA_ATTR_ARTIST      0x02
#define AVRCP_ENABLE_MEDIA_ATTR_ALBUM       0x04
#define AVRCP_ENABLE_MEDIA_ATTR_TRACK       0x08
#define AVRCP_ENABLE_MEDIA_ATTR_NUM_TRACKS  0x10
#define AVRCP_ENABLE_MEDIA_ATTR_GENRE       0x20
#define AVRCP_ENABLE_MEDIA_ATTR_DURATION    0x40

/* End of AvrcpMediaAttrIdMask */ 

/*---------------------------------------------------------------------------
 * AvrcpNotifyParms structure
 *
 * Defines the callback parameters for AVRCP_EVENT_ADV_RESPONSE when "advOp" is
 * set to AVRCP_OP_REGISTER_NOTIFY, and for the AVRCP_EVENT_ADV_NOTIFY event.
 */
typedef struct _AvrcpAdvNotifyParms {

    /* Defines the event ID that was received */
    AvrcpEventId   event;

    union {

        /* Play status of the media.  Valid when the event ID is
         * AVRCP_EID_MEDIA_STATUS_CHANGED.
         */ 
        AvrcpMediaStatus    mediaStatus;


        /* The position (ms) of the current track.  Valid when the event
         * ID is AVRCP_EID_PLAY_POS_CHANGED.
         */ 
        uint32_t                 position;


        /* Absolute volume.  Valid when the event ID is
         * AVRCP_EID_VOLUME_CHANGED.
         */
        uint8_t volume;
#if 0
        /* The addressed player.  Valid when the event ID is
         * AVRCP_EID_ADDRESSED_PLAYER_CHANGED.
         */ 
        struct {
            U16  playerId;
            U16  uidCounter;
        } addrPlayer;

        /* The UID counter.  Valid when the event ID is
         * AVRCP_EID_UIDS_CHANGED.
         */ 
        U16 uidCounter;
#endif
    } p;

} AvrcpAdvNotifyParms;

/*---------------------------------------------------------------------------
 * AvrcpAdvInfParms structure
 * 
 * Defines the callback parameters received during an AVRCP_EVENT_ADV_INFO
 * event.
 */
typedef union _AvrcpAdvInfParms {

    /* The Absolute Volume 
     * This is valid when "advOp" is set to AVRCP_OP_SET_ABSOLUTE_VOLUME.
     */
    uint8_t                  volume;

} AvrcpAdvInfParms;

/*---------------------------------------------------------------------------
 * AvrcpAdvCallbackParms structure
 *
 * Defines the callback parameters for advanced AVRCP commands/responses.
 */
typedef union _AvrcpAdvCallbackParms {

    /* Contains information about a command that altered the local settings.
     * This is valid during the AVRCP_EVENT_ADV_INFO event.  No response is
     * required, only support volume info now.
     */
    AvrcpAdvInfParms info;
    //uint8_t volume;

    /* Contains the parameters received in response to an advanced control
     * command. This is valid during the AVRCP_EVENT_ADV_RESPONSE event.
     */ 
    AvrcpAdvRspParms rsp;

    /* Contains the parameters of a notification or status of a registered 
     * event. This is valid during the AVRCP_EVENT_ADV_RESPONSE
     * when "advOp" is set to AVRCP_OP_REGISTER_NOTIFY.  Also valid
     * during the AVRCP_EVENT_ADV_NOTIFY event.
     */
    AvrcpAdvNotifyParms notify;
} AvrcpAdvCallbackParms;


typedef struct{
    /* AVRCP channel associated with the event */ 
    AvrcpEvent event;

    /*remote device addr*/
    BD_ADDR *addr;
    
    /* AVRCP channel associated with the event */ 
    void *chnl;
    
    /* AVRCP Advanced operation */
    AvrcpOperation  advOp;
    union{
        
        /* Panel indication received during AVRCP_EVENT_PANEL_CNF */ 
        struct{
            /* Operation to which the remote target responded */ 
            AvrcpPanelOperation operation;
            /* The press state of the key in the command to which the target responded.*/  
            uint32_t press;
            /* Response from the target. May indicate 
             * an "extended" response code such as 
             * AVRCP_RESPONSE_SKIPPED or AVRCP_RESPONSE_TIMEOUT.  
             */ 
            AvrcpResponse response;
        }panelCnf;
        
        /* Panel indication received during AVRCP_EVENT_PANEL_PRESS, 
         * AVRCP_EVENT_PANEL_HOLD, or AVRCP_EVENT_PANEL_RELEASE.  
         */ 
        struct {

            /* Operation corresponding to the key pressed, held, or 
             * released. AVRCP will only indicate a new operation 
             * when the previous one has been _RELEASE'd.  
             */ 
            AvrcpPanelOperation operation;

        } panelInd;
        
        /* Advanced AVRCP command/response parameters */
        AvrcpAdvCallbackParms adv;

    }param;

}avrcp_event_t;

typedef void(* avrcp_callback_func_t)(avrcp_event_t * event);
void bt_avrcp_set_cb_func(avrcp_callback_func_t func);

/*---------------------------------------------------------------------------
 * AVCTP_ConnectRsp()
 * 
 *     Responds to a connection request from the remote AVCTP device.  This 
 *     function is used to establish the lower layer connection (L2CAP), 
 *     which allows sending signaling messages, such as discover, 
 *     configuration, and stream management.
 *
 * Parameters:
 *
 *     Chnl - A registered and open AVCTP channel.
 *
 *     Accept - true accepts the connect or false rejects the connection.
 *
 * Returns:
 *
 *     BT_STATUS_PENDING - The connection responses has been successfully
 *         sent. When the connection process is complete, the application 
 *         callback will receive the AVCTP_EVENT_CONNECT event.
 *
 *     BT_STATUS_BUSY - The connection is already connected.
 *
 *     BT_STATUS_INVALID_PARM - The Chnl parameter does not contain a 
 *         valid pointer. (XA_ERROR_CHECK only).
 *
 *     BT_STATUS_NOT_FOUND - The specified device was not found in the device
 *         selector database.  The device must be discovered, paired, or added
 *         manually using DS_AddDevice().  
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).
 */
BtStatus avrcp_connect_rsp(void *chnl,bool accept);

/*---------------------------------------------------------------------------
 * AVRCP_CtGetCapabilities()
 *
 *     Get the capabilities of the target device.  The capabilities 
 *     supported by the target device may change.  If the target device 
 *     application changes to support less functionality, then errors 
 *     will begin to be received for capabilities that were previously 
 *     supported.  If this occurs, AVRCP_CtGetCapabilities() can be called
 *     again to get the most current capabilities. AVRCP_CtGetCapabilities()
 *     may be occasionally called to poll for capability changes.
 * 
 *     When the final response event is received (AVRCP_EVENT_ADV_RESPONSE),
 *     the 'advOp' field in the callback parameters will set to
 *     AVRCP_OP_GET_CAPABILITIES.
 *
 * Parameters:
 *     chnl - A registered and open AVRCP channel.
 * 
 *     capabilityId - The specific capability that is being requested.  
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has been started successfully. When
 *         the associated packet has been sent, the application callback will
 *         receive the AVRCP_EVENT_ADV_TX_DONE event.  When a response has
 *         been received from the remote device, a AVRCP_EVENT_ADV_RESPONSE
 *         event will be received.
 *
 *     BT_STATUS_IN_USE - The 'cmd' packet was in use and cannot be sent.
 *
 *     BT_STATUS_INVALID_PARM - The operation failed, because the 
 *         chnl or cmd parameter is invalid.  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel is not registered.  
 */

BtStatus avrcp_ct_get_capabilities(void *chnl, AvrcpCapabilityId capabilityId);

/*---------------------------------------------------------------------------
 * AVRCP_CtGetPlayStatus()
 *
 *     Get the play status of the current media player on the target 
 *     device.  
 *
 *     When the final response event is received (AVRCP_EVENT_ADV_RESPONSE),
 *     the 'advOp' field in the callback parameters will set to
 *     AVRCP_OP_GET_PLAY_STATUS.
 * 
 * Parameters:
 *     chnl - A registered and open AVRCP channel.  
 *
 *     cmd - A PDU for sending commands.
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has been started successfully. When
 *         the associated packet has been sent, the application callback will
 *         receive the AVRCP_EVENT_ADV_TX_DONE event.  When a response has
 *         been received from the remote device, a AVRCP_EVENT_ADV_RESPONSE
 *         event will be received.
 *
 *     BT_STATUS_IN_USE - The 'cmd' packet was in use and cannot be sent.
 *
 *     BT_STATUS_INVALID_PARM - The operation failed, because the 
 *         chnl or cmd parameter is invalid.  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel is not registered.  
 */
BtStatus avrcp_ct_get_playstaus(void       *chnl);

/*---------------------------------------------------------------------------
 * AVRCP_CtRegisterNotification()
 *
 *     Register for notification of events on the target device.  
 *
 *     When the final response event is received (AVRCP_EVENT_ADV_RESPONSE),
 *     the 'advOp' field in the callback parameters will set to
 *     AVRCP_OP_REGISTER_NOTIFY.
 * 
 * Parameters:
 *     chnl - A registered and open AVRCP channel.  
 *
 *     cmd - A PDU for sending commands.
 *
 *     eventId - The event for which the CT requires notifications.  
 *
 *     interval - For the AVRCP_EID_PLAY_POS_CHANGED event, the interval 
 *         parameter specifies the time interval (in seconds) at which the 
 *         change in the playback position will be notified.  If the song is 
 *         being forwarded or rewind, a notification will be received 
 *         whenever the playback position will change by this value even if 
 *         the interval has not yet expired.  For other events, the value of 
 *         this parameter is ignored.  
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has been started successfully. When
 *         the associated packet has been sent, the application callback will
 *         receive the AVRCP_EVENT_ADV_TX_DONE event.  When a response has
 *         been received from the remote device, a AVRCP_EVENT_ADV_RESPONSE
 *         event will be received.
 *
 *     BT_STATUS_IN_USE - The 'cmd' packet was in use and cannot be sent.
 *
 *     BT_STATUS_INVALID_PARM - The operation failed, because the 
 *         chnl or cmd parameter is invalid.  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel is not registered.  
 */
BtStatus avrcp_ct_register_notification(void          *chnl, AvrcpEventId eventId,uint32_t interval);

/*---------------------------------------------------------------------------
 * AVRCP_SetPanelKey()
 *
 *     Indicates the state of the key corresponding to the specified 
 *     panel subunit operation. Successive calls to this function will 
 *     queue up key events to be delivered to the target.  
 *
 * Parameters:
 *     chnl - A registered and open AVRCP channel.  
 *
 *     op - Panel operation code to send. If a previous call indicated a 
 *         different "advOp" as pressed, calling this function with a new 
 *         "advOp" will automatically release it.  
 *
 *     press - TRUE indicates the key corresponding to operation was pressed, 
 *         FALSE indicates the key was released. For FALSE, if the "advOp" 
 *         specified was not already pressed, this call signals a 
 *         single press-and-release of the key.  
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation was started successfully. 
 *         AVRCP_EVENT_PANEL_CNF message(s) will be sent corresponding 
 *         to the target's responses to the press, release, or hold messages 
 *         brought about by this command.  
 *
 *     BT_STATUS_NO_RESOURCES - The internal keystroke buffer is full.  
 *
 *     BT_STATUS_INVALID_PARM - The chnl parameter does not contain a valid 
 *         pointer. (XA_ERROR_CHECK only).  
 */
BtStatus avrcp_set_panel_key(void *chnl, AvrcpPanelOperation op, uint32_t press);

/*---------------------------------------------------------------------------
 * AVRCP_TgSetEventMask()
 *
 *     Allows the application to specify which events will be supported 
 *     by the current media player.  When a flag is set in the event 
 *     mask, then associated feature is supported.  
 *     AVRCP_ENABLE_PLAY_STATUS_CHANGED and AVRCP_ENABLE_TRACK_CHANGED 
 *     must both be set, and if not specified, will be added to the 
 *     mask.  
 *
 * Parameters:
 *     chnl - A registered AVRCP channel.  
 *
 *     mask - A bitmask with bits set to enable individual events.  
 *
 * Returns:
 *     BT_STATUS_SUCCESS - The operation completed successfully.  
 *
 *     BT_STATUS_INVALID_PARM - The operation failed, because the 
 *         chnl parameter is invalid.  
 */
BtStatus avrcp_tg_set_event_mask(void        *chnl, AvrcpEventMask mask);

/*---------------------------------------------------------------------------
 * AVRCP_TgSetAbsoluteVolume()
 *
 *     Allows the application to specify the current volume of the local
 *     device.  This function should only be called by the rendering device
 *     (Sink), which means that the local device may need to be registered as
 *     both a Category 2 Target and a Category 1 Controller.  The status is
 *     updated internally and, when requested, will be sent to the controller
 *     (Source).
 * 
 *     If the controller has registered for notification of a change in the
 *     volume (AVRCP_EID_VOLUME_CHANGED), then calling this function will
 *     result in completion of that notification.  The memory pointed to by
 *     the 'rsp' parameter will be used to send the completion event, and
 *     this function will return BT_STATUS_PENDING.
 * 
 *     If no request for notification has been registered by the controller,
 *     this function will return BT_STATUS_SUCCESS.
 * 
 * Parameters:
 *     chnl - A registered AVRCP channel.
 * 
 *     volume - The absolute volume of the Sink (0 - 0x7F).
 *
 * Returns:
 *     BT_STATUS_SUCCESS - The operation completed successfully.  
 *
 *     BT_STATUS_PENDING - The send response operation has been started 
 *         successfully. When the associated packet has been sent, 
 *         the application callback will receive the AVCTP_EVENT_TX_DONE 
 *         event.  
 *
 *     BT_STATUS_IN_USE - The 'rsp' packet was in use and cannot be sent.
 *
 *     BT_STATUS_INVALID_PARM - The operation failed, because the 
 *         chnl parameter is invalid.  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel is not registered.  
 */
BtStatus avrcp_tg_set_absolute_volume(void *chnl, uint8_t volume);

/*---------------------------------------------------------------------------
 * avrcp_connect()
 * 
 *     Initiates a connection to a remote AVRCP device.  This function is 
 *     used to establish the lower layer connection (L2CAP), which allows
 *     sending messages.
 *
 *     If the connection attempt is successful, the AVRCP_EVENT_CONNECT event 
 *     will be received.  If the connection attempt is unsuccessful, the 
 *     AVRCP_EVENT_DISCONNECT event will be received.  
 *
 * Parameters:
 *
 *      addr - The Bluetooth address of the device to which the connection 
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
BtStatus avrcp_connect(BD_ADDR *addr);

/*---------------------------------------------------------------------------
 * avrcp_disconnect()
 *
 *     Terminates a connection with a remote AVRCP device.  The lower layer 
 *     connection (L2CAP) is disconnected.  
 *
 * Parameters:
 *
 *     chnl - A registered and open AVRCP channel.  
 *
 * Returns:
 *
 *     BT_STATUS_PENDING - The disconnect process has been successfully 
 *         started. When the disconnect process is complete, the 
 *         application callback will receive the AVRCP_EVENT_DISCONNECT 
 *         event.  
 *
 *     BT_STATUS_INVALID_PARM - The chnl parameter does not contain a valid 
 *         pointer. (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NO_CONNECTION - No connection exists on the specified 
 *         channel.  
 *
 *     BT_STATUS_NOT_FOUND - The specified device was not found in the device 
 *         selector database.  The device must be discovered, paired, or 
 *         added manually using DS_AddDevice().  
 *
 *     It is possible to receive other error codes, depending on the lower 
 *     layer service in use (L2CAP or Management Entity).  
 */
BtStatus avrcp_disconnect(void *chnl);

/*---------------------------------------------------------------------------
 * avrcp_ct_get_media_info()
 *
 *     Get the media attributes for the current track on the current 
 *     media player on target device.  
 *
 *     When the final response event is received (AVRCP_EVENT_ADV_RESPONSE),
 *     the 'advOp' field in the callback parameters will set to
 *     AVRCP_OP_GET_MEDIA_INFO.
 * 
 * Parameters:
 *     chnl - A registered and open AVRCP channel.  
 *
 *     mediaMask - Defines which media attibutes are to be queried.  If set
 *                 to 0, then all attributes will be requested.
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has been started successfully. When
 *         the associated packet has been sent, the application callback will
 *         receive the AVRCP_EVENT_ADV_TX_DONE event.  When a response has
 *         been received from the remote device, a AVRCP_EVENT_ADV_RESPONSE
 *         event will be received.
 *
 *     BT_STATUS_IN_USE - The 'cmd' packet was in use and cannot be sent.
 *
 *     BT_STATUS_INVALID_PARM - The operation failed, because the 
 *         chnl or cmd parameter is invalid.  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel is not registered.  
 */
BtStatus avrcp_ct_get_media_info(void         *chnl, 
                              AvrcpMediaAttrIdMask  mediaMask);


/*---------------------------------------------------------------------------
 * avrcp_ct_set_absolute_volume()
 *
 *     Set the absolute volume on the target.  This function should be called
 *     only by the source (capturing) device, which is the opposite of
 *     almost all other controller commands.  This means that the local
 *     device may need to be registered as both a Category 1 Target and a
 *     Category 2 Controller.
 * 
 *     When the final response event is received (AVRCP_EVENT_ADV_RESPONSE),
 *     the 'advOp' field in the callback parameters will set to
 *     AVRCP_OP_SET_ABSOLUTE_VOLUME.
 *
 * Parameters:
 *     chnl - A registered and open AVRCP channel.
 * 
 *     volume - The absolute volume of the Sink (0 - 0x7F).
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has been started successfully. When
 *         the associated packet has been sent, the application callback will
 *         receive the AVRCP_EVENT_ADV_TX_DONE event.  When a response has
 *         been received from the remote device, a AVRCP_EVENT_ADV_RESPONSE
 *         event will be received.
 *
 *     BT_STATUS_IN_USE - The 'cmd' packet was in use and cannot be sent.
 *
 *     BT_STATUS_INVALID_PARM - The operation failed, because the 
 *         chnl or setting parameter is invalid.  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel is not registered.  
 */

BtStatus avrcp_ct_set_absolute_volume(void* *chnl, 
                                   uint8_t volume);

#endif
