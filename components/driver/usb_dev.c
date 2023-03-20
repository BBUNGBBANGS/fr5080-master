/*
  ******************************************************************************
  * @file    usb_dev.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   This file provides all the USBD device functions.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "usb_dev.h"

uint8_t USB_StatusBuffer[5];

usb_DescriptorsTypeDef_t DevDescriptors;

usb_StandardRequest_t StandardRequest;

usb_ReturnData_t ReturnData;

void (*Endpoint_0_ClassRequest_Handler)(usb_StandardRequest_t* pStandardRequest, usb_ReturnData_t* pReturnData)  = NULL;
void (*Endpoint_0_VendorRequest_Handler)(usb_StandardRequest_t* pStandardRequest, usb_ReturnData_t* pReturnData) = NULL;

void (*Endpoint_0_DataOut_Handler)(void) = NULL;

void (*Endpoints_Handler)(uint8_t RxStatus, uint8_t TxStatus) = NULL;

void (*USB_Reset_Handler)(void) = NULL;

/*********************************************************************
 * @fn      usbdev_get_dev_desc
 *
 * @brief   Get device descriptor buffer pointer.
 *********************************************************************/
void usbdev_get_dev_desc(uint8_t *Descriptor)
{
    DevDescriptors.DeviceDescriptor = Descriptor;
}

/*********************************************************************
 * @fn      usbdev_get_config_desc
 *
 * @brief   USB full speed device Get Configuration¡¢Interface¡¢
 *          Endpoint Descriptor.
 *********************************************************************/
void usbdev_get_config_desc(uint8_t *Descriptor)
{
    DevDescriptors.ConfigurationDescriptor = Descriptor;
}

/*********************************************************************
 * @fn      usbdev_get_hidreport_desc
 *
 * @brief   Get HID device report descriptor. (only HID device)
 *********************************************************************/
void usbdev_get_hidreport_desc(uint8_t fu8_ReportIndex, uint8_t *Descriptor)
{
    DevDescriptors.HIDReportDescriptor[fu8_ReportIndex] = Descriptor;
}

/*********************************************************************
 * @fn      usbdev_get_string_Manufacture
 *
 * @brief   Get device Manufacture string Descriptor.
 *********************************************************************/
void usbdev_get_string_Manufacture(uint8_t *Descriptor)
{
    DevDescriptors.stringManufacture = Descriptor;
}

/*********************************************************************
 * @fn      usbdev_get_string_Product
 *
 * @brief   Get device Product string Descriptor.
 *********************************************************************/
void usbdev_get_string_Product(uint8_t *Descriptor)
{
    DevDescriptors.stringProduct = Descriptor;
}

/*********************************************************************
 * @fn      usbdev_get_string_SerialNumber
 *
 * @brief   Get device SerialNumber string Descriptor.
 *********************************************************************/
void usbdev_get_string_SerialNumber(uint8_t *Descriptor)
{
    DevDescriptors.stringSerialNumber = Descriptor;
}

/*********************************************************************
 * @fn      usbdev_get_string_LanuageID
 *
 * @brief   Get device LanuageID string Descriptor.
 *********************************************************************/
void usbdev_get_string_LanuageID(uint8_t *Descriptor)
{
    DevDescriptors.stringLanuageID = Descriptor;
}

/*********************************************************************
 * @fn      usbdev_get_string_OS
 *
 * @brief   Get OS String Descriptor.
 *********************************************************************/
void usbdev_get_string_OS(uint8_t *Descriptor)
{
    DevDescriptors.stringOS = Descriptor;
}

/*********************************************************************
 * @fn      Endpoint_0_IRQHandler
 *
 * @brief   USB endpoint0 Interrupt Request handler
 *
 * @param   None.
 * @return  None.
 */
static void Endpoint_0_IRQHandler(void)
{
    uint8_t lu8_RxCount;
    
    static bool lb_WaitSetAddress = false;

    usb_selecet_endpoint(ENDPOINT_0);
    
    /* Clear SetupEnd status */
    if (USB_POINT0->CSR01 & USB_CSR01_SETUPEND)
    {
        USB_POINT0->CSR01 |= USB_CSR01_SVCSETUPEND;
    }


    /* endpoint 0 receive a packet */
    if (usb_Endpoint0_GET_RxPktRdy()) 
    {
        /* Data Out packet */
        if (Endpoint_0_DataOut_Handler != NULL) 
        {
            Endpoint_0_DataOut_Handler();
        }
        /* Standard Request packet */
        else 
        {
            lu8_RxCount = usb_Endpoint0_get_RxCount();

            usb_read_fifo(ENDPOINT_0, (uint8_t *)&StandardRequest, lu8_RxCount);

            /* Standard Request */
            if ((StandardRequest.bmRequestType & REQUEST_TYPE_MASK) == TYPE_STANDARD)
            {
                switch (StandardRequest.bRequest)
                {
                    /* request descriptor */
                    case REQUEST_GET_DESCRIPTOR: 
                    {
                        /* Build the returned data */
                        ReturnData.RequestLength  = (uint16_t)StandardRequest.wLength[1] << 8;
                        ReturnData.RequestLength |= (uint16_t)StandardRequest.wLength[0];

                        switch (StandardRequest.wValue[1])
                        {
                            case DESCRIPTOR_DEVICE: 
                            {
                                ReturnData.DataBuffer = DevDescriptors.DeviceDescriptor;
                                ReturnData.DataLength = ReturnData.DataBuffer[0];
                            }break;

                            case DESCRIPTOR_CONFIGURATION: 
                            {
                                ReturnData.DataBuffer  = DevDescriptors.ConfigurationDescriptor;
                                ReturnData.DataLength  = (uint32_t)ReturnData.DataBuffer[3] << 8;
                                ReturnData.DataLength |= (uint32_t)ReturnData.DataBuffer[2];
                            }break;

                            case DESCRIPTOR_STRING: 
                            {
                                switch (StandardRequest.wValue[0])
                                {
                                    case STRING_MANUFACTURE: 
                                    {
                                        ReturnData.DataBuffer = DevDescriptors.stringManufacture;
                                    }break;
                                    
                                    case STRING_PRODUCT: 
                                    {
                                        ReturnData.DataBuffer = DevDescriptors.stringProduct;
                                    }break;
                                    
                                    case STRING_SERIAL_Number: 
                                    {
                                        ReturnData.DataBuffer = DevDescriptors.stringSerialNumber;
                                    }break;
                                    
                                    case STRING_LANUAGE_ID: 
                                    {
                                        ReturnData.DataBuffer = DevDescriptors.stringLanuageID;
                                    }break;

                                    case STRING_OS: 
                                    {
                                        ReturnData.DataBuffer = DevDescriptors.stringOS;
                                    }break;
                                    
                                    default: break; 
                                }
                                
                                ReturnData.DataLength = ReturnData.DataBuffer[0];
                            }break;

                            case DESCRIPTOR_DEVICE_QUALIFIER:
                            {
                                /* USB2.0 full speed send stall */
                                usb_Endpoint0_SendStall();
                            }break;

                            case DESCRIPTOR_HID_REPORT:
                            {
                                ReturnData.DataLength =  DevDescriptors.HIDReportDescriptor[StandardRequest.wIndex[0]][0];
                                ReturnData.DataBuffer = &DevDescriptors.HIDReportDescriptor[StandardRequest.wIndex[0]][1];
                            }break;

                            default: break; 
                        }
                    }break;

                    /* set device address */
                    case REQUEST_SET_ADDRESS: 
                    {
                        /* wait an empty IN packet  */
                        lb_WaitSetAddress = true;
                    }break;

                    /* set configuration */
                    case REQUEST_SET_CONFIGURATION: 
                    {
                        
                    }break;

                    /* get status */
                    case REQUEST_GET_STATUS: 
                    {
                        /* Build the returned data */
                        ReturnData.RequestLength  = (uint16_t)StandardRequest.wLength[1] << 8;
                        ReturnData.RequestLength |= (uint16_t)StandardRequest.wLength[0];
                        
                        ReturnData.DataBuffer = USB_StatusBuffer;
                        ReturnData.DataLength = ReturnData.RequestLength;
                    }break;
                    
                    default: break; 
                }
            }
            /* Class Request */
            else if ((StandardRequest.bmRequestType & REQUEST_TYPE_MASK) == TYPE_CLASS)
            {
                if (Endpoint_0_ClassRequest_Handler != NULL) 
                {
                    Endpoint_0_ClassRequest_Handler(&StandardRequest, &ReturnData);
                }
            }
            /* Vendor Request */
            else if ((StandardRequest.bmRequestType & REQUEST_TYPE_MASK) == TYPE_VENDOR)
            {
                if (Endpoint_0_VendorRequest_Handler != NULL) 
                {
                    Endpoint_0_VendorRequest_Handler(&StandardRequest, &ReturnData);
                }
            }

            /* clear TxPktRdy/RxPktRdy */
            usb_Endpoint0_FlushFIFO();
        }

    }
    /* endpoint 0 transmit a packet */
    else 
    {
        if (lb_WaitSetAddress == true) 
        {
            lb_WaitSetAddress = false;

            usb_set_address(StandardRequest.wValue[0]);
        }
    }

    /* DataBuffer needs to be sent */
    if (ReturnData.DataLength)
    {
        if (ReturnData.RequestLength < ReturnData.DataLength) 
        {
            ReturnData.DataLength = ReturnData.RequestLength;
        }

        if (ReturnData.DataLength > ENDPOINT0_MAX) 
        {
            usb_write_fifo(ENDPOINT_0, ReturnData.DataBuffer, ENDPOINT0_MAX);

            ReturnData.DataLength -= ENDPOINT0_MAX;
            ReturnData.DataBuffer += ENDPOINT0_MAX;
        }
        else 
        {
            usb_write_fifo(ENDPOINT_0, ReturnData.DataBuffer, ReturnData.DataLength);

            ReturnData.DataLength = 0;

            usb_Endpoint0_DataEnd();
        }
        usb_Endpoint0_SET_TxPktRdy();
    }
}

/*********************************************************************
 * @fn      Endpoints_IRQHandler
 *
 * @brief   ALL Endpoint Interrupt Request handler
 *
 * @param   None.
 * @return  None.
 */
static void Endpoints_IRQHandler(void)
{
    volatile uint8_t lu8_RxStatus;
    volatile uint8_t lu8_TxStatus;
    volatile uint8_t lu8_EndpointBackup;

    lu8_EndpointBackup = usb_get_endpoint();

    lu8_TxStatus = usb_get_TxStatus();
    lu8_RxStatus = usb_get_RxStatus();

    /* endpoint 0 Interrupt handler */
    if (lu8_TxStatus & ENDPOINT_0_MASK) 
    {
        lu8_TxStatus &= ~ENDPOINT_0_MASK;

        Endpoint_0_IRQHandler();
    }

    /* endpoint 1 ~ 5 Rx¡¢Tx Interrupt handler */
    if (lu8_RxStatus | lu8_TxStatus) 
    {
        if (Endpoints_Handler != NULL) 
        {
            Endpoints_Handler(lu8_RxStatus, lu8_TxStatus);
        }
    }

    usb_selecet_endpoint((enum_Endpoint_t)lu8_EndpointBackup);
}

static void USB_Status_IRQHandler(void)
{
    volatile uint8_t lu8_EndpointBackup;
	
    lu8_EndpointBackup = usb_get_endpoint();

    /* USB Bus reset signal, Will be clear USB register */
    /* need configure USB register again */
    if (usb_get_USBStatus() & USB_INT_STATUS_RESET) 
    {
        USB->IntrUSBE = 0x04;    /* Enable Bus reset INT */
        USB->IntrTx1E = 0x01;    /* Enable Endpoint0 INT */
        USB->IntrTx2E = 0x00;
        USB->IntrRx1E = 0x00;
        USB->IntrRx2E = 0x00;

        if (USB_Reset_Handler != NULL) 
        {
            USB_Reset_Handler();
        }
    }

    usb_selecet_endpoint((enum_Endpoint_t)lu8_EndpointBackup);
}

#if USB_DEVICE_ENABLE == 0
/*********************************************************************
 * @fn      USB_IRQHandler
 *
 * @brief   USB interrupt Request handler
 *
 * @param   None.
 * @return  None.
 */
__attribute__((section("ram_code"))) void USB_IRQHandler(void) 
{
    Endpoints_IRQHandler();

    USB_Status_IRQHandler();
}
#endif
