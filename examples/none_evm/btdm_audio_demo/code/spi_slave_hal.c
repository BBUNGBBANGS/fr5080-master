#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "driver_ssp.h"
#include "driver_plf.h"
#include "driver_syscntl.h"
#include "driver_gpio.h"
#include "driver_exti.h"
#include "driver_uart.h"
#include "os_mem.h"

#include "spi_slave_hal.h"

#define SPI_SLAVE_RX_FF_TRIG_LVL        64



struct spi_slave_tx_env_t {
    uint8_t *src;       // 将要发送的数据的指针
    uint32_t length;    // 还剩余多少数据要发送
    p_spi_s2m_write_complite_callback_t callback;
};

struct spi_slave_rx_env_t {
    uint8_t *dst;       // 新收到数据存放的地址
    uint32_t length;    // 记录当前已经收到了多少数据
};



struct spi_slave_tx_env_t spi_slave_tx_env;
struct spi_slave_rx_env_t spi_slave_rx_env;
struct spi_env_t spi_env;
uint16_t user_spi_send_length;
extern enum spi_recv_state_t spi_recv_state;
__attribute__((section("ram_code"))) void ssp_isr(void)
{
    uint32_t isr_status;

    isr_status = ssp_get_isr_status();
    if(isr_status & SSP_INT_RX_FF) {
        
        //uart_putc_noint_no_wait('r');
        //printf("%d,%d\r\n",spi_env.trans_dir,spi_env.rx_buffer_size);
        /* 产生接收中断后，只有当前状态处于接收状态才进行处理，否则把fifo里面数据读出来就完 */
        if(spi_env.trans_dir == SPI_TRANS_DIR_INPUT) {
            uint16_t last_length = spi_env.rx_buffer_size - spi_slave_rx_env.length;
            uint16_t act_length;
            act_length = ssp_get_data_from_fifo(spi_slave_rx_env.dst, last_length);
            last_length -= act_length;
            if(last_length < SPI_SLAVE_RX_FF_TRIG_LVL) {
                ssp_set_ff_int_ctrl(last_length, SSP_FIFO_SIZE>>1);
            }
            spi_slave_rx_env.length += act_length;
            spi_slave_rx_env.dst += act_length;
            
            //printf("%d,%d,%x\r\n",act_length,spi_env.rx_buffer_size,spi_env.recv_callback);
            if(spi_slave_rx_env.length >= spi_env.rx_buffer_size) {
                if(spi_env.recv_callback) {
                    //uart_putc_noint_no_wait('r');
                    spi_env.trans_dir = SPI_TRANS_DIR_NONE;
                    ssp_disable_interrupt(SSP_INT_RX_FF);
                    spi_env.recv_callback(spi_env.rx_buffer, spi_env.rx_buffer_size);
                    //os_free(spi_env.rx_buffer);
                    //spi_env.rx_buffer = (void *)os_malloc(spi_env.rx_buffer_size);
                }
                spi_slave_rx_env.length = 0;
            }
        }
        else {
            uint32_t dummy;
            while(ssp_get_data_from_fifo((void *)dummy, sizeof(uint32_t)));
        }
        ssp_clear_isr_status(SSP_INT_RX_FF);
    }

    if(isr_status & SSP_INT_TX_FF) {
        
        //uart_putc_noint_no_wait('t');
        if(spi_env.trans_dir == SPI_TRANS_DIR_OUTPUT) {
            if(spi_slave_tx_env.length == 0) {
                while(ssp_get_status() & SSP_STATUS_BUSY);
                //uart_putc_noint_no_wait('f');
                //gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_7,0);

                ssp_slave_output_disable();
                clear_data_from_fifo();
                ssp_disable_interrupt(SSP_INT_TX_FF);
                //ssp_clear_rx_fifo();
//                if(spi_slave_tx_env.callback) {
//                    spi_slave_tx_env.callback();
//                }
            }
            else {
                uint32_t frame_len;
                if(spi_slave_tx_env.length >= (SSP_FIFO_SIZE>>1)) {
                    frame_len = (SSP_FIFO_SIZE>>1);
                }
                else {
                    ssp_set_ff_int_ctrl(SPI_SLAVE_RX_FF_TRIG_LVL, 1);
                    frame_len = spi_slave_tx_env.length;
                }

                frame_len = ssp_put_data_to_fifo(spi_slave_tx_env.src+user_spi_send_length, frame_len);
                user_spi_send_length += frame_len;
                spi_slave_tx_env.length -= frame_len;
            }
        }
        ssp_clear_isr_status(SSP_INT_TX_FF);
    }
}

#if 0
__attribute__((section("ram_code"))) void exti_isr_ram(void)
{
    uint32_t status;
    uint8_t index;

    status = ext_int_get_status();

    ext_int_clear(status);
    
    printf("exti_usr:%x\r\n",status);

    index = spi_env.m2s_io_irq_cfg.portx * 8 + spi_env.m2s_io_irq_cfg.pin;
    if(status & (1<<index)) {
        /* 这里先临时关掉中断，由回调修改触发条件？建议修改方式如下：
         * 开始传输触发条件                 修改为
         * 低电平                      高电平
         * 高电平                      高电平
         * 上升沿                      低电平
         * 下降沿                      高电平
         */
        ext_int_disable((enum system_port_t)spi_env.m2s_io_irq_cfg.portx, (enum system_port_bit_t)spi_env.m2s_io_irq_cfg.pin);
        if(spi_env.m2s_trigger_callback) {
            spi_env.m2s_trigger_callback(spi_env.m2s_io_irq_cfg.pin_irq);
        }
    }

    ext_int_clear(status);
}
#endif

/*!****************************************************************************
\brief 
    主机触发到从机通信 io 初始化,且告知同时注册此io中断响应函数
\param[in] 
    m2s_io_irq_cfg   指定io及中断极性配置，中断极性根据硬件设计决定，初始化io时应根据极性判断出需要设置的常态电平
    fun              io触发时，中断响应函数, 且由参数irq_state指示出当前是哪种极性的中断
\param[out] 
\retval      成功返回HAL_SUCCESS，其他见hal_error.h
*****************************************************************************/
uint32_t trigger_io_master_to_slave_isr_init(IO_IRQ_DEF_T m2s_io_irq_cfg, p_trigger_io_m2s_isr_callback_t fun)
{
    enum ext_int_type_t type;

    if(m2s_io_irq_cfg.pin_irq == PIN_IRQ_RISING) {
        type = EXT_INT_TYPE_POS;
    }
    else if(m2s_io_irq_cfg.pin_irq == PIN_IRQ_FALLING) {
        type = EXT_INT_TYPE_NEG;
    }
    else if(m2s_io_irq_cfg.pin_irq == PIN_IRQ_LOW) {
        type = EXT_INT_TYPE_LOW;
    }
    else if(m2s_io_irq_cfg.pin_irq == PIN_IRQ_HIGH) {
        type = EXT_INT_TYPE_HIGH;
    }
    else {
        return HAL_ERROR_INVALID_PARAM;
    }

    spi_env.m2s_io_irq_cfg = m2s_io_irq_cfg;
    spi_env.m2s_trigger_callback = fun;

    system_set_port_mux((enum system_port_t)m2s_io_irq_cfg.portx, (enum system_port_bit_t)m2s_io_irq_cfg.pin, 0);
    gpio_int_enable((enum system_port_t)m2s_io_irq_cfg.portx, (enum system_port_bit_t)m2s_io_irq_cfg.pin);

    /* 设置50us的防抖中断 */
    ext_int_set_control((enum system_port_t)m2s_io_irq_cfg.portx, (enum system_port_bit_t)m2s_io_irq_cfg.pin, 100000, 5);
    ext_int_set_type((enum system_port_t)m2s_io_irq_cfg.portx, (enum system_port_bit_t)m2s_io_irq_cfg.pin, type);
    ext_int_enable((enum system_port_t)m2s_io_irq_cfg.portx, (enum system_port_bit_t)m2s_io_irq_cfg.pin);

    return HAL_SUCCESS;
}

/*!****************************************************************************
\brief 
    从机触发到主机通信 io 初始化，且告知此模块内部 有效触发电平是高还是低
\param[in] 
    s2m_io_cfg       指定io
    pin_en_set       设置有效触发电平，比如设置为EN_LOW，则表示常态下次io不能为低，从机需要发送数据到主机了才置低电平
\param[out] 
\retval      成功返回HAL_SUCCESS，其他见hal_error.h
*****************************************************************************/
uint32_t trigger_io_slave_to_master_init(IO_DEF_T s2m_io_cfg, PIN_EN_VALUE_E pin_en_set)
{
    spi_env.s2m_io_cfg = s2m_io_cfg;
    spi_env.pin_en_set = pin_en_set;
    system_set_port_mux((enum system_port_t)s2m_io_cfg.portx, (enum system_port_bit_t)s2m_io_cfg.pin, 0);
    gpio_set_dir((enum system_port_t)s2m_io_cfg.portx, (enum system_port_bit_t)s2m_io_cfg.pin, GPIO_DIR_OUT);
    if(pin_en_set == EN_LOW) {
        gpio_set_pin_value((enum system_port_t)s2m_io_cfg.portx, (enum system_port_bit_t)s2m_io_cfg.pin, 1);
    }
    else {
        gpio_set_pin_value((enum system_port_t)s2m_io_cfg.portx, (enum system_port_bit_t)s2m_io_cfg.pin, 0);
    }
    return HAL_SUCCESS;
}

/*!****************************************************************************
\brief 
    使能从机到主机通信IO，告知主机，从机有数据需要给主机
\param[in] 
    s2m_io_cfg       指定io
\param[out] 
\retval      成功返回HAL_SUCCESS，其他见hal_error.h
*****************************************************************************/
uint32_t trigger_io_slave_to_master_enable(void)
{
    if(spi_env.pin_en_set == EN_LOW) {
        gpio_set_pin_value((enum system_port_t)spi_env.s2m_io_cfg.portx, (enum system_port_bit_t)spi_env.s2m_io_cfg.pin, 0);
    }
    else {
        gpio_set_pin_value((enum system_port_t)spi_env.s2m_io_cfg.portx, (enum system_port_bit_t)spi_env.s2m_io_cfg.pin, 1);
    }
    
    return HAL_SUCCESS;
}

/*!****************************************************************************
\brief 
    除能从机到主机通信IO
\param[in] 
    s2m_io_cfg       指定io
\param[out] 
\retval      成功返回HAL_SUCCESS，其他见hal_error.h
*****************************************************************************/
uint32_t trigger_io_slave_to_master_disable(void)
{
    if(spi_env.pin_en_set == EN_LOW) {
        gpio_set_pin_value((enum system_port_t)spi_env.s2m_io_cfg.portx, (enum system_port_bit_t)spi_env.s2m_io_cfg.pin, 1);
    }
    else {
        gpio_set_pin_value((enum system_port_t)spi_env.s2m_io_cfg.portx, (enum system_port_bit_t)spi_env.s2m_io_cfg.pin, 0);
    }
    
    return HAL_SUCCESS;
}

/*!****************************************************************************
\brief 
    spi初始化
\param[in] 
    config   spi 从机配置
    fun      从机收到主机发来数据后的回调，把数据及其长度告知给上层应用
\param[out] 
\retval      成功返回HAL_SUCCESS，其他见hal_error.h
*****************************************************************************/
uint32_t spi_slave_init(SPI_CONFIG_T *config, p_spi_slave_receive_from_master_t fun)
{
    uint8_t spi_mode;

    if(config->data_size != 8) {
        return HAL_ERROR_DATA_SIZE;
    }
    system_set_port_pull_up(GPIO_PD0,true);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_0, 0x04);

    /* 准备好数据发送和接收buffer */
    spi_env.rx_buffer_size = config->rx_buf_size;
    spi_env.tx_buffer_size = config->tx_buf_size;
    spi_env.rx_buffer = (void *)os_malloc(spi_env.rx_buffer_size);
    spi_env.tx_buffer = (void *)os_malloc(spi_env.tx_buffer_size);
    spi_env.trans_dir = SPI_TRANS_DIR_NONE;
    spi_env.recv_callback = fun;

    if(config->mode.m_s_mode == SPI_MASTER) {
        spi_mode = SSP_MASTER_MODE;
    }
    else {
        spi_mode = SSP_SLAVE_MODE;
    }
    ssp_init(8, SSP_FRAME_MOTO, spi_mode, config->freq, 2, 0);
    ssp_set_clk_type(config->mode.clk_mode.CLKPolarity, config->mode.clk_mode.CLKPhase);
    
    system_set_port_mux((enum system_port_t)config->clk.portx, (enum system_port_bit_t)config->clk.pin, 0x04);
    system_set_port_mux((enum system_port_t)config->cs.portx, (enum system_port_bit_t)config->cs.pin, 0x04);
    system_set_port_mux((enum system_port_t)config->mosi.portx, (enum system_port_bit_t)config->mosi.pin, 0x04);
    system_set_port_mux((enum system_port_t)config->miso.portx, (enum system_port_bit_t)config->miso.pin, 0x04);

    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_0, 0);

    ssp_slave_output_disable();

    return HAL_SUCCESS;
}

/*!****************************************************************************
\brief            从机写数据给到主机，
\param[in] 
    buf         要写的数据
    buf_size    要写数据的长度
    fun         告知上层应用数据已写完的回调
\param[out] 
\retval     成功返回HAL_SUCCESS，其他见hal_error.h
*****************************************************************************/
__attribute__((section("ram_code"))) uint32_t spi_slave_write_to_master(uint8_t *buf,uint16_t buf_size, p_spi_s2m_write_complite_callback_t fun)
{
    uint32_t frame_length = SSP_FIFO_SIZE;
    user_spi_send_length = 0;
    if(buf_size == 0) {
        return HAL_SUCCESS;
    }

    if(buf_size > spi_env.tx_buffer_size) {
        return HAL_ERROR_RESOURCES;
    }
    memcpy(spi_env.tx_buffer, buf, buf_size);

    spi_slave_tx_env.src = spi_env.tx_buffer;
    spi_slave_tx_env.length = buf_size;
    spi_slave_tx_env.callback = fun;
    
    ssp_disable();

    ssp_put_data_to_fifo((void *)&buf_size, sizeof(uint16_t));
    if(spi_slave_tx_env.length > (SSP_FIFO_SIZE-sizeof(uint16_t))) {
        /* 如果fifo中不能完全填充数据，则先填满fifo，然后配置成半空中断 */
        ssp_set_ff_int_ctrl(SPI_SLAVE_RX_FF_TRIG_LVL, SSP_FIFO_SIZE>>1);
        frame_length = SSP_FIFO_SIZE-sizeof(uint16_t);
    }
    else {
        /* 如果fifo中能完全填充数据，则全部填完，发送中断配置成<=1中断，这时在即将发送完成时即产生中断 */
        ssp_set_ff_int_ctrl(SPI_SLAVE_RX_FF_TRIG_LVL, 1);
        frame_length = spi_slave_tx_env.length;
    }

    frame_length = ssp_put_data_to_fifo(spi_slave_tx_env.src, frame_length);
    spi_slave_tx_env.length -= frame_length;
    user_spi_send_length+=frame_length;
    spi_env.trans_dir = SPI_TRANS_DIR_OUTPUT;

    ssp_enable_interrupt(SSP_INT_TX_FF);
    ssp_slave_output_enable();
    ssp_enable();
    NVIC_EnableIRQ(SSP_IRQn);

    return HAL_SUCCESS;
}

void spi_slave_receive_data(uint16_t length)
{
    spi_env.trans_dir = SPI_TRANS_DIR_INPUT;
    spi_env.rx_buffer_size = length;
		//printf("set:%d",length);
    /* 开启接收中断，是否需要改成收到主机中断信号之后再开启接收中断？ */
    spi_slave_rx_env.dst = spi_env.rx_buffer;
    spi_slave_rx_env.length = 0;
    ssp_enable_interrupt(SSP_INT_RX_FF);
    
    //uart_putc_noint_no_wait((uint8_t)(length>>8));
    //uart_putc_noint_no_wait((uint8_t)(length&0xff));

    if(length > SPI_SLAVE_RX_FF_TRIG_LVL) {
        ssp_set_ff_int_ctrl(SPI_SLAVE_RX_FF_TRIG_LVL, SSP_FIFO_SIZE>>1);
    }
    else {
        ssp_set_ff_int_ctrl(length, SSP_FIFO_SIZE>>1);
    }
    ssp_enable();

    NVIC_EnableIRQ(SSP_IRQn);
}

