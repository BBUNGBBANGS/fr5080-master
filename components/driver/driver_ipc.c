#include <stdint.h>
#include <string.h>

#define CFG_CPU_CORTEX_M3
#include "arch.h"

#include "driver_ipc.h"

#include "co_list.h"
#include "os_mem.h"

struct ipc_ctrl_t {
    uint32_t msgin00_int_en:1;
    uint32_t msgin01_int_en:1;
    uint32_t msgin10_int_en:1;
    uint32_t msgin11_int_en:1;
    uint32_t msgout00_int_en:1;
    uint32_t msgout01_int_en:1;
    uint32_t msgout10_int_en:1;
    uint32_t msgout11_int_en:1;
    uint32_t msgin00_int_status:1;
    uint32_t msgin01_int_status:1;
    uint32_t msgin10_int_status:1;
    uint32_t msgin11_int_status:1;
    uint32_t msgout00_int_status:1;
    uint32_t msgout01_int_status:1;
    uint32_t msgout10_int_status:1;
    uint32_t msgout11_int_status:1;
};

struct ipc_dma_ctrl_t {
    uint32_t i2sdac_en:1;   // from DSP to i2s
    uint32_t i2sadc_en:1;   // from i2s to DSP
    uint32_t adcin_en:1;    // from codec adc to DSP
    uint32_t dacout_en:1;   // from DSP to codec dac
    uint32_t pdm0_l_en:1;   // from pdm0 left to DSP
    uint32_t pdm0_r_en:1;   // from pdm0 right to DSP
    uint32_t pdm1_l_en:1;   // from pdm1 left to DSP
    uint32_t pdm1_r_en:1;   // from pdm1 right to DSP
    uint32_t esco0_src_en:1;    // from esco0 to DSP
    uint32_t esco1_src_en:1;    // from esco1 to DSP
    uint32_t esco0_snk_en:1;    // from DSP to esco0
    uint32_t esco1_snk_en:1;    // from DSP to esco1
    uint32_t i2sadc_mode:1;     // 0: mono; 1: stereo
    uint32_t i2sdac_mode:1;     // 0: mono; 1: stereo
    uint32_t dacout_mode:1;     // 0: mono; 1: stereo
    uint32_t reserved0:1;
    uint32_t i2sadc_des_rst:1;  // reset i2sadc frame counter and tog value
    uint32_t adcin_des_rst:1;   // reset codec adc frame counter and tog value
    uint32_t pdm0_l_des_rst:1;  // reset pdm0 left frame counter and tog value
    uint32_t pdm0_r_des_rst:1;  // reset pdm0 right frame counter and tog value
    uint32_t pdm1_l_des_rst:1;  // reset pdm1 left frame counter and tog value
    uint32_t pdm1_r_des_rst:1;  // reset pdm1 right frame counter and tog value
    uint32_t esco0_dest_rst:1;
    uint32_t esco1_dest_rst:1;
    uint32_t i2sdac_src_rst:1;
    uint32_t dacout_src_rst:1;
    uint32_t esco0_src_rst:1;
    uint32_t esco1_src_rst:1;
    uint32_t reserved1:4;
};

struct ipc_dma_level_t {
    uint32_t dsp_int_gen_level:10;  // interrupt generate level in DSP side
    uint32_t reserved1:6;
    uint32_t dma_trans_len:10;      // dma single transmit length
    uint32_t reserved0:6;
};

struct ipc_dma_buff_addr_t {
    uint32_t address0:11;   // used by DMA read or write when toggle is 0
                            // used by DSP read or write when toggle is 1
    uint32_t reserved0:5;
    uint32_t address1:11;   // used by DMA read or write when toggle is 1
                            // used by DSP read or write when toggle is 0
    uint32_t reserved1:5;
};

struct ipc_dma_int_mask_t {
    uint32_t i2s_adc_int:1;
    uint32_t codec_adc_int:1;
    uint32_t pdm0_l_int:1;
    uint32_t pdm0_r_int:1;
    uint32_t pdm1_l_int:1;
    uint32_t pdm1_r_int:1;
    uint32_t esco_int:1;
    uint32_t i2s_dac_int:1;
    uint32_t codec_dac_int:1;
    uint32_t cm3_int:1;
    uint32_t reserved:22;
};

struct ipc_t {
    struct ipc_ctrl_t ctr;
    uint32_t reserved0;
    struct ipc_msg_t msg_in[4];
    struct ipc_msg_t msg_out[4];
    uint32_t reserved1[6];
    
    struct ipc_dma_ctrl_t dma_ctrl;             // 0x40
    struct ipc_dma_level_t i2s_level;
    struct ipc_dma_level_t codec_level;
    struct ipc_dma_level_t esco_level;
    struct ipc_dma_level_t pdm0_left_level;     // 0x50
    struct ipc_dma_level_t pdm0_right_level;
    struct ipc_dma_level_t pdm1_left_level;
    struct ipc_dma_level_t pdm1_right_level;
    
    struct ipc_dma_buff_addr_t i2s_bb2dsp;      // 0x60
    struct ipc_dma_buff_addr_t codec_bb2dsp;
    struct ipc_dma_buff_addr_t pdm0_l_bb2dsp;
    struct ipc_dma_buff_addr_t pdm0_r_bb2dsp;
    struct ipc_dma_buff_addr_t pdm1_l_bb2dsp;   // 0x70
    struct ipc_dma_buff_addr_t pdm1_r_bb2dsp;
    struct ipc_dma_buff_addr_t esco0_bb2dsp;
    struct ipc_dma_buff_addr_t esco1_bb2dsp;
    struct ipc_dma_buff_addr_t i2s_dsp2bb;      // 0x80
    struct ipc_dma_buff_addr_t codec_dsp2bb;
    struct ipc_dma_buff_addr_t esco0_dsp2bb;
    struct ipc_dma_buff_addr_t esco1_dsp2bb;

    struct ipc_dma_int_mask_t dma_int_mask;     // 0x90
};

struct ipc_env_t {
    uint8_t available_buffer;

    uint8_t ipc_i2s_mcu2dsp_tog;
    uint8_t ipc_codec_mcu2dsp_tog;
    uint8_t ipc_pdm0_l_mcu2dsp_tog;
    uint8_t ipc_pdm0_r_mcu2dsp_tog;
    uint8_t ipc_pdm1_l_mcu2dsp_tog;
    uint8_t ipc_pdm1_r_mcu2dsp_tog;
    uint8_t ipc_esco0_mcu2dsp_tog;
    uint8_t ipc_esco1_mcu2dsp_tog;
    uint8_t ipc_i2s_dsp2mcu_tog;
    uint8_t ipc_codec_dsp2mcu_tog;
    uint8_t ipc_esco0_dsp2mcu_tog;
    uint8_t ipc_esco1_dsp2mcu_tog;

    ipc_tx_callback tx_callback[4];
    ipc_rx_callback rx_callback;
};

// ���ڿɿ����Զ������ͷ���
struct ipc_msg_elem_t {
    struct co_list_hdr hdr;
    
    ipc_tx_callback callback;
    bool with_payload;
    
    enum ipc_msg_type_t msg;
    union {
        uint16_t sub_msg;
        struct {
            uint16_t length;
            uint8_t *buffer;
        } payload;
    } p;
};

static struct ipc_env_t *ipc_env_p = (struct ipc_env_t *)0x20001600;

extern volatile struct ipc_t *ipc;

enum ipc_mic_type_t ipc_mic_type = IPC_MIC_TYPE_ANLOG_MIC;
enum ipc_spk_type_t ipc_spk_type = IPC_SPK_TYPE_CODEC;
enum ipc_media_type_t ipc_media_type = IPC_MEDIA_TYPE_BT;

enum ipc_mic_type_t ipc_get_mic_type(void)
{
    return ipc_mic_type;
}

enum ipc_spk_type_t ipc_get_spk_type(void)
{
    return ipc_spk_type;
}

enum ipc_media_type_t ipc_get_media_type(void)
{
    return ipc_media_type;
}

// Ӧ�ò㷢��ipc��Ϣʱ�����û�з��䵽channel����ô�Ȼ��浽����
static struct co_list ipc_msg_list = {NULL, NULL};

__attribute__((section("ram_code"))) static void ipc_msg_sent(void)
{
    uint8_t chn;
    CPU_SR cpu_sr;

    GLOBAL_INT_OLD_DISABLE();
    if(co_list_is_empty(&ipc_msg_list) == 0) {
        struct ipc_msg_elem_t *ele = (struct ipc_msg_elem_t *)co_list_pick(&ipc_msg_list);
        if(ele->with_payload) {
            chn = ipc_alloc_channel(ele->p.payload.length);
        }
        else {
            chn = ipc_alloc_channel(0);
        }
        if(chn != 0xff) {
            co_list_pop_front(&ipc_msg_list);
            if(ele->with_payload) {
                if(ele->p.payload.length) {
                    uint8_t *buffer = ipc_get_buffer_offset(IPC_DIR_MCU2DSP, chn);
                    memcpy(buffer, ele->p.payload.buffer, ele->p.payload.length);
                    os_free(ele->p.payload.buffer);
                }
                ipc_insert_msg(chn, ele->msg, ele->p.payload.length, ele->callback);
            }
            else {
                ipc_insert_msg(chn, ele->msg, ele->p.sub_msg, ele->callback);
            }
            os_free((void *)ele);
        }
    }
    
    GLOBAL_INT_OLD_RESTORE();
}

__attribute__((section("ram_code"))) void ipc_msg_send(enum ipc_msg_type_t msg, uint16_t sub_msg, ipc_tx_callback callback)
{
    uint8_t chn;
    CPU_SR cpu_sr;

    GLOBAL_INT_OLD_DISABLE();
    chn = ipc_alloc_channel(0);
    if(chn != 0xff) {
        ipc_insert_msg(chn, msg, sub_msg, callback);
    }
    else {
        struct ipc_msg_elem_t *elt;
        elt = (struct ipc_msg_elem_t *)os_malloc(sizeof(struct ipc_msg_elem_t));
        elt->with_payload = false;
        elt->msg = msg;
        elt->p.sub_msg = sub_msg;
        elt->callback = callback;
        co_list_push_back(&ipc_msg_list, &elt->hdr);
    }
    GLOBAL_INT_OLD_RESTORE();
}

__attribute__((section("ram_code"))) void ipc_msg_with_payload_send(enum ipc_msg_type_t msg,
                                                                        void *header,
                                                                        uint16_t header_length,
                                                                        uint8_t *payload,
                                                                        uint16_t payload_length,
                                                                        ipc_tx_callback callback)
{
    uint8_t chn;
    uint8_t *ipc_buffer;
    CPU_SR cpu_sr;
    uint16_t total_length = header_length+payload_length;

    GLOBAL_INT_OLD_DISABLE();
    chn = ipc_alloc_channel(total_length);
    if(chn != 0xff) {
        /* ��ȡ��ͨ�����Ӧ��buffer */
        ipc_buffer = ipc_get_buffer_offset(IPC_DIR_MCU2DSP, chn);
        memcpy(ipc_buffer, header, header_length);
        memcpy(ipc_buffer+header_length, payload, payload_length);
        ipc_insert_msg(chn, msg, total_length, callback);
    }
    else {
        struct ipc_msg_elem_t *elt;
        elt = (struct ipc_msg_elem_t *)os_malloc(sizeof(struct ipc_msg_elem_t));
        elt->with_payload = true;
        elt->msg = msg;
        if(total_length) {
            uint8_t *tmp_buffer;
            tmp_buffer = os_malloc(total_length);
            memcpy(tmp_buffer, header, header_length);
            memcpy(tmp_buffer+header_length, payload, payload_length);
            elt->p.payload.buffer = tmp_buffer;
        }
        elt->p.payload.length = total_length;
        elt->callback = callback;
        co_list_push_back(&ipc_msg_list, &elt->hdr);
    }
    GLOBAL_INT_OLD_RESTORE();
}

__attribute__((section("ram_code"))) void ipc_msg_flush(void)
{
    struct ipc_msg_elem_t *ele;
    CPU_SR cpu_sr;
    
    GLOBAL_INT_OLD_DISABLE();
    while(co_list_is_empty(&ipc_msg_list) == 0){
        ele = (struct ipc_msg_elem_t *)co_list_pop_front(&ipc_msg_list);
        if(ele->with_payload == true){
            os_free((void *)ele->p.payload.buffer);
        }
        os_free((void *)ele);
    } 
    GLOBAL_INT_OLD_RESTORE();
}

__attribute__((section("ram_code"))) void ipc_clear_sending_msg(uint8_t chn)
{
    *(uint32_t *)&ipc->msg_out[chn] = 0;
}

__attribute__((section("ram_code"))) void ipc_isr_ram(void)
{
    uint32_t status;
    struct ipc_msg_t msg;
    ipc_tx_callback callback;

    status = *(volatile uint32_t *)0x500b0000;
    *(volatile uint32_t *)0x500b0000 = status;    // clear interrupt

    if(status & IPC_MSGOUT00_STATUS) {
        callback = ipc_env_p->tx_callback[0];
        ipc_free_channel(0);
        ipc_msg_sent();
        if(callback) {
            callback(0);
        }
    }
    if(status & IPC_MSGOUT01_STATUS) {
        callback = ipc_env_p->tx_callback[1];
        ipc_free_channel(1);
        ipc_msg_sent();
        if(callback) {
            callback(1);
        }
    }
    if(status & IPC_MSGOUT10_STATUS) {
        callback = ipc_env_p->tx_callback[2];
        ipc_free_channel(2);
        ipc_msg_sent();
        if(callback) {
            callback(2);
        }
    }
    if(status & IPC_MSGOUT11_STATUS) {
        callback = ipc_env_p->tx_callback[3];
        ipc_free_channel(3);
        ipc_msg_sent();
        if(callback) {
            callback(3);
        }
    }

    if(status & IPC_MSGIN00_STATUS) {
        if(ipc_env_p->rx_callback) {
            *(uint32_t *)&msg = *(uint32_t *)0x500b0008;
            ipc_env_p->rx_callback(&msg, 0);
        }
    }
    if(status & IPC_MSGIN01_STATUS) {
        if(ipc_env_p->rx_callback) {
            *(uint32_t *)&msg = *(uint32_t *)0x500b000c;
            ipc_env_p->rx_callback(&msg, 1);
        }
    }
    if(status & IPC_MSGIN10_STATUS) {
        if(ipc_env_p->rx_callback) {
            *(uint32_t *)&msg = *(uint32_t *)0x500b0010;
            ipc_env_p->rx_callback(&msg, 2);
        }
    }
    if(status & IPC_MSGIN11_STATUS) {
        if(ipc_env_p->rx_callback) {
            *(uint32_t *)&msg = *(uint32_t *)0x500b0014;
            ipc_env_p->rx_callback(&msg, 3);
        }
    }
}

static void ipc_set_voice_share_mem_addr(void)
{
    ipc->codec_dsp2bb.address0 = 0;
    ipc->codec_dsp2bb.address1 = 120;
    ipc->esco0_dsp2bb.address0 = 240;
    ipc->esco0_dsp2bb.address1 = 360;
    ipc->esco1_dsp2bb.address0 = 480;
    ipc->esco1_dsp2bb.address1 = 600;
    ipc->i2s_dsp2bb.address0 = 720;
    ipc->i2s_dsp2bb.address1 = 840;

    ipc->codec_bb2dsp.address0 = 0;
    ipc->codec_bb2dsp.address1 = 120;
    ipc->esco0_bb2dsp.address0 = 240;
    ipc->esco0_bb2dsp.address1 = 360;
    ipc->pdm0_l_bb2dsp.address0 = 480;
    ipc->pdm0_l_bb2dsp.address1 = 600;
    ipc->pdm0_r_bb2dsp.address0 = 720;
    ipc->pdm0_r_bb2dsp.address1 = 840;
    ipc->pdm1_l_bb2dsp.address0 = 960;
    ipc->pdm1_l_bb2dsp.address1 = 1080;
    ipc->pdm1_r_bb2dsp.address0 = 1200;
    ipc->pdm1_r_bb2dsp.address1 = 1320;
    ipc->esco1_bb2dsp.address0 = 1440;
    ipc->esco1_bb2dsp.address1 = 1560;
    ipc->i2s_bb2dsp.address0 = 1680;
    ipc->i2s_bb2dsp.address1 = 1800;
}

static void ipc_set_media_share_mem_addr(void)
{
    if(ipc_get_spk_type() == IPC_SPK_TYPE_I2S){
        ipc->i2s_dsp2bb.address0 = 0;     // unit: byte
        ipc->i2s_dsp2bb.address1 = 1024;  // unit: byte
    }else{
        ipc->codec_dsp2bb.address0 = 0;     // unit: byte
        ipc->codec_dsp2bb.address1 = 1024;  // unit: byte
    }
}

static void ipc_set_voice_trigger(void)
{
#if 0
///EV3
    ipc->codec_level.dma_trans_len = 30;
    ipc->codec_level.dsp_int_gen_level = 30;
    ipc->esco_level.dma_trans_len = 30;
    ipc->esco_level.dsp_int_gen_level = 30;

    ipc->i2s_level.dma_trans_len = 30;
    ipc->i2s_level.dsp_int_gen_level = 30;

//TEST_PDM
    ipc->pdm0_left_level.dsp_int_gen_level = 30;
    ipc->pdm0_left_level.dma_trans_len = 30;
    ipc->pdm0_right_level.dsp_int_gen_level = 30;
    ipc->pdm0_right_level.dma_trans_len = 30;
    ipc->pdm1_left_level.dsp_int_gen_level = 30;
    ipc->pdm1_left_level.dma_trans_len = 30;
    ipc->pdm1_right_level.dsp_int_gen_level = 30;
    ipc->pdm1_right_level.dma_trans_len = 30;

#else   
///2ev3
    ipc->codec_level.dma_trans_len = 30;
    ipc->codec_level.dsp_int_gen_level = 60;
    ipc->esco_level.dma_trans_len = 60;
    ipc->esco_level.dsp_int_gen_level = 60;
    
    ipc->i2s_level.dma_trans_len = 30;
    ipc->i2s_level.dsp_int_gen_level = 60;

    ipc->pdm0_left_level.dsp_int_gen_level = 60;
    ipc->pdm0_left_level.dma_trans_len = 30;
    ipc->pdm0_right_level.dsp_int_gen_level = 60;
    ipc->pdm0_right_level.dma_trans_len = 30;
    ipc->pdm1_left_level.dsp_int_gen_level = 60;
    ipc->pdm1_left_level.dma_trans_len = 30;
    ipc->pdm1_right_level.dsp_int_gen_level = 60;
    ipc->pdm1_right_level.dma_trans_len = 30;

#endif  // #if TEST_ESCO_EV3 == 1
}

static void ipc_set_media_trigger(void)
{
    if(ipc_get_spk_type() == IPC_SPK_TYPE_I2S){
        ipc->i2s_level.dma_trans_len = 32;        // unit: sample ( 2*2 bytes )
        ipc->i2s_level.dsp_int_gen_level = 256;   // unit: half word
    }else{
        ipc->codec_level.dma_trans_len = 32;        // unit: sample ( 2*2 bytes ),equal to codec fifo int level
        ipc->codec_level.dsp_int_gen_level = 256;   // unit: half word, shall be equal to dsp single write len
    }
}

static void ipc_set_mic_loop_dma_enable(void)
{
    *(uint32_t *)(&ipc->dma_int_mask) = 0xffffffff;
    *(uint32_t *)(&ipc->dma_ctrl) = 0;
    
    ipc->dma_ctrl.adcin_en = 1;
    ipc->dma_ctrl.dacout_mode = 0;
    ipc->dma_ctrl.dacout_en = 1;
    //ipc->dma_ctrl.esco0_src_en = 1;
    //ipc->dma_ctrl.esco0_snk_en = 1;
    
    ipc->dma_int_mask.codec_adc_int = 0; 
    ipc->dma_int_mask.codec_dac_int = 0;
    //ipc->dma_int_mask.esco_int = 0; 
    if(ipc_get_mic_type() == IPC_MIC_TYPE_PDM){
        ipc->dma_ctrl.pdm0_l_des_rst = 1;
        ipc->dma_ctrl.pdm0_r_des_rst = 1;
        ipc->dma_ctrl.pdm1_l_des_rst = 1;
        ipc->dma_ctrl.pdm1_r_des_rst = 1;

        ipc->dma_ctrl.pdm0_l_des_rst = 0;
        ipc->dma_ctrl.pdm0_r_des_rst = 0;
        ipc->dma_ctrl.pdm1_l_des_rst = 0;
        ipc->dma_ctrl.pdm1_r_des_rst = 0;

        ipc->dma_int_mask.pdm0_l_int = 1; 
        ipc->dma_int_mask.pdm0_r_int = 1; 
        ipc->dma_int_mask.pdm1_l_int = 0; 
        ipc->dma_int_mask.pdm1_r_int = 0; 
        ipc->dma_ctrl.pdm0_l_en = 0;
        ipc->dma_ctrl.pdm0_r_en = 0;
        ipc->dma_ctrl.pdm1_l_en = 1;
        ipc->dma_ctrl.pdm1_r_en = 1;

        ipc->dma_ctrl.adcin_en = 0;
        ipc->dma_int_mask.codec_adc_int = 1; 
    //printf("config pdm\r\n");
    }
}

static void ipc_set_mic_dma_enable(void)
{
    *(uint32_t *)(&ipc->dma_int_mask) = 0xffffffff;
    *(uint32_t *)(&ipc->dma_ctrl) = 0;
    
    ipc->dma_ctrl.adcin_en = 1;
    //ipc->dma_ctrl.dacout_mode = 0;
    //ipc->dma_ctrl.dacout_en = 1;
    //ipc->dma_ctrl.esco0_src_en = 1;
    //ipc->dma_ctrl.esco0_snk_en = 1;
    
    ipc->dma_int_mask.codec_adc_int = 0; 
    //ipc->dma_int_mask.codec_dac_int = 0;
    //ipc->dma_int_mask.esco_int = 0; 

    if(ipc_get_mic_type() == IPC_MIC_TYPE_I2S){
        ipc->dma_ctrl.i2sadc_en = 1; 
        ipc->dma_ctrl.i2sadc_mode  = 0;        
        ipc->dma_ctrl.adcin_en = 0;
        ipc->dma_int_mask.codec_adc_int = 1; 
        ipc->dma_int_mask.i2s_adc_int = 0; 
    }else if(ipc_get_mic_type() == IPC_MIC_TYPE_PDM){
        ipc->dma_ctrl.pdm0_l_des_rst = 1;
        ipc->dma_ctrl.pdm0_r_des_rst = 1;
        ipc->dma_ctrl.pdm1_l_des_rst = 1;
        ipc->dma_ctrl.pdm1_r_des_rst = 1;
        
        ipc->dma_ctrl.pdm0_l_des_rst = 0;
        ipc->dma_ctrl.pdm0_r_des_rst = 0;
        ipc->dma_ctrl.pdm1_l_des_rst = 0;
        ipc->dma_ctrl.pdm1_r_des_rst = 0;

        ipc->dma_int_mask.pdm0_l_int = 1; 
        ipc->dma_int_mask.pdm0_r_int = 1; 
        ipc->dma_int_mask.pdm1_l_int = 0; 
        ipc->dma_int_mask.pdm1_r_int = 0; 
        ipc->dma_ctrl.pdm0_l_en = 0;
        ipc->dma_ctrl.pdm0_r_en = 0;
        ipc->dma_ctrl.pdm1_l_en = 1;
        ipc->dma_ctrl.pdm1_r_en = 1;
        
        ipc->dma_ctrl.adcin_en = 0;
        ipc->dma_int_mask.codec_adc_int = 1; 
        //printf("config pdm\r\n");
    }
}

static void ipc_set_voice_dma_enable(void)
{
    *(uint32_t *)(&ipc->dma_int_mask) = 0xffffffff;
    *(volatile uint32_t *)(&ipc->dma_ctrl) = 0x0fff0000;
    *(volatile uint32_t *)(&ipc->dma_ctrl) = 0;
 
    GLOBAL_INT_DISABLE();
    
    *(uint32_t *)0x40000664 = 0x09;
    *(uint32_t *)0x40000668 = 120<<16;
    *(uint32_t *)0x4000066c = 120<<16;
    
//    ipc->dma_ctrl.adcin_en = 1;
//    ipc->dma_ctrl.dacout_mode = 0;
//    ipc->dma_ctrl.dacout_en = 1;
//    ipc->dma_ctrl.esco0_src_en = 1;
//    ipc->dma_ctrl.esco0_snk_en = 1;
    *(uint32_t *)(&ipc->dma_ctrl) = 0x50c;
    
//    ipc->dma_int_mask.codec_adc_int = 0; 
//    ipc->dma_int_mask.codec_dac_int = 0;
//    ipc->dma_int_mask.esco_int = 0; 
    *(uint32_t *)(&ipc->dma_int_mask) &= (~0x142);

    if(ipc_get_mic_type() == IPC_MIC_TYPE_I2S){
        ipc->dma_ctrl.i2sadc_en = 1; 
        ipc->dma_ctrl.i2sadc_mode  = 0;        
        ipc->dma_ctrl.adcin_en = 0;
        ipc->dma_int_mask.codec_adc_int = 1; 
        ipc->dma_int_mask.i2s_adc_int = 0; 
    }else if(ipc_get_mic_type() == IPC_MIC_TYPE_PDM){
        ipc->dma_ctrl.pdm0_l_des_rst = 1;
        ipc->dma_ctrl.pdm0_r_des_rst = 1;
        ipc->dma_ctrl.pdm1_l_des_rst = 1;
        ipc->dma_ctrl.pdm1_r_des_rst = 1;
        
        ipc->dma_ctrl.pdm0_l_des_rst = 0;
        ipc->dma_ctrl.pdm0_r_des_rst = 0;
        ipc->dma_ctrl.pdm1_l_des_rst = 0;
        ipc->dma_ctrl.pdm1_r_des_rst = 0;

        ipc->dma_int_mask.pdm0_l_int = 1; 
        ipc->dma_int_mask.pdm0_r_int = 1; 
        ipc->dma_int_mask.pdm1_l_int = 0; 
        ipc->dma_int_mask.pdm1_r_int = 0; 
        ipc->dma_ctrl.pdm0_l_en = 0;
        ipc->dma_ctrl.pdm0_r_en = 0;
        ipc->dma_ctrl.pdm1_l_en = 1;
        ipc->dma_ctrl.pdm1_r_en = 1;
        
        ipc->dma_ctrl.adcin_en = 0;
        ipc->dma_int_mask.codec_adc_int = 1; 
        //printf("config pdm\r\n");
    }

    if(ipc_get_spk_type() == IPC_SPK_TYPE_I2S){
        ipc->dma_ctrl.i2sdac_en = 1; 
        ipc->dma_ctrl.i2sdac_mode  = 0;
        ipc->dma_ctrl.dacout_en = 0;
        ipc->dma_int_mask.codec_dac_int = 1;
        ipc->dma_int_mask.i2s_dac_int = 0; 
    }
    GLOBAL_INT_RESTORE();
}

static void ipc_set_media_dma_enable(uint8_t stereo)
{
    *(volatile uint32_t *)(&ipc->dma_int_mask) = 0xffffffff;
    *(volatile uint32_t *)(&ipc->dma_ctrl) = 0;

    if(ipc_get_spk_type() == IPC_SPK_TYPE_I2S){
        ipc->dma_ctrl.i2sdac_mode = stereo;
        ipc->dma_ctrl.i2sdac_en = 1;
        
        ipc->dma_int_mask.i2s_dac_int = 0;
    }else{
        ipc->dma_ctrl.dacout_mode = stereo;
        ipc->dma_ctrl.dacout_en = 1;
    
        ipc->dma_int_mask.codec_dac_int = 0;
    }
}

void ipc_config_voice_dma(void)
{
    ipc_set_voice_share_mem_addr();
    ipc_set_voice_trigger();
    ipc_set_voice_dma_enable();
}

void ipc_config_media_dma(uint8_t stereo)
{
    ipc_set_media_share_mem_addr();
    ipc_set_media_trigger();
    ipc_set_media_dma_enable(stereo);
}

void ipc_config_mic_only_dma(void)
{
    ipc_set_voice_share_mem_addr();
    ipc_set_voice_trigger();
    ipc_set_mic_dma_enable();
}

void ipc_config_mic_loop_dma(void)
{
    ipc_set_voice_share_mem_addr();
    ipc_set_voice_trigger();
    ipc_set_mic_loop_dma_enable();
}

void ipc_config_reset_dma(void)
{
    *(uint32_t *)0x40000664 = 0x00;
    *(uint32_t *)(&ipc->dma_int_mask) = 0xffffffff;
    *(volatile uint32_t *)(&ipc->dma_ctrl) = 0x0fff0000;
    *(volatile uint32_t *)(&ipc->dma_ctrl) = 0;
}

void ipc_config_media_dma_disable(void)
{
    *(uint32_t *)(&ipc->dma_int_mask) = 0xffffffff;
    *(uint32_t *)(&ipc->dma_ctrl) = 0;
}

void ipc_set_audio_inout_type(enum ipc_mic_type_t mic_type,enum ipc_spk_type_t spk_type,enum ipc_media_type_t media_type)
{
    ipc_mic_type = mic_type;
    ipc_spk_type = spk_type;
    ipc_media_type = media_type;
}

