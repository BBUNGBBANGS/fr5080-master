#include "ipc_load_code.h"
#include "driver_ipc.h"
#include "driver_plf.h"
#include "pskeys.h"
#include "user_ipc.h"

static uint32_t text_src = 0;
static uint32_t text_dst = 0;
static uint32_t data_src = 0;
static uint32_t data_dst = 0;

static uint32_t basic_text_length = 0;
static uint32_t basic_data_length = 0;
static uint32_t nrec_text_length = 0;
static uint32_t nrec_data_length = 0;
static uint32_t aac_text_length = 0;
static uint32_t aac_data_length = 0;

struct dsp_code_store_info_t {
    uint32_t dst;
    uint32_t length;
    uint32_t data[1];
};

void print_mem(uint32_t *ptr, uint8_t count)
{
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
    printf("\r\n");
}

void ipc_get_related_code_len(void)
{
    struct dsp_code_store_info_t *info_ptr;
    info_ptr = (struct dsp_code_store_info_t *)(QSPI_DAC_ADDRESS + pskeys.dsp_code_base);
    basic_text_length = info_ptr->length;

    info_ptr = (struct dsp_code_store_info_t *)(QSPI_DAC_ADDRESS + pskeys.dsp_code_base + sizeof(uint32_t) + sizeof(uint32_t) + basic_text_length);
    basic_data_length = info_ptr->length;

    uint32_t user_code_base_addr = QSPI_DAC_ADDRESS + pskeys.dsp_code_base \
                                                + sizeof(uint32_t) + sizeof(uint32_t) + basic_text_length \
                                                + sizeof(uint32_t) + sizeof(uint32_t) + basic_data_length;

    info_ptr = (struct dsp_code_store_info_t *)user_code_base_addr;
    nrec_text_length = info_ptr->length;

    info_ptr = (struct dsp_code_store_info_t *)(user_code_base_addr + sizeof(uint32_t) + sizeof(uint32_t) + nrec_text_length);
    nrec_data_length = info_ptr->length;

}
void ipc_user_code_action_sent(uint8_t chn)
{
    printf("ipc_user_code_action_sent\r\n");
    //ipc_free_channel(chn);
}

void ipc_load_user_data_done(enum ipc_load_code_result_t result)
{
    printf("ipc_load_user_data_done\r\n");

    ipc_load_code_exec_user_code((void *)0x82000,(void *)ipc_user_code_action_sent);
}

void ipc_load_nrec_code_done(enum ipc_load_code_result_t result)
{
    struct ipc_load_code_cmd_t load_cmd;
    
    printf("ipc_load_user_code_done\r\n");

    load_cmd.reboot_dsp_first = false;
    load_cmd.reboot_dsp_after = false;
    load_cmd.src = (uint8_t *)data_src;
    load_cmd.dst = (uint8_t *)data_dst;
    load_cmd.length = nrec_data_length;
    load_cmd.reboot_vector = 0;

    ipc_load_code(&load_cmd, ipc_load_user_data_done);
}

void ipc_load_aac_code_done(enum ipc_load_code_result_t result)
{
    struct ipc_load_code_cmd_t load_cmd;
    
    printf("ipc_load_user_code_done\r\n");

    load_cmd.reboot_dsp_first = false;
    load_cmd.reboot_dsp_after = false;
    load_cmd.src = (uint8_t *)data_src;
    load_cmd.dst = (uint8_t *)data_dst;
    load_cmd.length = aac_data_length;
    load_cmd.reboot_vector = 0;

    ipc_load_code(&load_cmd, ipc_load_user_data_done);
}


void ipc_load_resident_data_done(enum ipc_load_code_result_t result)
{
    printf("ipc_load_data_done\r\n");
}

void ipc_load_resident_code_done(enum ipc_load_code_result_t result)
{
    struct ipc_load_code_cmd_t load_cmd;
    
    printf("ipc_load_code_done\r\n");

    load_cmd.reboot_dsp_first = false;
    load_cmd.reboot_dsp_after = true;
    load_cmd.src = (uint8_t *)data_src;
    load_cmd.dst = (uint8_t *)data_dst;
    load_cmd.length = basic_data_length;
    load_cmd.reboot_vector = 0x80000;

    ipc_load_code(&load_cmd, ipc_load_resident_data_done);
}


void ipc_load_resident_code(void)
{
    struct ipc_load_code_cmd_t load_cmd;
    struct dsp_code_store_info_t *info_ptr;

    info_ptr = (struct dsp_code_store_info_t *)(QSPI_DAC_ADDRESS + pskeys.dsp_code_base);
    text_src = (uint32_t)&info_ptr->data[0];
    text_dst = info_ptr->dst;
    basic_text_length = info_ptr->length;

    info_ptr = (struct dsp_code_store_info_t *)(QSPI_DAC_ADDRESS + pskeys.dsp_code_base + sizeof(uint32_t) + sizeof(uint32_t) + basic_text_length);
    data_src = (uint32_t)&info_ptr->data[0];
    data_dst = info_ptr->dst;
    basic_data_length = info_ptr->length;

    printf("resident:0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\r\n", text_src, text_dst, basic_text_length, data_src, data_dst, basic_data_length);
    //print_mem((void *)text_src, 0x10);
    //print_mem((void *)data_src, 0x10);

    ipc_get_related_code_len();

    load_cmd.reboot_dsp_first = true;
    load_cmd.reboot_dsp_after = false;
    load_cmd.src = (uint8_t *)text_src;
    load_cmd.dst = (uint8_t *)text_dst;
    load_cmd.length = basic_text_length;
    load_cmd.reboot_vector = 0x80000;

    NVIC_DisableIRQ(IPC_IRQn);
    ipc_load_code(&load_cmd, ipc_load_resident_code_done);
    NVIC_EnableIRQ(IPC_IRQn);
}

void ipc_nrec_start(void)
{
    struct ipc_load_code_cmd_t load_cmd;
    struct dsp_code_store_info_t *info_ptr;

    uint32_t user_code_base_addr = QSPI_DAC_ADDRESS + pskeys.dsp_code_base \
                                                + sizeof(uint32_t) + sizeof(uint32_t) + basic_text_length \
                                                + sizeof(uint32_t) + sizeof(uint32_t) + basic_data_length;

    info_ptr = (struct dsp_code_store_info_t *)user_code_base_addr;
    text_src = (uint32_t)&info_ptr->data[0];
    text_dst = info_ptr->dst;
    //text_length = info_ptr->length;

    info_ptr = (struct dsp_code_store_info_t *)(user_code_base_addr + sizeof(uint32_t) + sizeof(uint32_t) + nrec_text_length);
    data_src = (uint32_t)&info_ptr->data[0];
    data_dst = info_ptr->dst;
    //data_length = info_ptr->length;

    printf("nrec:0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\r\n", text_src, text_dst, nrec_text_length, data_src, data_dst, nrec_data_length);
    //print_mem((void *)text_src, 0x10);
    //print_mem((void *)data_src, 0x10);

    load_cmd.reboot_dsp_first = true;
    load_cmd.reboot_dsp_after = false;
    load_cmd.src = (uint8_t *)text_src;
    load_cmd.dst = (uint8_t *)text_dst;
    load_cmd.length = nrec_text_length;
    load_cmd.reboot_vector = 0x80000;

    ipc_load_code(&load_cmd, ipc_load_nrec_code_done);

}
void ipc_nrec_stop(void)
{
    uint8_t ch;
    printf("ipc nrec stops\r\n");

    /* 分配IPC通道 */
    ch = ipc_alloc_channel(0);
    if(ch == 0xFF) {
        return;
    }
    /* 发送到DSP */
    ipc_insert_msg(ch, IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_DECODER_STOP, NULL);

}
void ipc_aac_start(void)
{
    struct ipc_load_code_cmd_t load_cmd;
    struct dsp_code_store_info_t *info_ptr;

    uint32_t user_code_base_addr = QSPI_DAC_ADDRESS + pskeys.dsp_code_base \
                                                + sizeof(uint32_t) + sizeof(uint32_t) + basic_text_length \
                                                + sizeof(uint32_t) + sizeof(uint32_t) + basic_data_length \
                                                + sizeof(uint32_t) + sizeof(uint32_t) + nrec_text_length \
                                                + sizeof(uint32_t) + sizeof(uint32_t) + nrec_data_length;

    info_ptr = (struct dsp_code_store_info_t *)user_code_base_addr;
    text_src = (uint32_t)&info_ptr->data[0];
    text_dst = info_ptr->dst;
    aac_text_length = info_ptr->length;

    info_ptr = (struct dsp_code_store_info_t *)(user_code_base_addr + sizeof(uint32_t) + sizeof(uint32_t) + aac_text_length);
    data_src = (uint32_t)&info_ptr->data[0];
    data_dst = info_ptr->dst;
    aac_data_length = info_ptr->length;

    printf("aac:0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x, 0x%08x\r\n", text_src, text_dst, aac_text_length, data_src, data_dst, aac_data_length);
    //print_mem((void *)text_src, 0x10);
    //print_mem((void *)data_src, 0x10);

    load_cmd.reboot_dsp_first = true;
    load_cmd.reboot_dsp_after = false;
    load_cmd.src = (uint8_t *)text_src;
    load_cmd.dst = (uint8_t *)text_dst;
    load_cmd.length = aac_text_length;
    load_cmd.reboot_vector = 0x80000;

    ipc_load_code(&load_cmd, ipc_load_aac_code_done);

}

void ipc_aac_stop(void)
{
    uint8_t ch;
    printf("ipc aac stops\r\n");

    /* 分配IPC通道 */
    ch = ipc_alloc_channel(0);
    if(ch == 0xFF) {
        return;
    }
    /* 发送到DSP */
    ipc_insert_msg(ch, IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_DECODER_STOP, NULL);

}

