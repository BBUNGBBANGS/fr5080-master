#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "os_msg_q.h"
#include "os_mem.h"
#include "os_timer.h"

#include "user_task.h"

#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_plf.h"
#include "bt_api.h"
#include "hf_api.h"
#include "avrcp_api.h"
#include "a2dp_api.h"
#include "user_bt.h"
#include "user_utils.h"
#include "driver_pmu.h"

#define AT_RECV_MAX_LEN             32

static uint8_t app_at_recv_char;
static uint8_t at_recv_buffer[AT_RECV_MAX_LEN];
static uint8_t at_recv_index = 0;
static uint8_t at_recv_state = 0;

extern struct user_bt_env_tag *user_bt_env;

static void app_at_recv_cmd_A(uint8_t sub_cmd, uint8_t *data)
{
    //struct bd_addr addr;
    //uint8_t tmp_data = 0;

    switch(sub_cmd)
    {
        case 'G':
            printf("hello world!!!!\r\n");
            break;
        case 'U':
            {
                uint32_t *ptr = (uint32_t *)(ascii_strn2val((const char *)&data[0], 16, 8) & (~3));
                uint8_t count = ascii_strn2val((const char *)&data[9], 16, 2);
                uint32_t *start = (uint32_t *)((uint32_t)ptr & (~0x0f));
                printf("ptr is 0x%08x, count = %d.\r\n", ptr, count);
                for(uint8_t i=0; i<count;) {
                    if(((uint32_t)start & 0x0c) == 0) {
                        printf("0x%08x: ", start);
                    }
                    if(start < ptr) {
                        printf("        ");
                    }
                    else {
                        i++;
                        printf("%08x", *start);
                    }
                    if(((uint32_t)start & 0x0c) == 0x0c) {
                        printf("\r\n");
                    }
                    else {
                        printf(" ");
                    }
                    start++;
                }
            }
            break;
    }
}

///used for test hf,a2dp,avrcp,me api, TBD by marvin
static void app_at_recv_cmd_B(uint8_t sub_cmd, uint8_t *data)
{
    BtSniffInfo sniff_info; 
    BD_ADDR addr;

    switch(sub_cmd)
    {
        case 'A':
            bt_start_inquiry(5,5);
            printf("OK\r\n");
        break;
        case 'B':
            bt_cancel_inquiry();
            printf("OK\r\n");
        break;

        case 'C':
            bt_set_accessible_mode_nc(BAM_GENERAL_ACCESSIBLE,NULL);
            printf("OK\r\n");
        break;
    
        case 'D':
            bt_set_accessible_mode_nc(BAM_NOT_ACCESSIBLE,NULL);
            printf("OK\r\n");
        break;
        
        case 'E':
            sniff_info.maxInterval = 0x320;
            sniff_info.minInterval = 0x320;
            sniff_info.attempt = 0x01;
            sniff_info.timeout = 0x00;
            bt_start_sniff(user_bt_env->dev[0].remDev,(const BtSniffInfo *)&sniff_info);
            printf("OK\r\n");
        break;
        
        case 'F':
            printf("current mode = %d\r\n",bt_get_current_mode(user_bt_env->dev[0].remDev));
        break;
        
        case 'G':
            bt_reconnect(RECONNECT_TYPE_USER_CMD,NULL);
            printf("OK\r\n");
        break;

        case 'H':
            bt_set_spk_volume(BT_VOL_MEDIA,ascii_strn2val((const char*)&data[0],16,2));
            printf("OK\r\n");
        break;

        case 'I':
            bt_set_spk_volume(BT_VOL_HFP,ascii_strn2val((const char*)&data[0],16,2));
            printf("OK\r\n");
        break;
        
        case 'J':
            bt_disconnect();
            printf("OK\r\n");
        break;

        case 'K':
            hf_redial(user_bt_env->dev[0].hf_chan);
            printf("OK\r\n");
        break;
        case 'L':
            //hf_hang_up(user_bt_env->dev[0].hf_chan);
            //printf("OK\r\n");
            if(data[2] == '_'){
                ool_write(co_ascii_strn2val((const char*)&data[0],16,2),co_ascii_strn2val((const char*)&data[3],16,2));
                printf("\r\nOK\r\n");

            }else{
                printf("\r\n0x%x\r\n",ool_read(co_ascii_strn2val((const char*)&data[0],16,2)));
            }
            
        break;
        case 'M':
            if(data[8] == '\r') {
            //read uint32
                printf("\r\n0x%x\r\n",REG_PL_RD(co_ascii_strn2val((const char*)&data[0],16,8)));
                return;
            }else if(data[8] == '_') {
                REG_PL_WR(co_ascii_strn2val((const char*)&data[0],16,8),co_ascii_strn2val((const char*)&data[9],16,8));
                printf("\r\nOK\r\n");
            }
        break;

        case 'N':
            #if 0
            addr.A[0] = 0xA7;
            addr.A[1] = 0x50;
            addr.A[2] = 0x58;
            addr.A[3] = 0x16;
            addr.A[4] = 0x52;
            addr.A[5] = 0x1c;
            #else
            addr.A[0] = 0x23;
            addr.A[1] = 0xd2;
            addr.A[2] = 0x57;
            addr.A[3] = 0x16;
            addr.A[4] = 0x52;
            addr.A[5] = 0x1c;

            #endif
            memcpy(&user_bt_env->dev[0].bd,&addr,6);
            a2dp_open_stream(AVDTP_STRM_ENDPOINT_SNK, &addr);
            printf("OK\r\n");
        break;

        case 'O':
            bt_set_a2dp_type(bt_get_a2dp_type()^0x01);
            printf("OK\r\n");
        break;

        case 'P':
            a2dp_start_stream(user_bt_env->dev[0].pstream);
            printf("OK\r\n");
        break;
        
        case 'Q':
            a2dp_suspend_stream(user_bt_env->dev[0].pstream);
            printf("OK\r\n");
        break;

        case 'R':
            hf_enable_voice_recognition(user_bt_env->dev[0].hf_chan, TRUE);
            printf("OK\r\n");
        break;
        case 'S':
            hf_enable_voice_recognition(user_bt_env->dev[0].hf_chan, FALSE);
            printf("OK\r\n");
        break;
        case 'T':
            avrcp_set_panel_key(user_bt_env->dev[0].rcp_chan, AVRCP_POP_PLAY,TRUE);
            printf("OK\r\n");
        break;
        case 'U':
            avrcp_set_panel_key(user_bt_env->dev[0].rcp_chan, AVRCP_POP_PAUSE,TRUE);
            printf("OK\r\n");
        break;
        case 'V':
            avrcp_ct_get_media_info(user_bt_env->dev[0].rcp_chan, 0x41);
            printf("OK\r\n");
        break;
        case 'W':
            uint8_t *data_spp = os_malloc(10);
            memcpy(data_spp,"123456789a",10);
            spp_send_data(data_spp, 10);
            printf("OK\r\n");
        break;
        default:
            
        break;
    }
}

static void app_at_recv_cmd_C(uint8_t sub_cmd, uint8_t *data)
{
    BtSniffInfo sniff_info; 
    BD_ADDR addr;

    switch(sub_cmd)
    {
        case 'A':
            hf_answer_call(user_bt_env->dev[0].hf_chan);
            printf("OK\r\n");
        break;
        case 'B':
            hf_hang_up(user_bt_env->dev[0].hf_chan);
            printf("OK\r\n");
        break;

        case 'C':
            hf_redial(user_bt_env->dev[0].hf_chan);
            printf("OK\r\n");
        break;
    
        case 'D':
            avrcp_set_panel_key(user_bt_env->dev[0].rcp_chan, AVRCP_POP_PAUSE,TRUE);
            printf("OK\r\n");
        break;
        
        case 'E':
            avrcp_set_panel_key(user_bt_env->dev[0].rcp_chan, AVRCP_POP_PLAY,TRUE);
            printf("OK\r\n");
        break;
        
        case 'F':
            avrcp_set_panel_key(user_bt_env->dev[0].rcp_chan, AVRCP_POP_FORWARD,TRUE);
            printf("OK\r\n");
        break;
        
        case 'G':
            avrcp_set_panel_key(user_bt_env->dev[0].rcp_chan, AVRCP_POP_BACKWARD,TRUE);
            printf("OK\r\n");
        break;
  
        default:
        break;
    }
}

void app_at_cmd_recv_handler(uint8_t *data, uint16_t length)
{
    switch(data[0])
    {
        case 'A':
            app_at_recv_cmd_A(data[1], &data[2]);
            break;
        case 'B':
            app_at_recv_cmd_B(data[1], &data[2]);
        case 'C':
            app_at_recv_cmd_C(data[1], &data[2]);
        default:
            break;
    }
}

#define __RAM_CODE          __attribute__((section("ram_code")))
__RAM_CODE static void app_at_recv_c(uint8_t c)
{
    switch(at_recv_state)
    {
        case 0:
            if(c == 'A')
            {
                at_recv_state++;
            }
            break;
        case 1:
            if(c == 'T')
                at_recv_state++;
            else
                at_recv_state = 0;
            break;
        case 2:
            if(c == '#')
                at_recv_state++;
            else
                at_recv_state = 0;
            break;
        case 3:
            at_recv_buffer[at_recv_index++] = c;
            if((c == '\n')
               ||(at_recv_index >= AT_RECV_MAX_LEN))
            {
                os_event_t at_cmd_event;
                at_cmd_event.event_id = USER_EVT_AT_COMMAND;///event id
                at_cmd_event.param = at_recv_buffer;        ///串口接收buffer
                at_cmd_event.param_len = at_recv_index;     ///串口接收buffer长度
                os_msg_post(user_task_id, &at_cmd_event);   ///发送消息

                at_recv_state = 0;
                at_recv_index = 0;
            }
            break;
    }
}

__attribute__((section("ram_code"))) void app_at_uart_recv(void*dummy, uint8_t status)
{
    app_at_recv_c(app_at_recv_char);
    uart0_read_for_hci(&app_at_recv_char, 1, app_at_uart_recv, NULL);
}

__attribute__((section("ram_code"))) void app_at_init_app(void)
{
    system_set_port_pull_up(GPIO_PB6, true);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_6, PORTB6_FUNC_CM3_UART_RXD);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_7, PORTB7_FUNC_CM3_UART_TXD);
    uart_init(BAUD_RATE_115200);

    uart0_read_for_hci(&app_at_recv_char, 1, app_at_uart_recv, NULL);
}

typedef void (*rwip_eif_callback) (void*, uint8_t);

struct uart_txrxchannel
{
    /// call back function pointer
    rwip_eif_callback callback;
};

struct uart_env_tag
{
    /// rx channel
    struct uart_txrxchannel rx;
    uint32_t rxsize;
    uint8_t *rxbufptr;
    void *dummy;
    /// error detect
    uint8_t errordetect;
    /// external wakeup
    bool ext_wakeup;
};

__attribute__((section("ram_code"))) void uart_isr_ram(void)
{
    uint8_t int_id;
    uint8_t c;
    rwip_eif_callback callback;
    void *dummy;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART_BASE;
    struct uart_env_tag *uart1_env = (struct uart_env_tag *)0x200015bc;

    int_id = uart_reg->u3.iir.int_id;

    if(int_id == 0x04 || int_id == 0x0c )   /* Receiver data available or Character time-out indication */
    {
        while(uart_reg->lsr & 0x01)
        {
            c = uart_reg->u1.data;
            *uart1_env->rxbufptr++ = c;
            uart1_env->rxsize--;
            if((uart1_env->rxsize == 0)
               &&(uart1_env->rx.callback))
            {
                uart_reg->u3.fcr.data = 0xf1;
                NVIC_DisableIRQ(UART_IRQn);
                uart_reg->u3.fcr.data = 0x21;
                callback = uart1_env->rx.callback;
                dummy = uart1_env->dummy;
                uart1_env->rx.callback = 0;
                uart1_env->rxbufptr = 0;
                callback(dummy, 0);
                break;
            }
        }
    }
    else if(int_id == 0x06)
    {
        volatile uint32_t line_status = uart_reg->lsr;
    }
}

