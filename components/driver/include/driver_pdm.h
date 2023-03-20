#ifndef _DRIVER_PDM_H
#define _DRIVER_PDM_H
#include "type_lib.h"
#include <stdint.h>           // standard integer functions
/*interrupt enable reg*/
#define bmINTEN_LFFFULLINTEN       BIT0
#define bmINTEN_LFFAFULLINTEN      BIT1
#define bmINTEN_LFFEMPTYINTEN      BIT2
#define bmINTEN_RFFFULLINTEN       BIT3
#define bmINTEN_RFFAFULLINTEN      BIT4
#define bmINTEN_RFFEMPTYINTEN      BIT5

struct pdm_config_t{
    uint32_t enable:1;
    uint32_t mono:1;
    uint32_t left_rising:1;
    uint32_t sr_mode:1;
    uint32_t hpf_en:1;
    uint32_t zerodet_en:1;
    uint32_t lfifo_apb_sel:1;
    uint32_t rfifo_apb_sel:1;
    uint32_t lfifo_en:1;
    uint32_t rfifo_en:1;
    uint32_t esco_mask:1;
    uint32_t rsv:21;
};

struct pdm_vol_gain_t{
    uint32_t volgain_l:12;
    uint32_t rsv0:4;
    uint32_t volgain_r:12;
    uint32_t rsv1:4;
};

struct pdm_volstep_t{
    uint32_t vol_step:10;
    uint32_t rsv0:6;
    uint32_t vol_dir:1;
    uint32_t rsv1:15;
};

struct pdm_fifo_clr{
    uint32_t left_wr_clr:1;
    uint32_t left_rd_clr:1;
    uint32_t right_wr_clr:1;
    uint32_t right_rd_clr:1;
    uint32_t rsv:28;
}; 

struct pdm_fifo_status{
    uint32_t left_full:1;
    uint32_t left_afull:1;
    uint32_t left_empty:1;
    uint32_t right_full:1;
    uint32_t right_afull:1;
    uint32_t right_empty:1;
    uint32_t rsv:26;
};

struct pdm_fifo_aflr{
    uint32_t left_aflr:6;
    uint32_t right_aflr:6;
    uint32_t rsv:20;
};


struct pdm_reg_t{
    struct pdm_config_t cfg;        //0x00
    struct pdm_vol_gain_t volgain;  //0x04
    struct pdm_volstep_t volstep;   //0x08
    uint32_t pdm_ldata; // low 16bit    //0x0c
    uint32_t pdm_rdata; // low 16bit    //0x10
    struct pdm_fifo_clr clr;            //0x14
    struct pdm_fifo_status status;      //0x18
    uint32_t pdm_fifo_inten;            //0x1c
    struct pdm_fifo_aflr aflr;          //0x20
};


void  pdm_init(uint8_t index);
void pdm_stop(uint8_t index);
void  pdm_rfifo_wptr_clr(uint8_t index);
void  pdm_rfifo_rptr_clr(uint8_t index);
void  pdm_lfifo_wptr_clr(uint8_t index);
void  pdm_lfifo_rptr_clr(uint8_t index);

void  pdm_volume_ctrl(uint8_t index,uint32_t stepctrl, uint32_t volgain_r, uint32_t volgain_l);


#endif

