#include "driver_pdm.h"
#include "driver_plf.h"

#define PDM0_REG_BASE           ((volatile struct pdm_reg_t *)PDM0_BASE)
#define PDM1_REG_BASE           ((volatile struct pdm_reg_t *)PDM1_BASE)


void pdm_init(uint8_t index)
{
    volatile struct pdm_reg_t *pdm_reg;
    if(index == 0){
        pdm_reg = PDM0_REG_BASE;
    }else{
        pdm_reg = PDM1_REG_BASE;
    }
    pdm_reg->cfg.enable = 1;
    pdm_reg->cfg.hpf_en = 1;
    pdm_reg->cfg.lfifo_apb_sel = 0;
    pdm_reg->cfg.rfifo_apb_sel = 0;
    pdm_reg->cfg.rfifo_en = 1;
    pdm_reg->cfg.lfifo_en = 1;
    pdm_reg->cfg.esco_mask = 1; // 1--- for voice call, 0---for only pdm test
    pdm_reg->cfg.mono  = 0;
    pdm_reg->cfg.sr_mode = 1;//1;8k 0:16k

    pdm_reg->pdm_fifo_inten = bmINTEN_RFFAFULLINTEN | bmINTEN_LFFAFULLINTEN;

    // set almost full Threshold level
    // Control the level of entries (or above) that triggers the
    // AFULL interrupt. The valid range is 0-63.
    // Reset value: 0x20
    pdm_reg->aflr.left_aflr = 30;
    pdm_reg->aflr.right_aflr = 30;
    
    // RXFIFO pointer clear
    pdm_rfifo_wptr_clr(index);
    pdm_rfifo_rptr_clr(index);
    pdm_lfifo_wptr_clr(index);
    pdm_lfifo_rptr_clr(index);

}

void pdm_stop(uint8_t index)
{
    volatile struct pdm_reg_t *pdm_reg;
    if(index == 0){
        pdm_reg = PDM0_REG_BASE;
    }else{
        pdm_reg = PDM1_REG_BASE;
    }

    pdm_reg->cfg.enable = 0;
}

void  pdm_lfifo_wptr_clr(uint8_t index)
{
    volatile struct pdm_reg_t *pdm_reg;
    if(index == 0){
        pdm_reg = PDM0_REG_BASE;
    }else{
        pdm_reg = PDM1_REG_BASE;
    }

    pdm_reg->clr.left_wr_clr = 1;
    pdm_reg->clr.left_wr_clr = 0;

}

void  pdm_lfifo_rptr_clr(uint8_t index)
{
    volatile struct pdm_reg_t *pdm_reg;
    if(index == 0){
        pdm_reg = PDM0_REG_BASE;
    }else{
        pdm_reg = PDM1_REG_BASE;
    }

    pdm_reg->clr.left_rd_clr = 1;
    pdm_reg->clr.left_rd_clr = 0;

}

void  pdm_rfifo_wptr_clr(uint8_t index)
{
    volatile struct pdm_reg_t *pdm_reg;
    if(index == 0){
        pdm_reg = PDM0_REG_BASE;
    }else{
        pdm_reg = PDM1_REG_BASE;
    }

    pdm_reg->clr.right_wr_clr = 1;
    pdm_reg->clr.right_wr_clr = 0;

}

void  pdm_rfifo_rptr_clr(uint8_t index)
{
    volatile struct pdm_reg_t *pdm_reg;
    if(index == 0){
        pdm_reg = PDM0_REG_BASE;
    }else{
        pdm_reg = PDM1_REG_BASE;
    }

    pdm_reg->clr.right_rd_clr = 1;
    pdm_reg->clr.right_rd_clr = 0;

}

// volgain range from 0~4095, 0 denotes mute, default value D(370)
// volgain Vs dB: 20xlog10((volgain)/256) dB
// stepctrl used to control volume changing speed: each (stepctrl+1)
// clk samples(8k/16k), volume ++ or --
// stepctrl range from 0 ~ 1023, default value 0
void  pdm_volume_ctrl(uint8_t index,uint32_t stepctrl, uint32_t volgain_r, uint32_t volgain_l)
{
    volatile struct pdm_reg_t *pdm_reg;
    if(index == 0){
        pdm_reg = PDM0_REG_BASE;
    }else{
        pdm_reg = PDM1_REG_BASE;
    }
    pdm_reg->volstep.vol_step = stepctrl;
    pdm_reg->volgain.volgain_l = volgain_l;
    pdm_reg->volgain.volgain_r = volgain_r;
}


