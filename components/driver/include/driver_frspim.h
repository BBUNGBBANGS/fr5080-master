/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */
#ifndef _DRIVER_FRSPIM_H_
#define _DRIVER_FRSPIM_H_

#include <stdint.h>

#define FR_SPI_PMU_CHAN             0

/*********************************************************************
* @fn      frspim_init
*
* @brief   initialize internal frspim module.
*
* @param   ratio    - clock dividor.
*
* @return  None.
*/
void frspim_init(uint8_t ratio);

/*********************************************************************
* @fn      frspim_rd
*
* @brief   read data from frspim module.
*
* @param   chan_num - indicate which internal module is read. Such as
*                     FR_SPI_RF_COB_CHAN.
*          addr     - which register is read
*          len      - this parameter should be 1, 2, 4
*
* @return  return read value.
*/
uint32_t frspim_rd (uint8_t chan_num, uint8_t addr, uint8_t len);

/*********************************************************************
* @fn      frspim_rd
*
* @brief   write data to frspim module.
*
* @param   chan_num - indicate which internal module is read. Such as
*                     FR_SPI_RF_COB_CHAN.
*          addr     - which register is written
*          len      - this parameter should be 1, 2, 4
*          val      - data to be written
*
* @return  None.
*/
void frspim_wr (uint8_t chan_num, uint8_t addr, uint8_t len, uint32_t val);

#endif
