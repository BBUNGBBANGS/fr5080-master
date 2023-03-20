#ifndef _USER_CONFIG_H_
#define _USER_CONFIG_H_


///mp3 audio source,1---mp3 from mp3_sample.h, 0--- from external mcu 
#define MP3_SRC_LOCAL       1
    #define MP3_SRC_SDCARD  1

///dsp work in xip mode --- 1，dsp load code from m3  --- 0
#define DSP_USE_XIP         1
    /* these pins have to be remap to DSP for QSPI usage in XIP mode */
    #define FR5086D_V1_0        1
    #define FR5082DM_V1_0       0
    #define FR5086_JET          0
    #define FR5086D_V2_0        0
    #define FR5088D_V1_0        0
    #define FR5086DQ            0

///ble test mode(HCI) enable --- 1, disable ---- 0
#define BLE_TEST_MODE       0

//enable fixed frequency transmit mode
#define FIXED_FREQ_TX_TEST  0

//bt test mode
#define BT_TEST_MODE        0

//enable usb to download songs
#define USB_DEVICE_ENABLE   0

//enable usb audio class 
#define USB_AUDIO_ENABLE    0

//enable sd card
#define SD_DEVICE_ENABLE    1

//enable uart as interface for exchanging msg between mcu and coprocessor,TBD!!!!!!!!!!
#define COPROCESSOR_UART_ENABLE    0

//enable mic loop mode(DSP works),adc --> dsp --> dac
#define DSP_MIC_LOOP     0

//enable pbap
#define PBAP_FUC         1

//enable hid
#define HID_ENABLE      0
#endif
