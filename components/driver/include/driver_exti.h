/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */
#ifndef _DRIVER_EXTI_H_
#define _DRIVER_EXTI_H_

/*
 * INCLUDE
 */
#include <stdbool.h>          // standard boolean definitions
#include <stdint.h>           // standard integer functions

#include "driver_plf.h"
#include "driver_iomux.h"

enum ext_int_type_t
{
    EXT_INT_TYPE_LOW,
    EXT_INT_TYPE_HIGH,
    EXT_INT_TYPE_POS,
    EXT_INT_TYPE_NEG,
};

/*
 * FUNCTION
 */

/*********************************************************************
 * @fn      ext_int_enable
 *
 * @brief   enable interrupt from the specified pin.
 *
 * @param   port    - which port this channel belongs to. @ref system_port_t
 *          bit     - channel number. @ref system_port_bit_t
 *
 * @return  None.
 */
void ext_int_enable(enum system_port_t port, enum system_port_bit_t bit);

/*********************************************************************
 * @fn      ext_int_disable
 *
 * @brief   disable interrupt of the specified pin.
 *
 * @param   port    - which port this channel belongs to. @ref system_port_t
 *          bit     - channel number. @ref system_port_bit_t
 *
 * @return  None.
 */
void ext_int_disable(enum system_port_t port, enum system_port_bit_t bit);

/*********************************************************************
 * @fn      ext_int_get_status
 *
 * @brief   get current external interrupt status.
 *
 * @param   None.
 *
 * @return  None.
 */
uint32_t ext_int_get_status(void);

/*********************************************************************
 * @fn      ext_int_clear
 *
 * @brief   clear external interrupt.
 *
 * @param   None.
 *
 * @return  None.
 */
void ext_int_clear(uint32_t exti_src);

/*********************************************************************
 * @fn      ext_int_set_type
 *
 * @brief   set interrupt trigger type of the specified pin.
 *
 * @param   port    - which port this channel belongs to. @ref system_port_t
 *          bit     - channel number. @ref system_port_bit_t
 *          type    - trigger type. @reg ext_int_type_t
 *
 * @return  None.
 */
void ext_int_set_type(enum system_port_t port, enum system_port_bit_t bit, enum ext_int_type_t type);

/*********************************************************************
 * @fn      ext_int_set_control
 *
 * @brief   set anti-shake parameter of the specified pin.
 *
 * @param   port    - which port this channel belongs to. @ref system_port_t
 *          bit     - channel number. @ref system_port_bit_t
 *          clk     - anti-shake module frequency
 *          counter - an effective interrupt will be generate after how many 
 *                    cycles have gone through since trigger type has been detected.
 *
 * @return  None.
 */
void ext_int_set_control(enum system_port_t port, enum system_port_bit_t bit, uint32_t clk, uint8_t counter);

/// @} EXTI
#endif /* _DRIVER_EXTI_H_ */

