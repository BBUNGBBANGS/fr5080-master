#ifndef _BT_ERROR_H
#define _BT_ERROR_H
#include <stdint.h>

typedef uint8_t BtErrorCode;

#define BEC_NO_ERROR             0x00 /* No error */ 
#define BEC_UNKNOWN_HCI_CMD      0x01 /* Unknown HCI Command */
#define BEC_NO_CONNECTION        0x02 /* No connection */
#define BEC_HARDWARE_FAILURE     0x03 /* Hardware Failure */
#define BEC_PAGE_TIMEOUT         0x04 /* Page timeout */
#define BEC_AUTHENTICATE_FAILURE 0x05 /* Authentication failure */
#define BEC_MISSING_KEY          0x06 /* Missing key */
#define BEC_MEMORY_FULL          0x07 /* Memory full */
#define BEC_CONNECTION_TIMEOUT   0x08 /* Connection timeout */
#define BEC_MAX_CONNECTIONS      0x09 /* Max number of connections */
#define BEC_MAX_SCO_CONNECTIONS  0x0a /* Max number of SCO connections to a device */
#define BEC_ACL_ALREADY_EXISTS   0x0b /* The ACL connection already exists. */
#define BEC_COMMAND_DISALLOWED   0x0c /* Command disallowed */
#define BEC_LIMITED_RESOURCE     0x0d /* Host rejected due to limited resources */
#define BEC_SECURITY_ERROR       0x0e /* Host rejected due to security reasons */
#define BEC_PERSONAL_DEVICE      0x0f /* Host rejected (remote is personal device) */
#define BEC_HOST_TIMEOUT         0x10 /* Host timeout */
#define BEC_UNSUPPORTED_FEATURE  0x11 /* Unsupported feature or parameter value */
#define BEC_INVALID_HCI_PARM     0x12 /* Invalid HCI command parameters */
#define BEC_USER_TERMINATED      0x13 /* Other end terminated (user) */
#define BEC_LOW_RESOURCES        0x14 /* Other end terminated (low resources) */
#define BEC_POWER_OFF            0x15 /* Other end terminated (about to power off) */
#define BEC_LOCAL_TERMINATED     0x16 /* Terminated by local host */
#define BEC_REPEATED_ATTEMPTS    0x17 /* Repeated attempts */
#define BEC_PAIRING_NOT_ALLOWED  0x18 /* Pairing not allowed */
#define BEC_UNKNOWN_LMP_PDU      0x19 /* Unknown LMP PDU */
#define BEC_UNSUPPORTED_REMOTE   0x1a /* Unsupported Remote Feature */
#define BEC_SCO_OFFSET_REJECT    0x1b /* SCO Offset Rejected */
#define BEC_SCO_INTERVAL_REJECT  0x1c /* SCO Interval Rejected */
#define BEC_SCO_AIR_MODE_REJECT  0x1d /* SCO Air Mode Rejected */
#define BEC_INVALID_LMP_PARM     0x1e /* Invalid LMP Parameters */
#define BEC_UNSPECIFIED_ERR      0x1f /* Unspecified Error */
#define BEC_UNSUPPORTED_LMP_PARM 0x20 /* Unsupported LMP Parameter Value */
#define BEC_ROLE_CHG_NOT_ALLOWED 0x21 /* Role Change Not Allowed */        
#define BEC_LMP_RESPONSE_TIMEOUT 0x22 /* LMP Response Timeout */           
#define BEC_LMP_TRANS_COLLISION  0x23 /* LMP Error Transaction Collision */
#define BEC_LMP_PDU_NOT_ALLOWED  0x24 /* LMP PDU Not Allowed */            
#define BEC_ENCRYP_MODE_NOT_ACC  0x25 /* Encryption Mode Not Acceptable */
#define BEC_UNIT_KEY_USED        0x26 /* Unit Key Used */
#define BEC_QOS_NOT_SUPPORTED    0x27 /* QoS is Not Supported */
#define BEC_INSTANT_PASSED       0x28 /* Instant Passed */
#define BEC_PAIR_UNITKEY_NO_SUPP 0x29 /* Pairing with Unit Key Not Supported */
#define BEC_NOT_FOUND            0xf1 /* Item not found */
#define BEC_REQUEST_CANCELLED    0xf2 /* Pending request cancelled */


/*---------------------------------------------------------------------------
 * BtStatus type
 *
 *     This type is returned from most stack APIs to indicate the success
 *     or failure of the operation. In many cases, BT_STATUS_PENDING
 *     is returned, meaning that a future callback will indicate the
 *     result of the operation.
 */
typedef int16_t BtStatus;

#define BT_STATUS_SUCCESS           0
#define BT_STATUS_FAILED            1
#define BT_STATUS_PENDING           2
#define BT_STATUS_BUSY              11
#define BT_STATUS_NO_RESOURCES      12
#define BT_STATUS_NOT_FOUND         13
#define BT_STATUS_DEVICE_NOT_FOUND  14
#define BT_STATUS_CONNECTION_FAILED 15
#define BT_STATUS_TIMEOUT           16
#define BT_STATUS_NO_CONNECTION     17
#define BT_STATUS_INVALID_PARM      18
#define BT_STATUS_IN_PROGRESS       19
#define BT_STATUS_RESTRICTED        20
#define BT_STATUS_INVALID_TYPE      21
#define BT_STATUS_HCI_INIT_ERR      22
#define BT_STATUS_NOT_SUPPORTED     23
#define BT_STATUS_IN_USE            5
#define BT_STATUS_SDP_CONT_STATE    24
#define BT_STATUS_CANCELLED         25

/* The last defined status code */
#define BT_STATUS_LAST_CODE         25
/* End of BtStatus */

#endif
