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
#include "user_utils.h"

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


void app_at_cmd_recv_handler(uint8_t *data, uint16_t length)
{
    switch(data[0])
    {
        case 'A':
            app_at_recv_cmd_A(data[1], &data[2]);
            break;
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
                at_cmd_event.event_id = USER_EVT_AT_COMMAND;
                at_cmd_event.param = at_recv_buffer;
                at_cmd_event.param_len = at_recv_index;
                os_msg_post(user_task_id, &at_cmd_event);

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
    system_set_port_pull_up((GPIO_PORT_B*8)+GPIO_BIT_6, true);
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

