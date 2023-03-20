/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 * NOTE: This driver is only for exteral spi_flash operation through ssp interface.
 *
 */
#ifndef _DRIVER_FLASH_SSP_H
#define _DRIVER_FLASH_SSP_H

#include <stdint.h>               // standard integer functions

#include "driver_ssp.h"

/**
 ****************************************************************************************
 * @addtogroup FLASH
 * @ingroup DRIVERS
 *
 * @brief Flash memory driver
 *
 * @{
 ****************************************************************************************
 */

/*
 * DEFINES
 ****************************************************************************************
 */

///Flash type code used to select the correct erasing and programming algorithm
#define FLASH_TYPE_UNKNOWN             0
#define FLASH_TYPE_INTEL_28F320C3      1
#define FLASH_TYPE_INTEL_28F800C3      2
#define FLASH_TYPE_NUMONYX_M25P128     3

///Base address of Flash on system bus
#define FLASH_BASE_ADDR          0x03000000

#define FLASH_READ_DEVICE_ID            0x90
#define FLASH_READ_IDENTIFICATION       0x9F

#define FLASH_AAI_PROGRAM_OPCODE        0xAF
#define FLASH_PAGE_PROGRAM_OPCODE       0x02
#define FLASH_READ_OPCODE               0x03
#define FLASH_FAST_READ_OPCODE          0x0B

#define FLASH_CHIP_ERASE_OPCODE         0x60
#define FLASH_SECTORE_ERASE_OPCODE      0x20
#define FLASH_BLOCK_32K_ERASE_OPCODE    0x52
#define FLASH_BLOCK_64K_ERASE_OPCODE    0xD8
#define FLASH_ST_SECTORE_ERASE_OPCODE   0xD8
#define FLASH_ST_BULK_ERASE_OPCODE      0xC7

#define FLASH_SEC_REG_ERASE_OPCODE      0x44
#define FLASH_SEC_REG_PROGRAM_OPCODE    0x42
#define FLASH_SEC_REG_READ_OPCODE      	0x48


#define FLASH_WRITE_DISABLE_OPCODE      0x04
#define FLASH_WRITE_ENABLE_OPCODE       0x06
#define FLASH_WRITE_STATUS_REG_OPCODE   0x01
#define FLASH_READ_STATUS_REG_OPCODE    0x05
#define FLASH_READ_STATUS_REG2_OPCODE   0x35


#define FLASH_ST_ID                     0x20
#define FLASH_SST_ID                    0xBF

/*
 * FUNCTION DECLARATIONS
 ****************************************************************************************
 */
 
uint32_t ssp_flash_init(void);
uint8_t ssp_flash_identify(uint8_t* id);
uint8_t ssp_flash_erase(uint32_t offset, uint32_t size);
uint8_t ssp_flash_chip_erase(void);
uint8_t ssp_flash_write(uint32_t offset, uint32_t length, uint8_t *buffer);
uint8_t ssp_flash_read(uint32_t offset, uint32_t length, uint8_t *buffer);
void ssp_flash_protect_enable(void);
void ssp_flash_protect_disable(void);


#endif // _DRIVER_FLASH_SSP_H
