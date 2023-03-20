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
#include <string.h>

#include "driver_ssp.h"
#include "driver_plf.h"
#include "driver_syscntl.h"
#include "driver_gpio.h"

/*
 * MACROS 
 */
/*
 * TYPEDEFS
 */
typedef unsigned int    u32;
typedef unsigned short  u16;
typedef unsigned char   u8;

struct ssp_cr0
{
    uint32_t dss:4;  /* data size select : = DATASIZE - 1*/

    uint32_t frf:2;  /* frame format */

    uint32_t spo:1;  /* sclk polarity */
    uint32_t sph:1;  /* sclk phase */
    uint32_t scr:8;  /* serial clock rate */
    uint32_t unused:16;
};

struct ssp_cr1
{
    uint32_t rie:1;
    uint32_t tie:1;
    uint32_t rorie:1;

    uint32_t lbm:1;  /* loop back mode */
    uint32_t sse:1;  /* synchronous serial port enable*/

    uint32_t ms:1;   /* master mode or slave mode */
    uint32_t sod:1;  /* output disable in slave mode */

    uint32_t unused:25;
};

struct ssp_dr
{
    uint32_t data:8;

    uint32_t unused:24;
};

struct ssp_sr
{
    uint32_t tfe:1;  /* transmit fifo empty */
    uint32_t tnf:1;  /* transmit fifo not full */
    uint32_t rne:1;  /* receive fifo not empty */
    uint32_t rff:1;  /* receive fifo full */
    uint32_t bsy:1;  /* ssp busy flag */

    uint32_t unused:27;
};

struct ssp_cpsr
{
    uint32_t cpsdvsr:8;  /* clock prescale divisor 2-254 */

    uint32_t unused:24;
};

struct ssp_iir
{
    uint32_t ris:1;
    uint32_t tis:1;
    uint32_t roris:1;
    uint32_t reserved:29;
};

struct ssp_ff_int_ctrl {
    uint32_t rx_ff:8;
    uint32_t tx_ff:8;

    uint32_t unused:16;
};

struct ssp
{
    struct ssp_cr0 ctrl0;
    struct ssp_cr1 ctrl1; /*is also error clear register*/
    uint32_t data;
    struct ssp_sr status;
    struct ssp_cpsr clock_prescale;
    struct ssp_iir iir;
    struct ssp_ff_int_ctrl ff_ctrl;
};

struct ssp_env_t
{
    void (*ssp_cs_ctrl)(uint8_t);
};

/*
 * LOCAL VARIABLES
 */

/*********************************************************************
 * @fn      ssp_get_isr_status
 *
 * @brief   get current interrupt status
 *
 * @param   None
 * 
 * @return  current status ,@ref ssp_int_status_t.
 */
__attribute__((section("ram_code"))) uint32_t ssp_get_isr_status(void)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP_BASE;
    return *(volatile uint32_t *)&ssp->iir;
}

/*********************************************************************
 * @fn      ssp_clear_isr_status
 *
 * @brief   used to clear interrupt status
 *
 * @param   status  - which status should be cleard, @ref ssp_int_status_t
 *
 * @return  None
 */
__attribute__((section("ram_code"))) void ssp_clear_isr_status(uint32_t status)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP_BASE;
    *(volatile uint32_t *)&ssp->iir = status;
}

/*********************************************************************
 * @fn      ssp_get_status
 *
 * @brief   get current fifo and ssp bus status
 *
 * @param   None
 * 
 * @return  current status ,@ref ssp_status_t.
 */
__attribute__((section("ram_code"))) uint32_t ssp_get_status(void)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP_BASE;
    return *(volatile uint32_t *)&ssp->status;
}

/*********************************************************************
 * @fn      ssp_put_data_to_fifo
 *
 * @brief   put data to send fifo, this function will return when all data have
 *          been put into tx fifo or tx fifo is full
 *
 * @param   buffer  - data pointer to be send
 *          length  - how many bytes to be send.
 *
 * @return  how many data have been put into tx fifo.
 */
__attribute__((section("ram_code"))) uint16_t ssp_put_data_to_fifo(uint8_t *buffer, uint16_t length)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP_BASE;
    uint16_t send_length = 0;

    for(uint16_t i=0; i<length; i++) {
        if(ssp->status.tnf) {
            ssp->data = *buffer++;
            send_length++;
        }
        else {
            break;
        }
    }

    return send_length;
}

/*********************************************************************
 * @fn      ssp_get_data_from_fifo
 *
 * @brief   get data from receive fifo, this function will return when length
 *          is reached or rx fifo is empty.
 *
 * @param   buffer  - data pointer to be store received buffer
 *          length  - how many bytes to get.
 *
 * @return  how many data have been got from rx fifo.
 */
__attribute__((section("ram_code"))) uint16_t ssp_get_data_from_fifo(uint8_t *buffer, uint16_t length)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP_BASE;
    uint16_t recv_length = 0;

    for(uint16_t i=0; i<length; i++) {
        if(ssp->status.rne) {
            *buffer++ = ssp->data;
            recv_length++;
        }
        else {
            break;
        }
    }

    return recv_length;
}

__attribute__((section("ram_code")))uint16_t  ssp_clear_rx_fifo(void)
{
	volatile struct ssp * const ssp = (volatile struct ssp *)SSP_BASE;
	uint8_t buffer;
	uint16_t length = 0;
	
	while(ssp->status.rne)
	{
		buffer = ssp->data;
     length++;
	}
	return length;
}

__attribute__((section("ram_code"))) void ssp_wait_busy_bit(void)
{
	volatile struct ssp * const ssp = (volatile struct ssp *)SSP_BASE;
	
    while(ssp->status.bsy);
}

/*********************************************************************
 * @fn      ssp_slave_output_enable
 *
 * @brief   enable output when working as slave
 *
 * @param   None
 * 
 * @return  None
 */
__attribute__((section("ram_code"))) void ssp_slave_output_enable(void)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP_BASE;
    ssp->ctrl1.sod = 0;
}

/*********************************************************************
 * @fn      ssp_slave_output_disable
 *
 * @brief   disable output when working as slave
 *
 * @param   None
 * 
 * @return  None
 */
__attribute__((section("ram_code"))) void ssp_slave_output_disable(void)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP_BASE;
    ssp->ctrl1.sod = 1;
}

/*********************************************************************
 * @fn      ssp_set_clk_type
 *
 * @brief   set spi clock parameter
 *
 * @param   polarity    -
 *          phase       -
 * 
 * @return  None
 */
__attribute__((section("ram_code"))) void ssp_set_clk_type(uint8_t polarity, uint8_t phase)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP_BASE;
    ssp->ctrl0.spo = polarity;
    ssp->ctrl0.sph = phase;
}

uint8_t ssp_send_then_recv_flash(uint8_t* tx_buffer, uint32_t n_tx, uint8_t* rx_buffer, uint32_t n_rx)
{
    volatile uint8_t i,temp;
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP_BASE;
    uint32_t block = 0,last,times = 0;
    
    uint8_t spi_send_data[SSP_FIFO_SIZE];
    memset(spi_send_data,0,SSP_FIFO_SIZE);
    memcpy(spi_send_data,tx_buffer,5);
    
    if(n_tx !=5)
    {
        return 0;
    }
    
    if(n_rx <=0)
    {
        return 0;
    }
   
    block = n_rx/SSP_FIFO_SIZE;
    last = n_rx%SSP_FIFO_SIZE;

     ssp->ctrl1.sse = 1;
    while(last)
    {
        if(block)
        {
            for(i = 0; i < SSP_FIFO_SIZE;i++)
            {
                ssp->data = spi_send_data[i];
            }
        }
        else
        {
            for(i = 0; i < last+5;i++)
            {
                ssp->data = spi_send_data[i];
            }
        }

        while(ssp->status.bsy);
        for(i = 0; i < 5;i++)
        {
            temp = ssp->data;
        }
        if(block)
        {
            for(i = 0; i < SSP_FIFO_SIZE;i++)
            {
                rx_buffer[i+SSP_FIFO_SIZE*times] = ssp->data;
            }
            times++;
            block--;
        }
        else
        {
            for(i = 0; i < last;i++)
            {
              rx_buffer[i+SSP_FIFO_SIZE*times] = ssp->data;
            }               
            last = 0;
        }
    }
    ssp->ctrl1.sse = 0;

    return 1;
}

