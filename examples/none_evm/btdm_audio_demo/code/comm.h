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
#define TYPE_EVENT_INQ     ('A')    /* 搜索结果 */
#define TYPE_EVENT_CONN    ('B')    /* 蓝牙已连接 */
#define TYPE_EVENT_DISC    ('C')    /* 蓝牙断开 */
#define TYPE_EVENT_ACC     ('D')    /* 接入模式改变 */
#define TYPE_EVENT_RING    ('E')    /* 来电振铃 */
#define TYPE_EVENT_CLI     ('F')    /* 来电号码 */
#define TYPE_EVENT_AC      ('J')    /* 语音通路建立 */
#define TYPE_EVENT_AD      ('H')    /* 语音通路断开 */
#define TYPE_EVENT_MS      ('I')    /* 音乐通路建立 */
#define TYPE_EVENT_MT      ('G')    /* 音乐通路暂停 */
#define TYPE_EVENT_MC      ('K')    /* 歌曲改变 */
#define TYPE_EVENT_MN      ('L')    /* 歌曲名称时长 */
#define TYPE_EVENT_VOL     ('M')    /* 音量改变 */
#define TYPE_EVENT_MP3     ('N')    /* 歌曲详细信息 */
#define TYPE_EVENT_NAME    ('O')    /* 歌曲简要信息 */
#define TYPE_EVENT_MNUM    ('P')    /* MP3总数量 */
#define TYPE_EVENT_PROG    ('Q')    /* MP3播放进度 */
#define TYPE_EVENT_PEC     ('R')    /* PBAP完成 */
#define TYPE_EVENT_PEA     ('S')    /* PBAP故障 */
#define TYPE_EVENT_PP      ('T')    /* PBAP电话本信息 */
#define TYPE_EVENT_MAC     ('U')    /* 蓝牙地址 */
#define TYPE_EVENT_VER     ('V')    /* 蓝牙SDK版本 */
#define TYPE_EVENT_STATE   ('W')    /* 连接状态 */
#define TYPE_EVENT_X        /* reserve */
#define TYPE_EVENT_Y        /* reserve */
#define TYPE_EVENT_Z        /* reserve */
#define TYPE_EVENT_1       ('1')    /* ATT属性值, 读请求回复 */
#define TYPE_EVENT_2       ('2')    /* ATT属性值, 主动上报 */

/* communication command index */
#define TYPE_CMD_A    ('A')
#define TYPE_CMD_A_B     ('B')    /* 本地播放下一曲 */
#define TYPE_CMD_A_C     ('C')    /* 本地播放开始 */
#define TYPE_CMD_A_D     ('D')    /* 本地播放暂停 */
#define TYPE_CMD_A_E     ('E')    /* 本地播放上一曲 */
#define TYPE_CMD_A_L     ('L')    /* 播放本地提示音 */
#define TYPE_CMD_A_M     ('M')    /* 开启MIC回环 */
#define TYPE_CMD_A_N     ('N')    /* 关闭MIC回环 */
#define TYPE_CMD_A_S     ('S')    /* 开启MIC录音 */
#define TYPE_CMD_A_T     ('T')    /* 关闭MIC录音 */
#define TYPE_CMD_A_V     ('V')    /* 耳机播放下一曲 */
#define TYPE_CMD_A_W     ('W')    /* 耳机开始播放 */
#define TYPE_CMD_A_X     ('X')    /* 耳机播放暂停 */
#define TYPE_CMD_A_Y     ('Y')    /* 耳机播放上一曲 */

#define TYPE_CMD_B    ('B')
#define TYPE_CMD_B_R     ('R')    /* 打开手机Siri */
#define TYPE_CMD_B_S     ('S')    /* 关闭手机Siri */

#define TYPE_CMD_C    ('C')
#define TYPE_CMD_C_A     ('A')    /* 接听电话 */
#define TYPE_CMD_C_B     ('B')    /* 挂断电话 */
#define TYPE_CMD_C_C     ('C')    /* 回拨电话 */
#define TYPE_CMD_C_D     ('D')    /* 手机音乐暂停 */
#define TYPE_CMD_C_E     ('E')    /* 手机音乐播放 */
#define TYPE_CMD_C_F     ('F')    /* 手机音乐下一曲 */
#define TYPE_CMD_C_G     ('G')    /* 手机音乐上一曲 */
#define TYPE_CMD_C_H     ('H')    /* 搜索设备 */
#define TYPE_CMD_C_I     ('I')    /* 取消搜索 */
#define TYPE_CMD_C_J     ('J')    /* 连接蓝牙设备 + Param */
#define TYPE_CMD_C_K     ('K')    /* 断开连接 */
#define TYPE_CMD_C_L     ('L')    /* 查询蓝牙状态 */
#define TYPE_CMD_C_N     ('N')    /* 拨打电话 + Param */
#define TYPE_CMD_C_O     ('O')    /* 切换到Source */
#define TYPE_CMD_C_Q     ('Q')    /* 切换到SINK */
#define TYPE_CMD_C_R     ('R')    /* 调节音量 */
#define TYPE_CMD_C_X     ('X')    /* 关闭蓝牙 */
#define TYPE_CMD_C_Y     ('Y')    /* 进入配对 */

#define TYPE_CMD_F    ('F')
#define TYPE_CMD_F_A     ('A')    /* 歌曲详细信息 */
#define TYPE_CMD_F_B     ('B')    /* 歌曲简要信息 */
#define TYPE_CMD_F_C     ('C')    /* 获取歌曲总数目 */
#define TYPE_CMD_F_D     ('D')    /* 获取蓝牙地址 */
#define TYPE_CMD_F_E     ('E')    /* 获取蓝牙版本 */
#define TYPE_CMD_F_F     ('F')    /* 使能事件上报 */
#define TYPE_CMD_F_G     ('G')    /* 关机 */

#define TYPE_CMD_P    ('P')
#define TYPE_CMD_P_C     ('C')    /* 建立PBAP连接 */
#define TYPE_CMD_P_P     ('P')    /* 拉取手机电话本 */
#define TYPE_CMD_P_T     ('T')    /* 断开PBAP连接 */

/* ------------------------------------------------------------------------ */
/* ---------------------------- BLE Command ------------------------------- */
/* ------------------------------------------------------------------------ */
#define TYPE_CMD_L    ('L')
#define TYPE_CMD_L_A     ('A')    /* 开启BLE广播 */
#define TYPE_CMD_L_B     ('B')    /* 关闭BLE广播 */
#define TYPE_CMD_L_C     ('C')    /* BLE发起连接 */
#define TYPE_CMD_L_R     ('R')    /* 读ATT属性值 */
#define TYPE_CMD_L_W     ('W')    /* 写ATT属性值 */

/* Exported parameter ---------------------------------------------------------*/
extern rt_list_t DataPool_List;

extern volatile bool gb_ack_TxStatus;
extern volatile bool gb_ack_RxStatus;

/* Exported functions ---------------------------------------------------------*/
void comm_peripheral_init(void);

void comm_add_cmd(uint8_t fu8_type, uint8_t fu8_index, uint8_t fu8_dataLength, uint8_t *fu8_data);

void comm_transmit_poll(void);



#endif
