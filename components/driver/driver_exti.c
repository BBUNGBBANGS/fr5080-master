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

#include "driver_syscntl.h"
#include "driver_exti.h"
#include "driver_plf.h"

#define EXT_INT_TYPE_MSK        0x03
#define EXT_INT_TYPE_LEN        2

struct ext_int_t
{
    uint32_t ext_int_en;
    uint32_t ext_int_status;
    uint32_t ext_int_type[2];
    uint32_t ext_int_control[32];
    uint32_t ext_int_raw_status;
};

volatile struct ext_int_t *const ext_int_reg = (struct ext_int_t *)(GPIO_BASE+0x8000);

void ext_int_enable(enum system_port_t port, enum system_port_bit_t bit)
{
    uint8_t exti_channel = (port*8 + bit);
    ext_int_reg->ext_int_en |= (1<<exti_channel);
}

void ext_int_disable(enum system_port_t port, enum system_port_bit_t bit)
{
    uint8_t exti_channel = (port*8 + bit);
    ext_int_reg->ext_int_en &= (~(1<<exti_channel));
}

uint32_t ext_int_get_status(void)
{
    return ext_int_reg->ext_int_status;
}

void ext_int_clear(uint32_t bit)
{
    ext_int_reg->ext_int_status = bit;
}

void ext_int_set_type(enum system_port_t port, enum system_port_bit_t bit, enum ext_int_type_t type)
{
    uint8_t offset, index;
    uint32_t value;
    uint8_t exti_channel = (port*8 + bit);

    index = exti_channel / 16;
    offset = (exti_channel % 16) << 1;

    value = ext_int_reg->ext_int_type[index];
    value &= (~(EXT_INT_TYPE_MSK<<offset));
    value |= (type << offset);
    ext_int_reg->ext_int_type[index] = value;
}

void ext_int_set_control(enum system_port_t port, enum system_port_bit_t bit, uint32_t clk, uint8_t counter)
{
    uint32_t pclk;
    uint8_t exti_channel = (port*8 + bit);

    pclk = system_get_pclk();

    ext_int_reg->ext_int_control[exti_channel] = (((pclk/clk-1)<<4) | (counter-1));
}

//__attribute__((section("ram_code"))) void exti_isr_ram(void)
//{
//    uint32_t status;

//    status = ext_int_get_status();
//    ext_int_clear(status);
//}


