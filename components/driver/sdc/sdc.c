#include <string.h>

#include "otg_reg.h"
#include "sdc.h"
#include "ff.h"

#define SDC_OCR_CARD_NO_BUSY_MASK 	0x80000000
#define SDC_OCR_CARD_CCS_MASK		0x40000000
#define FIFO_DEPTH	16

#define SDC_BUS_RESET_STATUS   0
#define SDC_BUS_WORK_STATUS   1

extern void audio_decoder_Stop(void);

  extern uint8 sdc_bus_status;
//static uint32_t uSDC_DMA_Enable = 0;
//static uint32_t pSDC_DMA_DESC[4];

volatile SDSTATUS  Card_Status;
static SDMEM_T g_sdmem_info;
static SDC_HOST_T g_sdc_host;
#if _SDC_IDMA_FLAG
static uint8_t g_dma_start;
#endif
extern FATFS Fatfs_buf[2];		/* File system object for each logical drive */
/*
void SDC_Handler()
{
}
*/
RES_T sdc_card_detect()
{
    if(Card_Status == SD_IN)
        return RES_SUCCESS;
    else
        return RES_SD_ERR_NO_CARD;

}
void sdc_card_get_id(uint8_t id[5])
{

    id[0] = g_sdmem_info.cid[1] >> 24;
    id[1] = g_sdmem_info.cid[2] >> 0;
    id[2] = g_sdmem_info.cid[2] >> 8;
    id[3] = g_sdmem_info.cid[2] >> 16;
    id[4] = g_sdmem_info.cid[2] >> 24;


}
RES_T sdc_card_get_wrtprt()
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    if(REGW(&psdio->wrt_prt) & 1)
        return RES_SD_ERR_WRITEPROTECT;
    else
        return RES_SUCCESS;
}

void sdc_cfg_card_width(uint32_t is_4bit)
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    /*if 0, 1-bit mode, else 4-bit mode*/
    REGW(&psdio->c_type.v) = is_4bit;
}

void sdc_internel_reset(uint32_t dma_res, uint32_t fifo_res, uint32_t ctler_res)
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    SDC_CONTROL_U ctrl;
    ctrl.v = psdio->ctrl.v;

    if (dma_res)
    {
        SDC_BMOD_U bmod;
        bmod.v = REGW(&psdio->bus_mode.v);

        ctrl.bit_info.dam_reset = 1;
        REGW(&psdio->ctrl.v) = ctrl.v;
        /*wait for dma reset auto-cleared*/
        while (REGW(&psdio->ctrl.v) & 4);

        bmod.bit_info.swr = 1;
        REGW(&psdio->bus_mode.v) = bmod.v;
        /*wait for software reset auto-cleared*/
        while (REGW(&psdio->bus_mode.v) & 1);
    }

    if (fifo_res)
    {
        ctrl.bit_info.fifo_reset = 1;
        REGW(&psdio->ctrl.v) = ctrl.v;
        /*wait for fifo reset auto-cleared*/
        while (REGW(&psdio->ctrl.v) & 2);
    }

    if (ctler_res)
    {
        ctrl.bit_info.controller_reset = 1;
        REGW(&psdio->ctrl.v) = ctrl.v;
        /*wait for controller reset auto-cleared*/
        while (REGW(&psdio->ctrl.v) & 1);
    }
    return;
}

void  sdc_cfg_dma_mui_trans_size(uint32_t burst_size)
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    SDC_FIFOTH_U fifo_th;
    fifo_th.v = REGW(&psdio->fifo_th.v);
    fifo_th.bit_info.dw_dma_mul_trans_size = burst_size;
    REGW(&psdio->fifo_th.v) = fifo_th.v;
    return;
}

uint32_t sdc_bus_wait_program_done()
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
	uint32 i=0;
    while(REGW(&psdio->status.v)&(SDC_STATUS_DAT_BUSY_MASK | SDC_STATUS_DAT_MC_BUSY_MASK))
	{//timeout.
			if(i++>=0x7ffffff0)
		break;
	}

    return 0;
}

void sdc_cfg_clken(uint32_t is_low_power, uint32_t clk_en)
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    SDC_CLKENA_U clk_ena;

    clk_ena.bit_info.clk_enable = clk_en;
    clk_ena.bit_info.clk_low_power_en = is_low_power;
    REGW(&psdio->clk_ena.v) = clk_ena.v;
    return;
}

#pragma push
#pragma O0
uint32_t  sdc_is_cmd_accept()
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;

    if(REGW(&psdio->cmd.v) & 0x80000000)
        return 0;

    return 1;
}
#pragma pop
uint32_t sdc_get_status()
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    return REGW(&psdio->status.v);

}
#pragma push
#pragma O0
uint32_t sdc_get_raw_int_status()
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    return REGW(&psdio->rint_sts.v);
}

void sdc_bus_update_clk()
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
	uint32 i;
	i=0;
    while (1)
    {
        REGW(&psdio->cmd.v) = SDC_CMD_DUMMY | 0x80000000;
        while (1)
        {//for timeout.
			if(i++>=0x7ffffff0)
				return;
            if (sdc_is_cmd_accept())
                return;
            if (sdc_get_raw_int_status() & SDC_INT_HLE)
                break;
        }
    }
}
#pragma pop
void sdc_set_clk_division(uint8_t val)
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    REGW(&psdio->clk_div.v) = val;
    return;
}

void sdc_set_clk_src( uint32_t source_num)
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    REGW(&psdio->clk_src) = source_num & 0x3;
    return;
}

void sdc_bus_set_clk(uint32_t clk_div)
{
    g_sdc_host.clk_div = clk_div;
    sdc_bus_wait_program_done();

    sdc_cfg_clken(0, 0);

    sdc_bus_update_clk();

    sdc_set_clk_division(clk_div);
    sdc_set_clk_src( SDC_CLKSRC_CLKDIV0);
    sdc_bus_update_clk();

    sdc_cfg_clken( 0, 1);
    sdc_bus_update_clk();
    return;
}

uint32_t sdc_bus_get_clk_div()
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    uint32_t div;

    div = REGW(&psdio->clk_div.v) & 0xff;

    if (div == 0)
        div =  1;
    else
        div = 2 * div;

    return  div;
}


void sdc_bus_config(uint32_t clk_div, uint32_t sdc_timeout)
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;

    /*clear interrupt status*/
    REGW(&psdio->rint_sts.v) = 0xffffffff;

    sdc_bus_set_clk( clk_div);

    if(sdc_timeout == 0)
    {
        sdc_timeout = sdc_bus_get_clk_div();
        sdc_timeout = sdc_timeout * 30;
    }

    /*rsp_tmout is 0xa0, and data_tmout is sdc_timeout*/
#if 0 //fpga 0224 version
    REGW(&psdio->tm_out.v) = (sdc_timeout << 8) | 0xb0;
#else
    REGW(&psdio->tm_out.v) = (sdc_timeout << 8) | 0xe0;
#endif
}


void sdc_bus_init()
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    SDC_FIFOTH_U fifo_th;

    /*power on for card 0*/
    REGW(&psdio->pwr_en) = 0;
    REGW(&psdio->pwr_en) = 1;

    /*clear interrupt status*/
    REGW(&psdio->rint_sts.v) = 0xffffffff;

    /*cfg card width to 1bit mode*/
    REGW(&psdio->c_type.v) = 0;

#if  _SDC_IDMA_FLAG
    sdc_internel_reset(1, 1, 1);
#else
    sdc_internel_reset(0, 1, 1);
#endif

    /*set debounce counter*/
    REGW(&psdio->debounce_count) = 0x800000;

    sdc_cfg_dma_mui_trans_size(0x00);

    /*set fifo threshold watermarklevel*/
    fifo_th.v = REGW(&psdio->fifo_th.v);
    fifo_th.bit_info.tx_wmark = FIFO_DEPTH / 2;
    fifo_th.bit_info.rx_wmark = (FIFO_DEPTH / 2) - 1;
    REGW(&psdio->fifo_th.v) = fifo_th.v;

}

uint32_t sdc_bus_wait_cmd_accept()
{
    while (!sdc_is_cmd_accept())
    {
        if (sdc_get_raw_int_status() & SDC_INT_HLE)
            return 0;
    }
    return 1;
}

uint32_t sdc_get_cmd_fms()
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    SDC_STATUS_U stat;
    stat.v = REGW(&psdio->status.v);
    return stat.bit_info.cmd_fms_states;
}

uint32_t sdc_bus_wait_cmd_done(uint32_t timeout)
{
    uint32_t sdc_status;
    uint32_t sdc_div;

    sdc_div = sdc_bus_get_clk_div();

    /* Command Timeout = Command+Ncr+Respons = 48 + 64 + Max{48, 128}  < 300 */
    timeout = sdc_div * 3000;

    while(timeout-- && sdc_get_cmd_fms())
    {
        /*Check if response_timeout error, response_CRC error, or response error
        is set. This can be done
        	either by responding to an interrupt raised by these errors or by polling
        bits 1, 6, and 8 from the
        	RINTSTS register*/
    }
    sdc_status = sdc_get_raw_int_status();
    return sdc_status;
}
#pragma push
#pragma O0
uint32_t sdc_bus_send_cmd(uint32_t cmd_code, uint32_t cmd_arg)
{
    uint32_t status;
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
	uint32 i;
	i=0;
    do
    {
        /*clear CD and */
        REGW(&psdio->rint_sts.v) = (1 << 2) | (1 << 8);
        /*set cmd arg*/
        REGW(&psdio->cmd_arg) = cmd_arg;
        /*set and issue a cmd*/
        REGW(&psdio->cmd.v) = cmd_code | 0x80000000;
        if (sdc_bus_wait_cmd_accept())
            break;
			if(i++>=0x7ffffff0)
			break;//for timeout.			
    }
    while (1);
    status = sdc_bus_wait_cmd_done(0);
    return status;
}
#pragma pop
uint32_t sdc_get_short_response()
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    return REGW(&psdio->rsp0);
}

void sdc_get_long_response(uint32_t *p_response)
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    *p_response++ = REGW(&psdio->rsp0);
    *p_response++ = REGW(&psdio->rsp1);
    *p_response++ = REGW(&psdio->rsp2);
    *p_response = REGW(&psdio->rsp3);
}

RES_T sdc_mmc_card_init(SDMEM_T *p_sdmem)
{
    uint32_t status;
    uint32_t timeout_count;
    SDC_HOST_T *p_sd_host;
    p_sd_host = p_sdmem->p_sd_host;
    p_sdmem->bus_width = 0;
    do
    {

#if 0 //fpga 0224 version need following line
        sdc_bus_init();
#endif
        if (p_sd_host->dma_flow_ctrl > 0)
            sdc_cfg_dma_mui_trans_size( DMA_BURST_SIZE_8);

        /*For example, if cclk_in is 32 Mhz, then the value is 32,000/(2*400) = 40.*/
#if 0 //fpga 0224 version need following line
        sdc_bus_config(p_sd_host->clk_div, p_sd_host->timeout);
#endif

        /* default value */
        p_sdmem->card_type = CARD_TYPE_MMC;//mmc card
        status = sdc_bus_send_cmd(SDC_CMD0, 0);
        if (status & SDC_INT_RTO)
            break;

        /* sd card soft reset */
        timeout_count = 100;
        do
        {
            timeout_count--;
            status = sdc_bus_send_cmd(SDC_CMD1 , MMC_CMDARG_CMD1);
            if (status & SDC_INT_RTO)
                return RES_SD_ERR_CMD_BUSY;
            p_sdmem->ocr = sdc_get_short_response();
            if((p_sdmem->ocr & SDC_OCR_CARD_NO_BUSY_MASK))
            {
                if (p_sdmem->ocr & SDC_OCR_CARD_CCS_MASK)
                {
                    p_sdmem->is_high_capability = 1;
                }
                break;
            }
            if (SDC_INT_RTO & status)
                continue;
        }
        while(timeout_count);

        if (status & SDC_INT_RTO)
            break;

        /* read CID CMD2*/
        status = sdc_bus_send_cmd(SDC_CMD2 , 0);
        if (status & SDC_INT_RTO)
            break;
#define NEED_SD_CARD_ID
#ifdef NEED_SD_CARD_ID
        sdc_get_long_response(&p_sdmem->cid[0]);
#endif

        /* idle to standby CMD3 read sca */
        status = sdc_bus_send_cmd(SDC_CMD3 , 0);
        if (status & SDC_INT_RTO)
            break;

        p_sdmem->rca = 0xFFFF0000 & sdc_get_short_response();

        /* CMD 9 read CSR CMD9*/
        status = sdc_bus_send_cmd(SDC_CMD9 , p_sdmem->rca );
        if (status & SDC_INT_RTO)
            break;

        sdc_get_long_response(&p_sdmem->csd[0]);
        if(p_sdmem->is_high_capability)
        {
            p_sdmem->block_len = 512;
            p_sdmem->csd_c_size =
                ((p_sdmem->csd[2] & 0x3f) << 16) | p_sdmem->csd[1] >> 16;
            p_sdmem->block_count = (p_sdmem->csd_c_size + 1) * 1024;
        }
        else
        {
            p_sdmem->csd_read_bl_len = (p_sdmem->csd[2] & 0x000f0000) >> 16;
            p_sdmem->csd_c_size =
                ((p_sdmem->csd[1] & 0xc0000000) >> 30) | ((p_sdmem->csd[2] & 0x3ff) << 2);
            p_sdmem->csd_c_size_mult = (p_sdmem->csd[1] & 0x38000) >> 15;
            p_sdmem->block_len = 1 << (p_sdmem->csd_read_bl_len);
            p_sdmem->block_count =
                (1 << (p_sdmem->csd_c_size_mult + 2)) * (p_sdmem->csd_c_size + 1);

            /* support up 512 block lenght card */
            p_sdmem->block_count <<= (p_sdmem->csd_read_bl_len - 9);

            /*force to 512 block length*/
            p_sdmem->block_len = 512;
        }
        p_sdmem->csd_cmd_class = p_sdmem->csd[2] >> 20;

        /* standby to tranfer CMD7 */
        status = sdc_bus_send_cmd(SDC_CMD7 , p_sdmem->rca );
        if (status & SDC_INT_RTO)
            break;
    }
    while(0);

    if (status & SDC_INT_RTO)
    {
        return RES_SD_ERR_CMD_BUSY;
    }
    return RES_SUCCESS;
}

RES_T sdc_sdmem_card_init(SDMEM_T *p_sdmem)
{
    uint32_t status = 0;
    uint32_t timeout_count;

    SDC_HOST_T *p_sd_host;

    p_sd_host = p_sdmem->p_sd_host;


    do
    {
        if (p_sd_host->dma_flow_ctrl > 0)
            sdc_cfg_dma_mui_trans_size(DMA_BURST_SIZE_8);

#if 0 //fpga 0224 version need following module //ali?
        status = sdc_bus_send_cmd(SDC_CMD0, 0);
        if (status & SDC_INT_RTO)
            break;

        if(p_sdmem->card_ver != SD_CARD_VER_1_0)
        {
            status = sdc_bus_send_cmd(SDC_CMD8, 0x1AA );//2//2.7-3.6V
            if (status & SDC_INT_RTO)
                break;
        }
#endif

        /* sd card soft reset */
        timeout_count = 1000;
        while(timeout_count--)
        {

            /* CMD 55 APP CMD*/
            status = sdc_bus_send_cmd(SDC_CMD55 , 0);
            if (status & SDC_INT_RTO)
                return RES_SD_ERR_CMD_BUSY;

            /* CMD 41 reset card*/
            if(SD_CARD_VER_2_0 == p_sdmem->card_ver)
                status = sdc_bus_send_cmd(SDC_ACMD41,
                                          SDC_CMDARG_HCS | SDC_CMDARG_ACMD41 );
            else
                status = sdc_bus_send_cmd(SDC_ACMD41,
                                          SDC_CMDARG_ACMD41 );

            if (SDC_INT_RTO & status)
            {
                delay_ms( 50);
                continue;
            }

            p_sdmem->ocr = sdc_get_short_response();
            if((p_sdmem->ocr & SDC_OCR_CARD_NO_BUSY_MASK))
            {
                if (p_sdmem->ocr & SDC_OCR_CARD_CCS_MASK)
                    p_sdmem->is_high_capability = 1;

                break;
            }
            delay_ms(50);
        }

        if (status & SDC_INT_RTO)
            break;

        /* read CID CMD2*/
        status = sdc_bus_send_cmd(SDC_CMD2 , 0);
        if (status & SDC_INT_RTO)
            break;
#define  NEED_SD_CARD_ID
#ifdef NEED_SD_CARD_ID
        sdc_get_long_response(&p_sdmem->cid[0]);
#endif

        /* idle to standby CMD3 read sca */
        status = sdc_bus_send_cmd(SDC_CMD3 , 0);
        if (status & SDC_INT_RTO)
            break;

        p_sdmem->rca = 0xFFFF0000 & sdc_get_short_response();

        /* CMD 9 read CSR CMD9*/
        status = sdc_bus_send_cmd(SDC_CMD9 , p_sdmem->rca );
        if (status & SDC_INT_RTO)
        {
            break;
        }

        delay_ms( 50);
        sdc_get_long_response(&p_sdmem->csd[0]);
        if(p_sdmem->is_high_capability)
        {
            p_sdmem->block_len = 512;
            p_sdmem->csd_c_size =
                ((p_sdmem->csd[2] & 0x3f) << 16) | p_sdmem->csd[1] >> 16;
            p_sdmem->block_count = (p_sdmem->csd_c_size + 1) * 1024;
        }
        else
        {
            p_sdmem->csd_read_bl_len = (p_sdmem->csd[2] & 0x000f0000) >> 16;
            p_sdmem->csd_c_size =
                ((p_sdmem->csd[1] & 0xc0000000) >> 30) | ((p_sdmem->csd[2] & 0x3ff) << 2);
            p_sdmem->csd_c_size_mult = (p_sdmem->csd[1] & 0x38000) >> 15;
            p_sdmem->block_len = 1 << (p_sdmem->csd_read_bl_len);
            p_sdmem->block_count =
                (1 << (p_sdmem->csd_c_size_mult + 2)) * (p_sdmem->csd_c_size + 1);

            /* support up 512 block lenght card */
            p_sdmem->block_count <<= (p_sdmem->csd_read_bl_len - 9);

            /*force to 512 block length*/
            p_sdmem->block_len = 512;
        }
        p_sdmem->csd_cmd_class = p_sdmem->csd[2] >> 20;

        /* standby to tranfer CMD7 */
        status = sdc_bus_send_cmd(SDC_CMD7 , p_sdmem->rca );
        if (status & SDC_INT_RTO)
            break;

        /* ACMD 42 disconnect 50K ohm pull_up on pin CD/DAT3 */
        status = sdc_bus_send_cmd(SDC_CMD55 , p_sdmem->rca );
        if (status & SDC_INT_RTO)
            break;

        status = sdc_bus_send_cmd(SDC_ACMD42 , 0 );
    }
    while(0);

    if (status & SDC_INT_RTO)
        return RES_SD_ERR_CMD_BUSY;

    return RES_SUCCESS;


}


RES_T sdc_card_enum(SDMEM_T *p_sdmem)
{
    uint32_t status;
    RES_T status1;
    SDC_HOST_T *p_sd_host;
    p_sd_host = p_sdmem->p_sd_host;


    if(sdc_card_detect())
        return RES_SD_ERR_NO_CARD;

    sdc_bus_init();

    /*For example, if cclk_in is 32 Mhz, then the value is 32,000/(2*400) = 40.*/
    sdc_bus_config(p_sd_host->clk_div, p_sd_host->timeout);

    /* default value */
    p_sdmem->card_type = CARD_TYPE_SD; //sd card
    p_sdmem->card_ver  = SD_CARD_VER_1_0;	//verison 1.*
    p_sdmem->is_high_capability = 0;	//low capability
    //p_sdmem->is_high_speed = 0;		//low speed

#if 0
    status = sdc_bus_send_cmd(SDC_CMD5, 0);
    if(status & SDC_INT_RTO)
    {
#endif
        /*SD or MMC card*/
        status = sdc_bus_send_cmd(SDC_CMD0, 0);
        if (status & SDC_INT_RTO)
            return RES_SD_ERR_CMD_BUSY;

        delay_ms(50);

        status = sdc_bus_send_cmd(SDC_CMD8, 0x1AA );//2//2.7-3.6V
        if (SDC_INT_RTO & status)
        {
            /* CMD 55 APP CMD*/
            status = sdc_bus_send_cmd(SDC_CMD55 , 0);
            if (SDC_INT_RTO & status)
            {
                /* mmc card */
                p_sdmem->card_type = CARD_TYPE_MMC;
                status1 = sdc_mmc_card_init(p_sdmem);
            }
            else
            {
                /*sd card*/
                status1 = sdc_sdmem_card_init(p_sdmem);
            }
            return status1;

        }
        else
        {
            /*sd 2.0 card*/
            p_sdmem->card_ver = SD_CARD_VER_2_0;
            status1 = sdc_sdmem_card_init(p_sdmem);
            return status1;
        }
#if 0
    }
    else
    {
        /*SDIO card*/
        p_sdmem->card_type = CARD_TYPE_SDIO;
        status1 = sdc_sdmem_card_init(p_sdmem);
    }
#endif
    //   return status1;
}


RES_T  sdmem_set_bus_width(SDMEM_T *p_sdmem, uint32_t bus_width)
{
    uint32_t status = RES_ERROR;

    p_sdmem->bus_width  = bus_width;

    do
    {
        status = RES_ERROR;

        /* set block lenght to 1024 is if use block lenght 512 no need send CMD16*/
        status = sdc_bus_send_cmd(SDC_CMD16 , p_sdmem->block_len );
        if (status & SDC_INT_RTO)
            break;
        if(p_sdmem->card_type != CARD_TYPE_MMC)
        {
            status = RES_ERROR;
            /* ACMD 6 select bus width 1bit or 4bit*/
            status = sdc_bus_send_cmd(SDC_CMD55 , p_sdmem->rca );
            if (status & SDC_INT_RTO)
                break;
            status = sdc_bus_send_cmd(SDC_ACMD6 , p_sdmem->bus_width );
            if (status & SDC_INT_RTO)
                break;
        }
    }
    while (0);
    return ((status & SDC_INT_RTO) == 0) ? RES_SUCCESS : RES_ERROR;
}

void sdc_bus_set_data_length(uint32_t data_len,
                             uint32_t block_len, uint32_t bus_mode)
{

    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    /* Write the data size in bytes in the BYTCNT register */
    REGW(&psdio->byt_cnt) = data_len;

    /*Write or Read the block size in bytes in the BLKSIZ register*/
    REGW(&psdio->blk_size) = block_len;

    sdc_cfg_card_width(bus_mode);
    return;
}

uint32_t sdc_bus_read_block(uint32_t block_count,
                          uint32_t block_len, uint32_t bus_width)
{
    sdc_bus_set_data_length( block_len * block_count,
                             block_len, bus_width);
    return block_count;
}

uint32_t sdc_bus_write_block(uint32_t block_count,
                           uint32_t block_len, uint32_t bus_width)
{
    sdc_bus_set_data_length( block_len * block_count,
                             block_len, bus_width);
    return block_count;
}



uint32_t sdc_get_tb_bcnt()
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    return REGW(&psdio->tb_bcnt);
}

RES_T sdc_init(uint32_t irq_enable, uint32_t dma_enable,
               uint32_t clk_div, uint32_t card_width)
{
    RES_T ret;
    int i;

    
    //REGW(SC_RSTCTRL) = (1<<9);
    g_sdmem_info.p_sd_host = &g_sdc_host;
    g_sdc_host.timeout = 0xFFFFFF;
    g_sdc_host.clk_div = clk_div;	/* 1 -- 24M Hz */
#if _SDC_IDMA_FLAG
    g_dma_start = 0;
#endif
	sdc_bus_status=SDC_BUS_WORK_STATUS;
    if(sdc_card_detect())
        return RES_SD_ERR_NO_CARD;

    for(i = 0; i < 20; i++)
    {
        if ((ret = sdc_card_enum(&g_sdmem_info)) == RES_SUCCESS)
            break;
        delay_ms( 20);
    }
    if (ret != RES_SUCCESS)
        return ret;
    ret = sdmem_set_bus_width(&g_sdmem_info, card_width);
    return ret;

}

uint32_t sdc_bus_wait_data_rd_done(uint32_t data_len,
                                 uint32_t timeout)
{
    uint32_t sdc_status = 0;
    uint32_t sdc_div;
    /*Software should look for data error interrupts; that is, bits 7, 9, 13,
    and 15 of the RINTSTS
    register. If required, software can terminate the data transfer by sending a
    STOP command.*/
    sdc_div = sdc_bus_get_clk_div();
    /* Data Timeout(Read) = Min{100ms, Nac} + data_len * 8 */
    //timeout = sdc_div*(300000 + data_len);
    timeout = sdc_div * (600000 + data_len);
#if 0
    if (int_registered)
    {
        while(timeout--)
        {
            if ( g_int_status & SDC_INT_DATA_DONE)
                break;
            if (g_int_status & (SDC_INT_DCRC | SDC_INT_DRTO | SDC_INT_SBE | SDC_INT_EBE
                               ))
            {
                bl_sd_bus_send_cmd(SDC_CMD12, 0);
                break;
            }
        }
        sdc_status = g_int_status;
        g_int_status = 0;
    }
    else
#endif
    {
        while(timeout--)
        {
            sdc_status = sdc_get_raw_int_status();
            if( sdc_status & (SDC_INT_DATA_DONE) )
                break;
            if (sdc_status & (SDC_INT_DCRC | SDC_INT_DRTO | SDC_INT_SBE | SDC_INT_EBE))
            {
                sdc_bus_send_cmd(SDC_CMD12, 0);
                break;
            }
        }
    }
    return sdc_status;
}

uint32_t sdc_bus_wait_data_wr_done(uint32_t data_len, uint32_t timeout)
{
    uint32_t sdc_status = 0;
    uint32_t sdc_div;

    /*Software should look for data error interrupts; that is, for bits 7, 9,
    and 15 of the RINTSTS
    register. If required, software can terminate the data transfer by sending
    the STOP command.*/
    sdc_div = sdc_bus_get_clk_div();
    /* Data Timeout(Read) = Min{100ms, Nac} + data_len * 8 */
    timeout = sdc_div * (300000 + data_len);
#if 0
    if (int_registered)
    {
        while(timeout--)
        {
            if ( g_int_status & SDC_INT_DATA_DONE)
                break;
            if (g_int_status & (SDC_INT_DCRC | SDC_INT_DRTO | SDC_INT_EBE))
            {
                bl_sd_bus_send_cmd(base_addr, SDC_CMD12, 0);
                break;
            }
        }
        sdc_status = g_int_status;
        g_int_status = 0;
    }
    else
#endif
    {
        while(timeout--)
        {
            sdc_status = sdc_get_raw_int_status();
            if( sdc_status & (SDC_INT_DATA_DONE) )
                break;
            if (sdc_status & (SDC_INT_DCRC | SDC_INT_DRTO | SDC_INT_EBE))
            {
                sdc_bus_send_cmd(SDC_CMD12, 0);
                break;
            }
        }
    }
    return sdc_status;
}


uint32_t sdc_get_blkcnt()
{
    return g_sdmem_info.block_count;
}

RES_T sdc_toggle_trans2stdby()
{
    uint32_t	 status;
    //SDMEM_T *p_sdmem = &g_sdmem_info;
    status =  sdc_bus_send_cmd(SDC_CMD7, 0);//p_sdmem->rca);
    if (status & SDC_INT_RTO)
        return RES_SD_ERR_CMD_BUSY;
    return RES_SUCCESS;

}


uint32 sdc_card_det_func()
{

}
#pragma push
#pragma O0
void sd_curstatus_check(void)
{
    
}

#pragma pop


uint32_t get_card_cap(void)
{
    return g_sdmem_info.block_count;
}


#if _SDC_IDMA_FLAG
void sdc_bus_idma_start(uint32_t cmd, uint32_t cmd_arg, SDC_IDMA_DES_T *p_des)
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    SDC_BMOD_U bmod;
    uint32_t tmp;

    // enable idma
    bmod.v = REGW(&psdio->bus_mode.v);
    //bmod.v |= 0x80; //enable idmac
    bmod.bit_info.de = 1;
    bmod.bit_info.fb = 0;
    REGW(&psdio->bus_mode.v) = bmod.v;
    tmp = REGW(&psdio->ctrl.v);
    tmp |= (1 << 25);
    REGW(&psdio->ctrl.v) = tmp;

    // clear all existing interrupts
    REGW(&psdio->id_sts.v) = 0x337;
    REGW(&psdio->id_int_en.v) = 0x337;

    // set up descriptor
    REGW(&psdio->db_addr) = (uint32_t)p_des;

    // write command and command argument. Need start?
    REGW(&psdio->cmd_arg) = cmd_arg;
    REGW(&psdio->cmd.v) = cmd | 0x80000000;;

    // if OWN bit is not set, write poll demand
    if ((REGW(&p_des->des0.v) & 0x80000000) == 0)
    {
        p_des->des0.v |= 0x80000000;
        REGW(&psdio->poll_demand) = 0x0001;
    }
    g_dma_start = 1;
    // DMA is now supposed to start...
}


void sdc_bus_idma_stop()
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    SDC_BMOD_U bmod;
    SDC_CONTROL_U ctrl;

    // enable idma
    bmod.v = REGW(&psdio->bus_mode.v);
    bmod.bit_info.de = 0;	/* disable dma */
    bmod.bit_info.fb = 0;
    REGW(&psdio->bus_mode.v) = bmod.v;
    // do not use internal dma
    ctrl.v = REGW(&psdio->ctrl.v);
    ctrl.bit_info.use_internal_dma = 0;
    REGW(&psdio->ctrl.v) = ctrl.v;
}

uint32_t sdc_idma_get_status()
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    return REGW(&psdio->id_sts.v);
}

uint32_t sdc_idma_wait_done()
{
    uint32_t timeout = 0x7ffffff;
    if(g_dma_start)
    {
        while((sdc_idma_get_status() & 0x377) == 0)
        {
            if(timeout-- == 0)
                return 1;
        }
        sdc_bus_idma_stop();
        g_dma_start = 0;
    }
    return 0;
}


RES_T sdmem_read_single_block_idma(void *buf, uint32_t block_index)
{
    SDC_IDMA_DES_T des;
    uint32_t cmd_arg;
    uint32_t status = RES_SUCCESS;
    SDMEM_T *p_sdmem = &g_sdmem_info;
    uint32_t block_len = p_sdmem->block_len;


    des.des0.v = 0x8000001c;
    des.des1.v = block_len;
    des.bap1 = (uint32_t)buf & 0xfffffffc;
    des.bap2 = 0;
    if (p_sdmem->is_high_capability)
        cmd_arg = block_index;
    else
        cmd_arg = block_index * block_len;

    //sdc_bus_config(p_sdmem->p_sd_host->clk_div, p_sdmem->p_sd_host->timeout);

    // check status of SD card?
    status = sdc_bus_send_cmd(SDC_CMD13, p_sdmem->rca);
    if (status & SDC_INT_RTO)
        return RES_SD_ERR_CMD_BUSY;

    status = sdc_get_short_response();
    if((status & 0x100) == 0)
        return RES_SD_ERR_DATA_BUSY;

    // reset fifo
    sdc_internel_reset(0, 1, 0);

    status = sdc_bus_send_cmd(SDC_CMD16, block_len);
    if (SDC_INT_RTO & status)
        return RES_SD_ERR_DATA;
    // set block length?
    sdc_bus_read_block(1, block_len, p_sdmem->bus_width);

    // start dma
    sdc_bus_idma_start(SDC_CMD17, cmd_arg, &des);

    if(SDC_IDMA_WAIT())
        return RES_SD_ERR_TIMEOUT;

    return RES_SUCCESS;
}

#else

uint32_t sdc_idma_wait_done_dummy()
{
    return 0;
}
#pragma push
#pragma O0
RES_T sdmem_read_single_block(void *pbuf, uint32_t block_index)
{
    uint32_t	 status;
    uint32_t time_out;
    //    SDC_HOST_T *p_sd_host;
    uint32_t  i, j;
    SDC_T *psdio;
    SDMEM_T *p_sdmem = &g_sdmem_info;
    uint32_t *buf = (uint32_t *)pbuf;
    //    p_sd_host = p_sdmem->p_sd_host;
    psdio = (SDC_T *)SDC_BASE_ADDR;
    if (block_index + 1 > p_sdmem->block_count)
        return RES_ERROR;

    p_sdmem->timeout_data = p_sdmem->timeout_read * 1;
    do
    {
        sdc_bus_config(p_sdmem->p_sd_host->clk_div, p_sdmem->p_sd_host->timeout);
        time_out = 3000;
        while(time_out--)
        {
            status =  sdc_bus_send_cmd(SDC_CMD13, p_sdmem->rca);
            if (status & SDC_INT_RTO)
                return RES_SD_ERR_CMD_BUSY;
            status = sdc_get_short_response();
            if(status & 0x100) //ready for data
            {
                status = 0;
                break;
            }
        }
        if (time_out == 0)
            return RES_SD_ERR_DATA_BUSY;	// card is not ready

        status = sdc_bus_send_cmd(SDC_CMD16, p_sdmem->block_len);
        if (SDC_INT_RTO & status)
            break;
        sdc_bus_read_block(1, p_sdmem->block_len, p_sdmem->bus_width);

#if 0
        if(1 == p_sd_host->dma_flow_ctrl)
            sdc_internel_reset(1, 0, 0);
        else if(p_sd_host->dma_flow_ctrl > 1)
            sdc_internel_reset(1, 0, 0);
#endif

        // reset fifo
        sdc_internel_reset(0, 1, 0);
        /*use CMD17 for a single-block read and CMD18 for a multiple-block read.
           For SDIO cards, use CMD53 for both single-block and multiple-block transfers.*/
        if( p_sdmem->is_high_capability)
        {
            status = sdc_bus_send_cmd(SDC_CMD17, block_index );
        }
        else
        {
            status = sdc_bus_send_cmd(SDC_CMD17, block_index * p_sdmem->block_len );
        }
        if (status & SDC_INT_RTO)
            break;

        /*
        		if (0)
        		{
        			status =  sdc_bus_send_cmd(SDC_CMD13, p_sdmem->rca);
        			if (status & SDC_INT_RTO)
        				return RES_SD_ERR_CMD_BUSY;

        			status = sdc_get_short_response();
        			if (status & (1 << 25))	// card is locked
        				return RES_SD_ERR_CMD_BUSY;
        			if ((status & 0x1E00) != 0xA00)	// we expect it's in data state
        			{
        				return RES_SD_ERR_DATA_BUSY;
        			}
        		}
        */
        //		if(0 == p_sd_host->dma_flow_ctrl)
        {
#if 0
            for (i = 0;  i < p_sdmem->block_len / 4; i++)
            {
                //while (sdc_get_status(p_sd_host->base_addr) & SDC_STATUS_FIFO_EMPTY_MASK)
                while (REGW(SDC_STATUS) & SDC_STATUS_FIFO_EMPTY_MASK)
                {
                    //if (sdc_get_raw_int_status(p_sd_host->base_addr) & SDC_INT_DRTO)
                    if(REGW(&psdio->rint_sts.v)&SDC_INT_DRTO)
                        return RES_SD_ERR_TIMEOUT;
                }
                //*(uint32_t*)buf = REGW(&psdio->data);//sdc_read_fifo(p_sd_host->base_addr);
                //++(uint32_t*)buf;
                *((uint32_t *)buf)++ = REGW(&psdio->data);
            }
#else

for (i = 0;  i < p_sdmem->block_len / 32; i++)
{
    while (((REGW(SDC_STATUS) >> 17) & 0x1fff) < 8)
    {
        //if (sdc_get_raw_int_status() & SDC_INT_DRTO)
        if(REGW(&psdio->rint_sts.v)&SDC_INT_DRTO)
        {
            REGW(&psdio->rint_sts.v) = SDC_INT_DRTO;
            return RES_SD_ERR_TIMEOUT;
        }
    }
    for(j = 0; j < 8; j++)
    {
        //  *((uint32_t *)buf)++ = REGW(&psdio->data); //sdc_read_fifo(p_sd_host->base_addr);
        *buf++ = REGW(&psdio->data);
    }
}
#endif
            status = sdc_get_raw_int_status();

            /*Software should look for data error interrupts; that is, bits 7, 9, 13,
               and 15 of the RINTSTS  register. If required, software can terminate
               the data transfer by sending a STOP command.*/
            if (status & (SDC_INT_DCRC | SDC_INT_DRTO | SDC_INT_SBE | SDC_INT_EBE))
            {
                REGW(&psdio->rint_sts.v) = status;
                sdc_bus_send_cmd(SDC_CMD12, 0);
                sdc_internel_reset(0, 1, 0);
                return RES_SD_ERR_DATA;
            }
        }

        status = sdc_bus_wait_data_rd_done(p_sdmem->block_len, p_sdmem->timeout_data);
        if (! (SDC_INT_DATA_DONE & status))
            break;
        sdc_bus_wait_program_done();
    }
    while (0);

    if (p_sdmem->block_len != sdc_get_tb_bcnt())
        status = RES_SD_ERR_DATA;
    else
        status = RES_SUCCESS;

    return (RES_T)status;
}

RES_T sdmem_write_single_block(void *pbuf, uint32_t block_index)
{
    uint32_t	 status;
    uint32_t time_out;
    SDC_HOST_T *p_sd_host;
    SDMEM_T *p_sdmem = &g_sdmem_info;
    SDC_T *psdio;
    uint32_t i;
    uint32_t *buf = (uint32_t *)pbuf;
    p_sd_host = p_sdmem->p_sd_host;
    psdio = (SDC_T *)SDC_BASE_ADDR;
    if (block_index + 1 > p_sdmem->block_count)
        return RES_ERROR;

    //sdc_bus_config(p_sdmem->p_sd_host->base_addr, p_sdmem->p_sd_host->clk_div,
    //p_sdmem->p_sd_host->timeout);

    p_sdmem->timeout_data = p_sdmem->timeout_write;
    do
    {
        sdc_bus_config(p_sdmem->p_sd_host->clk_div, p_sdmem->p_sd_host->timeout);
        /*Before a data transfer command, software should confirm that the card is
        not busy and is in a transfer
        state, which can be done using the CMD13 and CMD7 commands, respectively.*/
        time_out = 3000;
        while(time_out--)
        {
            status =  sdc_bus_send_cmd(SDC_CMD13, p_sdmem->rca);
            if (status & SDC_INT_RTO)
                return RES_SD_ERR_CMD_BUSY;
            status = sdc_get_short_response();
            if(status & 0x100)
            {
                status = 0;
                break;
            }
        }
        if (time_out == 0)
            return RES_SD_ERR_DATA_BUSY;	// card is not ready

        status = sdc_bus_send_cmd(SDC_CMD16, p_sdmem->block_len);
        if (SDC_INT_RTO & status)
            break;

        sdc_bus_write_block(1, p_sdmem->block_len, p_sdmem->bus_width);

        if(p_sdmem->is_high_capability)
        {
            status = sdc_bus_send_cmd(SDC_CMD24, block_index );
        }
        else
        {
            status = sdc_bus_send_cmd(SDC_CMD24, block_index * p_sdmem->block_len );
        }
        if (status & SDC_INT_RTO)
            break;

        if(0 == p_sd_host->dma_flow_ctrl)
        {
            for  ( i = 0; i < p_sdmem->block_len / 4; i++)
            {
                uint32_t retry_count = 0;
                while (sdc_get_status() &SDC_STATUS_FIFO_FULL_MASK)
                {
                    if (retry_count++ > 1000)
                    {
                        // break if the card is no longer in rcv state
                        status =  sdc_bus_send_cmd(SDC_CMD13, p_sdmem->rca);
                        if (status & SDC_INT_RTO)
                            return RES_SD_ERR_CMD_BUSY;
                        status = sdc_get_short_response();
                        if ((status & 0x1E00) != 0x0C00)
                            return RES_SD_ERR_TIMEOUT;
                        retry_count = 0;
                    }
                }
                // REGW(&psdio->data) = *((uint32_t *)buf)++;
                REGW(&psdio->data) = *buf++;
            }

            status = sdc_get_raw_int_status();

            /*Software should look for data error interrupts; that is, for bits 7, 9,
               and 15 of the RINTSTS register. If required, software can terminate
               the data transfer by sending the STOP command.*/
            if (status & (SDC_INT_DCRC | SDC_INT_DRTO | SDC_INT_EBE))
            {
                sdc_bus_send_cmd(SDC_CMD12, 0);
                sdc_internel_reset(0, 1, 0);
                return RES_SD_ERR_DATA;
            }
        }

        status = sdc_bus_wait_data_wr_done(p_sdmem->block_count * p_sdmem->block_len, p_sdmem->timeout_write);
        if (! (SDC_INT_DATA_DONE & status))
            break;
        sdc_bus_wait_program_done();
    }
    while (0);

    if (p_sdmem->block_len != sdc_get_tb_bcnt())
    {
        status = RES_ERROR;
    }
    else
    {
        status = RES_SUCCESS;
    }
    return (RES_T)status;
}
#pragma pop

#endif
#if 1  //xujg
RES_T sdc_toggle_stdby2trans()
{
    uint32_t	 status;
    SDMEM_T *p_sdmem = &g_sdmem_info;
    status =  sdc_bus_send_cmd(SDC_CMD7, p_sdmem->rca);
    if (status & SDC_INT_RTO)
        return RES_SD_ERR_CMD_BUSY;

    return RES_SUCCESS;

}
#endif

void sd_suspend()
{//sd to dtandby
       SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
  //  SDC_FIFOTH_U fifo_th;

    /*power on for card 0*/

	if(pskeys.option_save_flag&(ENABLE_SD_IIC_NOSHARE_PIN)) 
		return;
	if(pskeys.enable_sd_exit& ENABLE_SD_RESET)	
    		sdc_internel_reset(0, 0, 1);	//reest ctrl
	if(pskeys.enable_sd_exit& ENABLE_SD_POWEROFF)		
    		REGW(&psdio->pwr_en) = 0;	
	if(pskeys.enable_sd_exit& ENABLE_SD_EXIT_BUSINIT)		
		sdc_bus_init();

    if(pskeys.opera_mask & 0x80)
    {
        IO_WRITE(pskeys.mem_addr7, pskeys.content7);
    }
   sdc_bus_status=SDC_BUS_RESET_STATUS;

       iic_sd_mux(SDC_MUX);  	
	
}
#if 0
uint32 sdreg_buf[40];
 void  sd_reg_save()
 {
 /*
#define SDC_CTRL            (SDC_BASE_ADDR+0x00)                // SDC Control register
#define SDC_PWREN           (SDC_BASE_ADDR+0x04)                // SDC Power-enable register
#define SDC_CLKDIV          (SDC_BASE_ADDR+0x08)                // SDC Clock-divider register
#define SDC_CLKSRC          (SDC_BASE_ADDR+0x0C)                // SDC Clock-source register
#define SDC_CLKENA          (SDC_BASE_ADDR+0x10)                // SDC Clock-enable register
#define SDC_TMOUT           (SDC_BASE_ADDR+0x14)                // SDC Time-out register
#define SDC_CTYPE           (SDC_BASE_ADDR+0x18)                // SDC Card-type register
#define SDC_BLKSIZ          (SDC_BASE_ADDR+0x1C)                // SDC Block-size register
#define SDC_BYTCNT          (SDC_BASE_ADDR+0x20)                // SDC Byte-count register
#define SDC_INTMASK         (SDC_BASE_ADDR+0x24)                // SDC Interrupt-mask register
*/
uint32 i;
for(i=0;i<10;i++)
	sdreg_buf[i]=REGW(SDC_BASE_ADDR+i*4);
 }
  void  sd_reg_restore()
 {
uint32 i;
for(i=0;i<10;i++)
	REGW(SDC_BASE_ADDR+i*4)=sdreg_buf[i];
 }
#endif
#if 0
uint32_t sdc_read_fifo()
{
    SDC_T *psdio = (SDC_T *)SDC_BASE_ADDR;
    return REGW(&psdio->data);
}
#endif
