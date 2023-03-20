#include <stdint.h>

#include "driver_syscntl.h"
#include "driver_plf.h"
#include "driver_ipc.h"

//extern void syscntl_init_rom(void);

uint8_t system_clk_map[] = {48, 24, 16, 12};
volatile struct system_regs_t * system_regs = (volatile struct system_regs_t *)(SYSTEM_REG_BASE);;
extern uint8_t system_clk;

uint32_t system_get_pclk(void)
{
    return system_clk*1000000;
}

__attribute__((section("ram_code")))void system_set_pclk(uint8_t clk)
{
    system_regs->clk_ctrl.clk_bb_div = clk;
    system_clk = system_clk_map[clk];
}

__attribute__((section("ram_code")))uint8_t system_get_pclk_config(void)
{
    //return system_regs->clk_ctrl.clk_bb_div;
	return system_clk;
}

__attribute__((section("ram_code"))) void system_set_port_pull_up(uint32_t port, uint8_t pull)
{

    if(pull) {
        system_regs->pad_he |= port;
        system_regs->pad_len |= port;
    }
    else {
        system_regs->pad_he &= (~port);
    }
}

void system_set_port_pull_down(uint32_t port, uint8_t pull)
{

    if(pull) {
        system_regs->pad_len &= (~port);
        system_regs->pad_he &= (~port);
    }
    else {
        system_regs->pad_len |= port;
    }
}

__attribute__((section("ram_code"))) void system_set_port_mux(uint8_t port, uint8_t bit, uint8_t func)
{
    uint32_t value;
    
    value = system_regs->port_mux[port];

    value &= (~(SYSTEM_PORT_MUX_MSK<<(SYSTEM_PORT_MUX_LEN*bit)));
    value |= (func << (SYSTEM_PORT_MUX_LEN*bit));

    system_regs->port_mux[port] = value;

}

void system_reset_dsp_set(void)
{
    system_regs->reset_ctrl_level.dsp_mas_sft_rst = 1;
}

void system_reset_dsp_release(void)
{
    system_regs->reset_ctrl_level.dsp_mas_sft_rst = 0;
}

void system_reset_dsp(void)
{
    system_regs->reset_ctrl.dsp_mas_sft_rst = 1;
}

void system_set_dsp_clk(enum system_dsp_clk_src_sel_t src, uint8_t div)
{
    system_regs->clk_enable.dsp_mas_cken = 0;
    system_regs->clk_ctrl.dsp_clk_sel = src;
    system_regs->clk_ctrl.dsp_mas_clk_div = div;
    system_regs->clk_enable.dsp_mas_cken = 1;
}

void system_set_dsp_vector(uint32_t vector)
{
    system_regs->dsp_reset_vector = vector;
}

void system_set_dsp_mem_config(enum system_dsp_mem_ctrl_t ctrl)
{
    system_regs->dsp_mem_ctrl.mux = ctrl;
}



void system_set_voice_config(void)
{
    system_regs->misc_ctrl.audio_src = 0x00;
    if((ipc_get_mic_type() == IPC_MIC_TYPE_I2S)||(ipc_get_spk_type() == IPC_SPK_TYPE_I2S)){
        system_regs->misc_ctrl.audio_format = 1;//voice to external i2s from dsp
        system_regs->misc_ctrl.audio_dst = 0x01;
    }else{
        system_regs->misc_ctrl.audio_format = 0x00;
        system_regs->misc_ctrl.audio_dst = 0x00;
    }
    system_regs->misc_ctrl.sbc2cdc_en = 0;
    system_regs->misc_ctrl.sbc2iis_en = 0;
    
    system_regs->cdc_mas_clk_adj.cdc_adj_bp = 1;
}

void system_set_sbc_config(void)
{
    system_regs->misc_ctrl.audio_src = 0x00;
    if(ipc_get_spk_type() == IPC_SPK_TYPE_I2S){
        system_regs->misc_ctrl.sbc2cdc_en = 0;
        system_regs->misc_ctrl.sbc2iis_en = 1;
        system_regs->misc_ctrl.audio_format = 0x01;
        system_regs->misc_ctrl.audio_dst = 0x01;
    }else{
        system_regs->misc_ctrl.sbc2cdc_en = 1;
        system_regs->misc_ctrl.sbc2iis_en = 0;
        system_regs->misc_ctrl.audio_format = 0x00;
        system_regs->misc_ctrl.audio_dst = 0x00;
    }
}

void system_set_aac_config(void)
{
    system_regs->misc_ctrl.audio_src = 0x00;
    system_regs->misc_ctrl.sbc2cdc_en = 0;
    system_regs->misc_ctrl.sbc2iis_en = 0;
    if(ipc_get_spk_type() == IPC_SPK_TYPE_I2S){
        system_regs->misc_ctrl.audio_format = 0x01;
        system_regs->misc_ctrl.audio_dst = 0x01;
    }else{
        system_regs->misc_ctrl.audio_format = 0x00;
        system_regs->misc_ctrl.audio_dst = 0x00;
    }
}

