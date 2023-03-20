/**
 * Copyright (c) 2019, Freqchip
 * 
 * All rights reserved.
 * 
 * 
 */
 
 /*
 * INCLUDES (包含头文件)
 */
#include <stdio.h>
#include <string.h>

#include "os_timer.h"
#include "driver_pmu.h"


#include "gap_api.h"
#include "gatt_api.h"
#include "gatt_sig_uuid.h"
#include "user_utils.h"
#include "co_log.h"

/*
 * MACROS (宏定义)
 */
#define JUMP_TABLE_STATIC_KEY_OFFSET    0x7F000
#define BLE_BONDING_INFO_SAVE_ADDR      0x7D000
#define BLE_REMOTE_SERVICE_SAVE_ADDR    0x7E000

/*
 * CONSTANTS (常量定义)
 */

#define SP_CHAR1_UUID            0xFFF1
#define SP_CHAR2_UUID            0xFFF2

const gatt_uuid_t client_att_tb[] =
{
    [0]  =
    { UUID_SIZE_2, UUID16_ARR(SP_CHAR1_UUID)},
    [1]  =
    { UUID_SIZE_2, UUID16_ARR(SP_CHAR2_UUID)},
};

/*
 * TYPEDEFS (类型定义)
 */

/*
 * GLOBAL VARIABLES (全局变量)
 */
uint8_t client_id;
/*
 * LOCAL VARIABLES (本地变量)
 */
//static os_timer_t button_anti_shake_timer;		//Timer for button anti-sahke
//static uint32_t curr_button_before_anti_shake = 0;		//before anti-sahke timeout, store the pressed button value
 
/*
 * LOCAL FUNCTIONS (本地函数)
 */
static void simple_central_start_scan(void);



/*********************************************************************
 * @fn      app_gap_evt_cb
 *
 * @brief   Application layer GAP event callback function. Handles GAP evnets.
 *
 * @param   p_event - GAP events from BLE stack.
 *       
 *
 * @return  None.
 */
static void app_gap_evt_cb(gap_event_t *p_event)	//GAP callback function, p_evnet is pointer to content of evnet
{
    switch(p_event->type)		//event type from BLE stack
    {
        case GAP_EVT_SCAN_END:	//scan action is end
            LOG_INFO("scan_end,status:0x%02x\r\n",p_event->param.scan_end_status);
//						gap_set_link_rssi_report(false);

            if(gap_get_connect_status(0))
                LOG_INFO("idex = 0 connected\r\n");
            else
                simple_central_start_scan();
            break;
 
        case GAP_EVT_ADV_REPORT:	//got adv report
        {
            uint8_t scan_name[] = "Simple Peripheral";
            uint8_t adv_addr[6] = "\x1\x2\x3\x4\x5\x6";
            if(memcmp(p_event->param.adv_rpt->src_addr.addr.addr,adv_addr,6) == 0){
                LOG_INFO("p_event->param.adv_rpt->data[0] = %x\r\n",p_event->param.adv_rpt->data[0]);
                LOG_INFO("p_event->param.adv_rpt->data[1] = %x\r\n",p_event->param.adv_rpt->data[1]);
                LOG_INFO("name= %s\r\n",&(p_event->param.adv_rpt->data[2]));
                
                LOG_INFO("rpt_length:%d\r\n",p_event->param.adv_rpt->length);

            }
            //if(memcmp(event->param.adv_rpt->src_addr.addr.addr,"\x0C\x0C\x0C\x0C\x0C\x0B",6)==0)
            
            //LOG_INFO("rpt_addr:%x %x %x %x %x %x  \r\n",p_event->param.adv_rpt->src_addr.addr.addr[0],p_event->param.adv_rpt->src_addr.addr.addr[1],p_event->param.adv_rpt->src_addr.addr.addr[2],p_event->param.adv_rpt->src_addr.addr.addr[3],p_event->param.adv_rpt->src_addr.addr.addr[4],p_event->param.adv_rpt->src_addr.addr.addr[5]);
            
            if (p_event->param.adv_rpt->data[0] == 0x12
                && p_event->param.adv_rpt->data[1] == GAP_ADVTYPE_LOCAL_NAME_COMPLETE
                && memcmp(&(p_event->param.adv_rpt->data[2]), scan_name, 0x10) == 0)
            {
                gap_stop_scan();
                
                LOG_INFO("evt_type:0x%02x,rssi:%d\r\n",p_event->param.adv_rpt->evt_type,p_event->param.adv_rpt->rssi);

                LOG_INFO("content:");
                show_reg(p_event->param.adv_rpt->data,p_event->param.adv_rpt->length,1);
                
                gap_start_conn(&(p_event->param.adv_rpt->src_addr.addr),
                                p_event->param.adv_rpt->src_addr.addr_type,
                                12, 12, 0, 300);
            }
        }
        break;

        case GAP_EVT_MASTER_CONNECT: //As master role, link is established.
        {
						// p_event->param.master_connect.conidx is link_id, link_id is assigned by BLE stack,
						// from 0 to 19. In case of multi-connection, link_id represets different links
//            if (gap_security_get_bond_status()) //return if peer device in last link is bond or not
//                gap_security_enc_req(p_event->param.master_connect.conidx); // if bond, then start encryption
//            else
//                gap_security_pairing_req(p_event->param.master_connect.conidx); // if not bond, then start bond
        }
        break;
        case GAP_EVT_SLAVE_CONNECT:
        {
            LOG_INFO("dev[%d],connect. link_num:%d\r\n",p_event->param.slave_connect.conidx,gap_get_connect_num());
        }
        break;

        case GAP_EVT_DISCONNECT:	//link is disconnect evnet
        {
            LOG_INFO("Link[%d] disconnect,reason:0x%02X\r\n",p_event->param.disconnect.conidx
                      ,p_event->param.disconnect.reason);
        }
        break;

        case GAP_EVT_LINK_PARAM_REJECT:
            LOG_INFO("Link[%d]param reject,status:0x%02x\r\n"
                      ,p_event->param.link_reject.conidx,p_event->param.link_reject.status);
            break;

        case GAP_EVT_LINK_PARAM_UPDATE:
            LOG_INFO("Link[%d]param update,interval:%d,latency:%d,timeout:%d\r\n",p_event->param.link_update.conidx
                      ,p_event->param.link_update.con_interval,p_event->param.link_update.con_latency,p_event->param.link_update.sup_to);
            break;

        case GAP_EVT_CONN_END:	//connection action is end
            LOG_INFO("conn_end,reason:0x%02x\r\n",p_event->param.conn_end_reason);
            break;

        case GAP_SEC_EVT_MASTER_ENCRYPT:  //link encryption action is over
				{
						//discovery all service groups on peer device. client_id is profile idx assigned when profile is created
            gatt_discovery_all_peer_svc(client_id,p_event->param.master_encrypt_conidx);	
					
						//if only need to discovery one service group, you can use below function 
            //uint8_t group_uuid[] = {0xb7, 0x5c, 0x49, 0xd2, 0x04, 0xa3, 0x40, 0x71, 0xa0, 0xb5, 0x35, 0x85, 0x3e, 0xb0, 0x83, 0x07};
            //gatt_discovery_peer_svc(client_id,event->param.master_encrypt_conidx,16,group_uuid);
				}
						break;
				
        default:
            break;
    }
}

/*********************************************************************
 * @fn      simple_central_start_scan
 *
 * @brief   Set central role scan parameters and start scanning BLE devices.
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
static void simple_central_start_scan(void)
{
    // Start Scanning
    LOG_INFO("Start scanning...\r\n");
    gap_scan_param_t scan_param;
    scan_param.scan_mode = GAP_SCAN_MODE_GEN_DISC;
    scan_param.dup_filt_pol = 0;
    scan_param.scan_intv = 32;  //scan event on-going time
    scan_param.scan_window = 30;	//scan windows open time
    scan_param.duration = 1000;	//will not stop by self
//		gap_set_link_rssi_report(true);
    gap_start_scan(&scan_param);
}

/*********************************************************************
 * @fn      simple_central_msg_handler
 *
 * @brief   Simple Central GATT message handler, handles messages from GATT layer.
 *          Messages like read/write response, notification/indication values, etc.
 *
 * @param   p_msg       - GATT message structure.
 *
 * @return  uint16_t    - Data length of the GATT message handled.
 */
static uint16_t simple_central_msg_handler(gatt_msg_t *p_msg)		//GATT event callback function, p_msg is pointer to event content
{
    switch(p_msg->msg_evt)	//event type, GATT event type is defined in gatt_msg_evt_t in "gatt_api.h"
    {
        case GATTC_MSG_NTF_REQ:	//receive notification data from peer device
        {
            if(p_msg->att_idx == 0)	//ntf data is for UUID array idx 0 attribution
            {
                show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1); //printf received ntf data
            }
        }
        break;
        
        case GATTC_MSG_READ_IND:	//receive read response data from peer device.
        {
            if(p_msg->att_idx == 0)	//read reponse data is for UUID array idx 0 attribution
            {
                show_reg(p_msg->param.msg.p_msg_data,p_msg->param.msg.msg_len,1);	//printf received read reponse data
            }
        }
        break;
        
        case GATTC_MSG_CMP_EVT:	//A GATTC operation is over
        {
            if(p_msg->param.op.operation == GATT_OP_PEER_SVC_REGISTERED)	//if operation is discovery serivce group on peer device
            {
                uint16_t att_handles[2];
                memcpy(att_handles,p_msg->param.op.arg,4);	//copy discoveried handles on peer device
                show_reg((uint8_t *)att_handles,4,1);	//printf handlers for UUID array idx. if 0 means handler nnumber is not not found

                gatt_client_enable_ntf_t ntf_enable;	//below code is to inform peer device that notification is enabled
                ntf_enable.conidx = p_msg->conn_idx;	//GATT event from which link_id
                ntf_enable.client_id = client_id;			//profile id assigned by BLE stack when profile is created
                ntf_enable.att_idx = 0; 							//This operation is for UUID array idx 0 attribution
                gatt_client_enable_ntf(ntf_enable);		//Start enable_ntf operation 

                gatt_client_write_t write;
                write.conidx = p_msg->conn_idx;				//GATT event from which link_id
                write.client_id = client_id;					//profile id assigned by BLE stack when profile is created
                write.att_idx = 1;										//This operation is for UUID array idx 1 attribution
                write.p_data = "\x1\x2\x3\x4\x5\x6\x7";	//Buffer to be written
                write.data_len = 7;										//buffer len
                gatt_client_write_cmd(write);					//start write opertaion

                gatt_client_read_t read;
                read.conidx = p_msg->conn_idx;				//GATT event from which link_id
                read.client_id = client_id;						//profile id assigned by BLE stack when profile is created
                read.att_idx = 0; 										//This operation is for UUID array idx 0 attribution	
                gatt_client_read(read);								//start read opertaion
            }
        }
        break;
        
        default:
        break;
    }

    return 0;
}

/*
 * EXTERN FUNCTIONS (外部函数)
 */

/*
 * PUBLIC FUNCTIONS (全局函数)
 */

/** @function group ble peripheral device APIs (ble外设相关的API)
 * @{
 */

/*********************************************************************
 * @fn      simple_central_init
 *
 * @brief   Initialize simple central, BLE related parameters.
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void simple_central_init(void)
{	
    // set local device name
    uint8_t local_name[] = "Simple Central";
    gap_set_dev_name(local_name, sizeof(local_name));
    LOG_INFO("simple_central_init\r\n");
    gap_set_cb_func(app_gap_evt_cb);
    
    // Initialize security related settings.
    gap_bond_manager_init(BLE_BONDING_INFO_SAVE_ADDR, BLE_REMOTE_SERVICE_SAVE_ADDR, 8, true);

    gap_security_param_t param =
    {
        .mitm = false,
        .ble_secure_conn = false,
        .io_cap = GAP_IO_CAP_NO_INPUT_NO_OUTPUT,
        .pair_init_mode = GAP_PAIRING_MODE_WAIT_FOR_REQ,
        .bond_auth = true,
        .password = 0,
    };

    gap_security_param_init(&param);
    
    // Initialize GATT 
    gatt_client_t client;
    
    client.p_att_tb = client_att_tb;
    client.att_nb = 2;
    client.gatt_msg_handler = simple_central_msg_handler;
    client_id = gatt_add_client(&client);
    
    simple_central_start_scan();
}




