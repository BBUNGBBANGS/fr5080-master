#ifndef _USER_IPC_H
#define _USER_IPC_H
#include "ipc_load_code.h"

enum ipc_user_sub_msg_type_t {
    IPC_SUB_MSG_NEED_MORE_SBC,  // MCU notice DSP to send more sbc frame
    IPC_SUB_MSG_DECODER_START,  // MCU notice DSP to initiate MP3, AAC, etc. decoder
    IPC_SUB_MSG_REINIT_DECODER, // MCU notice DSP to reinitiate decoder engine
    IPC_SUB_MSG_NREC_START,         // MCU notice DSP to start nrec algorithm, not used,start immeadiately after user data load done
    IPC_SUB_MSG_NREC_STOP,          // MCU notice DSP to stop nrec algorithm
    IPC_SUB_MSG_DECODER_STOP,   // MCU notice DSP to stop adecoder
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
};

void ipc_load_resident_code(void);

void ipc_nrec_start(void);

void ipc_nrec_stop(void);

void ipc_aac_start(void);

void ipc_aac_stop(void);

#endif
