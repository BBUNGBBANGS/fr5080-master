#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "co_log.h"
#include "os_mem.h"
#include "os_task.h"
#include "user_utils.h"

#include "bt_api.h"
#include "a2dp_api.h"

#include "driver_plf.h"
#include "driver_i2s.h"
#include "driver_ipc.h"
#include "driver_sdc.h"

#include "user_fs.h"
#include "user_bt.h"
#include "user_task.h"
#include "user_dsp.h"
#include "audio_source.h"
#include "app_user_bt.h"
#include "app_error.h"

#define MAX_SBC_FRAME_NUM                   10

#define APP_BT_AUDIO_PLAY_BUF_MAX_SIZE      2048

#define AUDIO_EVENT_END                     0x01
#define AUDIO_EVENT_SWITCH                  0x02
#define AUDIO_EVENT_PAUSE                   0x04
#define AUDIO_EVENT_CONTINUE                0x08

enum audio_source_state_t {
    AUDIO_SOURCE_STATE_IDLE,
    AUDIO_SOURCE_STATE_PLAY,
    AUDIO_SOURCE_STATE_PAUSE,
};

enum audio_source_action_t {
    AUDIO_SOURCE_ACTION_NONE,
    AUDIO_SOURCE_ACTION_PLAY,
    AUDIO_SOURCE_ACTION_PAUSE,
    AUDIO_SOURCE_ACTION_NEXT,
    AUDIO_SOURCE_ACTION_PREV,
    AUDIO_SOURCE_ACTION_FAST_FORWARD,
    AUDIO_SOURCE_ACTION_FAST_BACKWARD,
};

extern uint8_t ipc_mp3_start_flag;
extern struct user_bt_env_tag *user_bt_env;
struct audio_source_env_tag audio_source_env ={0};
SbcStreamInfo sbcinfo;
struct app_audio_play_env_t app_audio_play_env;
static enum audio_source_state_t audio_source_state = AUDIO_SOURCE_STATE_IDLE;
static enum audio_source_action_t audio_source_current_action = AUDIO_SOURCE_ACTION_NONE;

#if MP3_SRC_LOCAL
static uint32_t dsp_buffer_space = 0;
static uint8_t mp3_raw_data[256];

extern uint32_t mp3_read_size;
extern uint32_t mp3_read_total_size;

#if MP3_SRC_SDCARD == 0
static uint32_t mp3_source_index = 0;
#include "mp3_sample.h"
#endif
#endif

bool audio_source_action_play(void);

static void audio_source_sbc_list_free(void)
{
    struct user_a2dp_sbc_packet_t *sbcpacket;

    GLOBAL_INT_DISABLE();
    while(!co_list_is_empty(&audio_source_env.sbc_pkt_list)){
        sbcpacket = (struct user_a2dp_sbc_packet_t *)co_list_pop_front(&audio_source_env.sbc_pkt_list);
        os_free((void *)sbcpacket->packet->data);
        os_free((void *)sbcpacket->packet);
        os_free((void *)sbcpacket);
    }
    audio_source_env.sbc_pkt_num = 0;
    GLOBAL_INT_RESTORE();
}

void audio_source_init(void)
{
    memset(&audio_source_env, 0, sizeof(struct audio_source_env_tag));
    co_list_init(&audio_source_env.sbc_pkt_list);
    //audio_source_free();

    /// for test ,init tone as sbc frame
    sbcinfo.allocMethod = 0;
    sbcinfo.bitPool = 0x23;
    sbcinfo.channelMode = 0;
    sbcinfo.numBlocks = 16;
    sbcinfo.numChannels = 2;
    sbcinfo.numSubBands = 8;
    sbcinfo.sampleFreq = 0;
}


__attribute__((section("ram_code")))void i2s_isr_ram(void)
{
    uint8_t i;
    uint32_t last;
    static uint8_t i2s_count = 0;
    os_event_t event;
    extern uint16_t user_task_id;

    last = REG_PL_RD(I2S_REG_STATUS);
    if(last & bmSTATUS_TXFFHEMPTY) {
        for(i=0; i<32; i++) {
            REG_PL_WR(I2S_REG_DATA, 0x0000);
        }
        //uart_putc_noint('#');
    }
    
    if(audio_source_env.a2dp_stream_start_flag == true){
        i2s_count ++;
        if(i2s_count == 20){
            i2s_count = 0;
            //uart_putc_noint('&');
            if(audio_source_env.sbc_pkt_num < MAX_SBC_FRAME_NUM){
                if((audio_source_state == AUDIO_SOURCE_STATE_PLAY)
                    && (audio_source_current_action == AUDIO_SOURCE_ACTION_NONE)) {
                    event.event_id = USER_EVT_AUDIO_PLAY_I2S_TRIG;
                    event.param = NULL;
                    event.param_len = 0;
                    os_msg_post(user_task_id, &event);
                }
            }
        }
    }
}

void audio_source_i2s_trig_handler(void)
{
    struct user_a2dp_sbc_packet_t *sbcpacket;

    while(1) {
        GLOBAL_INT_DISABLE();
        sbcpacket = (struct user_a2dp_sbc_packet_t *)co_list_pop_front(&audio_source_env.sbc_pkt_list);
        GLOBAL_INT_RESTORE();

        if(sbcpacket != NULL){
            //uart_putc_noint('S');
            //printf("...%x,%x\r\n",sbcpacket->packet->frameSize,sbcpacket->packet->dataLen);
            a2dp_stream_send_sbc_packet(user_bt_env->dev[0].pstream,sbcpacket->packet,&sbcinfo);
            os_free((void *)sbcpacket);
        }
        else {
            break;
        }
    }
}

static void raw_data_to_dsp_sent(uint8_t channel)
{
    os_event_t event;
    event.event_id = USER_EVT_RAW_DATA_TO_DSP_SENT;
    event.param = NULL;
    event.param_len = 0;
    os_msg_post(user_task_id, &event);
}

#if USB_DEVICE_ENABLE
extern uint8_t usb_in_flag;
#endif
extern uint8_t user_evt_notify_enable;

static __attribute__((section("ram_code"))) void send_raw_data_to_dsp(uint8_t channel)
{
    uint8_t str[16];

    if((audio_source_state != AUDIO_SOURCE_STATE_PLAY)
        || (audio_source_current_action != AUDIO_SOURCE_ACTION_NONE)) {
        return;
    }

#if MP3_SRC_LOCAL
    if(dsp_buffer_space != 0) {
#if MP3_SRC_SDCARD
        {
#if USB_DEVICE_ENABLE
        if(usb_in_flag == 0){
#endif
            uint32_t read_size;
            read_size = fs_read(mp3_raw_data, 256);
            if(read_size) {
                mp3_read_total_size += read_size;
                mp3_read_size += read_size;
                if(mp3_read_size >= 80*256){
                    if(user_evt_notify_enable&USER_EVT_NOTIFY_MP3_PROG){
                        mp3_read_size = 0;
                        //printf("+PROG:%08x\r\n",mp3_read_total_size);
                        co_sprintf((char *)(str),"+PROG:%08x\r\n",mp3_read_total_size);
                        uart_send((void *)str,16);  
                    }
                }
                dsp_buffer_space -= read_size;
                ipc_msg_with_payload_send((enum ipc_msg_type_t)IPC_MSG_RAW_FRAME, NULL, 0, mp3_raw_data, 256, raw_data_to_dsp_sent);
            }
            else {
                mp3_read_total_size = 0;
                mp3_read_size = 0;
                audio_source_action_next();
            }
#if USB_DEVICE_ENABLE
        }
        else{
            a2dp_suspend_stream(user_bt_env->dev[user_bt_env->last_active_dev_index].pstream);
        }
#endif
        }
#else
        uint32_t last_size = mp3_sample_get_size() - mp3_source_index;
        if(last_size >= 256) {
            ipc_msg_with_payload_send((enum ipc_msg_type_t)IPC_MSG_RAW_FRAME, NULL, 0, (uint8_t *)&mp3_sample[mp3_source_index], 256, raw_data_to_dsp_sent);
            mp3_source_index += 256;
            if(last_size == 256) {
                mp3_source_index = 0;
            }
        }
        else {
            memcpy(&mp3_raw_data[0], (uint8_t *)&mp3_sample[mp3_source_index], last_size);
            memcpy(&mp3_raw_data[last_size], (uint8_t *)&mp3_sample[0], 256-last_size);
            mp3_source_index = 256-last_size;
            ipc_msg_with_payload_send((enum ipc_msg_type_t)IPC_MSG_RAW_FRAME, NULL, 0, (uint8_t *)&mp3_raw_data[0], 256, raw_data_to_dsp_sent);
        }
        
        dsp_buffer_space -= 256;
#endif
    }
#endif
}

void audio_source_statemachine(uint8_t event, void *arg)
{
    //LOG_INFO("audio_source_statemachine: %d,%d,%d.\r\n", event,audio_source_state,audio_source_current_action);

    switch(event) {
        case USER_EVT_DSP_OPENED:
            dsp_buffer_space = 0;
            if(audio_source_current_action == AUDIO_SOURCE_ACTION_PLAY) {
                dsp_working_label_set(DSP_WORKING_LABEL_AUDIO_SOURCE);

                if(bt_get_link_mode() == BLM_SNIFF_MODE) {
                    bt_stop_sniff(user_bt_env->dev[user_bt_env->last_active_dev_index].remDev);
                }
                else {
                    a2dp_start_stream(user_bt_env->dev[user_bt_env->last_active_dev_index].pstream);
                }
            }
            else if((audio_source_current_action == AUDIO_SOURCE_ACTION_NEXT)
                ||(audio_source_current_action == AUDIO_SOURCE_ACTION_PREV)){
                /*
                 * notice DSP to prepare for next song, no IPC_MSG_RAW_FRAME and IPC_SUB_MSG_NEED_MORE_SBC should be send 
                 * once this message is send to avoid confusion in DSP side.
                 */
                ipc_msg_send((enum ipc_msg_type_t)IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_DECODER_PREP_NEXT,NULL);
                
            }
            break;
            
        case USER_EVT_DECODER_PREPARE_NEXT_DONE:
            dsp_buffer_space = 0;
            if(audio_source_state == AUDIO_SOURCE_STATE_PLAY){
                audio_source_current_action = AUDIO_SOURCE_ACTION_NONE;
                ipc_msg_send((enum ipc_msg_type_t)IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_DECODER_START, NULL);
            }else{
                if((audio_source_current_action == AUDIO_SOURCE_ACTION_NEXT) 
                    ||(audio_source_current_action == AUDIO_SOURCE_ACTION_PREV)){
                    dsp_working_label_set(DSP_WORKING_LABEL_AUDIO_SOURCE);
                    audio_source_current_action = AUDIO_SOURCE_ACTION_PLAY;

                    if(bt_get_link_mode() == BLM_SNIFF_MODE) {
                        bt_stop_sniff(user_bt_env->dev[user_bt_env->last_active_dev_index].remDev);
                    }
                    else {
                        a2dp_start_stream(user_bt_env->dev[user_bt_env->last_active_dev_index].pstream);
                    }
                }
            }

            break;
        case USER_EVT_DSP_CLOSED:
            if(audio_source_env.sbc_pkt_num) {
                audio_source_sbc_list_free();
            }
            audio_source_state = AUDIO_SOURCE_STATE_IDLE;
            audio_source_current_action = AUDIO_SOURCE_ACTION_NONE;
            break;
        case USER_EVT_BT_EXIT_SNIFF:
            if(audio_source_current_action == AUDIO_SOURCE_ACTION_PLAY) {
                a2dp_start_stream(user_bt_env->dev[user_bt_env->last_active_dev_index].pstream);
            }
            break;
        case USER_EVT_A2DP_STREAM_STARTED:
            if(audio_source_current_action == AUDIO_SOURCE_ACTION_PLAY) {
                audio_source_env.a2dp_stream_start_flag = true;

                if(audio_source_state == AUDIO_SOURCE_STATE_IDLE) {
                    audio_source_env.sbc_pkt_num = 0;
                    ipc_msg_with_payload_send((enum ipc_msg_type_t)IPC_MSG_SET_SBC_CODEC_PARAM, (void *)&sbcinfo, sizeof(SbcStreamInfo), NULL, 0, NULL);
                    ipc_msg_send((enum ipc_msg_type_t)IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_DECODER_START, NULL);
                }
                else {
                    /* stream is resumed in pause mode, send following raw data to DSP requested previously */
                    if(dsp_buffer_space) {
                        audio_source_state = AUDIO_SOURCE_STATE_PLAY;
                        audio_source_current_action = AUDIO_SOURCE_ACTION_NONE;
                        send_raw_data_to_dsp(0xff);
                    }
                }
                audio_source_current_action = AUDIO_SOURCE_ACTION_NONE;
                audio_source_state = AUDIO_SOURCE_STATE_PLAY;

                i2s_init_(I2S_DIR_TX, 12000000, 44100, I2S_MODE_MASTER);
                i2s_start_();
                NVIC_SetPriority(I2S_IRQn, 2);
                NVIC_EnableIRQ(I2S_IRQn);
            }
            break;
        case USER_EVT_A2DP_STREAM_SUSPENDED:
            audio_source_env.a2dp_stream_start_flag = false;
            i2s_stop_();
            NVIC_DisableIRQ(I2S_IRQn);
            audio_source_current_action = AUDIO_SOURCE_ACTION_NONE;
            audio_source_state = AUDIO_SOURCE_STATE_PAUSE;
            dsp_working_label_clear(DSP_WORKING_LABEL_AUDIO_SOURCE);
            break;
        case USER_EVT_NEW_SBC_FRAME:
            {
                uint32_t *param = arg;
                uint8_t *buffer = (uint8_t *)param[0];
                struct user_a2dp_sbc_packet_t *sbcpacket;

                sbcpacket = (struct user_a2dp_sbc_packet_t *)os_malloc(sizeof(struct user_a2dp_sbc_packet_t));
                sbcpacket->packet = (A2dpSbcPacket *)os_malloc(sizeof(A2dpSbcPacket));
                sbcpacket->packet->frameSize = buffer[0]|(buffer[1]<<8);
                sbcpacket->packet->dataLen = param[1] - 2;
                
                sbcpacket->packet->data = (uint8_t *)os_malloc(sbcpacket->packet->dataLen);
                memcpy(sbcpacket->packet->data, &buffer[2], sbcpacket->packet->dataLen);
                
                GLOBAL_INT_DISABLE();
                audio_source_env.sbc_pkt_num ++;
                co_list_push_back(&audio_source_env.sbc_pkt_list, &sbcpacket->hdr);
                GLOBAL_INT_RESTORE();
            }
            break;
        case USER_EVT_DECODER_NEED_DATA:
            if(audio_source_state == AUDIO_SOURCE_STATE_PLAY){
#if MP3_SRC_LOCAL
                uint16_t length = *(uint16_t *)arg * 256;

                dsp_buffer_space += length;
                send_raw_data_to_dsp(0xff);
#else   // MP3_SRC_LOCAL
                audio_fifo_state_t fifo_state;
                uint16_t length = *(uint16_t *)arg;

                fifo_state.allow_fill_level = length;
                fifo_state.free_space = length;
                fifo_state.total_space = length;
                app_audio_play_env.req_audio_data_cb(&fifo_state);
#endif  // MP3_SRC_LOCAL
            }
            break;
        case USER_EVT_RAW_DATA_TO_DSP_SENT:
            send_raw_data_to_dsp(0xff);
            break;
        default:
            break;
    }
}

bool audio_source_action_play(void)
{
    LOG_INFO("audio_source_action_play\r\n");
    /*
     * condition:
     * 1. current state is not PLAY
     * 2. current action is NONE
     * 
     * procedure:
     * 1. open dsp
     * 2. exit sniff
     * 3. start a2dp stream
     * 4. receive a2dp stream started
     * 4. send IPC_SUB_MSG_DECODER_START to DSP
     * 5. receive USER_EVT_DECODER_NEED_DATA from DSP
     */
#if USB_DEVICE_ENABLE&&MP3_SRC_SDCARD
    if(usb_in_flag != 0){
        return false;
    }
#endif
    if((audio_source_state == AUDIO_SOURCE_STATE_PLAY)
        || (audio_source_current_action != AUDIO_SOURCE_ACTION_NONE)) {
        return false;
    }
    if(is_fs_has_mp3_item() == false){
        return false;
    }
    
    bt_prevent_sniff_set(1);
    audio_source_current_action = AUDIO_SOURCE_ACTION_PLAY;
    
    sdc_init(0, 0, 7, 0);
    sdc_bus_set_clk(0);

    fs_uart_send_mp3_info();

    if(dsp_open(DSP_LOAD_TYPE_AUDIO_SOURCE) == DSP_OPEN_SUCCESS){
        dsp_working_label_set(DSP_WORKING_LABEL_AUDIO_SOURCE);

        if(bt_get_link_mode() == BLM_SNIFF_MODE) {
            bt_stop_sniff(user_bt_env->dev[user_bt_env->last_active_dev_index].remDev);
        }
        else {
            a2dp_start_stream(user_bt_env->dev[user_bt_env->last_active_dev_index].pstream);
        }
    }

    return true;
}

bool audio_source_action_pause(void)
{
    LOG_INFO("audio_source_action_pause\r\n");
    /*
     * condition:
     * 1. current state is play
     * 2. current action is NONE
     * 
     * procedure:
     * 1. change current state
     * 2. suspend a2dp stream
     * 3. receive a2dp stream suspended
     * 4. disable I2S
     * 5. clear DSP working label
     */

    if((audio_source_state != AUDIO_SOURCE_STATE_PLAY)
        || (audio_source_current_action != AUDIO_SOURCE_ACTION_NONE)) {
        return false;
    }

    audio_source_current_action = AUDIO_SOURCE_ACTION_PAUSE;

    a2dp_suspend_stream(user_bt_env->dev[user_bt_env->last_active_dev_index].pstream);

    return true;
}

bool audio_source_action_next(void)
{
    LOG_INFO("audio_source_action_next\r\n");
    /*
     * condition:
     * 1. current action is NONE
     * 
     * procedure:
     * 1. change current state
     * 2. open next file
     * 3. notice DSP to prepare for next track
     * 4. receive USER_EVT_DECODER_PREPARE_NEXT_DONE from DSP
     * 5. send IPC_SUB_MSG_DECODER_START to DSP
     * 6. receive USER_EVT_DECODER_NEED_DATA from DSP
     */
#if USB_DEVICE_ENABLE&&MP3_SRC_SDCARD
    if(usb_in_flag != 0){
        return false;
    }
#endif

    /*
    if((audio_source_state != AUDIO_SOURCE_STATE_PLAY)
        || (audio_source_current_action != AUDIO_SOURCE_ACTION_NONE)) {
        return false;
    }*/
    if(audio_source_current_action != AUDIO_SOURCE_ACTION_NONE) {
        return false;
    }
    if(is_fs_has_mp3_item() == false){
        return false;
    }
    bt_prevent_sniff_set(1);
    
    sdc_init(0, 0, 7, 0);
    sdc_bus_set_clk(0);

    audio_source_current_action = AUDIO_SOURCE_ACTION_NEXT;
    
    /* open next music file to play */
    fs_prepare_next();
    fs_uart_send_mp3_info();

    if(dsp_open(DSP_LOAD_TYPE_AUDIO_SOURCE) == DSP_OPEN_SUCCESS){
        /*
         * notice DSP to prepare for next song, no IPC_MSG_RAW_FRAME and IPC_SUB_MSG_NEED_MORE_SBC should be send 
         * once this message is send to avoid confusion in DSP side.
         */
        ipc_msg_send((enum ipc_msg_type_t)IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_DECODER_PREP_NEXT,NULL);
        
    }

    return true;
}

bool audio_source_action_prev(void)
{
    LOG_INFO("audio_source_action_prev\r\n");
    /*
     * condition:
     * 1. current action is NONE
     * 
     * procedure:
     * 1. change current state
     * 2. open prev file
     * 3. notice DSP to prepare for next track
     * 4. receive USER_EVT_DECODER_PREPARE_NEXT_DONE from DSP
     * 5. send IPC_SUB_MSG_DECODER_START to DSP
     * 6. receive USER_EVT_DECODER_NEED_DATA from DSP
     */
#if USB_DEVICE_ENABLE&&MP3_SRC_SDCARD
    if(usb_in_flag != 0){
        return false;
    }
#endif

    /*
    if((audio_source_state != AUDIO_SOURCE_STATE_PLAY)
        || (audio_source_current_action != AUDIO_SOURCE_ACTION_NONE)) {
        return false;
    }*/
    if(audio_source_current_action != AUDIO_SOURCE_ACTION_NONE) {
        return false;
    }
    
    if(is_fs_has_mp3_item() == false){
        return false;
    }
    
    bt_prevent_sniff_set(1);

    audio_source_current_action = AUDIO_SOURCE_ACTION_PREV;
    
    sdc_init(0, 0, 7, 0);
    sdc_bus_set_clk(0);
    /* open next music file to play */
    fs_prepare_prev();
    fs_uart_send_mp3_info();

    if(dsp_open(DSP_LOAD_TYPE_AUDIO_SOURCE) == DSP_OPEN_SUCCESS){
        /*
         * notice DSP to prepare for next song, no IPC_MSG_RAW_FRAME and IPC_SUB_MSG_NEED_MORE_SBC should be send 
         * once this message is send to avoid confusion in DSP side.
         */
        ipc_msg_send((enum ipc_msg_type_t)IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_DECODER_PREP_NEXT,NULL);
        
    }

    return true;

}

void audio_source_action_fast_forward(void)
{
}

void audio_source_action_fast_backward(void)
{
}

#if MP3_SRC_LOCAL == 0
void app_bt_audio_play_init(void)
{
    memset(&app_audio_play_env,0,sizeof(struct app_audio_play_env_t));
    //app_audio_play_env.audio_play_buf = os_malloc(APP_BT_AUDIO_PLAY_BUF_MAX_SIZE);
    co_list_init(&app_audio_play_env.audio_buf_list);
}

void app_bt_audio_play_buf_free(void)
{
    struct audio_packet_t *audio_pkt;

    while(!co_list_is_empty(&app_audio_play_env.audio_buf_list)){
        audio_pkt = (struct audio_packet_t *)co_list_pop_front(&app_audio_play_env.audio_buf_list);
        os_free((void *)audio_pkt->packet);
        os_free((void *)audio_pkt);
    }
}

allow_play_state_t app_bt_check_is_allow_play_audio(check_is_allow_play_audio_hook hook_cb)
{

    allow_play_state_t  state;
    if(bt_get_a2dp_type() == BT_A2DP_TYPE_SINK){
        state = PLAY_OTHER;
    }else{
        if(user_bt_env->dev[user_bt_env->last_active_dev_index].prf_all_connected == true){
            state = PLAY_ALLOWABLE;
        }else{
            state = PLAY_UNALLOWED;
            app_audio_play_env.audio_play_req_flag = true;
            app_audio_play_env.audio_play_allowed_cb = hook_cb;
        }
    }

    return state;
}
void ido_send_raw_data_to_dsp_done(uint8_t channel);

uint32_t app_bt_fill_audio_data(uint8_t *data_buf, uint32_t data_len)
{
    send_raw_data_to_dsp(data_buf,data_len);
    return NRF_SUCCESS;
}

uint32_t app_bt_report_audio_fifo_state_register(report_audio_fifo_state fifo_state_cb)
{
    app_audio_play_env.req_audio_data_cb = fifo_state_cb;
    return NRF_SUCCESS;
}

uint32_t app_bt_get_audio_fifo_state(audio_fifo_state_t *fifo_state)
{
    ///用户自己管理dsp需求的raw data size，从app_bt_report_audio_fifo_state_register注册
    ///的回调函数中获取
    
    //GLOBAL_INT_DISABLE();
    //fifo_state->free_space = app_audio_play_env.dsp_req_len;
    //GLOBAL_INT_RESTORE();
    return NRF_SUCCESS;
}

uint32_t app_bt_event_launch_new_play_audio(void)
{
    if((bt_get_a2dp_type() == BT_A2DP_TYPE_SRC)
        &&(user_bt_env->dev[user_bt_env->last_active_dev_index].prf_all_connected == true)){
        if(BT_STATUS_PENDING == a2dp_start_stream(user_bt_env->dev[user_bt_env->last_active_dev_index].pstream)){
            if(user_bt_env->dev[user_bt_env->last_active_dev_index].mode == BLM_SNIFF_MODE){
                ipc_mp3_start_flag = 1;
                bt_stop_sniff(user_bt_env->dev[user_bt_env->last_active_dev_index].remDev);
            }else
            {
                ipc_mp3_start();
            }
            return NRF_SUCCESS; 
        }else{
            return NRF_ERROR_BUSY;
        }
    }else{
            return NRF_ERROR_INVALID_STATE;
    }
}

uint32_t app_bt_event_complet_current_play_audio(void)
{
    if((bt_get_a2dp_type() == BT_A2DP_TYPE_SRC)&&(bt_get_state() == BT_STATE_MEDIA_PLAYING)){
        if(BT_STATUS_PENDING == a2dp_suspend_stream(user_bt_env->dev[user_bt_env->last_active_dev_index].pstream)){
            app_audio_play_env.audio_event |= AUDIO_EVENT_END;
            return NRF_SUCCESS; 
        }else{
            return NRF_ERROR_BUSY;
        }
    }else{
            return NRF_ERROR_INVALID_STATE;
    }
}

uint32_t app_bt_ack_event_complet_current_play_audio_register(ack_event_complet_current_play_audio ack_evt_cplt_cb)
{
    app_audio_play_env.audio_play_complet_ack_cb = ack_evt_cplt_cb;
    
    return NRF_SUCCESS; 
}

uint32_t app_bt_event_notif(play_event_t play_evt)
{
    
    if(play_evt == SWITCH_SONG){
        app_bt_audio_play_buf_free();
        if((bt_get_a2dp_type() == BT_A2DP_TYPE_SRC)&&(bt_get_state() == BT_STATE_MEDIA_PLAYING)){
            if(BT_STATUS_PENDING == a2dp_suspend_stream(user_bt_env->dev[user_bt_env->last_active_dev_index].pstream)){
                app_audio_play_env.audio_event |= AUDIO_EVENT_SWITCH;
                return NRF_SUCCESS; 
            }else{
                return NRF_ERROR_BUSY;
            }
        }else{
                return NRF_ERROR_INVALID_STATE;
        }
    }else if(play_evt == PAUSE_PLAY){
        ///���浱ǰmp3������ʽ����
        ///...
        app_bt_audio_play_buf_free();
        if((bt_get_a2dp_type() == BT_A2DP_TYPE_SRC)&&(bt_get_state() == BT_STATE_MEDIA_PLAYING)){
            if(BT_STATUS_PENDING == a2dp_suspend_stream(user_bt_env->dev[user_bt_env->last_active_dev_index].pstream)){
                app_audio_play_env.audio_event |= AUDIO_EVENT_PAUSE;
                return NRF_SUCCESS; 
            }else{
                return NRF_ERROR_BUSY;
            }
        }else{
                return NRF_ERROR_INVALID_STATE;
        }
        
    }else if(play_evt == CONTINUE_PLAY){
        ///�ָ������mp3������ʽ����
        ///...����dsp
        if((bt_get_a2dp_type() == BT_A2DP_TYPE_SRC)
            &&(user_bt_env->dev[user_bt_env->last_active_dev_index].prf_all_connected == true)){
            if(BT_STATUS_PENDING == a2dp_start_stream(user_bt_env->dev[user_bt_env->last_active_dev_index].pstream)){
                if(user_bt_env->dev[user_bt_env->last_active_dev_index].mode == BLM_SNIFF_MODE){
                    ipc_mp3_start_flag = 1;
                    bt_stop_sniff(user_bt_env->dev[user_bt_env->last_active_dev_index].remDev);
                }else
                {
                    ipc_mp3_start();
                }
                
                app_audio_play_env.audio_event |= AUDIO_EVENT_CONTINUE;
                return NRF_SUCCESS; 
            }else{
                return NRF_ERROR_BUSY;
            }
        }else{
                return NRF_ERROR_INVALID_STATE;
        }
    }

    return NRF_ERROR_INVALID_PARAM;
}

uint32_t app_bt_ack_event_complet_register(ack_notify_event_complet ack_notify_cbk)
{
    app_audio_play_env.audio_play_notify_ack_cb = ack_notify_cbk;
    
    return NRF_SUCCESS;
}

uint32_t app_bt_report_exception_event_register(report_excepion_event_state exception_state_cb)
{
    app_audio_play_env.exception_state_cb = exception_state_cb;
    
    return NRF_SUCCESS;
}
#endif
