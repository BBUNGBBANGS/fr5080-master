#ifndef _USER_DSP_H
#define _USER_DSP_H

#include <stdbool.h>
#include <stdint.h>

/*
 * used to indicate which task DSP is executing. DSP may run more than one task at the
 * same time. DSP should not be closed until all LABEL are cleared. @ref dsp_working_label_set
 * and dsp_working_label_clear
 */
#define DSP_WORKING_LABEL_GUI                   0x01
#define DSP_WORKING_LABEL_VOICE_ALGO            0x02
#define DSP_WORKING_LABEL_AUDIO_SOURCE          0x04
#define DSP_WORKING_LABEL_NATIVE_PLAYBACK       0x08
#define DSP_WORKING_LABEL_MSBC_PLAYBACK         0x10
#define DSP_WORKING_LABEL_MSBC_ENCODER          0x20

enum ipc_user_sub_msg_type_t {
    IPC_SUB_MSG_NEED_MORE_SBC,      // MCU notice DSP to send more sbc frame
    IPC_SUB_MSG_DECODER_START,      // MCU notice DSP to initiate MP3, AAC, etc. decoder
    IPC_SUB_MSG_REINIT_DECODER,     // MCU notice DSP to reinitiate decoder engine
    IPC_SUB_MSG_NREC_START,         // MCU notice DSP to start nrec algorithm, not used,start immeadiately after user data load done
    IPC_SUB_MSG_NREC_STOP,          // MCU notice DSP to stop nrec algorithm
    IPC_SUB_MSG_FLASH_COPY_ACK,
    IPC_SUM_MSG_DSP_USER_CODE_READY,
    IPC_SUB_MSG_DECODER_STOP,       // MCU notice DSP to stop adecoder
    IPC_SUB_MSG_DECODER_PREP_NEXT,  // MCU notice DSP to prepare for next song
    IPC_SUB_MSG_DECODER_PREP_READY, // DSP notice MCU ready to play next song
    IPC_SUB_MSG_DECODER_START_LOCAL,// MCU notice DSP to initiate MP3, AAC, etc. decoder for native playback
    IPC_SUB_MSG_NEED_MORE_MSBC,     // MCU notice DSP to send more msbc frame
};

enum ipc_user_msg_type_t {
    /*IPC_MSG_LOAD_CODE = 0,
    IPC_MSG_LOAD_CODE_DONE = 1,
    IPC_MSG_EXEC_USER_CODE = 2,
    IPC_MSG_DSP_READY = 10,*/
    IPC_MSG_RAW_FRAME = 3,          // MCU send new raw frame to DSP
    IPC_MSG_DECODED_PCM_FRAME = 4,  // DSP send decoded pcm data to MCU
    IPC_MSG_ENCODED_SBC_FRAME = 5,  // DSP send encoded sbc frame to MCU
    IPC_MSG_WITHOUT_PAYLOAD = 6,    // some command without payload, use length segment in ipc-msg to indicate sub message
    IPC_MSG_RAW_BUFFER_SPACE = 7,   // used by DSP to tell MCU how much buffer space left to save raw data, return_value = actual_length / 256
    IPC_MSG_FLASH_OPERATION = 8,
    IPC_MSG_SET_SBC_CODEC_PARAM = 9, // MCU send codec sbc parameters to DSP(bitpool,sample rate...)
    IPC_MSG_ENCODED_MSBC_FRAME = 11, // DSP send encoded msbc frame to MCU
};

enum dsp_load_type_t {
    /* 
     * when dsp works in XIP mode, only basic code need to be loaded. when dsp works in
     * RAM mode, basic code need to be loaded before loading user code.
     */
    DSP_LOAD_TYPE_BASIC,

    /* when dsp works in RAM mode, different applications need to be load dynamicly in different scenarios */
    DSP_LOAD_TYPE_VOICE_ALGO,       // running voice algorithm when making call, such as AEC, NR, etc.
    DSP_LOAD_TYPE_A2DP_DECODER,     // used to decode a2dp stream, such as AAC, LC3, etc.
    DSP_LOAD_TYPE_AUDIO_SOURCE,     // used when working in audio source mode, decode MP3 files and encode these data to SBC packet
    DSP_LOAD_TYPE_NATIVE_PLAYBACK,  // decode MP3 files and playback with codec on chip
    DSP_LOAD_TYPE_MSBC_PLAYBACK,    // decode msbc files and playback with codec on chip
    DSP_LOAD_TYPE_MSBC_ENCODER,     // encoder msbc
};

enum dsp_state_t {
    DSP_STATE_CLOSED,
    DSP_STATE_OPENING,
    DSP_STATE_OPENED,
};

enum dsp_open_result_t {
    DSP_OPEN_SUCCESS,
    DSP_OPEN_FAILED,
    DSP_OPEN_PENDING,
};

/*********************************************************************
 * @fn      dsp_working_label_clear
 *
 * @brief   set dsp working label, dsp should be kept in opened state while any bit is set.
 *
 * @param   label   - target mode
 *
 * @return  true or false
 */
void dsp_working_label_set(uint16_t label);

/*********************************************************************
 * @fn      dsp_working_label_clear
 *
 * @brief   clear dsp working label, dsp should be closed when all bits are cleared.
 *
 * @param   label   - target mode
 *
 * @return  true or false
 */
void dsp_working_label_clear(uint16_t label);

/*********************************************************************
 * @fn      dsp_open
 *
 * @brief   used to open dsp with specific mode.
 *
 * @param   type    - target type. @ref dsp_load_type_t
 *
 * @return  @ref dsp_open_result_t
 */
enum dsp_open_result_t dsp_open(enum dsp_load_type_t type);

/*********************************************************************
 * @fn      dsp_close
 *
 * @brief   used to close dsp to save power.
 *
 * @param   None
 *
 * @return  None
 */
void dsp_close(void);

/*********************************************************************
 * @fn      dsp_get_state
 *
 * @brief   get current dsp state.
 *
 * @param   None
 *
 * @return  @ref dsp_state_t
 */
enum dsp_state_t dsp_get_state(void);

#endif  // _USER_DSP_H

