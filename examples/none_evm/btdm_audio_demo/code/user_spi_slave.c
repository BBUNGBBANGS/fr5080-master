#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "co_list.h"
#include "os_mem.h"
#include "os_timer.h"
#include "spi_slave_hal.h"
#include "user_utils.h"
#include "driver_ssp.h"
#include "driver_plf.h"
#include "driver_gpio.h"
#include "driver_exti.h"
#include "driver_uart.h"
#include "driver_syscntl.h"


typedef void (*spi_sent_callback)(uint8_t *buffer, uint16_t length);

enum spi_recv_state_t {
    SPI_RECV_STATE_RECV_IDLE,
    SPI_RECV_STATE_RECV_HEADER,
    SPI_RECV_STATE_RECV_PAYLOAD,
};

enum spi_send_state_t {
    SPI_SEND_STATE_SEND_IDLE,
    SPI_SEND_STATE_SEND_WAITING,
    SPI_SEND_STATE_SENDING,
};

struct spi_send_elt_t {
    struct co_list_hdr hdr;
    uint8_t *buffer;
    uint16_t length;
    spi_sent_callback callback;
};

enum spi_recv_state_t spi_recv_state = SPI_RECV_STATE_RECV_IDLE;
enum spi_send_state_t spi_send_state = SPI_SEND_STATE_SEND_IDLE;
bool spi_recv_req_pending = false;
struct co_list spi_send_list;
uint8_t user_salve_send_flag;
uint16_t user_salve_send_count;

extern uint8_t raw_data_send_flag;
extern void trigger_raw_data_send(void);

__attribute__((section("ram_code"))) void spi_send_recv_ready(void)
{
    gpio_portc_write(gpio_portc_read() | 0x04);
    gpio_set_dir(GPIO_PORT_C, GPIO_BIT_2, GPIO_DIR_OUT);
    gpio_portc_write(gpio_portc_read() | 0x04);
    co_delay_10us(1);
    gpio_portc_write(gpio_portc_read() & 0xfb);
    gpio_set_dir(GPIO_PORT_C, GPIO_BIT_2, GPIO_DIR_IN);
}

__attribute__((section("ram_code"))) void spi_recv_int_recover(void)
{
    ext_int_clear(GPIO_PC2);
    ext_int_enable(GPIO_PORT_C, GPIO_BIT_2);
}

__attribute__((section("ram_code"))) void exti_isr_ram(void)
{
    uint32_t status;
    struct spi_send_elt_t *elt;

    status = ext_int_get_status();
#if 0
    if(status & GPIO_PC2) {
        uart_putc_noint_no_wait('E');
        ext_int_disable(GPIO_PORT_C, GPIO_BIT_2);
        if(spi_recv_state == SPI_RECV_STATE_RECV_IDLE) {
            spi_recv_state = SPI_RECV_STATE_RECV_HEADER;
            //gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_6,1);
            spi_slave_receive_data(2);
            spi_send_recv_ready();
        }
        else {
            spi_recv_req_pending = true;
        }
    }

    if(status & GPIO_PC3) {
        //elt = (struct spi_send_elt_t *)co_list_pick(&spi_send_list);
        if(spi_recv_state == SPI_RECV_STATE_RECV_HEADER)
        {
            ssp_disable_interrupt(SSP_INT_RX_FF);
            spi_recv_int_recover();
            spi_recv_state = SPI_RECV_STATE_RECV_IDLE;
        }
        ext_int_disable(GPIO_PORT_C, GPIO_BIT_3);
        system_set_port_pull_up(GPIO_PC3, true);
        system_set_port_pull_down(GPIO_PC3, false);
        gpio_int_enable(GPIO_PORT_C, GPIO_BIT_3);
        spi_send_state = SPI_SEND_STATE_SENDING;
        //spi_slave_write_to_master(elt->buffer, elt->length, spi_sent);
        //spi_slave_write_to_master(test_data,SPI_SLAVE_TEST_LEN,NULL);
        user_salve_send_count = 0;
        user_salve_send_flag = 0;
    }
#endif
    ext_int_clear(status);
}

__attribute__((section("ram_code"))) static uint32_t spi_slave_receive_from_master_t(uint8_t *buf,uint16_t buf_size)
{
    static uint16_t packet_length;
    uint8_t recv_done = false;
    
    if(spi_recv_state == SPI_RECV_STATE_RECV_HEADER) {

        packet_length = (buf[1] << 8) | buf[0];
        //uart_putc_noint_no_wait((uint8_t)(packet_length>>8));
        //uart_putc_noint_no_wait((uint8_t)(packet_length&0xff));
        if(packet_length != 0) {
            spi_recv_state = SPI_RECV_STATE_RECV_PAYLOAD;
            spi_slave_receive_data(packet_length);
        }
        else {
            recv_done = true;
        }
    }
    else if(spi_recv_state == SPI_RECV_STATE_RECV_PAYLOAD) {
        recv_done = true;
        uart_putc_noint_no_wait('P');
        #if 0
        printf("packet_length: %d\r\n", packet_length);
        for(uint16_t i=0; i<buf_size; i++) {
            if((i % 16) == 0) {
                printf("\r\n");
            }
            printf("%02x ", buf[i]);
        }
        printf("\r\n");
        #else
        if(raw_data_send_flag == true){
            trigger_raw_data_send();
            raw_data_send_flag = false;
        }
        #endif
    }

    if(recv_done) {
        spi_recv_state = SPI_RECV_STATE_RECV_IDLE;
        if(spi_recv_req_pending) {
            spi_recv_state = SPI_RECV_STATE_RECV_HEADER;
            spi_slave_receive_data(2);
            //spi_send_recv_ready();
            spi_recv_req_pending = false;
        }
        else {
            //spi_recv_int_recover();
        }
    }

    return 0;
}
void spi_set_slave_rx_state_header(void)
{
    spi_recv_state = SPI_RECV_STATE_RECV_HEADER;
}
void spi_slave_api_init(void)
{
    uint16_t i;
    SPI_CONFIG_T config = {
        .mode = {
            .clk_mode = {
                .CLKPolarity = 1,
                .CLKPhase = 1,
            },
            .m_s_mode = SPI_SLAVE,
        },
        .freq = HAL_SPI_FREQ_2M,
        .data_size = 8,
        .tx_buf_size = 0,
        .rx_buf_size = 4*1024,
        .clk = {
            .portx = PORT_C,
            .pin = GPIO_BIT_4,
        },
        .cs = {
            .portx = PORT_C,
            .pin = GPIO_BIT_5,
        },
        .mosi = {
            .portx = PORT_C,
            .pin = GPIO_BIT_6,
        },
        .miso = {
            .portx = PORT_C,
            .pin = GPIO_BIT_7,
        },
    };

    IO_IRQ_DEF_T m2s_io_irq_cfg = {
        .portx = PORT_A,
        .pin = GPIO_BIT_5,
        .pin_irq = PIN_IRQ_FALLING,
    };

    IO_DEF_T s2m_io_cfg = {
        .portx = PORT_A,
        .pin = GPIO_BIT_4,
    };

    spi_slave_init(&config, spi_slave_receive_from_master_t);
#if 0
    /* 将PORTB0配置成低电平中断，用于接收主机数据的交互 */
    system_set_port_pull_up(GPIO_PC2, false);
    system_set_port_pull_down(GPIO_PC2, false);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_2, PORTC2_FUNC_C2);
    gpio_set_dir(GPIO_PORT_C, GPIO_BIT_2, GPIO_DIR_IN);
    gpio_int_enable(GPIO_PORT_C, GPIO_BIT_2);
    ext_int_set_control(GPIO_PORT_C, GPIO_BIT_2, 1000000, 10);
    ext_int_set_type(GPIO_PORT_C, GPIO_BIT_2, EXT_INT_TYPE_LOW);
    ext_int_enable(GPIO_PORT_C, GPIO_BIT_2);

    system_set_port_pull_up(GPIO_PC3, true);
    system_set_port_pull_down(GPIO_PC3, false);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_3, PORTC3_FUNC_C3);
    gpio_set_dir(GPIO_PORT_C, GPIO_BIT_3, GPIO_DIR_IN);
    gpio_int_enable(GPIO_PORT_C, GPIO_BIT_3);
    ext_int_set_control(GPIO_PORT_C, GPIO_BIT_3, 1000000, 10);
    ext_int_set_type(GPIO_PORT_C, GPIO_BIT_3, EXT_INT_TYPE_NEG);

    NVIC_EnableIRQ(GPIO_IRQn);
#endif
}

