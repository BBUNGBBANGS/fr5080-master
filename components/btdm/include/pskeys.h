#ifndef _PSKEYS_H
#define _PSKEYS_H

#include <stdbool.h>       // standard boolean definitions
#include <stddef.h>        // standard definitions
#include <stdint.h>        // standard integer definitions
//#include "co_bt_defines.h"
#include "type_lib.h"

#define MAX_KEY_NUM             8
#define BT_NAME_MAX_LEN        24
#define BD_ADDR_LEN         6
#define DEV_CLASS_LEN       3


enum{
    TWS_ROLE_MASTER,
    TWS_ROLE_SLAVE,
    TWS_ROLE_NULL,
};
    
enum storage_type_t
{
    STORAGE_TYPE_NONE,
    STORAGE_TYPE_FLASH,
    STORAGE_TYPE_RAM,
    //STORAGE_TYPE_EEPROM,
};

///enable_profiles
#define ENABLE_PROFILE_HF           BIT0
#define ENABLE_PROFILE_A2DP         BIT1    //if a2dp is enabled, avrcp is enabled automatically
#define ENABLE_PROFILE_HID          BIT2
#define ENABLE_PROFILE_PBAP         BIT3
#define ENABLE_PROFILE_SPP          BIT4

///system option
#define SYSTEM_OPTION_SLEEP_ENABLE          BIT0
#define SYSTEM_OPTION_EXT_WAKEUP_ENABLE     BIT1
#define SYSTEM_OPTION_ENABLE_RTOS           BIT2
#define SYSTEM_OPTION_DISABLE_HANDSHAKE     BIT3
#define SYSTEM_OPTION_LANG_SEL_B            BIT4
#define SYSTEM_OPTION_ENABLE_QSPI_QMODE     BIT5
#define SYSTEM_OPTION_ENABLE_CACHE          BIT6
#define SYSTEM_OPTION_ENABLE_HCI_MODE       BIT7


#define APP_BOOT_UART_PC_EN_MAGIC           0x55
#define APP_BOOT_ESCAPE_HS_MAGIC            0xFE

enum rtos_entry_type_t 
{
    RTOS_ENTRY_TYPE_INIT,
    RTOS_ENTRY_TYPE_STACK_PUSH,
    RTOS_ENTRY_TYPE_STACK_YIELD,
    RTOS_ENTRY_TYPE_WAKEUP_RESTORE,
    RTOS_ENTRY_TYPE_BLE_STACK_READY,
    RTOS_ENTRY_TYPE_BT_STACK_READY,
};
    
enum stack_mode_t 
{
    STACK_MODE_BTDM,
    STACK_MODE_BT,
    STACK_MODE_BLE,
};

struct linkkey_table
{
    uint8 valid;
    uint8 key_type;
    uint8 pin_len;
    uint8 prior;
    struct bdaddr_t  bdaddr;
    struct link_key_t key;
};   

struct pskeys_app_t 
{
    uint8  checkword1[4];
    uint8  a2dp_vol;
    uint8  hf_vol;
    uint8  tone_vol;
    uint8  misc_flag; //0---a2dp sink or hf,//1---a2dp source
    struct bdaddr_t local_bdaddr;
    struct bdaddr_t last_device;
    struct bdaddr_t tws_remote_device;
    struct linkkey_table keyTable[MAX_KEY_NUM]; //max support 5 link key
    uint8  checkword2[4];
};

struct reconnect_param_t 
{
    uint8 reconnect_times;
    uint8 reconnect_interval;   //unit 100ms
};

enum patch_mode
{
    PATCH_MODE_XIP,
    PATCH_MODE_RAM,
};

struct pskeys_t 
{
    /// used to identify 5080
    uint8_t reserved[4]; //'5','0','8','0'

    /************************************************************************************/
    ///used internal, change outside shall be careful，
    uint32_t image_base;
    uint32_t image_size;
    uint8_t patch_mode;
    uint32_t fw_version;
    uint32_t stack_top_address;
    uint16_t stack_size;
    uint32_t rwip_heap_base; //  change inside code
    uint32_t rwip_heap_length; //  change inside code
    void * entry;
    uint32_t dsp_code_base;
    uint32_t dsp_code_size;
    uint32_t tone_base;
    uint8_t initial_qspi_bandrate;
    uint8_t tws_mode;          
    /************************************************************************************/
    
    uint8_t localname[BT_NAME_MAX_LEN]; //include '\0'
    struct bdaddr_t local_bdaddr;
    struct class_of_device_t classofdevice;
    uint8_t enable_profiles; // indicate which bt profile is enabled, refer to 'enable_profiles'
    uint32_t system_options; // enable revelated option, refer to 'system option'
    uint8_t stack_mode; // select BT,BLE or BTDM mode,refer to enum stack_mode_t 

    /************************************************************************************/
    ///used internal, don't change outside
    uint32_t slp_max_dur;
    uint8_t sleep_algo_dur;// uint: 312.5us, target sleep time - actual sleep time
    uint16_t twext;         //uint: us
    uint16_t twosc;         //uint: us
    uint16_t lp_cycle_sleep_delay;  //uint: lp cycle, minimum sleep time
    uint8_t sleep_delay_for_os;//uint: ms
    uint8_t handshake_to;
    /************************************************************************************/

    /// default volume,[0,0x3f]
    uint8  a2dp_vol;
    uint8  hf_vol;
    uint8  tone_vol;

    /// used to manage reconnecting and accessiable state changing, optionaly
    struct reconnect_param_t power_on_reconnect;
    struct reconnect_param_t press_button_reconnect;
    struct reconnect_param_t link_loss_reconnect;
    uint8_t pairing_to_standby; //uint: 10s,0---always pairing
    uint8_t standby_to_off;     //uint: 10s,0---always standby,0xff---power off immediately


    /************************************************************************************/
    ///used internal, don't change outside
    uint8_t pmu_mode;//pmu_on_off_mode_t, press,switch or no pmu
    uint8_t pmu_charger_poweron_sel;//1:still when charging, 0:power off when chargin

    uint16_t tone_A_size[26];
    uint16_t tone_B_size[26];

    uint32_t image_max_size;
    uint32_t dsp_max_size;
    /************************************************************************************/
    uint32_t checkword; ///0x51 0x52 0x51 0x52

    ///app data, like link_key,last dev info.
    struct pskeys_app_t app_data;
};

extern  struct pskeys_t pskeys;

///update app_data in flash
void pskeys_update_app_data(void);


#endif
