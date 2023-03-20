#include "bt_api.h"
#include "pskeys.h"
#include "user_bt.h"
#include "driver_plf.h"
#include "a2dp_api.h"
#include "hf_api.h"
#include "avrcp_api.h"
#include <string.h>

extern struct user_bt_env_tag *user_bt_env;

BtStatus reconnect_according_to_profile(BD_ADDR *bdaddr)
{
    BtStatus ret = BT_STATUS_SUCCESS;

    bt_set_accessible_mode_nc(BAM_NOT_ACCESSIBLE, NULL);

    if(pskeys.enable_profiles & ENABLE_PROFILE_HF) {
        ret = hf_connect(bdaddr);
    }else if(pskeys.enable_profiles & ENABLE_PROFILE_A2DP) {
        ret = a2dp_open_stream(AVDTP_STRM_ENDPOINT_SRC, bdaddr);
    }

    return ret;
}


enum app_bt_state_t app_bt_get_state(void)
{
    enum app_bt_state_t ret_state = APP_BT_STATE_IDLE;
    if((user_bt_env->dev[0].state == BT_CONNECTED)||(user_bt_env->dev[1].state == BT_CONNECTED)){
        ret_state = APP_BT_STATE_CONNECTED;
        if(user_bt_env->dev[user_bt_env->last_active_dev_index].active == HF_CALL_ACTIVE){
            //multi party call
            ret_state = APP_BT_STATE_HFP_CALLACTIVE;
            #if 0
            if((user_bt_env->dev[user_bt_env->last_active_dev_index].call_held == HF_CALL_HELD_ACTIVE)||(user_bt_env->dev[user_bt_env->last_active_dev_index].call_held == HF_CALL_HELD_NO_ACTIVE)){
                ret_state = APP_STATE_CAMULTY;
            }            
        
            if(user_bt_env->dev[user_bt_env->last_active_dev_index].setup_state == HF_CALL_SETUP_IN){
                ret_state = APP_STATE_HFP_CAIMG;
            }
            else if((user_bt_env->dev[user_bt_env->last_active_dev_index].setup_state == HF_CALL_SETUP_OUT) 
                ||(user_bt_env->dev[user_bt_env->last_active_dev_index].setup_state == HF_CALL_SETUP_ALERT)){
                ret_state = APP_STATE_HFP_CAOGG;
            }
            #endif
        }else{
            if(user_bt_env->dev[user_bt_env->last_active_dev_index].setup_state == HF_CALL_SETUP_IN){
                ret_state = APP_BT_STATE_HFP_INCOMMING;
            }
            else if((user_bt_env->dev[user_bt_env->last_active_dev_index].setup_state == HF_CALL_SETUP_OUT) 
                ||(user_bt_env->dev[user_bt_env->last_active_dev_index].setup_state == HF_CALL_SETUP_ALERT)){
                ret_state = APP_BT_STATE_HFP_OUTGOING;
            }else if(user_bt_env->dev[user_bt_env->last_active_dev_index].conFlags&LINK_STATUS_MEDIA_PLAYING){
                ret_state = APP_BT_STATE_MEDIA_PLAYING;
            }
        }
    }
    else if((user_bt_env->dev[0].state == BT_CONNECTING)||(user_bt_env->dev[1].state == BT_CONNECTING)){
        ret_state = APP_BT_STATE_CONNECTING;
    }
    else{
        if(user_bt_env->access_state == ACCESS_PAIRING){
            ret_state = APP_BT_STATE_PAIRING;
        }
        else if(user_bt_env->access_state == ACCESS_STANDBY){
            ret_state = APP_BT_STATE_STANDBY;
        }
        else{
            ret_state = APP_BT_STATE_IDLE;
        }
    }
    return ret_state;

}

bool bt_reconnect(enum bt_reconnect_type_t type,BD_ADDR *addr)
{
    bool ret = false;
    BD_ADDR bdaddr;
    uint8_t get_proper_dev_flag = false;
    printf("reconnect... \r\n");
    if(type == RECONNECT_TYPE_POWER_ON){
        if(bt_get_last_device(&bdaddr) == BT_STATUS_SUCCESS){
            //last dev addr exists
            get_proper_dev_flag = true;
        }
    }
    else if(type == RECONNECT_TYPE_USER_CMD){
        if(addr == NULL){
            if(bt_get_last_device(&bdaddr) == BT_STATUS_SUCCESS){
                //last dev addr exists
                get_proper_dev_flag = true;
            }
        }else{
            get_proper_dev_flag = true;
            memcpy(bdaddr.A,addr->A,BD_ADDR_SIZE);
        }
    }
    else if(type == RECONNECT_TYPE_LINK_LOSS){
        memcpy(bdaddr.A, user_bt_env->dev[0].bd.A, BD_ADDR_SIZE);
        get_proper_dev_flag = true;
    }

    if(get_proper_dev_flag == false)
    {
        printf("no proper dev\r\n");
        user_bt_env->dev[0].state = BT_IDLE;
    }else{
        if(user_bt_env->dev[0].reconnecting == 0){                     
            user_bt_env->dev[0].state = BT_CONNECTING;
            user_bt_env->dev[0].inUse = 1;
            user_bt_env->dev[0].reconnecting = type;
            if(type == RECONNECT_TYPE_POWER_ON){
                user_bt_env->dev[0].reconnect_times = pskeys.power_on_reconnect.reconnect_times;
                memcpy( user_bt_env->dev[0].bd.A, bdaddr.A, BD_ADDR_SIZE);   
            }else if(type == RECONNECT_TYPE_LINK_LOSS){
                user_bt_env->dev[0].reconnect_times = pskeys.link_loss_reconnect.reconnect_times;
                
            }else if(type == RECONNECT_TYPE_USER_CMD){
                user_bt_env->dev[0].reconnect_times = pskeys.press_button_reconnect.reconnect_times;
                
                memcpy( user_bt_env->dev[0].bd.A, bdaddr.A, BD_ADDR_SIZE);   
            }
        }

        if(user_bt_env->dev[0].reconnect_times > 0){
            if(user_bt_env->dev[0].reconnect_times != 0xff)
                user_bt_env->dev[0].reconnect_times --;
            if(reconnect_according_to_profile(&bdaddr) == BT_STATUS_FAILED){   
                memset(&(user_bt_env->dev[0]),0,sizeof(user_bt_env->dev[0]));
            }else{
                ret = true;
            }
        }else{
            printf("reconnect time exceeds\r\n");
            memset(&(user_bt_env->dev[0]),0,sizeof(APP_DEVICE));
        }
    }
    return ret;
}

void bt_disconnect(void)
{
    uint8_t i;

    for(i=0;i<NUM_DEVICES;i++){
        if(user_bt_env->dev[i].state == BT_CONNECTED){
            if(user_bt_env->dev[i].conFlags&LINK_STATUS_SCO_CONNECTED){
                hf_disconnect_audio_link(user_bt_env->dev[i].hf_chan);
            }
            if(user_bt_env->dev[i].conFlags&LINK_STATUS_HF_CONNECTED){ 
                hf_disconnect(user_bt_env->dev[i].hf_chan);
            }
            if(user_bt_env->dev[i].conFlags&LINK_STATUS_AVC_CONNECTED){
                
                avrcp_disconnect(user_bt_env->dev[i].rcp_chan);
            }
            if(user_bt_env->dev[i].conFlags&LINK_STATUS_AV_CONNECTED){  
                
                a2dp_close_stream(user_bt_env->dev[i].pstream);
            }
        }
    }

}

bool bt_set_spk_volume(enum bt_volume_type_t type,uint8_t vol)
{
    uint8_t avrcp_vol,hf_vol;
    //printf("set spk vol = %d,%x,%x\r\n",vol,user_bt_env->dev[0].hf_chan,user_bt_env->dev[1].hf_chan);
    if((type >= BT_VOL_MAX)||(vol > 0x3f)){
        return false;
    }
    if(type == BT_VOL_HFP){
        pskeys.app_data.hf_vol = vol;
        if(user_bt_env->dev[0].conFlags&LINK_STATUS_SCO_CONNECTED){
            if(vol == 0){
                hf_vol = 0;
            }else{
                hf_vol = 1+ (15*vol)/64;
            }
            hf_report_speaker_volume(user_bt_env->dev[0].hf_chan, hf_vol);
            
            REG_PL_WR(0x50022000+(0xd6<<2),vol);
            REG_PL_WR(0x50022000+(0xd7<<2),vol);
        }
    }
    else if(type == BT_VOL_MEDIA){
        pskeys.app_data.a2dp_vol = vol;
        if(user_bt_env->dev[0].conFlags&LINK_STATUS_MEDIA_PLAYING){
            if(vol == 0x3f){
                avrcp_vol = 127;
            }else{
                avrcp_vol = (127*vol)/64;
            }
            avrcp_tg_set_absolute_volume(user_bt_env->dev[0].rcp_chan,avrcp_vol);
            REG_PL_WR(0x50022000+(0xd6<<2),vol);
            REG_PL_WR(0x50022000+(0xd7<<2),vol);
        }
    }
    else if(type == BT_VOL_TONE){
        pskeys.app_data.tone_vol = vol;
    }
    
    pskeys_update_app_data();
    return true;
}


void app_bt_check_conn(uint8_t dev_num)
{
    uint8_t conn_flag = 0;
    
    //start timer to make sure all connections connected
    if(pskeys.enable_profiles&ENABLE_PROFILE_HF){
        conn_flag |=  LINK_STATUS_HF_CONNECTED;
    }
    if(pskeys.enable_profiles&ENABLE_PROFILE_A2DP){
        conn_flag |=  LINK_STATUS_AV_CONNECTED | LINK_STATUS_AVC_CONNECTED;
    }

    //co_printf("conFlags=%x,conn_flag=%x,and=%x\r\n",app_env.dev[dev_num].conFlags,conn_flag,conn_flag & (app_env.dev[dev_num].conFlags));
    if((conn_flag & (user_bt_env->dev[dev_num].conFlags)) != conn_flag){
        //restart timer
    }
    else{
        /*
        if(dev_num == 0){
            timer_id = APP_EVENT_BT_DEV0_ACT2SNIFF_TO;
        }else if(dev_num == 1){
            timer_id = APP_EVENT_BT_DEV1_ACT2SNIFF_TO;
        }
        app_env.dev[dev_num].mode = BLM_ACTIVE_MODE;
        if(((pskeys.dev_info.localfeatures0&SNIFF_FEATURE)==SNIFF_FEATURE)&&(pskeys.active_to_sniff !=0xff)){
            ke_timer_set(timer_id,TASK_APP,pskeys.active_to_sniff*100);
        }
        app_bt_env.dev[dev_num].tone_dis_flag = true;
        audio_codec_play_single_tone(APP_TONE_CONNECTED,AUDIO_CODEC_TONE_PRIO_MEDIAL);
        DDB_SaveLastDevice(&app_env.dev[dev_num].bd);
        */
        
        //pskeys_update_app();
        bt_save_last_device(&user_bt_env->dev[dev_num].bd);
        //system_sleep_enable();

        printf("\r\nIV\r\n");
    }
}

