/*
  ******************************************************************************
  * @file    comm.h
  * @author  FreqChip Firmware Team
  * @version V1.0.0
  * @date    2021
  * @brief   Header file of Multimachine communication.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 FreqChip.
  * All rights reserved.
  ******************************************************************************
*/
#ifndef __FREQ_COMM_H__
#define __FREQ_COMM_H__

#include "string.h"
#include "os_mem.h"
#include "os_msg_q.h"
#include "user_task.h"
#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_pmu.h"
#include "driver_syscntl.h"

/* communication start character, stop character */
#define CHAR_START_0    ('/')    /* 0x2F */
#define CHAR_START_1    ('#')    /* 0x23 */
#define CHAR_START_2    ('D')    /* 0x44, Data package */
#define CHAR_START_3    ('S')    /* 0x53, Status package */

/* communication status index */
#define TYPE_STATUS    ('S')
#define TYPE_STATUS_READY    ('R')    /* Receive ready */
#define TYPE_STATUS_OK       ('O')    /* OK */
#define TYPE_STATUS_FAIL     ('F')    /* FAIL */

typedef enum
{
    RX_IDLE,
    RX_START_0,
    RX_START_1,
    RX_REVEICE_LENGTH_1,
    RX_REVEICE_LENGTH_2,
    RX_REVEICE_DATA,
    RX_STATUS_1,
    RX_STATUS_2,
}enum_RxStatus_t;

/**
 * Double List structure
 */
struct rt_list_node
{
    struct rt_list_node *next;            /**< point to next node. */
    struct rt_list_node *prev;            /**< point to prev node. */
};
typedef struct rt_list_node rt_list_t;    /**< Type for lists. */

/* initialize a list */
static inline void rt_list_init(rt_list_t *l)
{
    l->next = l->prev = l;
}
/* insert a node after a list */
static inline void rt_list_insert_after(rt_list_t *l, rt_list_t *n)
{
    l->prev->next = n;
    n->prev = l->prev;

    l->prev = n;
    n->next = l;
}
/* insert a node before a list */
static inline void rt_list_insert_before(rt_list_t *l, rt_list_t *n)
{
    l->next->prev = n;
    n->next = l->next;

    l->next = n;
    n->prev = l;
}
/* rt_list_remove */
static inline void rt_list_remove(rt_list_t *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;

    n->next = n->prev = n;
}

typedef struct 
{
    rt_list_t list;

    uint8_t  u8_DataLength;
    uint8_t  u8_type;
    uint8_t  u8_type_index;
    uint8_t  u8_Data[];
}struct_CMDData_t;

/* ------------------------------------------------------------------------ */
/* ---------------------------- BT Command -------------------------------- */
/* ------------------------------------------------------------------------ */

/* communication event index */
#define TYPE_EVENT     ('E')
#define TYPE_EVENT_INQ     ('A')    /* ������� */
#define TYPE_EVENT_CONN    ('B')    /* ���������� */
#define TYPE_EVENT_DISC    ('C')    /* �����Ͽ� */
#define TYPE_EVENT_ACC     ('D')    /* ����ģʽ�ı� */
#define TYPE_EVENT_RING    ('E')    /* �������� */
#define TYPE_EVENT_CLI     ('F')    /* ������� */
#define TYPE_EVENT_AC      ('J')    /* ����ͨ·���� */
#define TYPE_EVENT_AD      ('H')    /* ����ͨ·�Ͽ� */
#define TYPE_EVENT_MS      ('I')    /* ����ͨ·���� */
#define TYPE_EVENT_MT      ('G')    /* ����ͨ·��ͣ */
#define TYPE_EVENT_MC      ('K')    /* �����ı� */
#define TYPE_EVENT_MN      ('L')    /* ��������ʱ�� */
#define TYPE_EVENT_VOL     ('M')    /* �����ı� */
#define TYPE_EVENT_MP3     ('N')    /* ������ϸ��Ϣ */
#define TYPE_EVENT_NAME    ('O')    /* ������Ҫ��Ϣ */
#define TYPE_EVENT_MNUM    ('P')    /* MP3������ */
#define TYPE_EVENT_PROG    ('Q')    /* MP3���Ž��� */
#define TYPE_EVENT_PEC     ('R')    /* PBAP��� */
#define TYPE_EVENT_PEA     ('S')    /* PBAP���� */
#define TYPE_EVENT_PP      ('T')    /* PBAP�绰����Ϣ */
#define TYPE_EVENT_MAC     ('U')    /* ������ַ */
#define TYPE_EVENT_VER     ('V')    /* ����SDK�汾 */
#define TYPE_EVENT_STATE   ('W')    /* ����״̬ */
#define TYPE_EVENT_X        /* reserve */
#define TYPE_EVENT_Y        /* reserve */
#define TYPE_EVENT_Z        /* reserve */
#define TYPE_EVENT_1       ('1')    /* ATT����ֵ, ������ظ� */
#define TYPE_EVENT_2       ('2')    /* ATT����ֵ, �����ϱ� */

/* communication command index */
#define TYPE_CMD_A    ('A')
#define TYPE_CMD_A_B     ('B')    /* ���ز�����һ�� */
#define TYPE_CMD_A_C     ('C')    /* ���ز��ſ�ʼ */
#define TYPE_CMD_A_D     ('D')    /* ���ز�����ͣ */
#define TYPE_CMD_A_E     ('E')    /* ���ز�����һ�� */
#define TYPE_CMD_A_L     ('L')    /* ���ű�����ʾ�� */
#define TYPE_CMD_A_M     ('M')    /* ����MIC�ػ� */
#define TYPE_CMD_A_N     ('N')    /* �ر�MIC�ػ� */
#define TYPE_CMD_A_S     ('S')    /* ����MIC¼�� */
#define TYPE_CMD_A_T     ('T')    /* �ر�MIC¼�� */
#define TYPE_CMD_A_V     ('V')    /* ����������һ�� */
#define TYPE_CMD_A_W     ('W')    /* ������ʼ���� */
#define TYPE_CMD_A_X     ('X')    /* ����������ͣ */
#define TYPE_CMD_A_Y     ('Y')    /* ����������һ�� */

#define TYPE_CMD_B    ('B')
#define TYPE_CMD_B_R     ('R')    /* ���ֻ�Siri */
#define TYPE_CMD_B_S     ('S')    /* �ر��ֻ�Siri */

#define TYPE_CMD_C    ('C')
#define TYPE_CMD_C_A     ('A')    /* �����绰 */
#define TYPE_CMD_C_B     ('B')    /* �Ҷϵ绰 */
#define TYPE_CMD_C_C     ('C')    /* �ز��绰 */
#define TYPE_CMD_C_D     ('D')    /* �ֻ�������ͣ */
#define TYPE_CMD_C_E     ('E')    /* �ֻ����ֲ��� */
#define TYPE_CMD_C_F     ('F')    /* �ֻ�������һ�� */
#define TYPE_CMD_C_G     ('G')    /* �ֻ�������һ�� */
#define TYPE_CMD_C_H     ('H')    /* �����豸 */
#define TYPE_CMD_C_I     ('I')    /* ȡ������ */
#define TYPE_CMD_C_J     ('J')    /* ���������豸 + Param */
#define TYPE_CMD_C_K     ('K')    /* �Ͽ����� */
#define TYPE_CMD_C_L     ('L')    /* ��ѯ����״̬ */
#define TYPE_CMD_C_N     ('N')    /* ����绰 + Param */
#define TYPE_CMD_C_O     ('O')    /* �л���Source */
#define TYPE_CMD_C_Q     ('Q')    /* �л���SINK */
#define TYPE_CMD_C_R     ('R')    /* �������� */
#define TYPE_CMD_C_X     ('X')    /* �ر����� */
#define TYPE_CMD_C_Y     ('Y')    /* ������� */

#define TYPE_CMD_F    ('F')
#define TYPE_CMD_F_A     ('A')    /* ������ϸ��Ϣ */
#define TYPE_CMD_F_B     ('B')    /* ������Ҫ��Ϣ */
#define TYPE_CMD_F_C     ('C')    /* ��ȡ��������Ŀ */
#define TYPE_CMD_F_D     ('D')    /* ��ȡ������ַ */
#define TYPE_CMD_F_E     ('E')    /* ��ȡ�����汾 */
#define TYPE_CMD_F_F     ('F')    /* ʹ���¼��ϱ� */
#define TYPE_CMD_F_G     ('G')    /* �ػ� */

#define TYPE_CMD_P    ('P')
#define TYPE_CMD_P_C     ('C')    /* ����PBAP���� */
#define TYPE_CMD_P_P     ('P')    /* ��ȡ�ֻ��绰�� */
#define TYPE_CMD_P_T     ('T')    /* �Ͽ�PBAP���� */

/* ------------------------------------------------------------------------ */
/* ---------------------------- BLE Command ------------------------------- */
/* ------------------------------------------------------------------------ */
#define TYPE_CMD_L    ('L')
#define TYPE_CMD_L_A     ('A')    /* ����BLE�㲥 */
#define TYPE_CMD_L_B     ('B')    /* �ر�BLE�㲥 */
#define TYPE_CMD_L_C     ('C')    /* BLE�������� */
#define TYPE_CMD_L_R     ('R')    /* ��ATT����ֵ */
#define TYPE_CMD_L_W     ('W')    /* дATT����ֵ */

/* Exported parameter ---------------------------------------------------------*/
extern rt_list_t DataPool_List;

extern volatile bool gb_ack_TxStatus;
extern volatile bool gb_ack_RxStatus;

/* Exported functions ---------------------------------------------------------*/
void comm_peripheral_init(void);

void comm_add_cmd(uint8_t fu8_type, uint8_t fu8_index, uint8_t fu8_dataLength, uint8_t *fu8_data);

void comm_transmit_poll(void);



#endif
