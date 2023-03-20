/*
 * @Author: your name
 * @Date: 2021-02-22 17:30:43
 * @LastEditTime: 2021-06-18 20:04:13
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \projd:\My Document\fr5080_sdk\examples\none_evm\btdm_audio_demo\code\audio_source.h
 */
#ifndef _AUDIO_PLAY_H
#define _AUDIO_PLAY_H
//#include "rwip_task.h"
#include "user_utils.h"
#include "co_list.h"
#include "a2dp_api.h"

struct audio_source_env_tag{
    co_list_t sbc_pkt_list;
    uint16_t sbc_pkt_num;
    uint8_t a2dp_stream_start_flag;

};

struct user_a2dp_sbc_packet_t{
    struct co_list_hdr hdr;
    A2dpSbcPacket *packet;
};

extern struct audio_source_env_tag audio_source_env;

void audio_source_statemachine(uint8_t event, void *arg);

bool audio_source_action_play(void);
bool audio_source_action_pause(void);
bool audio_source_action_next(void);
bool audio_source_action_prev(void);

void audio_source_init(void);
void audio_source_i2s_trig_handler(void);

#endif
