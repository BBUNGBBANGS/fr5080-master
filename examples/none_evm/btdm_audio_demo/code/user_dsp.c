#include <stdint.h>

#include "pskeys.h"
#include "co_log.h"
#include "os_task.h"
#include "ipc_load_code.h"

#include "driver_plf.h"
#include "driver_syscntl.h"
#include "driver_pmu.h"
#include "driver_ipc.h"

#include "user_config.h"
#include "user_dsp.h"
#include "user_bt.h"
#include "user_task.h"
#include "audio_source.h"
#include "native_playback.h"

struct dsp_code_store_info_t {
    uint32_t dst;
    uint32_t length;
    uint32_t data[1];
};

static uint32_t text_src = 0;
static uint32_t text_dst = 0;
static uint32_t data_src = 0;
static uint32_t data_dst = 0;
static uint32_t text_len = 0;
static uint32_t data_len = 0;

static uint16_t dsp_working_label = 0x00;
static enum dsp_state_t dsp_state = DSP_STATE_CLOSED;
#if DSP_USE_XIP == 0
static enum dsp_load_type_t dsp_load_type;
#endif

static void dsp_get_code_storage_info(enum dsp_load_type_t type)
{
    struct dsp_code_store_info_t *info_ptr;
    enum dsp_load_type_t current_type = DSP_LOAD_TYPE_BASIC;

    info_ptr = (struct dsp_code_store_info_t *)(QSPI_DAC_ADDRESS + pskeys.dsp_code_base);
    while(current_type <= type) {
        text_src = (uint32_t)&info_ptr->data[0];
        text_dst = info_ptr->dst;
        text_len = info_ptr->length;

        if(text_len != 0x70696863) {
            info_ptr = (struct dsp_code_store_info_t *)((uint32_t)info_ptr + sizeof(uint32_t) + sizeof(uint32_t) + text_len);
            data_src = (uint32_t)&info_ptr->data[0];
            data_dst = info_ptr->dst;
            data_len = info_ptr->length;

            info_ptr = (struct dsp_code_store_info_t *)((uint32_t)info_ptr + sizeof(uint32_t) + sizeof(uint32_t) + data_len);
        }
        else {
            text_len = 0;
            info_ptr = (struct dsp_code_store_info_t *)((uint32_t)info_ptr + sizeof(uint32_t) + sizeof(uint32_t));
        }

        current_type++;
    }
}

void dsp_user_code_action_sent(uint8_t chn)
{
    LOG_INFO("ipc_user_code_action_sent\r\n");
}

#if DSP_USE_XIP == 0
static void dsp_load_user_data_done(enum ipc_load_code_result_t result)
{
    LOG_INFO("ipc_load_user_data_done\r\n");

    ipc_load_code_exec_user_code((void *)0x82000,(void *)dsp_user_code_action_sent);
}

static void dsp_load_user_text_done(enum ipc_load_code_result_t result)
{
    LOG_INFO("dsp_load_user_text_done\r\n");
    
    struct ipc_load_code_cmd_t load_cmd;

    load_cmd.reboot_dsp_first = false;
    load_cmd.reboot_dsp_after = false;
    load_cmd.src = (uint8_t *)data_src;
    load_cmd.dst = (uint8_t *)data_dst;
    load_cmd.length = data_len;
    load_cmd.reboot_vector = 0;

    ipc_load_code(&load_cmd, dsp_load_user_data_done);
}
#endif

static void dsp_load_basic_data_done(enum ipc_load_code_result_t result)
{
    LOG_INFO("dsp_load_basic_data_done\r\n");
#if !DSP_USE_XIP
    struct ipc_load_code_cmd_t load_cmd;

    dsp_get_code_storage_info(dsp_load_type);
    if(text_len != 0) {
        load_cmd.reboot_dsp_first = true;
        load_cmd.reboot_dsp_after = false;
        load_cmd.src = (uint8_t *)text_src;
        load_cmd.dst = (uint8_t *)text_dst;
        load_cmd.length = text_len;
        load_cmd.reboot_vector = 0x80000;
        
        ipc_load_code(&load_cmd, dsp_load_user_text_done);
    }
#else
    ipc_load_code_exec_user_code((void *)0x20000000, dsp_user_code_action_sent);
#endif
}

static void dsp_load_basic_text_done(enum ipc_load_code_result_t result)
{
    struct ipc_load_code_cmd_t load_cmd;
    
    LOG_INFO("dsp_load_basic_text_done\r\n");

    load_cmd.reboot_dsp_first = false;
    load_cmd.reboot_dsp_after = true;
    load_cmd.src = (uint8_t *)data_src;
    load_cmd.dst = (uint8_t *)data_dst;
    load_cmd.length = data_len;
#if DSP_USE_XIP
    load_cmd.reboot_vector = 0xa0000;
#else
    load_cmd.reboot_vector = 0x80000;
#endif

    ipc_load_code(&load_cmd, dsp_load_basic_data_done);
}

static void dsp_load_basic_code(void)
{
    struct ipc_load_code_cmd_t load_cmd;

    dsp_get_code_storage_info(DSP_LOAD_TYPE_BASIC);
    LOG_INFO("resident:0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\r\n", text_src, text_dst, text_len, data_src, data_dst, data_len);

    load_cmd.reboot_dsp_first = true;
    load_cmd.reboot_dsp_after = false;
    load_cmd.src = (uint8_t *)text_src;
    load_cmd.dst = (uint8_t *)text_dst;
    load_cmd.length = text_len;
#if DSP_USE_XIP
    load_cmd.reboot_vector = 0xa0000;
#else
    load_cmd.reboot_vector = 0x80000;
#endif

    NVIC_DisableIRQ(IPC_IRQn);
    ipc_load_code(&load_cmd, dsp_load_basic_text_done);
    NVIC_EnableIRQ(IPC_IRQn);
}

/*********************************************************************
 * @fn      dsp_working_label_clear
 *
 * @brief   set dsp working label, dsp should be kept in opened state while any bit is set.
 *
 * @param   mode    - target mode
 *
 * @return  None
 */
void dsp_working_label_set(uint16_t label)
{
    dsp_working_label |= label;
}

/*********************************************************************
 * @fn      dsp_working_label_clear
 *
 * @brief   clear dsp working label, dsp should be closed when all bits are cleared.
 *
 * @param   mode    - target mode
 *
 * @return  true or false
 */
void dsp_working_label_clear(uint16_t label)
{
    dsp_working_label &= (~label);
    if(dsp_working_label == 0){
        dsp_close();
    }
}

/*********************************************************************
 * @fn      dsp_open
 *
 * @brief   used to open dsp with specific mode.
 *
 * @param   type    - target type. @ref dsp_load_type_t
 *
 * @return  @ref dsp_open_result_t
 */
enum dsp_open_result_t dsp_open(enum dsp_load_type_t type)
{
#if DSP_USE_XIP
    if(dsp_state == DSP_STATE_CLOSED) {
        system_reset_dsp_set();
        system_set_dsp_vector(0x40000);
        system_set_dsp_mem_config(SYSTEM_DSP_MEM_CTRL_I32_D384);
        system_set_dsp_clk(SYSTEM_DSP_SRC_312M, 0);
        
        ipc_load_code_init();

        ool_write(PMU_REG_DLDO_CTRL_0, ool_read(PMU_REG_DLDO_CTRL_0)|0x20);//dldo bypass
        pmu_power_on_DSP();
        dsp_load_basic_code();
        ool_write(PMU_REG_DLDO_CTRL_0, ool_read(PMU_REG_DLDO_CTRL_0)&0xdf);

        dsp_state = DSP_STATE_OPENING;

        return DSP_OPEN_PENDING;
    }
    else if(dsp_state == DSP_STATE_OPENING) {
        return DSP_OPEN_PENDING;
    }
#else
    if((dsp_state == DSP_STATE_CLOSED)
        || (dsp_load_type != type)) {
        system_reset_dsp_set();
        system_set_dsp_vector(0x40000);
        system_set_dsp_clk(SYSTEM_DSP_SRC_312M, 0);
        
        ipc_load_code_init();
        
        ool_write(PMU_REG_DLDO_CTRL_0, ool_read(PMU_REG_DLDO_CTRL_0)|0x20);//dldo bypass
        pmu_power_on_DSP();
        dsp_load_basic_code();
        ool_write(PMU_REG_DLDO_CTRL_0, ool_read(PMU_REG_DLDO_CTRL_0)&0xdf);

        dsp_state = DSP_STATE_OPENING;
        dsp_load_type = type;

        return DSP_OPEN_PENDING;
    }
    else if((dsp_state == DSP_STATE_OPENING)&&(dsp_load_type == type)) {
        return DSP_OPEN_PENDING;
    }
#endif

    return DSP_OPEN_SUCCESS;
}

/*********************************************************************
 * @fn      dsp_close
 *
 * @brief   used to close dsp to save power.
 *
 * @param   None
 *
 * @return  None
 */
void dsp_close(void)
{
    pmu_power_off_DSP();
    ipc_msg_flush();

    dsp_working_label = 0;
    dsp_state = DSP_STATE_CLOSED;

    audio_source_statemachine(USER_EVT_DSP_CLOSED, NULL);
    native_playback_statemachine(USER_EVT_DSP_CLOSED, NULL);
    bt_statemachine(USER_EVT_DSP_CLOSED, NULL);
}

/*********************************************************************
 * @fn      dsp_get_state
 *
 * @brief   get current dsp state.
 *
 * @param   None
 *
 * @return  @ref dsp_state_t
 */
enum dsp_state_t dsp_get_state(void)
{
    return dsp_state;
}

/*********************************************************************
 * @fn      ipc_user_rx_int
 *
 * @brief   used to handle received IPC message from dsp side.
 *
 * @param   msg     - message pointer
 *          chn     - ipc channel index
 *
 * @return  None
 */
__attribute__((section("ram_code"))) void ipc_user_rx_int(struct ipc_msg_t *msg, uint8_t chn)
{
    os_event_t event;

    switch(msg->format) {
        case IPC_MSG_ENCODED_SBC_FRAME:
            {
                uint32_t param[2];

                param[0] = (uint32_t)ipc_get_buffer_offset(IPC_DIR_DSP2MCU, chn);
                param[1] = msg->length;

                audio_source_statemachine(USER_EVT_NEW_SBC_FRAME, (void *)&param[0]);
            }
            break;
        case IPC_MSG_RAW_BUFFER_SPACE:
            //uart_putc_noint_no_wait('~');
            //printf("raw space = %d\r\n",msg->length * 256);
            {
                event.event_id = USER_EVT_REQ_RAW_DATA;
                event.param = NULL;
                event.param_len = msg->length;
                os_msg_post(user_task_id, &event);
            }
            break;
        case IPC_MSG_WITHOUT_PAYLOAD:
            if(msg->length == IPC_SUM_MSG_DSP_USER_CODE_READY) {
                dsp_state = DSP_STATE_OPENED;
                event.event_id = USER_EVT_DSP_OPENED;
                event.param = NULL;
                event.param_len = 0;
                os_msg_post(user_task_id, &event);
            }
            else if(msg->length == IPC_SUB_MSG_DECODER_PREP_READY) {
                event.event_id = USER_EVT_DECODER_PREPARE_NEXT_DONE;
                event.param = NULL;
                event.param_len = 0;
                os_msg_post(user_task_id, &event);
            }
            break;
        case IPC_MSG_ENCODED_MSBC_FRAME:
            {
//                uint8_t *param;
//                char *hex2ascii = "0123456789abcdef";
//
//                param = (void *)ipc_get_buffer_offset(IPC_DIR_DSP2MCU, chn);
//                param += 2;
//                for(uint8_t i=0; i<msg->length-2; i++) {
//                    uint8_t data = *param++;
//                    uart_putc_noint(hex2ascii[data>>4]);
//                    uart_putc_noint(hex2ascii[data&0x0f]);
//                }
//                uart_putc_noint('\r');
//                uart_putc_noint('\n');
            }
            break;
        default:
            break;
    }
}

