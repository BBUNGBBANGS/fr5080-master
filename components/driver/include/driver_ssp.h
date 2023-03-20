#ifndef _DRIVER_SSP_H
#define _DRIVER_SSP_H

#include <stdint.h>      // standard integer definition

/*
 * TYPEDEFS (类型定义)
 */
#define SSP_FIFO_SIZE           128

enum ssp_int_type_t {
    SSP_INT_RX_FF   = (1<<0),
    SSP_INT_TX_FF   = (1<<1),
    SSP_INT_RX_FFOV = (1<<2),
};

enum ssp_int_status_t {
    SSP_INT_STATUS_RX       = (1<<0),
    SSP_INT_STATUS_TX       = (1<<1),
    SSP_INT_STATUS_RX_FFOV  = (1<<2),
};

enum ssp_status_t {
    SSP_STATUS_TFE      = (1<<0),
    SSP_STATUS_TNF      = (1<<1),
    SSP_STATUS_RNE      = (1<<2),
    SSP_STATUS_RFF      = (1<<3),
    SSP_STATUS_BUSY     = (1<<4),
};

enum ssp_frame_type_t
{
    SSP_FRAME_MOTO,
    SSP_FRAME_SS,
    SSP_FRAME_NATTIONAL_M,
};

enum ssp_ms_mode_t
{
    SSP_MASTER_MODE,
    SSP_SLAVE_MODE,
};

enum ssp_cs_ctrl_op_t
{
    SSP_CS_ENABLE,
    SSP_CS_DISABLE,
};

/*
 * PUBLIC FUNCTIONS (全局函数)
 */

/*********************************************************************
 * @fn      ssp_enable
 *
 * @brief   enable ssp controller
 *
 * @param   None
 *			
 * @return  None.
 */
void ssp_enable(void);

/*********************************************************************
 * @fn      ssp_disable
 *
 * @brief   disable ssp controller
 *
 * @param   None
 *			
 * @return  None.
 */
void ssp_disable(void);

/*********************************************************************
 * @fn      ssp_enable_interrupt
 *
 * @brief   set ssp interrupt condition.
 *
 * @param   ints  - reference enum ssp_int_type_t
 *			
 * @return  None.
 */
void ssp_enable_interrupt(uint8_t ints);

/*********************************************************************
 * @fn      ssp_disable_interrupt
 *
 * @brief   clear ssp interrupt condition.
 *
 * @param   ints  - reference enum ssp_int_type_t
 *			
 * @return  None.
 */
void ssp_disable_interrupt(uint8_t ints);

/*********************************************************************
 * @fn      ssp_set_ff_int_ctrl
 *
 * @brief   set ssp fifo interrupt trigger level.
 *
 * @param   rx_lvl  - rx fifo interrupt will generate if rx_fifo >= rx_lvl
 *          tx_lvl  - tx fifo interrupt will generate if tx_fifo <= tx_lvl
 *          
 * @return  None.
 */
void ssp_set_ff_int_ctrl(uint8_t rx_lvl, uint8_t tx_lvl);

/*********************************************************************
 * @fn      ssp_init_
 *
 * @brief   Initialize ssp instance.
 *
 * @param   bit_width   - trans or recv bits witdh
 *          frame_type  -
 *          ms          - indicate ssp controller working mode
 *          bit_rate    - ssp bus frame clock
 *          prescale    - ssp controller prescale
 *          ssp_cs_ctrl - if cs is controlled by software, this parameter
 *                        should be corresponding function.
 *
 * @return  None.
 */
void ssp_init(uint8_t bit_width, uint8_t frame_type, uint8_t ms, uint32_t bit_rate, uint8_t prescale, void (*ssp_cs_ctrl)(uint8_t));

/*********************************************************************
 * @fn      ssp_get_isr_status
 *
 * @brief   get current interrupt status
 *
 * @param   None
 * 
 * @return  current status ,@ref ssp_int_status_t.
 */
uint32_t ssp_get_isr_status(void);

/*********************************************************************
 * @fn      ssp_clear_isr_status
 *
 * @brief   used to clear interrupt status
 *
 * @param   status  - which status should be cleard, @ref ssp_int_status_t
 *
 * @return  None
 */
void ssp_clear_isr_status(uint32_t status);

/*********************************************************************
 * @fn      ssp_get_status
 *
 * @brief   get current fifo and ssp bus status
 *
 * @param   None
 * 
 * @return  current status ,@ref ssp_status_t.
 */
uint32_t ssp_get_status(void);

/*********************************************************************
 * @fn      ssp_put_data_to_fifo
 *
 * @brief   put data to send fifo, this function will return when all data have
 *          been put into tx fifo or tx fifo is full
 *
 * @param   buffer  - data pointer to be send
 *          length  - how many bytes to be send.
 *
 * @return  how many data have been put into tx fifo.
 */
uint16_t ssp_put_data_to_fifo(uint8_t *buffer, uint16_t length);

/*********************************************************************
 * @fn      ssp_get_data_from_fifo
 *
 * @brief   get data from receive fifo, this function will return when length
 *          is reached or rx fifo is empty.
 *
 * @param   buffer  - data pointer to be store received buffer
 *          length  - how many bytes to get.
 *
 * @return  how many data have been got from rx fifo.
 */
uint16_t ssp_get_data_from_fifo(uint8_t *buffer, uint16_t length);

/*********************************************************************
 * @fn      ssp_slave_output_enable
 *
 * @brief   enable output when working as slave
 *
 * @param   None
 * 
 * @return  None
 */
void ssp_slave_output_enable(void);

/*********************************************************************
 * @fn      ssp_slave_output_disable
 *
 * @brief   disable output when working as slave
 *
 * @param   None
 * 
 * @return  None
 */
void ssp_slave_output_disable(void);

/*********************************************************************
 * @fn      ssp_set_clk_type
 *
 * @brief   set spi clock parameter
 *
 * @param   polarity    -
 *          phase       -
 * 
 * @return  None
 */
void ssp_set_clk_type(uint8_t polarity, uint8_t phase);

/*********************************************************************
 * @fn      ssp_recv_data
 *
 * @brief   receive data from peripheral.
 *
 * @param   buffer  - data pointer to store received data
 *          length  - how many bytes to be send
 *			
 * @return  None.
 */
void ssp_recv_data(uint8_t *buffer, uint32_t length);

/*********************************************************************
 * @fn      ssp_send_data
 *
 * @brief   send data to peripheral.
 *
 * @param   buffer  - data pointer to be send
 *          length  - how many bytes to be send
 *			
 * @return  None.
 */
void ssp_send_data(uint8_t *buffer, uint32_t length);

uint16_t  ssp_clear_rx_fifo(void);

void ssp_wait_busy_bit(void);

uint8_t ssp_send_then_recv_flash(uint8_t* tx_buffer, uint32_t n_tx, uint8_t* rx_buffer, uint32_t n_rx);

#endif

