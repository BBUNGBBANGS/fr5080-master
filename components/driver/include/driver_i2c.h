/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */

#ifndef _DRIVER_IIC_H
#define _DRIVER_IIC_H

#include <stdint.h>           // standard integer functions

/*
 * TYPEDEFS
 */
enum iic_channel_t
{
    IIC_CHANNEL_0,
    IIC_CHANNEL_1,
    IIC_CHANNEL_MAX,
};

enum iic_addr_type_t {
    IIC_ADDR_10_BITS,
    IIC_ADDR_7_BITS,
};

/*********************************************************************
 * @fn      i2c_write_byte
 *
 * @brief   write one byte to slave.
 *
 * @param   channel     - IIC_CHANNEL_0 or IIC_CHANNEL_1.
 *          slave_addr  - slave address
 *          reg_addr    - which register to be writen
 *          data        - data to be writen
 *			
 * @return  None.
 */
uint8_t i2c_write_byte(enum iic_channel_t channel, uint8_t slave_addr, uint8_t reg_addr, uint8_t data);

/*********************************************************************
 * @fn      i2c_write_bytes
 *
 * @brief   write multi-bytes to slave.
 *
 * @param   channel     - IIC_CHANNEL_0 or IIC_CHANNEL_1.
 *          addr_type   - 1: 7-its, 0: 10-bits
 *          slave_addr  - slave address
 *          reg_addr    - which register to be writen
 *          buffer      - pointer to data buffer
 *          length      - how many bytes to be written
 *			
 * @return  None.
 */
uint8_t i2c_write_bytes(enum iic_channel_t channel, uint8_t slave_addr, uint8_t reg_addr, uint8_t *buffer, uint16_t length);

/*********************************************************************
 * @fn      i2c_read_byte
 *
 * @brief   read one byte frome slave.
 *
 * @param   channel     - IIC_CHANNEL_0 or IIC_CHANNEL_1.
 *          slave_addr  - slave address
 *          reg_addr    - which register to be written
 *          buffer      - store data to buffer
 *			
 * @return  None.
 */
uint8_t i2c_read_byte(enum iic_channel_t channel, uint8_t slave_addr, uint8_t reg_addr, uint8_t *buffer);

/*********************************************************************
 * @fn      i2c_read_bytes
 *
 * @brief   read multi-bytes frome slave.
 *
 * @param   channel     - IIC_CHANNEL_0 or IIC_CHANNEL_1.
 *          slave_addr  - slave address
 *          reg_addr    - which register to be written
 *          buffer      - buffer pointer to be written
 *          length      - how many bytes to be read
 *			
 * @return  None.
 */
uint8_t i2c_read_bytes(enum iic_channel_t channel, uint8_t slave_addr, uint8_t reg_addr, uint8_t *buffer, uint16_t length);


/*********************************************************************
 * @fn      i2c_init
 *
 * @brief   Initialize iic instance.
 *
 * @param   channel     - IIC_CHANNEL_0 or IIC_CHANNEL_1.
 *          speed       - SCL speed when working as master, N * 1000
 *          slave_addr  - local address when working as slave
 *			
 * @return  None.
 */
void i2c_init(enum iic_channel_t channel, uint16_t speed, uint16_t slave_addr);

#endif

