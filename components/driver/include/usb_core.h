/*
  ******************************************************************************
  * @file    usb_core.h
  * @author  FreqChip Firmware Team
  * @version V1.1.0
  * @date    2021
  * @brief   Header file for usb_core.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#ifndef __USB_CORE_H__
#define __USB_CORE_H__

#include "stdint.h"
#include "stdbool.h"

/* Control Register for USB */
typedef struct
{
    volatile uint32_t USB_PHY_DAP_CFG : 8;
    volatile uint32_t OTG_VBUS_VAL_I  : 1;
    volatile uint32_t OTG_VBUS_SES_I  : 1;
    volatile uint32_t OTG_VBUSLO_I    : 1;
    volatile uint32_t OTG_CID_I       : 1;
    volatile uint32_t USB_PHY_SEL     : 1;
    volatile uint32_t USB_UART_EN     : 1;
    volatile uint32_t USB_STDBY_SEL   : 1;
    volatile uint32_t USB_STDBY_REG   : 1;
    volatile uint32_t USB_IO_PS0      : 1;
    volatile uint32_t USB_IO_PS1      : 1;  
    volatile uint32_t USB_IO_PE0      : 1;
    volatile uint32_t USB_IO_PE1      : 1;
    volatile uint32_t USB_UART_SEL    : 1;
    volatile uint32_t rsv_1           : 11;
}REG_USB_OTG_CTRL_t;

#define USB_OTG_CTRL_BASE    (0x5000003C)

#define USB_OTG_CTRL         ((volatile REG_USB_OTG_CTRL_t *)USB_OTG_CTRL_BASE)

/** @addtogroup DMA_Registers_Section
  * @{
  */
/* ################################ Register bit define Start ################################ */

#define USB_INT_STATUS_VBUS_ERROR        (0x80)
#define USB_INT_STATUS_SESSREQ           (0x40)
#define USB_INT_STATUS_DISCON            (0x20)
#define USB_INT_STATUS_CONN              (0x10)
#define USB_INT_STATUS_SOF               (0x08)
#define USB_INT_STATUS_RESET             (0x04)
#define USB_INT_STATUS_RESUME            (0x02)
#define USB_INT_STATUS_SUSPEND           (0x01)

#define USB_CSR01_SVCSETUPEND            (0x80)
#define USB_CSR01_SVCRXPKTRDY            (0x40)
#define USB_CSR01_SENDSTALL              (0x20)
#define USB_CSR01_SETUPEND               (0x10)
#define USB_CSR01_DATAEND                (0x08)
#define USB_CSR01_SENTSTALL              (0x04)
#define USB_CSR01_TXPKTRDY               (0x02)
#define USB_CSR01_RXPKTRDY               (0x01)

#define USB_TXCSR1_CLRDATATOG            (0x40)
#define USB_TXCSR1_SENTSTALL             (0x20)
#define USB_TXCSR1_SENDSTALL             (0x10)
#define USB_TXCSR1_FLUSHFIFO             (0x08)
#define USB_TXCSR1_UNDERRUN              (0x04)
#define USB_TXCSR1_FIFO_NOTEMPTY         (0x02)
#define USB_TXCSR1_TXPKTRDY              (0x01)

#define USB_TXCSR2_DMA_MODE              (0x04)
#define USB_TXCSR2_FrcDataTog            (0x08)
#define USB_TXCSR2_DMA_ENABLE            (0x10)
#define USB_TXCSR2_MODE                  (0x20)
#define USB_TXCSR2_ISO                   (0x40)
#define USB_TXCSR2_AUTO_SET              (0x80)

#define USB_RXCSR1_CLRDATATOG            (0x80)
#define USB_RXCSR1_SENTSTALL             (0x40)
#define USB_RXCSR1_SENDSTALL             (0x20)
#define USB_RXCSR1_FLUSHFIFO             (0x10)
#define USB_RXCSR1_DATAERROR             (0x08)
#define USB_RXCSR1_OVERRUN               (0x04)
#define USB_RXCSR1_FIFO_FULL             (0x02)
#define USB_RXCSR1_RXPKTRDY              (0x01)

#define USB_RXCSR2_DMA_MODE              (0x10)
#define USB_RXCSR2_DMA_ENABLE            (0x20)
#define USB_RXCSR2_ISO                   (0x40)
#define USB_RXCSR2_AUTO_CLEAR            (0x80)

/* ################################ Register bit define END ################################ */
/**
  * @}
  */
  
typedef enum
{
    ENDPOINT_0,
    ENDPOINT_1,
    ENDPOINT_2,
    ENDPOINT_3,
    ENDPOINT_4,
    ENDPOINT_5,
}enum_Endpoint_t;

/** @addtogroup DMA_Registers_Section
  * @{
  */
/* ################################ Register Section Start ################################ */

/* -------------------------------------------*/
/*            Common USB registers            */
/* -------------------------------------------*/
typedef struct 
{
    volatile uint8_t FAddr;         // offset 0x00
    volatile uint8_t Power;         // offset 0x01
    volatile uint8_t IntrTx1;       // offset 0x02
    volatile uint8_t IntrTx2;       // offset 0x03
    volatile uint8_t IntrRx1;       // offset 0x04
    volatile uint8_t IntrRx2;       // offset 0x05
    volatile uint8_t IntrUSB;       // offset 0x06
    volatile uint8_t IntrTx1E;      // offset 0x07
    volatile uint8_t IntrTx2E;      // offset 0x08
    volatile uint8_t IntrRx1E;      // offset 0x09
    volatile uint8_t IntrRx2E;      // offset 0x0A
    volatile uint8_t IntrUSBE;      // offset 0x0B
    volatile uint8_t Frame1;        // offset 0x0C
    volatile uint8_t Frame2;        // offset 0x0D
    volatile uint8_t Index;         // offset 0x0E
    volatile uint8_t DevCtl;        // offset 0x0F
}usb_common_t;

/* ------------------------------------------------*/
/*      Control Status registers for endpoint0     */
/* ------------------------------------------------*/
typedef struct 
{
    volatile uint8_t rsv_0;         // offset 0x10
    volatile uint8_t CSR01;         // offset 0x11
    volatile uint8_t CSR02;         // offset 0x12
    volatile uint8_t rsv_1[3];      // offset 0x13 ~ 0x15
    volatile uint8_t Count0;        // offset 0x16
    volatile uint8_t rsv_2[9];      // offset 0x17 ~ 0x1F
    volatile uint8_t FIFO;          // offset 0x20
}usb_endpoint0_t;

/* ----------------------------------------------------*/
/*      Control Status registers for endpoint1 ~ 5     */
/* ----------------------------------------------------*/
typedef struct 
{
    volatile uint8_t TxMaxP;         // offset 0x10
    volatile uint8_t TxCSR1;         // offset 0x11
    volatile uint8_t TxCSR2;         // offset 0x12
    volatile uint8_t RxMaxP;         // offset 0x13
    volatile uint8_t RxCSR1;         // offset 0x14
    volatile uint8_t RxCSR2;         // offset 0x15
    volatile uint8_t RxCount1;       // offset 0x16
    volatile uint8_t RxCount2;       // offset 0x17
    volatile uint8_t rsv_0[4];       // offset 0x18 ~ 0x1B
    volatile uint8_t TxFIFO1;        // offset 0x1C
    volatile uint8_t TxFIFO2;        // offset 0x1D
    volatile uint8_t RxFIFO1;        // offset 0x1E
    volatile uint8_t RxFIFO2;        // offset 0x1F
    volatile uint8_t FIFO[24];       // offset 0x20
}usb_endpoint1_5_t;


#define USB_BASE             (0x40020000)
#define USB_ENDPOINT_BASE    (0x40020010)

#define USB              ((volatile usb_common_t *)USB_BASE)
#define USB_POINT0       ((volatile usb_endpoint0_t *)USB_ENDPOINT_BASE)
#define USB_POINT1_5     ((volatile usb_endpoint1_5_t *)USB_ENDPOINT_BASE)


/* ################################ Register Section END ################################ */
/**
  * @}
  */



/* Exported functions --------------------------------------------------------*/

/* usb_device_init */
void usb_device_init(void);

/* usb_selecet_endpoint */
void usb_selecet_endpoint(enum_Endpoint_t Endpoint);

/* usb_get_endpoint */
uint8_t usb_get_endpoint(void);

/* usb_set_address */
void usb_set_address(uint8_t address);

/* usb_get_frame */
uint32_t usb_get_frame(void);

/* usb_RxSyncEndpoint_enable */
/* usb_TxSyncEndpoint_enable */
void usb_RxSyncEndpoint_enable(void);
void usb_TxSyncEndpoint_enable(void);

/* usb_TxInt_Enable、usb_TxInt_Disable */
/* usb_RxInt_Enable、usb_RxInt_Disable */
void usb_TxInt_Enable(enum_Endpoint_t Endpoint);
void usb_TxInt_Disable(enum_Endpoint_t Endpoint);
void usb_RxInt_Enable(enum_Endpoint_t Endpoint);
void usb_RxInt_Disable(enum_Endpoint_t Endpoint);

/* usb_endpoint_Txfifo_config */
/* usb_endpoint_Rxfifo_config */
void usb_endpoint_Txfifo_config(uint32_t StartAddress, uint32_t MaxPacket);
void usb_endpoint_Rxfifo_config(uint32_t StartAddress, uint32_t MaxPacket);

/* usb_TxMaxP_set */
/* usb_RxMaxP_set */
void usb_TxMaxP_set(uint32_t MaxPacket);
void usb_RxMaxP_set(uint32_t MaxPacket);

/* usb_write_fifo */
/* usb_read_fifo  */
void usb_write_fifo(enum_Endpoint_t Endpoint, uint8_t *Buffer, uint32_t Size);
void usb_read_fifo(enum_Endpoint_t Endpoint, uint8_t *Buffer, uint32_t Size);

/* usb_DP_Pullup_Enable */
/* usb_DP_Pullup_Disable */
void usb_DP_Pullup_Enable(void);
void usb_DP_Pullup_Disable(void);


/* Exported inline functions --------------------------------------------------------*/

/* usb_get_USBStatus */
static inline uint8_t usb_get_USBStatus(void)
{
    return USB->IntrUSB;
}

/* usb_get_TxStatus */
/* usb_get_RxStatus */
static inline uint8_t usb_get_TxStatus(void)
{
    return USB->IntrTx1;
}
static inline uint8_t usb_get_RxStatus(void)
{
    return USB->IntrRx1;
}

/* ---------------------------- */
/* ---- Endpoint0 function ---- */
/* ---------------------------- */ 
/* usb_get_Endpoint0_RxCount */
/* usb_get_Endpoint0_FlushFIFO */
/* usb_Endpoint0_SendStall */
/* usb_Endpoint0_DataEnd */
/* usb_Endpoint0_SET_TxPktRdy */
static inline uint8_t usb_Endpoint0_get_RxCount(void)
{
    return USB_POINT0->Count0;
}

static inline void usb_Endpoint0_FlushFIFO(void)
{
    USB_POINT0->CSR02 = 1;
}

static inline void usb_Endpoint0_SendStall(void)
{
    USB_POINT0->CSR01 |= USB_CSR01_SENDSTALL;
}

static inline void usb_Endpoint0_DataEnd(void)
{
    USB_POINT0->CSR01 |= USB_CSR01_DATAEND;
}

static inline void usb_Endpoint0_SET_TxPktRdy(void)
{
    USB_POINT0->CSR01 |= USB_CSR01_TXPKTRDY;
}

static inline bool usb_Endpoint0_GET_TxPktRdy(void)
{
    return (USB_POINT0->CSR01 & USB_CSR01_TXPKTRDY) ? true : false;
}

static inline bool usb_Endpoint0_GET_RxPktRdy(void)
{
    return (USB_POINT0->CSR01 & USB_CSR01_RXPKTRDY) ? true : false;
}


/* ---------------------------------- */
/* ---- other Endpoints function ---- */
/* ---------------------------------- */ 

/* usb_Endpoints_GET_RxPktRdy */
/* usb_Endpoints_SET_TxPktRdy */
/* usb_Endpoints_GET_TxPkrRdy */
/* usb_Endpoints_FlushRxFIFO */
/* usb_Endpoints_FlushTxFIFO */
/* usb_Endpoints_get_RxCount */
static inline bool usb_Endpoints_GET_RxPktRdy(void)
{
    return (USB_POINT1_5->RxCSR1 & USB_RXCSR1_RXPKTRDY) ? true : false;
}

static inline void usb_Endpoints_SET_TxPktRdy(void)
{
    USB_POINT1_5->TxCSR1 |= USB_TXCSR1_TXPKTRDY;
}

static inline bool usb_Endpoints_GET_TxPkrRdy(void)
{
    return (USB_POINT1_5->TxCSR1 & USB_TXCSR1_TXPKTRDY) ? true : false;
}

static inline void usb_Endpoints_FlushRxFIFO(void)
{
    USB_POINT1_5->RxCSR1 |= USB_RXCSR1_FLUSHFIFO;
}

static inline void usb_Endpoints_FlushTxFIFO(void)
{
    USB_POINT1_5->TxCSR1 |= USB_TXCSR1_FLUSHFIFO;
}

static inline uint8_t usb_Endpoints_get_RxCount(void)
{
    return USB_POINT1_5->RxCount1;
}

#endif
