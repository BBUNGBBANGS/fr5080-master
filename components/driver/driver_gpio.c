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
#include "driver_gpio.h"
#include "driver_iomux.h"

/*********************************************************************
 * @fn      gpio_set_dir
 *
 * @brief   set specific gpio channel working mode: output or input.
 *
 * @param   port    - which port this channel belongs to. @ref system_port_t
 *          bit     - channel number. @ref system_port_bit_t
 *          dir     - in-out selection, should be GPIO_DIR_IN or GPIO_DIR_OUT.
 *
 * @return  None.
 */

void gpio_set_dir(enum system_port_t port, enum system_port_bit_t bit, uint8_t dir)
{
    switch(port)
    {
        case GPIO_PORT_A:
            gpio_porta_set_dir((gpio_porta_get_dir()&(~(1<<bit)))|(dir<<bit));
            break;
        case GPIO_PORT_B:
            gpio_portb_set_dir((gpio_portb_get_dir()&(~(1<<bit)))|(dir<<bit));
            break;
        case GPIO_PORT_C:
            gpio_portc_set_dir((gpio_portc_get_dir()&(~(1<<bit)))|(dir<<bit));
            break;
        case GPIO_PORT_D:
            gpio_portd_set_dir((gpio_portd_get_dir()&(~(1<<bit)))|(dir<<bit));
            break;
        default:
            break;
    }
}

/*********************************************************************
 * @fn      gpio_set_pin_value
 *
 * @brief   set specific gpio pin vale.
 *
 * @param   port    - which port this channel belongs to. @ref system_port_t
 *          bit     - channel number. @ref system_port_bit_t
 *          value   - high-low voltage value. 0, output as low voltage; 1 output as high voltage
 *
 * @return  None.
 */

void gpio_set_pin_value(enum system_port_t port, enum system_port_bit_t bit, uint8_t value)
{
    switch(port)
    {
        case GPIO_PORT_A:
            gpio_porta_write((gpio_porta_read()&(~(1<<bit)))|(value<<bit));
            break;
        case GPIO_PORT_B:
            gpio_portb_write((gpio_portb_read()&(~(1<<bit)))|(value<<bit));
            break;
        case GPIO_PORT_C:
            gpio_portc_write((gpio_portc_read()&(~(1<<bit)))|(value<<bit));
            break;
        case GPIO_PORT_D:
            gpio_portd_write((gpio_portd_read()&(~(1<<bit)))|(value<<bit));
            break;
        default:
            break;
    }
}

uint8_t gpio_get_pin_value(enum system_port_t port, enum system_port_bit_t bit)
{
    switch(port)
    {
        case GPIO_PORT_A:
            return ( (gpio_porta_read() & (1<<bit)) != 0 );
        case GPIO_PORT_B:
            return ( (gpio_portb_read() & (1<<bit)) != 0 );
        case GPIO_PORT_C:
            return ( (gpio_portc_read() & (1<<bit)) != 0 );
        case GPIO_PORT_D:
            return ( (gpio_portd_read() & (1<<bit)) != 0 );
        default:
            return 0;
    }
}

void gpio_int_enable(enum system_port_t port, enum system_port_bit_t bit)
{
    *(volatile uint32_t *)GPIO_INT_ENABLE |= (1<<(port*8+bit));
}

void gpio_int_disable(enum system_port_t port, enum system_port_bit_t bit)
{
    *(volatile uint32_t *)GPIO_INT_ENABLE &= (~(1<<(port*8+bit)));
}

