
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "pskeys.h"

#include "os_mem.h"
#include "driver_plf.h"
#include "driver_syscntl.h"
#include "driver_flash.h"
#include "driver_wdt.h"
#include "driver_uart.h"

#include "gatt_api.h"
#include "gap_api.h"

#include "user_utils.h"

#include "ota.h"
#include "ota_service.h"



static struct app_otas_status_t
{
    uint8_t read_opcode;
    uint16_t length;
    uint32_t base_addr;
} app_otas_status;
static struct buffed_pkt
{
    uint8_t *buf;
    uint16_t len;   //current length in the buffer
    uint16_t malloced_pkt_num;  //num of pkts
} first_pkt = {0};
static uint8_t first_loop = false;
static uint16_t ota_data_idx;
static bool ota_recving_data = false;
static uint16_t ota_recving_data_index = 0;
static uint16_t ota_recving_expected_length = 0;
static uint8_t *ota_recving_buffer = NULL;

extern uint8_t app_boot_get_storage_type(void);
extern void app_boot_save_data(uint32_t dest, uint8_t *src, uint32_t len);
extern void app_boot_load_data(uint8_t *dest, uint32_t src, uint32_t len);
extern void system_set_cache_config(uint8_t value, uint8_t count_10us);

static uint32_t app_otas_get_curr_firmwave_version(void)
{
    struct pskeys_t *pskeys_a = (struct pskeys_t *)0x01000000;
    if(system_regs->qspi_map_des != 0x01003000)  // part B
    {
        struct pskeys_t *pskeys_b = (struct pskeys_t *)(0x01001000);
        return pskeys_b->fw_version;
    }
    else        // part A
        return pskeys_a->fw_version;
}
static uint32_t app_otas_get_curr_image_base(void)
{
    struct pskeys_t *jump_table_tmp = (struct pskeys_t *)0x01000000;

    uint32_t tone_size = 0;
    for(uint8_t i = 0; i<26; i++)
    {
        tone_size += jump_table_tmp->tone_A_size[i];
        tone_size += jump_table_tmp->tone_B_size[i];
    }
    if(system_regs->qspi_map_des != 0x01003000)  // part B
        //return (0x3000 + jump_table_tmp->image_max_size + tone_size + jump_table_tmp->dsp_max_size);
        return (jump_table_tmp->dsp_code_base + jump_table_tmp->dsp_max_size);
    else    // part A
        return 0x3000;
}
static uint32_t app_otas_get_storage_base(void)
{
    struct pskeys_t *jump_table_tmp = (struct pskeys_t *)0x01000000;

    uint32_t tone_size = 0;
    for(uint8_t i = 0; i<26; i++)
    {
        tone_size += jump_table_tmp->tone_A_size[i];
        tone_size += jump_table_tmp->tone_B_size[i];
    }
    if(system_regs->qspi_map_des != 0x01003000)      //partB, then return partA flash Addr
        return 0x3000;
    else
        return (jump_table_tmp->dsp_code_base + jump_table_tmp->dsp_max_size);
        //return (0x3000 + jump_table_tmp->image_max_size + tone_size + jump_table_tmp->dsp_max_size);  //partA, then return partB flash Addr
}
static uint32_t app_otas_get_image_size(void)
{
    struct pskeys_t *jump_table_tmp = (struct pskeys_t *)0x01000000;
    return jump_table_tmp->image_max_size;
}



__attribute__((section("ram_code"))) static void app_otas_save_data(uint32_t dest, uint8_t *src, uint32_t len)
{
    GLOBAL_INT_DISABLE();

    uint32_t current_qspi_map_src = system_regs->qspi_map_src;
    uint32_t current_map_len = system_regs->qspi_map_len;
    uint32_t current_map_des = system_regs->qspi_map_des;
    
    system_regs->qspi_map_src = 0x01003000;
    system_regs->qspi_map_len = 0;
    system_regs->qspi_map_des = 0x01003000;
    
    //void (*flash_write_)(uint32_t offset, uint32_t length, uint8_t * buffer, uint8_t protect) = (void (*)(uint32_t, uint32_t, uint8_t *, uint8_t))0x00026379;
    //flash_write_(dest, len, src, false);

    flash_write(dest, len, src);

    system_regs->qspi_map_src = current_qspi_map_src;
    system_regs->qspi_map_len = current_map_len;
    system_regs->qspi_map_des = current_map_des;

    GLOBAL_INT_RESTORE();
}

__attribute__((section("ram_code"))) static void app_otas_load_data(uint32_t src, uint8_t *dest, uint32_t len)
{
    GLOBAL_INT_DISABLE();

    uint32_t current_qspi_map_src = system_regs->qspi_map_src;
    uint32_t current_map_len = system_regs->qspi_map_len;
    uint32_t current_map_des = system_regs->qspi_map_des;
    
    system_regs->qspi_map_src = 0x01003000;
    system_regs->qspi_map_len = 0;
    system_regs->qspi_map_des = 0x01003000;
    
    //void (*flash_write_)(uint32_t offset, uint32_t length, uint8_t * buffer, uint8_t protect) = (void (*)(uint32_t, uint32_t, uint8_t *, uint8_t))0x00026379;
    //flash_write_(dest, len, src, false);

    disable_cache();
    flash_read(src, len, dest);
    enable_cache(true);

    system_regs->qspi_map_src = current_qspi_map_src;
    system_regs->qspi_map_len = current_map_len;
    system_regs->qspi_map_des = current_map_des;

    GLOBAL_INT_RESTORE();
}


void ota_clr_buffed_pkt(uint8_t conidx)
{
    //current_conidx = 200;
    if(first_pkt.buf != NULL)
    {
        //system_latency_enable(conidx);
        first_loop = true;
        os_free(first_pkt.buf);
        memset(&first_pkt,0x0,sizeof(first_pkt));
    }
}
void ota_init(uint8_t conidx)
{
    app_otas_status.read_opcode = OTA_CMD_NULL;
    first_loop = true;
    ota_data_idx = 0;
}
void ota_deinit(uint8_t conidx)
{
    ota_clr_buffed_pkt(conidx);

    if(ota_recving_buffer != NULL)
    {
        os_free(ota_recving_buffer);
        ota_recving_buffer = NULL;
    }
}
void __attribute__((weak)) ota_change_flash_pin(void)
{
    ;
}
void __attribute__((weak)) ota_recover_flash_pin(void)
{
    ;
}

void app_otas_recv_data(uint8_t conidx,uint8_t *p_data,uint16_t len)
{
    struct app_ota_cmd_hdr_t *cmd_hdr = (struct app_ota_cmd_hdr_t *)p_data;
    struct app_ota_rsp_hdr_t *rsp_hdr;
    uint16_t rsp_data_len = (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN+OTA_HDR_RESULT_LEN);

    if(first_loop)
    {
        first_loop = false;
        gap_conn_param_update(conidx, 12, 12, 0, 500);
        //system_latency_disable(conidx);
        gatt_mtu_exchange_req(conidx);

        if(ota_recving_buffer == NULL)
        {
            ota_recving_buffer = os_malloc(512);
        }
    }
    printf("app_otas_recv_data[%d]: %d, %d. %d\r\n",ota_data_idx, gatt_get_mtu(conidx), len, cmd_hdr->cmd.write_data.length);
    show_reg(p_data,sizeof(struct app_ota_cmd_hdr_t),1);
    ota_data_idx++;

    // 支持手机端将包从应用层进行拆分的功能，而不是应用层发长包，L2CAP去拆分�?
    if(ota_recving_data)
    {
        memcpy(ota_recving_buffer+ota_recving_data_index, p_data, len);
        ota_recving_data_index += len;
        ota_recving_expected_length -= len;
        if(ota_recving_expected_length != 0)
        {
            return;
        }
        ota_recving_data = false;
        ota_recving_buffer[0] = OTA_CMD_WRITE_DATA;
        p_data = ota_recving_buffer;
        cmd_hdr = (struct app_ota_cmd_hdr_t *)ota_recving_buffer;
    }

    ota_change_flash_pin();
    wdt_feed();

    switch(cmd_hdr->opcode)
    {
        case OTA_CMD_NVDS_TYPE:
            rsp_data_len += 1;
            break;
        case OTA_CMD_GET_STR_BASE:
            ota_data_idx = 0;
            ota_clr_buffed_pkt(conidx);
            rsp_data_len += sizeof(struct storage_baseaddr);
            break;
        case OTA_CMD_READ_FW_VER:
            rsp_data_len += sizeof(struct firmware_version);
            break;
        case OTA_CMD_PAGE_ERASE:
            rsp_data_len += sizeof(struct page_erase_rsp);
            break;
        case OTA_CMD_WRITE_DATA:
            rsp_data_len += sizeof(struct write_data_rsp);
            break;
        case OTA_CMD_READ_DATA:
            rsp_data_len += sizeof(struct read_data_rsp) + cmd_hdr->cmd.read_data.length;
            if(rsp_data_len > OTAS_NOTIFY_DATA_SIZE)
            {
                // 数据太长，不能通过notify返回，通知client采用read方式获取
                rsp_data_len = sizeof(struct read_data_rsp) + (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN+OTA_HDR_RESULT_LEN);
                app_otas_status.read_opcode = OTA_CMD_READ_DATA;
                app_otas_status.length = cmd_hdr->cmd.read_data.length;
                app_otas_status.base_addr = cmd_hdr->cmd.read_data.base_address;
            }
            break;
        case OTA_CMD_WRITE_MEM:
            rsp_data_len += sizeof(struct write_mem_rsp);
            break;
        case OTA_CMD_READ_MEM:
            rsp_data_len += sizeof(struct read_mem_rsp) + cmd_hdr->cmd.read_mem.length;
            if(rsp_data_len > OTAS_NOTIFY_DATA_SIZE)
            {
                // 数据太长，不能通过notify返回，通知client采用read方式获取
                rsp_data_len = sizeof(struct read_data_rsp) + (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN+OTA_HDR_RESULT_LEN);
                app_otas_status.read_opcode = OTA_CMD_READ_MEM;
                app_otas_status.length = cmd_hdr->cmd.read_data.length;
                app_otas_status.base_addr = cmd_hdr->cmd.read_data.base_address;
            }
            else
            {
                app_otas_status.read_opcode = OTA_CMD_NULL;
            }
            break;
        case OTA_CMD_NULL:
            memcpy(ota_recving_buffer, p_data, len);
            ota_recving_expected_length = cmd_hdr->cmd.write_data.length;
            ota_recving_data_index = len;
            ota_recving_data = true;
            ota_recover_flash_pin();
            return;
    }

    struct otas_send_rsp *req = os_malloc(sizeof(struct otas_send_rsp) + rsp_data_len);
    uint16_t base_length;

    req->conidx = conidx;
    req->length = rsp_data_len;
    rsp_hdr = (struct app_ota_rsp_hdr_t *)&req->buffer[0];
    rsp_hdr->result = OTA_RSP_SUCCESS;
    rsp_hdr->org_opcode = cmd_hdr->opcode;
    rsp_hdr->length = rsp_data_len - (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN+OTA_HDR_RESULT_LEN);

    switch(cmd_hdr->opcode)
    {
        case OTA_CMD_NVDS_TYPE:
            rsp_hdr->rsp.nvds_type = app_boot_get_storage_type() | 0x10;    // 0x10 is used to identify FR8010H
            printf("nvds_type:0x%08X\r\n",rsp_hdr->rsp.nvds_type);
            break;
        case OTA_CMD_GET_STR_BASE:
            rsp_hdr->rsp.baseaddr.baseaddr = app_otas_get_storage_base();
            break;
        case OTA_CMD_READ_FW_VER:
            rsp_hdr->rsp.version.firmware_version = pskeys.fw_version;
            break;
        case OTA_CMD_PAGE_ERASE:
        {
            rsp_hdr->rsp.page_erase.base_address = cmd_hdr->cmd.page_erase.base_address;
#if 1
            printf("erase_addr:%x\r\n",rsp_hdr->rsp.page_erase.base_address);
            if( (rsp_hdr->rsp.page_erase.base_address < (app_otas_get_curr_image_base() + app_otas_get_image_size()) )
                && (rsp_hdr->rsp.page_erase.base_address >= app_otas_get_curr_image_base())
              )
            {
                gap_disconnect_req(conidx);
                break;
            }
#endif
            flash_erase(rsp_hdr->rsp.page_erase.base_address, 0x1000);
        }
        break;
        case OTA_CMD_CHIP_ERASE:
            break;
        case OTA_CMD_WRITE_DATA:
        {
            rsp_hdr->rsp.write_data.base_address = cmd_hdr->cmd.write_data.base_address;
            rsp_hdr->rsp.write_data.length = cmd_hdr->cmd.write_data.length;
            printf("write_addr:%x,len:%d\r\n",rsp_hdr->rsp.write_data.base_address,rsp_hdr->rsp.write_data.length);
            app_otas_save_data(rsp_hdr->rsp.write_data.base_address,
                               p_data + (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN)+sizeof(struct write_data_cmd),
                               rsp_hdr->rsp.write_data.length);
        }
        break;
        case OTA_CMD_READ_DATA:
            rsp_hdr->rsp.read_data.base_address = cmd_hdr->cmd.read_data.base_address;
            rsp_hdr->rsp.read_data.length = cmd_hdr->cmd.read_data.length;
            base_length = sizeof(struct read_data_rsp) + (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN+OTA_HDR_RESULT_LEN);
            if(rsp_data_len != base_length)
            {
                app_boot_load_data((uint8_t*)rsp_hdr+base_length,
                                   rsp_hdr->rsp.read_data.base_address,
                                   rsp_hdr->rsp.read_data.length);
            }
            break;
        case OTA_CMD_WRITE_MEM:
            rsp_hdr->rsp.write_mem.base_address = cmd_hdr->cmd.write_mem.base_address;
            rsp_hdr->rsp.write_mem.length = cmd_hdr->cmd.write_mem.length;
            memcpy((void *)rsp_hdr->rsp.write_mem.base_address,
                   p_data + (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN)+sizeof(struct write_data_cmd),
                   rsp_hdr->rsp.write_mem.length);
            break;
        case OTA_CMD_READ_MEM:
            rsp_hdr->rsp.read_mem.base_address = cmd_hdr->cmd.read_mem.base_address;
            rsp_hdr->rsp.read_mem.length = cmd_hdr->cmd.read_mem.length;
            base_length = sizeof(struct read_mem_rsp) + (OTA_HDR_OPCODE_LEN+OTA_HDR_LENGTH_LEN+OTA_HDR_RESULT_LEN);
            if(rsp_data_len != base_length)
            {
                memcpy((uint8_t*)rsp_hdr+base_length,
                       (void *)rsp_hdr->rsp.read_mem.base_address,
                       rsp_hdr->rsp.read_data.length);
            }
            break;
        case OTA_CMD_REBOOT:
            {
                if(first_pkt.buf == NULL)
                    first_pkt.buf = os_malloc(256);
                struct pskeys_t *pskey_tmp = (struct pskeys_t *)(first_pkt.buf);

                if( app_otas_get_curr_image_base() == 0x3000)
                    memcpy(first_pkt.buf,(uint8_t *)0x01000000,256);
                else
                    memcpy(first_pkt.buf,(uint8_t *)0x01001000,256);

                pskey_tmp->image_size = pskey_tmp->image_max_size;      //51500;//
                pskey_tmp->image_base = app_otas_get_storage_base();
                printf("old_ver:%08X\r\n",app_otas_get_curr_firmwave_version());
                pskey_tmp->fw_version  = app_otas_get_curr_firmwave_version() + 1;
                printf("new_ver:%08X\r\n",pskey_tmp->fw_version);

                flash_erase(0x0, 0x1000);
                flash_erase(0x1000, 0x1000);
                if( app_otas_get_curr_image_base() == 0x3000)
                    app_otas_save_data(0x1000,first_pkt.buf,256);
                else
                    app_otas_save_data(0x0,first_pkt.buf,256);

                uart_finish_transfers();
                ota_clr_buffed_pkt(conidx);
                platform_reset(0);
            }
            break;
        default:
            rsp_hdr->result = OTA_RSP_UNKNOWN_CMD;
            break;
    }

    ota_gatt_report_notify(conidx,req->buffer,req->length);
    ota_recover_flash_pin();
    os_free(req);
}



uint16_t app_otas_read_data(uint8_t conidx,uint8_t *p_data)
{
    uint16_t length;
    switch(app_otas_status.read_opcode)
    {
        case OTA_CMD_READ_DATA:
            app_boot_load_data(p_data,app_otas_status.base_addr,app_otas_status.length);
            length = app_otas_status.length;
            break;
        case OTA_CMD_READ_MEM:
            memcpy(p_data, (uint8_t *)app_otas_status.base_addr, app_otas_status.length);
            length = app_otas_status.length;
            break;
        default:
            length = 0;
            break;
    }
    app_otas_status.read_opcode = OTA_CMD_NULL;
    return length;
}

