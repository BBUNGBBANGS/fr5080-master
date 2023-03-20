#include <stdint.h>
#include <string.h>
#include "driver_efuse.h"
#include "driver_plf.h"
#include "driver_triming.h"
#include "driver_syscntl.h"

#include "co_math.h"

/*
 * TYPEDEFS (���Ͷ���)
 */
struct efuse_ctrl_t {
    uint32_t go:1;
    uint32_t rd_mode:1;
    uint32_t burn_mode:1;
    uint32_t done:1;
    uint32_t reserved:28;
};

struct efuse_ctrl_len_t {
    uint32_t burn_len:6;
    uint32_t reserved0:2;
    uint32_t avdd_len:6;
    uint32_t reserved1:18;
};

struct efuse_t {
    struct efuse_ctrl_t ctrl;
    uint32_t data[3];
    struct efuse_ctrl_len_t len;
};

volatile static struct efuse_t *efuse_ctrl = (struct efuse_t *)EFUSE_BASE;

/*********************************************************************
 * @fn      efuse_read
 *
 * @brief   read data from efuse
 *
 * @param   buffer  - data buffer
 *			
 * @return  None
 */
void efuse_read(uint32_t *buffer)
{
    system_regs->reset_ctrl.efuse_mas_sft_rst = 1;
    co_delay_10us(1);
    system_regs->reset_ctrl.efuse_mas_sft_rst = 0;

//    efuse_ctrl->ctrl.done = 1;  // clear
    efuse_ctrl->ctrl.burn_mode = 0;
    efuse_ctrl->ctrl.rd_mode = 1;
    efuse_ctrl->ctrl.go = 1;
    co_delay_10us(100);
    while(efuse_ctrl->ctrl.done == 0);
    efuse_ctrl->ctrl.done = 1;  // clear
    memcpy((uint8_t *)buffer, (uint8_t *)&efuse_ctrl->data[0], 3*4);
}

/*********************************************************************
 * @fn      efuse_write
 *
 * @brief   write data to efuse
 *
 * @param   buffer  - data buffer
 *          mask    - indicate which bit to be written: 1 means to be
 *                    written, 0 means keep origin value.
 *			
 * @return  None
 */
void efuse_write(uint32_t *buffer, uint32_t *mask)
{
    uint32_t tmp_buffer[3];

    efuse_read(tmp_buffer);
    tmp_buffer[0] &= (~mask[0]);
    tmp_buffer[0] |= buffer[0] & mask[0];
    tmp_buffer[1] &= (~mask[1]);
    tmp_buffer[1] |= buffer[1] & mask[1];
    tmp_buffer[2] &= (~mask[2]);
    tmp_buffer[2] |= buffer[2] & mask[2];

    memcpy((uint8_t *)&efuse_ctrl->data[0], (uint8_t *)tmp_buffer, 3*4);
    
    efuse_ctrl->len.burn_len = 48;
    efuse_ctrl->ctrl.burn_mode = 1;
    efuse_ctrl->ctrl.rd_mode = 0;
    efuse_ctrl->ctrl.go = 1;
    while(efuse_ctrl->ctrl.done == 0);
    efuse_ctrl->ctrl.done = 1;  // clear
}

void efuse_get_chip_unique_id(struct chip_unique_id_t * id_buff)
{
    uint8_t i = 0;
    uint32_t data[3];
    uint8_t l_id_sub[4];
    uint8_t msb_value, lsb_value;
    uint16_t dc, offset;

    efuse_read(data);
//    printf("%08x, %08x, %08x\r\n", data[0], data[1], data[2]);

    triming_get_avdd_param(&dc, &offset);
    id_buff->unique_id[0] = offset;
    id_buff->unique_id[1] = dc;
    triming_get_internal_ref_param(&dc, &offset);
    id_buff->unique_id[2] = offset;
    id_buff->unique_id[3] = dc;

    l_id_sub[0] = data[0]>>24;
    l_id_sub[1] = (data[1]>>0)&0xff;
    l_id_sub[2] = (data[1]>>8)&0xff;
    l_id_sub[3] = (data[1]>>16)&0xff;

    for(i=0; i<4; i++) {
        msb_value = l_id_sub[i];
        for(uint8_t j=0; j<4; j++) {
            if(msb_value & (1<<j)) {
                lsb_value |= (1<<(3-j));
            }
        }
        l_id_sub[i] = lsb_value;
        if(l_id_sub[i] > 0x39) {
            l_id_sub[i] -= 'A';
        }
        else {
            l_id_sub[i] -= '0';
        }
    }

    id_buff->unique_id[4] = l_id_sub[0] | (l_id_sub[1]<<4);
    id_buff->unique_id[4] = l_id_sub[2] | (l_id_sub[3]<<4);
}

