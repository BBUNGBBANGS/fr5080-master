#ifndef _IPC_LOAD_CODE_H
#define _IPC_LOAD_CODE_H
#include <stdint.h>
#include <stdbool.h>

enum ipc_load_code_result_t {
    IPC_LOAD_CODE_SUCCESS,
    IPC_LOAD_CODE_FAILED,
};

struct ipc_load_code_cmd_t {
    bool reboot_dsp_first;  // 开始加载前是否需要给dsp重新上电
    bool reboot_dsp_after;  // 加载完成之后是否要重启dsp
    uint8_t *src;           // 加载数据源
    uint8_t *dst;           // 加载到哪里
    uint32_t length;        // 加载数据长度
    uint32_t reboot_vector; // 加载完成之后如果需要重启，重启后的复位地址
};

typedef void (*ipc_load_code_done_callback)(enum ipc_load_code_result_t);

/*********************************************************************
 * @fn      ipc_load_code_exec_user_code
 *
 * @brief   通过IPC加载完用户程序后，调用该函数执行用户程序的初始化函数。
 *
 * @param   entry_ptr   - 用户程序入口函数地址的存放位置
 *
 * @return  None
 */
void ipc_load_code_exec_user_code(uint32_t *entry_ptr,void *callback);

/*********************************************************************
 * @fn      ipc_load_code_init
 *
 * @brief   初始化用到的环境变量
 *
 * @param   None
 *
 * @return  None
 */
void ipc_load_code_init(void);

/*********************************************************************
 * @fn      ipc_load_code
 *
 * @brief   用于加载程序到DSP端
 *
 * @param   cmd         - 加载过程中用到的一些参数，参考ipc_load_code_cmd_t结构体.
 *          callback    - 加载完成之后的回调函数
 *
 * @return  false-参数或者状态不正确，true-开始执行加载动作。
 */
bool ipc_load_code(struct ipc_load_code_cmd_t *cmd, ipc_load_code_done_callback callback);

#endif  // _IPC_LOAD_CODE_H

