/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */
#ifndef _DRIVER_EFUSE_H
#define _DRIVER_EFUSE_H

/*
 * INCLUDES (包含头文件)
 */
#include <stdint.h>

struct	chip_unique_id_t {
	uint8_t unique_id[6];
};

/*********************************************************************
 * @fn      efuse_read
 *
 * @brief   read data from efuse
 *
 * @param   buffer  - data buffer
 *			
 * @return  None
 */
void efuse_read(uint32_t *buffer);

/*********************************************************************
 * @fn      efuse_write
 *
 * @brief   write data to efuse,set PD4 to high, and set aldo to highest.
 *
 * @param   buffer  - data buffer
 *          mask    - indicate which bit to be written: 1 means to be
 *                    written, 0 means keep origin value.
 *			
 * @return  None
 * 
 * example:
 
  uint32_t data[3];
  uint32_t mask[3] = {0x0,0x0,0xffffffff};
  efuse_read(data);
  printf("data0:0x%08x,data1:0x%08x,data2:0x%08x\r\n",data[0],data[1],data[2]);
  if(data[2] == 0x0)
  {
      system_set_port_mux(GPIO_PORT_D, GPIO_BIT_4, PORTD4_FUNC_D4);
      gpio_set_dir(GPIO_PORT_D, GPIO_BIT_4, GPIO_DIR_OUT);
      gpio_set_pin_value(GPIO_PORT_D,GPIO_BIT_4,1);
      pmu_set_ioldo_voltage(PMU_ALDO_VOL_3_3);
      uint32_t data[3];
      uint32_t mask[3] = {0x0,0x0,0xffffffff};
      data[2] = 0x1;
      efuse_write(data,mask);
  }
 *  
 */
void efuse_write(uint32_t *buffer, uint32_t *mask);

/*********************************************************************
 * @fn      efuse_get_chip_unique_id
 *
 * @brief   used to get unique id stored in efuse.
 *
 * @param   id_buff - buffer used to store unique id
 *			
 * @return  None
 */
void efuse_get_chip_unique_id(struct chip_unique_id_t * id_buff);

#endif  // _DRIVER_EFUSE_H

