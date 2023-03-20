#include <string.h>

#include "driver_uart.h"
#include "os_timer.h"
#include "otg_reg.h"
#include "usb.h"
#include "usbdev.h"
#include "usbmass.h"

#include "usb_bot.h"
#include "mass_mal.h"

#define USB_EP2_IN_IDLE         0
#define USB_EP2_IN_TX           1

#define USB_EP2_OUT_IDLE        0
#define USB_EP2_OUT_RX          1

#define USB_EP2_OUT_IDLE        0
#define USB_HS      0
#define USB_FS      1
#define USB_LS      2

#define _EP_IN      0x80
#define _EP_OUT     0x00
#define _EP01_OUT   0x01
#define _EP01_IN    0x81
#define _EP02_OUT   0x02
#define _EP02_IN    0x82

#define MAX_PACKETSIZE  64

extern uint8_t  usbdev_event_flag;
extern uint8_t setup_state;

static uint32_t ep2_rx_buf[512/4];

static DEVICE_DESCRIPTOR usbmass_device_descriptor =
{
    sizeof(DEVICE_DESCRIPTOR),
    USB_DT_DEVICE,
    {0x00, 0x02}, //USB Spec Release Number in BCD format
    USB_CLASS_PER_INTERFACE, // Class Code
    0,              // Subclass code
    0,              // Protocol code
    64,             // Max packet size or EP0
    {0x83, 0x04},              // Vendor ID
    {0x20, 0x57},               // idProduct
    {0x00, 0x02},           // bcdDevice
    0x01,           // iManufacturer;
    0x02,               // iProduct;
    0x03,           // iSerial Number
    0x01            // Num of Configurations
} ;

#define CONFIG_DESCRIPTOR_LEN   (sizeof(CONFIG_DESCRIPTOR) + \
                                    sizeof(INTERFACE_DESCRIPTOR) + \
                                    sizeof(ENDPOINT_DESCRIPTOR) * 2)

static __packed struct
{
    CONFIG_DESCRIPTOR    configuration_desc;
    INTERFACE_DESCRIPTOR interface_desc;
    ENDPOINT_DESCRIPTOR bulk_epout_desc;
    ENDPOINT_DESCRIPTOR bulk_epin_desc;
} usbmass_confDesc =
{
    {
        //configuration_desc
        sizeof(CONFIG_DESCRIPTOR),
        USB_CONFIGURATION_DESCRIPTOR_TYPE,
        {CONFIG_DESCRIPTOR_LEN, 0},
        1,  // Number of interfaces
        1,
        0,
        0xC0,//USB_CONFIG_ATT_ONE | USB_CONFIG_ATT_SELFPOWER,
        0x32
    },
    {
        //interface0_desc
        sizeof(INTERFACE_DESCRIPTOR),
        USB_INTERFACE_DESCRIPTOR_TYPE,
        0x00,
        0x00,
        0x02,   /* ep number */
        USB_DEVICE_CLASS_STORAGE,
        0x06/*USB_MSC_SUBCLASS_SCSI*/,
        0x50/*MASS_BULK_ONLY_PROTOCOL*/,
        0x4
    },
    {
        //bulk_epin_desc
        sizeof(ENDPOINT_DESCRIPTOR),
        USB_ENDPOINT_DESCRIPTOR_TYPE,
        _EP01_IN,
        USB_BULK_TRANSFER_TYPE,
        {MAX_PACKETSIZE, 0x00},
        0
    },
    {
        //bulk_epout_desc
        sizeof(ENDPOINT_DESCRIPTOR),
        USB_ENDPOINT_DESCRIPTOR_TYPE,
        _EP02_OUT,
        USB_BULK_TRANSFER_TYPE,
        {MAX_PACKETSIZE, 0x00},
        0
    }
};
    
os_timer_t usb_in_check_timer;
uint32_t usb_rx_cnt = 0;
uint32_t usb_first_in = false;
__attribute__((weak)) void usb_user_notify(enum usb_action_t flag)
{
}

#define USB_DATA_THRESHOLD 200
static void usb_in_check_timer_func(void *arg)
{
    //printf("time out\r\n");
    if(usb_rx_cnt == 0){
        usb_user_notify(USB_ACTION_OUT);//usb out
    }
    else{
        os_timer_start(&usb_in_check_timer,2000,false);
        if(usb_first_in == true){
            usb_first_in = false;
        }
        else{
            if(usb_rx_cnt > USB_DATA_THRESHOLD){
                usb_user_notify(USB_ACTION_DATA_TRANSFER); //usb data transfering
            }else{
                usb_user_notify(USB_ACTION_IDLE); //usb no data transfer
            }
        }
        usb_rx_cnt = 0;
    }
}

void usbmass_device_reset()
{
    REGB(OTG_FADDR) = 0;
    REGB(OTG_INDEX) = 0;
    REGB(OTG_CSR0) = 0xc0;
    REGB(OTG_CSR02) = 1;    //flush endp0 FIFO

    REGB(OTG_INDEX) = 1;
    REGB(OTG_TXMAXP) = MAX_PACKETSIZE / 8;
    REGB(OTG_TXCSR1) = 0x48;
    REGB(OTG_TXCSR2) = 0x20;

    REGB(OTG_TXFIFO1) = 8;
    REGB(OTG_TXFIFO2) = 0x60;

    REGB(OTG_INDEX) = 2;
    REGB(OTG_RXMAXP) = MAX_PACKETSIZE / 8;
    REGB(OTG_RXCSR1) = 0x90;

    REGB(OTG_RXFIFO2) = 0x60;   //64bytes
    REGB(OTG_RXFIFO1) = 0x10;   // FIFO address 0x10 * 8 = 0x80

    REGB(OTG_INTRTX1E) = 3;     //EP0 EP1
    REGB(OTG_INTRRX1E) = 0x04;  //EP2
    REGB(OTG_INTRUSBE) = 0x04;
}

int get_usbmass_dev_desc(USBREQPACKET *dreq, uint8_t **ptr, int *size)
{
    if(dreq->wLength[0] > sizeof(usbmass_device_descriptor))
    {
        *size = sizeof(usbmass_device_descriptor);
    }
    else
    {
        *size = dreq->wLength[0];
    }
    *ptr = (uint8_t *)&usbmass_device_descriptor;
    return 0;
}

int get_usbmass_config_desc(USBREQPACKET *dreq, uint8_t **ptr, int *size)
{
    uint32_t len;
    len = dreq->wLength[1] << 8;
    len |= dreq->wLength[0];

    if(len > sizeof(usbmass_confDesc))
    {
        *size = sizeof(usbmass_confDesc);
    }
    else
    {
        *size = len;
    }
    *ptr = (uint8_t *)&usbmass_confDesc;
    return 0;
}

int get_usbmass_string_desc(USBREQPACKET *dreq, uint8_t **ptr, int *size)
{
    static uint16_t str_product[] = {0x0320, 'f', 'r', 'e', 'q', 'c', 'h', 'i', 'p',' ','S','D',
                                     0x5361,0x8BFB,0x5361,0x5668,
                                    };
    static uint16_t str_lang[] = {0x0304, 0x0409};
    static uint16_t str_manufact[] = {0x0310, 'U', 'D', 'I', 'S', 'K', ' ', ' '};
    static uint16_t str_xxx[] = {0x031a, '0', '4', 'E', 'E', '8', 'F', '5', '5', '3', '0', '3', '1'};
    switch(dreq->wValue[0])
    {
        case 0:
            //Get String LangID
            if ( dreq->wLength[0] > sizeof(str_lang) )
                *size =  sizeof(str_lang);
            else
                *size = dreq->wLength[0];
            *ptr = (uint8_t *)str_lang;
            break;
        case 1:
            //Get String iManufature
            if ( dreq->wLength[0] > sizeof(str_manufact) )
                *size = sizeof(str_manufact);
            else
                *size = dreq->wLength[0];
            *ptr = (uint8_t *)str_manufact;
            break;
        case 2:
            //Get String iProduct
            if ( dreq->wLength[0] > sizeof(str_product) )
                *size = sizeof(str_product);
            else
                *size = dreq->wLength[0];
            *ptr = (uint8_t *)str_product;
            break;
        case 3:
            //Get driver info
            if ( dreq->wLength[0] > sizeof(str_xxx) )
                *size = sizeof(str_xxx);
            else
                *size = dreq->wLength[0];
            *ptr = (uint8_t *)str_xxx;
            break;
        default:
            //Get Non-Exist String!
            break;
    }
    return 0;
}

int get_usbmass_configuration(USBREQPACKET *dreq, uint8_t **ptr, int *size)
{
    *ptr = (uint8_t *)&usbmass_confDesc.configuration_desc.bConfigurationValue;
    *size = 1;
    return 0;
}

int set_usbmass_interface(USBREQPACKET *dreq)
{
    return 0;
}

static void get_usbmass_maxlun(USBREQPACKET *dreq)
{
    const static uint8_t maxlun = MAX_LUN;
    setup_state = SETUP_SEND_RESULT_STAT;
    usbdev_set_ep0_retval((uint8_t *)&maxlun, 1);
}

void usbmass_class_req_dec(USBREQPACKET *dreq)
{
    uint8_t request_type = dreq->bmRequestType;
    uint8_t request = dreq->bRequest;

    if(request_type == 0xa1)
    {
        if(request == 0xfe/*MSC_BOT_GET_MAX_LUN*/)
        {
            get_usbmass_maxlun(dreq);
        }
        else
        {
            setup_state = SETUP_SEND_RESULT_STAT;
        }
    }
    else
    {
        setup_state = SETUP_SEND_RESULT_STAT;
    }
    return;
}

static void usbmass_ep1_tx_handler()
{
    uint8_t state;
    REGB(OTG_INDEX) = 1;
    state = REGB(OTG_TXCSR1);
    if(state & USBD_TXCSR1_SENTSTALL)
    {
        REGB(OTG_TXCSR1) = state & (~USBD_TXCSR1_SENTSTALL);
        REGB(OTG_TXCSR1) = USBD_TXCSR1_CLRDATATOG | USBD_TXCSR1_FLUSHFIFO;
    }

    Mass_Storage_In();
}

static void usbmass_ep2_rx_handler()
{
    uint32_t size;

    REGB(OTG_INDEX) = 2;
    size = REGB(OTG_RXCOUNT1);
    usb_rx_cnt ++;

    if(size > 0)
    {
        usb_read_fifo((uint8_t *)ep2_rx_buf, size, OTG_EP2FIFO);

        REGB(OTG_RXCSR1) = REGB(OTG_RXCSR1) & (~USBD_RXCSR1_RXPKTRDY);

        Mass_Storage_Out((uint8_t *)ep2_rx_buf,size);
    }
}

static void usb_mass_handler(uint8_t val)
{
    //uint8_t byte;

    if (val & USB_INTRUSB_RESET)
    {
        usb_user_notify(USB_ACTION_IN);
        usb_rx_cnt = 0;
        usb_first_in = true;
        os_timer_start(&usb_in_check_timer,5000,false);
        usbdev_device_reset();
    }
    if(val & USB_INTRUSB_SUSPEND)
    {
    }
    if(val & USB_INTRUSB_RESUME)
    {
    }
}

void usbmass_var_init()
{
    usbdev_event_flag = 0;
    os_timer_init(&usb_in_check_timer,usb_in_check_timer_func,NULL);
}

#if USB_DEVICE_ENABLE
__attribute__((section("ram_code"))) void USB_IRQHandler (void)
{
    uint8_t IntrUSB;
    uint8_t IntrRX;
    uint8_t IntrTX;

    IntrUSB = REGB(OTG_INTRUSB);
    IntrTX  = REGB(OTG_INTRTX1);
    IntrRX = REGB(OTG_INTRRX1);

    if(IntrUSB != 0)
    {
        //uart_putc_noint_no_wait('I');
        //uart_putc_noint_no_wait('\r');
        //uart_putc_noint_no_wait('\n');
        //printf("val = %x\r\n",IntrUSB);
        usb_mass_handler(IntrUSB);
    }

    /* Check for endpoint 0 interrupt */
    if (IntrTX & USB_INTR_EP0)
    {
        //uart_putc_noint_no_wait('E');
        //uart_putc_noint_no_wait('P');
        //uart_putc_noint_no_wait('0');
        //uart_putc_noint_no_wait(',');
        usbdev_ep0_handler();
    }

    /* Check for bulk in endpoint 1 tx interrupt */
    if (IntrTX & 2)
    {
        //uart_putc_noint_no_wait('E');
        //uart_putc_noint_no_wait('P');
        //uart_putc_noint_no_wait('1');
        //uart_putc_noint_no_wait(',');
        usbmass_ep1_tx_handler();
    }

    /* Check for bulk out endpoint 2 rx interrupt */
    if (IntrRX & 4)
    {
        //gpiod_toggle(2);
        //printf("EP2,");
        //uart_putc_noint_no_wait('E');
        //uart_putc_noint_no_wait('P');
        //uart_putc_noint_no_wait('2');
        //uart_putc_noint_no_wait(',');
        usbmass_ep2_rx_handler();
    }
    //printf("\r\n");


}
#endif

/*******************************************************************************
* Function Name  : SetEPTxStatus
* Description    : Set the status of Tx endpoint.
* Input          : bEpNum: Endpoint Number.
*                  wState: new state.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SetEPTxStatus(uint8_t bEpNum, uint16_t wState)
{
    //_SetEPTxStatus(bEpNum, wState);
    REGB(OTG_INDEX) = bEpNum;
    if(bEpNum == USBMASS_OUT_EP && wState == EP_TX_VALID)
        REGB(OTG_TXCSR1) = USBD_TXCSR1_TXPKTRDY;
    else if(bEpNum == USBMASS_OUT_EP && wState == EP_TX_STALL)
        REGB(OTG_TXCSR1) = USBD_TXCSR1_SENDSTALL;
}

/*******************************************************************************
* Function Name  : SetEPRxStatus
* Description    : Set the status of Rx endpoint.
* Input          : bEpNum: Endpoint Number.
*                  wState: new state.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SetEPRxStatus(uint8_t bEpNum, uint16_t wState)
{
    //_SetEPRxStatus(bEpNum, wState);
    REGB(OTG_INDEX) = bEpNum;
    if(bEpNum == USBMASS_IN_EP && wState == EP_RX_VALID)
        ;//REGB(OTG_RXCSR1) = REGB(OTG_RXCSR1) & (~USBD_RXCSR1_SENDSTALL);
    else if(bEpNum == USBMASS_IN_EP && wState == EP_RX_STALL)
        ;//REGB(OTG_RXCSR1) = USBD_RXCSR1_SENDSTALL;
}

/*******************************************************************************
* Function Name  : GetEPRxStatus
* Description    : Returns the endpoint Rx status.
* Input          : bEpNum: Endpoint Number.
* Output         : None.
* Return         : Endpoint RX Status
*******************************************************************************/
uint16_t GetEPRxStatus(uint8_t bEpNum)
{
    //return(_GetEPRxStatus(bEpNum));
    return 0;
}

/*******************************************************************************
* Function Name  : SetEPTxCount.
* Description    : Set the Tx count.
* Input          : bEpNum: Endpoint Number.
*                  wCount: new count value.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SetEPTxCount(uint8_t bEpNum, uint16_t wCount)
{
    //_SetEPTxCount(bEpNum, wCount);
}


