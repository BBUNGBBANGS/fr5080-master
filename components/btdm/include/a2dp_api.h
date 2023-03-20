#ifndef _A2DP_API_H
#define _A2DP_API_H
#include "co_list.h"
#include "bt_error.h" 
#include "type_lib.h"
#define AVDTP_MAX_CODEC_ELEM_SIZE   6


/*---------------------------------------------------------------------------
 * A2dpEvent type
 *      
 *     All indications and confirmations are sent through a callback 
 *     function as events. The type of event will determine which 
 *     fields in the A2dpCallbackParms structure are valid.  The 'len', and 
 *     'error' fields are always valid.  The 'event' field contains the event 
 *     associated with the callback.  
 */
typedef unsigned char A2dpEvent;
                                        
/** This event is received by the application when the remote device is 
 *  requesting that stream be opened.  The application can accept or reject 
 *  this request by calling A2DP_OpenStreamRsp().  
 *  
 */
#define A2DP_EVENT_STREAM_OPEN_IND           1

/** This event is received by the application when a stream is open.  This 
 *  can happen in response to a call from A2DP_OpenStream() or 
 *  A2D_OpenStreamRsp().  
 * 
 */
#define A2DP_EVENT_STREAM_OPEN               2

#if 0
///not used
/** This event is received by the application during the establishment of an 
 *  outgoing stream (A2DP_OpenStream()) to indicate the capabilities of the 
 *  remote device.  This event may also be received after calling 
 *  A2DP_GetStreamCapabilities().  
 * 
 *  During this callback, the 'p.codec' parameter is valid.  
 */
#define A2DP_EVENT_CODEC_INFO                3

/** This event is received by the application during the establishment of an 
 *  outgoing stream (A2DP_OpenStream()) to indicated the capabilities of the 
 *  remote device.  This event may also be received after calling 
 *  A2DP_GetStreamCapabilities().  
 * 
 *  During this callback, the 'p.cp' parameter is valid.  
 */
#define A2DP_EVENT_CP_INFO                   4

/** This event is received by the application during the establishment of an 
 *  outgoing stream (A2DP_OpenStream()) to indicated the capabilities of the 
 *  remote device.  If this event is received, then the remote device
 *  supports the Delay Reporting feature.
 */
#define A2DP_EVENT_DELAY_REPORTING_INFO      5

/** This event is received by the application when a stream is opening and 
 *  all matching capabilities have been found.  The application must call 
 *  A2DP_SetStreamConfig() to configure the stream.  If successful, the 
 *  stream will be opened.  A2DP_CloseStream() can also be called to close 
 *  the stream.  This event is only received on outgoing connections.  
 */
#define A2DP_EVENT_GET_CONFIG_IND            6

/** This event is received after calling A2DP_GetStreamCapabilities().  This 
 *  event is received after all the capabilities of the remote device have 
 *  been indicated (see A2DP_EVENT_CODEC_INFO and A2DP_EVENT_CP_INFO).  
 */
#define A2DP_EVENT_GET_CAP_CNF               7
#endif

/** This event is received by the application when an open stream has been 
 *  closed.  This can happen as a result of a call to A2DP_CloseStream(), if 
 *  the stream has been closed by the remote device, if a link loss has been 
 *  detected, or if the remote device rejects a request to open the stream.  
 *

 *  The 'error' field will contain the error code. If the close was 
 *  successful, 'error' will contain AVDTP_ERR_NO_ERROR.  
 *  
 *  If the stream was closed because of an L2CAP disconnect, the 'status'
 *  field will contain the L2CAP disconnect reason (L2capDiscReason),
 *  otherwise it will contain L2DISC_REASON_UNKNOWN.
 */
#define A2DP_EVENT_STREAM_CLOSED             8

/** This event is received by the application when an open stream has been 
 *  set to the idle state.  This happens as a result of a call to 
 *  A2DP_IdleStream().  
 */
#define A2DP_EVENT_STREAM_IDLE               9

/** When the remote device requests streaming to begin, this event will be 
 *  received by the application.  The application can accept or reject 
 *  this request by calling A2DP_StartStreamRsp(). (Note: this event 
 *  will be received only after A2DP_EVENT_STREAM_OPEN_IND but it may arrive 
 *  before A2DP_EVENT_STREAM_OPEN.)  
 */
#define A2DP_EVENT_STREAM_START_IND         10

/** When streaming has been started, this event will be received by the 
 *  application.  This can happen as the result to a call to 
 *  A2DP_StartStream() or A2DP_StartStreamRsp().  
 */
#define A2DP_EVENT_STREAM_STARTED           11

/** When streaming has been suspended, this event will be received by the 
 *  application.  This happens as the result to a call to 
 *  A2DP_SuspendStream(), or when the remote device suspends the stream.  
 */
#define A2DP_EVENT_STREAM_SUSPENDED         12

#if 0
///not used
/** When the remote device wishes to reconfigure an open stream, this event 
 *  is received by the application.  The application can accept or reject 
 *  the request by calling A2DP_ReconfigStreamRsp().  
 * 
 *  During this callback, the 'p.configReq' parameter will be valid.  
 */
#define A2DP_EVENT_STREAM_RECONFIG_IND      13

/** When an open stream is reconfigured, this event is received by the 
 *  application.  
 * 
 *  During this callback, the 'p.configReq' parameter is valid when the 
 *  'error' field is set to AVDTP_ERR_NO_ERROR.  Otherwise, 
 *  'p.capability.type' contains the capability that caused the failure.  
 */
#define A2DP_EVENT_STREAM_RECONFIG_CNF      14

/** This event is received when the remote device requests the security 
 *  process to begin.  The application responds to this request by calling 
 *  A2DP_SecurityControlRsp().  
 * 
 *  If 'error' contains no error, the 'p.data' parameter is valid.  
 */
#define A2DP_EVENT_STREAM_SECURITY_IND      15

/** This event is received by the application when the remote device responds 
 *  to the security process request.  
 * 
 *  If 'error' contains no error, the 'p.data' parameter is valid.  
 */
#define A2DP_EVENT_STREAM_SECURITY_CNF      16

/** When the stream is aborted, this event is received by the application.  
 *  This can happen in response to a request from the remote device to abort 
 *  the stream, or as the result of a call to A2DP_AbortStream().  When a 
 *  stream is aborted, the stream is closed.  
 */
#define A2DP_EVENT_STREAM_ABORTED           17
#endif

/** This event is received when stream data has been received from the remote 
 *  device.  The data is raw and is not parsed by A2DP.  It should contain 
 *  a single media packet.  
 * 
 *  If 'error' contains no error, the 'p.data' parameter is valid.  
 */
#define A2DP_EVENT_STREAM_DATA_IND          18

/** This event is received when raw data has been sent to the remote device.  
 *  This happens as the result of a call to A2DP_StreamSendRawPacket().  
 * 
 *  During this callback, the 'p.packet' parameter is valid.  
 */
#define A2DP_EVENT_STREAM_PACKET_SENT       19

/** This event is received when SBC data has been sent to the remote device.  
 *  This happens as the result of a call to A2DP_StreamSendSbcPacket().  
 */
#define A2DP_EVENT_STREAM_SBC_PACKET_SENT   20

#if 0
///not used
/** This event is received by a Source when the Sink reports the value of its
 *  buffer/processing delay.  This may happen when the stream is configured
 *  (or reconfigured), and when the stream is in the streaming state.
 *  
 *  During this callback, the 'p.delayMs' parameter is valid.
 */
#define A2DP_EVENT_DELAY_REPORT_IND         21

/** This event is received by a Sink when the Source acknowldeges the
 *  transmitted buffer/processing delay.
 */
#define A2DP_EVENT_DELAY_REPORT_CNF         22

/** When the the lower level AVDTP connection is established between the
 *  local and remote devices, this event will be generated.
 *  
 *  During this callback, the 'p.device' parameter contains a pointer to the
 *  device that was connected.
 */
#define A2DP_EVENT_AVDTP_CONNECT            23

/** When the the lower level AVDTP connection is disconnected, this event
 *  will be generated.
 *  
 *  During this callback, the 'p.device' parameter contains a pointer to the
 *  device that was disconnected.  The 'status' parameter contains the L2CAP
 *  disconnect reason (L2capDiscReason).  If 'status' equals
 *  L2DISC_LINK_DISCON, then the 'discReason' parameter contains the reason
 *  for the link disconnect (BtErrorCode).
 */
#define A2DP_EVENT_AVDTP_DISCONNECT         24
#endif

/* End of A2dpEvent */ 

/*---------------------------------------------------------------------------
 * A2dpError type
 * 
 *     Error codes for the A2DP profile.  If errors are detected in the
 *     Codec or Content protection requests, these error codes are used.
 *     Each error code description indicates the codec to which the error
 *     is applicable.
 *
 *     This variable type can also contain AVDTP errors.  See AvdtpError
 *     for more error codes.
 */
typedef uint8_t A2dpError;

/** No error
 */
#define A2DP_ERR_NO_ERROR                         0x00

/** Bad Service
 * 
 *  Generic
 */
#define A2DP_ERR_BAD_SERVICE                      0x80

/** Insufficient Resources 
 * 
 *  Generic
 */
#define A2DP_ERR_INSUFFICIENT_RESOURCE            0x81

/** Codec media type not valid
 *
 *  Generic
 */
#define A2DP_ERR_INVALID_CODEC_TYPE               0xC1

/** Codec media type not supported
 * 
 *  Generic
 */
#define A2DP_ERR_NOT_SUPPORTED_CODEC_TYPE   AVDTP_ERR_NOT_SUPPORTED_CODEC_TYPE

/** Sampling frequency not valid
 * 
 *  Generic
 */
#define A2DP_ERR_INVALID_SAMPLING_FREQUENCY       0xC3

/** Sampling frequency not supported
 * 
 *  Generic
 */
#define A2DP_ERR_NOT_SUPPORTED_SAMP_FREQ          0xC4

/** Channel mode not valid
 * 
 *  SBC
 *  MPEG-1,2 Audio
 *  ATRAC family
 */
#define A2DP_ERR_INVALID_CHANNEL_MODE             0xC5

/** Channel mode not supported
 * 
 *  SBC
 *  MPEG-1,2 Audio
 *  ATRAC family
 */
#define A2DP_ERR_NOT_SUPPORTED_CHANNEL_MODE       0xC6

/** Number of subbands not valid
 * 
 *  SBC
 */
#define A2DP_ERR_INVALID_SUBBANDS                 0xC7

/** Number of subbands not supported
 * 
 *  SBC
 */
#define A2DP_ERR_NOT_SUPPORTED_SUBBANDS           0xC8

/** Allocation method not valid
 * 
 *  SBC
 */
#define A2DP_ERR_INVALID_ALLOCATION_METHOD        0xC9

/** Allocation method not supported
 * 
 *  SBC
 */
#define A2DP_ERR_NOT_SUPPORTED_ALLOC_METHOD       0xCA

/** Minimum bitpool value not valid
 * 
 *  SBC
 */
#define A2DP_ERR_INVALID_MIN_BITPOOL_VALUE        0xCB

/** Minimum bitpool value not supported
 * 
 *  SBC
 */
#define A2DP_ERR_NOT_SUPPORTED_MIN_BITPOOL_VALUE  0xCC

/** Maximum bitpool value not valid
 * 
 *  SBC
 */
#define A2DP_ERR_INVALID_MAX_BITPOOL_VALUE        0xCD

/** Maximum bitpool value not supported
 * 
 *  SBC
 */
#define A2DP_ERR_NOT_SUPPORTED_MAX_BITPOOL_VALUE  0xCE

/** None or multiple values have been selected for Layer
 * 
 *  MPEG-1,2 Audio
 */
#define A2DP_ERR_INVALID_LAYER                    0xCF

/** Layer is not supported
 * 
 *  MPEG-1,2 Audio
 */
#define A2DP_ERR_NOT_SUPPORTED_LAYER              0xD0

/** CRC is not supported
 * 
 *  MPEG-1,2 Audio
 */
#define A2DP_ERR_NOT_SUPPORTED_CRC                0xD1

/** MPF-2 is not supported
 * 
 *  MPEG-1,2 Audio
 */
#define A2DP_ERR_NOT_SUPPORTED_MPF                0xD2

/** VBR is not supported
 * 
 *  MPEG-1,2 Audio
 *  MPEG-2,4 AAC
 *  ATRAC family
 */
#define A2DP_ERR_NOT_SUPPORTED_VBR                0xD3

/** None or multiple values have been selected for Bit Rate
 * 
 *  MPEG-1,2 Audio
 *  ATRAC family
 */
#define A2DP_ERR_INVALID_BIT_RATE                 0xD4

/** Bit Rate is not supported
 * 
 *  MPEG-1,2 Audio
 *  MPEG-2,4 AAC
 *  ATRAC family
 */
#define A2DP_ERR_NOT_SUPPORTED_BIT_RATE           0xD5

/** Either 1) Object type is not valid (b3-b0) or 2) None or multiple values 
 *  have been selected for Object Type
 * 
 *  MPEG-2,4 AAC
 */
#define A2DP_ERR_INVALID_OBJECT_TYPE              0xD6

/** Object Type is not supported
 * 
 *  MPEG-2,4 AAC
 */
#define A2DP_ERR_NOT_SUPPORTED_OBJECT_TYPE        0xD7

/** None or multiple values have been selected for Channels
 * 
 *  MPEG-2,4 AAC
 */
#define A2DP_ERR_INVALID_CHANNELS                 0xD8

/** Channels is not supported
 * 
 *  MPEG-2,4 AAC
 */
#define A2DP_ERR_NOT_SUPPORTED_CHANNELS           0xD9

/** Version is not valid
 * 
 *  ATRAC family
 */
#define A2DP_ERR_INVALID_VERSION                  0xDA

/** Version is not supported
 * 
 *  ATRAC family
 */
#define A2DP_ERR_NOT_SUPPORTED_VERSION            0xDB

/** Maximum SUL is not acceptable for the Decoder in the SNK
 * 
 *  ATRAC family
 */
#define A2DP_ERR_NOT_SUPPORTED_MAXIMUM_SUL        0xDC

/** None or multiple values have been selected for Block Length
 * 
 *  SBC
 */
#define A2DP_ERR_INVALID_BLOCK_LENGTH             0xDD

/** The requested CP Type is not supported
 * 
 *  Generic
 */
#define A2DP_ERR_INVALID_CP_TYPE                  0xE0

/** The format of Content Protection Service Capability/Content Protection 
 *  Scheme Dependent Data is not correct 
 * 
 *  Generic
 */
#define A2DP_ERR_INVALID_CP_FORMAT                0xE1

/** Unknown error
 */
#define A2DP_ERR_UNKNOWN_ERROR                    AVDTP_ERR_UNKNOWN_ERROR

/* End of A2dpError */ 

/*---------------------------------------------------------------------------
 * BtPacket structure
 *
 *     Represents a packet of data. These packets may be used by applications
 *     to provide data to the stack. Certain stack events may also provide
 *     data to the application enclosed in a BtPacket structure.
 */
typedef struct _BtPacket
{
    co_list_t    node;    /* Used internally by the stack. */

    uint8_t          *data;    /* Points to a buffer of user data.  */
    uint16_t          dataLen; /* Indicates the length of "data" in bytes. */

    uint16_t          flags;   /* Must be initialized to BTP_FLAG_NONE by
                           * applications running on top of L2CAP. Other
                           * higher layer protocols must never modify this
                           * value.
                           */
    /* Group: The following fields are for internal use only by the stack. */
    void        *ulpContext;
    uint8_t          *tail;
    uint16_t          tailLen;

    uint16_t          llpContext;
    uint16_t          remoteCid;

    uint16_t          offset;
    uint8_t           headerLen;
    uint8_t           header[28];
} BtPacket;


/*---------------------------------------------------------------------------
 * A2dpSbcPacket structure
 *
 * Used for transmitting SBC data.  
 */
typedef struct _A2dpSbcPacket {
    
    void           *resv1;           /* Used internally by A2DP  */ 
    void           *resv2;           /* Used internally by A2DP  */
    uint8_t       *data;             /* Pointer to transmit data */ 
    uint16_t       dataLen;          /* Length of transmit data  */ 
    uint16_t       frameSize;        /* Size of an SBC frame     */ 

    /* === Internal use only === */ 
    BtPacket  packet;           /* For sending over L2CAP   */ 
    uint16_t       dataSent;
    uint16_t       frmDataSent;
} A2dpSbcPacket;



typedef struct _SbcStreamInfo {

    /* Bitpool affects the bitrate of the stream according to the following 
     * formula: bit_rate = 8 * frameLength * sampleFreq / numSubBands / 
     * numBlocks.  The frameLength value can be determined by setting the 
     * bitPool value and calling the SBC_FrameLen() function.  
     * Bitpool can be changed dynamically from frame to frame during 
     * encode/decode without suspending the stream.  
     */
    uint8_t   bitPool;

    /* Sampling frequency of the stream */ 
    uint8_t sampleFreq;

    /* The channel (Mono, Stereo, etc.) */ 
    uint8_t channelMode;

    /* The allocation method for the stream */ 
    uint8_t allocMethod;

    /* Number of blocks used to encode the stream (4, 8, 12, or 16) */ 
    uint8_t   numBlocks;

    /* The number of subbands in the stream (4 or 8) */ 
    uint8_t   numSubBands;

    /* The number of channels in the stream (calculated from channelMode) */ 
    uint8_t   numChannels;

} SbcStreamInfo;

/*---------------------------------------------------------------------------
 * AvdtpCodecType type
 *
 * Defines codes types used by AVDTP.
 */
typedef uint8_t AadtpCodecType;

#define AVDTP_CODEC_TYPE_SBC           0x00

#define AVDTP_CODEC_TYPE_MPEG1_2_AUDIO 0x01

#define AVDTP_CODEC_TYPE_MPEG2_4_AAC   0x02

#define AVDTP_CODEC_TYPE_NON_A2DP      0xFF

/* End of AvdtpCodecType */

/*---------------------------------------------------------------------------
 * AvdtpStrmEndPointType type
 * 
 * Defines the stream endpoint types for AVDTP  
 */
typedef uint8_t AvdtpStrmEndPointType;

/* Stream Source */ 
#define AVDTP_STRM_ENDPOINT_SRC       0

/* Stream Sink */ 
#define AVDTP_STRM_ENDPOINT_SNK       1

/* End of AvdtpStrmEndPointType */ 

/*---------------------------------------------------------------------------
 * A2dpCodec structure
 *
 * Used to describe the codec type and elements.
 */
typedef struct _AvdtpCodec {
    AadtpCodecType  codecType;         /* The type of codec */ 
    uint8_t              elemLen;           /* Length of the codec elements */ 

    uint8_t              elements[AVDTP_MAX_CODEC_ELEM_SIZE];

    uint8_t              freq;              /* add by owen for app usage */
                                       /* updated in A2DP_EVENT_STREAM_OPEN_IND event */
     
    uint32_t            discoverable;      /* Used internally by AVDTP */
} AvdtpCodec;


typedef struct{
    A2dpEvent event;
    BD_ADDR *addr;
    void *stream;
    AvdtpCodec *codec;
    A2dpSbcPacket      *sbcPacket;  /* SBC Transmit packet           */ 
}a2dp_event_t;

enum bt_a2dp_type_t{
    BT_A2DP_TYPE_SINK,                         //设置a2dp为audio sink 
    BT_A2DP_TYPE_SRC,                          //设置a2dp为audio source
    BT_A2DP_TYPE_MAX,
};

typedef void(* a2dp_callback_func_t)(a2dp_event_t * event);
void bt_a2dp_set_cb_func(a2dp_callback_func_t func);


/****************************************************************************
 *
 * Function Reference
 *
 ****************************************************************************/

/*---------------------------------------------------------------------------
 * a2dp_start_stream()
 *
 *     Initiates streaming on the open stream.  Calling this function puts 
 *     the stream into an active state, which allows media packets to be sent 
 *     on the stream.  
 *
 * Parameters:
 *
 *     Stream - A registered and open A2DP stream.  
 *
 * Returns:
 *
 *     BT_STATUS_PENDING - The operation was initiated successfully.  
 *         An A2DP_EVENT_STREAM_STARTED event will be received when the 
 *         stream is ready for streaming.
 *
 *     BT_STATUS_FAILED - The stream is not in the open state.  
 *
 *     BT_STATUS_INVALID_PARM - The 'Stream' parameter does not contain a 
 *         valid pointer. (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_IN_USE - One of the specified streams is already in use.  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel or stream is not 
 *         registered.  
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).  
 */
BtStatus a2dp_start_stream(void *Stream);

/*---------------------------------------------------------------------------
 * a2dp_start_stream_rsp()
 *
 *     Called in response to an A2DP_EVENT_STREAM_START_IND event.  Calling 
 *     this function will either accept the streaming request or reject it.  
 *
 * Parameters:
 *
 *     Stream - A registered and open A2DP stream.  
 *
 *     Error - If the streaming request is accepted, this parameter must be 
 *         set to A2DP_ERR_NO_ERROR.  If the streaming request is rejected, 
 *         this parameter must be set to the appropriate error defined by 
 *         A2dpError.  
 *
 * Returns:
 *
 *     BT_STATUS_PENDING - The Start Streams Response operation was started 
 *         successfully.  An A2DP_EVENT_STREAM_STARTED event will be received 
 *         when the stream has been started.  If the start stream request was 
 *         rejected, the A2DP_EVENT_STREAM_SUSPENDED event will be received.  
 *
 *     BT_STATUS_FAILED - The stream is not in the open state.  
 *
 *     BT_STATUS_INVALID_PARM - The Stream parameter does not contain a 
 *         valid pointer. (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_IN_USE - One of the specified streams is already in use.  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel or stream is not 
 *         registered.  
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).  
 */
BtStatus a2dp_start_stream_rsp(void *Stream, A2dpError Error);

/*---------------------------------------------------------------------------
 * a2dp_open_stream()
 *
 *    Searches for and begins to open a stream on the remote device.  This 
 *    function searches for a stream with capabilities matching those that 
 *    were registered using A2DP_Register() and 
 *    A2DP_AddContentProtection().  As streams are discovered, the 
 *    application is notified of matching capability types through the 
 *    A2DP_EVENT_CODEC_INFO and A2DP_EVENT_CP_INFO events.  The application 
 *    can cache or verify the specific elements of these capabilities.  When 
 *    all matching capabilities have been discovered, the application is 
 *    notified through the A2DP_EVENT_GET_CONFIG_IND event.  When this event 
 *    is received, the application must call A2DP_SetStreamConfig().  The 
 *    connection progress will not continue until the stream has been 
 *    configured. Once configured, the application will receive the 
 *    A2DP_STREAM_OPEN event.  When this event has been received with no 
 *    errors, the stream is open.  If no streams with matching capabilities 
 *    can be found, the stream is closed.  
 *
 *    A2DP_AbortStream() can be called to close the stream to abort this 
 *    process if necessary.  
 *
 * Parameters:
 *
 *     type - The type of endpoint to connect to (Source or Sink).
 *
 *     Addr - The Bluetooth address of the device to which the connection 
 *            should be made.  If Addr is 0, then a device inquiry will be 
 *            initiated.  A Device Monitor handler must be registered to 
 *            receive events and to select the desired device.  It is only 
 *            possible to use and value of 0 for Addr if connecting to a 
 *            device that is not already connected.  If the device was 
 *            already connected, an attempt to connect again will fail.  To 
 *            establish a new stream to an already connected device, the 
 *            BD_ADDR_T of the connected device must be used in the Addr 
 *            parameter.  
 *
 * Returns:
 *
 *     BT_STATUS_PENDING - The Open Stream operation was started 
 *         successfully.  Codec and Content Protection events will be 
 *         received if a stream is discovered.  
 *
 *     BT_STATUS_INVALID_PARM - The 'Stream', or 'Cp' parameter does not 
 *         contain a valid pointer. (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_IN_USE - The specified stream is already in use.
 * 
 *     BT_STATUS_BUSY - A search is already in progress.
 *
 *     BT_STATUS_NOT_FOUND - The specified channel or stream is not 
 *         registered.  
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).  
 */
BtStatus a2dp_open_stream(AvdtpStrmEndPointType type, BD_ADDR *Addr);

/*---------------------------------------------------------------------------
 * a2dp_open_stream_rsp()
 *
 *    Responds to an request to open a connection with the remote device 
 *    (See A2DP_EVENT_STREAM_OPEN_IND).  The open request is accepted by the 
 *    application if this function is called with A2DP_ERR_NO_ERROR.  Any 
 *    other error code rejects the request.  
 *
 * Parameters:
 *
 *     Stream - A registered and configured A2DP stream.  
 *
 *     Error - If the request to open the connection is granted then 
 *             A2DP_ERR_NO_ERROR is passed in this parameter, otherwise 
 *             another appropriate error code should be used.  
 *
 *     CapType - Capability type associated with the reason for rejecting 
 *               the connection.  If 'Error' is set to A2DP_ERR_NO_ERROR, 
 *               this parameter is ignored.  
 *
 * Returns:
 *
 *     BT_STATUS_PENDING - The Open Stream Response operation was started 
 *         successfully.  An A2DP_EVENT_STREAM_OPEN event will be received 
 *         when the stream is open.  
 *
 *     BT_STATUS_INVALID_PARM - The 'Stream' parameter does not contain a 
 *         valid pointer.  (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NO_CONNECTION - The specified stream did not request a 
 *         connection.  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel or stream is not 
 *         registered.  
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).  
 */
BtStatus a2dp_open_stream_rsp(void       *Stream, 
                           A2dpError           Error, 
                           uint16_t                 delayMs);


/*---------------------------------------------------------------------------
 * a2dp_close_stream()
 *
 *     Initiate the closing an open stream.  
 *
 * Parameters:
 *
 *     Stream - A registered and open A2DP stream.  
 *
 * Returns:
 *
 *     BT_STATUS_PENDING - The Close Stream operation was started 
 *         successfully.  An A2DP_EVENT_STREAM_CLOSED event will be received 
 *         when the stream is closed.  
 *
 *     BT_STATUS_NO_CONNECTION - The stream is not in the open or active 
 *         state.  
 *
 *     BT_STATUS_INVALID_PARM - The 'Stream' parameter does not 
 *         contain a valid pointer. (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel or stream is not 
 *         registered.  
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).  
 */
BtStatus a2dp_close_stream(void *Stream);

/*---------------------------------------------------------------------------
 * a2dp_suspend_stream()
 *
 *     Suspends a stream that is currently active.  
 *
 * Parameters:
 *
 *     Stream - A registered and active A2DP stream.  
 *
 * Returns:
 *
 *     BT_STATUS_PENDING - The Suspend Streams Response operation was started 
 *         successfully.  An A2DP_EVENT_STREAM_SUSPENDED event will be 
 *         received when the stream has been suspended.  
 *
 *     BT_STATUS_FAILED - The stream is not in an active state.  
 *
 *     BT_STATUS_INVALID_PARM - The 'Stream' parameter does not contain a 
 *         valid pointer. (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel or stream is not 
 *         registered.  
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).  
 */
BtStatus a2dp_suspend_stream(void *Stream);

/*---------------------------------------------------------------------------
 * a2dp_abort_stream()
 *
 *     Aborts any open or active stream.  Once aborted, an open stream will 
 *     be in a closed state.  
 *
 * Parameters:
 *
 *     Stream - An open, or active A2DP stream.  
 *
 * Returns:
 *
 *     BT_STATUS_PENDING - The Abort Stream operation was started 
 *         successfully.  An A2DP_EVENT_STREAM_ABORTED event will be received 
 *         when the stream has been aborted.  
 *
 *     BT_STATUS_FAILED - The stream is not in the correct state.  
 *
 *     BT_STATUS_INVALID_PARM - The Stream parameter does not contain a 
 *         valid pointer. (XA_ERROR_CHECK only).  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel or stream is not 
 *         registered.  
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).  
 */
BtStatus a2dp_abort_stream(void *Stream);

/*---------------------------------------------------------------------------
 * a2dp_get_registered_codec()
 *
 *     Returns the registered codec.  
 *
 * Parameters:
 *
 *     Stream - An registered stream.  
 *
 * Returns:
 *
 *     The registered stream codec (see AvtdpCodec).  
 */

AvdtpCodec *a2dp_get_registered_codec(void *Stream);

/*---------------------------------------------------------------------------
 * a2dp_stream_send_sbc_packet()
 *
 *     Sends SBC data on the specified stream.  The stream must be open and 
 *     in an streaming state.  No checking is done on the validity of 
 *     the data format.  Data is delivered to the stream with the media 
 *     packet header and SBC header.  
 *
 * Parameters:
 *
 *     Stream - A registered and active A2DP stream.  
 *
 *     Packet - An initialized SbcPacket structure with valid SBC data.  The 
 *         data, datalen, and FrameSize fields must be initialized.  All 
 *         frames in the buffer to be sent must be the same size, which 
 *         means that the Bitpool value for each frame in the SBC data to be 
 *         transmitted must be the same.  The bitpool value can changed in 
 *         subsequent calls to this function.  
 *
 *     StreamInfo - Information about the SBC stream.  
 *
 * Returns:
 *
 *     BT_STATUS_PENDING - The Write operation was started successfully.  
 *         An A2DP_EVENT_STREAM_SBC_PACKET_SENT event will be received when 
 *         the data has been sent.  
 *
 *     BT_STATUS_FAILED - The stream is not in the active state.  
 *
 *     BT_STATUS_INVALID_PARM - The 'Stream'' 'Packet,' or 'StreamInfo' 
 *         parameter does not contain a valid pointer.  (XA_ERROR_CHECK 
 *         only).  
 *
 *     BT_STATUS_NOT_FOUND - The specified channel or stream is not 
 *         registered.  
 *
 *     Other - It is possible to receive other error codes, depending on the 
 *         lower layer service in use (L2CAP or Management Entity).  
 */
BtStatus a2dp_stream_send_sbc_packet(void         *Stream, 
                                  A2dpSbcPacket *Packet,
                                  SbcStreamInfo *StreamInfo);

/**********************************************************************************
* @fn       bt_get_a2dp_type
* @param    NULL
* @brief    获取当前a2dp 类型 
* @return   返回audio_type，sink 或者source，参考枚举类型bt_a2dp_type_e
***********************************************************************************/
enum bt_a2dp_type_t bt_get_a2dp_type(void);

/**********************************************************************************
* @fn       bt_set_a2dp_type
* @param    type ,audio sink 或�?source，参考枚举类型bt_a2dp_type_e
* @brief    设置a2dp类型，需要在非连接状态情况下进行
* @return   
*   BT_STATUS_FAILED       设置失败
*   BT_STATUS_SUCCESS      设置成功
*
***********************************************************************************/
BtStatus bt_set_a2dp_type(enum bt_a2dp_type_t type);

/**********************************************************************************
* @fn       bt_a2dp_get_connecting_state
* @param    NULL
* @brief    获取当前a2dp 是否正在被连接
* @return   
*   FALSE  没有被连接
*   TRUE   正在被连接
*
***********************************************************************************/
uint8_t  bt_a2dp_get_connecting_state(void);

/**********************************************************************************
* @fn       bt_a2dp_get_sbc_bitpool
* @param    NULL
* @brief    获取当前连接的a2dp codec的bitpool
* @return   
*   val     bitpool大小
*
***********************************************************************************/
uint8_t bt_a2dp_get_sbc_bitpool(void);

/**********************************************************************************
* @fn       a2dp_remove_datalink
* @param    NULL
* @brief    强制关闭正在连接的a2dp profile，重新进入配对状态
* @return   
*   stutus     参考BtStatus
*
***********************************************************************************/
BtStatus a2dp_remove_datalink(void);

#endif
