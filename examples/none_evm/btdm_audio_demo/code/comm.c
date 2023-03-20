/*
  ******************************************************************************
  * @file    comm.c
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   freqchip Multimachine communication.
  *          This file provides firmware functions to manage the 
  *          Multimachine communication.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#include "comm.h"

/* import functions */
void app_at_recv_cmd_A(uint8_t sub_cmd, uint8_t *data);
void app_at_recv_cmd_B(uint8_t sub_cmd, uint8_t *data);
void app_at_recv_cmd_C(uint8_t sub_cmd, uint8_t *data);
void app_at_recv_cmd_F(uint8_t sub_cmd, uint8_t *data);
void app_at_recv_cmd_P(uint8_t sub_cmd, uint8_t *data);
void app_at_recv_cmd_L(uint8_t sub_cmd, uint8_t *data);
/* END */

/* private functions */
static void comm_receive_cmd_handle(uint8_t *fp8_buffer, uint32_t fu32_DataLength);
static void comm_receive_status_handle(uint8_t *fp8_buffer);
static void comm_send_TxACK(void);
static void comm_remove_TxACK(void);
static void comm_receive_RxACK(void);
static void comm_remove_RxACK(void);
/* END */

uint8_t RxBuffer[800];
uint8_t TxBuffer[800];

rt_list_t DataPool_List;

volatile bool gb_ack_TxStatus;
volatile bool gb_ack_RxStatus;

volatile bool gb_comm_init = false;

void comm_post_msg(void)
{
    os_event_t at_cmd_event;
    
    at_cmd_event.event_id  = USER_EVT_5088_COMM;
    at_cmd_event.param     = 0;
    at_cmd_event.param_len = 0;
    os_msg_post(user_task_id, &at_cmd_event);
}

#if FR5088_COMM_ENABLE

/************************************************************************************
 * @fn      pmu_gpio_isr_ram
 *
 * @brief   pmu gpio interrupt handle
 */
__attribute__((section("ram_code"))) void pmu_gpio_isr_ram(void)
{
//    uint8_t lu8_PortC_Value = ool_read(PMU_REG_PORTC_INPUT_VAL);

//    if((lu8_PortC_Value & 0x10) == 0)
//    {
//        comm_send_TxACK();
//    }

//    ool_write(PMU_REG_STATE_MON_IN_C, lu8_PortC_Value);
    
    uint8_t lu8_PortC_Value = ool_read(PMU_REG_PORTC_INPUT_VAL);

    if((lu8_PortC_Value & 0x40) == 0)
    {
        comm_send_TxACK();
        
        comm_post_msg();
    }

    ool_write(PMU_REG_STATE_MON_IN_C, lu8_PortC_Value);
}

/************************************************************************************
 * @fn      uart_isr_ram
 *
 * @brief   uart interrupt handle
 */
__attribute__((section("ram_code"))) void uart_isr_ram(void)
{
    uint8_t lu8_InterruptID;
    uint8_t lu8_Buffer;

    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART_BASE;

    static enum_RxStatus_t RxStatus = RX_IDLE;
    static uint32_t lu32_Datalength;
    static uint32_t lu32_Count = 0;

    lu8_InterruptID = uart_reg->u3.iir.int_id;

    switch (lu8_InterruptID)
    {
        /* Receiver data available or Character time-out indication */
        case 0x04: 
        case 0x0C:
        {
            /* Data ready */
            while(uart_reg->lsr & 0x01)
            {          
                lu8_Buffer = uart_reg->u1.data;

                switch (RxStatus)
                {
                    /* check start character */
                    case RX_IDLE:
                    {
                        if (lu8_Buffer == CHAR_START_0) 
                        {
                            RxStatus = RX_START_0;
                        }
                    }break;

                    /* receive first start character */
                    case RX_START_0:
                    {
                        if (lu8_Buffer == CHAR_START_1) 
                        {
                            RxStatus = RX_START_1;
                        }
                        else
                        {
                            lu32_Count = 0;
                            RxStatus = RX_IDLE;
                        }
                    }break;

                    /* receive second start character */
                    case RX_START_1:
                    {
                        if (lu8_Buffer == CHAR_START_2) 
                        {
                            RxStatus = RX_REVEICE_LENGTH_1;
                        }
                        else if (lu8_Buffer == CHAR_START_3) 
                        {
                            RxStatus = RX_STATUS_1;
                        }
                        else
                        {
                            lu32_Count = 0;
                            RxStatus = RX_IDLE;
                        }
                    }break;

                    case RX_REVEICE_LENGTH_1:
                    {
                        lu32_Datalength = (uint32_t)lu8_Buffer << 8;
                        RxStatus = RX_REVEICE_LENGTH_2;
                    }break;

                    case RX_REVEICE_LENGTH_2:
                    {
                        lu32_Datalength |= (uint32_t)lu8_Buffer;
                        RxStatus = RX_REVEICE_DATA;

                        if (lu32_Datalength == 0) 
                        {
                            lu32_Count = 0;
                            RxStatus = RX_IDLE;
                        }
                    }break;

                    case RX_REVEICE_DATA:
                    {
                        RxBuffer[lu32_Count++] = lu8_Buffer;

                        if (lu32_Count >= lu32_Datalength) 
                        {
                            lu32_Count = 0;
                            RxStatus = RX_IDLE;

                            comm_receive_cmd_handle(RxBuffer, lu32_Datalength);
                        }
                    }break;

                    /* receive frist status character */
                    case RX_STATUS_1:
                    {
                        RxBuffer[lu32_Count++] = lu8_Buffer;
                        RxStatus = RX_STATUS_2;
                    }break;

                    /* receive second status character */
                    case RX_STATUS_2:
                    {
                        RxBuffer[lu32_Count++] = lu8_Buffer;

                        lu32_Count = 0;
                        RxStatus = RX_IDLE;

                        comm_receive_status_handle(RxBuffer);
                    }break;

                    default:break;
                }
            }
        }break;

        /* Tx Empty */
        case 0x02:
        {
            
        }break;
        
        default: break; 
    }
}

#endif

/************************************************************************************
 * @fn      uart_send_data
 *
 * @brief   uart send data
 */
__attribute__((section("ram_code"))) void uart_send_data(uint8_t *fp8_Buffer, uint32_t fu32_Length)
{
    uint32_t i;
    
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART_BASE;

    while (fu32_Length) 
    {
        if (uart_reg->lsr & 0x20) 
        {
            if (fu32_Length / 30) 
            {
                for (i = 0; i < 30; i++)
                {
                    uart_reg->u1.data = *fp8_Buffer++;
                }
                fu32_Length -= 30;
            }
            else 
            {
                for (i = 0; i < fu32_Length; i++)
                {
                    uart_reg->u1.data = *fp8_Buffer++;
                }

                fu32_Length = 0;
            }
        }
    }
}

/************************************************************************************
 * @fn      comm_receive_cmd_handle
 *
 * @brief   freqchip Multimachine communication command handle.
 */
static void comm_receive_cmd_handle(uint8_t *fp8_buffer, uint32_t fu32_DataLength)
{
    switch (fp8_buffer[0])
    {
        case TYPE_CMD_A: app_at_recv_cmd_A(fp8_buffer[1], &fp8_buffer[2]); break;
        case TYPE_CMD_B: app_at_recv_cmd_B(fp8_buffer[1], &fp8_buffer[2]); break;
        case TYPE_CMD_C: app_at_recv_cmd_C(fp8_buffer[1], &fp8_buffer[2]); break;
        case TYPE_CMD_F: app_at_recv_cmd_F(fp8_buffer[1], &fp8_buffer[2]); break;
        case TYPE_CMD_P: app_at_recv_cmd_P(fp8_buffer[1], &fp8_buffer[2]); break;

        case TYPE_CMD_L: app_at_recv_cmd_L(fp8_buffer[1], &fp8_buffer[2]); break;

        default:break;
    }
}

/************************************************************************************
 * @fn      comm_receive_data_handle
 *
 * @brief   freqchip Multimachine communication receive status handle.
 */
static void comm_receive_status_handle(uint8_t *fp8_buffer)
{
    switch (fp8_buffer[0])
    {
        case TYPE_STATUS:
        {
            switch (fp8_buffer[1])
            {
                case TYPE_STATUS_READY: 
                {
                    /* CMD DATA queu not empty */
                    if (DataPool_List.next != &DataPool_List)
                    {
                        comm_receive_RxACK();
                    }
                }break;

                default: break; 
            }
        }break;

        default:break;
    }
}

/************************************************************************************
 * @fn      comm_add_cmd
 *
 * @brief   Add cmd and data to datapool
 */
void comm_add_cmd(uint8_t fu8_type, uint8_t fu8_index, uint8_t fu8_dataLength, uint8_t *fu8_data)
{
    if (gb_comm_init == false)
        return;

    struct_CMDData_t *CMDDataNode;

    CMDDataNode = os_malloc((sizeof(struct_CMDData_t) + fu8_dataLength));

    CMDDataNode->u8_DataLength = fu8_dataLength;
    CMDDataNode->u8_type       = fu8_type;
    CMDDataNode->u8_type_index = fu8_index;

    memcpy(CMDDataNode->u8_Data, fu8_data, fu8_dataLength);

    GLOBAL_INT_DISABLE();
    rt_list_insert_after(&DataPool_List, &CMDDataNode->list);
    GLOBAL_INT_RESTORE();

    comm_post_msg();
}

/************************************************************************************
 * @fn      comm_remove_cmd
 *
 * @brief   remove cmd and data from datapool
 */
void comm_remove_cmd(rt_list_t *fp_list)
{
    GLOBAL_INT_DISABLE();
    rt_list_remove(fp_list);
    GLOBAL_INT_RESTORE();
}

/************************************************************************************
 * @fn      comm_request_send
 *
 * @brief   request cmd data transmit.
 */
static void comm_request_send(void)
{
    volatile uint32_t lu32_delay = 20;

//    pmu_set_gpio_value(GPIO_PORT_C, 0x20, 0);    /* PC5 */
//    while(lu32_delay--);
//    pmu_set_gpio_value(GPIO_PORT_C, 0x20, 1);    /* PC5 */
    pmu_set_gpio_value(GPIO_PORT_C, 0x80, 0);    /* PC7 */
    while(lu32_delay--);
    pmu_set_gpio_value(GPIO_PORT_C, 0x80, 1);    /* PC7 */
}

/************************************************************************************
 * @fn      comm_send_TxACK
 *
 * @brief   Add ack to datapool
 */
static void comm_send_TxACK(void)
{
    gb_ack_TxStatus = true;
}

/************************************************************************************
 * @fn      comm_remove_TxACK
 *
 * @brief   remove ack from datapool
 */
static void comm_remove_TxACK(void)
{
    gb_ack_TxStatus = false;
}

/************************************************************************************
 * @fn      comm_receive_RxACK
 *
 * @brief   get ack
 */
static void comm_receive_RxACK(void)
{
    gb_ack_RxStatus = true;
}

/************************************************************************************
 * @fn      comm_remove_RxACK
 *
 * @brief   remove ack
 */
static void comm_remove_RxACK(void)
{
    gb_ack_RxStatus = false;
}

/************************************************************************************
 * @fn      comm_receive_poll
 *
 * @brief   freqchip Multimachine communication receive poll handle.
 */
void comm_receive_poll(void)
{
    
}

/************************************************************************************
 * @fn      comm_transmit_poll
 *
 * @brief   freqchip Multimachine communication receive poll handle.
 */
void comm_transmit_poll(void)
{
    uint32_t i;
    uint32_t lu32_TxLength;
	
	static uint32_t lu32_TimeCount  = 0;
	static uint32_t lu32_TimeBackup = 0;

	lu32_TimeCount++;

    struct_CMDData_t *CMDDataNode;
    
    /* construct transmit data */

    /* send response */
    if (gb_ack_TxStatus) 
    {
        TxBuffer[0] = CHAR_START_0;
        TxBuffer[1] = CHAR_START_1;
        TxBuffer[2] = CHAR_START_3;
        TxBuffer[3] = TYPE_STATUS;
        TxBuffer[4] = TYPE_STATUS_READY;

        uart_send_data(TxBuffer, 5);

        comm_remove_TxACK();
    }
    else if (DataPool_List.next != &DataPool_List)
    {
        /* Received response */
        if (gb_ack_RxStatus) 
        {
            CMDDataNode = (struct_CMDData_t *)DataPool_List.next;

            if (CMDDataNode->u8_type == TYPE_STATUS) 
            {
                TxBuffer[0] = CHAR_START_0;
                TxBuffer[1] = CHAR_START_1;
                TxBuffer[2] = CHAR_START_3;
                TxBuffer[3] = TYPE_STATUS;
                TxBuffer[4] = CMDDataNode->u8_type_index;

                uart_send_data(TxBuffer, 5);
            }
            else 
            {
                lu32_TxLength = 2 + CMDDataNode->u8_DataLength;

                TxBuffer[0] = CHAR_START_0;
                TxBuffer[1] = CHAR_START_1;
                TxBuffer[2] = CHAR_START_2;
                TxBuffer[3] = (lu32_TxLength >> 8) & 0xFF;
                TxBuffer[4] = (lu32_TxLength) & 0xFF;

                uart_send_data(TxBuffer, 5);
                
                uart_send_data(&CMDDataNode->u8_type, lu32_TxLength);
            }
            comm_remove_cmd(&CMDDataNode->list);
            comm_remove_RxACK();
        }
        else 
        {
			if (lu32_TimeCount - lu32_TimeBackup > 10)
			{
				comm_request_send();
				
				lu32_TimeBackup = lu32_TimeCount;
			}
            
            comm_post_msg();
        }
    }
}

/************************************************************************************
 * @fn      comm_peripheral_init
 *
 * @brief   freqchip Multimachine communication uart gpio init.
 */
void comm_peripheral_init(void)
{
    /* init Datapool list  */
    rt_list_init(&DataPool_List);
    
    /* init uart */
    system_set_port_pull_up(GPIO_PB6, true);
    system_regs->reset_ctrl.cm3_uart_sft_rst = 1;
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_A6);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_7, PORTA6_FUNC_A6);

    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_6, PORTB6_FUNC_CM3_UART_RXD);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_7, PORTB7_FUNC_CM3_UART_TXD);

    uart_init(BAUD_RATE_921600);

    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART_BASE;
    uart_reg->u3.fcr.data = 0x91;

    /* Rx interrupt enable */
    uart_enable_isr(1, 0);
    
    NVIC_EnableIRQ(UART_IRQn);

//    /* init gpio */
//    /* enable PC4 as wakeup pin */
//    pmu_set_pin_pull_up(GPIO_PORT_C, 1 << GPIO_BIT_4, true);
//    pmu_port_wakeup_func_set(GPIO_PC4);

//    /* send request gpio */
//    pmu_set_pin_to_PMU(GPIO_PORT_C, 1 << GPIO_BIT_5);
//    pmu_set_pin_dir(GPIO_PORT_C,    1 << GPIO_BIT_5, GPIO_DIR_OUT);
//    pmu_set_gpio_value(GPIO_PORT_C, 1 << GPIO_BIT_5, 1);

    /* init gpio */
    /* enable PC6 as wakeup pin */
    pmu_set_pin_pull_up(GPIO_PORT_C, 1 << GPIO_BIT_6, true);
    pmu_port_wakeup_func_set(GPIO_PC6);

    /* send request gpio */
    pmu_set_pin_to_PMU(GPIO_PORT_C, 1 << GPIO_BIT_7);
    pmu_set_pin_dir(GPIO_PORT_C,    1 << GPIO_BIT_7, GPIO_DIR_OUT);
    pmu_set_gpio_value(GPIO_PORT_C, 1 << GPIO_BIT_7, 1);
    
    gb_comm_init = true;
}

/************************************************************************************
 * @fn      comm_poll
 *
 * @brief   freqchip Multimachine communication poll handle.
 */
void main_schedule(void)
{
//    comm_transmit_poll();
}

