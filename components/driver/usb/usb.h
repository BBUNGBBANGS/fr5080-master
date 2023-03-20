#ifndef _USB_COMMON_H
#define _USB_COMMON_H

#include <stdint.h>

// usb 1.1 specification defines
//
// build from  Universal Serial Bus Specification Revision 1.1
//

#define USB_DEVICE_DESCRIPTOR_TYPE              0x01
#define USB_CONFIGURATION_DESCRIPTOR_TYPE       0x02
#define USB_STRING_DESCRIPTOR_TYPE              0x03
#define USB_INTERFACE_DESCRIPTOR_TYPE           0x04
#define USB_ENDPOINT_DESCRIPTOR_TYPE            0x05
#define USB_POWER_DESCRIPTOR_TYPE               0x06

/*Class Specific Descriptor Types*/
#define CS_UNDEFINED 0x20
#define CS_DEVICE 0x21
#define CS_CONFIGURATION 0x22
#define CS_STRING 0x23
#define CS_INTERFACE 0x24
#define CS_ENDPOINT 0x25


//
// standard request code
//
#define USB_REQUEST_GET_STATUS                  0x00
#define USB_REQUEST_CLEAR_FEATURE               0x01
//Reserved for future use 2
#define USB_REQUEST_SET_FEATURE                 0x03
//Reserved for future use 4
#define USB_REQUEST_SET_ADDRESS                 0x05
#define USB_REQUEST_GET_DESCRIPTOR              0x06
#define USB_REQUEST_SET_DESCRIPTOR              0x07
#define USB_REQUEST_GET_CONFIGURATION           0x08
#define USB_REQUEST_SET_CONFIGURATION           0x09
#define USB_REQUEST_GET_INTERFACE               0x0a
#define USB_REQUEST_SET_INTERFACE               0x0b
#define USB_REQUEST_SYNCH_FRAME                 0x0c

// bulk-only mass storage request code
#define USB_REQUEST_GET_MAXLUN                  0xfe
#define USB_REQUEST_BULK_ONLY_RESET             0xff

//
// defined USB device classes
//
#define USB_HOST                                0x00
#define USB_DEVICE_CLASS_RESERVED               0x00
#define USB_DEVICE_CLASS_AUDIO                  0x01
#define USB_DEVICE_CLASS_COMMUNICATIONS         0x02
#define USB_DEVICE_CLASS_HUMAN_INTERFACE        0x03
#define USB_DEVICE_CLASS_MONITOR                0x04
#define USB_DEVICE_CLASS_PHYSICAL_INTERFACE     0x05
#define USB_DEVICE_CLASS_POWER                  0x06
#define USB_DEVICE_CLASS_PRINTER                0x07
#define USB_DEVICE_CLASS_STORAGE                0x08
#define USB_DEVICE_CLASS_HUB                    0x09
#define USB_DEVICE_CLASS_VENDOR_SPECIFIC        0xff

// bmRequestType
#define USB_RECIPIENT                           0x1f
#define USB_RECIPIENT_DEVICE                    0x00
#define USB_RECIPIENT_INTERFACE                 0x01
#define USB_RECIPIENT_ENDPOINT                  0x02

/* Endpoint Direction */

#define USB_ENDPOINT_IN                             0x80
#define USB_ENDPOINT_OUT                            0x0

#define USB_ENDPOINT_DIRECTION_MASK                 0x80


/* Endpoint Attributes */

#define USB_CONTROL_TRANSFER_TYPE                   0x0
#define USB_ISOC_TRANSFER_TYPE                      0x1
#define USB_BULK_TRANSFER_TYPE                      0x2
#define USB_INTERRUPT_TRANSFER_TYPE                 0x3

#define USB_ENDPOINT_ATTRIBUTES_MASK                0x3

#define USB_ENDPOINT_NUMBER_MASK					0xF /* endpoints 0-15 */



#define USB_DT_DEVICE        0x01
#define USB_CLASS_PER_INTERFACE  0	/* for DeviceClass */
#define USB_CLASS_CDC  2	/* for DeviceClass */



/* USB Common Descriptor Struct */

typedef __packed struct _COMMON_DESCRIPTOR
{
    uint8_t       bLength;
    uint8_t       bDescriptorType;
} COMMON_DESCRIPTOR;

typedef __packed struct _DeviceDescriptor
{

    uint8_t               bLength;                // Size of this descriptor in bytes
    uint8_t               bDescriptorType;        // DEVICE descriptor type
    uint8_t               bcdUSB[2];              // USB Spec Release Number
    uint8_t               bDeviceClass;           // Class is specified in the UINT32erface descriptor
    uint8_t               bDeviceSubClass;        // Subclass is specified in the UINT32erface descriptor
    uint8_t               bDeviceProtocol;        // Protocol is specified in the UINT32erface descriptor
    uint8_t               bMaxPacketSize0;        // Maximum packet size for endpoUINT32 zero. (only 8,16,
    // 32, or 64 are valid(08h,10h,20h,40h)).
    uint8_t               idVendor[2];            // Vendor ID (assigned by the USB-IF).//GET THIS
    uint8_t               idProduct[2];           // Product ID (assigned by the manufacturer)//GET THIS
    uint8_t               bcdDevice[2];           // Device release number in binary-coded decimal.
    uint8_t               iManufacturer;          // Index of string descriptor describing the manufacturer.
    uint8_t               iProduct;               // Index of string descriptor describing this product.
    uint8_t               iSerialNumber;          // Index of string descriptor describing the device's SN//GET THIS
    uint8_t               bNumConfigurations;     // Number of possible configurations.

} DEVICE_DESCRIPTOR;


typedef __packed struct _ConfigurationDescriptor
{

    uint8_t               bLength;                // Size of this descritpor in bytes.
    uint8_t               bDescriptorType;        // CONFIGURATION Descriptor Type.
    uint8_t               wTotalLength[2];        // Total length of data returned for this configuration.
    uint8_t               bNumInterfaces;         // Number of UINT32erfaces supported by this configuration.
    uint8_t               bConfigurationValue;    // Value to use as an argument to the SetConfiguration()
    // request.
    uint8_t               iConfiguration;         // Index of string descriptor describing this configuration.
    uint8_t               bmAttributes;           // Configuration characteristics
    uint8_t               MaxPower;               // Maximum power consumption of the USB device from the bus.

} CONFIG_DESCRIPTOR;


typedef __packed struct _InterfaceDescritpor
{

    uint8_t               bLength;                // Size of this descriptor in bytes
    uint8_t               bDescriptorType;        // INTERFACE Descriptor Type.
    uint8_t               bInterfaceNumber;       // Number of UINT32erface. Zero-based value identifying the
    // index in the array of concurrent UINT32erfaces supported
    // this configuration.
    uint8_t               bAlternateSetting;      // Value used to select alternate setting for the UINT32erface
    // identified in the prior field.
    uint8_t               bNumEndpoints;          // Number of endpoUINT32s used by this UINT32erface (excluding
    // endpoUINT32 zero). This value shall be at lease 2
    uint8_t               bInterfaceClass;        // MASS STORAGE Class
    uint8_t               bInterfaceSubClass;     // Subclass code (assinged by the USB-IF)
    uint8_t               bInterfaceProtocol;     // BULK-ONLY TRANSPORT
    uint8_t               iInterface;             // Index to string descriptor describing this UINT32erface.

} INTERFACE_DESCRIPTOR;


typedef __packed struct _EndpointDescriptor
{

    uint8_t               bLength;                // Size of this descriptor in bytes.
    uint8_t               bDescriptorType;        // ENDPOINT Descritpor Type.
    uint8_t               bEndpointAddress;       // The address of this endpoUINT32 on the USB device.
    // bit[3..0]    The endpoUINT32 number
    // bit[6..4]    Reserved, set to 0
    // bit[7]       1 = In,0 = Out
    uint8_t               bmAttributes;           // This is a Bulk endpoUINT32
    uint8_t               wMaxPacketSize[2];      // Maximum packet size. Shall be 8,16,32 or 64 bytes
    // (08h,10h,20h,40h)
    uint8_t               bInterval;              // Does not apply to Bulk endpoUINT32.

} ENDPOINT_DESCRIPTOR;


// configuration descritpor, include UINT32erface and endpoUINT32s
typedef __packed struct _ConfigDescriptor_A
{

    CONFIG_DESCRIPTOR       ConfigDescr;
    INTERFACE_DESCRIPTOR    InterfaceDescr;
    ENDPOINT_DESCRIPTOR     EPInDescr;
    ENDPOINT_DESCRIPTOR     EPOutDescr;

} CONFIG_DESCRIPTOR_A;


// USB Device Requests
typedef __packed struct _USBRequestPacket
{

    uint8_t               bmRequestType;          // Characteristics of request
    // D7: Data transfer direction
    //   0 = Host-to-device 1 = Device-to-host
    // D6...5: Type
    //   0 = Standard 1 = Class 2 = Vendor 3 = Reserved
    // D4...0: Recipient
    //   0 = Device 1 = Interface 2 = Endpoint 3 = Other
    //   4...31 = Reserved
    uint8_t               bRequest;               // Specific request
    uint8_t               wValue[2];              // Word-sized field that varies according to request
    uint8_t               wIndex[2];              // Word-sized field that varies according to request;
    // typically used to pass an index or offset
    uint8_t               wLength[2];             // Number of bytes to transfer if there is a Data stage

} USBREQPACKET;


// USB Device Requests
typedef __packed struct _USBDevState
{

    USBREQPACKET        USBReqPkt;
    uint8_t               bUSBFuncAddr;
    uint8_t               bDevConfig;
    uint8_t               bConfigState;
    uint8_t               bAlternateSetting;

} USBDEVSTATE;


#if 0
// Bulk-Only SCSI Command Block Wrapper
typedef __packed struct _CommandBlockWrapper
{

    uint8_t               dCBWSignature[4];       // 43425355h (little endian)

    uint8_t               dCBWTag[4];             // A Command Block Tag sent by the host. The device
    // shall echo the contents of this field back to the
    // host in the dCSWTag field of the associated CSW.

    uint8_t               dCBWDataTransferLength[4]; // The number of bytes of data that the host expects
    // to transfer on the Bulk-In or Bulk-Out endpoUINT32 (as
    // indicated by the Direction bit) during the execution
    // of this command.

    uint8_t               bmCBWFlags;             // Bit 7    Direction - the device shall ignore this bit
    //          if the dCBWDataTransferLength field is zero,
    //          otherwise: 0 = Data-Out from host to the device,
    //          1 = Data-In from the device to the host.
    // Bit 6    Obsolete. The host shall set this bit to zero.
    // Bits 5..0 Reserved - the host shall set these bits to zero.

    uint8_t               bCBWLUN;                // The device Logical Unit Number (LUN) to which the command
    // block is being sent.

    uint8_t               bCBWCBLength;           // The valid length of the CBWCB in bytes.

    uint8_t               CBWCB[16];              // The command block to be executed by the device.

} CBW;

// Bulk-Only SCSI Command Status Wrapper
typedef __packed struct _CommandStatusWrapper
{

    uint8_t               dCSWSignature[4];       // 53425355h (little endian)

    uint8_t               dCSWTag[4];             // The device shall set this field to the value received
    // in the dCBWTag of the associated CBW.

    uint8_t               dCSWDataResidue[4];     // For Data-Out the device shall report in the
    // dCSWDataResidue the difference between the amount of data
    // expected as stated in the dCBWDataTransferLength, and the
    // actual amount of data processed by the device.
    // For Data-In the device shall report in the
    // dCSWDataResidue the difference between the amount of data
    // expected as stated in the dCBWDataTransferLength and the
    // actual amount of relevant data sent by the device.

    uint8_t               bCSWStatus;             // bCSWStatus indicates the success or failure of the command.
    // 00h              Command Passed ("good status")
    // 01h              Command Failed
    // 02h              Phase Error
    // 03h and 04h      Reserved (Obsolete)
    // 05h to FFh       Reserved

} CSW;
#endif

typedef __packed struct _STD_INQUIRYDATA
{

    uint8_t               DeviceType : 5;
    uint8_t               Reserved0 : 3;

    uint8_t               Reserved1 : 7;
    uint8_t               RemovableMedia : 1;

    uint8_t               Reserved2;

    uint8_t               Reserved3 : 5;
    uint8_t               NormACA : 1;
    uint8_t               Obsolete0 : 1;
    uint8_t               AERC : 1;

    uint8_t               Reserved4[3];

    uint8_t               SoftReset : 1;
    uint8_t               CommandQueue : 1;
    uint8_t               Reserved5 : 1;
    uint8_t               LinkedCommands : 1;
    uint8_t               Synchronous : 1;
    uint8_t               Wide16Bit : 1;
    uint8_t               Wide32Bit : 1;
    uint8_t               RelativeAddressing : 1;

    uint8_t               VendorId[8];

    uint8_t               ProductId[16];

    uint8_t               ProductRevisionLevel[4];

    /*
    //  Above is 36 bytes
    //  can be tranmitted by Bulk
    */

    uint8_t               VendorSpecific[20];
    uint8_t               InfoUnitSupport : 1;
    uint8_t               QuickArbitSupport : 1;
    uint8_t               Clocking : 2;
    uint8_t               Reserved6 : 4;

    uint8_t               Reserved7 ;
    uint8_t               VersionDescriptor[16] ;

    uint8_t               Reserved8[22];

} STD_INQUIRYDATA;


// USB Device Requests
typedef __packed struct _USB_DEVICEREQUEST
{

    uint8_t   bmRequestType;      // Characteristics of request
    // D7: Data transfer direction
    //   0 = Host-to-device 1 = Device-to-host
    // D6...5: Type
    //   0 = Standard 1 = Class 2 = Vendor 3 = Reserved
    // D4...0: Recipient
    //   0 = Device 1 = Interface 2 = Endpoint 3 = Other
    //   4...31 = Reserved
    uint8_t   bRequest;           // Specific request
    uint8_t   wValue[2];          // Word-sized field that varies according to request
    uint8_t   wIndex[2];          // Word-sized field that varies according to request;
    // typically used to pass an index or offset
    uint8_t   wLength[2];         // Number of bytes to transfer if there is a Data stage

} USB_DEVICEREQUEST;

typedef struct _USB_DEVICE
{
    uint8_t                   sie;
    uint8_t                   port;
    uint8_t                   address;
    uint8_t                   hub_port;
    uint8_t                   speed;
    uint8_t                   EP0_max_pkt;
    uint8_t                   enum_state;
    struct CLASS_DRIVER     *driver;
    DEVICE_DESCRIPTOR   dev_descr;
    uint8_t					cfg_descr[64];
    uint8_t                   *setup_packet_buffer;
    INTERFACE_DESCRIPTOR *inf_descr;
    uint8_t					idleCheck;
} USB_DEVICE;

typedef struct CLASS_DRIVER
{
    uint8_t   class;
    uint8_t   subclass;
    uint8_t   protocol;
    uint16_t  vendor_ID;
    uint16_t  product_ID;

    void (*init)(void);               /* Callback function that is called
                                       * when the driver is first initialized.
                                       */
    uint16_t (*start)(USB_DEVICE *dev); /* Callback function that is called
                                       * when the device is first enumerated.
                                       */
    uint16_t (*stop)(USB_DEVICE *dev);  /* Called when the device is removed. */
    void (*run)(USB_DEVICE *dev);     /* Called to allow driver to perform
                                       * idle time processing.
                                       */
    uint16_t (*ioctl)(USB_DEVICE *, uint16_t, uint16_t, uint16_t );
} CLASS_DRIVER;


enum USB_EP0_STAT
{
    EP0_IDLE_STAT,
    EP0_TX_STAT,
    EP0_RX_STAT
};

//Setup Command machine state
enum USB_SETUP_STAT
{
    SETUP_DEC_CMMD_STAT,
    SETUP_REQUIRE_EXTRA_DATA_STAT,
    SETUP_GOT_EXTRA_DATA_STAT,
    SETUP_SEND_RESULT_STAT,
    SETUP_FINISHING
};

enum USB_REQ_TYPE
{
    REQ_STANDARD,
    REQ_CLASS,
    REQ_VENDOR
};

#define USBDEV_REQ_DIR(x)			((x&0x8)>>7)
#define USBDEV_REQ_TYPE(x)			((x&0x60)>>5)
#define USBDEV_REQ_RECIPIENT(x)	(x&0x1f)


#define USB_INTRUSB_SUSPEND		(1<<0)
#define USB_INTRUSB_RESUME		(1<<1)
#define USB_INTRUSB_RESET			(1<<2)
#define USB_INTRUSB_BABBLE			(1<<2)
#define USB_INTRUSB_SOF			(1<<3)
#define USB_INTRUSB_CONN			(1<<4)
#define USB_INTRUSB_DISCON			(1<<5)
#define USB_INTRUSB_SESSREQ		(1<<6)
#define USB_INTRUSB_VBUSERRO		(1<<7)

#define USB_INTR_EP0	(1<<0)
#define USB_INTR_EP1	(1<<1)
#define USB_INTR_EP2	(1<<2)
#define USB_INTR_EP3	(1<<3)
#define USB_INTR_EP4	(1<<4)
#define USB_INTR_EP5	(1<<5)
#define USB_INTR_EP6	(1<<6)
#define USB_INTR_EP7	(1<<7)

#define USBD_CSR0_RXPKTRDY			(1<<0)
#define USBD_CSR0_TXPKTRDY			(1<<1)
#define USBD_CSR0_SENTSTALL		(1<<2)
#define USBD_CSR0_DATAEND			(1<<3)
#define USBD_CSR0_SETUPEND		(1<<4)
#define USBD_CSR0_SENDSTALL		(1<<5)
#define USBD_CSR0_SVCRXPKTRDY		(1<<6)
#define USBD_CSR0_SVCSETUPEND		(1<<7)

#define USBD_RXCSR1_RXPKTRDY		(1<<0)
#define USBD_RXCSR1_FIFOFULL		(1<<1)
#define USBD_RXCSR1_OVERRUN		(1<<2)
#define USBD_RXCSR1_DATAERRO		(1<<3)
#define USBD_RXCSR1_FLUSHFIFO		(1<<4)
#define USBD_RXCSR1_SENDSTALL		(1<<5)
#define USBD_RXCSR1_SENTSTALL		(1<<6)
#define USBD_RXCSR1_CLRDATATOG	(1<<7)

#define USBD_TXCSR1_TXPKTRDY		(1<<0)
#define USBD_TXCSR1_FIFONOTEMPTY	(1<<1)
#define USBD_TXCSR1_UNDERRUN		(1<<2)
#define USBD_TXCSR1_FLUSHFIFO		(1<<3)
#define USBD_TXCSR1_SENDSTALL		(1<<4)
#define USBD_TXCSR1_SENTSTALL		(1<<5)
#define USBD_TXCSR1_CLRDATATOG	(1<<6)

#define USBD_TXCSR2_DMAMODE		(1<<2)
#define USBD_TXCSR2_FRCDATATOG	(1<<3)
#define USBD_TXCSR2_DMAENAB		(1<<4)
#define USBD_TXCSR2_MODE			(1<<5)
#define USBD_TXCSR2_ISO			(1<<6)
#define USBD_TXCSR2_AUTOSET		(1<<7)

/* STAT_TX[1:0] STATus for TX transfer */
#define EP_TX_DIS      (0x0000) /* EndPoint TX DISabled */
#define EP_TX_STALL    (0x0010) /* EndPoint TX STALLed */
#define EP_TX_NAK      (0x0020) /* EndPoint TX NAKed */
#define EP_TX_VALID    (0x0030) /* EndPoint TX VALID */
#define EPTX_DTOG1     (0x0010) /* EndPoint TX Data TOGgle bit1 */
#define EPTX_DTOG2     (0x0020) /* EndPoint TX Data TOGgle bit2 */
#define EPTX_DTOGMASK  (EPTX_STAT|EPREG_MASK)

/* STAT_RX[1:0] STATus for RX transfer */
#define EP_RX_DIS      (0x0000) /* EndPoint RX DISabled */
#define EP_RX_STALL    (0x1000) /* EndPoint RX STALLed */
#define EP_RX_NAK      (0x2000) /* EndPoint RX NAKed */
#define EP_RX_VALID    (0x3000) /* EndPoint RX VALID */
#define EPRX_DTOG1     (0x1000) /* EndPoint RX Data TOGgle bit1 */
#define EPRX_DTOG2     (0x2000) /* EndPoint RX Data TOGgle bit1 */
#define EPRX_DTOGMASK  (EPRX_STAT|EPREG_MASK)

void usb_read_fifo(uint8_t *p_des, uint32_t size, uint32_t fifo_addr);
void usb_read_fifo32(uint32_t *p_des, uint32_t size, uint32_t fifo_addr);
void usb_write_fifo(uint8_t *p_src, uint32_t size, uint32_t fifo_addr);

uint32_t USB_SIL_Write(uint8_t bEpAddr, uint8_t* pBufferPointer, uint32_t wBufferSize);

#endif // _USB_COMMON_H

