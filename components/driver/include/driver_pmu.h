/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */
#ifndef _DRIVER_PMU_H
#define _DRIVER_PMU_H

/*
 * INCLUDES 
 */
#include <stdint.h>
#include <stdbool.h>

#include "co_math.h"

#include "driver_iomux.h"
#include "driver_pmu_regs.h"
#include "driver_frspim.h"

/*
 * MACROS 
 */
#define ool_write(addr, data)       frspim_wr(FR_SPI_PMU_CHAN,(addr),1, (data))
#define ool_read(addr)              (uint8_t)frspim_rd(FR_SPI_PMU_CHAN,(addr),1)

#define ool_write16(addr,data)      frspim_wr(FR_SPI_PMU_CHAN,(addr),2, (data))
#define ool_read16(addr)            (uint16_t)frspim_rd(FR_SPI_PMU_CHAN,(addr),2)

#define ool_write32(addr,data)      frspim_wr(FR_SPI_PMU_CHAN,(addr),4, (data))
#define ool_read32(addr)            (uint32_t)frspim_rd(FR_SPI_PMU_CHAN,(addr),4)

/*
 * TYPEDEFS 
 */
enum pmu_isr_enable_t
{
    PMU_ISR_BIT_ONKEY_OFF =            CO_BIT(0),
    PMU_ISR_BIT_ALMA =                 CO_BIT(1),
    PMU_ISR_BIT_ONKEY =                CO_BIT(2),
    PMU_ISR_BIT_BAT_FULL =             CO_BIT(3),
    PMU_ISR_BIT_ACOK =                 CO_BIT(4),
    PMU_ISR_BIT_ACOFF =                CO_BIT(5),
    PMU_ISR_BIT_LVD =                  CO_BIT(6),
    PMU_ISR_BIT_OTD =                  CO_BIT(7),
    PMU_ISR_BIT_CAL_DONE =             CO_BIT(8),
    PMU_ISR_BIT_ADKEY0 =               CO_BIT(9),
    PMU_ISR_BIT_ADKEY1 =               CO_BIT(10),
    PMU_ISR_BIT_ONE_WIRE =             CO_BIT(11),
    PMU_ISR_BIT_ULV =                  CO_BIT(12),
    PMU_ISR_BIT_LED0 =                 CO_BIT(13),
    PMU_ISR_BIT_LED1 =                 CO_BIT(14),
    PMU_ISR_BIT_GPIO =                 CO_BIT(15),
};
/*
*Select output value:
*/
enum port_output_sel
{
    SEL_PMU_REG,
    SEL_LED_CTL
};

enum pmu_ioldo_voltage_t
{
    PMU_ALDO_VOL_3_3 = 0xf0,
    PMU_ALDO_VOL_3_2 = 0xe0,
    PMU_ALDO_VOL_3_1 = 0xd0,
    PMU_ALDO_VOL_3_0 = 0xc0,
    PMU_ALDO_VOL_2_9 = 0xb0,
    PMU_ALDO_VOL_2_8 = 0xa0,
    PMU_ALDO_VOL_2_7 = 0x90,
    PMU_ALDO_VOL_2_6 = 0x80,
    PMU_ALDO_VOL_2_5 = 0x70,
    PMU_ALDO_VOL_2_4 = 0x60,
    PMU_ALDO_VOL_2_3 = 0x50,
    PMU_ALDO_VOL_2_2 = 0x40,
    PMU_ALDO_VOL_2_1 = 0x30,
    PMU_ALDO_VOL_2_0 = 0x20,
    PMU_ALDO_VOL_1_9 = 0x10,
    PMU_ALDO_VOL_1_8 = 0x00,
};

enum charge_current_t
{
    CHG_CUR_20MA    =   0x43,
    CHG_CUR_30MA    =   0x45,
    CHG_CUR_40MA    =   0x48,
    CHG_CUR_50MA    =   0x4a,
    CHG_CUR_60MA    =   0x4b,
    CHG_CUR_70MA    =   0x4e,
    CHG_CUR_80MA    =   0x50,
    CHG_CUR_90MA    =   0x42,
    CHG_CUR_100MA   =   0x54,
    CHG_CUR_120MA    =  0x58,
    CHG_CUR_150MA   =   0x5e,
    CHG_CUR_200MA   =   0x68,
};

enum charge_term_vol_t
{
    CHG_VOL_4_10V    =   0,
    CHG_VOL_4_15V    =   1,
    CHG_VOL_4_20V    =   2,
    CHG_VOL_4_25V    =   3,
    CHG_VOL_4_30V    =   4,
    CHG_VOL_4_35V    =   5,
    CHG_VOL_4_40V    =   6,
};

/*********************************************************************
 * @fn      pmu_set_io_voltage
 *
 * @brief   set the aldo output voltage, also known as IO voltage.
 *
 * @param   value   - voltage target value.
 *
 * @return  None.
 */
void pmu_set_ioldo_voltage(enum pmu_ioldo_voltage_t value);

/*********************************************************************
 * @fn      pmu_power_on_DSP
 *
 * @brief   Enable DSP power supply.
 *
 * @param   None
 *
 * @return  None.
 */
void pmu_power_on_DSP(void);

/*********************************************************************
 * @fn      pmu_power_off_DSP
 *
 * @brief   Disable DSP power supply.
 *
 * @param   None
 *
 * @return  None.
 */
void pmu_power_off_DSP(void);

/*********************************************************************
 * @fn      pmu_set_pin_to_PMU
 *
 * @brief   Hand over the IO control from main digital core to PMU (always on),
 *          this function can be used to set more than one IO belong
 *          to the same port. 
 *          example usage: pmu_set_pin_to_PMU(GPIO_PORT_A, (1<<GPIO_BIT_0)|((1<<GPIO_BIT_1))
 *
 * @param   port    - which group the io belongs to, @ref system_port_t
 *          bits    - io channel number
 *
 * @return  None.
 */
void pmu_set_pin_to_PMU(enum system_port_t port, uint8_t bits);

/*********************************************************************
 * @fn      pmu_set_pin_to_CPU
 *
 * @brief   Hand over the IO control from PMU to main digital core, this function
 *          can be used to set more than one IO belong to the same port.
 *          example usage: pmu_set_pin_to_CPU(GPIO_PORT_A, (1<<GPIO_BIT_0)|((1<<GPIO_BIT_1))
 *
 * @param   port    - which group the io belongs to, @ref system_port_t
 *          bits    - the numbers of io
 *
 * @return  None.
 */
void pmu_set_pin_to_CPU(enum system_port_t port, uint8_t bits);

/*********************************************************************
 * @fn      pmu_set_pin_dir
 *
 * @brief   set the in-out of IOs which are controlled by PMU.
 *          example usage:
 *          pmu_set_pin_dir(GPIO_PORT_A, (1<<GPIO_BIT_0)|((1<<GPIO_BIT_1), GPIO_DIR_OUT)
 *
 * @param   port    - which group the io belongs to, @ref system_port_t
 *          bits    - the numbers of io
 *          dir     - the direction of in-out, GPIO_DIR_OUT or GPIO_DIR_IN
 *
 * @return  None.
 */
void pmu_set_pin_dir(enum system_port_t port, uint8_t bits, uint8_t dir);

/*********************************************************************
 * @fn      pmu_set_pin_pull_up
 *
 * @brief   set pull-up of IOs which are controlled by PMU.
 *          example usage:
 *          pmu_set_pin_pull(GPIO_PORT_A, (1<<GPIO_BIT_0)|((1<<GPIO_BIT_1), true)
 *
 * @param   port    - which group the io belongs to, @ref system_port_t
 *          bits    - the numbers of io
 *          flag    - true: enable pull-up, false: disable pull-up.
 *
 * @return  None.
 */
void pmu_set_pin_pull_up(enum system_port_t port, uint8_t bits, bool flag);


/*********************************************************************
 * @fn      pmu_set_pin_pull_down
 *
 * @brief   set pull-up of IOs which are controlled by PMU.
 *          example usage:
 *          pmu_set_pin_pull(GPIO_PORT_A, (1<<GPIO_BIT_0)|((1<<GPIO_BIT_1), true)
 *
 * @param   port    - which group the io belongs to, @ref system_port_t
 *          bits    - the numbers of io
 *          flag    - true: enable pull-down, false: disable pull-down.
 *
 * @return  None.
 */
void pmu_set_pin_pull_down(enum system_port_t port, uint8_t bits, bool flag);


/*********************************************************************
 * @fn      pmu_set_gpio_output_select
 *
 * @brief   Select output value of IOs which are controlled   
 *          example usage:
 *          pmu_set_gpio_output_select(GPIO_PORT_A, BIT(0)|BIT(1), SEL_PMU_REG);
 *
 * @param   port    - which group the io belongs to, @ref system_port_t
 *          bits    - the numbers of io
 *          value   - SEL_LED_CTL: from LED control, SEL_PMU_REG: from PMU reg.
 *
 * @return  None.
 */

void pmu_set_gpio_output_select(enum system_port_t port, uint8_t bits, enum port_output_sel sel);


/*********************************************************************
 * @fn      pmu_set_gpio_value
 *
 * @brief   set value of IOs which are controlled by PMU.
 *          example usage:
 *          pmu_set_gpio_value(GPIO_PORT_A, (1<<GPIO_BIT_0)|((1<<GPIO_BIT_1), 1)
 *
 * @param   port    - which group the io belongs to, @ref system_port_t
 *          bits    - the numbers of io
 *          value   - 1: set the IO to high, 0: set the IO to low.
 *
 * @return  None.
 */

void pmu_set_gpio_value(enum system_port_t port, uint8_t bits, uint8_t value);

/*********************************************************************
 * @fn      pmu_get_gpio_value
 *
 * @brief   get value of IO which are controlled by PMU and in GPIO mode.
 *          example usage:
 *          pmu_get_gpio_value(GPIO_PORT_A, GPIO_BIT_0)
 *
 * @param   port    - which group the io belongs to, @ref system_port_t
 *          bit     - the number of io
 *
 * @return  1: the IO is high, 0: the IO is low..
 */
uint8_t pmu_get_gpio_value(enum system_port_t port, uint8_t bit);

/*********************************************************************
 * @fn      pmu_set_sys_power_mode
 *
 * @brief   used to set system power supply mode, BUCK or LDO
 *
 * @param   mode    - indicate the mode to be used, @ref pmu_sys_pow_mode_t
 *
 * @return  None.
 */

/*********************************************************************
 * @fn      pmu_enable_irq
 *
 * @brief   enable the irq of modules inside PMU.
 *
 * @param   irqs    -- indicate which irq should be enable,
 *                     refer to pmu_isr_enable_t
 *
 * @return  None.
 */
void pmu_enable_isr(uint16_t irqs);

/*********************************************************************
 * @fn      pmu_disable_irq
 *
 * @brief   disable the irq of modules inside PMU.
 *
 * @param   irqs    -- indicate which irq should be disabled,
 *                     refer to pmu_isr_enable_t
 *
 * @return  None.
 */
void pmu_disable_isr(uint16_t irqs);

/*********************************************************************
 * @fn      pmu_clear_isr_state
 *
 * @brief   clear PMU interrupt.
 *
 * @param   state_map   - indicate which irq should be clear
 *
 * @return  None.
 */
void pmu_clear_isr_state(uint16_t state_map);

/*********************************************************************
 * @fn      pmu_port_wakeup_func_set
 *
 * @brief   indicate which ports should be checked by PMU GPIO monitor module.
 *          once the state of corresponding GPIO changes, an PMU interrupt
 *          will be generated.
 *
 * @param   gpios   - 32bit value, bit num corresponding to pin num.
 *                    sample: 0x08080808 means PA3, PB3, PC3, PD3 will be
 *                    checked.
 *
 * @return  None.
 */
void pmu_port_wakeup_func_set(uint32_t gpios);

/*********************************************************************
 * @fn      pmu_get_rc_clk
 *
 * @brief   get inner rc clk frequecy 
 *
 * @param   NULL
 *
 * @return  -   rc clk frequecy.  unit: Hz.
 */
uint32_t pmu_get_rc_clk(void);

/*********************************************************************
 * @fn      pmu_set_charge
 *
 * @brief   set charge current & voltage
 *
 * @param   cur - charge current, @ref enum charge_current_t
 *          vol - charge terminal voltage, @ref enum charge_term_vol_t
 *
 * @return  None.
 */
void pmu_set_charge(enum charge_current_t cur, enum charge_term_vol_t vol);


void pmu_shutdown(void);

#endif
