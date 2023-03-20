#include <stdint.h>
#include <string.h>

#include "pskeys.h"
#include "user_utils.h"

#include "driver_codec.h"
#include "driver_ipc.h"
#include "driver_syscntl.h"
#include "driver_i2s.h"
#include "driver_pmu.h"
#include "usb_audio.h"

#include "os_timer.h"
#include "co_list.h"
#include "os_mem.h"

#define ENABLE_EQ             1


os_timer_t audio_codec_timer;
uint8_t dsp_nrec_start_flag = false;

#if ENABLE_EQ
const uint32_t speech_spk_eq_param[] = {
    0x00004000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000041d7, 0x0001866c, 0x00003874, 
    0x000079cb, 0x0001c5ec, 0x00003cc1, 0x00019000, 0x0000363b, 0x00007000, 0x0001cd03, 0x00005a67, 
    0x0001edc7, 0x00001029, 0x0001f31a, 0x0001f490, 0x00003f08, 0x000181ee, 0x00003f08, 0x00007df9, 
    0x0001c1d6, 0x00003d82, 0x000184fb, 0x00003d82, 0x00007aec, 0x0001c4e3, 0x00003ca9, 0x000186ad, 
    0x00003ca9, 0x0000793b, 0x0001c695, 0x00004000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
    0x00004000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00004000, 0x00000000, 0x00000000, 
    0x00000000, 0x00000000, 0x00004000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00007ffd, 
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};
const uint32_t speech_mic_eq_param[] = {
    0x00004000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x000041d7, 0x0001866c, 0x00003874, 
    0x000079cb, 0x0001c5ec, 0x00003cc1, 0x00019000, 0x0000363b, 0x00007000, 0x0001cd03, 0x00005a67, 
    0x0001edc7, 0x00001029, 0x0001f31a, 0x0001f490, 0x00003f08, 0x000181ee, 0x00003f08, 0x00007df9, 
    0x0001c1d6, 0x00003d82, 0x000184fb, 0x00003d82, 0x00007aec, 0x0001c4e3, 0x00003ca9, 0x000186ad, 
    0x00003ca9, 0x0000793b, 0x0001c695, 0x00004000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
    0x00004000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00004000, 0x00000000, 0x00000000, 
    0x00000000, 0x00000000, 0x00004000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00007ffd, 
    0x00000000, 0x00000000, 0x00000000, 0x00000000
};

const uint32_t music_eq_param[] = { 
    0x00002000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00003f90, 0x00018111, 0x00003f5f, 
    0x00007eef, 0x0001c110, 0x00003e3c, 0x000185ac, 0x00003d37, 0x00007a54, 0x0001c48c, 0x00002cc9, 
    0x0001d3e8, 0x0000247a, 0x00002c18, 0x0001eebc, 0x0000403d, 0x0001820d, 0x00003de9, 0x00007df3, 
    0x0001c1da, 0x00004094, 0x00018516, 0x00003af2, 0x00007aea, 0x0001c47a, 0x00004012, 0x00018141, 
    0x00003eb8, 0x00007ebf, 0x0001c135, 0x00004066, 0x00018376, 0x00003c85, 0x00007c8a, 0x0001c315, 
    0x000048f9, 0x0001a081, 0x00001ff5, 0x00005f7f, 0x0001d711, 0x00004160, 0x00019330, 0x00003001, 
    0x00006cd0, 0x0001ce9f, 0x00004000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00007ffd, 
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00002000, 0x00000000, 0x00000000, 0x00000000, 
    0x00000000, 0x00003f90, 0x00018111, 0x00003f5f, 0x00007eef, 0x0001c110, 0x00003e3c, 0x000185ac, 
    0x00003d37, 0x00007a54, 0x0001c48c, 0x00002cc9, 0x0001d3e8, 0x0000247a, 0x00002c18, 0x0001eebc, 
    0x0000403d, 0x0001820d, 0x00003de9, 0x00007df3, 0x0001c1da, 0x00004094, 0x00018516, 0x00003af2, 
    0x00007aea, 0x0001c47a, 0x00004012, 0x00018141, 0x00003eb8, 0x00007ebf, 0x0001c135, 0x00004066, 
    0x00018376, 0x00003c85, 0x00007c8a, 0x0001c315, 0x000048f9, 0x0001a081, 0x00001ff5, 0x00005f7f, 
    0x0001d711, 0x00004160, 0x00019330, 0x00003001, 0x00006cd0, 0x0001ce9f, 0x00004000, 0x00000000, 
    0x00000000, 0x00000000, 0x00000000, 0x00007ffd, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 
};

void codec_eq_param_switch(void)
{
    REG_PL_WR(0x50022318,0x00000001);
}

void codec_eq_param_set(uint8_t mode)
{
    if(mode == 0){
        ///music
        memcpy((void *)0x50022134,(void *)music_eq_param, sizeof(music_eq_param));
        #if 1
        REG_PL_WR(0x50022318,0x00000001);
        REG_PL_WR(0x50022080,0x3f);
        REG_PL_WR(0x50022084,0x3f);
        REG_PL_WR(0x50022090,0x02);
        REG_PL_WR(0x500220a8,0x03);
        REG_PL_WR(0x500220e4,0x10);
        REG_PL_WR(0x500220e8,0x16);
        digital_codec_reg->adc_cfg.dec_gpf_en = 0;
        #else
        /* Mix left and right */
        REG_PL_WR(0x50022318,0x00000001);
        REG_PL_WR(0x50022088,0x01);
        REG_PL_WR(0x500220C0,0x23);
        REG_PL_WR(0x50022080,0x3f);
        REG_PL_WR(0x50022084,0x3f);
        REG_PL_WR(0x50022090,0x04);
        REG_PL_WR(0x500220a8,0x04);
        REG_PL_WR(0x500220e4,0x10);
        REG_PL_WR(0x500220e8,0x16);
        digital_codec_reg->adc_cfg.dec_gpf_en = 0;
        #endif
    }
    else{
        ///speech
        memcpy((void *)0x50022134,(void *)speech_spk_eq_param, sizeof(speech_spk_eq_param));
        memcpy((void *)0x50022224,(void *)speech_mic_eq_param, sizeof(speech_mic_eq_param));
        REG_PL_WR(0x50022318,0x00000001);
        REG_PL_WR(0x50022080,0x3f);
        REG_PL_WR(0x50022084,0x3f);
        REG_PL_WR(0x50022090,0x03);
        REG_PL_WR(0x500220a8,0x00);
        REG_PL_WR(0x500220e8,0x10);
        REG_PL_WR(0x500220ec,0x16);
        digital_codec_reg->adc_cfg.dec_gpf_en = 1;
    }
    #if 0
    REG_PL_WR(0x50022080,0x3f);
    REG_PL_WR(0x50022084,0x3f);
    REG_PL_WR(0x50022090,0x02);
    REG_PL_WR(0x500220a8,0x03);
    REG_PL_WR(0x500220e4,0x10);
    REG_PL_WR(0x500220e8,0x16);
    
    digital_codec_reg->dac_cfg.dac_gpf_en = 1;
    digital_codec_reg->sys_cfg.gpf_nrst_rst = 0;
    digital_codec_reg->sys_cfg.gpf_nrst_rst = 1;
    digital_codec_reg->sys_cfg.gpf_en = 1;              // enable gpf
    #else

    digital_codec_reg->dac_cfg.dac_gpf_en = 1;
    digital_codec_reg->sys_cfg.gpf_nrst_rst = 0;
    digital_codec_reg->sys_cfg.gpf_nrst_rst = 1;
    digital_codec_reg->sys_cfg.gpf_en = 1;              // enable gpf

    #endif
}
#endif

void audio_codec_timer_func(void *arg)
{
    if(ipc_get_spk_type() == IPC_SPK_TYPE_I2S){
        //analog pd
        REG_PL_WR(0x50022360,0x3b); // adc en
        REG_PL_WR(0x50022368,0xff); // pa&mix pd
        REG_PL_WR(0x5002236c,0x20); // pga pu
        //i2s_init_imp(8000);
    }
    else{
        
        //printf("codec pu\r\n");
        //analog pd
        REG_PL_WR(0x50022360,0x00); // adc en
        REG_PL_WR(0x50022368,0x00); // pa&mix pd
        REG_PL_WR(0x5002236c,0x20); // pga pu
        REG_PL_WR(0x50022378,0x18); // mic config, single

        //ʨ׃spk Ӵ
        REG_PL_WR(0x50022000+(0xd6<<2),pskeys.app_data.hf_vol);
        REG_PL_WR(0x50022000+(0xd7<<2),pskeys.app_data.hf_vol);
        
        REG_PL_WR(0x50022000+(0xdf<<2),0x2f);   // mic gain
    }
    ///unmute source (mic data)
    REG_PL_WR(0x400005D4, (REG_PL_RD(0x400005D4) & ~((uint32_t)0x00400000)));
    
}

void codec_power_off(void)
{
    REG_PL_WR(0x50022360,0x7f);// pd
    REG_PL_WR(0x50022368,0xff);
    REG_PL_WR(0x5002236c,0x3f);
}

/* configure codec working in SCO mode */
void codec_audio_reg_config(void)
{
    uint32_t count = 0;
    system_set_voice_config();
     
     /* disable audio codec first */
    *(uint32_t *)&digital_codec_reg->sys_cfg = 0x00;
    *(uint32_t *)&digital_codec_reg->sys_cfg = 0x30;
    *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0x00;
    
    *(uint32_t *)&digital_codec_reg->sample_rate = 0x00;    // 8K sample rate
    
    digital_codec_reg->adcff_cfg0.aflr = 30;
    digital_codec_reg->dacff_cfg0.aelr = 34;
    
    /* clear dac fifo first */
    digital_codec_reg->esco_mask.esco_mask = 0;
    digital_codec_reg->dacff_cfg1.apb_sel = 1;
    digital_codec_reg->dacff_cfg1.enable = 1;
    for(uint8_t i=0; i<64; i++) {
        digital_codec_reg->dacff_data = 0x00;
    }
    *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0x00;

    ool_write(0xad,0x00); // audio buck&ldo pu 

    *(uint32_t *)&digital_codec_reg->dac_cfg = 0x0a;        // mono, right channel is used
    digital_codec_reg->i2s_cfg.lrswap = 1;
    digital_codec_reg->i2s_cfg.format = 2;
    digital_codec_reg->dac_cfg1.cpy_when_mono = 0;                 // copy output data from left to right
    
    //digital_codec_reg->esco_mask.esco_mask = 0;
    digital_codec_reg->dac_cfg1.int_din_src = 0;
    *(uint32_t *)&digital_codec_reg->sys_cfg = 0x33;
    #if 1
    GLOBAL_INT_DISABLE();
    uint32_t org_diag_cfg = *(volatile uint32_t *)0x40000450;
    *(volatile uint32_t *)0x40000450 = (org_diag_cfg & 0xffff00ff) | 0x0000b200;
    uint32_t org_diag_value = *(volatile uint32_t *)0x40000454;
    org_diag_value &= 0x00000200;
    do {
        uint32_t tmp_diag_value = (*(volatile uint32_t *)0x40000454) & 0x00000200;
        if(count>120){
            break;
        }
        count++;
        co_delay_100us(1);
        if(tmp_diag_value != org_diag_value) {
            break;
        }
    } while(1);
    
    *(uint32_t *)&digital_codec_reg->adcff_cfg1 = 0xb4;
    *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0x94;

    ipc_config_voice_dma();
    *(volatile uint32_t *)0x40000450 = org_diag_cfg;
    GLOBAL_INT_RESTORE();
    #else
    *(uint32_t *)&digital_codec_reg->adcff_cfg1 = 0xb4;
    *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0x94;
    
    ipc_config_voice_dma();
    #endif
    //printf("codec audio config\r\n");
    //os_timer_init(&audio_codec_timer, audio_codec_timer_func,NULL);
    //os_timer_start(&audio_codec_timer, 100, 0);

#if ENABLE_EQ
    codec_eq_param_set(1);
#endif

#ifndef CFG_FT_CODE
    if(dsp_nrec_start_flag == true){
        os_timer_init(&audio_codec_timer, audio_codec_timer_func,NULL);
        os_timer_start(&audio_codec_timer, 100, 0);
    }
#endif
#if 0
    if(ipc_get_spk_type() == IPC_SPK_TYPE_I2S){
        //analog pd
        REG_PL_WR(0x50022360,0x3b);// adc en
        REG_PL_WR(0x50022368,0xff); //pa&mix pd
        REG_PL_WR(0x5002236c,0x20); //pga pu
        //i2s_init_imp(8000);
    }else{
        //analog pd
        REG_PL_WR(0x50022360,0x00);// adc en
        REG_PL_WR(0x50022368,0x00); //pa&mix pd
        REG_PL_WR(0x5002236c,0x20); //pga pu
        REG_PL_WR(0x50022378,0x18); // mic config, single

        //ʨ׃spk Ӵ
        REG_PL_WR(0x50022000+(0xd6<<2),pskeys.app_data.hf_vol);
        REG_PL_WR(0x50022000+(0xd7<<2),pskeys.app_data.hf_vol);
        
        REG_PL_WR(0x50022000+(0xdf<<2),0x2f);//mic gain
    }
#endif

    if(ipc_get_mic_type() == IPC_MIC_TYPE_I2S) {
        i2s_init_(I2S_DIR_TX, 2000000, 8000, I2S_MODE_MASTER);
        i2s_set_tx_src(I2S_TX_SRC_IPC_MCU);
        i2s_start_without_int();
    }
    else if(ipc_get_mic_type() == IPC_MIC_TYPE_PDM){
        system_set_port_mux(GPIO_PORT_C, GPIO_BIT_2, PORTC2_FUNC_PDM1_SCK);
        system_set_port_mux(GPIO_PORT_C, GPIO_BIT_3, PORTC3_FUNC_PDM1_SDA);
        pdm_init(1);
    }
}

/* 
 * configure codec working in aac mode, aac is decoded in DSP, PCM data
 * are transfered via IPC DMA from DSP to codec
 */
void codec_aac_reg_config(uint8_t sample_rate)
{
    system_set_aac_config();

#if 0
    digital_codec_reg->sample_rate.dac_sr = sample_rate;
    ///dac fifo enable
    digital_codec_reg->dacff_cfg1.enable = 1;
    ///dac enable
    digital_codec_reg->sys_cfg.dac_en = 1;
    
    
    //digital_codec_reg->dacff_cfg1.aept_int_en = 1;
    //digital_codec_reg->dacff_cfg1.full_int_en = 1;
    digital_codec_reg->dac_cfg1.dacff_clk_inv_sel = 1;
#else
    codec_dac_reg_config(sample_rate<<4, AUDIO_CODEC_DAC_SRC_IPC_DMA);
    //codec_dac_reg_config(AUDIO_CODEC_SAMPLE_RATE_DAC_44100, AUDIO_CODEC_DAC_SRC_IPC_DMA);

#endif
    ipc_config_media_dma(1);
    
}

void codec_test_aac_reg_config(uint8_t sample_rate)
{
    system_set_aac_config();
    
    {
         /* disable audio codec first */
        *(uint32_t *)&digital_codec_reg->sys_cfg = 0x00;
        
        *(uint32_t *)&digital_codec_reg->sample_rate = sample_rate;    // 8K sample rate 0x00;16k sample rate 0x02;
        
        digital_codec_reg->dacff_cfg0.aelr = 32;

        ool_write(0xad,0x00); // audio buck&ldo pu 

        *(uint32_t *)&digital_codec_reg->sys_cfg = 0x22;
        *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0x94;
        
        //digital_codec_reg->dacff_cfg1.aept_int_en = 1;
        //digital_codec_reg->dacff_cfg1.full_int_en = 1;
        digital_codec_reg->dac_cfg1.dacff_clk_inv_sel = 1;

        if(sample_rate == 0xbb) {  // 44100
            system_regs->cdc_mas_clk_adj.cdc_adj_bp = 0;
        }
        else {
            system_regs->cdc_mas_clk_adj.cdc_adj_bp = 1;
        }

        if(ipc_get_spk_type() == IPC_SPK_TYPE_CODEC){
            ool_write(0xad,0x00); // audio buck&ldo pu 
            //analog pd
            REG_PL_WR(0x50022360,0x44);// dac en
            REG_PL_WR(0x50022368,0x00); //pa&mix pu
            REG_PL_WR(0x5002236c,0x2f); //pga pd
            REG_PL_WR(0x50022000+(0xd6<<2),pskeys.app_data.a2dp_vol);
            REG_PL_WR(0x50022000+(0xd7<<2),pskeys.app_data.a2dp_vol);
        }
        else if(ipc_get_spk_type() == IPC_SPK_TYPE_I2S) {
            if(sample_rate == 0xbb) {
                i2s_init_(I2S_DIR_TX, 12000000, 44100, I2S_MODE_MASTER);
            }
            else {
                i2s_init_(I2S_DIR_TX, 12000000, 48000, I2S_MODE_MASTER);
            }
            i2s_set_tx_src(I2S_TX_SRC_SBC_DECODER);
            i2s_start_without_int();
        }
    }
}

/* 
 * configure codec working in sbc mode, sbc is decoded in hardware SBC decoder,
 * PCM data are transfered via IPC DMA from decoder to codec
 */
void codec_sbc_reg_config(uint8_t sample_rate)
{
    system_set_sbc_config();
    
    ipc_config_media_dma_disable();

    digital_codec_reg->sample_rate.dac_sr = sample_rate;
    ///dac fifo enable
    digital_codec_reg->dacff_cfg1.enable = 1;
    ///dac enable
    digital_codec_reg->sys_cfg.dac_en = 1;
    

    //digital_codec_reg->dacff_cfg1.aept_int_en = 1;
    //digital_codec_reg->dacff_cfg1.full_int_en = 1;
    digital_codec_reg->dac_cfg1.dacff_clk_inv_sel = 1;

    if(sample_rate == 0x0b) {  // 44100
        system_regs->cdc_mas_clk_adj.cdc_adj_bp = 0;
    }
    else {
        system_regs->cdc_mas_clk_adj.cdc_adj_bp = 1;
    }
    
#if ENABLE_EQ
    codec_eq_param_set(0);
#endif

    if(ipc_get_spk_type() == IPC_SPK_TYPE_CODEC){
        ool_write(0xad,0x00); // audio buck&ldo pu 
        //analog pd
        REG_PL_WR(0x50022360,0x44);// dac en
        REG_PL_WR(0x50022368,0x00); //pa&mix pu
        REG_PL_WR(0x5002236c,0x2f); //pga pd
        REG_PL_WR(0x50022000+(0xd6<<2),pskeys.app_data.a2dp_vol);
        REG_PL_WR(0x50022000+(0xd7<<2),pskeys.app_data.a2dp_vol);
    }
    else if(ipc_get_spk_type() == IPC_SPK_TYPE_I2S) {
        if(sample_rate == 0x0b) {
            i2s_init_(I2S_DIR_TX, 12000000, 44100, I2S_MODE_MASTER);
        }
        else {
            i2s_init_(I2S_DIR_TX, 12000000, 48000, I2S_MODE_MASTER);
        }
        i2s_set_tx_src(I2S_TX_SRC_SBC_DECODER);
        i2s_start_without_int();
    }
}

/* configure codec working in tone mode, the sample rate is 16K */
uint8_t codec_tone_reg_config(uint8_t tone_index)
{
    uint8_t ret = 0;
    if(tone_index == 0){
        ///һҥ؅ߪܺӴ
        ret = 1;
    }else{
        system_set_sbc_config();
        ipc_config_media_dma_disable();
        digital_codec_reg->sample_rate.dac_sr = 0x02; //16k
        ///dac fifo enable
        digital_codec_reg->dacff_cfg1.enable = 1;
        ///dac enable
        digital_codec_reg->sys_cfg.dac_en = 1;
    
        //digital_codec_reg->dacff_cfg1.aept_int_en = 1;
        //digital_codec_reg->dacff_cfg1.full_int_en = 1;
        digital_codec_reg->dac_cfg1.dacff_clk_inv_sel = 1;
        
        if(ipc_get_spk_type() == IPC_SPK_TYPE_CODEC){
            ool_write(0xad,0x00); // audio buck&ldo pu 
            
            //analog pd
            REG_PL_WR(0x50022360,0x44);// dac en
            REG_PL_WR(0x50022368,0x00); //pa&mix pu
            REG_PL_WR(0x5002236c,0x2f); //pga pd
            
            REG_PL_WR(0x50022000+(0xd6<<2),pskeys.app_data.tone_vol);
            REG_PL_WR(0x50022000+(0xd7<<2),pskeys.app_data.tone_vol);
        }
    }
    return ret;
}

void codec_dac_reg_config(uint8_t sample_rate, enum audio_codec_dac_src_t src)
{
    system_set_aac_config();

     /* disable audio codec first */
    *(uint32_t *)&digital_codec_reg->sys_cfg = 0x00;
    /* configure sample rate */
    *(uint32_t *)&digital_codec_reg->sample_rate = sample_rate;
    /* configure fifo almost empty threshold, fifo depth is 64 samples */
    digital_codec_reg->dacff_cfg0.aelr = 32;

    /* audio buck&ldo power on */
    ool_write(0xad,0x00);

    /* enable dac and release dac reset */
    *(uint32_t *)&digital_codec_reg->sys_cfg = 0x22;
    if(src == AUDIO_CODEC_DAC_SRC_IPC_DMA) {
        /* enable dac fifo and empty interrupt, almost empty interrupt */
        *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0x94;
    }
    else {
        /* enable dac fifo */
        *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0x80;
    }
    
    digital_codec_reg->dac_cfg1.dacff_clk_inv_sel = 1;

    if((sample_rate & AUDIO_CODEC_SAMPLE_RATE_DAC_MASK) == AUDIO_CODEC_SAMPLE_RATE_DAC_44100) {  // 44100
        system_regs->cdc_mas_clk_adj.cdc_adj_bp = 0;
    }
    else {
        system_regs->cdc_mas_clk_adj.cdc_adj_bp = 1;
    }

    /* analog power configuration */
    REG_PL_WR(0x50022360,0x44); // remove dac pd
    REG_PL_WR(0x50022368,0x00); // remove pa&mix pd
    REG_PL_WR(0x5002236c,0x2f); // pga pd
    REG_PL_WR(0x50022000+(0xd6<<2),pskeys.app_data.a2dp_vol);
    REG_PL_WR(0x50022000+(0xd7<<2),pskeys.app_data.a2dp_vol);
}

void codec_adc_reg_config(uint8_t sample_rate, enum audio_codec_adc_dest_t dest, uint8_t mic_gain)
{
    /* disable audio codec first */
    *(uint32_t *)&digital_codec_reg->sys_cfg = 0x00;
    /* configure sample rate */
    *(uint32_t *)&digital_codec_reg->sample_rate = sample_rate;
    /* configure fifo almost full threshold, fifo depth is 64 samples */
    digital_codec_reg->adcff_cfg0.aflr = 32;

    /* audio buck&ldo power on */
    ool_write(0xad,0x00);

    digital_codec_reg->esco_mask.esco_mask = 1;
    /* enable dac and release dac reset */
    *(uint32_t *)&digital_codec_reg->sys_cfg = 0x11;
    if(dest == AUDIO_CODEC_ADC_DEST_IPC_DMA) {
        /* enable adc fifo and full interrupt, almost full interrupt */
        *(uint32_t *)&digital_codec_reg->adcff_cfg1 = 0x85;
    }
    else if(dest == AUDIO_CODEC_ADC_DEST_USER){
    /* enable adc fifo and full interrupt, almost full interrupt,enable apb option*/
    *(uint32_t *)&digital_codec_reg->adcff_cfg1 = 0xc5;

    }
    else {
        /* enable adc fifo */
        *(uint32_t *)&digital_codec_reg->adcff_cfg1 = 0x80;
    }

    if((sample_rate & AUDIO_CODEC_SAMPLE_RATE_ADC_MASK) == AUDIO_CODEC_SAMPLE_RATE_ADC_44100) {  // 44100
        system_regs->cdc_mas_clk_adj.cdc_adj_bp = 0;
    }
    else {
        system_regs->cdc_mas_clk_adj.cdc_adj_bp = 1;
    }

    /* analog power configuration */
    REG_PL_WR(0x50022360,0x00);// adc en
    REG_PL_WR(0x50022368,0x00); //pa&mix pd
    REG_PL_WR(0x5002236c,0x20); //pga pu
    REG_PL_WR(0x50022378,0x18); // mic config, single
    REG_PL_WR(0x50022000+(0xdf<<2), mic_gain);//mic gain    0x00~0x3F
}

void codec_dac_adc_reg_config(uint8_t sample_rate, enum audio_codec_dac_src_t src, enum audio_codec_adc_dest_t dest, uint8_t mic_gain)
{
    system_set_voice_config();

    /* disable audio codec first */
    *(uint32_t *)&digital_codec_reg->sys_cfg = 0x00;
    
    /* configure sample rate */
    *(uint32_t *)&digital_codec_reg->sample_rate = sample_rate;

    /* configure fifo almost empty threshold, fifo depth is 64 samples */
    digital_codec_reg->dacff_cfg0.aelr = 34;
    /* configure fifo almost full threshold, fifo depth is 64 samples */
    digital_codec_reg->adcff_cfg0.aflr = 30;

    /* audio buck&ldo power on */
    ool_write(0xad,0x00);
    
    digital_codec_reg->esco_mask.esco_mask = 1;
    
    if(dest == AUDIO_CODEC_ADC_DEST_IPC_DMA) {
        /* enable adc fifo and full interrupt, almost full interrupt */
        *(uint32_t *)&digital_codec_reg->adcff_cfg1 = 0x85;
    }
    else if(dest == AUDIO_CODEC_ADC_DEST_USER){
    /* enable adc fifo and full interrupt, almost full interrupt,enable apb option*/
    *(uint32_t *)&digital_codec_reg->adcff_cfg1 = 0xc5;
    }
    else {
        /* enable adc fifo */
        *(uint32_t *)&digital_codec_reg->adcff_cfg1 = 0x80;
    }
    
    if(src == AUDIO_CODEC_DAC_SRC_IPC_DMA) {
        /* enable dac fifo and empty interrupt, almost empty interrupt */
        *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0x94;
        digital_codec_reg->esco_mask.esco_mask = 0;
    }
    else if(src == AUDIO_CODEC_DAC_SRC_USER){
        /* enable dac fifo and apb function*/
        *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0xc0;
    }
    else {
        /* enable dac fifo */
        *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0x80;
    }
    
    *(uint32_t *)&digital_codec_reg->dac_cfg = 0x1a;        // mono, left channel is used
    //digital_codec_reg->i2s_cfg.lrswap = 1;

    digital_codec_reg->dac_cfg1.cpy_when_mono = 1;
    digital_codec_reg->dac_cfg1.dacff_clk_inv_sel = 1;

    if((sample_rate & AUDIO_CODEC_SAMPLE_RATE_ADC_MASK) == AUDIO_CODEC_SAMPLE_RATE_ADC_44100) {  // 44100
        system_regs->cdc_mas_clk_adj.cdc_adj_bp = 0;
    }
    else {
        system_regs->cdc_mas_clk_adj.cdc_adj_bp = 1;
    }
        
    /* enable adc, dac and release adc, dac reset */
    *(uint32_t *)&digital_codec_reg->sys_cfg = 0x33;

    #if 0
    GLOBAL_INT_DISABLE();
    uint32_t org_diag_cfg = *(volatile uint32_t *)0x40000450;
    uint32_t count;
    *(volatile uint32_t *)0x40000450 = (org_diag_cfg & 0xffff00ff) | 0x0000b200;
    uint32_t org_diag_value = *(volatile uint32_t *)0x40000454;
    org_diag_value &= 0x00000200;
    do {
        uint32_t tmp_diag_value = (*(volatile uint32_t *)0x40000454) & 0x00000200;
        if(count>120){
            break;
        }
        count++;
        co_delay_100us(1);
        if(tmp_diag_value != org_diag_value) {
            break;
        }
    } while(1);
    
    *(uint32_t *)&digital_codec_reg->adcff_cfg1 = 0xb4;
    *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0x94;
    
    ipc_config_voice_dma();
    *(volatile uint32_t *)0x40000450 = org_diag_cfg;
    GLOBAL_INT_RESTORE();
    #endif
    
    //analog pd
    REG_PL_WR(0x50022360,0x00);// dac en
    REG_PL_WR(0x50022368,0x00); //pa&mix pu
    REG_PL_WR(0x5002236c,0x20); //pga pd
    REG_PL_WR(0x50022000+(0xd6<<2),pskeys.app_data.a2dp_vol);
    REG_PL_WR(0x50022000+(0xd7<<2),pskeys.app_data.a2dp_vol);

    REG_PL_WR(0x50022378,0x18); // mic config, single
    REG_PL_WR(0x50022000+(0xdf<<2), mic_gain);//mic gain    0x00~0x3F
}

#if USB_AUDIO_ENABLE
/* usb audio adc dac config */
void codec_usb_audio_adc_dac_config(uint8_t sample_rate, enum audio_codec_dac_src_t src, enum audio_codec_adc_dest_t dest, uint8_t mic_gain)
{
    system_set_voice_config();

    /* disable audio codec first */
    *(uint32_t *)&digital_codec_reg->sys_cfg = 0x00;
    
    /* configure sample rate */
    *(uint32_t *)&digital_codec_reg->sample_rate = sample_rate;

    /* configure fifo almost empty threshold, fifo depth is 64 samples */
    digital_codec_reg->dacff_cfg0.aelr = 16;
    /* configure fifo almost full threshold, fifo depth is 64 samples */
    digital_codec_reg->adcff_cfg0.aflr = 48;

    /* audio buck&ldo power on */
    ool_write(0xad,0x00);
    
    digital_codec_reg->esco_mask.esco_mask = 1;

    if(dest == AUDIO_CODEC_ADC_DEST_IPC_DMA) {
        /* enable adc fifo and full interrupt, almost full interrupt */
        *(uint32_t *)&digital_codec_reg->adcff_cfg1 = 0x85;
    }
    else if(dest == AUDIO_CODEC_ADC_DEST_USER){
    /* enable adc fifo and almost full interrupt,enable apb option*/
    *(uint32_t *)&digital_codec_reg->adcff_cfg1 = 0xc4;
    }
    else {
        /* enable adc fifo */
        *(uint32_t *)&digital_codec_reg->adcff_cfg1 = 0x80;
    }
    
    if(src == AUDIO_CODEC_DAC_SRC_IPC_DMA) {
        /* enable dac fifo and empty interrupt, almost empty interrupt */
        *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0x94;
        digital_codec_reg->esco_mask.esco_mask = 0;
    }
    else if(src == AUDIO_CODEC_DAC_SRC_USER){
        /* enable dac fifo and apb function*/
        *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0xc0;
    }
    else {
        /* enable dac fifo */
        *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0x80;
    }
    
    *(uint32_t *)&digital_codec_reg->dac_cfg = 0x02;        // mono, left channel is used

    digital_codec_reg->dac_cfg1.dacff_clk_inv_sel = 1;

    if((sample_rate & AUDIO_CODEC_SAMPLE_RATE_ADC_MASK) == AUDIO_CODEC_SAMPLE_RATE_ADC_44100) {  // 44100
        system_regs->cdc_mas_clk_adj.cdc_adj_bp = 0;
    }
    else {
        system_regs->cdc_mas_clk_adj.cdc_adj_bp = 1;
    }
      /* enable adc, dac and release adc, dac reset */
    *(uint32_t *)&digital_codec_reg->sys_cfg = 0x33;  

    //analog pd
    REG_PL_WR(0x50022360,0x00);// dac en
    REG_PL_WR(0x50022368,0x00); //pa&mix pu
    REG_PL_WR(0x5002236c,0x20); //pga pd
    REG_PL_WR(0x50022000+(0xd6<<2),pskeys.app_data.a2dp_vol);
    REG_PL_WR(0x50022000+(0xd7<<2),pskeys.app_data.a2dp_vol);

    REG_PL_WR(0x50022378,0x18); // mic config, single
    REG_PL_WR(0x50022000+(0xdf<<2), mic_gain);//mic gain    0x00~0x3F
}
#endif

/* used to power down codec to save power */
void codec_off_reg_config(void)
{
    ipc_config_reset_dma();
    codec_power_off();
    
    if(ipc_get_spk_type() == IPC_SPK_TYPE_I2S){
        REG_PL_WR(I2S_REG_CTRL,0x00);
    }
    ool_write(0xad,0x09); // audio buck&ldo pd 
    
    if(ipc_get_spk_type() == IPC_SPK_TYPE_I2S) {
        i2s_stop_();
    }
}


void codec_mic_loop_reg_config(void)
{
    system_set_voice_config();
    
    /* clear dac fifo first */
    digital_codec_reg->dacff_cfg1.apb_sel = 1;
    digital_codec_reg->dacff_cfg1.enable = 1;
    for(uint8_t i=0; i<64; i++) {
        digital_codec_reg->dacff_data = 0x00;
    }
    *(uint32_t *)&digital_codec_reg->dacff_cfg1 = 0x00;
    #if DSP_MIC_LOOP
    codec_dac_adc_reg_config(AUDIO_CODEC_SAMPLE_RATE_ADC_8000|AUDIO_CODEC_SAMPLE_RATE_DAC_8000, AUDIO_CODEC_DAC_SRC_IPC_DMA,AUDIO_CODEC_ADC_DEST_IPC_DMA,0x3b);
    ipc_config_mic_loop_dma();
    #else
    codec_dac_adc_reg_config(AUDIO_CODEC_SAMPLE_RATE_ADC_16000|AUDIO_CODEC_SAMPLE_RATE_DAC_16000,AUDIO_CODEC_DAC_SRC_USER,AUDIO_CODEC_ADC_DEST_USER, 0x3b);
    NVIC_EnableIRQ(CDC_IRQn);

    #endif
}

void codec_mic_only_reg_config(void)
{
    system_set_voice_config();
    #if DSP_MIC_LOOP
    codec_adc_reg_config(AUDIO_CODEC_SAMPLE_RATE_ADC_8000, AUDIO_CODEC_ADC_DEST_IPC_DMA, 0x3b);
    ipc_config_mic_only_dma();
    #else
    codec_adc_reg_config(AUDIO_CODEC_SAMPLE_RATE_ADC_16000,AUDIO_CODEC_ADC_DEST_USER, 0x3b);
    NVIC_EnableIRQ(CDC_IRQn);
    #endif
}

void codec_native_playback_reg_config(void)
{
    system_set_aac_config();

    codec_dac_reg_config(AUDIO_CODEC_SAMPLE_RATE_DAC_44100, AUDIO_CODEC_DAC_SRC_IPC_DMA);
    ipc_config_media_dma(1);
}

/* used to do codec initialziation */
__attribute__((section("ram_code"))) void codec_init_reg_config(void)
{
    REG_PL_WR(0x50022340,0x08);
    REG_PL_WR(0x50022344,0x05);
    REG_PL_WR(0x50022348,0x23);
    REG_PL_WR(0x5002234c,0x00);
    REG_PL_WR(0x50022350,0x3c);
    REG_PL_WR(0x50022354,0xd1);
    
    REG_PL_WR(0x50022358,0x0f);
    REG_PL_WR(0x5002235c,0x0f);
    
    REG_PL_WR(0x50022360,0x7f);// pd
    
    REG_PL_WR(0x50022364,0x00);
    
    REG_PL_WR(0x50022368,0xff);
    REG_PL_WR(0x5002236c,0x3f);

    REG_PL_WR(0x50022370,0x55);
    REG_PL_WR(0x50022374,0x07);
    REG_PL_WR(0x50022378,0x08);
    REG_PL_WR(0x5002237c,0x17);
    REG_PL_WR(0x50022380,0x00);
    REG_PL_WR(0x50022384,0x00);
    REG_PL_WR(0x50022388,0x00);
    REG_PL_WR(0x5002238c,0x45);
    REG_PL_WR(0x50022390,0xba);
    
    system_regs->cdc_mas_clk_adj.cdc_adj_bp = 1;
}

#if USB_AUDIO_ENABLE

volatile uint16_t MIC_Buffer[480];
volatile uint32_t MIC_Packet;
volatile uint32_t MIC_RxCount;
volatile uint32_t MIC_TxCount;
volatile bool     MIC_Start;

volatile uint32_t Speaker_Buffer[480];
volatile uint32_t Speaker_Packet;
volatile uint32_t Speaker_RxCount;
volatile uint32_t Speaker_TxCount;
volatile bool     Speaker_Start;

__attribute__((section("ram_code"))) void cdc_isr_imp(void)
{
    uint32_t i;
    
    if (digital_codec_reg->adcff_cfg1.afull_status)
    {
        MIC_RxCount = MIC_Packet * 48;

        for (i = 0; i < 48; i++)
        {
            MIC_Buffer[MIC_RxCount + i] = digital_codec_reg->adcff_data;
        }

        MIC_Packet += 1;

        if (MIC_Packet >= 10) 
        {
            MIC_Packet = 0;
        }

        if (MIC_Packet >= 5 && MIC_Start == false) 
        {
            usb_selecet_endpoint(ENDPOINT_1);
            
            /* start the frist packet transmission */
            if (usb_Endpoints_GET_TxPkrRdy() == false) 
            {
                usb_write_fifo(ENDPOINT_1, (uint8_t *)&MIC_Buffer[MIC_TxCount], 96);
                usb_Endpoints_SET_TxPktRdy();

                MIC_TxCount += 48;

                if (MIC_TxCount >= 480) 
                {
                    MIC_TxCount = 0;
                }

                 MIC_Start = true;
            }
            /* start fail */
            else 
            {
                MIC_Packet  = 0;
                MIC_RxCount = 0;
                MIC_TxCount = 0;
                MIC_Start   = false;
            }
        }

        if (MIC_Start) 
        {
            if (MIC_RxCount == MIC_TxCount) 
            {
                MIC_Packet  = 0;
                MIC_RxCount = 0;
                MIC_TxCount = 0;
                MIC_Start   = false;
            }
        }
    }
        
    if (digital_codec_reg->dacff_cfg1.aept_status)
    {
        if (Speaker_Start)
        {
            for (i = 0; i < 48; i++)
            {
                digital_codec_reg->dacff_data = Speaker_Buffer[Speaker_TxCount++];
            }
            
            if (Speaker_TxCount >= 480) 
            {
                Speaker_TxCount = 0;
            }

            if (Speaker_TxCount == Speaker_RxCount) 
            {
                Speaker_Packet  = 0;
                Speaker_RxCount = 0;
                Speaker_TxCount = 0;
                Speaker_Start   = false;
            }
        }
    }
}

#else

#ifndef CFG_FT_CODE
//#if (!defined CFG_FT_CODE)||(defined CFG_FT_CODE_NEW)
//uint16_t pcm_sin_wave_1k[]={0x5175,0x0000,0xae8a,0x8ccc,0xae8a,0xffff,0x5175,0x7333};
__attribute__((section("ram_code"))) void cdc_isr_imp(void)
{
    struct pcm_data_t *pcm_data;
    uint16_t last;
    uint16_t *data;

    if(digital_codec_reg->adcff_cfg1.afull_status)
    {
        for(uint8_t i=0; i<32; i++)
        {
            uint16_t tmp = digital_codec_reg->adcff_data;
            digital_codec_reg->dacff_data = tmp;
        }
    }
    
}
#endif

#endif
