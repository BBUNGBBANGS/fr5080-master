#ifndef _USER_HID_H
#define _USER_HID_H

#if HID_ENABLE == 1
#include "bt_hid_api.h"

/****************************************************************************
 * HID SDP Entries
 ****************************************************************************/

/*---------------------------------------------------------------------------
 * SDP ServiceClassIDList
 */
static const uint8_t HidSrcClassId0[] = {
    SDP_ATTRIB_HEADER_8BIT(3),                  /* Data Element Sequence, 3 bytes */
    SDP_UUID_16BIT(SC_HUMAN_INTERFACE_DEVICE),  /* Human Interface Device */
};

/*---------------------------------------------------------------------------
 * SDP Protocol Descriptor List object registered by HID.
 * 
 * Value of the protocol descriptor list for the HID Profile.
 * This structure is a ROM'able representation of the RAM structure.
 * This structure is copied into a RAM structure used to register the 
 * service.
 */
static const uint8_t HidProtoDescList0[] = {
    SDP_ATTRIB_HEADER_8BIT(13),  /* Data element sequence, 13 bytes */

    /* Each element of the list is a Protocol descriptor which is a
     * data element sequence. The first element is L2CAP which only
     * has a UUID element.
     */
    SDP_ATTRIB_HEADER_8BIT(6),    /* Data element sequence for L2CAP, 6 bytes */     
    SDP_UUID_16BIT(PROT_L2CAP),      /* L2CAP UUID */
    SDP_UINT_16BIT(0x0011), /* HID PSM */

    /* Next protocol descriptor in the list is HID.
     */
    SDP_ATTRIB_HEADER_8BIT(3),   /* Data element sequence for AVDTP, 2 bytes */
    SDP_UUID_16BIT(PROT_HIDP)    /* HID UUID */
};

/*---------------------------------------------------------------------------
 * SDP AdditionalProtocol Descriptor List object registered by HID.
 * 
 * Value of the additional protocol descriptor list for the HID.
 * This structure is a ROM'able representation of the RAM structure.
 * During HCRP_ServerRegisterSDP, this structure is copied into the RAM 
 * structure and used to register the client or server.
 */
static const uint8_t HcrpAddProtoDescList0[] = {
    SDP_ATTRIB_HEADER_8BIT(15),  /* Data element sequence, 15 bytes  */

    /* Each element of this list is a protocol descriptor list.  For
     * HID, there is only one list.
     */

    SDP_ATTRIB_HEADER_8BIT(13),  /* Data element sequence, 13 bytes */

    /* Each element of the list is a Protocol descriptor which is a
     * data element sequence. The first element is L2CAP which has
     * a UUID element and a PSM.  The second element is HID.
     */
    SDP_ATTRIB_HEADER_8BIT(6),   /* Data element sequence for L2CAP */     
    SDP_UUID_16BIT(PROT_L2CAP),  /* Uuid16 L2CAP                    */
    SDP_UINT_16BIT(0x0013),  /* L2CAP PSM (varies)         */

    /* The next protocol descriptor is for HCRP. It contains one element
     * which is the UUID.
     */
    SDP_ATTRIB_HEADER_8BIT(3),   /* Data element sequence for HCRP Control */
    SDP_UUID_16BIT(PROT_HIDP)    /* Uuid16 HCRP Channel (varies)           */
};

/*---------------------------------------------------------------------------
 * SDP Language Base Attribute ID List.  
 *
 * Only defines the English language.  This must be modified if other language
 * support is required.
 */
static const uint8_t HidLangBaseAttrIdList0[] = {
    SDP_ATTRIB_HEADER_8BIT(9),

    /* Elements of this list occur in triplets that describe the Language
     * Base Attribute ID.
     */
    SDP_UINT_16BIT(0x656e),      /* English "en" */
    SDP_UINT_16BIT(0x006a),      /* UTF-8 endoding */
    SDP_UINT_16BIT(0x0100)       /* Primary Language Base ID */

    /* Additional languages may be defined.  Each language is defined by a
     * "triplet" of 16-bit elements.
     * 
     * The first element of the triplet defines the language.  The language is
     * encoded according to ISO 639:1988 (E/F):  "Code for the representation
     * of names of languages."  
     *
     * The second element describes the character encoding.  Values for
     * character encoding can be found in IANA's database2, and have the values
     * that are referred to as MIBEnum values. The recommended character
     * encoding is UTF-8.
     *
     * The third element of each triplet contains an attribute ID that serves as
     * the base attribute ID for the natural language in the service record.
     *
     * The length of this attribute must be modified to include the length
     * of any additional triplets.
     */
};

/*---------------------------------------------------------------------------
 * HID Language ID Base List.
 *
 * Defines how Bluetooth strings are mapped to HID LANGID and string 
 * indices.  For a complete description, see section 7.11.7 of the
 * Bluetooth HID Profile specification.  The default value is a sample
 * that supports only United States English.
 */
static const uint8_t HidLangIdBaseList0[] = {
    SDP_ATTRIB_HEADER_8BIT(8),   /* Data element sequence */

    /* Each element of the list is a data element sequence describing a HID
     * language ID base.  Only one element is included.
     */
    SDP_ATTRIB_HEADER_8BIT(6),   /* Data element sequence. */
    SDP_UINT_16BIT(0x0409),      /* Language = English (United States) */
    SDP_UINT_16BIT(0x0100)       /* Bluetooth String Offset */

    /* Additional languages may be defined.  Each language is defined by
     * a data element sequence with 2 elements.
     *
     * The first element defines the language and is encoded according to the
     * the "Universal Serial Bus Language Identifiers (LANGIDs)" specification.
     *
     * The second element defines the Bluetooth base attribute ID as is defined
     * in the Language Base Attribute ID List (see HidLangBaseAttrIdList above).
     *
     * Headers must be added along with the actual element (HID Language ID
     * base) data.  Also,  the length in the first header must be adjusted.
     */
};

/*---------------------------------------------------------------------------
 * SDP Public Browse Group.
 */
static const uint8_t HidBrowseGroup0[] = {
    SDP_ATTRIB_HEADER_8BIT(3),               /* 3 bytes */
    SDP_UUID_16BIT(SC_PUBLIC_BROWSE_GROUP)   /* Public Browse Group */
};

/*---------------------------------------------------------------------------
 * SDP Profile Descriptor List
 */
static const uint8_t HidProfileDescList0[] = {
    SDP_ATTRIB_HEADER_8BIT(8),   /* Data element sequence, 8 bytes */
    SDP_ATTRIB_HEADER_8BIT(6),   /* Data element sequence for ProfileDescriptor,
                                  * 6 bytes.
                                  */
    SDP_UUID_16BIT(SC_HUMAN_INTERFACE_DEVICE),
    SDP_UINT_16BIT(0x0100)
};

/*---------------------------------------------------------------------------
 * Device Release Number.
 */
static const uint8_t HidDeviceRelease0[] = {
    SDP_UINT_16BIT(HID_DEVICE_RELEASE)
};

/*---------------------------------------------------------------------------
 * Parser Version.
 */
static const uint8_t HidParserVersion0[] = {
    SDP_UINT_16BIT(HID_PARSER_VERSION)
};

/*---------------------------------------------------------------------------
 * Device Subclass.
 */
static const uint8_t HidDeviceSubclass0[] = {
    SDP_UINT_8BIT(HID_DEVICE_SUBCLASS)
};

/*---------------------------------------------------------------------------
 * Country Code.
 */
static const uint8_t HidCountrycode0[] = {
    SDP_UINT_8BIT(HID_COUNTRY_CODE)
};

/*---------------------------------------------------------------------------
 * Virtual Cable.
 */
static const uint8_t HidVirtualCable0[] = {
    SDP_BOOL(HID_VIRTUAL_CABLE)
};

/*---------------------------------------------------------------------------
 * Initiate Reconnect.
 */
static const uint8_t HidReconnectInitiate0[] = {
    SDP_BOOL(HID_RECONNECT_INITIATE)
};

/*---------------------------------------------------------------------------
 * Descriptor List.
 */
uint8_t HidDescriptorList_app0[] = {
    SDP_ATTRIB_HEADER_8BIT(HID_DESCRIPTOR_LEN + 6), /* Data element sequence */

    /* Each element of the list is a HID descriptor which is a
     * data element sequence.
     */
    SDP_ATTRIB_HEADER_8BIT(HID_DESCRIPTOR_LEN + 4),
    SDP_UINT_8BIT(HID_DESCRIPTOR_TYPE),  /* Report Descriptor Type */

    /* Length of the HID descriptor */
    SDP_TEXT_8BIT(HID_DESCRIPTOR_LEN),

    /* The actual descriptor is defined in hid.h or in overide.h */
    HID_DESCRIPTOR

    /* Addition descriptors may be added, but the header must be added
     * along with the actual descriptor data.  Also, all lengths in the 
     * headers above must be adjusted.
     */
};

/*---------------------------------------------------------------------------
 * Profile Version
 */
static const uint8_t HidProfileVersion0[] = {
    SDP_UINT_16BIT(0x0100)
};

/*---------------------------------------------------------------------------
 * Boot Device
 */
static const uint8_t HidBootDevice0[] = {
    SDP_BOOL(HID_BOOT_DEVICE)
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  ServiceName
 *
 * This is the English string.  Other languguages can be defined.
 */
static const uint8_t HidServiceName0[] = {
    SDP_TEXT_8BIT(9),          /* Null terminated text string */
    'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd', '\0'
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  ServiceDescription
 *
 * This is the English string.  Other languguages can be defined.
 */
static const uint8_t HidServiceDescription0[] = {
    SDP_TEXT_8BIT(9),          /* Null terminated text string */
    'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd', '\0'
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  ProviderName
 *
 * This is the English string.  Other languguages can be defined.
 */
static const uint8_t HidProviderName0[] = {
    SDP_TEXT_8BIT(9),          /* Null terminated text string */
    'F', 'r', 'e', 'q', 'c', 'h', 'i', 'p', '\0'
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  HidSdpDisable
 */
static const uint8_t HidSdpDisable0[] = {
    SDP_BOOL(0)
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  HidBatteryPower
 */
static const uint8_t HidBatteryPower0[] = {
    SDP_BOOL(HID_BATTERY_POWER)
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  HidRemoteWake
 */
static const uint8_t HidRemoteWake0[] = {
    SDP_BOOL(HID_REMOTE_WAKE)
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  HidSupervisionTimeout
 */
static const uint8_t HidSupervisionTimeout0[] = {
    SDP_UINT_16BIT(HID_SUPERVISION_TIMEOUT)
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  HidNormallyConnectable
 */
static const uint8_t HidNormallyConnectable0[] = {
    SDP_BOOL(HID_NORMALLY_CONNECTABLE)
};

/*---------------------------------------------------------------------------
 * HID attributes.
 *
 * This is a ROM template for the RAM structure used to register the
 * HID SDP record.
 */
static const SdpAttribute HidSdpAttributes0[] = {
    
    /* Mandatory SDP Attributes */

    /* HID class ID List attribute */
    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, HidSrcClassId0), 
    /* HID protocol descriptor list attribute */
    SDP_ATTRIBUTE(AID_PROTOCOL_DESC_LIST, HidProtoDescList0),
    /* Public Browse Group Service */
    SDP_ATTRIBUTE(AID_BROWSE_GROUP_LIST, HidBrowseGroup0), 
    /* Language Base ID List */
    SDP_ATTRIBUTE(AID_LANG_BASE_ID_LIST, HidLangBaseAttrIdList0),
    /* HID profile descriptor list attribute */
    SDP_ATTRIBUTE(AID_BT_PROFILE_DESC_LIST, HidProfileDescList0),
    /* HID additional protocol descriptor list attribute */
    SDP_ATTRIBUTE(AID_ADDITIONAL_PROT_DESC_LISTS, HcrpAddProtoDescList0),

    /* Optional Human readable attributes.  The strings provided are English.
     * Other languages can be added.  Each language should have a different
     * Language Base Attribute ID (defined in HidLangBaseAttrIdList).  This ID
     * is added to the universal attribute ID for Service Name, Service
     * Description, and Provider Name.
     */

    /* HID Service Name in English */
    SDP_ATTRIBUTE((AID_SERVICE_NAME + 0x0100), HidServiceName0),
    /* HID Service Description in English */
    SDP_ATTRIBUTE((AID_SERVICE_DESCRIPTION + 0x0100), HidServiceDescription0),
    /* HID Provider Name in English*/
    SDP_ATTRIBUTE((AID_PROVIDER_NAME + 0x0100), HidProviderName0),

    /* Mandatory HID attributes */

    /* Device release number */
    SDP_ATTRIBUTE(AID_HID_DEVICE_RELEASE, HidDeviceRelease0),
    /* HID parser version */
    SDP_ATTRIBUTE(AID_HID_PARSER_VERSION, HidParserVersion0),
    /* Device subclass */
    SDP_ATTRIBUTE(AID_HID_DEVICE_SUBCLASS, HidDeviceSubclass0),
    /* Country Code */
    SDP_ATTRIBUTE(AID_HID_COUNTRY_CODE, HidCountrycode0),
    /* Virtual Cable */
    SDP_ATTRIBUTE(AID_HID_VIRTUAL_CABLE, HidVirtualCable0),
    /* Device initiates reconnect */
    SDP_ATTRIBUTE(AID_HID_RECONNECT_INIT, HidReconnectInitiate0),
    /* HID descriptor list */
    SDP_ATTRIBUTE(AID_HID_DESCRIPTOR_LIST, HidDescriptorList_app0),
    /* Language ID Base List */
    SDP_ATTRIBUTE(AID_HID_LANG_ID_BASE_LIST, HidLangIdBaseList0),

    /* Optional HID attibutes */

    /* SDP Disable/Enable */
    SDP_ATTRIBUTE(AID_HID_SDP_DISABLE, HidSdpDisable0),
    /* Battery powered */
    SDP_ATTRIBUTE(AID_HID_BATTERY_POWER, HidBatteryPower0),
    /* Device support of remote wakeup */
    SDP_ATTRIBUTE(AID_HID_REMOTE_WAKE, HidRemoteWake0),

    /* Mandatory HID attribute */

    /* HID profile version*/
    SDP_ATTRIBUTE(AID_HID_PROFILE_VERSION, HidProfileVersion0),

    /* Optional HID attributes */

    /* Recommended supervision timeout */
    SDP_ATTRIBUTE(AID_HID_SUPERV_TIMEOUT, HidSupervisionTimeout0),
    /* Device connectability */
    SDP_ATTRIBUTE(AID_HID_NORM_CONNECTABLE, HidNormallyConnectable0),

    /* Mandatory HID attribute */

    /* Support for boot protocol */
    SDP_ATTRIBUTE(AID_HID_BOOT_DEVICE, HidBootDevice0),
};

/*---------------------------------------------------------------------------
 * ServiceClassIDList
 */
static const uint8_t DeviceIdSrcClassId0[] = {
    SDP_ATTRIB_HEADER_8BIT(3),          /* Data Element Sequence, 3 bytes */
    SDP_UUID_16BIT(SC_PNP_INFO),        /* Plug and Play Information */
};

/*---------------------------------------------------------------------------
 * Specification Id
 */
static const uint8_t DeviceIdSpecId0[] = {
    SDP_UINT_16BIT(HID_DEVID_SPEC_ID)
};

/*---------------------------------------------------------------------------
 * Vendor Id
 */
static const uint8_t DeviceIdVendorId0[] = {
    SDP_UINT_16BIT(HID_DEVID_VENDOR_ID)
};

/*---------------------------------------------------------------------------
 * Product Id
 */
static const uint8_t DeviceIdProductId0[] = {
    SDP_UINT_16BIT(HID_DEVID_PRODUCT_ID)
};

/*---------------------------------------------------------------------------
 * Version
 */
static const uint8_t DeviceIdVersion0[] = {
    SDP_UINT_16BIT(HID_DEVICE_RELEASE)
};

/*---------------------------------------------------------------------------
 * Primary Record
 */
static const uint8_t DeviceIdPrimaryRec0[] = {
    SDP_BOOL(TRUE)
};

/*---------------------------------------------------------------------------
 * Vendor ID Source
 */
static const uint8_t DeviceIdVendorIdSrc0[] = {
    SDP_UINT_16BIT(HID_DEVID_VENDOR_ID_SRC)
};

/*---------------------------------------------------------------------------
 * Device ID attributes.
 *
 * This is a ROM template for the RAM structure used to register the
 * Device ID SDP record.
 */
static const SdpAttribute DeviceIdSdpAttributes0[] = {
    
    /* Mandatory SDP Attributes */

    /* PNP class ID List attribute */
    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, DeviceIdSrcClassId0), 
    /* Device ID Specification ID attribute */
    SDP_ATTRIBUTE(AID_DEVID_SPEC_ID, DeviceIdSpecId0),
    /* Device ID Vendor ID attribute */
    SDP_ATTRIBUTE(AID_DEVID_VENDOR_ID, DeviceIdVendorId0), 
    /* Device ID Product ID */
    SDP_ATTRIBUTE(AID_DEVID_PRODUCT_ID, DeviceIdProductId0),
    /* Device ID Version */
    SDP_ATTRIBUTE(AID_DEVID_VERSION, DeviceIdVersion0),
    /* Device ID Primary Record */
    SDP_ATTRIBUTE(AID_DEVID_PRIMARY_RECORD, DeviceIdPrimaryRec0),
    /* Device ID Vendor ID Source attribute */
    SDP_ATTRIBUTE(AID_DEVID_VENDOR_ID_SRC, DeviceIdVendorIdSrc0), 
};

#endif
#endif

