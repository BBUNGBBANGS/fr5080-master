#include <stdint.h>

#include "os_task.h"
#include "os_msg_q.h"
#include "os_mem.h"

#include "driver_ipc.h"
#include "driver_uart.h"

#include "user_dsp.h"
#include "user_task.h"
#include "user_bt.h"
#include "app_user_bt.h"
#include "audio_source.h"
#include "native_playback.h"
#include "msbc_playback.h"
#include "app_at.h"
#include "ll.h"
#include "comm.h"

uint16_t user_task_id;

void ipc_load_code_trans_following(void *arg);

///用户消息处理函数
static int user_task_func(os_event_t *param)
{
    switch(param->event_id)
    {
        ///事件类型为串口AT命令
        case USER_EVT_AT_COMMAND:
            ///消息处理
            app_at_cmd_recv_handler(param->param, param->param_len);
#if COPROCESSOR_UART_ENABLE
            uart_lp_env.rx_pending = false;
            if((uart_lp_env.tx_sending == false)&&(uart_lp_env.pending_num > 0)){
                uart_lp_env.tx_sending = true;
                
                //uart_putc_noint('X');
                uart_req_tx();
            }
#endif
            break;
        case USER_EVT_REQ_RAW_DATA:
            {
                uint16_t length = param->param_len;
                audio_source_statemachine(USER_EVT_DECODER_NEED_DATA, &length);
                native_playback_statemachine(USER_EVT_DECODER_NEED_DATA, &length);
                msbc_playback_statemachine(USER_EVT_DECODER_NEED_DATA, &length);
            }
            break;
        case USER_EVT_AUDIO_PLAY_I2S_TRIG:
            //uart_putc_noint('R');
            ipc_msg_send((enum ipc_msg_type_t)IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_NEED_MORE_SBC, 0);
            audio_source_i2s_trig_handler();
            break;
        case USER_EVT_SEND_FOLLOWING_DSP_CODE:
            ipc_load_code_trans_following(NULL);
            break;
        case USER_EVT_DSP_OPENED:
            audio_source_statemachine(USER_EVT_DSP_OPENED, NULL);
            native_playback_statemachine(USER_EVT_DSP_OPENED, NULL);
            msbc_playback_statemachine(USER_EVT_DSP_OPENED, NULL);
            bt_statemachine(USER_EVT_DSP_OPENED, NULL);
            break;
        case USER_EVT_DECODER_PREPARE_NEXT_DONE:
            audio_source_statemachine(USER_EVT_DECODER_PREPARE_NEXT_DONE, NULL);
            native_playback_statemachine(USER_EVT_DECODER_PREPARE_NEXT_DONE, NULL);
            msbc_playback_statemachine(USER_EVT_DECODER_PREPARE_NEXT_DONE, NULL);
            break;
        case USER_EVT_RAW_DATA_TO_DSP_SENT:
            audio_source_statemachine(USER_EVT_RAW_DATA_TO_DSP_SENT, NULL);
            native_playback_statemachine(USER_EVT_RAW_DATA_TO_DSP_SENT, NULL);
            break;
#if COPROCESSOR_UART_ENABLE
        case USER_EVT_UART_TX_READY:
            struct uart_msg_elem_t *elem = NULL;
            os_event_t uart_tx_cmp_event;
            uint8_t str[9] = {'Z','Z','#'};
            
            GLOBAL_INT_DISABLE();
            elem = (struct uart_msg_elem_t *)co_list_pop_front(&uart_lp_env.msg_list);
            if(uart_lp_env.pending_num > 0){
                uart_lp_env.pending_num --;
            }
            GLOBAL_INT_RESTORE();
            if(elem){
                //sprintf((char *)(str),"ZZ#%x",elem->data_len);
                str[3] = hex4bit_to_char((elem->data_len>>12)&0x0f);
                str[4] = hex4bit_to_char((elem->data_len>>8)&0x0f);                
                str[5] = hex4bit_to_char((elem->data_len>>4)&0x0f);
                str[6] = hex4bit_to_char(elem->data_len&0x0f);
                //str[3] = (elem->data_len>>8)&0xff;
                //str[4] elem->data_len&0xff
                //printf("%s",str);
                uart_write(str,7);
                
                uart_write(elem->data,elem->data_len);
                os_free((void *)elem->data);
                os_free((void *)elem);
                
                uart_tx_cmp_event.event_id = USER_EVT_UART_TX_CMP;
                uart_tx_cmp_event.param = NULL;        
                uart_tx_cmp_event.param_len = 0;     
                os_msg_post(user_task_id, &uart_tx_cmp_event);   
                uart_lp_env.tx_sending = false;
            }
            break;
        case USER_EVT_UART_TX_CMP:
            if(uart_lp_env.rx_pending == true){
                uart_send_ack();
            }else{
                if((uart_lp_env.pending_num > 0)&&(uart_lp_env.tx_sending == false)){
                    uart_lp_env.tx_sending = true;
                    //uart_putc_noint('Y');
                    uart_req_tx();
                }
            }
            break;

#endif
         

#if FR5088_COMM_ENABLE
        
        case USER_EVT_5088_COMM:
        {
            comm_transmit_poll();
        }break;
#endif    // COPROCESSOR_UART_ENABLE
    }

    ///返回EVT_CONSUMED，LIB对该消息进行释放
    return EVT_CONSUMED;
}

void user_task_init(void)
{
    ///创建用户任务
    user_task_id = os_task_create(user_task_func);
}

