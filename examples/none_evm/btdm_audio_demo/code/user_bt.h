#ifndef _USER_BT_H
#define _USER_BT_H


#include <stdint.h>
#include <stdbool.h>

#include "bt_api.h"
#include "a2dp_api.h"
#include "hf_api.h"
#include "avrcp_api.h"
#include "spp_api.h"
#include "pbap_api.h"
#include "type_lib.h"
#include "bt_hid_api.h"

#define USER_EVT_NOTIFY_MP3_INFO    BIT0
#define USER_EVT_NOTIFY_MP3_PROG    BIT1
#define USER_EVT_NOTIFY_MEDIA_INFO  BIT2
#define USER_EVT_NOTIFY_VOLUME      BIT3


enum bt_reconnect_type_t{
    RECONNECT_TYPE_POWER_ON= 0x01,      //开机回连
    RECONNECT_TYPE_LINK_LOSS,           //超距断开回连
    RECONNECT_TYPE_USER_CMD,            //用户自定义回连
    RECONNECT_TYPE_MAX,
};

enum bt_volume_type_t{
    BT_VOL_HFP,                         //设置通话音量
    BT_VOL_MEDIA,                       //设置媒体音量
    BT_VOL_TONE,                        //设置提示音量
    BT_VOL_MAX,
};
    

enum bt_state_t{
    BT_STATE_IDLE,                          //IDLE
    BT_STATE_PAIRING,                       //PAIRING, connectable and discoverable
    BT_STATE_STANDBY,                       //STANDBY, connectable but can not discoverable
    BT_STATE_CONNECTING,                    //CONNECTING, connect is ongoing
    BT_STATE_CONNECTED,                     //CONNECTED, connected,no call and music
    BT_STATE_HFP_INCOMMING,                 //INCOMMING CALL
    BT_STATE_HFP_OUTGOING,                  //OUTGOING CALL
    BT_STATE_HFP_CALLACTIVE,                //CALL IS ACTIVE
    BT_STATE_MEDIA_PLAYING,                 //MUSIC PLAYING
    BT_STATE_DISCONNECTING,
    BT_STATE_MAX,                           
};
    
enum app_bt_link_state_t {
    BT_IDLE = 0x00,     //just after power on
    BT_CONNECTING,      //5080 reconnecting
    BT_CONNECTED,
    BT_CONNECT_PENDING, //phone is connecting to 5080
    BT_DISCONNECTING,   //doing disconnect
};

enum app_bt_access_state_t{
    ACCESS_IDLE = 0x00,
    ACCESS_STANDBY,
    ACCESS_PAIRING,
};


//status
#define LINK_STATUS_HF_CONNECTED    (1<<0)
#define LINK_STATUS_AV_CONNECTED    (1<<1)
#define LINK_STATUS_AVC_CONNECTED   (1<<2)
#define LINK_STATUS_MEDIA_PLAYING   (1<<3)
#define LINK_STATUS_SCO_CONNECTED   (1<<4)  //ScoMapHciToConnect(BtRemoteDevice.hciHandle) can also used to detect whether SCO exists or not
#define LINK_STATUS_HID_CONNECTED   (1<<5)
#define LINK_STATUS_PBAP_CONNECTED  (1<<6)
#define LINK_STATUS_SPP_CONNECTED   (1<<7)

enum bt_state_t bt_get_state(void);
extern struct user_bt_env_tag *user_bt_env;

bool bt_reconnect(enum bt_reconnect_type_t type,BD_ADDR *addr);
void bt_check_conn(uint8_t dev_num);
bool bt_set_spk_volume(enum bt_volume_type_t type,uint8_t vol);
void bt_hf_evt_func(hf_event_t *event);
void bt_a2dp_evt_func(a2dp_event_t *event);
void bt_me_evt_func(me_event_t *event);
void bt_avrcp_evt_func(avrcp_event_t *event);
void bt_spp_evt_func(spp_event_t *event);
void bt_disconnect(void);
void PbaClientCallback(PbapClientCallbackParms_new *Parms);
void bt_hid_evt_func(void *chan, hid_event_t *event);

/**
 * @brief get current BT status
 * 
 * @return uint8_t BLM_ACTIVE_MODE or BLM_SNIFF_MODE
 */
uint8_t bt_get_link_mode(void);

bool bt_statemachine(uint8_t event, void *arg);

void bt_set_user_evt_notify_enable(uint8_t flag);

#endif

