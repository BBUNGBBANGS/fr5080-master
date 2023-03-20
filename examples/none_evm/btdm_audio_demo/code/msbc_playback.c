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

#include "user_fs.h"
#include "user_task.h"
#include "user_dsp.h"
#include "msbc_playback.h"
#include "msbc_sample.h"

#define MSBC_SINGLE_FRAME_SIZE              57

#define AUDIO_EVENT_END                     0x01
#define AUDIO_EVENT_SWITCH                  0x02
#define AUDIO_EVENT_PAUSE                   0x04
#define AUDIO_EVENT_CONTINUE                0x08

enum msbc_playback_state_t {
    MSBC_PLAYBACK_STATE_IDLE,
    MSBC_PLAYBACK_STATE_PLAY,
    MSBC_PLAYBACK_STATE_PAUSE,
};

enum msbc_playback_action_t {
    MSBC_PLAYBACK_ACTION_NONE,
    MSBC_PLAYBACK_ACTION_PLAY,
    MSBC_PLAYBACK_ACTION_PAUSE,
    MSBC_PLAYBACK_ACTION_NEXT,
    MSBC_PLAYBACK_ACTION_PREV,
    MSBC_PLAYBACK_ACTION_FAST_FORWARD,
    MSBC_PLAYBACK_ACTION_FAST_BACKWARD,
};

static enum msbc_playback_state_t msbc_playback_state;
static enum msbc_playback_action_t msbc_playback_current_action = MSBC_PLAYBACK_ACTION_NONE;

static uint32_t dsp_buffer_space = 0;
static uint8_t msbc_single_frame[MSBC_SINGLE_FRAME_SIZE];

bool msbc_playback_action_play(void);
bool msbc_playback_action_next(void);

void msbc_playback_init(void)
{
}

bool msbc_request_raw_data(uint8_t *buffer, uint32_t length)
{
    static uint32_t read_index = 0;
    uint32_t sample_total_length = sizeof(msbc_sample);
    uint32_t last_length;
    
    last_length = sample_total_length - read_index;
    if(last_length >= length) {
        memcpy(buffer, &msbc_sample[read_index], length);
        read_index += length;
        if(read_index >= sample_total_length) {
            read_index = 0;
        }
    }
    else {
        memcpy(buffer, &msbc_sample[read_index], last_length);
        length -= last_length;
        memcpy(&buffer[last_length], &msbc_sample[0], length);
        read_index = length;
    }
    
    return true;
}

static void msbc_playback_start(void)
{
    dsp_buffer_space = 0;
    
    msbc_playback_current_action = MSBC_PLAYBACK_ACTION_NONE;
    msbc_playback_state = MSBC_PLAYBACK_STATE_PLAY;

    ipc_msg_send((enum ipc_msg_type_t)IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_DECODER_START_LOCAL, NULL);
    
    codec_dac_reg_config(AUDIO_CODEC_SAMPLE_RATE_DAC_16000, AUDIO_CODEC_DAC_SRC_IPC_DMA);
    ipc_config_media_dma(1);
}

static __attribute__((section("ram_code"))) void send_raw_data_to_dsp(uint8_t channel)
{
    if((msbc_playback_state != MSBC_PLAYBACK_STATE_PLAY)
        || (msbc_playback_current_action != MSBC_PLAYBACK_ACTION_NONE)) {
        return;
    }

    if(dsp_buffer_space != 0) {
        uint32_t request_size = MSBC_SINGLE_FRAME_SIZE * 4;
        while(request_size > dsp_buffer_space) {
            request_size -= MSBC_SINGLE_FRAME_SIZE;
        }
        if(msbc_request_raw_data(&msbc_single_frame[0], request_size)) {
            ipc_msg_with_payload_send((enum ipc_msg_type_t)IPC_MSG_RAW_FRAME, NULL, 0, (uint8_t *)&msbc_single_frame[0], request_size, send_raw_data_to_dsp);

            CPU_SR cpu_sr;
            GLOBAL_INT_OLD_DISABLE();
            dsp_buffer_space -= request_size;
            GLOBAL_INT_OLD_RESTORE();
        }
    }
}

void msbc_playback_statemachine(uint8_t event, void *arg)
{
    // LOG_INFO("audio_source_statemachine: %d.\r\n", event);

    switch(event) {
        case USER_EVT_DSP_OPENED:
        case USER_EVT_DECODER_PREPARE_NEXT_DONE:
            if((msbc_playback_current_action == MSBC_PLAYBACK_ACTION_PLAY)
                ||(msbc_playback_current_action == MSBC_PLAYBACK_ACTION_NEXT)){
                dsp_working_label_set(DSP_WORKING_LABEL_MSBC_PLAYBACK);
                msbc_playback_start();
            }

            break;
        case USER_EVT_DSP_CLOSED:
            msbc_playback_state = MSBC_PLAYBACK_STATE_IDLE;
            msbc_playback_current_action = MSBC_PLAYBACK_ACTION_NONE;
            break;
        case USER_EVT_DECODER_NEED_DATA:
            if(msbc_playback_state == MSBC_PLAYBACK_STATE_PLAY){
                uint16_t length = *(uint16_t *)arg * MSBC_SINGLE_FRAME_SIZE;

                dsp_buffer_space += length;
                send_raw_data_to_dsp(0xff);
            }
            break;
        default:
            break;
    }
}

bool msbc_playback_action_play(void)
{
    LOG_INFO("msbc_playback_action_play\r\n");
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

    if((msbc_playback_state == MSBC_PLAYBACK_STATE_PLAY)
        || (msbc_playback_current_action != MSBC_PLAYBACK_ACTION_NONE)) {
        return false;
    }

    msbc_playback_current_action = MSBC_PLAYBACK_ACTION_PLAY;

    if(dsp_open(DSP_LOAD_TYPE_MSBC_PLAYBACK) == DSP_OPEN_SUCCESS) {
        dsp_working_label_set(DSP_WORKING_LABEL_MSBC_PLAYBACK);

        msbc_playback_start();
    }

    return true;
}

bool msbc_playback_action_pause(void)
{
    LOG_INFO("msbc_playback_action_pause\r\n");
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

    if((msbc_playback_state != MSBC_PLAYBACK_STATE_PLAY)
        || (msbc_playback_current_action != MSBC_PLAYBACK_ACTION_NONE)) {
        return false;
    }
        
    codec_off_reg_config();
    msbc_playback_current_action = MSBC_PLAYBACK_ACTION_NONE;
    msbc_playback_state = MSBC_PLAYBACK_STATE_PAUSE;
    dsp_working_label_clear(DSP_WORKING_LABEL_MSBC_PLAYBACK);

    return true;
}

bool msbc_playback_action_next(void)
{
    LOG_INFO("msbc_playback_action_next\r\n");
    /*
     * condition:
     * 1. current state is play
     * 2. current action is NONE
     * 
     * procedure:
     * 1. change current state
     * 2. open next file
     * 3. notice DSP to prepare for next track
     * 4. receive USER_EVT_DECODER_PREPARE_NEXT_DONE from DSP
     * 5. send IPC_SUB_MSG_DECODER_START to DSP
     * 6. receive USER_EVT_DECODER_NEED_DATA from DSP
     */

    if((msbc_playback_state != MSBC_PLAYBACK_STATE_PLAY)
        || (msbc_playback_current_action != MSBC_PLAYBACK_ACTION_NONE)) {
        return false;
    }

    msbc_playback_current_action = MSBC_PLAYBACK_ACTION_NEXT;

    /* open next music file to play */
    fs_prepare_next();

    /*
     * notice DSP to prepare for next song, no IPC_MSG_RAW_FRAME and IPC_SUB_MSG_NEED_MORE_SBC should be send 
     * once this message is send to avoid confusion in DSP side.
     */
    ipc_msg_send((enum ipc_msg_type_t)IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_DECODER_PREP_NEXT,NULL);
    
    return true;
}

void msbc_playback_action_prev(void)
{
}

void msbc_playback_action_fast_forward(void)
{
}

void msbc_playback_action_fast_backward(void)
{
}
