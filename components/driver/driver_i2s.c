/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */

/*
 * INCLUDES
 */
#include <stdint.h>
#include "type_lib.h"
#include "driver_i2s.h"
//#include "driver_system.h"
#include "driver_plf.h"
#include "driver_uart.h"
#include "driver_syscntl.h"

/*
 * MACROS
 */
#define I2S_PCLK        12000000
//#define I2S_BCLK        1500000
#define I2S_BCLK_DIV    ((I2S_PCLK/(I2S_BCLK*2))-2)



/*
 * LOCAL VARIABLES
 */
volatile struct i2s_reg_t *i2s_reg = (struct i2s_reg_t *)I2S_BASE;

void i2s_clear_fifo_(void)
{
    uint8_t i;
    volatile uint32_t data;

    for(i=0; i<I2S_FIFO_DEPTH; i++)
    {
        data = i2s_reg->data;
    }
}

/*********************************************************************
 * @fn      i2s_init
 *
 * @brief   Initialize i2s.
 *
 * @param   type        - @ref i2s_dir_t.
 *          bus_clk     - i2s bus bit clock
 *          sample_rate - i2s sample rate
 *          mode        - which mode i2s works in. @ref i2s_mode_t
 *
 * @return  None.
 */

void i2s_init_(uint8_t type, uint32_t bus_clk, uint32_t sample_rate, enum i2s_mode_t mode)
{
 uint32_t frm_div;
    
    frm_div = (bus_clk/(sample_rate*2))-1;
    if(bus_clk == 12000000){
        i2s_reg->bclk_div = 0xffff;
    }else{
        i2s_reg->bclk_div = ((I2S_PCLK/(bus_clk*2))-1);
    }
    
    i2s_reg->frm_div = frm_div;
    *(uint32_t *)&i2s_reg->mask = 0;

    if(type & I2S_DIR_RX) {
        i2s_reg->mask.rx_half_full = 1;
        i2s_reg->mask.rx_full = 1;

        i2s_reg->ctrl.rx_int_en = 1;
    }

    if(type & I2S_DIR_TX) {
        i2s_reg->mask.tx_emtpy = 1;
        i2s_reg->mask.tx_half_empty = 1;

        i2s_reg->ctrl.tx_int_en = 1;
    }

    i2s_reg->ctrl.dlyen = 1;
    i2s_reg->ctrl.frminv = 1;
    i2s_reg->ctrl.format = 1;
    i2s_reg->ctrl.mode = mode;
    i2s_reg->ctrl.reserved = 1;
}

/*********************************************************************
 * @fn      i2s_start
 *
 * @brief   start i2s.
 *
 * @param   None.
 *
 * @return  None.
 */
void i2s_start_(void)
{
    i2s_clear_fifo_();  
    
    i2s_reg->ctrl.inten = 1;
    i2s_reg->ctrl.en = 1;
}

/*********************************************************************
 * @fn      i2s_start_without_int
 *
 * @brief   start i2s with interrupt disabled.
 *
 * @param   None.
 *
 * @return  None.
 */
void i2s_start_without_int(void)
{
    i2s_clear_fifo_();  
    
    i2s_reg->ctrl.en = 1;
}

/*********************************************************************
 * @fn      i2s_set_tx_src
 *
 * @brief   select the source of tx fifo.
 *
 * @param   src     - @ref i2s_tx_src_t
 *
 * @return  None.
 */
void i2s_set_tx_src(enum i2s_tx_src_t src)
{
    i2s_reg->tx_src_sel.sel = src;
}

/*********************************************************************
 * @fn      i2s_stop
 *
 * @brief   stop i2s.
 *
 * @param   None.
 *
 * @return  None.
 */
void i2s_stop_(void)
{
    i2s_reg->ctrl.en = 0;
    i2s_reg->ctrl.inten = 0;
}

/*********************************************************************
 * @fn      i2s_get_int_status
 *
 * @brief   get current i2s interrupt status.
 *
 * @param   None.
 *
 * @return  current interrupt status.
 */
uint32_t i2s_get_int_status_(void)
{
    return *(uint32_t *)&i2s_reg->status;
}

/*********************************************************************
 * @fn      i2s_get_data
 *
 * @brief   read i2s data from rx fifo.
 *
 * @param   buffer  - pointer to a buffer used to store data.
 *          length  - how many data to read, should no larger than I2S_FIFO_DEPTH
 *          type    - data is mono (2 bytes for one sample) or stereo (4 bytes for
 *                    one sample), @ref i2s_data_type_t.
 *
 * @return  None.
 */
void i2s_get_data_(void *buffer, uint8_t length, uint8_t type)
{
    uint16_t *mono_data;
    uint32_t *stereo_data;
    uint8_t i;

    if(type == I2S_DATA_MONO) {
        mono_data = (uint16_t *)buffer;
        for(i=0; i<length; i++) {
            *mono_data++ = (uint16_t)i2s_reg->data;
        }
    }
    else if(type == I2S_DATA_STEREO) {
        stereo_data = (uint32_t *)buffer;
        for(i=0; i<length; i++) {
            *stereo_data++ = i2s_reg->data;
        }
    }
}

/*********************************************************************
 * @fn      i2s_send_data
 *
 * @brief   write data to i2s tx fifo.
 *
 * @param   buffer  - pointer to a buffer store avaliable data.
 *          length  - how many data to send, should no larger than I2S_FIFO_DEPTH
 *          type    - data is mono (2 bytes for one sample) or stereo (4 bytes for
 *                    one sample), @ref i2s_data_type_t.
 *
 * @return  None.
 */
void i2s_send_data_(void *buffer, uint8_t length, uint8_t type)
{
    uint16_t *mono_data;
    uint32_t *stereo_data;
    uint8_t i;

    if(type == I2S_DATA_MONO) {
        mono_data = (uint16_t *)buffer;
        for(i=0; i<length; i++) {
            i2s_reg->data = *mono_data++;
        }
    }
    else if(type == I2S_DATA_STEREO) {
        stereo_data = (uint32_t *)buffer;
        for(i=0; i<length; i++) {
            i2s_reg->data = *stereo_data;
        }
    }
}



