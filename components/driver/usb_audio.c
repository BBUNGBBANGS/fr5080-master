/*
  ******************************************************************************
  * @file    usb_audio.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   This file provides the high layer firmware functions to manage the 
  *          USB Audio Device.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  * 
  ******************************************************************************
*/
#include "usb_audio.h"

uint8_t gu8_Respond[5];
uint8_t gu8_AudioReport;

uint32_t gu32_SamplingFreqSpeaker;
uint32_t gu32_SamplingFreqMic;

uint16_t gu16_VolumeSpeaker = 0xAF;
uint16_t gu16_VolumeMic     = 0xAF;
uint8_t  gu8_MuteSpeaker    = 0x00;
uint8_t  gu8_MuteMic        = 0x00;

/* Audio Set Control */
typedef enum
{
    AUDIO_SET_VOL_SPEAKER,        /* set Speaker    Volume */
    AUDIO_SET_VOL_MIC,            /* set Microphone Volume */
    AUDIO_SET_MUTE_SPEAKER,       /* set Speaker    Mute */
    AUDIO_SET_MUTE_MIC,           /* set Microphone Mute */
    AUDIO_SET_SAMPLING_SPEAKER,   /* set Speaker    Sampling Frequency */
    AUDIO_SET_SAMPLING_MIC,       /* set Microphone Sampling Frequency */
}Audio_SetControl;

Audio_SetControl ge_CTLIndex;

/* USB Standard Device Descriptor */
const uint8_t USB_Audio_DeviceDesc[] =
{
    0x12,    /* bLength */
    0x01,    /* bDescriptorType */
    0x00,    /* bcdUSB */
    0x02,
    0x00,    /* bDeviceClass */
    0x00,    /* bDeviceSubClass */
    0x00,    /* bDeviceProtocol */
    0x40,    /* bMaxPacketSize */
    0xA4,    /* idVendor */
    0xA5,    /* idVendor */
    0xA6,    /* idProduct */
    0xA7,    /* idProduct */
    0x00,    /* bcdDevice rel. 2.00 */
    0x02,
    0x01,    /* Index of manufacturer string */
    0x02,    /* Index of product string */
    0x03,    /* Index of serial number string */
    0x01,    /* bNumConfigurations */
};

/* USB Standard Configuration Descriptor */
const uint8_t USB_Audio_ConfigurationDesc[] =
{
    /* Configuration Descriptor */
    0x09,    /* bLength */             
    0x02,    /* bDescriptorType */     
    0xF1,    /* wTotalLength */        
    0x00,                              
    0x04,    /* bNumInterfaces */      
    0x01,    /* bConfigurationValue */ 
    0x00,    /* iConfiguration */      
    0xC0,    /* bmAttributes */        
    0x32,    /* bMaxPower */           

        /* HID Interface_0 Descriptor */
        0x09,    /* bLength */           
        0x04,    /* bDescriptorType */   
        0x00,    /* bInterfaceNumber */  
        0x00,    /* bAlternateSetting */ 
        0x00,    /* bNumEndpoints */     
        0x01,    /* bInterfaceClass: Audio */   
        0x01,    /* bInterfaceSubClass: Audio Control */
        0x00,    /* bInterfaceProtocol */
        0x00,    /* iConfiguration */    

            /* Audio Control Interface Header Descriptor */
            0x0A,    /* bLength */
            0x24,    /* bDescriptorType: CS_INTERFACE */
            0x01,    /* bDescriptorSubtype: HEADER descriptor subtype */
            0x00,    /* bcdADC */
            0x01,
            0x4C,    /* wTotalLength */
            0x00,
            0x02,    /* bInCollection */
            0x01,    /* baInterfaceNr(1) */
            0x02,    /* baInterfaceNr(2) */

            /* Audio Control Input Terminal Descriptor */
            0x0C,    /* bLength */
            0x24,    /* bDescriptorType: CS_INTERFACE */
            0x02,    /* bDescriptorSubtype: INPUT_TERMINAL */
            0x01,    /* bTerminalID */
            0x01,    /* wTerminalType: USB streaming */
            0x01,    
            0x00,    /* bAssocTerminal */
            0x02,    /* bNrChannels */
            0x03,    /* wChannelConfig */
            0x00,    
            0x00,    /* iChannelNames */
            0x00,    /* iTerminal */

            /* Audio Control Feature Unit Descriptor */
            0x0D,    /* bLength */
            0x24,    /* bDescriptorType: CS_INTERFACE */
            0x06,    /* bDescriptorSubtype: FEATURE_UNIT */
            0x02,    /* bUnitID */
            0x01,    /* bSourceID */
            0x02,    /* bControlSize */
            0x01,    
            0x00,    
            0x02,    
            0x00,    
            0x02,    
            0x00,    
            0x00,    /* iFeature */

            /* Audio Control Output Terminal Descriptor */
            0x09,    /* bLength */
            0x24,    /* bDescriptorType: CS_INTERFACE */
            0x03,    /* bDescriptorSubtype: OUTPUT_TERMINAL */
            0x03,    /* bTerminalID */
            0x02,    /* wTerminalType: Headphones */
            0x03,    
            0x00,    /* bAssocTerminal */
            0x02,    /* bSourceID */
            0x00,    /* iTerminal */
            
            /* Audio Control Input Terminal Descriptor */
            0x0C,    /* bLength */
            0x24,    /* bDescriptorType: CS_INTERFACE */
            0x02,    /* bDescriptorSubtype: INPUT_TERMINAL */
            0x04,    /* bTerminalID */
            0x01,    /* wTerminalType: Microphone */
            0x02,    
            0x00,    /* bAssocTerminal */
            0x01,    /* bNrChannels */
            0x04,    /* wChannelConfig */
            0x00,    
            0x00,    /* iChannelNames */
            0x00,    /* iTerminal */
            
            /* Audio Control Feature Unit Descriptor */
            0x0B,    /* bLength */
            0x24,    /* bDescriptorType: CS_INTERFACE */
            0x06,    /* bDescriptorSubtype: FEATURE_UNIT */
            0x05,    /* bUnitID */
            0x04,    /* bSourceID */
            0x02,    /* bControlSize */
            0x01,    
            0x00,    
            0x02,    
            0x00,    
            0x00,    /* iFeature */

            /* Audio Control Output Terminal Descriptor */
            0x09,    /* bLength */
            0x24,    /* bDescriptorType: CS_INTERFACE */
            0x03,    /* bDescriptorSubtype: OUTPUT_TERMINAL */
            0x06,    /* bTerminalID */
            0x01,    /* wTerminalType: USB streaming */
            0x01,    
            0x00,    /* bAssocTerminal */
            0x05,    /* bSourceID */
            0x00,    /* iTerminal */

        /* HID Interface_1/0 Descriptor */
        0x09,    /* bLength */           
        0x04,    /* bDescriptorType */   
        0x01,    /* bInterfaceNumber */  
        0x00,    /* bAlternateSetting */ 
        0x00,    /* bNumEndpoints */     
        0x01,    /* bInterfaceClass: Audio */   
        0x02,    /* bInterfaceSubClass: Audio Streaming */
        0x00,    /* bInterfaceProtocol */
        0x00,    /* iConfiguration */

        /* HID Interface_1/1 Descriptor */
        0x09,    /* bLength */           
        0x04,    /* bDescriptorType */   
        0x01,    /* bInterfaceNumber */  
        0x01,    /* bAlternateSetting */ 
        0x01,    /* bNumEndpoints */     
        0x01,    /* bInterfaceClass: Audio */   
        0x02,    /* bInterfaceSubClass: Audio Streaming */
        0x00,    /* bInterfaceProtocol */
        0x00,    /* iConfiguration */    

            /* Class-Specific AS Interface Descriptor */
            0x07,    /* bLength */
            0x24,    /* bDescriptorType: CS_INTERFACE */
            0x01,    /* bDescriptorSubtype: AS_GENERAL */
            0x01,    /* bTerminalLink */
            0x00,    /* bDelay */
            0x01,    /* wFormatTag: PCM */
            0x00,
            
            /* Class-Specific AS Format Type Descriptor */
            0x14,    /* bLength */
            0x24,    /* bDescriptorType: CS_INTERFACE */
            0x02,    /* bDescriptorSubtype: FORMAT_TYPE */
            0x01,    /* bFormatType: FORMAT_TYPE_I */
            0x02,    /* bNrChannels */
            0x02,    /* bSubframeSize */
            0x10,    /* bBitResolution */
            0x04,    /* bSamFreqType */
            0x80,    /* tSamFreq [1] */
            0x3E,
            0x00,
            0x00,    /* tSamFreq [2] */
            0x7D,
            0x00, 
            0x80,    /* tSamFreq [3] */
            0xBB,
            0x00,
            0x00,    /* tSamFreq [4] */
            0x77,
            0x01,

            /* Endpoint 2 Descriptor */
            0x09,    /* bLength */
            0x05,    /* bDescriptorType */
            0x02,    /* bEndpointAddress: OUT 2 */
            0x0D,    /* bmAttributes: Isochronous */ 
            0x00,    /* wMaxPacketSize: 256byte */
            0x01,
            0x01,    /* bInterval */
            0x00,    /* bRefresh */
            0x00,    /* bSynchAddress */
            
            /* Audio Streaming Isochronous Audio Data Endpoint Descriptor */
            0x07,    /* bLength */
            0x25,    /* bDescriptorType: CS_ENDPOINT */
            0x01,    /* bDescriptorSubtype: EP_GENERAL */
            0x01,    /* bmAttributes */
            0x01,    /* bLockDelayUnits */
            0x01,    /* wLockDelay */
            0x00,    /*  */

        /* HID Interface_2/0 Descriptor */
        0x09,    /* bLength */           
        0x04,    /* bDescriptorType */   
        0x02,    /* bInterfaceNumber */  
        0x00,    /* bAlternateSetting */ 
        0x00,    /* bNumEndpoints */     
        0x01,    /* bInterfaceClass: Audio */   
        0x02,    /* bInterfaceSubClass: Audio Streaming */
        0x00,    /* bInterfaceProtocol */
        0x00,    /* iConfiguration */    

        /* HID Interface_2/1 Descriptor */
        0x09,    /* bLength */           
        0x04,    /* bDescriptorType */   
        0x02,    /* bInterfaceNumber */  
        0x01,    /* bAlternateSetting */ 
        0x01,    /* bNumEndpoints */     
        0x01,    /* bInterfaceClass: Audio */   
        0x02,    /* bInterfaceSubClass: Audio Streaming */
        0x00,    /* bInterfaceProtocol */
        0x00,    /* iConfiguration */
    
            /* Class-Specific AS Interface Descriptor */
            0x07,    /* bLength */
            0x24,    /* bDescriptorType: CS_INTERFACE */
            0x01,    /* bDescriptorSubtype: AS_GENERAL */
            0x06,    /* bTerminalLink */
            0x00,    /* bDelay */
            0x01,    /* wFormatTag: PCM */
            0x00,

            /* Class-Specific AS Format Type Descriptor */
            0x14,    /* bLength */
            0x24,    /* bDescriptorType: CS_INTERFACE */
            0x02,    /* bDescriptorSubtype: FORMAT_TYPE */
            0x01,    /* bFormatType: FORMAT_TYPE_I */
            0x01,    /* bNrChannels */
            0x02,    /* bSubframeSize */
            0x10,    /* bBitResolution */
            0x04,    /* bSamFreqType */
            0x80,    /* tSamFreq [1] */
            0x3E,
            0x00,
            0x00,    /* tSamFreq [2] */
            0x7D,
            0x00,
            0x80,    /* tSamFreq [3] */
            0xBB,
            0x00,
            0x00,    /* tSamFreq [4] */
            0x77,
            0x01,

            /* Endpoint 1 Descriptor */
            0x09,    /* bLength */
            0x05,    /* bDescriptorType */
            0x81,    /* bEndpointAddress: IN 1 */
            0x0D,    /* bmAttributes: Isochronous */ 
            0x60,    /* wMaxPacketSize: 96byte */
            0x00,
            0x01,    /* bInterval */
            0x00,    /* bRefresh */
            0x00,    /* bSynchAddress */

            /* Audio Streaming Isochronous Audio Data Endpoint Descriptor */
            0x07,    /* bLength */
            0x25,    /* bDescriptorType: CS_ENDPOINT */
            0x01,    /* bDescriptorSubtype: EP_GENERAL */
            0x00,    /* bmAttributes */
            0x01,    /* bLockDelayUnits */
            0x01,    /* wLockDelay */
            0x00,

        /* HID Interface_3/0 Descriptor */
        0x09,    /* bLength */           
        0x04,    /* bDescriptorType */   
        0x03,    /* bInterfaceNumber */  
        0x00,    /* bAlternateSetting */ 
        0x01,    /* bNumEndpoints */     
        0x03,    /* bInterfaceClass: HID */
        0x00,    /* bInterfaceSubClass */
        0x00,    /* bInterfaceProtocol */
        0x00,    /* iConfiguration */

            /* HID Descriptor */
            0x09,    /* bLength */
            0x21,    /* bDescriptorType: HID */
            0x10,    /* bcdHID: 1.10 */
            0x01,    
            0x21,    /* bCountryCode */
            0x01,    /* bNumDescriptors */
            0x22,    /* bDescriptorType: Report */
            0x2A,    /* wDescriptorLength */
            0x00,

            /* Endpoint 2 Descriptor */
            0x07,    /* bLength */
            0x05,    /* bDescriptorType */
            0x82,    /* bEndpointAddress: IN 2 */
            0x03,    /* bmAttributes: Interrupt */ 
            0x02,    /* wMaxPacketSize: 2byte */
            0x00,
            0x14,    /* bInterval: 20ms */
};

/* USB Standard Manufacture Descriptor */
const uint8_t USB_Audio_ManufactureDesc[] =
{
    0x12,        /* bLength */    
    0x03,        /* bDescriptorType */    
    'F', 0x00,   /* BString */
    'R', 0x00,
    'E', 0x00,
    'Q', 0x00,
    'C', 0x00,
    'H', 0x00,
    'I', 0x00,
    'P', 0x00,
};

/* USB Standard Product Descriptor */
const uint8_t USB_Audio_ProductDesc[] =
{
    0x1E,         /* bLength */
    0x03,         /* bDescriptorType */
    'F', 0x00,    /* BString */
    'R', 0x00,    
    'E', 0x00,    
    'Q', 0x00,    
    'C', 0x00,    
    'H', 0x00,    
    'I', 0x00,    
    'P', 0x00,    
    '-', 0x00,    
    'A', 0x00,    
    'u', 0x00,    
    'd', 0x00,    
    'i', 0x00,    
    'o', 0x00,    
};

/* USB Standard SerialNumber Descriptor */
const uint8_t USB_Audio_SerialNumberDesc[] =
{
    0x1E,         /* bLength */
    0x03,         /* bDescriptorType */
    '2', 0x00,    /* BString */
    '0', 0x00,
    '2', 0x00,
    '1', 0x00,
    '-', 0x00,
    '1', 0x00,
    '0', 0x00,
    '0', 0x00,
    '1', 0x00,
    '-', 0x00,
    'B', 0x00,
    '1', 0x00,
    '3', 0x00,
    '5', 0x00,
};

/* USB Standard LanuageID Descriptor */
const uint8_t USB_Audio_LanuageIDDesc[] =
{
    0x04,    /* bLength */
    0x03,    /* bDescriptorType */
    0x09,    /* BString */
    0x04,
};

/* HID device report Descriptor */
uint8_t USB_HID_Audio_ReportDesc[] = 
{
    0x2A,          /* Length */

    0x06, 0x0C, 0x00,
    0x09, 0x01,
    0xA1, 0x01,
    0x25, 0x01,
    0x15, 0x00,
    0x0A, 0xE2, 0x00,    /* Mute                */
    0x0A, 0xEA, 0x00,    /* Volume Decrement    */
    0x0A, 0xE9, 0x00,    /* Volume Increment    */
    0x0A, 0xB6, 0x00,    /* Scan Previous Track */
    0x0A, 0xCD, 0x00,    /* Play/Pause          */
    0x0A, 0xB5, 0x00,    /* Scan Next Track     */
    0x0A, 0xB7, 0x00,    /* STOP                */
    0x0A, 0xB7, 0x00,    /* STOP                */
    0x75, 0x01,
    0x95, 0x08,
    0x81, 0x02,
    0xC0,
};

/*********************************************************************
 * @fn      usb_hid_set_mouse_report
 *
 * @brief   set report
 * 
 * @param   fu8_Value: reference @Audio_Report.
 *                     0x00: None.
 *                     0x01: Mute.
 *                     0x02: Volume decrease.
 *                     0x04: Volume increase.
 *                     0x08: Scan Previous Track.
 *                     0x10: Play/Pause.
 *                     0x20: Scan Next Track.
 *                     0x40: Stop.
 */
void usb_hid_set_Audio_report(uint8_t fu8_Value)
{
    gu8_AudioReport = fu8_Value;

    usb_selecet_endpoint(ENDPOINT_2);

    if (usb_Endpoints_GET_TxPkrRdy() == false) 
    {
        usb_write_fifo(ENDPOINT_2, &gu8_AudioReport, 1);
        usb_Endpoints_SET_TxPktRdy();
    }
}

/*********************************************************************
 * @fn      usb_Audio_SetControl
 *
 * @brief   Set Speaker volume¡¢            Set  microphone volume
 *          Set Speaker Mute¡¢              Set  microphone Mute
 *          Set Speaker sampling frequency¡¢Set  microphone sampling frequency
 */
static void usb_Audio_SetControl(void)
{
    uint8_t lu8_RxCount;

    lu8_RxCount = usb_Endpoint0_get_RxCount();

    switch (ge_CTLIndex)
    {
        case AUDIO_SET_VOL_SPEAKER:      usb_read_fifo(ENDPOINT_0, (uint8_t *)&gu16_VolumeSpeaker,       lu8_RxCount); break;
        case AUDIO_SET_VOL_MIC:          usb_read_fifo(ENDPOINT_0, (uint8_t *)&gu16_VolumeMic,           lu8_RxCount); break;
        case AUDIO_SET_MUTE_SPEAKER:     usb_read_fifo(ENDPOINT_0, (uint8_t *)&gu8_MuteSpeaker,          lu8_RxCount); break;
        case AUDIO_SET_MUTE_MIC:         usb_read_fifo(ENDPOINT_0, (uint8_t *)&gu8_MuteMic,              lu8_RxCount); break;
        case AUDIO_SET_SAMPLING_SPEAKER: usb_read_fifo(ENDPOINT_0, (uint8_t *)&gu32_SamplingFreqSpeaker, lu8_RxCount); break;
        case AUDIO_SET_SAMPLING_MIC:     usb_read_fifo(ENDPOINT_0, (uint8_t *)&gu32_SamplingFreqMic,     lu8_RxCount); break;
        
        default: break; 
    }

    usb_Endpoint0_FlushFIFO();

    Endpoint_0_DataOut_Handler = NULL;

    /* MCU handle */
    uint32_t lu32_TempValue;

    switch (ge_CTLIndex)
    {
        case AUDIO_SET_VOL_SPEAKER:
        {
            lu32_TempValue = (gu16_VolumeSpeaker * 0x3F) / 0xAF;
            *(uint32_t *)0x50022358 = lu32_TempValue;
            *(uint32_t *)0x5002235C = lu32_TempValue;
        }break;

        case AUDIO_SET_VOL_MIC:
        {
            lu32_TempValue = (gu16_VolumeMic * 0x3F) / 0xAF;
            *(uint32_t *)0x5002237C = lu32_TempValue;
        }break;

        case AUDIO_SET_MUTE_SPEAKER:
        {
            if (gu8_MuteSpeaker) 
            {
                *(uint32_t *)0x50022358 = 0;
                *(uint32_t *)0x5002235C = 0;
            }
            else 
            {
                lu32_TempValue = (gu16_VolumeSpeaker * 0x3F) / 0xAF;
                *(uint32_t *)0x50022358 = lu32_TempValue;
                *(uint32_t *)0x5002235C = lu32_TempValue;
            }
        }break;

        case AUDIO_SET_MUTE_MIC:
        {
            if (gu8_MuteMic) 
            {
                *(uint32_t *)0x5002237C = 0;
            }
            else 
            {
                lu32_TempValue = (gu16_VolumeMic * 0x3F) / 0xAF;
                *(uint32_t *)0x5002237C = lu32_TempValue;
            }
        }break;

        default: break; 
    }
}

/*********************************************************************
 * @fn      usb_Audio_ClassRequest_Handler
 *
 * @brief   Audio Class Request Handler
 *
 * @param   None.
 * @return  None.
 */
static void usb_Audio_ClassRequest_Handler(usb_StandardRequest_t* pStandardRequest, usb_ReturnData_t* pReturnData)
{
    if (pStandardRequest->bmRequestType & RECIPIENT_INTERFACE)
    {
        switch (pStandardRequest->bRequest)
        {
            case GET_CUR: 
            {
                if (pStandardRequest->wValue[1] == MUTE_CONTROL) 
                {
                    gu8_Respond[0] = 0x00;
                    
                    pReturnData->DataBuffer = gu8_Respond;
                    pReturnData->DataLength = 1;
                }
                else if (pStandardRequest->wValue[1] == VOLUME_CONTROL)
                {
                    if (pStandardRequest->wIndex[1] == 0x02) 
                    {
                        gu8_Respond[0] = 0xAF;
                        gu8_Respond[1] = 0x00;
                    }
                    else if (pStandardRequest->wIndex[1] == 0x05) 
                    {
                        gu8_Respond[0] = 0x7F;
                        gu8_Respond[1] = 0x00;
                    }

                    pReturnData->DataBuffer = gu8_Respond;
                    pReturnData->DataLength = 2;
                }
            }break;

            case GET_MIN:
            {
                if (pStandardRequest->wValue[1] == VOLUME_CONTROL) 
                {
                    gu8_Respond[0] = 0x00;
                    gu8_Respond[1] = 0x00;
                     
                    pReturnData->DataBuffer = gu8_Respond;
                    pReturnData->DataLength = 2;
                }
            }break;
            
            case GET_MAX:
            {
                if (pStandardRequest->wValue[1] == VOLUME_CONTROL) 
                {
                    gu8_Respond[0] = 0xAF;
                    gu8_Respond[1] = 0x00;

                    pReturnData->DataBuffer = gu8_Respond;
                    pReturnData->DataLength = 2;
                }
            }break;

            case GET_RES:
            {
                if (pStandardRequest->wValue[1] == VOLUME_CONTROL) 
                {
                    gu8_Respond[0] = 0x01;
                    gu8_Respond[1] = 0x00;
                     
                    pReturnData->DataBuffer = gu8_Respond;
                    pReturnData->DataLength = 2;
                }
            }break;

            case SET_CUR:
            {
                if (pStandardRequest->wValue[1] == VOLUME_CONTROL) 
                {
                    /* Speaker */
                    if (pStandardRequest->wIndex[1] == 0x02) 
                    {
                        ge_CTLIndex = AUDIO_SET_VOL_SPEAKER;
                    }
                    /* microphone */
                    else if (pStandardRequest->wIndex[1] == 0x05) 
                    {
                        ge_CTLIndex = AUDIO_SET_VOL_MIC;
                    }

                    Endpoint_0_DataOut_Handler = usb_Audio_SetControl;
                }
                else if (pStandardRequest->wValue[1] == MUTE_CONTROL) 
                {
                    /* Speaker */
                    if (pStandardRequest->wIndex[1] == 0x02) 
                    {
                        ge_CTLIndex = AUDIO_SET_MUTE_SPEAKER;
                    }
                    /* microphone */
                    else if (pStandardRequest->wIndex[1] == 0x05) 
                    {
                        ge_CTLIndex = AUDIO_SET_MUTE_MIC;
                    }

                    Endpoint_0_DataOut_Handler = usb_Audio_SetControl;
                }
            }

            default: break; 
        }
    }
    else if (pStandardRequest->bmRequestType & RECIPIENT_ENDPOINT) 
    {
        switch (pStandardRequest->bRequest)
        {
            case SET_CUR: 
            {
                if (pStandardRequest->wValue[1] == SAMPLING_FREQ_CONTROL) 
                {
                    /* IN Endpoint */
                    if (pStandardRequest->wIndex[0] == 0x81) 
                    {
                        ge_CTLIndex = AUDIO_SET_SAMPLING_MIC;
                    }
                    /* OUT Endpoint */
                    else if (pStandardRequest->wIndex[0] == 0x02) 
                    {
                        ge_CTLIndex = AUDIO_SET_SAMPLING_SPEAKER;
                    }

                    Endpoint_0_DataOut_Handler = usb_Audio_SetControl;
                }
            }break;

            default: break; 
        }
    }
}

/*********************************************************************
 * @fn      usb_Audio_Endpoints_Handler
 *
 * @brief   Audio Endpoints Handler
 *
 */
void usb_Audio_Endpoints_Handler(uint8_t RxStatus, uint8_t TxStatus)
{
#if USB_AUDIO_ENABLE == 1
	uint32_t lu32_RxCount;
	
    /* ENDPOINT_2 Rx */
    if (RxStatus & 0x04) 
    {
        usb_selecet_endpoint(ENDPOINT_2);

        if (usb_Endpoints_GET_RxPktRdy())
        {
            lu32_RxCount = usb_Endpoints_get_RxCount();

            usb_read_fifo(ENDPOINT_2, (uint8_t *)&Speaker_Buffer[48 * Speaker_Packet], lu32_RxCount); 

            usb_Endpoints_FlushRxFIFO();

            Speaker_Packet += 1;

            if (Speaker_Packet >= 10) 
            {
                Speaker_Packet = 0;
            }
			Speaker_RxCount = Speaker_Packet * 48;

            if (Speaker_Packet >= 5 && Speaker_Start == false) 
            {
                Speaker_Start = true;
                
                /* enable dac fifo and apb function, almost empty interrupt */
                *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0xD0;
            }
        }
    }

    /* ENDPOINT_1 Tx */
    if (TxStatus & 0x02)
    {
        if (MIC_Start) 
        {
            usb_selecet_endpoint(ENDPOINT_1);

            if (usb_Endpoints_GET_TxPkrRdy() == false) 
            {
                usb_write_fifo(ENDPOINT_1, (uint8_t *)&MIC_Buffer[MIC_TxCount], 96);
                usb_Endpoints_SET_TxPktRdy();

                MIC_TxCount += 48;
                
                if (MIC_TxCount >= 480) 
                {
                    MIC_TxCount = 0;
                }
			}
        }
    }
#endif  // USB_AUDIO_ENABLE == 1
}

/*********************************************************************
 * @fn      usb_audio_init
 *
 * @brief   audio device parameter initialization 
 *
 * @param   None.
 * @return  None.
 */
void usb_audio_init(void)
{
    /* Initialize the relevant pointer */
    usbdev_get_dev_desc((uint8_t *)USB_Audio_DeviceDesc);

    usbdev_get_config_desc((uint8_t *)USB_Audio_ConfigurationDesc);
    usbdev_get_hidreport_desc(3, (uint8_t *)USB_HID_Audio_ReportDesc);
    usbdev_get_string_Manufacture((uint8_t *)USB_Audio_ManufactureDesc);
    usbdev_get_string_Product((uint8_t *)USB_Audio_ProductDesc);
    usbdev_get_string_SerialNumber((uint8_t *)USB_Audio_SerialNumberDesc);
    usbdev_get_string_LanuageID((uint8_t *)USB_Audio_LanuageIDDesc);

    Endpoint_0_ClassRequest_Handler = usb_Audio_ClassRequest_Handler;

    Endpoints_Handler = usb_Audio_Endpoints_Handler;

    USB_Reset_Handler = usb_audio_init;

    /* config data endpoint fifo */
    usb_selecet_endpoint(ENDPOINT_1);
    usb_TxSyncEndpoint_enable();
    usb_endpoint_Txfifo_config(0x08, 4);    /* 128 Byte, 64  ~ 191 */
    usb_TxMaxP_set(16);

    usb_selecet_endpoint(ENDPOINT_2);
    usb_RxSyncEndpoint_enable();
    usb_endpoint_Rxfifo_config(0x18, 5);    /* 256 Byte, 192 ~ 447 */
    usb_RxMaxP_set(32);
    usb_endpoint_Txfifo_config(0x38, 1);    /* 16 Byte,  448 ~ 464 */
    usb_TxMaxP_set(2);

    usb_TxInt_Enable(ENDPOINT_1);
    usb_RxInt_Enable(ENDPOINT_2);
}

/*
    How to use, for example:

volatile uint16_t MIC_Buffer[480];
volatile uint32_t MIC_Packet;
volatile uint32_t MIC_RxCount;
volatile uint32_t MIC_TxCount;
volatile bool     MIC_Start;

volatile uint32_t Speaker_Buffer[480];
volatile uint32_t Speaker_Packet;
volatile uint32_t Speaker_RxCount;
volatile uint32_t Speaker_TxCount;
volatile bool     Speaker_Start;

void main()
{
    NVIC_ClearPendingIRQ(USBMCU_IRQn);
    NVIC_SetPriority(USBMCU_IRQn, 0);
    NVIC_EnableIRQ(USBMCU_IRQn);

    usb_device_init();
    usb_audio_init();

    // Wait for other initialization of the MCU

    usb_DP_Pullup_Enable();

    while(1)
    {
        if (digital_codec_reg->adcff_cfg1.afull_status)
        {
            MIC_RxCount = MIC_Packet * 48;

            for (i = 0; i < 48; i++)
            {
                MIC_Buffer[MIC_RxCount + i] = digital_codec_reg->adcff_data;
            }

            MIC_Packet += 1;

            if (MIC_Packet >= 10) 
            {
                MIC_Packet = 0;
            }

            if (MIC_Packet >= 5 && MIC_Start == false) 
            {
                usb_selecet_endpoint(ENDPOINT_1);
                
                // start the frist packet transmission
                if (usb_Endpoints_GET_TxPkrRdy() == false) 
                {
                    usb_write_fifo(ENDPOINT_1, (uint8_t *)&MIC_Buffer[MIC_TxCount], 96);
                    usb_Endpoints_SET_TxPktRdy();

                    MIC_TxCount += 48;

                     MIC_Start = true;
                }
                // start fail
                else 
                {
                    MIC_Packet  = 0;
                    MIC_RxCount = 0;
                    MIC_TxCount = 0;
                    MIC_Start   = false;
                }
            }

            if (MIC_Start) 
            {
                if (MIC_RxCount == MIC_TxCount) 
                {
                    MIC_Packet  = 0;
                    MIC_RxCount = 0;
                    MIC_TxCount = 0;
                    MIC_Start   = false;
                }
            }
	    }

        if (Send_HID_Report)
        {
            usb_selecet_endpoint(ENDPOINT_2);

            if (usb_Endpoints_GET_TxPkrRdy() == false) 
            {
                usb_write_fifo(ENDPOINT_2, &gu8_AudioReport, 1);
                usb_Endpoints_SET_TxPktRdy();
            }
        }

        if (Speaker_Start) 
        {
            if (digital_codec_reg->dacff_cfg1.aept_status)
            {
                for (i = 0; i < 48; i++)
                {
                    digital_codec_reg->dacff_data = Speaker_Buffer[Speaker_TxCount++];
                }

                if (Speaker_TxCount >= 480) 
                {
                    Speaker_TxCount = 0;
                }

                if (Speaker_TxCount == Speaker_RxCount) 
                {
                    Speaker_Packet  = 0;
                    Speaker_RxCount = 0;
                    Speaker_TxCount = 0;
                    Speaker_Start   = false;
                }
            }
        }
    }
}

*/
