/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdio.h>
#include <string.h>

#include "os_timer.h"
#include "os_mem.h"
#include "driver_uart.h"
#include "driver_flash.h"
#include  "bt_api.h"
#include "user_utils.h"
#include "driver_syscntl.h"
#include "hf_api.h"
#include "a2dp_api.h"
#include "avrcp_api.h"
#include "os_msg_q.h"
#include "os_task.h"
#include "user_task.h"
#include "app_at.h"
#include "user_bt.h"
#include "driver_pmu.h"
#include "driver_button.h"
#include "ipc_load_code.h"
#include "driver_ipc.h"
#include "driver_i2s.h"
#include "user_ipc.h"
#include "spp_api.h"

__attribute__((section("compile_date_sec")))const uint8 compile_date[]  = __DATE__;
__attribute__((section("compile_time_sec")))const uint8 compile_time[]  = __TIME__;

//uint8_t slave_link_conidx;
//uint8_t master_link_conidx;

struct user_bt_env_tag *user_bt_env = (struct user_bt_env_tag *)0x20004c1c;


uint8_t bt_find_device(BD_ADDR *bdAddrP)
{
    bool searchingDevices;
    uint8_t   devNdx;

    devNdx = 0;
    do {
        searchingDevices = FALSE;
        if (NUM_DEVICES > devNdx) {
            if (0 != memcmp(bdAddrP, &user_bt_env->dev[devNdx].bd,6)) {
                searchingDevices = TRUE;
                devNdx += 1;
            }
        }
    } while(TRUE == searchingDevices);

    return(devNdx);

}

uint8_t bt_find_free_dev_index()
{
    uint8_t i;
    for(i=0;i<NUM_DEVICES;i++)
    {
        if(user_bt_env->dev[i].inUse == 0){
            break;
        }
    }
    return i;
}


//#include "app_ipc.h"
void proj_bt_me_evt_func(me_event_t *event)
{
    uint8_t i;
    uint8_t device_index;
    //printf("me func = %d\r\n",event->type);
    switch(event->type){
        case BTEVENT_HCI_INITIALIZED:
            //À¶ÑÀÒÑÁ¬½ÓÇé¿öÏÂ£¬½«À¶ÑÀÉèÖÃ³É²»¿É·¢ÏÖ£¬²»¿ÉÁ¬½Ó×´Ì¬ 
            bt_set_accessible_mode_c(BAM_NOT_ACCESSIBLE,NULL);
    
            //À¶ÑÀÎ´Á¬½ÓÇé¿öÏÂ£¬½«À¶ÑÀÉèÖÃ³É²»¿É·¢ÏÖ£¬¿ÉÁ¬½Ó×´Ì¬
            bt_set_accessible_mode_nc(BAM_NOT_ACCESSIBLE,NULL);
            break;
        case BTEVENT_INQUIRY_CANCELED:
            
            break;  
        case BTEVENT_INQUIRY_COMPLETE:
            //ËÑË÷Íê³Éºó£¬ÖØÐÂËÑË÷¸½½üÉè±¸£¬ËÑË÷Ê±¼ä5*1.28s£¬×î´óËÑË÷½á¹ûÏÞÖÆÎª5¸öÉè±¸
            bt_start_inquiry(5,5);
            break;
        case BTEVENT_INQUIRY_RESULT:
            //ËÑË÷½á¹û´òÓ¡³öÀ´£¬´Ë´¦´òÓ¡µÄÊÇÉè±¸µØÖ·£¬ÆäËû²ÎÊý¿É²Î¿¼½á¹¹ÌåBtInquiryResult 
            printf("me bt inquiry result\r\n");
            for(i=0;i<6;i++)
            {
                printf("0x%02x ",event->param.inqResult->bdAddr.A[i]);
            }
            printf("\r\n");
            break;
        case BTEVENT_LINK_CONNECT_IND:
            device_index = bt_find_free_dev_index();
            if(device_index == NUM_DEVICES){
                 printf("shall not be here,me_callback\r\n");                     
                 //platform_reset(0);
            }
            user_bt_env->dev[device_index].inUse = 1;
            memcpy(&user_bt_env->dev[device_index].bd,event->addr,6);
            user_bt_env->dev[device_index].remDev = event->remDev;
            user_bt_env->dev[device_index].state = BT_CONNECTED;
            break;
        case BTEVENT_LINK_CONNECT_CNF:
            device_index = bt_find_device(event->addr);
            if((device_index == 0)&&(event->errCode == BEC_PAGE_TIMEOUT)){
                bt_reconnect((enum bt_reconnect_type_t)user_bt_env->dev[device_index].reconnecting,event->addr);
            }else if(event->errCode == BEC_NO_ERROR) {
                user_bt_env->dev[device_index].state = BT_CONNECTED;
                user_bt_env->dev[device_index].remDev = event->remDev;
                user_bt_env->dev[device_index].reconnecting = 0;
                bt_set_accessible_mode_nc(BAM_GENERAL_ACCESSIBLE,NULL);
            }
            break;

        case BTEVENT_LINK_DISCONNECT:
            device_index = bt_find_device(event->addr);
            memset(&(user_bt_env->dev[device_index]),0,sizeof(APP_DEVICE));
            break;
        case BTEVENT_ACCESSIBLE_CHANGE:
            if(event->param.aMode == BAM_GENERAL_ACCESSIBLE){
                user_bt_env->access_state = ACCESS_PAIRING;
                 printf("\r\nII\r\n");        
            }else if(event->param.aMode == BAM_CONNECTABLE_ONLY){
                user_bt_env->access_state = ACCESS_STANDBY;
                 printf("\r\nIJ\r\n");        
            }else if(event->param.aMode == BAM_NOT_ACCESSIBLE){
                user_bt_env->access_state = ACCESS_IDLE;
                 printf("\r\nIK\r\n");        
            }
            //system_sleep_enable();
            break;

        case BTEVENT_MODE_CHANGE:
            device_index = bt_find_device(event->addr);
            user_bt_env->dev[device_index].mode = event->param.modeChange.curMode;
            break;
        case BTEVENT_ROLE_CHANGE:

            break;
        default:
            break;
    }

}




__attribute__((section("ram_code"))) void pmu_gpio_isr_ram(void)
{
    uint32_t gpio_value = ool_read32(PMU_REG_PORTA_INPUT_VAL);
    
    button_toggle_detected(gpio_value);
    //printf("gpio new value = 0x%08x.\r\n", gpio_value);
    ool_write32(PMU_REG_STATE_MON_IN_A,gpio_value);
}

void porj_button_evt_func(struct button_msg_t *msg)
{
    printf("index = %x,type = %x, cnt = %x\r\n",msg->button_index,msg->button_type,msg->button_cnt);
    //system_sleep_enable();
}

/*********************************************************************
 * @fn      user_custom_parameters
 *
 * @brief   initialize several parameters, this function will be called 
 *          at the beginning of the program. Note that the parameters 
 *          changed here will overlap changes in uart tools 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void user_custom_parameters(void)
{
    ///print current compile time
    uint8_t i = 0;
    printf("\r\n Complile Date: ");
    for(i = 0; i < 11; i++){
        printf("%c",compile_date[i]);
    }
    printf("\r\n");
    printf("Ccomplile Time: ");
    for(i = 0; i < 8; i++){
        printf("%c",compile_time[i]);
    }
    printf("\r\n");
    pskeys.local_bdaddr.addr[0] = 0x02;
    pskeys.local_bdaddr.addr[1] = 0x02;
    pskeys.local_bdaddr.addr[2] = 0x03;
    pskeys.local_bdaddr.addr[3] = 0x04;
    pskeys.local_bdaddr.addr[4] = 0x05;
    pskeys.local_bdaddr.addr[5] = 0x08;

    memcpy(pskeys.localname,"FR5080_SDK",strlen("FR5080_SDK")+1);

    pskeys.fw_version = 0x00010001;

    pskeys.hf_vol = 0x20;
    pskeys.a2dp_vol = 0x20;
    pskeys.hf_vol = 0x20;

    pskeys.stack_mode = STACK_MODE_BTDM;
    pskeys.enable_profiles = ENABLE_PROFILE_A2DP;  //enable hfp,a2dp profile
    pskeys.system_options = SYSTEM_OPTION_ENABLE_QSPI_QMODE|SYSTEM_OPTION_ENABLE_CACHE
                            |SYSTEM_OPTION_EXT_WAKEUP_ENABLE|SYSTEM_OPTION_SLEEP_ENABLE;
    pskeys.sleep_algo_dur = 3;

    
}

__attribute__((section("ram_code"))) void user_entry_before_sleep_imp(void)
{
    uart_putc_noint('s');
    //uart_putc_noint_no_wait(UART1, 's');
    //ool_write32(PMU_REG_SLP_VAL_0, 0);
}

__attribute__((section("ram_code"))) void user_entry_after_sleep_imp(void)
{
    app_at_init_app();
    uart_putc_noint('w');
    //uart_putc_noint_no_wait(UART0, 'w');
}

void user_entry_before_stack_init(void)
{    
    printf("user_entry_before_stack_init\r\n");
    bt_me_set_cb_func(proj_bt_me_evt_func);

}

void user_entry_after_stack_init(void)
{
    //printf("user_entry_after_ble_init\r\n");
    printf("user_entry_after_stack_init\r\n");
    user_task_init();
    app_at_init_app();
    //simple_peripheral_init();
    system_sleep_disable();

    test_drivers();

}

