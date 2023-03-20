/*!**********************************************************************************
    \file   ser_spi_slave_hal.h
    \brief  
        双机通信中从机读写数据接口,依赖mcu提供spi_hal,gpio_hal,interrupt_hal等
        硬件接线方式：
        Master                     Slave
        trigger_m2s_pin    ->      trigger_m2s_pin
        trigger_s2m_pin    <-      trigger_s2m_pin
        cs                 --      cs
        clk                ->      clk
        mosi               ->      mosi
        miso               <-      miso

        工作流程：
        0、使用半双工模式，收发不能存在于同一时刻。
        1、初始化：从机spi，trigger_m2s_pin（中断）、trigger_s2m_pin（输出）。trigger_m2s_pin应该是中断口，常态电平由硬件决定，模块内软件
           根据传入参数设置常态电平，
           中断响应函数中，应该给出当前中断的触发沿，以便外部了解当前中断服务是主机发数据到从机的起始还是结束。
        2、主机发数据到从机：先通过 trigger_m2s_pin 给出信号告知从机，随后发起spi总线通信，主机发完除能信号，主机发送期间，从机不可发出数据。
        3、从机发数据到主机：先通过 trigger_s2m_pin 给出信号告知主机，传完除能信号，在 trigger_s2m_pin有效即从机可发出数据期间，
            spi总线通信由主机发起即clk由主机生成，且主机不可输出数据，只接收从机发来的数据。

        4、不论从机是主动要发数据给主机，还是从机被动应答主机查数据，只要是在总线上数据是由从机流向主机的，则使能trigger_s2m_pin信号，否则除能之。
        5、trigger_m2s_pin信号 同4理。
        6、本模块只负责物理层数据交互，总线上数据的逻辑意义及相应处理由其他模块负责。
        
        

    CopyRight @2020 Idoo
    Revision:
    2020-08-10   chenguohui 创建
***********************************************************************************/


#ifndef __SPI_SLAVE_HAL_H__
#define __SPI_SLAVE_HAL_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "hal_error.h"

/*spi外设*/
typedef enum{
    SPI_0,
}SPI_E;

/*SPI CLK极性 相位定义*/
typedef struct{
    uint8_t CLKPolarity;
    uint8_t CLKPhase;
}SPI_CLK_MODE_T;

/*spi 主从定义*/
typedef enum{
    SPI_MASTER,
    SPI_SLAVE
}SPI_M_S_MODE_E;

/*spi 模式*/
typedef struct{
    SPI_CLK_MODE_T clk_mode;
    SPI_M_S_MODE_E m_s_mode;
}SPI_MODE_T;

/*bit传输顺序*/
typedef enum{
    HAL_SPI_BIT_ORDER_MSB,
    HAL_SPI_BIT_ORDER_LSB
}SPI_BIT_ORDER_E;

/*spi 时钟频率*/
typedef enum{
    HAL_SPI_FREQ_125K = 125000,
    HAL_SPI_FREQ_250K = 250000,
    HAL_SPI_FREQ_500K = 500000,
    HAL_SPI_FREQ_1M = 1000000,
    HAL_SPI_FREQ_2M = 2000000,
    HAL_SPI_FREQ_4M = 4000000,
		HAL_SPI_FREQ_6M = 6000000,
    HAL_SPI_FREQ_8M = 8000000,
    HAL_SPI_FREQ_16M = 16000000,
    HAL_SPI_FREQ_24M = 24000000,
    HAL_SPI_FREQ_MAX,
}SPI_FREQ_E;

/*端口*/
typedef enum{
    PORT_A,      
    PORT_B,
    PORT_C,
    PORT_D,
}PORT_DEF_E;

/*io端口号、引脚号*/
typedef struct{
    PORT_DEF_E      portx;
    uint32_t        pin;
}IO_DEF_T;

/*中断触发方式*/
typedef enum{
    PIN_IRQ_RISING,
    PIN_IRQ_FALLING,
    PIN_IRQ_RISING_FALLING,
    PIN_IRQ_LOW,
    PIN_IRQ_HIGH,
    PIN_IRQ_LOW_HIGH
}IO_IRQ_E;

/*外部中断io端口号、引脚号*/
typedef struct{
    PORT_DEF_E      portx;
    uint32_t        pin;
    IO_IRQ_E        pin_irq;
}IO_IRQ_DEF_T;

/**/
typedef struct {
    SPI_E                        inst;           //哪个外设
    SPI_MODE_T                   mode;           //时钟极性、相位
    SPI_BIT_ORDER_E              bit_order;      //先传高位还是低位
    SPI_FREQ_E                   freq;           //速率
    uint8_t                      data_size;      //数据 8位宽度还是  16位宽度
    uint16_t                     tx_buf_size;    //发送缓存深度
    uint16_t                     rx_buf_size;    //接收缓存深度
    IO_DEF_T                     cs;             //片选io定义
    IO_DEF_T                     clk;            //io定义
    IO_DEF_T                     mosi;           //io定义
    IO_DEF_T                     miso;           //io定义

    void                        *param;          //扩展使用
}SPI_CONFIG_T;

typedef enum{
    EN_LOW,
    EN_HIGH
}PIN_EN_VALUE_E;
    
enum spi_trans_dir_t {
    SPI_TRANS_DIR_NONE,
    SPI_TRANS_DIR_INPUT,
    SPI_TRANS_DIR_OUTPUT,
};

//
typedef void (*p_trigger_io_m2s_isr_callback_t)(IO_IRQ_E irq_state);
typedef uint32_t (*p_spi_slave_receive_from_master_t)(uint8_t *buf,uint16_t buf_size);
typedef void (*p_spi_s2m_write_complite_callback_t)(void);


struct spi_env_t {
    uint8_t *tx_buffer;
    uint8_t *rx_buffer;
    uint16_t tx_buffer_size;
    uint16_t rx_buffer_size;
    p_spi_slave_receive_from_master_t recv_callback;
    p_trigger_io_m2s_isr_callback_t m2s_trigger_callback;
    IO_IRQ_DEF_T m2s_io_irq_cfg;
    IO_DEF_T s2m_io_cfg;
    PIN_EN_VALUE_E pin_en_set;
    enum spi_trans_dir_t trans_dir;
};

extern struct spi_env_t spi_env;
/*!****************************************************************************
\brief 
    主机触发到从机通信 io 初始化,且告知同时注册此io中断响应函数
\param[in] 
    m2s_io_irq_cfg   指定io及中断极性配置，中断极性根据硬件设计决定，初始化io时应根据极性判断出需要设置的常态电平
    fun              io触发时，中断响应函数, 且由参数irq_state指示出当前是哪种极性的中断
\param[out] 
\retval      成功返回HAL_SUCCESS，其他见hal_error.h
*****************************************************************************/
uint32_t trigger_io_master_to_slave_isr_init(IO_IRQ_DEF_T m2s_io_irq_cfg, p_trigger_io_m2s_isr_callback_t fun);

/*!****************************************************************************
\brief 
    从机触发到主机通信 io 初始化，且告知此模块内部 有效触发电平是高还是低
\param[in] 
    s2m_io_cfg       指定io
    pin_en_set       设置有效触发电平，比如设置为EN_LOW，则表示常态下次io不能为低，从机需要发送数据到主机了才置低电平
\param[out] 
\retval      成功返回HAL_SUCCESS，其他见hal_error.h
*****************************************************************************/
uint32_t trigger_io_slave_to_master_init(IO_DEF_T s2m_io_cfg, PIN_EN_VALUE_E pin_en_set);

/*!****************************************************************************
\brief 
    使能从机到主机通信IO，告知主机，从机有数据需要给主机
\param[in] 
    s2m_io_cfg       指定io
\param[out] 
\retval      成功返回HAL_SUCCESS，其他见hal_error.h
*****************************************************************************/
uint32_t trigger_io_slave_to_master_enable(void);

#if 1
/*!****************************************************************************
\brief 
    除能从机到主机通信IO
\param[in] 
    s2m_io_cfg       指定io
\param[out] 
\retval      成功返回HAL_SUCCESS，其他见hal_error.h
*****************************************************************************/
uint32_t trigger_io_slave_to_master_disable(void);
#endif

/*!****************************************************************************
\brief 
    spi初始化
\param[in] 
    config   spi 从机配置
    fun      从机收到主机发来数据后的回调，把数据及其长度告知给上层应用
\param[out] 
\retval      成功返回HAL_SUCCESS，其他见hal_error.h
*****************************************************************************/
uint32_t spi_slave_init(SPI_CONFIG_T *config, p_spi_slave_receive_from_master_t fun);

#if 0
/*!****************************************************************************
\brief 
\param[in] 
\param[out] 
\retval      成功返回HAL_SUCCESS，其他见hal_error.h
*****************************************************************************/
uint32_t spi_slave_read(uint8_t *buf,uint16_t buf_size);
#endif

/*!****************************************************************************
\brief            从机写数据给到主机，
\param[in] 
    buf         要写的数据
    buf_size    要写数据的长度
    fun         告知上层应用数据已写完的回调
\param[out] 
\retval     成功返回HAL_SUCCESS，其他见hal_error.h
*****************************************************************************/
uint32_t spi_slave_write_to_master(uint8_t *buf,uint16_t buf_size, p_spi_s2m_write_complite_callback_t fun);

void spi_slave_receive_data(uint16_t length);

#endif //__SPI_SLAVE_HAL_H__
