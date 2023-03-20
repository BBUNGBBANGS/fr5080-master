/*!**********************************************************************************
    \file
        app_user_bt.h
    \brief  
    定义了推流无线耳机模块的接口细节，详细参考《音频工作流�?.vsdx�?
        
    CopyRight @2020 Idoo
    Revision:
    2020-9-21   chenguohui 创建

***********************************************************************************/ 


#ifndef __APP_BT_H__
#define __APP_BT_H__
#include "stdint.h"
#include "co_list.h"


#define AUDIO_EVENT_END         0x01
#define AUDIO_EVENT_SWITCH      0x02
#define AUDIO_EVENT_PAUSE       0x04
#define AUDIO_EVENT_CONTINUE    0x08

typedef enum _allow_play_state
{
    PLAY_ALLOWABLE,          //允许播放
    PLAY_UNALLOWED,          //不允许播�?
    PLAY_OTHER,               //其他状态，有异�?

    PLEASE_BEFORE_MAX_ALLOW_PLAY_STATE,
    MAX_ALLOW_PLAY_STATE = PLEASE_BEFORE_MAX_ALLOW_PLAY_STATE,    
}allow_play_state_t;

typedef struct _audio_fifo_state
{
    uint32_t  total_space;            //总空�?
    uint32_t  free_space;             //剩余可用空间
    uint32_t  allow_fill_level;       //可新喂数据水�?
}audio_fifo_state_t;

typedef enum _play_event
{
    SWITCH_SONG,
    PAUSE_PLAY,
    CONTINUE_PLAY,

    PLEASE_BEFORE_MAX_PALAY_EVENT,
    MAX_PALAY_EVENT = PLEASE_BEFORE_MAX_PALAY_EVENT,
}play_event_t;

typedef enum _exception_state
{
    TWS_BATT_LOW,
    TWS_DISCONNECT,
    OTHER_ERROR,
    
    PLEASE_BEFORE_MAX_EXCEPTION,
    MAX_EXCEPTION = PLEASE_BEFORE_MAX_EXCEPTION,
}exception_state_t;
    
typedef void(*check_is_allow_play_audio_hook)(allow_play_state_t allow_state, audio_fifo_state_t *fifo_state);
typedef void(*report_audio_fifo_state)(audio_fifo_state_t *fifo_state);
typedef void(*ack_event_complet_current_play_audio)(allow_play_state_t allow_state);
typedef void(*report_excepion_event_state)(exception_state_t exception_state);
typedef void(*ack_notify_event_complet)(play_event_t play_evt );

struct audio_packet_t{
    struct co_list_hdr hdr;
    uint8_t *packet;
    uint32_t len;
};

struct app_audio_play_env_t{
    uint8_t audio_play_req_flag;        //���յ���ѯ�Ƿ���Բ���������ӽ����ú󣬻ص�audio_play_allowed_cb
    uint32_t dsp_req_len;               //dsp��Ҫԭʼmp3���ݵĳ���
    co_list_t audio_buf_list;           //mcu���յ�ԭʼmp3��������
    //uint16_t audio_save_num;            //
    uint32_t audio_event;               //���յ���event
    check_is_allow_play_audio_hook audio_play_allowed_cb;   //���Կ�ʼ���Żص�
    report_audio_fifo_state req_audio_data_cb;  //����ԭʼmp3���ݻص�
    ack_event_complet_current_play_audio audio_play_complet_ack_cb; //����������ɻص�
    ack_notify_event_complet audio_play_notify_ack_cb;  //��Ӧ�¼����и裬��ͣ���������ţ���ɻص�
    report_excepion_event_state exception_state_cb; //�쳣�¼��ص�
    
};

extern struct app_audio_play_env_t app_audio_play_env;
/*!****************************************************************************
\brief 
    检查是否可以开始新歌播放，
    如果当前可以，立即返�?PLAY_ALLOWABLE ，并忽略hook_cb
    如果当前不可以，返回 PLAY_UNALLOWED，并在可以开始新歌播放时，回调hook_cb
    若有异常 直接返回 PLAY_OTHER ，并忽略hook_cb
\param[in] 
\param[out] 
\retval     
*****************************************************************************/
allow_play_state_t app_bt_check_is_allow_play_audio(check_is_allow_play_audio_hook hook_cb);


/*!****************************************************************************
\brief 
    喂音频数�?
    data_buf 数据存储地址
    data_len 数据长度，单位字�?
    参考app_error.h返回
\param[in] 
\param[out] 
\retval     
*****************************************************************************/
uint32_t app_bt_fill_audio_data(uint8_t *data_buf, uint32_t data_len);


/*!****************************************************************************
\brief 
    注册进喂音频数据后，推流耳机模块更新数据池状态的回调
    参考app_error.h返回
\param[in] 
\param[out] 
\retval     
*****************************************************************************/
uint32_t app_bt_report_audio_fifo_state_register(report_audio_fifo_state fifo_state_cb);


/*!****************************************************************************
\brief 
    查询 推流耳机模块数据池状�?
    参考app_error.h返回,查询失败/不准查询/查询成功
\param[in] 
\param[out] 
\retval     
*****************************************************************************/
uint32_t app_bt_get_audio_fifo_state(audio_fifo_state_t *fifo_state);


/*!****************************************************************************
\brief 
    启动 开始播放新歌流�?
    参考app_error.h返回
\param[in] 
\param[out] 
\retval     
*****************************************************************************/
uint32_t app_bt_event_launch_new_play_audio(void);


/*!****************************************************************************
\brief 
    结束 当前这首歌播放完�?
    sequence_mum  随机序号，推流耳机模块应答此事件时原样送出，用于事�?应答的对应匹�?
    参考app_error.h返回
\param[in] 
\param[out] 
\retval     
*****************************************************************************/
uint32_t app_bt_event_complet_current_play_audio(void);


/*!****************************************************************************
\brief 
    注册进发出当前歌曲数据发送完毕事件后，推流耳机模块报告其已推空数据池即已把当前歌曲推送无线耳机完成的回�?
    sequence_mum  原样送出当前歌曲播放完毕事件送入的序列号，用于事�?应答的对应匹�?
    参考app_error.h返回
\param[in] 
\param[out] 
\retval     
*****************************************************************************/
uint32_t app_bt_ack_event_complet_current_play_audio_register(ack_event_complet_current_play_audio ack_evt_cplt_cb);


/*!****************************************************************************
\brief 
    通知事件准备给到推流无线耳机模块
    play_evt 具体事件类型
    sequence_mum   随即序列号，应答时原样送出，用于对应匹�?
    参考app_error.h返回
\param[in] 
\param[out] 
\retval     
*****************************************************************************/
uint32_t app_bt_event_notif(play_event_t play_evt);

/*!****************************************************************************
\brief 
    注册应答事件通知回调
    ack_notify_cbk 应答的哪种事�?
    sequence_mum   应答时按原样送出，用于对应匹�?
    参考app_error.h返回
\param[in] 
\param[out] 
\retval     
*****************************************************************************/
uint32_t app_bt_ack_event_complet_register(ack_notify_event_complet ack_notify_cbk);


/*!****************************************************************************
\brief 
    推流无线耳机模块报告异常状�?
    参考app_error.h返回
\param[in] 
\param[out] 
\retval     
*****************************************************************************/
uint32_t app_bt_report_exception_event_register(report_excepion_event_state exception_state_cb);

void app_bt_audio_play_init(void);

#endif

