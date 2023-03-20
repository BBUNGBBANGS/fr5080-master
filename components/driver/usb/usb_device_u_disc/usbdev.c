#include <string.h>

#include "driver_plf.h"

#include "usb.h"
#include "usbmass.h"
#include "otg_reg.h"

#define USB_SUPPORT_AUDIO_DEV           0

static uint32_t ep0_tx_buf[512/4];
static uint32_t ep0_tx_size, ep0_finished;
static uint32_t req_cmd_buf[2];
static uint32_t req_data_buf[32];
static uint8_t *g_ret_ptr;
static uint8_t g_ret_size;
static uint8_t g_set_addr_flag;
static uint8_t ep0_state;
int (*fp_set_func)(USBREQPACKET *dreq, uint8_t *arg);
uint8_t setup_state;

static void usbdev_ep0send_pkt(uint8_t *buf, int size)
{
    memcpy((void *)ep0_tx_buf, buf, size);
    ep0_tx_size = size;
    ep0_finished = 0;
}

static void usbdev_process_extra_data(USBREQPACKET *dreq, uint8_t *inbuf)
{
    uint8_t size;
    size = REGB(OTG_COUNT0);
    usb_read_fifo(inbuf, size, OTG_EP0FIFO);
}


static int usbdev_set_address(USBREQPACKET *dreq)
{
    REGB(OTG_FADDR) = dreq->wValue[0];
    return 0;
}


static int usbdev_get_dev_desc(USBREQPACKET *dreq, uint8_t **ptr, int *size)
{
    get_usbmass_dev_desc(dreq, ptr, size);

    return 0;
}

static int usbdev_get_config_desc(USBREQPACKET *dreq, uint8_t **ptr, int *size)
{
    get_usbmass_config_desc(dreq, ptr, size);

    return 0;
}

static int usbdev_get_string_desc(USBREQPACKET *dreq, uint8_t **ptr, int *size)
{
    get_usbmass_string_desc(dreq, ptr, size);

    return 0;
}

static int usbdev_get_configuration(USBREQPACKET *dreq, uint8_t **ptr, int *size)
{
    get_usbmass_configuration(dreq, ptr, size);

    return 0;
}

static int usbdev_set_interface(USBREQPACKET *dreq)
{
    set_usbmass_interface(dreq);

    return 0;
}

static void usbdev_class_req_dec(USBREQPACKET *dreq)
{
    usbmass_class_req_dec(dreq);
}

static void usbdev_std_req_dec(USBREQPACKET *dreq)
{
    uint8_t *ptr = 0;
    int size = 0;
    switch (dreq->bRequest)
    {
        case USB_REQUEST_GET_DESCRIPTOR:
            if (dreq->bmRequestType == 0x80)
            {
                switch(dreq->wValue[1])
                {
                    case USB_DEVICE_DESCRIPTOR_TYPE:
                        usbdev_get_dev_desc(dreq, &ptr, &size);
                        break;
                    case USB_CONFIGURATION_DESCRIPTOR_TYPE:
                        usbdev_get_config_desc(dreq, &ptr, &size);
                        break;
                    case USB_STRING_DESCRIPTOR_TYPE:
                        usbdev_get_string_desc(dreq, &ptr, &size);
                        break;
                    default:
                        // Unsupported or Unknown Descriptor Type
                        ptr = &setup_state;
                        size = 1;
                        break;
                }
                setup_state = SETUP_SEND_RESULT_STAT;
            }
            break;
        case USB_REQUEST_GET_CONFIGURATION:
            usbdev_get_configuration(dreq, &ptr, &size);
            setup_state = SETUP_SEND_RESULT_STAT;
            break;
        case USB_REQUEST_SET_ADDRESS:
            g_set_addr_flag = 1;
            setup_state = SETUP_FINISHING;
            break;
        case USB_REQUEST_SET_CONFIGURATION:
            setup_state = SETUP_FINISHING;
            break;
        case USB_REQUEST_SET_INTERFACE:
            usbdev_set_interface(dreq);
            setup_state = SETUP_FINISHING;
            break;
        default:
            //unsupported or unknown request types
            setup_state = SETUP_FINISHING;
            break;
    }

    g_ret_ptr = ptr;
    g_ret_size = size;
}


static void usbdev_req_dec(USBREQPACKET *dreq)
{
    uint8_t type;

    type = USBDEV_REQ_TYPE(dreq->bmRequestType) ;
    switch(type)
    {
        case REQ_STANDARD:
            usbdev_std_req_dec(dreq);
            break;
        case REQ_CLASS:
            usbdev_class_req_dec(dreq);
            break;
        default:
            //Unknown Request type
            setup_state = SETUP_FINISHING;
            break;
    }
}

void usbdev_set_ep0_retval(uint8_t *ptr, int size )
{
    g_ret_ptr = ptr;
    g_ret_size = size;
}

void usbdev_device_reset()
{
    volatile uint8_t byte;

    ep0_state = EP0_IDLE_STAT;
    setup_state = SETUP_FINISHING;

    REGB(OTG_INTRTX1E) = 0;
    REGB(OTG_INTRRX1E) = 0;
    REGB(OTG_INTRUSBE) = 0;

    byte = REGB(OTG_INTRUSB);
    byte = REGB(OTG_INTRRX1);
    byte = REGB(OTG_INTRTX1);

    usbmass_device_reset();
}

void usbdev_device_init()
{
    volatile uint8_t byte;
    uint32_t loop = 0;

    while (((REGB(OTG_DEVCTL) & 0x04) != 0) && (loop ++ < 0xaffff)) {};
    usbdev_device_reset();

    byte = REGB(OTG_DEVCTL);
}

void usbdev_init(void)
{
    usbmass_var_init();
    
    NVIC_SetPriority(USBMCU_IRQn,2);
    NVIC_EnableIRQ(USBMCU_IRQn);
    *(volatile uint32_t *)0x50000040  |= 0x00051f00;    //set dev mode. PB6 PB7
    //*(volatile uint32_t *)0x50000040  |= 0x00050f00;    //set dev mode. USB dedicated pin
}


void usbdev_ep0_handler()
{
    uint8_t         csr0;
    USBREQPACKET *dreq;
    dreq = (USBREQPACKET *)req_cmd_buf;

    /* Read CSR0 */
    REGB(OTG_INDEX) = 0;
    csr0 = REGB(OTG_CSR0);
    //printf("csr0:%x,ep0_state:%x,setup_state:%x,",csr0,ep0_state,setup_state);

    if (csr0 & USBD_CSR0_SENTSTALL)
    {
        REGB(OTG_CSR0) = csr0 & (~USBD_CSR0_SENTSTALL);
        ep0_state = EP0_IDLE_STAT;
    }

    /* Check for SetupEnd */
    if (csr0 & USBD_CSR0_SETUPEND)
    {
        REGB(OTG_CSR0) = csr0 | USBD_CSR0_SVCSETUPEND;
        ep0_state = EP0_IDLE_STAT;
    }

    /* Call relevant routines for endpoint 0 state */
    if (ep0_state == EP0_IDLE_STAT)
    {
        if(csr0 & USBD_CSR0_RXPKTRDY)
        {
            if(setup_state == SETUP_FINISHING)
            {
                setup_state = SETUP_DEC_CMMD_STAT;
                usb_read_fifo32((uint32_t *)dreq, 2, OTG_EP0FIFO);
                //show_reg((uint8_t *)dreq,sizeof(USBREQPACKET),1);
                usbdev_req_dec(dreq);
                if(setup_state == SETUP_FINISHING)
                    REGB(OTG_CSR0) = USBD_CSR0_SVCRXPKTRDY | USBD_CSR0_DATAEND;
                else if(setup_state == SETUP_DEC_CMMD_STAT)
                {
                    REGB(OTG_CSR0) = USBD_CSR0_SVCRXPKTRDY | USBD_CSR0_DATAEND;
                    setup_state = SETUP_FINISHING;
                }
            }

            if(setup_state == SETUP_SEND_RESULT_STAT)
            {
                setup_state = SETUP_FINISHING;
                if(g_ret_size)
                {
                    REGB(OTG_CSR0) = USBD_CSR0_SVCRXPKTRDY;
                    //printf("ep0send:%d,%d\r\n",g_ret_ptr, g_ret_size);
                    usbdev_ep0send_pkt(g_ret_ptr, g_ret_size);
                    ep0_state = EP0_TX_STAT;
                }
                else
                {
                    REGB(OTG_CSR0) = USBD_CSR0_SVCRXPKTRDY | USBD_CSR0_DATAEND;
                }
            }

            if(setup_state == SETUP_GOT_EXTRA_DATA_STAT)
            {
                setup_state = SETUP_FINISHING;
                dreq = (USBREQPACKET *)req_cmd_buf;
                usbdev_process_extra_data(dreq, (uint8_t *)req_data_buf);
                REGB(OTG_CSR0) = USBD_CSR0_SVCRXPKTRDY | USBD_CSR0_DATAEND;
                if(fp_set_func)
                {
                    fp_set_func(dreq, (uint8_t *)req_data_buf);
                    fp_set_func = NULL;
                }
            }
            else if(setup_state == SETUP_REQUIRE_EXTRA_DATA_STAT)
            {
                REGB(OTG_CSR0) = USBD_CSR0_SVCRXPKTRDY;
                setup_state = SETUP_GOT_EXTRA_DATA_STAT;
            }
        }
        else if(g_set_addr_flag)
        {
            /*this operation should be done after SET_ADDRESS hand-shake*/
            g_set_addr_flag = 0;
            usbdev_set_address(dreq);
        }
    }

    /*Return result to Host*/
    if (ep0_state == EP0_TX_STAT)
    {
        if (ep0_tx_size - ep0_finished <= 64)
        {
            usb_write_fifo((uint8_t *)((uint32_t)ep0_tx_buf + ep0_finished),
                           ep0_tx_size - ep0_finished, OTG_EP0FIFO);
            ep0_finished = ep0_tx_size;
            REGB(OTG_CSR0) = USBD_CSR0_TXPKTRDY | USBD_CSR0_DATAEND;
            ep0_state = EP0_IDLE_STAT;
        }
        else
        {
            usb_write_fifo((uint8_t *)((uint32_t)ep0_tx_buf + ep0_finished),
                           64, OTG_EP0FIFO);
            REGB(OTG_CSR0) = USBD_CSR0_TXPKTRDY;
            ep0_finished += 64;
        }
    }
    return;
}

