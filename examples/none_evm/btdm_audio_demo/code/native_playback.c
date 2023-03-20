#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "co_log.h"
#include "os_mem.h"
#include "os_task.h"
#include "user_utils.h"

#include "driver_codec.h"
#include "driver_ipc.h"
#include "driver_sdc.h"
#include "driver_syscntl.h"

#include "user_fs.h"
#include "user_task.h"
#include "user_dsp.h"
#include "native_playback.h"
#include "app_at.h"
#include "user_bt.h"

#define MAX_SBC_FRAME_NUM                   10

#define APP_BT_AUDIO_PLAY_BUF_MAX_SIZE      2048

#define AUDIO_EVENT_END                     0x01
#define AUDIO_EVENT_SWITCH                  0x02
#define AUDIO_EVENT_PAUSE                   0x04
#define AUDIO_EVENT_CONTINUE                0x08

enum native_playback_state_t {
    NATIVE_PLAYBACK_STATE_IDLE,
    NATIVE_PLAYBACK_STATE_PLAY,
    NATIVE_PLAYBACK_STATE_PAUSE,
};

enum native_playback_action_t {
    NATIVE_PLAYBACK_ACTION_NONE,
    NATIVE_PLAYBACK_ACTION_PLAY,
    NATIVE_PLAYBACK_ACTION_PAUSE,
    NATIVE_PLAYBACK_ACTION_NEXT,
    NATIVE_PLAYBACK_ACTION_PREV,
    NATIVE_PLAYBACK_ACTION_FAST_FORWARD,
    NATIVE_PLAYBACK_ACTION_FAST_BACKWARD,
    NATIVE_PLAYBACK_ACTION_SPECIFIED,
};

static enum native_playback_state_t native_playback_state;
static enum native_playback_action_t native_playback_current_action = NATIVE_PLAYBACK_ACTION_NONE;

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

#if USB_DEVICE_ENABLE
extern uint8_t usb_in_flag;
#endif

bool native_playback_action_play(void);
bool native_playback_action_next(void);

void native_playback_init(void)
{
}

static void native_playback_start(void)
{
    dsp_buffer_space = 0;
    if(native_playback_state != NATIVE_PLAYBACK_STATE_PAUSE){
        ipc_msg_send((enum ipc_msg_type_t)IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_DECODER_START_LOCAL, NULL);
    }
    
    native_playback_current_action = NATIVE_PLAYBACK_ACTION_NONE;
    native_playback_state = NATIVE_PLAYBACK_STATE_PLAY;
    audio_codec_func_start(AUDIO_CODEC_FUNC_NATIVE_PLAYBACK);
}

static void raw_data_to_dsp_sent(uint8_t channel)
{
    os_event_t event;
    event.event_id = USER_EVT_RAW_DATA_TO_DSP_SENT;
    event.param = NULL;
    event.param_len = 0;
    os_msg_post(user_task_id, &event);
}

extern uint8_t user_evt_notify_enable;
static __attribute__((section("ram_code"))) void send_raw_data_to_dsp(uint8_t channel)
{
    uint8_t str[16];
    if((native_playback_state != NATIVE_PLAYBACK_STATE_PLAY)
        || (native_playback_current_action != NATIVE_PLAYBACK_ACTION_NONE)) {
        return;
    }

#if MP3_SRC_LOCAL
    if(dsp_buffer_space != 0) {
#if MP3_SRC_SDCARD
        uint32_t read_size;
        read_size = fs_read(mp3_raw_data, 256);
        if(read_size) {
            mp3_read_total_size += read_size;
            mp3_read_size += read_size;
            if(mp3_read_size >= 80*256){
                mp3_read_size = 0;
                if(user_evt_notify_enable&USER_EVT_NOTIFY_MP3_PROG){
                    co_sprintf((char *)(str),"+PROG:%08x\r\n",mp3_read_total_size);
                    uart_send((void *)str,16);  
                }
            }
            dsp_buffer_space -= read_size;
            ipc_msg_with_payload_send((enum ipc_msg_type_t)IPC_MSG_RAW_FRAME, NULL, 0, mp3_raw_data, 256, send_raw_data_to_dsp);
        }
        else {
            mp3_read_total_size = 0;
            mp3_read_size = 0;
            native_playback_action_next();
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

void native_playback_statemachine(uint8_t event, void *arg)
{
    //LOG_INFO("audio_source_statemachine: %d.\r\n", event);

    switch(event) {
        case USER_EVT_DSP_OPENED:
            dsp_buffer_space = 0;
            if(native_playback_current_action == NATIVE_PLAYBACK_ACTION_PLAY){
                dsp_working_label_set(DSP_WORKING_LABEL_NATIVE_PLAYBACK);
                native_playback_start();
            }
            else if((native_playback_current_action == NATIVE_PLAYBACK_ACTION_NEXT)
                ||(native_playback_current_action == NATIVE_PLAYBACK_ACTION_PREV)
                ||(native_playback_current_action == NATIVE_PLAYBACK_ACTION_SPECIFIED)){
                
                /*
                 * notice DSP to prepare for next song, no IPC_MSG_RAW_FRAME and IPC_SUB_MSG_NEED_MORE_SBC should be send 
                 * once this message is send to avoid confusion in DSP side.
                 */
                ipc_msg_send((enum ipc_msg_type_t)IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_DECODER_PREP_NEXT,NULL);

            }
            
            break;
        case USER_EVT_DECODER_PREPARE_NEXT_DONE:
            dsp_buffer_space = 0;
            if((native_playback_current_action == NATIVE_PLAYBACK_ACTION_NEXT)
                ||(native_playback_current_action == NATIVE_PLAYBACK_ACTION_PREV)
                ||(native_playback_current_action == NATIVE_PLAYBACK_ACTION_SPECIFIED)){
                dsp_working_label_set(DSP_WORKING_LABEL_NATIVE_PLAYBACK);
                native_playback_start();
            }
            break;

        case USER_EVT_DSP_CLOSED:
            native_playback_state = NATIVE_PLAYBACK_STATE_IDLE;
            native_playback_current_action = NATIVE_PLAYBACK_ACTION_NONE;
            break;
        case USER_EVT_DECODER_NEED_DATA:
            if(native_playback_state == NATIVE_PLAYBACK_STATE_PLAY){
#if MP3_SRC_LOCAL
                uint16_t length = *(uint16_t *)arg * 256;

                GLOBAL_INT_DISABLE();
                dsp_buffer_space += length;
                GLOBAL_INT_RESTORE();
                //uart_putc_noint_no_wait('R');
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

bool native_playback_action_play(void)
{
    LOG_INFO("native_playback_action_play,%d,%d\r\n",native_playback_state,native_playback_current_action);
    /*
     * condition:
     * 1. current state is not PLAY
     * 2. current action is NONE
     * 
     * procedure:
     * 1. open dsp
     * 3. send IPC_SUB_MSG_DECODER_START to DSP
     * 4. receive USER_EVT_DECODER_NEED_DATA from DSP
     */
#if USB_DEVICE_ENABLE&&MP3_SRC_SDCARD
    if(usb_in_flag != 0){
        return false;
    }
#endif

    if((native_playback_state == NATIVE_PLAYBACK_STATE_PLAY)
        || (native_playback_current_action != NATIVE_PLAYBACK_ACTION_NONE)) {
        return false;
    }
#if MP3_SRC_LOCAL && MP3_SRC_SDCARD
    if(is_fs_has_mp3_item() == false){
        return false;
    }
#endif
    system_sleep_disable();
    native_playback_current_action = NATIVE_PLAYBACK_ACTION_PLAY;
    
#if MP3_SRC_LOCAL && MP3_SRC_SDCARD
    sdc_init(0, 0, 7, 0);
    sdc_bus_set_clk(0);
    
    fs_uart_send_mp3_info();
#endif

    if(dsp_open(DSP_LOAD_TYPE_NATIVE_PLAYBACK) == DSP_OPEN_SUCCESS) {
        dsp_working_label_set(DSP_WORKING_LABEL_NATIVE_PLAYBACK);

        native_playback_start();
    }

    return true;
}

bool native_playback_action_pause(void)
{
    LOG_INFO("native_playback_action_pause\r\n");
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

    if((native_playback_state != NATIVE_PLAYBACK_STATE_PLAY)
        || (native_playback_current_action != NATIVE_PLAYBACK_ACTION_NONE)) {
        return false;
    }
        
    //codec_off_reg_config();
    audio_codec_func_stop(AUDIO_CODEC_FUNC_NATIVE_PLAYBACK);
    native_playback_current_action = NATIVE_PLAYBACK_ACTION_NONE;
    native_playback_state = NATIVE_PLAYBACK_STATE_PAUSE;
    dsp_working_label_clear(DSP_WORKING_LABEL_NATIVE_PLAYBACK);
    system_sleep_enable();
    return true;
}

bool native_playback_action_next(void)
{
    LOG_INFO("native_playback_action_next\r\n");
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

    if(native_playback_current_action != NATIVE_PLAYBACK_ACTION_NONE) {
        return false;
    }

    if(is_fs_has_mp3_item() == false){
        return false;
    }
    system_sleep_disable();

    native_playback_current_action = NATIVE_PLAYBACK_ACTION_NEXT;
    
    sdc_init(0, 0, 7, 0);
    sdc_bus_set_clk(0);
    /* open next music file to play */
    fs_prepare_next();
    fs_uart_send_mp3_info();
    if(dsp_open(DSP_LOAD_TYPE_NATIVE_PLAYBACK) == DSP_OPEN_SUCCESS){

        /*
         * notice DSP to prepare for next song, no IPC_MSG_RAW_FRAME and IPC_SUB_MSG_NEED_MORE_SBC should be send 
         * once this message is send to avoid confusion in DSP side.
         */
        ipc_msg_send((enum ipc_msg_type_t)IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_DECODER_PREP_NEXT,NULL);
    }

    return true;
}

bool native_playback_action_prev(void)
{
    LOG_INFO("native_playback_action_prev\r\n");
    /*
     * condition:
     * 1. current action is NONE
     * 
     * procedure:
     * 1. change current state
     * 2. open next file
     * 3. notice DSP to prepare for previous track
     * 4. receive USER_EVT_DECODER_PREPARE_NEXT_DONE from DSP
     * 5. send IPC_SUB_MSG_DECODER_START to DSP
     * 6. receive USER_EVT_DECODER_NEED_DATA from DSP
     */
#if USB_DEVICE_ENABLE&&MP3_SRC_SDCARD
    if(usb_in_flag != 0){
        return false;
    }
#endif

    if(native_playback_current_action != NATIVE_PLAYBACK_ACTION_NONE) {
        return false;
    }
    if(is_fs_has_mp3_item() == false){
        return false;
    }
    
    system_sleep_disable();
    native_playback_current_action = NATIVE_PLAYBACK_ACTION_PREV;
    
    sdc_init(0, 0, 7, 0);
    sdc_bus_set_clk(0);
    /* open previous music file to play */
    fs_prepare_prev();
    fs_uart_send_mp3_info();
    if(dsp_open(DSP_LOAD_TYPE_NATIVE_PLAYBACK) == DSP_OPEN_SUCCESS){
        /*
         * notice DSP to prepare for next song, no IPC_MSG_RAW_FRAME and IPC_SUB_MSG_NEED_MORE_SBC should be send 
         * once this message is send to avoid confusion in DSP side.
         */
        ipc_msg_send((enum ipc_msg_type_t)IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_DECODER_PREP_NEXT,NULL);
    }

    return true;


}

bool native_playback_action_specified(void *item)
{
    LOG_INFO("native_playback_action_specified,%x\r\n",item);
    /*
     * condition:
     * 1. current action is NONE
     * 
     * procedure:
     * 1. change current state
     * 2. open next file
     * 3. notice DSP to prepare for previous track
     * 4. receive USER_EVT_DECODER_PREPARE_NEXT_DONE from DSP
     * 5. send IPC_SUB_MSG_DECODER_START to DSP
     * 6. receive USER_EVT_DECODER_NEED_DATA from DSP
     */
#if USB_DEVICE_ENABLE&&MP3_SRC_SDCARD
    if(usb_in_flag != 0){
        return false;
    }
#endif

    if(native_playback_current_action != NATIVE_PLAYBACK_ACTION_NONE) {
        return false;
    }
    if(is_fs_has_mp3_item() == false){
        return false;
    }
    system_sleep_disable();

    native_playback_current_action = NATIVE_PLAYBACK_ACTION_SPECIFIED;
    
    sdc_init(0, 0, 7, 0);
    sdc_bus_set_clk(0);

    /* open specified music file to play */
    fs_prepare_specified(item);
    fs_uart_send_mp3_info();
    if(dsp_open(DSP_LOAD_TYPE_NATIVE_PLAYBACK) == DSP_OPEN_SUCCESS){
        /*
         * notice DSP to prepare for specified song, no IPC_MSG_RAW_FRAME and IPC_SUB_MSG_NEED_MORE_SBC should be send 
         * once this message is send to avoid confusion in DSP side.
         */
        ipc_msg_send((enum ipc_msg_type_t)IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_DECODER_PREP_NEXT,NULL);
    }

    return true;


}

void native_playback_action_fast_forward(void)
{
}

void native_playback_action_fast_backward(void)
{
}

bool user_check_native_playback_allowable(uint8_t event)
{
    bool ret = true;
    switch(event)
    {
        case USER_EVT_NATIVE_PLAYBACK_START:
            #if USB_DEVICE_ENABLE&&MP3_SRC_SDCARD
            if(usb_in_flag != 0){
                ret = false;
            }
            #endif
            if((native_playback_state == NATIVE_PLAYBACK_STATE_PLAY)
                || (native_playback_current_action != NATIVE_PLAYBACK_ACTION_NONE)) {
                ret = false;
            }
        break;

        case USER_EVT_NATIVE_PLAYBACK_STOP:
            if((native_playback_state != NATIVE_PLAYBACK_STATE_PLAY)
                || (native_playback_current_action != NATIVE_PLAYBACK_ACTION_NONE)) {
                ret = false;
            }
        break;

        case USER_EVT_NATIVE_PLAYBACK_NEXT:
        case USER_EVT_NATIVE_PLAYBACK_PREV:
            #if USB_DEVICE_ENABLE&&MP3_SRC_SDCARD
            if(usb_in_flag != 0){
                ret = false;
            }
            #endif
            if(native_playback_current_action != NATIVE_PLAYBACK_ACTION_NONE) {
                ret = false;
            }
        break;

        default:
            ret = false;
        break;
    }
    return ret;
}
