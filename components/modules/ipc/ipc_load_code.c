#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define CFG_CPU_CORTEX_M3
#include "arch.h"
#include "os_timer.h"
#include "os_task.h"
#include "user_task.h"

#include "driver_ipc.h"
#include "driver_syscntl.h"

#include "ipc_load_code.h"

#define IPC_LOAD_CODE_RST_DSP_MAGIC         0xFFFFFFFF      // 如果reset_vector配置为该值表示加载完不重启
#define IPC_LOAD_CODE_FRAME_LENGTH          (512-sizeof(struct ipc_load_code_header_t))

enum ipc_load_code_state_t {
    IPC_LOAD_CODE_STATE_IDLE,
    IPC_LOAD_CODE_WAIT_DSP_BOOT,
    IPC_LOAD_CODE_LOADING,
    IPC_LOAD_CODE_REBOOT_DSP,
};

struct ipc_load_code_env_t {
    uint8_t state;
    uint8_t sending_pkt_cnt;
    uint8_t *curr_dst;
    uint8_t *curr_src;
    uint32_t total_length;
    uint32_t curr_length;
    uint32_t reset_vector;
    ipc_load_code_done_callback callback;
};

struct ipc_load_code_header_t {
    uint32_t dst_addr;
};

struct ipc_exec_user_code_header_t {
    uint32_t *entry_ptr;
};

struct ipc_load_code_env_t ipc_load_code_env;
//static os_timer_t ipc_load_code_trans_following_timer;

static void ipc_load_code_send_frame(void);

__attribute__((weak)) void ipc_user_rx_int(struct ipc_msg_t *msg, uint8_t chn)
{
    
}

__attribute__((section("ram_code"))) void ipc_load_code_trans_following(void *arg)
{
    ipc_load_code_done_callback callback;
    
    ipc_load_code_env.sending_pkt_cnt--;
    
    if(ipc_load_code_env.curr_length < ipc_load_code_env.total_length) {
        ipc_load_code_send_frame();
    }
    else {
        if(ipc_load_code_env.sending_pkt_cnt == 0) {
            if(ipc_load_code_env.reset_vector != IPC_LOAD_CODE_RST_DSP_MAGIC) {
                ipc_load_code_env.state = IPC_LOAD_CODE_REBOOT_DSP;
                system_set_dsp_vector(ipc_load_code_env.reset_vector);
                system_reset_dsp();
            }
            else {
                callback = ipc_load_code_env.callback;
                ipc_load_code_env.state = IPC_LOAD_CODE_STATE_IDLE;
                callback(IPC_LOAD_CODE_SUCCESS);
            }
        }
    }
}

/*********************************************************************
 * @fn      ipc_load_code_exec_user_code
 *
 * @brief   通过IPC加载完用户程序后，调用该函数执行用户程序的初始化函数。
 *
 * @param   entry_ptr   - 用户程序入口函数地址的存放位置
 *
 * @return  None
 */
void ipc_load_code_exec_user_code(uint32_t *entry_ptr, void *callback)
{
    struct ipc_exec_user_code_header_t hdr;

    hdr.entry_ptr = entry_ptr;
    ipc_msg_with_payload_send(IPC_MSG_EXEC_USER_CODE, (void *)&hdr, sizeof(hdr), NULL, 0, (ipc_tx_callback)callback);
}

/*********************************************************************
 * @fn      ipc_load_code_rx_int
 *
 * @brief   初始化ipc时注册的接收中断后调函数，用于处理DSP发给MCU的消息。
 *
 * @param   None
 *
 * @return  None
 */
__attribute__((section("ram_code"))) static void ipc_load_code_rx_int(struct ipc_msg_t *msg, uint8_t chn)
{
    GLOBAL_INT_DISABLE();
    
    
    //gpio_set_pin_value(GPIO_PORT_B, GPIO_BIT_0, 1);
    //printf("ipc_load_code_rx_int: %d.\r\n", msg->format);
    switch(msg->format) {
        case IPC_MSG_DSP_READY:
            
            ipc_clear_msg(chn);
            if(ipc_load_code_env.state == IPC_LOAD_CODE_WAIT_DSP_BOOT) {
                ipc_load_code_send_frame();
            }
            
            else if(ipc_load_code_env.state == IPC_LOAD_CODE_REBOOT_DSP) {
                ipc_load_code_done_callback callback;
                callback = ipc_load_code_env.callback;
                ipc_load_code_env.state = IPC_LOAD_CODE_STATE_IDLE;
                callback(IPC_LOAD_CODE_SUCCESS);
            }
            break;
        case IPC_MSG_LOAD_CODE_DONE:
            
            ipc_clear_msg(chn);
            //printf("IPC_MSG_LOAD_CODE_DONE, %d\r\n", chn);
            // use diffrent method to send next frame, this operation has to do outside of interrupt handler
            //os_timer_start(&ipc_load_code_trans_following_timer, 10, false);
            {
                os_event_t send_event;
                send_event.event_id = USER_EVT_SEND_FOLLOWING_DSP_CODE;///event id
                send_event.param = NULL;
                send_event.param_len = 0;
                os_msg_post(user_task_id, &send_event);   ///发送消息
            }
            break;
        default:
            ipc_user_rx_int(msg, chn);
            ipc_clear_msg(chn);
            break;
    }
    

    //gpio_set_pin_value(GPIO_PORT_B, GPIO_BIT_0, 0);
    GLOBAL_INT_RESTORE();
}

/*********************************************************************
 * @fn      ipc_load_code_tx_int
 *
 * @brief   MCU向DSP发送消息成功后收到的中断处理。在这里如果有数据仍要发送，
 *          就继续发送，如果没有数据则判断是不是所有的包都已经发送完成，发送
 *          完成之后复位DSP（如果需要），否则就直接调用回调函数上报加载完毕。
 *
 * @param   None
 *
 * @return  None
 */
__attribute__((section("ram_code"))) static void ipc_load_code_tx_int(uint8_t chn)
{
    //printf("ipc_load_code_tx_int\r\n");
    #if 0
    ipc_load_code_done_callback callback;

    //printf("ipc_load_code_tx_int\r\n");
    
    ipc_load_code_env.sending_pkt_cnt--;
    
    if(ipc_load_code_env.curr_length < ipc_load_code_env.total_length) {
        ipc_load_code_send_frame();
    }
    else {
        if(ipc_load_code_env.sending_pkt_cnt == 0) {
            if(ipc_load_code_env.reset_vector != IPC_LOAD_CODE_RST_DSP_MAGIC) {
                ipc_load_code_env.state = IPC_LOAD_CODE_REBOOT_DSP;
                system_set_dsp_vector(ipc_load_code_env.reset_vector);
                system_reset_dsp();
            }
            else {
                callback = ipc_load_code_env.callback;
                ipc_load_code_env.state = IPC_LOAD_CODE_STATE_IDLE;
                callback(IPC_LOAD_CODE_SUCCESS);
            }
        }
    }
    #endif
}

/*********************************************************************
 * @fn      ipc_load_code_send_frame
 *
 * @brief   发送一帧数据到DSP端，该函数可能会在调用ipc_load_code时和发送成功中断中调用
 *
 * @param   None
 *
 * @return  None
 */
static void ipc_load_code_send_frame(void)
{
    uint32_t frame_length;
    struct ipc_load_code_header_t hdr;
    CPU_SR cpu_sr;

    /* 中断中也有可能调用该函数，所以这里加上中断保护 */
    GLOBAL_INT_OLD_DISABLE();
    
    frame_length = ipc_load_code_env.total_length - ipc_load_code_env.curr_length;
    if(frame_length > IPC_LOAD_CODE_FRAME_LENGTH) {
        frame_length = IPC_LOAD_CODE_FRAME_LENGTH;
    }

    /* 发送数据给到DSP */
    hdr.dst_addr = (uint32_t)ipc_load_code_env.curr_dst;
    ipc_msg_with_payload_send(IPC_MSG_LOAD_CODE, (void *)&hdr, sizeof(hdr), ipc_load_code_env.curr_src, frame_length, ipc_load_code_tx_int);

    ipc_load_code_env.state = IPC_LOAD_CODE_LOADING;

    ipc_load_code_env.curr_length += frame_length;
    ipc_load_code_env.curr_src += frame_length;
    ipc_load_code_env.curr_dst += frame_length;
    ipc_load_code_env.sending_pkt_cnt++;
    GLOBAL_INT_OLD_RESTORE();
}

/*********************************************************************
 * @fn      ipc_load_code_init
 *
 * @brief   初始化用到的环境变量
 *
 * @param   None
 *
 * @return  None
 */
void ipc_load_code_init(void)
{
    memset((void *)&ipc_load_code_env, 0, sizeof(ipc_load_code_env));
    
    //os_timer_init(&ipc_load_code_trans_following_timer, ipc_load_code_trans_following, NULL);

    ipc_init(IPC_MSG_INT_ALL, ipc_load_code_rx_int);
    *(volatile uint8_t *)0x20001600 &= 0xfd;
}

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
bool ipc_load_code(struct ipc_load_code_cmd_t *cmd, ipc_load_code_done_callback callback)
{
    if((ipc_load_code_env.state != IPC_LOAD_CODE_STATE_IDLE)
        || (cmd->length == 0)) {
        return false;
    }

    memset((void *)&ipc_load_code_env, 0, sizeof(ipc_load_code_env));
    ipc_load_code_env.total_length = cmd->length;
    ipc_load_code_env.curr_dst = cmd->dst;
    ipc_load_code_env.curr_src = cmd->src;
    ipc_load_code_env.callback = callback;
    if(cmd->reboot_dsp_after) {
        ipc_load_code_env.reset_vector = cmd->reboot_vector;
    }
    else {
        ipc_load_code_env.reset_vector = IPC_LOAD_CODE_RST_DSP_MAGIC;
    }
    
    if(cmd->reboot_dsp_first) {
        ipc_load_code_env.state = IPC_LOAD_CODE_WAIT_DSP_BOOT;
        // reboot dsp
        system_reset_dsp_release();
        system_reset_dsp();
    }
    else {
        ipc_load_code_send_frame();
    }

    return true;
}

