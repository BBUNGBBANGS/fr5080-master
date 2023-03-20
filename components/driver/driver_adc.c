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
#include "driver_adc.h"

#define ADC_CHANNEL_MAX         8
#define ADC_SAMPLE_RATE         500000

/*
 * TYPEDEFS
 */
struct adc_ctrl_t {
    uint32_t adc_en:1;
    uint32_t fifo_en:1;
    uint32_t sample_mode:1; // 0: channel loop mode; 1: fixed channel
    uint32_t data_valid:1;
    uint32_t ch_sel:3;  //valid in fixed channel mode
    uint32_t reserved0:1;
    uint32_t ch_en:8;
    uint32_t err_int_clr:1;
    uint32_t reserved1:15;
};

struct adc_data_t {
    uint32_t data:10;
    uint32_t reserved0:22;
};

struct adc_int_ctrl_t {
    uint32_t ff_int_en:1;   // fifo full interrupt enable
    uint32_t fhf_int_en:1;  // fifo half full interrupt enable
    uint32_t fe_int_en:1;   // fifo empty interrupt enable
    uint32_t fhe_int_en:1;  // fifo half empty interrupt enable
    uint32_t err_int_en:1;  // adc error interrupt enable
    uint32_t reserved0:3;
    uint32_t ff_status:1;   // fifo full status
    uint32_t fhf_status:1;  // fifo half full status
    uint32_t fe_status:1;   // fifo empty status
    uint32_t fhe_status:1;  // fifo half empty status
    uint32_t err_status:1;
    uint32_t reserved1:19;
};

struct adc_cfg_t {
    uint32_t adc_pd:1;
    uint32_t adc_rstn:1;
    uint32_t clk_edge_sel:1;    // 1: negtive; 0: positive
    uint32_t input_swap:1;
    uint32_t ref_sel:1;     // 1: internal 1.2v; 0: VDD_IO
    uint32_t reserved0:1;
    uint32_t vbat_div_en:1; // Is vbat divided by 4 before sampled by AD convertor
    uint32_t vbe_buf_en:1;
    uint32_t refp1p2_buf_rl:4;
    uint32_t cap_trim:3;
    uint32_t refp1p2_buf_en:1;
    uint32_t adc_iso_enn:1;
    uint32_t reserved1:15;
};

struct adc_t {
    struct adc_ctrl_t ctrl;
    struct adc_data_t ch_data[ADC_CHANNEL_MAX];
    struct adc_data_t fifo_data;
    struct adc_int_ctrl_t int_ctrl;
    struct adc_cfg_t cfg;
};

/*
 * LOCAL VARIABLES (本地变量)
 */
struct adc_t *adc_env = (struct adc_t *)AUXADC_BASE;

/*********************************************************************
 * @fn      adc_init
 *
 * @brief   init AD convert.
 *
 * @param   channel_func    - select channel function,reference enum adc_channel_func_t.
 *                        
 *          fifo_en        -  enable fifo or not
 *          ref_sel        -  //1---1.2v, 0---vddio
 *			
 * @return  None.
 */
void adc_init(enum adc_channel_func_t channel_func, uint8_t fifo_en,uint8_t ref_sel)
{
    adc_env->ctrl.ch_sel = channel_func;

    adc_env->ctrl.fifo_en = fifo_en;
    adc_env->ctrl.sample_mode = 1; //fixed mode
    
    adc_env->cfg.ref_sel = ref_sel; //1---1.2v, 0---vddio
    adc_env->cfg.vbat_div_en = 1; //vbat/4

    adc_env->cfg.refp1p2_buf_rl = 0x0f;
    adc_env->cfg.cap_trim = 4;
    if(ref_sel)
        adc_env->cfg.refp1p2_buf_en =1;
    else
        adc_env->cfg.refp1p2_buf_en =0;
}

void adc_start(void)
{
    adc_env->cfg.adc_iso_enn = 1;
    adc_env->cfg.adc_pd = 0;
    adc_env->cfg.adc_rstn = 1;
    for(uint8_t i =0;i<10;i++){
        
    }
    adc_env->ctrl.adc_en = 1;
}

/*********************************************************************
 * @fn      adc_stop
 *
 * @brief   stop AD convert.
 *
 * @param   None
 *			
 * @return  None.
 */
void adc_stop(void)
{
    adc_env->ctrl.adc_en = 0;
    adc_env->cfg.adc_iso_enn = 0;
    adc_env->cfg.adc_pd = 1;
    adc_env->cfg.adc_rstn = 0;
}

/*********************************************************************
 * @fn      adc_enable_fifo
 *
 * @brief   enable adc fifo, fifo should not be enable in LOOP mode.
 *
 * @param   int_type    - represent which interrupts should be enable
 *			
 * @return  None.
 */
void adc_enable_fifo(uint8_t int_type)
{
    if(adc_env->ctrl.sample_mode == ADC_MODE_FIXED) {
        while(adc_env->int_ctrl.fe_status == 0) {
            volatile uint32_t data = adc_env->fifo_data.data;
        }
        *(uint32_t *)&adc_env->int_ctrl = int_type;
        adc_env->ctrl.fifo_en = 1;
    }
}

/*********************************************************************
 * @fn      adc_disable_fifo
 *
 * @brief   disable adc fifo.
 *
 * @param   None
 *			
 * @return  None.
 */
void adc_disable_fifo(void)
{
    adc_env->ctrl.fifo_en = 0;
}

/*********************************************************************
 * @fn      adc_get_fifo_status
 *
 * @brief   get current fifo status.
 *
 * @param   None
 *			
 * @return  status, only bit[12:8] are avaliable.
 */
uint32_t adc_get_fifo_status(void)
{
    return *(uint32_t *)&adc_env->int_ctrl;
}

uint8_t is_adc_data_valid(void)
{
    return adc_env->ctrl.data_valid;
}

/*********************************************************************
 * @fn      adc_read_data
 *
 * @brief   read data value.
 *
 * @param   None.
 *			
 * @return  fifo value.
 */
uint16_t adc_read_data(void)
{
    return adc_env->fifo_data.data;
}


