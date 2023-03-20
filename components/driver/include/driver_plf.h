/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */
#ifndef _DRIVER_PLF_H_
#define _DRIVER_PLF_H_

#define QSPI_DAC_ADDRESS    0x01000000
#define SYSTEM_REG_BASE     0x50000000
#define TIMER_BASE       	0x50010000
#define EFUSE_BASE			0x50020000
#define MDM_BASE			0x50021000
#define CODEC_BASE			0x50022000
#define CODEC_ANALOG_BASE   0x50022340
#define SSP_BASE			0x50030000
#define FLASH_CACHE_BASE	0x50031000
#define QSPI_BASE			0x50032000
#define I2C0_BASE           0x50040000
#define I2C1_BASE           0x50041000
#define UART_BASE         	0x50050000
#define GPIO_BASE			0x50060000
#define I2S_BASE			0x50070000
#define FRSPIM_BASE			0x50080000
#define TWS_BASE            0x50088000
#define AUXADC_BASE			0x50090000
#define PWM_BASE			0x500A0000
#define IPC_BASE			0x500B0000
#define SBC_BASE			0x500C0000
#define SDC_BASE			0x500D0000
#define PDM0_BASE	        0x500e0000
#define PDM1_BASE   	    0x500e1000


/*
 * ==========================================================================
 * ---------- Interrupt Number Definition -----------------------------------
 * ==========================================================================
 */

typedef enum IRQn
{
    /******  Cortex-M3 Processor Exceptions Numbers ***************************************************/
    NonMaskableInt_IRQn           = -14,    /*!< 2 Non Maskable Interrupt                           */
    HardFault_IRQn                = -13,    /*!< 3 Cortex-M3 Hard Fault Interrupt                   */
    SVCall_IRQn                   = -5,     /*!< 11 Cortex-M3 SV Call Interrupt                     */
    PendSV_IRQn                   = -2,     /*!< 14 Cortex-M3 Pend SV Interrupt                     */
    SysTick_IRQn                  = -1,     /*!< 15 Cortex-M3 System Tick Interrupt                 */

    /******  CMSDK Specific Interrupt Numbers *******************************************************/
    RWIP_IRQn                      = 0,
    RWBT_IRQn                   = 1,
    RWBLE_IRQn                   = 2,
    
    BT_AUDIO0_IRQn              =3,
    BT_AUDIO1_IRQn              =4,
    USBMCU_IRQn                 =5,
    SDC_IRQn                    =6,
    RESV_IRQn                   =7,
    

    TIMER0_IRQn                   = 8,
    TIMER1_IRQn                   = 9,
    IIC0_IRQn                     = 10,
    IIC1_IRQn                     = 11,
    
    UART_IRQn                     = 12,
    GPIO_IRQn                     = 13,
    I2S_IRQn                      = 14,
    AUXADC_IRQn                   = 15,
    PMU_IRQn                      = 16,
    
    PWM0_IRQn                     = 17,
    PWM1_IRQn                     = 18,
    PWM2_IRQn                     = 19,
    PWM3_IRQn                     = 20,
    PWM4_IRQn                     = 21,
    PWM5_IRQn                     = 22,

    USBDMA_IRQn                   = 23,
    IPC_IRQn                      = 24,
    SSP_IRQn                      = 25,
	SBC_IRQn					  = 26,

	DMA_IRQn                      = 27,
	CDC_IRQn                      = 28,
    PDM0_IRQn                     = 29,
    PDM1_IRQn                     = 30,
	TWS_IRQn                      = 31,

} IRQn_Type;

/* Configuration of the Cortex-M3 Processor and Core Peripherals */
#define __MPU_PRESENT             0         /*!< MPU present or not                               */
#define __NVIC_PRIO_BITS          3         /*!< Number of Bits used for Priority Levels          */
#define __Vendor_SysTickConfig    0         /*!< Set to 1 if different SysTick Config is used     */

/* Macro to read a platform register */
#define REG_PL_RD(addr)              (*(volatile uint32_t *)(addr))
#define REG_PL_RD8(addr)             (*(volatile uint8_t *)(addr))

/* Macro to write a platform register */
#define REG_PL_WR(addr, value)       (*(volatile uint32_t *)(addr)) = (value)
#define REG_PL_WR8(addr, value)      (*(volatile uint8_t *)(addr)) = (value)

/* definations and functions relatived MCU dependence */
#include "core_cm3.h"
#include "ll.h"


#endif

