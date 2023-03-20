#ifndef _SDC_H
#define _SDC_H

#include <stdint.h>

#include "type_lib.h"

#define SDC_BASE_ADDR               0x50000000

#define _SDC_IDMA_FLAG	0	/* 0 or 1 */
/* Setting _SDC_IDMA_FLAG to 1, then use idma to read data from sd card;  if 0,
 *  use cpu to read data from sd card. */


#define CARD_NUMBER	0

#define SDC_CLKSRC_CLKDIV0							0
#define SDC_CLKSRC_CLKDIV1							1
#define SDC_CLKSRC_CLKDIV2							2
#define SDC_CLKSRC_CLKDIV3							3

/* bits in CTYPE register */
#define CARD_CTYPE_WIDTH_8BIT_MODE				1
#define CARD_CTYPE_WIDTH_NON_8BIT_MODE			0
#define CARD_CTYPE_WIDTH_4BIT_MODE				1
#define CARD_CTYPE_WIDTH_NON_4BIT_MODE			0

/* bits in CMD register */
#define SDC_CMD_CMDINDEX(x)							(x & 0x3f)

#define SDC_CMD_RSP_EXP								(1 << 6)
#define SDC_CMD_RSP_SHORT_LEN						0
#define SDC_CMD_RSP_LONG_LEN						(1 << 7)
#define SDC_CMD_CHECK_RSP_CRC						(1 << 8)
#define SDC_CMD_DATA_EXP							(1 << 9)
#define SDC_CMD_DIR_READ							0
#define SDC_CMD_DIR_WRITE							(1 << 10)
#define SDC_CMD_TRANS_MODE_STREAM					(1 << 11)
#define SDC_CMD_SEND_AUTO_STOP						(1 << 12)
#define SDC_CMD_WAIT_PRI_DONE						(1 << 13)
#define SDC_CMD_ABORT_STOP_CMD						(1 << 14)
#define SDC_CMD_SEND_INIT							(1 << 15)

#define SDC_CMD_CARD_NUM(x)							((x& 0x1f) << 16)

#define SDC_CMD_UPDATE_CLK_REG						(1 << 21)
#define SDC_CMD_NOT_UPDATE_CLK_REG					0
#define SDC_CMD_RD_CEATA_DEV						(1 << 22)
#define SDC_CMD_NOT_RD_CEATA_DEV					0
#define SDC_CMD_CCS_EXP								(1 << 23)
#define SDC_CMD_START_CMD							(1 << 31)

/* define SD card command */
#define SDC_CMD_DUMMY					(SDC_CMD_UPDATE_CLK_REG | SDC_CMD_WAIT_PRI_DONE )
//#define SDC_CMD_DUMMY					(SDC_CMD_UPDATE_CLK_REG)
#define SDC_CMD0						(SDC_CMD_CMDINDEX(0) |SDC_CMD_SEND_INIT |SDC_CMD_CHECK_RSP_CRC |SDC_CMD_WAIT_PRI_DONE)
#define SDC_CMD1						(SDC_CMD_CMDINDEX(1) | SDC_CMD_RSP_EXP  | SDC_CMD_WAIT_PRI_DONE)
#define SDC_CMD2						(SDC_CMD_CMDINDEX(2) | SDC_CMD_RSP_EXP | SDC_CMD_RSP_LONG_LEN | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE)
#define SDC_CMD3						(SDC_CMD_CMDINDEX(3) | SDC_CMD_RSP_EXP  |SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE)
#define SDC_CMD5						(SDC_CMD_CMDINDEX(5) | SDC_CMD_RSP_EXP  |SDC_CMD_WAIT_PRI_DONE)
#define SDC_CMD6						(SDC_CMD_CMDINDEX(6) | SDC_CMD_RSP_EXP  |SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE \
	|SDC_CMD_DIR_READ | SDC_CMD_DATA_EXP)
#define SDC_CMD7 						(SDC_CMD_CMDINDEX(7) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE)
#define SDC_CMD8						(SDC_CMD_CMDINDEX(8) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE)
#define SDC_CMD9						(SDC_CMD_CMDINDEX(9) | SDC_CMD_RSP_EXP | SDC_CMD_RSP_LONG_LEN | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE)
#define SDC_CMD12						(SDC_CMD_CMDINDEX(12) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC |SDC_CMD_ABORT_STOP_CMD)
#define SDC_CMD13						(SDC_CMD_CMDINDEX(13) | SDC_CMD_RSP_EXP | SDC_CMD_CHECK_RSP_CRC)
#define SDC_ACMD13						(SDC_CMD_CMDINDEX(13) | SDC_CMD_RSP_EXP | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE\
	  |SDC_CMD_DIR_READ |SDC_CMD_DATA_EXP)
#define SDC_CMD15						(SDC_CMD_CMDINDEX(15) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE)
#define SDC_CMD16						(SDC_CMD_CMDINDEX(16) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE)
#define SDC_CMD17						(SDC_CMD_CMDINDEX(17) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC  | SDC_CMD_WAIT_PRI_DONE\
		|SDC_CMD_DIR_READ |SDC_CMD_DATA_EXP)
#define SDC_CMD18						(SDC_CMD_CMDINDEX(18) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE\
		|SDC_CMD_DIR_READ |SDC_CMD_DATA_EXP | SDC_CMD_SEND_AUTO_STOP)
#define SDC_CMD24						(SDC_CMD_CMDINDEX(24) | SDC_CMD_RSP_EXP   | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE\
		|SDC_CMD_DIR_WRITE |SDC_CMD_DATA_EXP)
#define SDC_CMD25						(SDC_CMD_CMDINDEX(25) | SDC_CMD_RSP_EXP   | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE\
		|SDC_CMD_DIR_WRITE |SDC_CMD_DATA_EXP)
#define SDC_CMD28						(SDC_CMD_CMDINDEX(28) | SDC_CMD_RSP_EXP   | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE)
#define SDC_CMD29						(SDC_CMD_CMDINDEX(29) | SDC_CMD_RSP_EXP   | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE)
#define SDC_CMD30						(SDC_CMD_CMDINDEX(30) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC  | SDC_CMD_WAIT_PRI_DONE)
#define SDC_CMD32						(SDC_CMD_CMDINDEX(32) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE )
#define SDC_CMD33						(SDC_CMD_CMDINDEX(33) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE )
#define SDC_CMD38						(SDC_CMD_CMDINDEX(38) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE )
#define SDC_CMD42						(SDC_CMD_CMDINDEX(42) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE \
	 |SDC_CMD_DIR_WRITE |SDC_CMD_DATA_EXP)
#define SDC_CMD55						(SDC_CMD_CMDINDEX(55) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE )
#define SDC_CMD7_NORSP				(SDC_CMD_CMDINDEX(7) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE )
#define SDC_ACMD6						(SDC_CMD_CMDINDEX(6) | SDC_CMD_RSP_EXP  | SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE )
#define SDC_ACMD23						(SDC_CMD_CMDINDEX(23) | SDC_CMD_RSP_EXP |SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE )
#define SDC_ACMD41						(SDC_CMD_CMDINDEX(41) | SDC_CMD_RSP_EXP |SDC_CMD_WAIT_PRI_DONE)
#define SDC_ACMD42						(SDC_CMD_CMDINDEX(42) | SDC_CMD_RSP_EXP |SDC_CMD_CHECK_RSP_CRC | SDC_CMD_WAIT_PRI_DONE )


#define SDC_CMDARG_ACMD41			0x00ff8000
#define MMC_CMDARG_CMD1				0x00ff8000
#define SDC_CMDARG_HCS				(1<<30)



/* interrupt masks */
#define SDC_INTMASK_EBE_INT_MASK					(1 << 15)
#define SDC_INTMASK_ACD_INT_MASK					(1 << 14)
#define SDC_INTMASK_SBE_INT_MASK					(1 << 13)
#define SDC_INTMASK_HLE_INT_MASK					(1 << 12)
#define SDC_INTMASK_FRUN_INT_MASK					(1 << 11)
#define SDC_INTMASK_HTO_INT_MASK					(1 << 10)
#define SDC_INTMASK_DRTO_INT_MASK					(1 << 9)
#define SDC_INTMASK_RTO_INT_MASK					(1 << 8)
#define SDC_INTMASK_DCRC_INT_MASK					(1 << 7)
#define SDC_INTMASK_RCRC_INT_MASK					(1 << 6)
#define SDC_INTMASK_RXDR_INT_MASK					(1 << 5)
#define SDC_INTMASK_TXDR_INT_MASK					(1 << 4)
#define SDC_INTMASK_DTO_INT_MASK					(1 << 3)
#define SDC_INTMASK_CMD_DONE_INT_MASK				(1 << 2)
#define SDC_INTMASK_RE_INT_MASK						(1 << 1)
#define SDC_INTMASK_CD_INT_MASK						(1 << 0)
#define SDC_REG_CLR_STATUS_MASK						0xffff

/* interrupt bits */
#define SDC_INT_COUNT						17
#define SDIO_INT_CARD0						(1 << 16)
#define SDC_INT(id)							(id << 16)
#define SDC_INT_EBE							(1 << 15)
#define SDC_INT_ACD							(1 << 14)
#define SDC_INT_SBE							(1 << 13)
#define SDC_INT_HLE							(1 << 12)
#define SDC_INT_FRUN						(1 << 11)
#define SDC_INT_HTO						(1 << 10)
#define SDC_INT_DRTO						(1 << 9)
#define SDC_INT_RTO						(1 << 8)
#define SDC_INT_DCRC						(1 << 7)
#define SDC_INT_RCRC						(1 << 6)
#define SDC_INT_RXDR						(1 << 5)
#define SDC_INT_TXDR						(1 << 4)
#define SDC_INT_DATA_DONE					(1 << 3)
#define SDC_INT_CMD_DONE					(1 << 2)
#define SDC_INT_RE							(1 << 1)
#define SDC_INT_CD							(1 << 0)
#define SDC_REG_FIFO_ADDR 0x100

/* IDMAC interrupt mask */
#define SDC_IDMAC_INT_TI_MASK				BIT_0	/* transmit interrupt */
#define SDC_IDMAC_INT_RI_MASK				BIT_1	/* receive interrupt */
#define SDC_IDMAC_INT_FBE_MASK				BIT_2	/* fatal bus error */
#define SDC_IDMAC_INT_DU_MASK				BIT_4	/* descriptor unavailable */
#define SDC_IDMAC_INT_CES_MASK				BIT_5	/* card error summary */
#define SDC_IDMAC_INT_NIS_MASK				BIT_8	/* normal interrupt summary */
#define SDC_IDMAC_INT_AIS_MASK				BIT_9	/* abnormal interrupt summary */

#define SDC_IDMAC_INT_COUNT					10

/* Status register */
#define SDC_STATUS_DMA_IRQ						(1 << 31)
#define SDC_STATUS_DMA_ACK						(1 << 30)
#define SDC_STATUS_FIFO_COUNT_MASK				(0x1fff << 17)
#define SDC_STATUS_RSP_INDEX_MASK				(0x1f << 11)
#define SDC_STATUS_DAT_MC_BUSY_MASK				(1 << 10)
#define SDC_STATUS_DAT_BUSY_MASK				( 1 << 9)
#define SDC_STATUS_DAT2_STATUS					( 1 << 8)
#define SDC_STATUS_CMD_FSM_STA_MASK				(0xf0)
#define SDC_STATUS_FIFO_FULL_MASK				(1 << 3)
#define SDC_STATUS_FIFO_EMPTY_MASK				(1 << 2)
#define SDC_STATUS_REACH_TX_WMARK				(1 << 1)
#define SDC_STATUS_REACH_RX_WMARK				(1 << 0)

#define DMA_BURST_SIZE_1		0
#define DMA_BURST_SIZE_4		1
#define DMA_BURST_SIZE_8		2
#define DMA_BURST_SIZE_16		3
#define DMA_BURST_SIZE_32		4
#define DMA_BURST_SIZE_64		5
#define DMA_BURST_SIZE_128		6
#define DMA_BURST_SIZE_256		7

typedef union _SDC_IDMA_DES0_U
{
    uint32_t			v;
    struct _SDC_IDMA_DES0_T
    {
        unsigned		reserved_0: 1;
        unsigned		dic: 1;			/* bool; disable interrupt on completion */
        unsigned		ld: 1;			/* bool; last descriptor */
        unsigned		fs: 1;			/* bool; first descriptor */
        unsigned		ch: 1;			/* bool; second address chained */
        unsigned		er: 1;			/* bool; end of ring */
        unsigned		reserved_6_29: 24;
        unsigned		ces: 1;			/* bool; card error summary */
        unsigned		own: 1;			/* bool; when set, this descriptor belongs to IDMA */
    } bit_info;
} SDC_IDMA_DES0_U;

typedef union _SDC_IDMA_DES1_U
{
    uint32_t			v;
    struct _SDC_IDMA_DES1_T
    {
        unsigned		bs1: 13;			/* buffer 1 size, in byte */
        unsigned		bs2: 13;			/* buffer 2 size, in bytes */
        unsigned		reserved_26_31: 6;
    } bit_info;
} SDC_IDMA_DES1_U;

/* Internal DMA descriptor */
typedef struct _SDC_IDMA_DES_T
{
    SDC_IDMA_DES0_U		des0;
    SDC_IDMA_DES1_U		des1;
    uint32_t					bap1;	/* buffer address pointer 1 */
    uint32_t					bap2;	/* buffer address pointer 2; if des0.ch then this is next
descriptor addr */
} SDC_IDMA_DES_T;


typedef union _SDC_CONTROL_U_
{
    unsigned long v;
    struct _SDC_CONTROL_T_
    {
        unsigned  controller_reset: 1;
        unsigned  fifo_reset: 1;
        unsigned  dam_reset: 1;
        unsigned  reserved_bit3: 1;
        unsigned  int_enable: 1;
        unsigned  dma_enable: 1;
        unsigned  read_wait: 1;
        unsigned  send_irq_res: 1;
        unsigned  abort_reaq_data: 1;
        unsigned  send_ccsd: 1;
        unsigned  send_auto_stop_ccsd: 1;
        unsigned  ceata_dev_int_status: 1;
        unsigned  reserved_bit12_bit15: 4;
        unsigned  card_vol_a: 4;
        unsigned  card_vlo_b: 4;
        unsigned  use_od_pullup: 1;
        unsigned  use_internal_dma: 1;
        unsigned  reserved_bit26_bit31: 6;
    } bit_info;
} SDC_CONTROL_U;

typedef union _SDC_CLKDIV_U_
{
    unsigned long v;
    struct _SDC_CLKDIV_T_
    {
        unsigned div0: 8;
        unsigned div1: 8;
        unsigned div2: 8;
        unsigned div3: 8;
    } bit_info;
} SDC_CLKDIV_U;

typedef union _SDC_CLKENA_U_
{
    unsigned long v;
    struct _SDC_CLKENA_T_
    {
        unsigned  clk_enable: 16;
        unsigned  clk_low_power_en: 16;
    } bit_info;
} SDC_CLKENA_U;

typedef union _SDC_TMOUT_U_
{
    unsigned long v;
    struct _SDC_TMOUT_T_
    {
        unsigned  rsp_tmout: 8;
        unsigned  data_timout: 24;
    } bit_info;
} SDC_TMOUT_U;

typedef union _SDC_CTYPE_U_
{
    unsigned long v;
    struct _SDC_CTYPE_T_
    {
        unsigned  card_width_1bit_4bit: 16;
        unsigned  card_width_8bit: 16;
    } bit_info;
} SDC_CTYPE_U;

typedef union _SDC_INTMASK_U_
{
    unsigned long v;
    struct _SDC_INTMASK_T_
    {
        unsigned  int_mask: 16;
        unsigned  sdio_int_mask: 16;
    } bit_info;
} SDC_INTMASK_U;

typedef union _SDC_CMD_U_
{
    unsigned long v;
    struct _SDC_CMD_T_
    {
        unsigned  cmd_index: 6;
        unsigned  rsp_exp: 1;
        unsigned  rsp_len: 1;
        unsigned  check_rsp_crc: 1;
        unsigned  data_exp: 1;
        unsigned  read_write: 1;
        unsigned  trans_mode: 1;
        unsigned  send_auto_stop: 1;
        unsigned  wait_prvdata_cmpl: 1;
        unsigned  stop_abort_cmd: 1;
        unsigned  send_init: 1;
        unsigned  card_num: 5;
        unsigned  update_clk: 1;
        unsigned  read_ceata_dev: 1;
        unsigned  ccs_exp: 1;
        unsigned  reserved_bit24_bit30: 7;
        unsigned  cmd_start: 1;
    } bit_info;
} SDC_CMD_U;

typedef union _SDC_MINTSTS_U_
{
    unsigned long v;
    struct _SDC_MINTSTS_T_
    {
        unsigned  int_status: 16;
        unsigned  sdio_int_status: 16;
    } bit_info;
} SDC_MINTSTS_U;

typedef union _SDC_RINTSTS_U_
{
    unsigned long v;
    struct _SDC_RNTSTS_T_
    {
        unsigned  int_status: 16;
        unsigned  sdio_int_status: 16;
    } bit_info;
} SDC_RINTSTS_U;

typedef union _SDC_STATUS_U_
{
    unsigned long v;
    struct _SDC_STATUS_T_
    {
        unsigned  fifo_rx_watermark: 1;
        unsigned  fifo_tx_watermark: 1;
        unsigned  fifo_empty: 1;
        unsigned  fifo_full: 1;
        unsigned  cmd_fms_states: 4;
        unsigned  data_3_status: 1;
        unsigned  data_busy: 1;
        unsigned  data_state_mc_busy: 1;
        unsigned response_index: 6;
        unsigned  fifo_count: 13;
        unsigned  dma_ack: 1;
        unsigned  dma_req: 1;
    } bit_info;
} SDC_STATUS_U;

typedef union _SDC_FIFOTH_U_
{
    unsigned long v;
    struct _SDC_FIFOTH_T_
    {
        unsigned  tx_wmark: 12;
        unsigned  reserved_bit12_bit15: 4;
        unsigned  rx_wmark: 12;
        unsigned  dw_dma_mul_trans_size: 3;
        unsigned  reserved_bit31: 1;
    } bit_info;
} SDC_FIFOTH_U;

typedef union _SDC_GPIO_U_
{
    unsigned long v;
    struct _SDC_GPIO_T_
    {
        unsigned  gpi: 8;
        unsigned  gpo: 16;
    } bit_info;
} SDC_GPIO_U;

typedef union _SDC_HCON_U_
{
    unsigned long v;
    struct _SDC_HCON_T_
    {
        unsigned  card_type: 1;
        unsigned  num_card: 5;
        unsigned  bus_type: 1;
        unsigned  data_width: 3;
        unsigned  addr_width: 6;
        unsigned  dma_interface: 2;
        unsigned  ge_dma_width: 3;
        unsigned  fifo_inside: 1;
        unsigned  imp_hold: 1;
        unsigned  set_clk_false: 1;
        unsigned  num_clk_div: 2;
        unsigned  area_opt: 1;
        unsigned  reserverd_bit27_bit31: 5;
    } bit_info;
} SDC_HCON_U;

typedef union _SDC_BMOD_U_
{
    unsigned long v;
    struct _SDC_BMOD_T_
    {
        unsigned  swr: 1;
        unsigned  fb: 1;
        unsigned  dsl: 5;
        unsigned  de: 1;
        unsigned  pbl: 3;
        unsigned  reserved_bit11_bit31: 21;
    } bit_info;
} SDC_BMOD_U;

typedef union _SDC_IDSTS_U_
{
    unsigned long v;
    struct _SDC_IDSTS_T_
    {
        unsigned  ti: 1;
        unsigned  ri: 1;
        unsigned  fbe: 1;
        unsigned  reserved_bit3: 1;
        unsigned  du: 1;
        unsigned  ces: 1;
        unsigned  reserved_bit6_bit7: 2;
        unsigned  nis: 1;
        unsigned  ais: 1;
        unsigned  eb: 3;
        unsigned  fsm: 4;
        unsigned        reserved_bit17_bit31: 15;
    } bit_info;
} SDC_IDMA_STATUS_U;

typedef union _SDC_IDINTEN_U_
{
    unsigned long v;
    struct _SDC_IDINTEN_T_
    {
        unsigned  ti: 1;
        unsigned  ri: 1;
        unsigned  fbe: 1;
        unsigned  reserved_bit3: 1;
        unsigned  du: 1;
        unsigned  ces: 1;
        unsigned  reserved_bit6_bit7: 2;
        unsigned  ni: 1;
        unsigned  ai: 1;
        unsigned  reserved_bit10_bit31: 22;
    } bit_info;
} SDC_IDMA_INTEN_U;

typedef struct _SDC_T_
{
    SDC_CONTROL_U 		ctrl;
    uint32_t					pwr_en;
    SDC_CLKDIV_U			clk_div;
    uint32_t					clk_src;
    SDC_CLKENA_U			clk_ena;
    SDC_TMOUT_U			tm_out;
    SDC_CTYPE_U			c_type;
    uint32_t					blk_size;
    uint32_t					byt_cnt;
    SDC_INTMASK_U		int_mask;
    uint32_t					cmd_arg;
    SDC_CMD_U			cmd;
    uint32_t					rsp0;
    uint32_t					rsp1;
    uint32_t					rsp2;
    uint32_t					rsp3;
    SDC_MINTSTS_U		mint_sts;
    SDC_RINTSTS_U		rint_sts;
    SDC_STATUS_U			status;
    SDC_FIFOTH_U			fifo_th;
    uint32_t					cd_detect;
    uint32_t					wrt_prt;
    SDC_GPIO_U			gpio;
    uint32_t					tc_bcnt;
    uint32_t					tb_bcnt;
    uint32_t					debounce_count;
    uint32_t					usr_id;
    uint32_t					ver_id;
    SDC_HCON_U			hw_config;
    uint32_t					reserved_0x74_0x7c[3];
    /*IDMA contrlo registers*/
    SDC_BMOD_U			bus_mode;
    uint32_t					poll_demand;
    uint32_t					db_addr;
    SDC_IDMA_STATUS_U	id_sts;
    SDC_IDMA_INTEN_U		id_int_en;
    uint32_t					dsc_addr;		/* SDC_IDMA_DES_T* */
    uint32_t					buf_addr;		/* SDC_IDMA_DES_T*; read only; updated by IDMAC */
    uint32_t					reserved_0x9c_0xfc[(0x100 - 0x9C) / 4];
    uint32_t					data;

} SDC_T;


/** SD card version 1.0 */
#define SD_CARD_VER_1_0	0
/** SD card version 2.0 */
#define SD_CARD_VER_2_0	1

/** Type SD card */
#define CARD_TYPE_SD	0
/** Type MMC card */
#define CARD_TYPE_MMC	1
/** Type SDIO card */
#define CARD_TYPE_SDIO	2

/** @struct SDC_HOST_T
 *  @brief Struct of SDIO configuration information
 */
typedef struct _SDC_HOST
{
    //	uint32_t reset_mask;          /**< SDIO reset mask */
    //	uint32_t irq_mask;            /**< SDIO interrupt mask */
    uint8_t dma_flow_ctrl;        /**< SDIO DMA flow contrl */
    //	uint8_t dma_pid;              /**< SDIO DMA pid */
    //	uint8_t dma_channle;          /**< SDIO DMA channel */
    //	uint8_t dma_group;            /**< SDIO DMA group */
    //	uint32_t flag;                /**< SDIO flag */
    uint32_t timeout;             /**< SDIO timeout */
    uint32_t clk_freq_hz; 		/**< Clock freq unit hz*/
    //	uint32_t cpu_circle_per_ms;	/**< CPU_CIRCLE_PER_MS */
    //	uint8_t speed_level;          /**< SDIO speed level */
    uint8_t clk_div; 				/**< CLK_DIV REG of SD controller */
    //	uint8_t bus_width;			/**< 1W 4W */
    //	uint8_t bus_spi;              /**< SDIO bus spi*/
} SDC_HOST_T;

/** @struct SDMEM_T
 *  @brief Struct of SDIO information
 */
typedef struct _SDMEM_INFO
{
    SDC_HOST_T *p_sd_host;      /**< SD host information */
    uint8_t card_type;			/**< 0:	SD Memory Card, 1:	mmc */
    uint8_t card_ver;		        /**< 0: version 1.X  	1:	2.00 or later */
    //	uint8_t is_high_speed;        /**< High speed or not */
    uint8_t is_high_capability;   /**< High capability or not */
    //	uint32_t bus_speed_hz;        /**< Bus speed in hz */
    uint32_t bus_width;           /**< Bus width */
    uint32_t block_count;			/**< Block count, by csd */
    uint32_t block_len;			/**< Block length, by csd default 512B */

#if 1
    uint32_t sdc_timeout;			/**< Controller wait act done */
    uint32_t timeout_cmd;			/**< Wait cmd done */
    uint32_t timeout_data;		/**< Time out data */
    uint32_t timeout_read;		/**< Wait data read done time */
    uint32_t timeout_write;		/**< Wait data write done time */
    //	uint32_t timeout_erase;		/**< Wait erase done time 	*/
    //	uint32_t delay_time;			/**< Wait card run time */
#endif

    uint32_t cid[4];		        /**< 128bit CID REG of SD card by cmd2 */
    /*
    cid:
    MID: 8BIT[127-120]
    OID:16BIT
    PRODUCT NAME:40BIT[103-64]
    PRODUCT VERSION:8BIT
    SEARIAL NUMBER: 32BIT
    RESERVED:4BIT
    DATE:12BIT
    CRC:7BIT
    RESERVED :1BIT

    */
    uint32_t rca; 			    /**< 16bit  RCA REG of SD card by cmd3 */
    uint32_t csd[4];		        /**< 128bit CSD REG of SD card by cmd9 */
    uint32_t ocr; 			    /**< 32bit  OCR REG of SD card by acmd41 */
    //	uint32_t card_status;

    /* CSD field*/
    uint32_t csd_read_bl_len;     /**< csd_read_bl_len */
    uint32_t csd_c_size;          /**< csd_c_size */
    uint32_t csd_c_size_mult;     /**< csd_c_size_mult */
    uint32_t csd_cmd_class;       /**< csd_cmd_class */
    //	uint32_t csd_max_tran_speed;  /**< csd_max_tran_speed */

#if 0
    uint32_t csd_r2w_factor;	/**< For write timeout */
    uint32_t csd_wtite_bl;	/**< For erase timeout */
    uint8_t csd_taac;			/**< For read timeout */
    uint8_t csd_nsac;			/**< For read timeout */
#endif
    /* CID field */
} SDMEM_T;


#if _SDC_IDMA_FLAG
#define SDMEM_READ_SINGLE_BLOCK	sdmem_read_single_block_idma
#define SDC_IDMA_WAIT				sdc_idma_wait_done
#else
#define SDMEM_READ_SINGLE_BLOCK	sdmem_read_single_block
#define SDC_IDMA_WAIT				sdc_idma_wait_done_dummy
#endif

void sdc_bus_set_clk(uint32_t clk_div);

RES_T sdc_init(uint32_t irq_enable, uint32_t dma_enable,
               uint32_t clk_div, uint32_t card_width);

#if _SDC_IDMA_FLAG
RES_T sdmem_read_single_block_idma(void *buf, uint32_t block_index);

uint32_t sdc_idma_wait_done(void);

#else
RES_T sdmem_read_single_block(void *buf, uint32_t block_index);
RES_T sdmem_write_single_block(void *buf, uint32_t block_index);
uint32_t sdc_idma_wait_done_dummy(void);

#endif

uint32_t sdc_get_blkcnt(void);
RES_T sdc_card_detect(void);
RES_T sdc_card_get_wrtprt(void);
RES_T sdc_toggle_standby_trans(void);
void sd_curstatus_check(void);

#endif // _SDC_H
