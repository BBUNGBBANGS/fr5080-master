#include <stdint.h>

#include "os_task.h"
#include "os_msg_q.h"

#include "user_task.h"
#include "app_at.h"

uint16_t user_task_id;

///用户消息处理函数
static int user_task_func(os_event_t *param)
{
    switch(param->event_id)
    {
        ///事件类型为串口AT命令
        case USER_EVT_AT_COMMAND:
            ///消息处理
            app_at_cmd_recv_handler(param->param, param->param_len);
            break;
    }

    ///返回EVT_CONSUMED，LIB对该消息进行释放
    return EVT_CONSUMED;
}

void user_task_init(void)
{
    ///创建用户任务
    user_task_id = os_task_create(user_task_func);
}

