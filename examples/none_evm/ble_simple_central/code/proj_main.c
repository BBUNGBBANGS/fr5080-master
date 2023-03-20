/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdio.h>
#include <string.h>

#include "gap_api.h"
#include "gatt_api.h"

#include "os_timer.h"
#include "os_mem.h"
//#include "jump_table.h"
#include "driver_uart.h"
#include "driver_flash.h"
//#include "user_task.h"
//#include "prf_server.h"
//#include "prf_client.h"
#include  "bt_api.h"
//#include "dev_info_service.h"
#include "user_utils.h"
#include "driver_syscntl.h"
//#include "demo_peripheral.h"
#include "hf_api.h"
#include "a2dp_api.h"
#include "avrcp_api.h"
#include "os_msg_q.h"
#include "os_task.h"
#include "user_task.h"
#include "app_at.h"
#include "ble_simple_central.h"
#include "co_log.h"

__attribute__((section("compile_date_sec")))const uint8 compile_date[]  = __DATE__;
__attribute__((section("compile_time_sec")))const uint8 compile_time[]  = __TIME__;

uint8_t slave_link_conidx;
uint8_t master_link_conidx;


void proj_ble_gap_evt_func(gap_event_t *event)
{
    switch(event->type)
    {
        case GAP_EVT_ADV_END:
        {
            LOG_INFO("adv_end,status:0x%02x\r\n",event->param.adv_end_status);
#if 0
            uint8_t adv_data[]="\x09\x08\x46\x52\x38\x30\x31\x30\x48\x00";
            uint8_t rsp_data[]="\x09\xFF\x00\x60\x52\x57\x2D\x42\x4C\x22";

            gap_adv_param_t adv_param;
            adv_param.adv_mode = GAP_ADV_MODE_UNDIRECT;
            adv_param.adv_addr_type = GAP_ADDR_TYPE_PUBLIC;
            adv_param.adv_chnl_map = GAP_ADV_CHAN_ALL;
            adv_param.adv_filt_policy = GAP_ADV_ALLOW_SCAN_ANY_CON_ANY;
            adv_param.adv_intv_min = 32;
            adv_param.adv_intv_max = 32;
            adv_param.adv_data = adv_data;
            adv_param.adv_data_len = sizeof(adv_data)-1;
            adv_param.adv_rsp_data = rsp_data;
            adv_param.adv_rsp_data_len = sizeof(rsp_data)-1;
            gap_set_advertising_param(&adv_param);
#endif
            //gap_start_advertising(0);
        }
        break;
        case GAP_EVT_SCAN_END:
            LOG_INFO("scan_end,status:0x%02x\r\n",event->param.scan_end_status);
            {
                //uint8_t adv_data[31];
                __disable_irq();
                //system_regs->rst.bb_mas_rst = 0;
                //system_regs->rst.bb_mas_rst = 1;

                //adv_send_init(0);
                //adv_send_start(1700, adv_data, 21);
                __enable_irq();
                
                gap_scan_param_t scan_param;
                scan_param.scan_mode = GAP_SCAN_MODE_GEN_DISC;
                scan_param.dup_filt_pol = 0;
                scan_param.scan_intv = 800;  //scan event on-going time
                scan_param.scan_window = 8;
                scan_param.duration = 0;
                gap_start_scan(&scan_param);
            }
            break;
        case GAP_EVT_ADV_REPORT:
        {
            if(memcmp(event->param.adv_rpt->src_addr.addr.addr,"\xfd\x37\xe3\xe1\xfC\x02",6)==0)
            {
                LOG_INFO("evt_type:0x%02x,rssi:%d\r\n",event->param.adv_rpt->evt_type,event->param.adv_rpt->rssi);

                LOG_INFO("content:");
                //show_reg(event->param.adv_rpt->data,event->param.adv_rpt->length,1);
            }

        }
        break;

        case GAP_EVT_ALL_SVC_ADDED:
        {
            LOG_INFO("all svc added\r\n");
            #if LONG_RANGE_TEST_CON
            gap_start_conn((mac_addr_t *)&__jump_table.addr, 1, 32, 32, 0, 500);
            #endif
        }
        break;

        case GAP_EVT_MASTER_CONNECT:
        {
            LOG_INFO("master[%d],connect. link_num:%d\r\n",event->param.master_connect.conidx,gap_get_connect_num());
            master_link_conidx = (event->param.master_connect.conidx);
#if CFG_BLE_BT_POWER_DEMO

            bt_exit_pairing();
            gap_conn_param_update(event->param.master_connect.conidx,8,8,100,500);
            
#endif
#if 0
#if 1
            if (gap_security_get_bond_status())
                gap_security_enc_req(event->param.master_connect.conidx);
            else
                gap_security_pairing_req(event->param.master_connect.conidx);
#else
            extern uint8_t client_id;
            gatt_discovery_all_peer_svc(client_id,event->param.master_encrypt_conidx);
#endif
#endif
        }
        break;

        case GAP_EVT_SLAVE_CONNECT:
        {
            LOG_INFO("slave[%d],connect. link_num:%d\r\n",event->param.slave_connect.conidx,gap_get_connect_num());
            slave_link_conidx = event->param.slave_connect.conidx;
            //gatt_mtu_exchange_req(event->param.slave_connect.conidx);
            
            //gap_conn_param_update(event->param.slave_connect.conidx, 8, 8, 100, 500);
        }
        break;

        case GAP_EVT_DISCONNECT:
        {
            //gap_bond_manager_info_clr("\x0C\x0C\x0C\x0C\x0C\x0B", 0);
            LOG_INFO("Link[%d] disconnect,reason:0x%02X\r\n",event->param.disconnect.conidx
                      ,event->param.disconnect.reason);
#ifdef USER_MEM_API_ENABLE
            show_mem_list();
            //show_msg_list();
            show_ke_malloc();
#endif
            #if LONG_RANGE_TEST_ADV
            gap_start_advertising(0);
            #endif

            
#if CFG_BLE_BT_POWER_DEMO
            bt_enter_pairing();
#endif
            
        }
        break;

        case GAP_EVT_LINK_PARAM_REJECT:
            LOG_INFO("Link[%d]param reject,status:0x%02x\r\n"
                      ,event->param.link_reject.conidx,event->param.link_reject.status);
            break;

        case GAP_EVT_LINK_PARAM_UPDATE:
            LOG_INFO("Link[%d]param update,interval:%d,latency:%d,timeout:%d\r\n",event->param.link_update.conidx
                      ,event->param.link_update.con_interval,event->param.link_update.con_latency,event->param.link_update.sup_to);
            break;

        case GAP_EVT_CONN_END:
            LOG_INFO("conn_end,reason:0x%02x\r\n",event->param.conn_end_reason);
            break;

        case GAP_EVT_PEER_FEATURE:
            LOG_INFO("peer[%d] feats ind\r\n",event->param.peer_feature.conidx);
            //show_reg((uint8_t *)&(event->param.peer_feature.features),8,1);
            break;

        case GAP_EVT_MTU:
            LOG_INFO("mtu update,conidx=%d,mtu=%d\r\n"
                      ,event->param.mtu.conidx,event->param.mtu.value);
            break;
        case GAP_EVT_LINK_RSSI:
            LOG_INFO("link rssi %d\r\n",event->param.link_rssi);
            break;
        case GAP_SEC_EVT_MASTER_AUTH_REQ:
            LOG_INFO("link[%d],recv auth req:0x%02x\r\n",event->param.auth_req.conidx,event->param.auth_req.auth);
            break;
        case GAP_SEC_EVT_MASTER_ENCRYPT:
            LOG_INFO("master[%d]_encrypted\r\n",event->param.master_encrypt_conidx);
            //extern uint8_t client_id;
            //gatt_discovery_all_peer_svc(client_id,event->param.master_encrypt_conidx);

            //uint8_t group_uuid[] = {0xb7, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07};
            //gatt_discovery_peer_svc(client_id,event->param.master_encrypt_conidx,16,group_uuid);
            break;
        case GAP_SEC_EVT_SLAVE_ENCRYPT:
            LOG_INFO("slave[%d]_encrypted\r\n",event->param.slave_encrypt_conidx);
            break;

        default:
            break;
    }
}

/*********************************************************************
 * @fn      user_custom_parameters
 *
 * @brief   initialize several parameters, this function will be called 
 *          at the beginning of the program. 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void user_custom_parameters(void)
{
    uint8_t i = 0;
    LOG_INFO("\r\n Complile Date: ");
    for(i = 0; i < 11; i++){
        LOG_INFO("%c",compile_date[i]);
    }
    LOG_INFO("\r\n");
    LOG_INFO("Ccomplile Time: ");
    for(i = 0; i < 8; i++){
        LOG_INFO("%c",compile_time[i]);
    }
    LOG_INFO("\r\n");
    pskeys.local_bdaddr.addr[0] = 0x01;
    pskeys.local_bdaddr.addr[1] = 0x02;
    pskeys.local_bdaddr.addr[2] = 0x03;
    pskeys.local_bdaddr.addr[3] = 0x04;
    pskeys.local_bdaddr.addr[4] = 0x05;
    pskeys.local_bdaddr.addr[5] = 0x06;

    memcpy(pskeys.localname,"FR5080_M",strlen("FR5080_M")+1);

    pskeys.fw_version = 0x00010001;

    pskeys.hf_vol = 0x20;
    pskeys.a2dp_vol = 0x20;
    pskeys.hf_vol = 0x20;

    pskeys.stack_mode = STACK_MODE_BLE;
    pskeys.system_options = SYSTEM_OPTION_ENABLE_QSPI_QMODE|SYSTEM_OPTION_ENABLE_CACHE
                            |SYSTEM_OPTION_EXT_WAKEUP_ENABLE|SYSTEM_OPTION_SLEEP_ENABLE;
    
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
    LOG_INFO("user_entry_before_stack_init\r\n");
}
__attribute__((section("ram_code"))) void cdc_isr_ram(void)
{}
void user_entry_after_stack_init(void)
{
    LOG_INFO("user_entry_after_stack_init\r\n");
    //*(volatile uint32_t *)0x40000078 |= 0x00200000;

    user_task_init();
    app_at_init_app();
    system_sleep_disable();
	simple_central_init();
	

}

