/**
 ****************************************************************************************
 * @addtogroup FLASH
 * @{
 ****************************************************************************************
 */

/*
 * INCLUDE FILES
 ****************************************************************************************
 */

#include <stdint.h>        // standard integer definition
#include <string.h>        // string manipulation

#include "driver_flash_ssp.h"     // flash definition
#include "driver_ssp.h"
#include "driver_plf.h"

/*
 * DEFINES
 ****************************************************************************************
 */
#define FLASH_BIT_RATE          1000000
#define FLASH_SSP_CLK           2
#define FLASH_DATA_SIZE         8

uint8_t ssp_put_data(uint16_t data);
uint8_t ssp_get_data(uint8_t *data);
void ssp_enable(void);
void ssp_disable(void);
void ssp_send_then_recv(uint8_t* tx_buffer, uint32_t n_tx, uint8_t* rx_buffer, uint32_t n_rx);
void ssp_recv_data(uint8_t *buffer, uint32_t length);
void ssp_send_data(uint8_t *buffer, uint32_t length);
void ssp_init(uint8_t bit_width, uint8_t frame_type, uint8_t ms, uint32_t bit_rate, uint8_t prescale, void (*ssp_cs_ctrl)(uint8_t));

/*
 * LOCAL FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */
static uint8_t ssp_flash_read_status_reg(void)
{
    uint8_t buffer[2] = {0x00, 0x00};

    ssp_put_data(FLASH_READ_STATUS_REG_OPCODE);
    ssp_put_data(0xff);
    ssp_enable();
    ssp_wait_busy_bit();
    ssp_get_data(&buffer[0]);
    ssp_get_data(&buffer[1]);
    ssp_disable();
    return buffer[1];
}

static void ssp_flash_write_status_reg(uint8_t status)
{
    ssp_put_data(FLASH_WRITE_STATUS_REG_OPCODE);
    ssp_put_data(status);
    ssp_enable();
    ssp_wait_busy_bit();
    ssp_disable();
    ssp_clear_rx_fifo();
}

static void ssp_flash_write_enable(void)
{
    uint8_t dummy;
    ssp_put_data(FLASH_WRITE_ENABLE_OPCODE);
    ssp_enable();
    ssp_wait_busy_bit();
    ssp_disable();
    ssp_get_data(&dummy);
}

static void ssp_flash_write_disable(void)
{
    ssp_put_data(FLASH_WRITE_DISABLE_OPCODE);
    ssp_enable();
    ssp_wait_busy_bit();
    ssp_disable();
    ssp_clear_rx_fifo();
}

static void flash_poll_busy_bit(void)
{
    volatile uint16_t i;

    while(ssp_flash_read_status_reg()&0x03)
    {
        //delay
        for(i=0; i<1000; i++);
    }
}

/*
 * EXPORTED FUNCTIONS DEFINITIONS
 ****************************************************************************************
 */

void ssp_flash_protect_enable(void)
{
    uint8_t status = ssp_flash_read_status_reg();
    if((status & 0x1c) != 0x1c){
    ssp_flash_write_enable();
    status = (status & 0x83) | (0x07 << 2);
    ssp_flash_write_status_reg(status);
    flash_poll_busy_bit();
}
}

void ssp_flash_protect_disable(void)
{
    uint8_t status = ssp_flash_read_status_reg();
    if((status & 0x1c) != 0){
    ssp_flash_write_enable();
    status = (status & 0x83);
    ssp_flash_write_status_reg(status);
    flash_poll_busy_bit();
}
}


uint32_t ssp_flash_init(void)
{
    uint32_t id;

    ssp_flash_identify((uint8_t *)&id);

    return (id & 0xFFFFFF);
}

uint8_t ssp_flash_identify(uint8_t* id)
{
    uint8_t dummy;
    ssp_put_data(FLASH_READ_IDENTIFICATION);
    ssp_put_data(0xff);
    ssp_put_data(0xff);
    ssp_put_data(0xff);
    ssp_enable();
    ssp_wait_busy_bit();
    ssp_disable();
    ssp_get_data(&dummy);
    ssp_get_data(&id[0]);
    ssp_get_data(&id[1]);
    ssp_get_data(&id[2]);
    
    return 0;
}

uint8_t ssp_flash_erase(uint32_t offset, uint32_t size)
{
    uint32_t addr;

    addr = offset & 0xFFFFF000;

    ssp_flash_write_enable();

    ssp_put_data(FLASH_SECTORE_ERASE_OPCODE);

    ssp_put_data(addr >> 16);
    ssp_put_data(addr >> 8);
    ssp_put_data(addr);
    ssp_enable();
    ssp_wait_busy_bit();
    ssp_disable();
    ssp_clear_rx_fifo();
    
    flash_poll_busy_bit();

    return 0;
}

uint8_t ssp_flash_chip_erase(void)
{
    ssp_flash_write_enable();

    ssp_put_data(FLASH_CHIP_ERASE_OPCODE);
    ssp_enable();
    ssp_wait_busy_bit();
    ssp_disable();
    ssp_clear_rx_fifo();

    flash_poll_busy_bit();
    return 0;
}

uint8_t ssp_flash_write(uint32_t offset, uint32_t length, uint8_t *buffer)
{
    unsigned char page_count;

    page_count = length >> 8;
    if((offset&0x000000ff)|(length&0x000000ff))
    {
        if((256-(offset&0x000000ff))>=(length&0x000000ff))
            page_count++;
        else
            page_count += 2;
    }

    if(length > 0)
    {
        for(; page_count > 0; page_count--)
        {
            ssp_flash_write_enable();
            ssp_put_data(FLASH_PAGE_PROGRAM_OPCODE);
            ssp_put_data(offset >> 16);
            ssp_put_data(offset >> 8);
            ssp_put_data(offset);
            ssp_enable();
            uint16_t page_left = 256-(offset&0xff);
            if(page_left > length) {
                page_left = length;
            }
            ssp_send_data(buffer, page_left);
            buffer += page_left;
            offset += page_left;
            //ssp_wait_busy_bit();
            //ssp_disable();
            ssp_clear_rx_fifo();
            
            flash_poll_busy_bit();
        }
    }

    return 0;
}

#define FLASH_READ_SINGLE_PACKET_LEN       (SSP_FIFO_SIZE-5)
uint8_t ssp_flash_read(uint32_t offset, uint32_t length, uint8_t *buffer)
{
    uint8_t write_buf[5];
    uint32_t read_times;
    uint8_t last_bytes;
    uint32_t i;

    read_times = length/FLASH_READ_SINGLE_PACKET_LEN;
    last_bytes = length%FLASH_READ_SINGLE_PACKET_LEN;

    for(i=0; i<read_times; i++)
    {
        write_buf[0] = FLASH_FAST_READ_OPCODE;
        write_buf[1] = offset >> 16;
        write_buf[2] = offset >> 8;
        write_buf[3] = offset ;
        write_buf[4] = 0xFF;
        ssp_send_then_recv_flash(write_buf, 5, buffer+(i*FLASH_READ_SINGLE_PACKET_LEN), FLASH_READ_SINGLE_PACKET_LEN);
        offset += FLASH_READ_SINGLE_PACKET_LEN;
    }
    if(last_bytes != 0)
    {
        write_buf[0] = FLASH_FAST_READ_OPCODE;
        write_buf[1] = offset >> 16;
        write_buf[2] = offset >> 8;
        write_buf[3] = offset ;
        write_buf[4] = 0xFF;
        ssp_send_then_recv_flash(write_buf, 5, buffer+(read_times*FLASH_READ_SINGLE_PACKET_LEN), last_bytes);
    }

    return 0;
}

/// @} FLASH

