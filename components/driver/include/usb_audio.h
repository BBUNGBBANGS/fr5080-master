/*
  ******************************************************************************
  * @file    usb_audio.h
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   Header file for usb_audio.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#ifndef __USB_AUDIO_H__
#define __USB_AUDIO_H__

#include "stdint.h"
#include "stdbool.h"
#include "string.h"

#include "usb_core.h"
#include "usb_dev.h"
#include "driver_codec.h"

/* Audio Class Specific Request Codes */
typedef enum
{
    SET_CUR  = 0x01,
    GET_CUR  = 0x81,
    SET_MIN  = 0x02,
    GET_MIN  = 0x82,
    SET_MAX  = 0x03,
    GET_MAX  = 0x83,
    SET_RES  = 0x04,
    GET_RES  = 0x84,
    SET_MEM  = 0x05,
    GET_MEM  = 0x85,
    GET_STAT = 0xFF,
}Audio_Class_Specific_Request_Codes;

/* Feature Unit Control Selectors */
typedef enum
{
    FU_CONTROL_UNDEFINED      = 0x00,
    MUTE_CONTROL              = 0x01,
    VOLUME_CONTROL            = 0x02,
    BASS_CONTROL              = 0x03,
    MID_CONTROL               = 0x04,
    TREBLE_CONTROL            = 0x05,
    GRAPHIC_EQUALIZER_CONTROL = 0x06,
    AUTOMATIC_GAIN_CONTROL    = 0x07,
    DELAY_CONTROL             = 0x08,
    BASS_BOOST_CONTROL        = 0x09,
    LOUDNESS_CONTROL          = 0x0A,
}Feature_Unit_Control_Selectors;

/* Endpoint Control Selectors */
typedef enum
{
    EP_CONTROL_UNDEFINED  = 0x00,
    SAMPLING_FREQ_CONTROL = 0x01,
    PITCH_CONTROL         = 0x02,
}Endpoint_Control_Selectors;

/* Audio report */
typedef enum
{
    AUDIO_REPORT_MUTE       = 0x01,
    AUDIO_REPORT_VOL_DEC    = 0x02,
    AUDIO_REPORT_VOL_INC    = 0x04,
    AUDIO_REPORT_PREVIOUS   = 0x08,
    AUDIO_REPORT_PLAY_PAUSE = 0x10,
    AUDIO_REPORT_NEXT       = 0x20,
    AUDIO_REPORT_STOP       = 0x40,
    AUDIO_REPORT_RSV        = 0x80,
}Audio_Report;

/* Exported inline functions --------------------------------------------------------*/

/* usb_audio_init */
void usb_audio_init(void);

/* usb_hid_set_Audio_report */
void usb_hid_set_Audio_report(uint8_t fu8_Value);

#endif
