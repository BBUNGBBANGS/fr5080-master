#ifndef _DRIVER_I2S_H
#define _DRIVER_I2S_H

#include <stdint.h>
#include "driver_plf.h"


#define I2S_REG_BASE_ADDRESS	I2S_BASE

#define I2S_REG_CTRL			I2S_REG_BASE_ADDRESS
#define I2S_REG_BCLK_DIV		(I2S_REG_BASE_ADDRESS+0x04)
#define I2S_REG_FRM_DIV			(I2S_REG_BASE_ADDRESS+0x08)
#define I2S_REG_DATA			(I2S_REG_BASE_ADDRESS+0x0C)
#define I2S_REG_STATUS			(I2S_REG_BASE_ADDRESS+0x10)
#define I2S_REG_INTEN			(I2S_REG_BASE_ADDRESS+0x14)
#define I2S_REG_DIS_DMA			(I2S_REG_BASE_ADDRESS+0x18) //not used
#define I2S_REG_TX_SIM          (I2S_REG_BASE_ADDRESS+0x1c) //enable sbc to i2s dma
#define I2S_REG_FF_CFG          (I2S_REG_BASE_ADDRESS+0x20)

/*bitmap*/
/*CONTROL REG*/
#define bmCTRL_I2SEN			BIT0
#define bmCTRL_I2SFMT			BIT1
#define bmCTRL_I2SDLYEN			BIT2
#define bmCTRL_I2SFRMINV		BIT3
#define bmCTRL_I2SBCLKINV		BIT4
#define bmCTRL_I2SMODE_MASTER	BIT5
#define bmCTRL_I2SLP			BIT6
#define bmCTRL_I2SDMARXEN		BIT7
#define bmCTRL_I2SDMATXEN		BIT8
#define bmCTRL_I2SINTEN			BIT9

/*status reg*/
#define bmSTATUS_RXFFFULL   BIT0
#define bmSTATUS_RXFFHFULL  BIT1
#define bmSTATUS_RXFFEMPTY  BIT2
#define bmSTATUS_TXFFFULL   BIT3
#define bmSTATUS_TXFFHEMPTY BIT4
#define bmSTATUS_TXFFEMPTY  BIT5

/*interrupt enable reg*/
#define bmINTEN_RXFFFULLINTEN	BIT0
#define bmINTEN_RXFFHFULLINTEN	BIT1
#define bmINTEN_RXFFEMPTYINTEN	BIT2
#define bmINTEN_TXFFFULLINTEN	BIT3
#define bmINTEN_TXFFHEMPTYINTEN	BIT4
#define bmINTEN_TXFFEMPTYINTEN	BIT5


#define I2S_FIFO_DEPTH  64

#define I2S_INT_STATUS_RX_FULL          (1<<0)
#define I2S_INT_STATUS_RX_HFULL         (1<<1)
#define I2S_INT_STATUS_RX_EMPTY         (1<<2)
#define I2S_INT_STATUS_TX_FULL          (1<<3)
#define I2S_INT_STATUS_TX_HEMPTY        (1<<4)
#define I2s_INT_STATUS_TX_EMPTY         (1<<5)

enum i2s_dir_t
{
    I2S_DIR_TX = (1<<0),
    I2S_DIR_RX = (1<<1),
};

enum i2s_mode_t
{
    I2S_MODE_SLAVE = 0,
    I2S_MODE_MASTER = 1,
};

enum i2s_data_type_t
{
    I2S_DATA_MONO = 0,
    I2S_DATA_STEREO = 1,
};

enum i2s_tx_src_t {
    I2S_TX_SRC_IPC_MCU,
    I2S_TX_SRC_SBC_DECODER,
};

/*
 * TYPEDEFS
 */
struct i2s_control_t {
	uint32_t en:1;
	uint32_t format:1;
	uint32_t dlyen:1;
	uint32_t frminv:1;
	uint32_t bclkinv:1;
	uint32_t mode:1;	// 1: master, 0: slave
	uint32_t lp:1;
	uint32_t rx_int_en:1;
	uint32_t tx_int_en:1;
	uint32_t inten:1;
	uint32_t reserved:22;
};

struct i2s_int_status_t {
	uint32_t rx_full:1;
	uint32_t rx_half_full:1;
	uint32_t rx_empty:1;
	uint32_t tx_full:1;
	uint32_t tx_half_empty:1;
	uint32_t tx_emtpy:1;
	uint32_t reserved:26;
};

struct i2s_int_en_t {
	uint32_t rx_full:1;
	uint32_t rx_half_full:1;
	uint32_t rx_empty:1;
	uint32_t tx_full:1;
	uint32_t tx_half_empty:1;
	uint32_t tx_emtpy:1;
	uint32_t reserved:26;
};

struct i2s_tx_src_sel_t {
    uint32_t sel:1; // 0: ipc/MCU, 1: SBC decoder
    uint32_t reserved:31;
};

struct i2s_reg_t {
	struct i2s_control_t ctrl;		// @0x00
	uint32_t bclk_div;
	uint32_t frm_div;
	uint32_t data;
	struct i2s_int_status_t status; // @0x10
	struct i2s_int_en_t mask;
    uint32_t reserved;
    struct i2s_tx_src_sel_t tx_src_sel;
};

extern volatile struct i2s_reg_t *i2s_reg;
/*********************************************************************
 * @fn      i2s_init
 *
 * @brief   Initialize i2s.
 *
 * @param   type        - @ref i2s_dir_t.
 *          bus_clk     - i2s bus bit clock
 *          sample_rate - i2s sample rate
 *          mode        - which mode i2s works in. @ref i2s_mode_t
 *
 * @return  None.
 */
void i2s_init_(uint8_t type, uint32_t bus_clk, uint32_t sample_rate, enum i2s_mode_t mode);


/*********************************************************************
 * @fn      i2s_start
 *
 * @brief   start i2s.
 *
 * @param   None.
 *
 * @return  None.
 */
void i2s_start_(void);

/*********************************************************************
 * @fn      i2s_start_without_int
 *
 * @brief   start i2s with interrupt disabled.
 *
 * @param   None.
 *
 * @return  None.
 */
void i2s_start_without_int(void);

/*********************************************************************
 * @fn      i2s_stop
 *
 * @brief   stop i2s.
 *
 * @param   None.
 *
 * @return  None.
 */
void i2s_stop_(void);

/*********************************************************************
 * @fn      i2s_get_int_status
 *
 * @brief   get current i2s interrupt status.
 *
 * @param   None.
 *
 * @return  current interrupt status.
 */
uint32_t i2s_get_int_status_(void);

/*********************************************************************
 * @fn      i2s_get_data
 *
 * @brief   read i2s data from rx fifo.
 *
 * @param   buffer  - pointer to a buffer used to store data.
 *          length  - how many data to read, should no larger than I2S_FIFO_DEPTH
 *          type    - data is mono (2 bytes for one sample) or stereo (4 bytes for
 *                    one sample), @ref i2s_data_type_t.
 *
 * @return  None.
 */
void i2s_get_data_(void *buffer, uint8_t length, uint8_t type);

/*********************************************************************
 * @fn      i2s_send_data
 *
 * @brief   write data to i2s tx fifo.
 *
 * @param   buffer  - pointer to a buffer store avaliable data.
 *          length  - how many data to send, should no larger than I2S_FIFO_DEPTH
 *          type    - data is mono (2 bytes for one sample) or stereo (4 bytes for
 *                    one sample), @ref i2s_data_type_t.
 *
 * @return  None.
 */
void i2s_send_data_(void *buffer, uint8_t length, uint8_t type);

/*********************************************************************
 * @fn      i2s_set_tx_src
 *
 * @brief   select the source of tx fifo.
 *
 * @param   src     - @ref i2s_tx_src_t
 *
 * @return  None.
 */
void i2s_set_tx_src(enum i2s_tx_src_t src);

#endif  // _DRIVER_I2S_H


