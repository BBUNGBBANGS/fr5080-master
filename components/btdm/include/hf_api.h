#ifndef _HF_API_H
#define _HF_API_H
#include "bt_error.h"
#include "type_lib.h"
#include <stdint.h>


/****************************************************************************
 *
 * Constants
 *
 ****************************************************************************/

/*---------------------------------------------------------------------------
 * HfEvent type
 *
 *     All indications and confirmations are sent through a callback 
 *     function.  Depending on the event, different elements of 
 *     the HfCallbackParms "params" union will be valid.  
 */
typedef uint16_t HfEvent;

/** An incoming service level connection is being established.  This happens 
 *  when the audio gateway establishes the service connection.  
 *  The data connection is not available yet for issuing commands to the 
 *  audio gateway.  When the HF_EVENT_SERVICE_CONNECTED event is received, 
 *  the channel is available for issuing commands.  
 *
 *  When this callback is received, "param" field is not used.
 */
#define HF_EVENT_SERVICE_CONNECT_REQ   0

/** A service level connection has been established.  This can happen as the 
 *  result of a call to HF_CreateServiceLink, or if the audio gateway 
 *  establishes the service connection.  When this event has been received, 
 *  a data connection is available for issuing commands to the audio 
 *  gateway.  
 *
 *  When this callback is received, "param" field is not used. 
 */
#define HF_EVENT_SERVICE_CONNECTED      1

/** The service level connection has been released.  This can happen as the 
 *  result of a call to HF_DisconnectServiceLink, or if the audio gateway 
 *  releases the service connection.  Communication with the audio gateways 
 *  is no longer possible.  In order to communicate with the audio gateway, 
 *  a new service level connection must be established.  
 *
 *  This event can also occur when an attempt to create a service level 
 *  connection (HF_CreateServiceLink) fails.  
 *
 *  When this callback is received, "param.errCode" fields contains the
 *  reason for disconnect.  
 */
#define HF_EVENT_SERVICE_DISCONNECTED   2

/** An audio connection has been established.  This event occurs whenever 
 *  the audio channel (SCO) comes up, whether it is initiated by the audio 
 *  gateway or the hands-free unit.  
 *
 *  When this callback is received, "param" field is not used 
 */
#define HF_EVENT_AUDIO_CONNECTED        3

/** An audio connection has been released.  This event occurs whenever the 
 *  audio channel (SCO) goes down, whether it is terminated by the audio 
 *  gateway or the hands-free unit.  
 *
 *  When this callback is received, "param.errCode" fields contains the
 *  reason for disconnect.  
 */
#define HF_EVENT_AUDIO_DISCONNECTED     4
 

/** After the service level connection has been established, this event will 
 *  indicate the features supported on the audio gateway.  
 *
 *  When this callback is received, the "param.features" field contains the
 *  features (see HfFeatures).  
 */
#define HF_EVENT_GATEWAY_FEATURES       7

/** After the service level connection has been established, this event will 
 *  indicate the hold features (3-Way calling) supported on the audio 
 *  gateway.  
 *
 *  When this callback is received, the "param.holdFeatures" field contains
 *  the features (see HfGwHoldFeatures).  
 */
#define HF_EVENT_GW_HOLD_FEATURES       8


/** After the service level connection has been established, this event will
 *  indicate whether at least one call is active on the gateway device.
 *  Whenever all calls are terminated, or when there were no calls and
 *  a new call is created, this event is generated.  
 *
 *  When this callback is received, the "param.call" field contains the 
 *  current call state (see HfCallActiveState).  
 */ 
#define HF_EVENT_CALL_IND              13

/** After the service level connection has been established, 
 *  this event will indicate the current call setup state.  Whenever the 
 *  call setup state changes, this event is generated.  
 *
 *  When this callback is received, the "param.callSetup" field 
 *  contains the current call setup state (see HfCallSetupState).
 */ 
#define HF_EVENT_CALLSETUP_IND         14

/** After the service level connection has been established, 
 *  this event will indicate the current call held state.  Whenever the held 
 *  state changes, this event is generated.  
 *
 *  When this callback is received, the "param.callHeld" field 
 *  contains the current call held state (see HfCallHeldState).  
 */ 
#define HF_EVENT_CALLHELD_IND          15

/** When an incoming call is received on the audio gateway, 
 *  this event is generated to indicate the incoming ring.  
 */
#define HF_EVENT_RING_IND              16

/** When call waiting is supported on the audio gateway and an 
 *  incoming call is received while another call is active, this event is 
 *  received.  
 *
 *  When this callback is received, the "param.callWaitParms" 
 *  field contains information about the waiting call (see 
 *  HfCallWaitParms).  
 */ 
#define HF_EVENT_WAIT_NOTIFY           17

/** If caller ID notification is active, this event is 
 *  received when an incoming call is detected on the audio gateway.  
 *
 *  When this callback is received, the "param.callerIdParms" 
 *  field contains the current caller ID information (see HfCallerIdParms).  
 */ 
#define HF_EVENT_CALLER_ID_NOTIFY      18

/** This event is received once for each call which exists on 
 *  the audio gateway.  This event is received after calling 
 *  HF_ListCurrentCalls().  
 *
 *  When this callback is received, the "param.callListParms" 
 *  field contains the current caller ID information (see HfCallListParms).  
 */                                     
#define HF_EVENT_CURRENT_CALL_STATE    19

/** The service indicator has been received from the audio gateway.  
 *
 *  When this callback is received, the "param.service" 
 *  field contains a pointer to the service state.  
 */ 
#define HF_EVENT_SERVICE_IND           21

/** The battery indicator has been received from the audio gateway.  
 *
 *  When this callback is received, the "param.battery" 
 *  field contains a pointer to the battery level.  
 */ 
#define HF_EVENT_BATTERY_IND           22

/** The signal strength indicator has been received from the audio gateway.  
 *
 *  When this callback is received, the "param.signal" 
 *  field contains a pointer to the signal strength.  
 */ 
#define HF_EVENT_SIGNAL_IND            23

/** The roam indicator has been received from the audio gateway.  
 *
 *  When this callback is received, the "param.roam" 
 *  field contains a pointer to the roam state.  
 */ 
#define HF_EVENT_ROAM_IND              24


/** The voice recognition state has changed.  This event occurs if the 
 *  audio gateway changes the state of voice recognition.  
 *
 *  When this callback is received, the "param.voiceRecognition" 
 *  field contains state of voice recognition.  
 */
#define HF_EVENT_VOICE_REC_STATE       26


/** A number was returned in response to the HF_GetLastVoiceTag function.  
 *
 *  When this callback is received, the "param.voiceTag" field 
 *  contains a pointer to the ASCII string representation of the number 
 *  (NULL terminated).  
 */
#define HF_EVENT_VOICE_TAG_NUMBER      27

/** The speaker volume has been received from the audio gateway.  
 */
#define HF_EVENT_SPEAKER_VOLUME        28

/** The microphone volume has been received from the audio gateway.  
 */
#define HF_EVENT_MIC_VOLUME            29

/** The in-band ring tone setting has been received from the audio gateway.  
 *
 *  When this callback is received, the "param.inBandRing" 
 *  field contains a pointer to the In-Band ring state.  
 */ 
#define HF_EVENT_IN_BAND_RING          30

#if 0
/** The network operator string has been received from the remote device.  
 *
 *  When this callback is received, the "HfCallbackParms.p.networkOper" 
 *  field contains a pointer to the operator string state.  
 */ 
#define HF_EVENT_NETWORK_OPERATOR      31
#endif

/** The subscriber number has been received from the audio gateway.  
 *
 *  When this callback is received, the "HfCallbackParms.p.subscriberNum" 
 *  field contains a pointer to the subscriber number.  
 */ 
#define HF_EVENT_SUBSCRIBER_NUMBER     32


/** The DELAYED event has been received from the audio gateway.  
 */
#define HF_EVENT_DELAYED               36

/** The BLACKLISTED event has been received from the audio gateway.  
 */
#define HF_EVENT_BLACKLISTED           37

/** A result code has been received from the audio gateway.  This event is 
 *  received for unsolicited result codes not handled by the internal 
 *  Hands-free AT parser.  
 *
 *  When this callback is received, the "HfCallbackParms.p.data" field 
 *  contains the AT result code.  
 */
#define HF_EVENT_AT_RESULT_DATA        42

/** A command to the audio gateway has completed.  This event is received 
 *  when the processing a command is complete.  
 *
 *  When this callback is received, the "param.command" field 
 *  contains the command that was sent.  If "param.status is set 
 *  to BT_STATUS_FAILED, then "param.command->cmeError" contains 
 *  the command error (if known).  
 */
#define HF_EVENT_COMMAND_COMPLETE      43

/* End of HfEvent */ 

/*---------------------------------------------------------------------------
 * HfCallActiveState type
 *
 * HfCallActiveState enumerates the possible current call states that can be 
 * indicated by the Audio Gateway.  
 */
typedef uint8_t HfCallActiveState;

/** No call exists on the Audio Gateway 
 */ 
#define HF_CALL_NONE           0

/** A call is active on the Audio Gateway 
  */ 
#define HF_CALL_ACTIVE         1

/* End of HfCallActiveState */ 

/*---------------------------------------------------------------------------
 * HfCallHeldState type
 *
 * HfCallHeldState enumerates the possible current held state that can be 
 * indicated by the audio gateway.  
 */
typedef uint8_t HfCallHeldState;

/** No calls are held on the audio gateway 
 */ 
#define HF_CALL_HELD_NONE      0

/** A call is held and another call is active on the audio gateway.  This 
 *  indication can be sent for several reasons, including when an active and 
 *  held call are swapped.  
 */ 
#define HF_CALL_HELD_ACTIVE    1

/** A call is held and no active call exists on the audio gateway 
 */ 
#define HF_CALL_HELD_NO_ACTIVE 2

/* End of HfCallHeldState */ 

/*---------------------------------------------------------------------------
 * HfCallSetupState type
 *
 * HfCallSetupState enumerates the possible current call setup state that 
 * can be indicated by the Audio Gateway.  
 */
typedef uint8_t HfCallSetupState;

/** No outgoing or incoming call is present on the Audio Gateway 
 */ 
#define HF_CALL_SETUP_NONE     0

/** An incoming call is present on the Audio Gateway 
 */ 
#define HF_CALL_SETUP_IN       1

/** An outgoing call is present on the Audio Gateway 
 */ 
#define HF_CALL_SETUP_OUT      2

/** An outgoing call is being alerted on the Audio Gateway 
 */ 
#define HF_CALL_SETUP_ALERT    3

/* End of HfCallSetupState */ 

/*---------------------------------------------------------------------------
 * HfCallStatus type
 *
 * HfCallStatus defines the current state of a call.  Not all states are 
 * supported by all Audio Gateways.  At the very minimum, 
 * HF_CALL_STATUS_NONE, HF_CALL_STATUS_DIALING, HF_CALL_STATUS_INCOMING, and 
 * HF_CALL_STATUS_ACTIVE will be supported.  
 */
typedef uint8_t HfCallStatus;

/** An active call exists.  
 */
#define HF_CALL_STATUS_ACTIVE     0

/** The call is held.  
 */
#define HF_CALL_STATUS_HELD       1

/** A call is outgoing.  This state occurs when attempting a call using any 
 *  of the dialing functions.  
 */
#define HF_CALL_STATUS_DIALING    2

/** The remote party is being alerted.  
 */
#define HF_CALL_STATUS_ALERTING   3

/** A call is incoming.  It can be answered by invoking HFG_AnswerCall() or 
 *  rejected by invoking HFG_Hangup().  This state occurs when a call is 
 *  being set up by a remote party, and there is currently no 
 *  established call.  
 */
#define HF_CALL_STATUS_INCOMING   4

/** A call is waiting.  It can be answered by invoking HFG_AnswerCall() or 
 *  rejected by invoking HFG_Hangup().  This state occurs when a call is 
 *  being set up by a remote party, and there is currently an 
 *  established call.  
 */
#define HF_CALL_STATUS_WAITING    5


/** No active call
 */
#define HF_CALL_STATUS_NONE       0xFF

/** Unknown call state
 */
#define HF_CALL_STATUS_UNKNOWN    0xFE

/* End of HfCallStatus */ 

/*---------------------------------------------------------------------------
 * HfCallMode type
 *
 * HfCallMode defines the current mode of a call.  It is only meaningful if 
 * HF_USE_CALL_MANAGER is defined as XA_DISABLED.  
 */
typedef uint8_t HfCallMode;

/** Voice Call 
  */ 
#define HF_CALL_MODE_VOICE     0

/** Data Call 
 */ 
#define HF_CALL_MODE_DATA      1

/** FAX Call 
 */ 
#define HF_CALL_MODE_FAX       2

/* End of HfCallMode */ 




/*---------------------------------------------------------------------------
 * HfCallWaitParms structure
 *
 *     This type is defined only if HF_USE_CALL_MANAGER is defined as 
 *     XA_DISABLED.  It is used to identify the waiting call.  
 */
typedef struct _HfCallWaitParms {

    /* Phone number of the waiting call */ 
    const char     *number;

    /* Voice parameters */ 
    uint8_t              classmap;

    /* Type of address */ 
    uint8_t              type;

} HfCallWaitParms;

/*---------------------------------------------------------------------------
 * HfCallerIdParms structure
 *
 *    It is used to identify the calling number.  
 */
typedef struct _HfCallerIdParms {

    /* Phone number of the caller */ 
    const char     *number;

    /* Type of address */ 
    uint8_t              type;

} HfCallerIdParms;

/*---------------------------------------------------------------------------
 * HfCallListParms structure
 *
 *     This type is defined only if HF_USE_CALL_MANAGER is defined as 
 *     XA_DISABLED.  It is used to identify the listed calls on the Audio 
 *     Gateway.  
 */
typedef struct _HfCallListParms {

    /* Index of the call on the audio gateway (1 based). */ 
    uint8_t              index;

    /* 0 - Mobile Originated, 1 = Mobile Terminated */ 
    uint8_t              dir;

    /* Call state (see HfCallState). */ 
    HfCallStatus   state;

    /* Call mode (see HfCallMode). */ 
    HfCallMode      mode;

    /* 0 - Not Multiparty, 1 - Multiparty */ 
    uint8_t              multiParty;
    
    /* Phone number of the call */ 
    const char     *number;

    /* Type of address */ 
    uint8_t              type;

} HfCallListParms;

/*---------------------------------------------------------------------------
 * HfCommand structure
 *
 * Structures of type HfCommand can be used to store the command type and 
 * parameters for sending Hands-Free SDK commands.  
 */
struct _HfCommand {

    /* Used Internally by the Hands-free SDK */ 
    void *node1;
    void *node2;

    /* The type of command */ 
    uint8_t          type;

    /* The command parameters */ 
    uint32_t         parms[6];

    /* The command status */ 
    BtStatus    status;

    /* CME Error when command fails */ 
    uint8_t  cmeError;

    /* Application context */ 
    void       *context;

    /* === Internal use only === */ 

    uint8_t          state;
};

typedef struct _HfCommand HfCommand;
/*--------------------------------------------------------------------------
 * HfCommandType type
 *
 *     HfCommandType corresponds to a logical command.  This value is 
 *     returned in the "HfCallbackParms.p.command" structure with the 
 *     HF_EVENT_COMMAND_COMPLETE event.  It indicates the API call 
 *     associated with this command structure.  Any API call that takes an 
 *     HfCommand parameter receives the HF_EVENT_COMMAND_COMPLETE event when 
 *     the command has completed.  The "HfCallbackParms.p.command->type" 
 *     field identifies the API call that was made.  
 */
typedef uint8_t HfCommandType;

/* HF_AnswerCall */ 
#define HF_COMMAND_ANSWER_CALL             0

/* HF_DialNumber */ 
#define HF_COMMAND_DIAL_NUMBER             1

/* HF_MemoryDial */ 
#define HF_COMMAND_DIAL_MEMORY             2

/* HF_Redial */ 
#define HF_COMMAND_REDIAL                  3

/* HF_CallHold */ 
#define HF_COMMAND_CALL_HOLD               4

/* HF_Hangup */ 
#define HF_COMMAND_HANGUP_CALL             7

/* HF_ListCurrentCalls */ 
#define HF_COMMAND_LIST_CURRENT_CALLS      8

/* HF_EnableCallerIdNotify */ 
#define HF_COMMAND_ENABLE_CID_NOTIFY       9

/* HF_EnableCallWaitNotify */ 
#define HF_COMMAND_ENABLE_WAIT_NOTIFY      10

/* HF_GenerateDtmf() */ 
#define HF_COMMAND_GENERATE_DTMF           11

/* HF_GetLastVoiceTag */ 
#define HF_COMMAND_GET_LAST_VOICE_TAG      12

/* HF_EnableVoiceRecognition */ 
#define HF_COMMAND_VOICE_RECOGNITION       13

/* HF_DisableNREC */ 
#define HF_COMMAND_DISABLE_NREC            14

/* HF_ReportMicVolume */ 
#define HF_COMMAND_REPORT_MIC_VOLUME       15

/* HF_ReportSpeakerVolume */ 
#define HF_COMMAND_REPORT_SPEAKER_VOLUME   16

/* HF_QueryNetworkOperator */ 
#define HF_COMMAND_QUERY_NETWORK_OPER      17

/* HF_QuerySubscriberNumber */ 
#define HF_COMMAND_QUERY_SUBSCRIBER_NUM    18

/* HF_EnableExtendedErrors */ 
#define HF_COMMAND_ENABLE_EXTENDED_ERR     19

/* HF_SendAtCommand */ 
#define HF_COMMAND_SEND_AT_COMMAND         20

/* HF_QueryPhonebooks */ 
#define HF_COMMAND_QUERY_PB                21

/* HF_SelectPhonebook */ 
#define HF_COMMAND_SELECT_PB               22

/* HF_GetCurrentPhonebookInfo */ 
#define HF_COMMAND_GET_CURRENT_PB_INFO     23

/* HF_GetPhonebookSize */ 
#define HF_COMMAND_GET_PB_SIZE             24

/* HF_ReadPhonebookEntries */ 
#define HF_COMMAND_READ_PB_ENTRIES         25

/* HF_FindPhonebookEntries */ 
#define HF_COMMAND_FIND_PB_ENTRIES         26

/* HF_WritePhonebookEntry */ 
#define HF_COMMAND_WRITE_PB_ENTRY          27


/* End of HfCommandType */ 

/*---------------------------------------------------------------------------
 * HfGatewayFeatures type
 *      
 *  HfGatewayFeatures is a bit mask specifying the gateway feature set.  The 
 *  service connection capabilities will be limited to the features 
 *  advertised by the profile.  
 */
typedef uint32_t HfGatewayFeatures;

/** 3-way calling
 */
#define HF_GW_FEATURE_3_WAY              0x00000001

/** Echo canceling and/or noise reduction function
 */
#define HF_GW_FEATURE_ECHO_NOISE         0x00000002

/** Voice recognition function
 */
#define HF_GW_FEATURE_VOICE_RECOGNITION  0x00000004

/** In-band ring tone
 */
#define HF_GW_FEATURE_IN_BAND_RING       0x00000008

/** Voice tag
 */
#define HF_GW_FEATURE_VOICE_TAG          0x00000010

/** Reject a call
 */
#define HF_GW_FEATURE_CALL_REJECT        0x00000020

/** Enhanced Call Status
 */
#define HF_GW_FEATURE_ENH_CALL_STATUS    0x00000040

/** Enhanced Call Control
 */
#define HF_GW_FEATURE_ENH_CALL_CTRL      0x00000080

/* End of HfGatewayFeatures */ 

/*---------------------------------------------------------------------------
 * HfGwHoldFeatures type
 *
 *  This type is used as a bit mask specifying the gateway's 3-Way calling 
 *  (hold) feature set.  The service connection capabilities will be limited 
 *  to the features advertised by the profile.  
 */
typedef uint8_t HfGwHoldFeatures;

/** Releases all held calls or sets User Determined User Busy 
 * (UDUB) for a waiting call.  
 */
#define HF_GW_HOLD_RELEASE_HELD_CALLS      0x01

/** Releases all active calls (if any exist) and accepts the other 
 * (held or waiting) call.  
 */
#define HF_GW_HOLD_RELEASE_ACTIVE_CALLS    0x02

/** Releases a specific call. */ 
#define HF_GW_HOLD_RELEASE_SPECIFIC_CALL   0x04

/** Places all active calls (if any exist) on hold and accepts the 
 * other (held or waiting) call.  
 */
#define HF_GW_HOLD_HOLD_ACTIVE_CALLS       0x08

/** Places a specific call on hold. */ 
#define HF_GW_HOLD_HOLD_SPECIFIC_CALL      0x10

/** Adds a held call to the conversation.  
 */
#define HF_GW_HOLD_ADD_HELD_CALL           0x20

/** Connects the two calls and disconnects the AG from 
 *  both calls (Explicit Call Transfer).  
 */
#define HF_GW_HOLD_CALL_TRANSFER           0x40

/* End of HfGwHoldFeatures */ 


/*---------------------------------------------------------------------------
 * HfHoldAction type
 *
 * HfHoldAction enumerates the possible actions that can be taken when 
 * calling the HF_CallHold() function.
 */
typedef uint8_t HfHoldAction;

/** Indicates that the code should release all held calls, or set the User 
 *  Determined User Busy (UDUB) indication for a waiting call.  
 */
#define HF_HOLD_RELEASE_HELD_CALLS   0

/** Indicates that the code should release all active calls (if any exist) 
 *  and accepts the other (held or waiting) call.  
 *
 *  If a call index is specified, the code should release the specific call.  
 */
#define HF_HOLD_RELEASE_ACTIVE_CALLS 1

/** Indicates that the code should place all active calls (if any exist) on 
 *  hold and accepts the other (held or waiting) call.  
 *
 *  If a call index is specified, the code should put all calls on hold 
 *  except the specified call.  
 */
#define HF_HOLD_HOLD_ACTIVE_CALLS    2

/** Indicates that the code should add a held call to the conversation.  
 */
#define HF_HOLD_ADD_HELD_CALL        3

/** Indicates that the code should connects the two calls and disconnect the 
 *  Audio Gateway from both calls.  In other words, the code should perform 
 *  an Explicit Call Transfer.  
 */
#define HF_HOLD_CALL_TRANSFER        4

/* End of HfHoldAction */ 

/*---------------------------------------------------------------------------
 * HfAtData structure
 *
 *     Structures of type HfAtData can be used to store raw AT data.  
 */
typedef struct _HfAtData {
    uint8_t         *data;
    uint16_t         dataLen;
} HfAtData;

/*
* hf_event_t struct
*/
typedef struct{
    HfEvent type;
    BD_ADDR *addr;
    BtErrorCode errCode;
    void *chan;
    union{
        void              *ptr;
        HfGatewayFeatures features;
        HfGwHoldFeatures  holdFeatures;
        int              service;
        int              roam;
        int              inBandRing;
        int              voiceRecognition;
        uint8_t                battery;
        uint8_t                signal;
        uint8_t                gain;
        HfCommand        *command;
        
        HfCallActiveState call;         
        HfCallSetupState  callSetup;     
        HfCallHeldState   callHeld;      
        HfCallerIdParms  *callerIdParms; 
        HfCallWaitParms  *callWaitParms; 
        HfCallListParms  *callListParms;
        HfAtData         *data;
    }param;

}hf_event_t;

/* End of hf_event_t */ 


typedef void(* hf_callback_func_t)(hf_event_t * event);
void bt_hf_set_cb_func(hf_callback_func_t func);


/****************************************************************************
 *
 * Function Reference
 *
 ****************************************************************************/
 /*---------------------------------------------------------------------------
 * hf_connect()
 *
 *     This function creates a service level connection with the Audio 
 *     Gateway.  This includes performing SDP Queries to find the 
 *     appropriate service and opening an RFCOMM channel.  The success of 
 *     the operation is indicated by the HF_EVENT_SERVICE_CONNECTED event.  
 *     If the connection fails, the application is notified by the 
 *     HF_EVENT_SERVICE_DISCONNECTED event.  
 * 
 *     If an ACL link does not exist to the audio gateway, one will be 
 *     created first.  If desired, however, the ACL link can be established 
 *     prior to calling this function.  
 *
 * Parameters:
 *
 *     Chan - Pointer to a registered channel structure.  
 *
 *     Addr - The Bluetooth address of the remote device.  
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started; the application will 
 *         be notified when the connection has been created (via the 
 *         callback function registered by HF_Register).  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_BUSY - The operation failed because a connection is already 
 *         open to the remote device, or a new connection is being created.  
 *
 *     BT_STATUS_FAILED - The channel has not been registered.  
 *
 *     BT_STATUS_CONNECTION_FAILED - The connection failed.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 *      
 *     BT_STATUS_NO_RESOURCES - Not enough channel to create link
 *
 */
BtStatus hf_connect(BD_ADDR *Addr);

/*---------------------------------------------------------------------------
 * hf_disconnect()
 *
 *     This function releases the service level connection with the Audio 
 *     Gateway.  This will close the RFCOMM channel and will also close the 
 *     SCO and ACL links if no other services are active, and no other link 
 *     handlers are in use (ME_CreateLink).  When the operation is complete 
 *     the application will be notified by the HF_EVENT_SERVICE_DISCONNECTED 
 *     event.  
 *
 * Parameters:
 *     Channel - Pointer to a registered channel structure.  
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started; the application will 
 *         be notified when the service level connection is down (via the 
 *         callback function registered by HF_Register).
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 */
BtStatus hf_disconnect(void *Chan);

/*---------------------------------------------------------------------------
 * hf_create_audio_link()
 *
 *     This function creates an audio (SCO) link to the Audio Gateway.  The 
 *     success of the operation is indicated by the HF_EVENT_AUDIO_CONNECTED 
 *     event.  If the connection fails, the application is notified by the 
 *     HF_EVENT_AUDIO_DISCONNECTED event.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started; the application will 
 *         be notified when the audio link has been established (via the 
 *         callback function registered by HF_Register).  
 *
 *     BT_STATUS_SUCCESS - The audio (SCO) link already exists.  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service level 
 *         connection does not exist to the audio gateway.  
 *
 *     BT_STATUS_FAILED - An audio connection already exists.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 */
BtStatus hf_create_audio_link(void *Chan);

/*---------------------------------------------------------------------------
 * hf_disconnect_audio_link()
 *
 *     This function releases the audio connection with the Audio Gateway.  
 *     When the operation is complete the application will be notified by 
 *     the HF_EVENT_AUDIO_DISCONNECTED event.  
 * 
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started; the application will 
 *         be notified when the audio connection is down (via the callback 
 *         function registered by HF_Register).  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 */
BtStatus hf_disconnect_audio_link(void *Chan);

/*---------------------------------------------------------------------------
 * hf_answer_call()
 *
 *     This function answers an incoming call.  This function is called 
 *     after receiving a HF_EVENT_CALL_STATE event that indicates an 
 *     incoming call.  To reject the incoming call, use the HF_Hangup 
 *     function.  When the call is accepted or rejected by the gateway, the 
 *     application will be notified of the call state change by the 
 *     HF_EVENT_CALL_STATE event.  
 *
 *     In addition, the HF_EVENT_COMMAND_COMPLETE event will be received.  
 *
 *     When an active call exists and a second incoming call is indicated, 
 *     the HF_AnswerCall function will perform the equivalent of HF_CallHold 
 *     with an "HfHoldAction" of HF_HOLD_HOLD_ACTIVE_CALLS.  If call waiting 
 *     is disabled, notification of a second incoming call will not occur 
 *     (see HF_EnableCallWaitNotify).  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_FAILED - No incoming call exists.  
 *
 *     BT_STATUS_NO_RESOURCES - Not enough memory to create cmd parameter
 */
BtStatus hf_answer_call(void *Chan);

/*---------------------------------------------------------------------------
 * hf_dial_number()
 *
 *     This function initiates an outgoing call using a phone number.  
 *
 *     During the process of calling, the HF_EVENT_CALL_STATE event will be 
 *     generated to show the progress of the call.  Not all states are 
 *     applicable to all services.  At a minimum, the application will be 
 *     notified of the following states:  HF_CALL_STATUS_DIALING and 
 *     HF_CALL_STATUS_ACTIVE.  
 *
 *     In addition, the HF_EVENT_COMMAND_COMPLETE event will be received.  
 *
 *     If a call is already active, it must be put on hold before calling 
 *     this function.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 *     Number - An ASCII string containing the number to be dialed.  Until 
 *         the Bluetooth stack sends the HF_EVENT_COMMAND_COMPLETE event for 
 *         this command back to the application, the application must not 
 *         reuse the memory space in this string, but instead must use 
 *         different strings for sending additional commands.  
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_FAILED - A call cannot be made.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NO_RESOURCES - Not enough memory to create cmd parameter
 */
BtStatus hf_dial_number(void *Chan, uint8_t *Number, uint16_t Len);


/*---------------------------------------------------------------------------
 * hf_redial()
 *
 *     This function initiates an outgoing call based on the last number 
 *     dialed in the audio gateway.  
 *
 *     During the process of calling, the HF_EVENT_CALL_STATE event will be 
 *     generated to show the progress of the call.  Not all states are 
 *     applicable to all services.  At a minimum, the application will be 
 *     notified of the following states:  HF_CALL_STATUS_DIALING and 
 *     HF_CALL_STATUS_ACTIVE.  
 *
 *     In addition, the HF_EVENT_COMMAND_COMPLETE event will be received.  
 *
 *     If a call is already active, it must be put on hold before calling 
 *     this function.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_FAILED - A call cannot be made.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NO_RESOURCES - Not enough memory to create cmd parameter
 */
BtStatus hf_redial(void *Chan);

/*---------------------------------------------------------------------------
 * hf_call_hold()
 *
 *     This function issues a command to the Audio Gateway to manage 
 *     multi-party calling.  This function allows the application to perform 
 *     explicit handling of 3-Way calls (see HfHoldAction).  During the 
 *     process of this command, the HF_EVENT_CALL_STATE event will be 
 *     generated to show the state change of the call.  
 *
 *     In addition, the HF_EVENT_COMMAND_COMPLETE event will be received.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 *     HoldAction - Describes the type of hold function.  
 *
 *     Index - Call to which the action applies.  Ignored if HoldAction is 
 *            not 1 or 2.  
 *
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NO_RESOURCES - Not enough memory to create cmd parameter
 */
BtStatus hf_call_hold(void *Chan, HfHoldAction HoldAction, uint8_t Index);

/*---------------------------------------------------------------------------
 * hf_hang_up()
 *
 *     This function terminates an existing (active) call, rejects an 
 *     incoming call, or cancels an outgoing call.  This function can be 
 *     called whenever an active call exists or after receiving a 
 *     HF_EVENT_CALL_STATE event that indicates an incoming or outgoing 
 *     call.  When the call is terminated, the application will be notified 
 *     of the call state change by the HF_EVENT_CALL_STATE event.
 *
 *     In addition, the HF_EVENT_COMMAND_COMPLETE event will be received.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.
 *
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_FAILED - No call exists. 
 *
 *     BT_STATUS_NO_RESOURCES - Not enough memory to create cmd parameter
 */
BtStatus hf_hang_up(void *Chan);

/*---------------------------------------------------------------------------
 * hf_list_current_calls()
 *
 *     This function queries the Audio Gateway for call state information.  
 *
 *     After making this call, the HF_EVENT_CURRENT_CALL_STATE event will be 
 *     generated for each call on the audio gateway.  
 *
 *     If HF_USE_CALL_MANAGER is set to XA_ENABLED, an HF_EVENT_CALL_STATE 
 *     event is received instead of the HF_EVENT_CURRENT_CALL_STATE event.  
 *
 *     The HF_EVENT_COMMAND_COMPLETE event will be received when the 
 *     gateway signals that all calls have been listed.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NO_RESOURCES - Not enough memory to create cmd parameter
 */
BtStatus hf_list_current_calls(void *Chan);

/*---------------------------------------------------------------------------
 * hf_enable_caller_id_notify()
 *
 *     This function enables notification of the calling line 
 *     identification.  When this command is complete, the 
 *     HF_EVENT_COMMAND_COMPLETE event will be received.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 *     Enabled - TRUE or FALSE.  
 *
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NO_RESOURCES - Not enough memory to create cmd parameter
 */
BtStatus hf_enable_caller_id_notify(void        *Chan, uint32_t Enabled);


/*---------------------------------------------------------------------------
 * hf_enable_call_wait_notify()
 *
 *     This function enables notification of call waiting.  When this 
 *     command is complete, the HF_EVENT_COMMAND_COMPLETE event will be 
 *     received.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 *     Enabled - TRUE or FALSE.  
 *
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NOT_SUPPORTED - Call waiting was not included in the list 
 *         of supported features (HF_SDK_FEATURES).  
 *
 *     BT_STATUS_NO_RESOURCES - Not enough memory to create cmd parameter
 */

BtStatus hf_enable_call_wait_notify(void        *Chan,uint32_t Enabled)
;

/*---------------------------------------------------------------------------
 * hf_generate_dtmf()
 *
 *     This function commands the Audio Gateway to send a DTMF code to the 
 *     network.  A call MUST be ongoing in order for the Audio Gateway to 
 *     generate a DTMF code.  The HF_EVENT_COMMAND_COMPLETE event will be 
 *     received when the DTMF code is sent.  
 *
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 *     dtmfTone - A single ASCII character in the set 0-9, #, *, A-D.  
 *
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_BUSY - No available RFCOMM packets.  
 *
 *     BT_STATUS_FAILED - A service link is not active.  
 *
 *     BT_STATUS_INVALID_PARM - Invalid ASCII character.  
 *
 *     BT_STATUS_NO_RESOURCES - Not enough memory to create cmd parameter
 */

BtStatus hf_generate_dtmf(void *Chan, uint8_t dtmfTone);
    
    
/*---------------------------------------------------------------------------
 * hf_enable_voice_recognition()
 *
 *     This function enables or disables voice recognition on the Audio 
 *     Gateway.  When this command is complete, the 
 *     HF_EVENT_COMMAND_COMPLETE event will be received.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 *     Enabled - Set to TRUE if voice recognition is enabled, and FALSE 
 *         if it is disabled.  
 *
 *
 * Returns:
 *     BT_STATUS_SUCCESS - The specified mode is already set.  
 *
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_FAILED - Could not initiate voice recognition.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NO_RESOURCES - Not enough memory to create cmd parameter
 */

BtStatus hf_enable_voice_recognition(void *Chan, uint32_t Enabled);


/*---------------------------------------------------------------------------
 * hf_disable_nrec()
 *
 *     This function disables noise reduction and echo canceling.  When this 
 *     command is complete, the HF_EVENT_COMMAND_COMPLETE event will be 
 *     received.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NO_RESOURCES - Not enough memory to create cmd parameter
 */

BtStatus hf_disable_nrec(void *Chan);

/*---------------------------------------------------------------------------
 * hf_send_at_command()
 *
 *     This function sends any AT command.  The 'AtString' parameter must be 
 *     initialized and the AT command must be a properly formatted AT 
 *     Command.  The "AT" characters must be included in the string when 
 *     needed.  A carriage return character will be appended to the end of 
 *     the string.  
 *
 *     When the AT command is completed, the HF_EVENT_COMMAND_COMPLETE 
 *     event will be received by the application's callback function.  In 
 *     addition, any unsolicited result code not recognized by the 
 *     Hands-free SDK will generate an HF_EVENT_AT_RESULT_DATA event.  
 *
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 *     AtString - A pointer to an initialized AT Command string.  Until the 
 *         Bluetooth stack sends the HF_EVENT_COMMAND_COMPLETE event for 
 *         this command back to the application, the application must not 
 *         reuse the memory space in this string, but instead must use 
 *         different strings for sending additional commands.  
 *   
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NO_RESOURCES - Not enough memory to create cmd parameter
 */

BtStatus hf_send_at_command(void  *Chan, const char *AtString);
/*---------------------------------------------------------------------------
 * hf_report_mic_volume()
 *
 *     This function reports the current microphone gain of the Hands-Free 
 *     device.  
 *
 *     When the command issued as a result of this call is completed, 
 *     the HF_EVENT_COMMAND_COMPLETE event will be received by the 
 *     application's callback function.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 *     Gain - The current gain level (0-15).  
 *
 *     Command - A command structure to be used for transmitting the 
 *         command.  Until the Bluetooth stack sends the 
 *         HF_EVENT_COMMAND_COMPLETE event for this command back to the 
 *         application, the application must not reuse the memory space in 
 *         this structure, but instead must use different command structure 
 *         instances for sending additional commands.  
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 * 
 *     BT_STATUS_NO_RESOURCES - Not enough memory to create cmd parameter
 */
BtStatus hf_report_mic_volume(void *Chan, uint8_t Gain);

/*---------------------------------------------------------------------------
 * hf_report_speaker_volume()
 *
 *     This function reports the current speaker volume of the Hands-Free 
 *     Unit.  
 *
 *     When the command issued as a result of this call is completed, 
 *     the HF_EVENT_COMMAND_COMPLETE event will be received by the 
 *     application's callback function.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 *     Gain - The current gain level (0-15).  
 *
 *     Command - A command structure to be used for transmitting the 
 *         command.  Until the Bluetooth stack sends the 
 *         HF_EVENT_COMMAND_COMPLETE event for this command back to the 
 *         application, the application must not reuse the memory space in 
 *         this structure, but instead must use different command structure 
 *         instances for sending additional commands.  
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NO_RESOURCES - Not enough memory to create cmd parameter
 */
BtStatus hf_report_speaker_volume(void *Chan, uint8_t Gain);

/*---------------------------------------------------------------------------
 * hf_gateway_features()
 *
 *     This function provides the features of the Audio Gateway.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 * Returns:
 *     HfGatewayFeatures
 *
 */
HfGatewayFeatures hf_gateway_features(void *Chan);

/*---------------------------------------------------------------------------
 * hf_gateway_hold_features()
 *
 *     This function provides the hold features of the audio gateway.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 * Returns:
 *     HfGwHoldFeatures
 *
 */
HfGwHoldFeatures hf_gateway_hold_features(void *Chan);


/*---------------------------------------------------------------------------
 * hf_is_nrec_enabled()
 *
 *     This function indicates whether Noise Reduction and Echo Cancelling 
 *     is enabled in the Audio Gateway.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 * Returns:
 *     TRUE - NREC is enabled in the AG.  
 *
 *     FALSE - NREC is disabled in the AG.  
 */
int hf_is_nrec_enabled(void *Chan);

/*---------------------------------------------------------------------------
 * hf_is_inbandring_enabled()
 *
 *     This function indicates whether In-band Ringing is enabled in the 
 *     Audio Gateway.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 * Returns:
 *     TRUE - In-band ringing is enabled in the AG.  
 *
 *     FALSE - In-band ringing is disabled in the AG.  
 */
int hf_is_inbandring_enabled(void *Chan);

/*---------------------------------------------------------------------------
 * hf_is_caller_id_notify_enabled()
 *
 *     This function indicates whether Caller ID notification is enabled in 
 *     the Audio Gateway.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 * Returns:
 *     TRUE - Caller ID notification is enabled in the AG.  
 *
 *     FALSE - Caller ID notification is disabled in the AG.  
 */
int hf_is_caller_id_notify_enabled(void *Chan);


/*---------------------------------------------------------------------------
 * hf_is_voice_rec_active()
 *
 *     This function indicates whether Voice Recognition is active in the 
 *     Audio Gateway.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 * Returns:
 *     TRUE - Voice Recognition is active in the AG.  
 *
 *     FALSE - Voice Recognition is inactive in the AG.  
 */
int hf_is_voice_rec_active(void *Chan);



/*---------------------------------------------------------------------------
 * hf_is_call_waiting_active()
 *
 *     This function indicates whether Call Waiting is active in the Audio 
 *     Gateway.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 * Returns:
 *     TRUE - Call Waiting is active in the AG.  
 *
 *     FALSE - Call Waiting is inactive in the AG.  
 */
int hf_is_call_waiting_active(void *Chan);

/*---------------------------------------------------------------------------
 * HF_QuerySubscriberNumber()
 *
 *     This function queries the Audio Gateway for the Subscriber Number.  
 *     The HF_EVENT_SUBSCRIBER_NUMBER event will be received from the audio 
 *     gateway.  
 *
 *     When the command issued as a result of this call is completed, 
 *     the HF_EVENT_COMMAND_COMPLETE event will be received by the 
 *     application's callback function.  
 *
 * Parameters:
 *     Chan - Pointer to a registered channel structure.  
 *
 * Returns:
 *     BT_STATUS_PENDING - The operation has started, the application will 
 *         be notified when the command has completed (via the callback 
 *         function registered by HF_Register).  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel has not been registered.  
 *
 *     BT_STATUS_NO_CONNECTION - The operation failed because a service link 
 *         does not exist to the audio gateway.  
 *
 *     BT_STATUS_INVALID_PARM - A parameter invalid or not properly 
 *         initialized (XA_ERROR_CHECK only).  
 */
BtStatus hf_query_subscriber_number(void *Chan);


#endif
