#include <stdint.h>

#include "os_task.h"
#include "os_msg_q.h"

#include "user_task.h"
#include "app_at.h"

uint16_t user_task_id;

static int user_task_func(os_event_t *param)
{
    switch(param->event_id)
    {
        case USER_EVT_AT_COMMAND:
            app_at_cmd_recv_handler(param->param, param->param_len);
            break;
    }

    return EVT_CONSUMED;
}

void user_task_init(void)
{
    user_task_id = os_task_create(user_task_func);
}

