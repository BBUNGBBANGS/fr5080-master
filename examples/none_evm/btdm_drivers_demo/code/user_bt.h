#ifndef _USER_BT_H
#define _USER_BT_H
#include "type_lib.h"
#include "stdint.h"
#include "stdbool.h"

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
    

enum app_bt_state_t{
    APP_BT_STATE_IDLE,                          //IDLE
    APP_BT_STATE_PAIRING,                       //PAIRING, connectable and discoverable
    APP_BT_STATE_STANDBY,                       //STANDBY, connectable but can not discoverable
    APP_BT_STATE_CONNECTING,                    //CONNECTING, connect is ongoing
    APP_BT_STATE_CONNECTED,                     //CONNECTED, connected,no call and music
    APP_BT_STATE_HFP_INCOMMING,                 //INCOMMING CALL
    APP_BT_STATE_HFP_OUTGOING,                  //OUTGOING CALL
    APP_BT_STATE_HFP_CALLACTIVE,                //CALL IS ACTIVE
    APP_BT_STATE_MEDIA_PLAYING,                 //MUSIC PLAYING
    APP_BT_STATE_MAX,                           
};
    
enum app_bt_link_state_t {
    BT_IDLE = 0x00,     //just after power on
    BT_CONNECTING,
    BT_CONNECTED,
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



enum app_bt_state_t app_bt_get_state(void);
bool bt_reconnect(enum bt_reconnect_type_t type,BD_ADDR *addr);
void app_bt_check_conn(uint8_t dev_num);
bool bt_set_spk_volume(enum bt_volume_type_t type,uint8_t vol);


#endif
