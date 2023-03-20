/*
  ******************************************************************************
  * @file    usb_core.c
  * @author  FreqChip Firmware Team
  * @version V1.1.0
  * @date    2021
  * @brief   This file provides all the USBD core functions.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "usb_core.h"

/*********************************************************************
 * @fn      usb_device_init
 *
 * @brief   Initializes the device stack.
 *
 * @param   None.
 * @return  None.
 */
void usb_device_init(void)
{
    /* USB 12M clock enable */
    /* USB 48M clock enable */
    /* USB HCLK enable */
    *(volatile uint32_t *)0x50000004 |= 0xE0000000;

    /* USB_IO_PS0     = 1 */
    /* OTG_VBUS_VAL_I = 1 */
    /* OTG_VBUS_SES_I = 1 */
    /* OTG_VBUSLO_I   = 1 */
    /* OTG_CID_I      = 1 */
    /* USB_STDBY_SEL  = 1 */
    *(volatile uint32_t *)0x50000040 |= 0x00051f00;

    /* Disable USB all Interrupt except endpoint0 and Bus reset*/
    USB->IntrUSBE = 0x04;    /* Enable Bus reset INT */
    USB->IntrTx1E = 0x01;    /* Enable Endpoint0 INT */
    USB->IntrTx2E = 0x00;
    USB->IntrRx1E = 0x00;
    USB->IntrRx2E = 0x00;
}

/*********************************************************************
 * @fn      usb_selecet_endpoint
 *
 * @brief   Selected Endpoint
 *
 * @param   Endpoint : endpoint select.
 * @return  None.
 */
void usb_selecet_endpoint(enum_Endpoint_t Endpoint)
{
    USB->Index = Endpoint;
}

/*********************************************************************
 * @fn      usb_get_endpoint
 *
 * @brief   get current Endpoint
 *
 * @param   None.
 * @return  current Endpoint
 */
uint8_t usb_get_endpoint(void)
{
    return USB->Index;
}

/*********************************************************************
 * @fn      usb_set_address
 *
 * @brief   set device address
 *
 * @param   address : device address.
 * @return  None.
 */
void usb_set_address(uint8_t address)
{
    USB->FAddr = address;
}

/*********************************************************************
 * @fn      usb_get_frame
 *
 * @brief   get current frame number
 *
 * @param   None.
 * @return  None.
 */
uint32_t usb_get_frame(void)
{
    uint32_t lu32_Frame;

    lu32_Frame  = (uint32_t)USB->Frame2 << 8;
    lu32_Frame |= (uint32_t)USB->Frame1;

    return lu32_Frame;
}

/*********************************************************************
 * @fn      usb_TxEndpointSync_enable
 *
 * @brief   Tx Synchronous endpoint enable
 */
void usb_TxSyncEndpoint_enable(void)
{
	USB_POINT1_5->TxCSR2 |= USB_TXCSR2_ISO;
}

/*********************************************************************
 * @fn      usb_RxSyncEndpoint_enable
 *
 * @brief   Rx Synchronous endpoint enable
 */
void usb_RxSyncEndpoint_enable(void)
{
    USB_POINT1_5->RxCSR2 |= USB_RXCSR2_ISO;
}

/*********************************************************************
 * @fn      usb_TxInt_Enable
 *
 * @brief   Enable transmit completion interrupt
 *
 * @param   Endpoint : endpoint select.
 * @return  None.
 */
void usb_TxInt_Enable(enum_Endpoint_t Endpoint)
{
    USB->IntrTx1E |= 1 << Endpoint;
}

/*********************************************************************
 * @fn      usb_TxInt_Disable
 *
 * @brief   Disable transmit completion interrupt
 *
 * @param   Endpoint : endpoint select.
 * @return  None.
 */
void usb_TxInt_Disable(enum_Endpoint_t Endpoint)
{
    USB->IntrTx1E &= ~(1 << Endpoint);
}

/*********************************************************************
 * @fn      usb_RxInt_Enable
 *
 * @brief   Enable receive completion interrupt
 *
 * @param   Endpoint : endpoint select.
 * @return  None.
 */
void usb_RxInt_Enable(enum_Endpoint_t Endpoint)
{
    if (Endpoint == ENDPOINT_0)
        return;

    USB->IntrRx1E |= 1 << Endpoint;
}

/*********************************************************************
 * @fn      usb_RxInt_Disable
 *
 * @brief   Disable receive completion interrupt
 *
 * @param   Endpoint : endpoint select.
 * @return  None.
 */
void usb_RxInt_Disable(enum_Endpoint_t Endpoint)
{
    if (Endpoint == ENDPOINT_0)
        return;

    USB->IntrRx1E &= ~(1 << Endpoint);
}

/*********************************************************************
 * @fn      usb_endpoint_Txfifo_config
 *
 * @brief   config Txfifo
 *
 * @param   StartAddress: Start address of the endpoint FIFO.
 *          MaxPacket   : Maximum packet size.
 *                        
 * @return  None.
 */
void usb_endpoint_Txfifo_config(uint32_t StartAddress, uint32_t MaxPacket)
{
    /* Start address of the endpoint FIFO in units of 8 TxFIFO1 bytes as follows: */
    /* --------------------------------- */
    /* |   StartAddress   |   Address  | */
    /* |       0x00       |    0x000   | */
    /* |       0x01       |    0x008   | */
    /* |       0x02       |    0x010   | */
    /* |      ...         |     ...    | */
    /* |       0x7F       |    0x3FF   | */
    /* --------------------------------- */
    
    
    /* Maximum packet size to be allowed for */
    /* --------------------------------------- */
    /* |   MaxPacket   |  Packet Size(Bytes) | */
    /* |       0       |          8          | */
    /* |       1       |          16         | */
    /* |       2       |          32         | */
    /* |       3       |          64         | */
    /* |       4       |          128        | */
    /* |       5       |          256        | */
    /* |       6       |          512        | */
    /* |       7       |          1024       | */
    /* --------------------------------------- */

    /* use only 7bit */
    StartAddress &= 0x7F;
    
    /* use only 3bit */
    MaxPacket &= 0x7;

    USB_POINT1_5->TxFIFO1 = StartAddress;
    
    USB_POINT1_5->TxFIFO2 = MaxPacket << 5;
}

/*********************************************************************
 * @fn      usb_endpoint_Rxfifo_config
 *
 * @brief   config Rxfifo
 *
 * @param   StartAddress: Start address of the endpoint FIFO.
 *          MaxPacket   : Maximum packet size.
 *                        
 * @return  None.
 */
void usb_endpoint_Rxfifo_config(uint32_t StartAddress, uint32_t MaxPacket)
{
    /* 
        reference usb_endpoint_Txfifo_config()
     */

    /* use only 7bit */
    StartAddress &= 0x7F;

    /* use only 3bit */
    MaxPacket &= 0x7;

    USB_POINT1_5->RxFIFO1 = StartAddress;

    USB_POINT1_5->RxFIFO2 = MaxPacket << 5;
}

/*********************************************************************
 * @fn      usb_TxMaxP_set
 *
 * @brief   the maximum packet size for transactions through the 
 *          currently-selected Tx endpoint
 *
 * @param   MaxPacket: in units of 8 bytes
 * @return  None.
 */
void usb_TxMaxP_set(uint32_t MaxPacket)
{
    /* Maximum packet size to be allowed for */
    /* --------------------------------------- */
    /* |   MaxPacket   |  Packet Size(Bytes) | */
    /* |       0       |          0          | */
    /* |       1       |          8          | */
    /* |       2       |          16         | */
    /* |      ...      |          ...        | */
    /* |       128     |          1024       | */
    /* --------------------------------------- */

    USB_POINT1_5->TxMaxP = MaxPacket;
}

/*********************************************************************
 * @fn      usb_RxMaxP_set
 *
 * @brief   the maximum packet size for transactions through the 
 *          currently-selected Rx endpoint
 *
 * @param   MaxPacket: in units of 8 bytes
 * @return  None.
 */
void usb_RxMaxP_set(uint32_t MaxPacket)
{
    /* 
        reference usb_TxMaxP_set()
     */
    
    USB_POINT1_5->RxMaxP = MaxPacket;
}

#if USB_DEVICE_ENABLE == 0
/*********************************************************************
 * @fn      usb_write_fifo
 *
 * @brief   Write data to the endpoint fifo
 *
 * @param   Endpoint : endpoint select.
 *          Buffer   : transmit buffer pointer.
 *          Size     : transmit Size.
 * @return  None.
 */
void usb_write_fifo(enum_Endpoint_t Endpoint, uint8_t *Buffer, uint32_t Size)
{
    volatile uint8_t *fifo;

    fifo = &USB_POINT1_5->FIFO[Endpoint * 4];

    while (Size--)
    {
        *fifo = *Buffer++;
    }
}

/*********************************************************************
 * @fn      usb_read_fifo
 *
 * @brief   Reads data from the endpoint fifo
 *
 * @param   Endpoint : endpoint select.
 *          Buffer   : receive buffer pointer.
 *          Size     : receive Size.
 * @return  None.
 */
void usb_read_fifo(enum_Endpoint_t Endpoint, uint8_t *Buffer, uint32_t Size)
{
    volatile uint8_t *fifo;

    fifo = &USB_POINT1_5->FIFO[Endpoint * 4];

    while (Size--)
    {
        *Buffer++ = *fifo;
    }
}
#endif

/*********************************************************************
 * @fn      usb_DP_Pullup_Enable
 *
 * @brief   USB DP port pull up enable 
 *
 * @param   None.
 * @return  None.
 */
void usb_DP_Pullup_Enable(void)
{
    USB_OTG_CTRL->USB_IO_PS0 = 1;
}

/*********************************************************************
 * @fn      usb_DP_Pullup_Disable
 *
 * @brief   USB DP port pull up disable 
 *
 * @param   None.
 * @return  None.
 */
void usb_DP_Pullup_Disable(void)
{
    USB_OTG_CTRL->USB_IO_PS0 = 0;
}

