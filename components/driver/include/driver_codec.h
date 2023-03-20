#ifndef _DRIVER_CODEC_H_
#define _DRIVER_CODEC_H_

#include <stdint.h>

#include "driver_plf.h"

#define CODEC_REG_D0        (CODEC_ANALOG_BASE + (0xd0<<2))
#define CODEC_VMID_CTL          0x07
#define CODEC_VMID_SEL          0x08
#define CODEC_VMID_PD           0x10

#define CODEC_REG_D1        (CODEC_ANALOG_BASE + (0xd1<<2))
#define CODEC_REG_D2        (CODEC_ANALOG_BASE + (0xd2<<2))
#define CODEC_REG_D3        (CODEC_ANALOG_BASE + (0xd3<<2))
#define CODEC_REG_D4        (CODEC_ANALOG_BASE + (0xd4<<2))
#define CODEC_REG_D5        (CODEC_ANALOG_BASE + (0xd5<<2))
#define CODEC_REG_D6        (CODEC_ANALOG_BASE + (0xd6<<2))
#define CODEC_REG_D7        (CODEC_ANALOG_BASE + (0xd7<<2))
#define CODEC_REG_D8        (CODEC_ANALOG_BASE + (0xd8<<2))
#define CODEC_REG_D9        (CODEC_ANALOG_BASE + (0xd9<<2))
#define CODEC_REG_DA        (CODEC_ANALOG_BASE + (0xda<<2))
#define CODEC_REG_DB        (CODEC_ANALOG_BASE + (0xdb<<2))
#define CODEC_REG_DC        (CODEC_ANALOG_BASE + (0xdc<<2))
#define CODEC_REG_DD        (CODEC_ANALOG_BASE + (0xdd<<2))
#define CODEC_REG_DE        (CODEC_ANALOG_BASE + (0xde<<2))
#define CODEC_REG_DF        (CODEC_ANALOG_BASE + (0xdf<<2))
#define CODEC_REG_E0        (CODEC_ANALOG_BASE + (0xe0<<2))
#define CODEC_REG_E1        (CODEC_ANALOG_BASE + (0xe1<<2))
#define CODEC_REG_E2        (CODEC_ANALOG_BASE + (0xe2<<2))
#define CODEC_REG_E3        (CODEC_ANALOG_BASE + (0xe3<<2))
#define CODEC_REG_E4        (CODEC_ANALOG_BASE + (0xe4<<2))

enum audio_codec_sco_source_t {
    AUDIO_CODEC_SCO_SRC_PCM,
    AUDIO_CODEC_SCO_SRC_IPC,
};

enum audio_codec_dac_src_t {
    AUDIO_CODEC_DAC_SRC_SCO,
    AUDIO_CODEC_DAC_SRC_SBC_HW,
    AUDIO_CODEC_DAC_SRC_IPC_DMA,
    AUDIO_CODEC_DAC_SRC_USER,
};

enum audio_codec_adc_dest_t {
    AUDIO_CODEC_ADC_DEST_SCO,
    AUDIO_CODEC_ADC_DEST_IPC_DMA,
    AUDIO_CODEC_ADC_DEST_USER,
};

enum audio_codec_sample_rate_t {
    AUDIO_CODEC_SAMPLE_RATE_ADC_MASK = 0x0f,
    AUDIO_CODEC_SAMPLE_RATE_ADC_8000 = 0x00,
    AUDIO_CODEC_SAMPLE_RATE_ADC_12000 = 0x01,
    AUDIO_CODEC_SAMPLE_RATE_ADC_16000 = 0x02,
    AUDIO_CODEC_SAMPLE_RATE_ADC_24000 = 0x03,
    AUDIO_CODEC_SAMPLE_RATE_ADC_32000 = 0x04,
    AUDIO_CODEC_SAMPLE_RATE_ADC_48000 = 0x05,
    AUDIO_CODEC_SAMPLE_RATE_ADC_44100 = 0x0b,
    AUDIO_CODEC_SAMPLE_RATE_DAC_MASK = 0xf0,
    AUDIO_CODEC_SAMPLE_RATE_DAC_8000 = 0x00,
    AUDIO_CODEC_SAMPLE_RATE_DAC_12000 = 0x10,
    AUDIO_CODEC_SAMPLE_RATE_DAC_16000 = 0x20,
    AUDIO_CODEC_SAMPLE_RATE_DAC_24000 = 0x30,
    AUDIO_CODEC_SAMPLE_RATE_DAC_32000 = 0x40,
    AUDIO_CODEC_SAMPLE_RATE_DAC_48000 = 0x50,
    AUDIO_CODEC_SAMPLE_RATE_DAC_44100 = 0xb0,
};

enum audio_codec_func_t{
    AUDIO_CODEC_FUNC_MIC_LOOP,
    AUDIO_CODEC_FUNC_MIC_ONLY,
    AUDIO_CODEC_FUNC_NATIVE_PLAYBACK,
};


#define AUDIO_CODEC_TONE_PRIO_LOW       0
#define AUDIO_CODEC_TONE_PRIO_MEDIAL    1
#define AUDIO_CODEC_TONE_PRIO_HIGH      2

///@0x50022000,0x70
struct codec_sysconfig_t{
    uint32_t adc_en:1;
    uint32_t dac_en:1;
    uint32_t gpf_en:1;
    uint32_t i2s_en:1;
    uint32_t adc_nrst_rst:1;///0---active,1---release
    uint32_t dac_nrst_rst:1;
    uint32_t gpf_nrst_rst:1;
    uint32_t rsvd:25;
};

///@@0x50022004,0x00
struct codec_sample_rate_t{
    ///0000---8K,0001---16k,0010---16k,0011---24k,0100---32k,0101---48k
    ///1000---8.0214k,1001---11.0259k,1010---22.0588k,1011---44.1k
    uint32_t adc_sr:4;
    uint32_t dac_sr:4;
    uint32_t rsvd:24;
};


///@@0x50022004,0x09
struct codec_i2s_config_t{
    uint32_t format:2;///00---right justified,01---lef justified,10---i2s format,11---dsp mode
    uint32_t dci_wl:2;///00---16bit,01---20bit,10---24bit,11---32bit
    uint32_t lrp:1;
    uint32_t lrswap:1;
    uint32_t chan_cp:1;
    uint32_t bclk_inv:1;
    uint32_t rsvd:24;
};

///@@0x50022008,0x01
struct codec_adc_config_t{
    uint32_t hpf_en:1;
    uint32_t adc_copy:1;///0---all zeros, 1---copy from left
    uint32_t adc_code:1;///0---raw sdm data, 1---coded sdm data
    uint32_t dec_gpf_en:1;
    uint32_t rsvd:28;
};

///@@0x5002200c,0x00
struct codec_dac_config_t{
    uint32_t dsm_mode:1;
    uint32_t scramble_en:1;
    uint32_t dither_en:1;
    uint32_t dac_mono_en:1;
    uint32_t momo_left:1; ///1---left channel used in mono mode
    uint32_t dac_mute_l:1;
    uint32_t dac_mute_r:1;
    uint32_t dac_gpf_en:1;
    uint32_t rsvd:24;
};
///@@0x50022038,0x80
struct codec_adcff_cfg0_t{
    uint32_t wr_clr:1;
    uint32_t rd_clr:1;
    uint32_t aflr:6;//adc fifo almost full level
    uint32_t rsvd:24;
};

///@@0x5002203c,0x20
struct codec_adcff_cfg1_t{
    uint32_t full_int_en:1;
    uint32_t full_status:1;
    uint32_t afull_int_en:1;
    uint32_t afull_status:1;
    uint32_t ept_int_en:1;
    uint32_t ept_status:1;
    uint32_t apb_sel:1;
    uint32_t enable:1;
};


///@@0x50022044,0x80
struct codec_dacff_cfg0_t{
    uint32_t wr_clr:1;
    uint32_t rd_clr:1;
    uint32_t aelr:6;//dac fifo almost empty level
    uint32_t rsvd:24;
};

///@@0x50022048,0x08
struct codec_dacff_cfg1_t{
    uint32_t full_int_en:1;
    uint32_t full_status:1;
    uint32_t ept_int_en:1;
    uint32_t ept_status:1;
    uint32_t aept_int_en:1;
    uint32_t aept_status:1;
    uint32_t apb_sel:1; ///1--- wrote by apb
    uint32_t enable:1;
};

///@@0x5002204c,0x00
struct codec_esco_mask_t{
    uint32_t esco_mask:1;
    uint32_t wr_zero_when_dac_ept:1;// when dac fifo empty,write dac fifo out zero ,otherwise,keep last 64 data
    uint32_t rsvd:30;
};

struct codec_dac_config1_t{
    uint32_t int_din_src:2;
    uint32_t cpy_when_mono:1;
    uint32_t dacff_clk_inv_sel:1;
    uint32_t rsvd:28;
};

struct codec_reg_t{
    struct codec_sysconfig_t sys_cfg;           //0x00
    struct codec_sample_rate_t sample_rate;     //0x01
    struct codec_i2s_config_t i2s_cfg;          //0x02
    struct codec_adc_config_t adc_cfg;          //0x03
    struct codec_dac_config_t dac_cfg;          //0x04
    uint32_t dac_vol_lh;                        //0x05, the volume for the left filter,[11:8]
    uint32_t dac_vol_ll;                        //0x06, the volume for the left filter,[7:0]
    uint32_t dac_vol_rh;                        //0x07,the volume for the right filter,[11:8]
    uint32_t dac_vol_rl;                        //0x08, the volume for the right filter,[7:0]
    uint32_t dac_dither0;                       //0x09,dither power,[22:16]
    uint32_t dac_dither1;                       //0x0a,[15:8]
    uint32_t dac_dither2;                       //0x0b,[7:0]
    uint32_t testmode;                          //0x0c, 00---normal mode,01---analog test,10---digital test,11---dig/ana loop back
    uint32_t adcff_data;                        //0x0d
    struct codec_adcff_cfg0_t adcff_cfg0;       //0x0e
    struct codec_adcff_cfg1_t adcff_cfg1;       //0x0f
    uint32_t dacff_data;                        //0x10
    struct codec_dacff_cfg0_t dacff_cfg0;       //0x11
    struct codec_dacff_cfg1_t dacff_cfg1;       //0x12
    struct codec_esco_mask_t esco_mask;         //0x13
    uint32_t gpf_clkdiv;                        //0x14, [7:0]
    struct codec_dac_config1_t dac_cfg1;        //0x15
    
};

extern volatile struct codec_reg_t *digital_codec_reg;

typedef void (*sbc_play_callback)(uint8_t *data);


/* get */
#define CUE_TONE  1u
#define AUDIO_SCO 2u
#define AUDIO_SBC 3u
extern int get_audio_type(void);

/* configure codec working in SCO mode */
void codec_audio_reg_config(void);

/* 
 * configure codec working in aac mode, aac is decoded in DSP, PCM data
 * are transfered via IPC DMA from DSP to codec
 */
void codec_aac_reg_config(uint8_t sample_rate);

/* 
 * configure codec working in sbc mode, sbc is decoded in hardware SBC decoder,
 * PCM data are transfered via IPC DMA from decoder to codec
 */
void codec_sbc_reg_config(uint8_t sample_rate);

/* configure codec working in tone mode, the sample rate is 16K */
uint8_t codec_tone_reg_config(uint8_t tone_index);

/* used to power down codec to save power */
void codec_off_reg_config(void);

/* used to do codec initialziation */
void codec_init_reg_config(void);

/* play external tone*/
void audio_play_tone(uint8_t tone_index);

/* stop play external tone*/
void audio_stop_tone(void);

/*register sbc play complete callback*/
void audio_play_sbc_register_callback(sbc_play_callback callback);

/*start play sbc frame*/
void audio_play_sbc(uint8_t *data, uint16_t len);

/*stop play sbc frame*/
void audio_stop_sbc(void);

void audio_play_pcm_register_callback(sbc_play_callback callback);

void audio_play_pcm(uint16_t *data, uint16_t len);

void audio_stop_pcm(void);

void audio_play_pcm_init(void);

/* start mic loop test */
void audio_codec_mic_loop_start(void);

/* stop mic loop test */
void audio_codec_mic_loop_stop(void);

/* analog codec power off*/
void codec_power_off(void);

/*
*start audio function,like mic loop,mic test,native playback
*msg_id, refer to 'enum audio_codec_func_t'
*/
void audio_codec_func_start(uint8_t msg_id);

/*
*stop audio function,like mic loop,mic test,native playback
*msg_id, refer to 'enum audio_codec_func_t'
*/
void audio_codec_func_stop(uint8_t msg_id);

/* used to set codec paramter in ADC or DAC mode */
void codec_dac_reg_config(uint8_t sample_rate, enum audio_codec_dac_src_t src);
void codec_adc_reg_config(uint8_t sample_rate, enum audio_codec_adc_dest_t dest, uint8_t mic_gain);
void codec_dac_adc_reg_config(uint8_t sample_rate, enum audio_codec_dac_src_t src, enum audio_codec_adc_dest_t dest, uint8_t mic_gain);

#if USB_AUDIO_ENABLE
extern volatile uint16_t MIC_Buffer[480];
extern volatile uint32_t MIC_Packet;
extern volatile uint32_t MIC_RxCount;
extern volatile uint32_t MIC_TxCount;
extern volatile bool     MIC_Start;

extern volatile uint32_t Speaker_Buffer[480];
extern volatile uint32_t Speaker_Packet;
extern volatile uint32_t Speaker_RxCount;
extern volatile uint32_t Speaker_TxCount;
extern volatile bool     Speaker_Start;

void codec_usb_audio_adc_dac_config(uint8_t sample_rate, enum audio_codec_dac_src_t src, enum audio_codec_adc_dest_t dest, uint8_t mic_gain);
#endif 

#endif  // _DRIVER_CODEC_H_
