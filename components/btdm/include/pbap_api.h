#ifndef PBAP_API_H_
#define PBAP_API_H_
#include <stdint.h>


/*---------------------------------------------------------------------------
 * PbapClientCallbackParms structure
 *
 *  Describes a callback event and any data that relates to the event. These
 *  callback parameters are used for all Phonebook Access Clients.  
 *  Determination of the valid fields can be done by evaluating which event 
 *  is being indicated as well as which client it is intended for.
 */
struct _PbapClientCallbackParms_new
{
    uint8_t               event;          /* PBAP event */
    uint32_t              oper;           /* PBAP operation */
    void                  *client;        /* PBAP client */

    union {
#if OBEX_PROVIDE_SDP_RESULTS == XA_ENABLED
        /* Group: During a PBAP_EVENT_TP_CONNECTED event, contains 
         * the SDP parsed information 
         */
        struct {
            uint16_t                     profileVersion; /* PBAP profile version */
            uint8_t                      suppPhonebooks; /* PBAP supported phonebooks */
        } connect;
#endif /* OBEX_PROVIDE_SDP_RESULTS == XA_ENABLED */

        /* Group: Valid during PBAP_EVENT_TP_DISCONNECTED event */
        uint8_t        discReason;     /* PBAP disconnect reason code */

        /* Group: Valid during PBAP_EVENT_ABORTED event */
        uint8_t        abortReason;    /* PBAP abort reason code */

#if OBEX_AUTHENTICATION == XA_ENABLED
        /* Group: Valid during PBAP_EVENT_AUTH_RESULT event */
        struct {
            _Bool        result;         /* Result of the PBAP Authentication attempt */
            uint8_t     reason;         /* PBAP Authentication failure reason */
        } auth;
#endif /* OBEX_AUTHENTICATION == XA_ENABLED */

        /* Group: Valid during PBAP_EVENT_PARAMS_RX event - provides Application
         * Parameter header information.  Valid for Pull Phonebook and
         * Pull Vcard Listing operations only.
         */
        struct {
            /* Number of new missed calls */
            uint8_t             newMissedCalls;
            /* Provides the size of the requested phonebook. The client 
             * should set its MaxListCount based on the phonebook size, 
             * if it is nonzero. 
             */
            uint16_t            phonebookSize;
        } paramsRx;

        /* Group: Valid during PBAP_EVENT_DATA_IND event */
        struct {
            /* Object name (null-terminated, ASCII or UTF-16) */
            uint16_t            *name;           /* Name pointer */
            /* Data Indication */
            uint8_t             *buffer;         /* Data pointer */
            uint16_t             len;            /* Length of data */
        } dataInd;
    } u;
};


typedef struct _Pbap_Vcard_Filter 
{
    /* Array of 8 bytes for this 64-bit filter value */
    uint8_t                  byte[8];
} Pbap_Vcard_Filter;

/* A transport layer connection has been established. There is no
 * operation associated with this event.
 */
#define PBAP_EVENT_TP_CONNECTED         0x01

/* The transport layer connection has been disconnected. There is no
 * operation associated with this event.
 */
#define PBAP_EVENT_TP_DISCONNECTED      0x02

/* Indicates that a phonebook operation (see PbapOp) has 
 * completed successfully.
 */
#define PBAP_EVENT_COMPLETE             0x03

/* Indicates that the current operation has failed or was rejected
 * by the remote device.
 */
#define PBAP_EVENT_ABORTED              0x04

/* Delivered to the application when it is time to issue
 * another request or response. The application must call either
 * PBAP_ServerContinue() or PBAP_ClientContinue().
 */
#define PBAP_EVENT_CONTINUE             0x05

/* Delivered to the application when the Application Parameters
 * header information has been fully parsed.
 */
#define PBAP_EVENT_PARAMS_RX            0x06


#define PBAP_EVENT_DATA_IND             0x0c

/*---------------------------------------------------------------------------
 * PbapOp type
 *
 *     Indicates the operation type of the current event. Each event 
 *     indication has an associated operation passed to a callback function 
 *     of type PbapClientCallback or PbapServerCallback. The 
 *     "PbapCallbackParms.oper" field will indicate one of the operation 
 *     types below.  Since the Pull Phonebook, Pull Vcard Listing, and Pull
 *     Vcard Entry operations cannot be known until the actual OBEX headers
 *     are processed, the initial operation start event will indicate merely
 *     that a generic Pull operation is occurring, until further information
 *     arrives.  The exact operation will be known either during the 
 *     PBAP_EVENT_PARAMS_RX or PBAP_EVENT_OBJECT_LENGTH_REQ events
 */
#define PBAPOP_NONE                0x0000 /* No current operation */
#define PBAPOP_CONNECT             0x0001 /* Connect operation */
#define PBAPOP_DISCONNECT          0x0002 /* Disconnect operation */
#define PBAPOP_SET_PHONEBOOK       0x0004 /* Set Phonebook operation */
#define PBAPOP_PULL                0x0008 /* Generic Pull operation  */
#define PBAPOP_PULL_PHONEBOOK      0x0010 /* Pull Phonebook operation */
#define PBAPOP_PULL_VCARD_LISTING  0x0020 /* Pull vCard listing operation */
#define PBAPOP_PULL_VCARD_ENTRY    0x0040 /* Pull vCard entry operation */


#define PBA_UNINITIALIZED       0x00   /* PBAP profile uninitialized */
#define PBA_IDLE                0x01   /* Transport disconnected, PBAP initialized */   
#define PBA_CONNECTING          0x02   /* Profile connecting */
#define PBA_TP_CONNECTED        0x03   /* Transport connected */
#define PBA_CONNECTED           0x04   /* Profile connected */
#define PBA_DISCONNECTING       0x05   /* Profile disconnecting */   
#define PBA_TP_DISCONNECTING    0x06   /* Transport disconnecting */

/* Group: Failure response codes */

#define PBRC_BAD_REQUEST           0x40 /* Bad Request */
#define PBRC_UNAUTHORIZED          0x41 /* Unauthorized */
#define PBRC_FORBIDDEN             0x43 /* Forbidden - operation is understood */
#define PBRC_NOT_FOUND             0x44 /* Not Found */
#define PBRC_NOT_ACCEPTABLE        0x46 /* Not Acceptable */
#define PBRC_PRECONDITION_FAILED   0x4c /* Precondition Failed */
#define PBRC_NOT_IMPLEMENTED       0x51 /* Not Implemented */
#define PBRC_SERVICE_UNAVAILABLE   0x53 /* Service Unavailable */
#define PBRC_LINK_DISCONNECT       0x80 /* Transport connection has been disconnected. */


#define AUTH_RECEIVED_CHAL      0x01
#define AUTH_AUTHENTICATED      0x02


/*---------------------------------------------------------------------------
 * Pull Phone Type
 */
enum pull_phone_type_t
{
    PhoneBookList,
    IncomingCallList,    
    OutgoingCallList,
    MissCallList,
    CombinedCallList,
    SpeedDialList,
    FavoriteNumberList,
};
/*---------------------------------------------------------------------------
 * PB_LOCAL_STORE_NAME constant
 *
 *     String constant for the local phonebook name.
 */
#define PB_LOCAL_STORE_NAME     "telecom/pb.vcf"

/*---------------------------------------------------------------------------
 * PB_LOCAL_OCH_NAME constant
 *
 *     String constant for the local phonebook outgoing call history.
 */
#define PB_LOCAL_OCH_NAME       "telecom/och.vcf"

/*---------------------------------------------------------------------------
 * PB_LOCAL_ICH_NAME constant
 *
 *     String constant for the local phonebook incoming call history.
 */
#define PB_LOCAL_ICH_NAME       "telecom/ich.vcf"

/*---------------------------------------------------------------------------
 * PB_LOCAL_MCH_NAME constant
 *
 *     String constant for the local phonebook missed call history.
 */
#define PB_LOCAL_MCH_NAME       "telecom/mch.vcf"

/*---------------------------------------------------------------------------
 * PB_LOCAL_CCH_NAME constant
 *
 *     String constant for the local phonebook combined call history.
 */
#define PB_LOCAL_CCH_NAME       "telecom/cch.vcf"

/*---------------------------------------------------------------------------
 * PB_LOCAL_SPD_NAME constant
 *
 *     String constant for the local phonebook speed dial.
 */
#define PB_LOCAL_SPD_NAME       "telecom/spd.vcf"

/*---------------------------------------------------------------------------
 * PB_LOCAL_FAV_NAME constant
 *
 *     String constant for the local phonebook favorite number .
 */
#define PB_LOCAL_FAV_NAME       "telecom/fav.vcf"

/*---------------------------------------------------------------------------
 * PB_SIM_STORE_NAME constant
 *
 *     String constant for the SIM phonebook name.
 */
#define PB_SIM_STORE_NAME       "SIM1/telecom/pb.vcf"

/*---------------------------------------------------------------------------
 * PB_SIM_OCH_NAME constant
 *
 *     String constant for the SIM phonebook outgoing call history.
 */
#define PB_SIM_OCH_NAME         "SIM1/telecom/och.vcf"

/*---------------------------------------------------------------------------
 * PB_SIM_ICH_NAME constant
 *
 *     String constant for the SIM phonebook incoming call history.
 */
#define PB_SIM_ICH_NAME         "SIM1/telecom/ich.vcf"

/*---------------------------------------------------------------------------
 * PB_SIM_MCH_NAME constant
 *
 *     String constant for the SIM phonebook missed call history.
 */
#define PB_SIM_MCH_NAME         "SIM1/telecom/mch.vcf"

/*---------------------------------------------------------------------------
 * PB_SIM_CCH_NAME constant
 *
 *     String constant for the SIM phonebook combined call history.
 */
#define PB_SIM_CCH_NAME         "SIM1/telecom/cch.vcf"

/*---------------------------------------------------------------------------
 * PB_SIM_CCH_NAME constant
 *
 *     String constant for the SIM phonebook combined call history.
 */
#define PB_SIM_SPD_NAME         "SIM1/telecom/spd.vcf"
/*---------------------------------------------------------------------------
 * PB_SIM_CCH_NAME constant
 *
 *     String constant for the SIM phonebook combined call history.
 */
#define PB_SIM_FAV_NAME         "SIM1/telecom/fav.vcf"

#define VCARD_FORMAT_21        0x00       /* Version 2.1 format */
#define VCARD_FORMAT_30        0x01       /* Version 3.0 format */

/*---------------------------------------------------------------------------
 * PbapVcardSearchAttribute type
 *
 *      Describes the 1-byte vCard search attribute value sent in the
 *      Application Parameters OBEX header from the Phonebook Access client
 *      to the server to dictate the type of search to be performed on
 *      the vCard entries on the Phonebook Access server.  This format is
 *      used for the Pull Vcard Listing operation.
 */
#define VCARD_SEARCH_ATTRIB_NAME        0x00        /* Search by Name */
#define VCARD_SEARCH_ATTRIB_NUMBER      0x01        /* Search by Number */
#define VCARD_SEARCH_ATTRIB_SOUND       0x02        /* Search by Sound */

/*---------------------------------------------------------------------------
 * PbapVcardSortOrder type
 * 
 *     Describes the 1-byte vCard sorting order value sent in the Application 
 *     Parameters OBEX header from the Phonebook Access client to the 
 *     server to dictate the ordering of the vCard entries returned in the 
 *     vCard listing. This format is used for the Pull Vcard Listing operation.
 */
#define VCARD_SORT_ORDER_INDEXED        0x00       /* Indexed sorting */
#define VCARD_SORT_ORDER_ALPHA          0x01       /* Alphabetical sorting */
#define VCARD_SORT_ORDER_PHONETICAL     0x02       /* Phonetical sorting */

typedef struct _PbapClientCallbackParms_new PbapClientCallbackParms_new;
typedef void (*pbaclient_cb_func_t)(PbapClientCallbackParms_new *Parms);
void bt_pbap_client_set_cb_func(pbaclient_cb_func_t func );
void pbap_set_client_state(uint8_t state);
void pbap_clientcontinue(void);
uint8_t *pbap_opName(uint8_t param);
uint8_t *pbap_abort_msg(uint8_t param);
void pba_connect(uint8_t *addr);
void pba_disconnect(void);
void PBA_PullPhonebook(const char *PbName, Pbap_Vcard_Filter *Filter, 
                       uint16_t MaxListCount, uint16_t ListOffset,uint8_t VcardVersion);
void pba_abort_client(void);
void PBA_PullVcardListing(const char *FolderName, uint8_t SearchType,
                          const char *SearchValue, uint8_t Order,
                          uint16_t MaxListCount, uint16_t ListOffset);
void PBA_PullVcardEntry(const char *ObjectName, Pbap_Vcard_Filter *Filter,uint8_t VcardVersion);

#endif

