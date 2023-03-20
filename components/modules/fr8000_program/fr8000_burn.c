#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "driver_syscntl.h"
#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_pmu.h"
#include "user_utils.h"
#include "fr8000_burn.h"

enum fr8000_progam_status{
    PROGRAM_END,
    PROGRAM_CONTINUE,
    PROGRAM_REST,
};
enum Fr8000_update_param_opcode_t
{
    Fr8000_OPCODE_GET_TYPE,         // 0
    Fr8000_OPCODE_SEND_TYPE,        // 1
    Fr8000_OPCODE_WRITE,            // 2
    Fr8000_OPCODE_WRITE_ACK,        // 3
    Fr8000_OPCODE_WRITE_RAM,        // 4
    Fr8000_OPCODE_WRITE_RAM_ACK,    // 5
    Fr8000_OPCODE_READ_ENABLE,      // 6
    Fr8000_OPCODE_READ_ENABLE_ACK,  // 7
    Fr8000_OPCODE_READ,             // 8
    Fr8000_OPCODE_READ_ACK,         // 9
    Fr8000_OPCODE_READ_RAM,         // a
    Fr8000_OPCODE_READ_RAM_ACK,     // b
    Fr8000_OPCODE_BLOCK_ERASE,      // c
    Fr8000_OPCODE_BLOCK_ERASE_ACK,  // d
    Fr8000_OPCODE_CHIP_ERASE,       // e
    Fr8000_OPCODE_CHIP_ERASE_ACK,   // f
    Fr8000_OPCODE_DISCONNECT,       // 10
    Fr8000_OPCODE_DISCONNECT_ACK,   // 11
    Fr8000_OPCODE_CHANGE_BANDRATE,  // 12
    Fr8000_OPCODE_CHANGE_BANDRATE_ACK,  // 13
    Fr8000_OPCODE_ERROR,            // 14
    Fr8000_OPCODE_EXECUTE_CODE,     //15
    Fr8000_OPCODE_BOOT_RAM,         //16
    Fr8000_OPCODE_EXECUTE_CODE_END, //17
    Fr8000_OPCODE_BOOT_RAM_ACK,     //18
    Fr8000_OPCODE_CALC_CRC32,       //19
    Fr8000_OPCODE_CALC_CRC32_ACK,   //1a
    Fr8000_OPCODE_MAX,
};
struct Fr8000_update_param_header_t
{
    uint8_t code;
    uint32_t address;
    uint16_t length;
} __attribute__((packed));

const uint8_t user_fr8000_boot_conn_req[] = {'F','R','8','0','0','0','R','Q'};//from embedded to pc, request
const uint8_t user_fr8000_boot_conn_ack[] = {'F','R','8','0','0','X','O','K'};//from pc to embedded,ack
const uint8_t user_fr8000_boot_conn_success[] = {'o','k'};

const uint8_t app_fr8000_boot_conn_req[] = {'f','r','e','q','c','h','i','p'};//from embedded to pc, request
const uint8_t app_fr8000_boot_conn_ack[] = {'F','R','8','0','0','0','O','K'};//from pc to embedded,ack
const uint8_t app_fr8000_boot_conn_success[] = {'o','k'};
static uint8_t *boot_recv_buffer = (uint8_t *)0x40014400;
static uint8_t *boot_send_buffer = (uint8_t *)0x40014000;
static uint8_t *bool_recv_back_buffer = (uint8_t *)0x40012000;

struct Fr8000_update_param_header_t *rsp_header;
struct Fr8000_update_param_header_t *recv_header;
/***********************************************************************************
 * @fn     fr8000_HdOnkey_Init
 * 
 * @brief  initialize gpio that control fr8000 HD Onkey pin
 * 
 * @param  NULL
 * 
 * @return NULL
 * **********************************************************************************/
void fr8000_HdOnkey_Init(void)
{
    pmu_set_pin_dir(GPIO_PORT_D, (1<<GPIO_BIT_1), GPIO_DIR_OUT);
    pmu_set_gpio_value(GPIO_PORT_D, (1<<GPIO_BIT_1), 1);
    pmu_set_pin_to_PMU(GPIO_PORT_D, (1<<GPIO_BIT_1));

    HD_ONKEY_OFF();
}

/***********************************************************************************
 * @fn     fr8000_Resetkey_Init
 * 
 * @brief  initialize gpio that control fr8000 reset pin
 * 
 * @param  NULL
 * 
 * @return NULL
 * **********************************************************************************/
void fr8000_Resetkey_Init(void)
{
    pmu_set_pin_dir(GPIO_PORT_D, (1<<GPIO_BIT_2), GPIO_DIR_OUT);
    pmu_set_gpio_value(GPIO_PORT_D, (1<<GPIO_BIT_2), 1);
    pmu_set_pin_to_PMU(GPIO_PORT_D, (1<<GPIO_BIT_2));

    RESET_RELEASE();
}

/***********************************************************************************
 * @fn     fr8000_InterruptKey_Init
 * 
 * @brief  initialize gpio that control fr8000 interrupt pin
 * 
 * @param  NULL
 * 
 * @return NULL
 * **********************************************************************************/
void fr8000_InterruptKey_Init(void)
{
    pmu_set_pin_dir(GPIO_PORT_C, (1<<GPIO_BIT_5), GPIO_DIR_IN);
    pmu_set_pin_pull_down(GPIO_PORT_C, (1<<GPIO_BIT_5), true);
    pmu_set_pin_to_PMU(GPIO_PORT_C, (1<<GPIO_BIT_5));
}

void fr8000_VolModeKey_Init(void)
{
    pmu_set_pin_dir(GPIO_PORT_D, (1<<GPIO_BIT_0), GPIO_DIR_OUT);
    pmu_set_gpio_value(GPIO_PORT_D, (1<<GPIO_BIT_0), 1);
    pmu_set_pin_to_PMU(GPIO_PORT_D, (1<<GPIO_BIT_0));
}

/***********************************************************************************
 * @fn     fr8000_set_volatge
 * 
 * @brief  set fr8000 volatge
 * 
* @param  mode 1:3.3v 0:1.8v
 * 
 * @return NULL
 * **********************************************************************************/
void fr8000_set_volatge(uint8_t mode)
{
    if(mode)
    {
        pmu_set_gpio_value(GPIO_PORT_D, (1<<GPIO_BIT_0), 1);
    }
    else
    {
        pmu_set_gpio_value(GPIO_PORT_D, (1<<GPIO_BIT_0), 0);
    }
}
/***********************************************************************************
 * @fn     uart_switch
 * 
 * @brief  switch uart gpio
 * 
* @param  0:PA0,PA1 1:PB6,PB7 default:1
 * 
 * @return NULL
 * **********************************************************************************/
void uart_switch(uint8_t mode)
{
    switch (mode)
    {
        case 0:
        {
            system_set_port_pull_up(GPIO_PB6,true);
            system_set_port_pull_up(GPIO_PB7,true);
            gpio_set_dir(GPIO_PORT_B,GPIO_BIT_6,GPIO_DIR_IN);
            gpio_set_dir(GPIO_PORT_B,GPIO_BIT_7,GPIO_DIR_IN);
            system_set_port_mux(GPIO_PORT_B,GPIO_BIT_6,PORTB6_FUNC_B6);
            system_set_port_mux(GPIO_PORT_B,GPIO_BIT_7,PORTB7_FUNC_B7);

            system_set_port_pull_up(GPIO_PA0,true);
            system_set_port_pull_up(GPIO_PA1,true);
            system_set_port_mux(GPIO_PORT_A,GPIO_BIT_0,PORTA0_FUNC_CM3_UART_RX);
            system_set_port_mux(GPIO_PORT_A,GPIO_BIT_1,PORTA1_FUNC_CM3_UART_TX);
        }
        break;
        case 1:
        {
            #if 0
            system_set_port_pull_up(GPIO_PA0,true);
            system_set_port_pull_up(GPIO_PA1,true);
            gpio_set_dir(GPIO_PORT_A,GPIO_BIT_0,GPIO_DIR_IN);
            gpio_set_dir(GPIO_PORT_A,GPIO_BIT_1,GPIO_DIR_IN);
            system_set_port_mux(GPIO_PORT_A,GPIO_BIT_0,PORTA0_FUNC_A0);
            system_set_port_mux(GPIO_PORT_A,GPIO_BIT_1,PORTA1_FUNC_A1);
            #endif
            gpio_set_dir(GPIO_PORT_A,GPIO_BIT_0,GPIO_DIR_OUT);
            gpio_set_dir(GPIO_PORT_A,GPIO_BIT_1,GPIO_DIR_OUT);
            gpio_set_pin_value(GPIO_PORT_A,GPIO_BIT_1,true);
            gpio_set_pin_value(GPIO_PORT_A,GPIO_BIT_0,true);
            
            system_set_port_mux(GPIO_PORT_A,GPIO_BIT_0,PORTA0_FUNC_A0);
            system_set_port_mux(GPIO_PORT_A,GPIO_BIT_1,PORTA1_FUNC_A1);

            system_set_port_pull_up(GPIO_PB6,true);
            system_set_port_mux(GPIO_PORT_B,GPIO_BIT_6,PORTB6_FUNC_CM3_UART_RXD);
            system_set_port_mux(GPIO_PORT_B,GPIO_BIT_7,PORTB7_FUNC_CM3_UART_TXD);
        }
        break;
        default:
        break;
    }
}
/***********************************************************************************
 * @fn     get_Handshark_data
 * 
 * @brief  wait until recv handshark data or timeout
 * 
 * @param  ms:wait time
 *         recv_buff:storage recv buff
 *         buff_size:need recv buff size
 * 
 * @return recv size
 * **********************************************************************************/
static int get_Handshark_data(uint16_t ms,uint8_t *rcv_buff ,uint32_t buff_size)
{
    int i, n = 0;
    uint32_t recv_size;

    for(i = 0; i < ms; i++)
    {
        co_delay_100us(10);
        recv_size = uart_get_data_nodelay_noint(rcv_buff+n, buff_size);
        n += recv_size;
        buff_size -= recv_size;
        if(0 == buff_size)
        {
            return n;
        }
    }

    return -1;
}
/***********************************************************************************
 * @fn     fr8000_hdonkey_wakeup
 * 
 * @brief  use HDONKEY pin wake up to fr8000
 * 
 * @param  NULL
 * 
 * @return NULL
 * **********************************************************************************/
static void fr8000_hdonkey_wakeup(void)
{
    HD_ONKEY_ON();
    co_delay_100us(10);
    HD_ONKEY_OFF();
}

/***********************************************************************************
 * @fn     fr8000_reset
 * 
 * @brief  reset fr8000 by hardware
 * 
 * @param  NULL
 * 
 * @return NULL
 * **********************************************************************************/
void fr8000_reset(void)
{
    RESET_EN();
    co_delay_100us(600);
    RESET_RELEASE();
}
/***********************************************************************************
 * @fn     fr8000_progaram
 * 
 * @brief  program fr8000
 * 
 * @param  NULL
 * 
 * @return NULL
 * **********************************************************************************/
static void fr8000_progaram_loop(void)
{
    uint8_t do_handshark = 0;
    uint8_t baud_rate;
    uint8_t retry_count = 1;
    uint8_t retry_count_2 = 30;
    uint8_t fr8000_program_mode = 1;
    uint8_t buffer[sizeof(app_fr8000_boot_conn_req)+1];
    uint8_t get_flag_type[sizeof(struct Fr8000_update_param_header_t)]={0};

    /*fr8000 handshark*/
    while(retry_count)  
    {
        /*reset fr8000*/
        RESET_EN();
        uart_switch(0);
        co_delay_100us(600);
        RESET_RELEASE();

        HD_ONKEY_ON();
        co_delay_100us(100);
        HD_ONKEY_OFF();
        
        while (retry_count_2--) {
            if (get_Handshark_data(10,buffer,1) == 1) {
                if(buffer[0] == 'f') {
                    break;
                }
            }
        }
        
        if(get_Handshark_data(100,&buffer[1],sizeof(app_fr8000_boot_conn_req)-1) == (sizeof(app_fr8000_boot_conn_req)-1))
        {
            if(memcmp(&buffer[0], app_fr8000_boot_conn_req, sizeof(app_fr8000_boot_conn_req)) == 0)
            {
               uart_write((uint8_t *)app_fr8000_boot_conn_ack, sizeof(app_fr8000_boot_conn_ack));
               uart_finish_transfers();
               if(get_Handshark_data(100,buffer,sizeof(app_fr8000_boot_conn_success)) == sizeof(app_fr8000_boot_conn_success))
               {
                   if(memcmp(buffer, app_fr8000_boot_conn_success, sizeof(app_fr8000_boot_conn_success)-1) == 0)
                   {
                       struct Fr8000_update_param_header_t *recv_header = (struct Fr8000_update_param_header_t *)&boot_recv_buffer[0];
                       uart_write((uint8_t *)get_flag_type, sizeof(struct Fr8000_update_param_header_t));
                       uart_finish_transfers();
                       if(get_Handshark_data(100,(uint8_t *)recv_header,sizeof(struct Fr8000_update_param_header_t)) == sizeof(struct Fr8000_update_param_header_t))
                       {
                           uart_switch(1);
                           uart_write((uint8_t *)recv_header, sizeof(struct Fr8000_update_param_header_t));
                           uart_finish_transfers();
                           do_handshark = 1;
                       }

                       break;
                   }
               }
            }
        }
        retry_count--;   
    }
    /*fr8000 program*/
    if(do_handshark)
    {
        uint8_t result = PROGRAM_CONTINUE;
        while(result == PROGRAM_CONTINUE)
        { 
            if(fr8000_program_mode)
            {
                recv_header = (struct Fr8000_update_param_header_t *)&boot_recv_buffer[0];
                uart_get_data_noint((uint8_t *)recv_header, sizeof(struct Fr8000_update_param_header_t));
                if(recv_header->code == Fr8000_OPCODE_CHANGE_BANDRATE)
                {
                    baud_rate = recv_header->address;
                }
                if((recv_header->length != 0)
                   &&(recv_header->code != Fr8000_OPCODE_READ)
                   &&(recv_header->code != Fr8000_OPCODE_READ_RAM))
                {
                    if(recv_header->code != Fr8000_OPCODE_WRITE) {
                        uart_get_data_noint(((uint8_t *)recv_header)+sizeof(struct Fr8000_update_param_header_t), recv_header->length);
                        uart_switch(0);
                        uart_write((uint8_t *)recv_header,sizeof(struct Fr8000_update_param_header_t)+recv_header->length);
                        uart_finish_transfers();
                    }
                    else {
                        uart_get_data_noint(bool_recv_back_buffer, recv_header->length);
                        uart_switch(0);
                        uart_write((uint8_t *)recv_header,sizeof(struct Fr8000_update_param_header_t));
                        uart_finish_transfers();
                        uart_write(bool_recv_back_buffer,recv_header->length);
                        uart_finish_transfers();
                    }
                }
                else
                {
                    uart_switch(0);
                    uart_write((uint8_t *)recv_header,sizeof(struct Fr8000_update_param_header_t));
                    uart_finish_transfers();
                }
                fr8000_program_mode = 0;
            }
            else
            {
                rsp_header = (struct Fr8000_update_param_header_t *)&boot_send_buffer[0];
                if(get_Handshark_data(30000,(uint8_t *)rsp_header,sizeof(struct Fr8000_update_param_header_t)) == 
                                sizeof(struct Fr8000_update_param_header_t))
                {
                    if(rsp_header->code != Fr8000_OPCODE_READ_ACK)
                    {
                        uart_switch(1);
                        uart_write((uint8_t *)rsp_header,sizeof(struct Fr8000_update_param_header_t));
                        uart_finish_transfers();
                    }
                    else
                    {
                        if(get_Handshark_data(30000,(uint8_t *)(rsp_header + sizeof(struct Fr8000_update_param_header_t)),rsp_header->length) == 
                                rsp_header->length)
                        {
                            uart_switch(1);
                            uart_write((uint8_t *)rsp_header,sizeof(struct Fr8000_update_param_header_t)+rsp_header->length);
                            uart_finish_transfers();
                        }
                    }
                }

                if(rsp_header->code == Fr8000_OPCODE_CHANGE_BANDRATE_ACK)
                {
                    uart_init(baud_rate);
                }
                if(rsp_header->code == Fr8000_OPCODE_DISCONNECT_ACK)
                {
                    result = PROGRAM_END;
                    co_delay_100us(100);
                    fr8000_reset();
                }
               fr8000_program_mode = 1; 
            }
            
        }
    }    
}
/***********************************************************************************
 * @fn     fr8000_handshark
 * 
 * @brief  handshark with fr8000
 * 
 * @param  NULL
 * 
 * @return NULL
 * **********************************************************************************/
void fr8000_program(void)
{
    uint8_t retry_count = 1;
    uint8_t do_handshark = 0;
    uint8_t buffer[sizeof(user_fr8000_boot_conn_ack)];
    uart_init(BAUD_RATE_115200);
    fr8000_HdOnkey_Init();
    fr8000_Resetkey_Init();
    fr8000_InterruptKey_Init();
    fr8000_VolModeKey_Init();
    fr8000_set_volatge(1);
    while (retry_count)
    {
        uart_write(user_fr8000_boot_conn_req,sizeof(user_fr8000_boot_conn_req));
        if(get_Handshark_data(500,buffer,sizeof(user_fr8000_boot_conn_ack)) == sizeof(user_fr8000_boot_conn_ack))
        {
            if(memcmp(buffer, user_fr8000_boot_conn_ack, sizeof(user_fr8000_boot_conn_ack)) == 0)
            {
                do_handshark = 1;
                uart_write((uint8_t *)user_fr8000_boot_conn_success, sizeof(user_fr8000_boot_conn_success));
                system_set_pclk(SYSTEM_SYS_CLK_48M);
                break;
            }
        }
        retry_count--;
    }
    
    if(do_handshark)
    {
       struct Fr8000_update_param_header_t *recv_header = (struct Fr8000_update_param_header_t *)&boot_recv_buffer[0];
       if(get_Handshark_data(500,(uint8_t *)recv_header,sizeof(struct Fr8000_update_param_header_t)) == sizeof(struct Fr8000_update_param_header_t))
       {
           if(recv_header->code == Fr8000_OPCODE_GET_TYPE)
           {
               fr8000_progaram_loop();
           }
       }
    }
}

