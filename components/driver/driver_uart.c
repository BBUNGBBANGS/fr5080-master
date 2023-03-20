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
#include "driver_plf.h"
#include "driver_uart.h"

__attribute__((weak)) __attribute__((section("ram_code"))) void uart_isr_ram(void)
{
    uint8_t int_id;
    volatile uint8_t c;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART_BASE;

    int_id = uart_reg->u3.iir.int_id;

    if(int_id == 0x04 || int_id == 0x0c )   /* Receiver data available or Character time-out indication */
    {
        c = uart_reg->u1.data;
    }
    else if(int_id == 0x06)
    {
        volatile uint32_t line_status = uart_reg->lsr;
    }
}

void uart_enable_isr(uint8_t rx_enable, uint8_t tx_enable)
{
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART_BASE;
    /*enable recv or tx interrupt*/
    uart_reg->u2.ier.erdi = rx_enable&0x01;
    uart_reg->u2.ier.eti = tx_enable&0x01;
}



