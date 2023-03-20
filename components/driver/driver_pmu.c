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
#include <stdio.h>

#include "user_utils.h"
#include "co_math.h"

#include "driver_syscntl.h"
#include "driver_pmu.h"
#include "driver_plf.h"

__attribute__((section("ram_code"))) void pmu_set_ioldo_voltage(enum pmu_ioldo_voltage_t value)
{
    ool_write(PMU_REG_IOLDO_CTRL, (ool_read(PMU_REG_IOLDO_CTRL) & 0x0f) | value);
    co_delay_10us(2);
}

void pmu_power_on_DSP(void)
{
    REG_PL_WR8(0x500210f1,0xe0);
    ool_write(PMU_REG_DSP_ISO, 0x80);       //dsp isolation enable
    ool_write(PMU_REG_LDO_DSP_CTRL, 0xc2);  //dsp power on,bit6
    co_delay_10us(2);
    ool_write(PMU_REG_DSP_ISO, 0x00);       //dsp isolation disable
}

void pmu_power_off_DSP(void)
{
    ool_write(PMU_REG_DSP_ISO, 0x80);       //dsp isolation enable
    //ool_write(PMU_REG_LDO_DSP_CTRL, 0x82);  //dsp power off,bit6
    system_regs->clk_enable.dsp_mas_cken = 0;
    REG_PL_WR8(0x500210f1,0xc0);
}

void pmu_set_pin_to_PMU(enum system_port_t port, uint8_t bits)
{
    uint8_t port_sel_addr[] = {PMU_REG_PORTA_SEL,PMU_REG_PORTB_SEL,PMU_REG_PORTC_SEL,PMU_REG_PORTD_SEL};
    uint8_t sel_reg = port_sel_addr[port];
    
    ool_write(sel_reg, (ool_read(sel_reg) & (~bits)));
}

void pmu_set_pin_to_CPU(enum system_port_t port, uint8_t bits)
{
    uint8_t port_sel_addr[] = {PMU_REG_PORTA_SEL,PMU_REG_PORTB_SEL,PMU_REG_PORTC_SEL,PMU_REG_PORTD_SEL};
    uint8_t sel_reg = port_sel_addr[port];
    
    ool_write(sel_reg, (ool_read(sel_reg) | bits));
}

void pmu_set_pin_dir(enum system_port_t port, uint8_t bits, uint8_t dir)
{
    uint8_t port_oen_addr[][2] = {{PMU_REG_PORTA_OEN,PMU_REG_PORTA_IE},{PMU_REG_PORTB_OEN,PMU_REG_PORTB_IE},{PMU_REG_PORTC_OEN,PMU_REG_PORTC_IE},{PMU_REG_PORTD_OEN,PMU_REG_PORTD_IE},};

    uint8_t dir_oe_reg = port_oen_addr[port][0];
    uint8_t dir_ie_reg = port_oen_addr[port][1];
    if(dir == GPIO_DIR_OUT){
        ool_write(dir_oe_reg, (ool_read(dir_oe_reg) & (~bits)));
        ool_write(dir_ie_reg, (ool_read(dir_ie_reg) & (~bits)));
    }
    else{
        ool_write(dir_oe_reg, (ool_read(dir_oe_reg) | bits));
        ool_write(dir_ie_reg, (ool_read(dir_ie_reg) | bits));
    }

}

void pmu_set_pin_pull_up(enum system_port_t port, uint8_t bits, bool flag)
{
    uint8_t port_he_addr[] = {PMU_REG_PORTA_HE,PMU_REG_PORTB_HE,PMU_REG_PORTC_HE,PMU_REG_PORTD_HE};
    uint8_t port_len_addr[] = {PMU_REG_PORTA_LEN,PMU_REG_PORTB_LEN,PMU_REG_PORTC_LEN,PMU_REG_PORTD_LEN};

    uint8_t sel_reg_he = port_he_addr[port];
    uint8_t sel_reg_len = port_len_addr[port];

    if(flag == true){
        ool_write(sel_reg_he, (ool_read(sel_reg_he) | bits));
        ool_write(sel_reg_len, (ool_read(sel_reg_len) | bits));    
    }
    else{
        ool_write(sel_reg_he, (ool_read(sel_reg_he) & (~bits)));
    }

}

void pmu_set_pin_pull_down(enum system_port_t port, uint8_t bits, bool flag)
{
    uint8_t port_he_addr[] = {PMU_REG_PORTA_HE,PMU_REG_PORTB_HE,PMU_REG_PORTC_HE,PMU_REG_PORTD_HE};
    uint8_t port_len_addr[] = {PMU_REG_PORTA_LEN,PMU_REG_PORTB_LEN,PMU_REG_PORTC_LEN,PMU_REG_PORTD_LEN};

    uint8_t sel_reg_he = port_he_addr[port];
    uint8_t sel_reg_len = port_len_addr[port];

    if(flag == true){
        ool_write(sel_reg_he, (ool_read(sel_reg_he) & (~bits)));
        ool_write(sel_reg_len, (ool_read(sel_reg_len) & (~bits)));   
    }
    else{
        ool_write(sel_reg_len, (ool_read(sel_reg_len) | bits));    
    }

}
void pmu_set_gpio_output_select(enum system_port_t port, uint8_t bits, enum port_output_sel sel)
{
    uint8_t port_output_sel_addr[] = {PMU_REG_PORTA_OUTPUT_SEL,PMU_REG_PORTB_OUTPUT_SEL,PMU_REG_PORTC_OUTPUT_SEL,PMU_REG_PORTD_OUTPUT_SEL};

    uint8_t sel_reg = port_output_sel_addr[port];

    if(sel == SEL_PMU_REG){
        ool_write(sel_reg, (ool_read(sel_reg) & (~bits)));
    }
    else{
        ool_write(sel_reg, (ool_read(sel_reg) | bits));    
    }
}


void pmu_set_gpio_value(enum system_port_t port, uint8_t bits, uint8_t value)
{
    uint8_t port_output_val[] = {PMU_REG_PORTA_OUTPUT_VAL,PMU_REG_PORTB_OUTPUT_VAL,PMU_REG_PORTC_OUTPUT_VAL,PMU_REG_PORTD_OUTPUT_VAL};
    uint8_t sel_reg = port_output_val[port];
    if( value == 0 )
        ool_write(sel_reg, (ool_read(sel_reg) & (~bits)) );
    else
        ool_write(sel_reg, (ool_read(sel_reg) | bits ) );
    
}

uint8_t pmu_get_gpio_value(enum system_port_t port, uint8_t bit)
{
    uint8_t port_input_val[] = {PMU_REG_PORTA_INPUT_VAL,PMU_REG_PORTB_INPUT_VAL,PMU_REG_PORTC_INPUT_VAL,PMU_REG_PORTD_INPUT_VAL};
    uint8_t sel_reg = port_input_val[port];
    return ( (ool_read(sel_reg) & CO_BIT(bit))>>bit );
}

void pmu_clear_isr_state(uint16_t state_map)
{
    ool_write16(PMU_REG_INT_CLR_1, state_map);
    co_delay_100us(1);
    //co_delay_10us(12);
    ool_write16(PMU_REG_INT_CLR_1, 0);
}

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
void pmu_set_charge(enum charge_current_t cur, enum charge_term_vol_t vol)
{
    //ool_write(0xA,0x15);
    //ool_write(0xC,0x55);

    //BIT[5:0], charge current
    // 00'0000:29mA; 00'0010:40mA; 00 0100:72mA; 00 1000:113mA; 00 1100:152mA; 01 0000:185mA
    ool_write(PMU_REG_CHG_CTRL_0, (ool_read(PMU_REG_CHG_CTRL_0)&(~ 0x3F)) | (cur<<0) );

    //BIT[2:0], charge termination voltage control
    // 000=4.1; 001=4.15; 010=4.2; 011=4.25; 100=4.3; 101=4.35; 110=4.4;
    ool_write(PMU_REG_CHG_CTRL_3, (ool_read(PMU_REG_CHG_CTRL_3)&(~ 0x7)) | (vol<<0) );
}

extern uint32_t lp_frequency;
uint32_t pmu_get_rc_clk(void)
{
    if(lp_frequency < 40000){
        lp_frequency = 40000;
    }
    return lp_frequency;
}


__attribute__((section("ram_code")))void pmu_shutdown(void)
{
    ool_write(PMU_REG_POWER_MAGIC,PMU_FIRST_MAGIC);
    ool_write32(PMU_REG_BB_SLEEP_VAL0, 0);//sleep duration 
    REG_PL_WR(0x40000030,0x04);//deepsleep en
}

#if 1

extern void charge_isr_ram(uint8_t type);
extern void lvd_isr_ram(void);
extern void otd_isr_ram(void);
extern void pmu_gpio_isr_ram(void);
extern void onkey_isr_ram(void);
extern void pmu_adkey0_isr_ram(void);

__attribute__((weak)) __attribute__((section("ram_code"))) void charge_isr_ram(uint8_t type)
{
    if(type == 2)
    {
        //printf("charge full\r\n");
        pmu_disable_isr(PMU_ISR_BIT_BAT_FULL);
    }
    else if(type == 1)
    {
    
        //pmu_set_gpio_value(GPIO_PORT_B, 1<<GPIO_BIT_0, 0);
        //printf("charge out\r\n");
    }
    else if(type == 0)
    {
    
        //pmu_set_gpio_value(GPIO_PORT_B, 1<<GPIO_BIT_0, 1);
        //printf("charge in\r\n");
    }
}

__attribute__((weak)) __attribute__((section("ram_code"))) void lvd_isr_ram(void)
{
    //printf("lvd\r\n");
    pmu_disable_isr(PMU_INT_LVD);
}

__attribute__((weak)) __attribute__((section("ram_code"))) void otd_isr_ram(void)
{
    //printf("otd\r\n");
    pmu_disable_isr(PMU_INT_OTD);
}
__attribute__((weak)) __attribute__((section("ram_code"))) void pmu_gpio_isr_ram(void)
{
    uint32_t gpio_value = ool_read32(PMU_REG_PORTA_INPUT_VAL);
    
    //printf("gpio new value = 0x%08x.\r\n", gpio_value);
    ool_write32(PMU_REG_STATE_MON_IN_A,gpio_value);
}

__attribute__((weak)) __attribute__((section("ram_code"))) void onkey_isr_ram(void)
{
    printf("onkey isr\r\n");
}

__attribute__((weak)) __attribute__((section("ram_code"))) void pmu_adkey0_isr_ram(void)
{
    printf("adkey 0 isr\r\n");
}

__attribute__((weak)) __attribute__((section("ram_code"))) void pmu_adkey1_isr_ram(void)
{
    printf("adkey 1 isr\r\n");
}

__attribute__((weak)) __attribute__((section("ram_code"))) void rtc_isr_ram(void)
{
    printf("rtc isr\r\n");
}

__attribute__((weak)) __attribute__((section("ram_code"))) void pmu_onkey_isr_ram(void)
{
    printf("onkey isr\r\n");
}


#endif
