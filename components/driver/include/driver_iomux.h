/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */
#ifndef DRIVER_IOMUX_H
#define DRIVER_IOMUX_H

#define SYSTEM_PORT_MUX_MSK     0xF
#define SYSTEM_PORT_MUX_LEN     4

#define PORTA0_FUNC_A0                  0x00
#define PORTA0_FUNC_DSP_IO0             0x01
#define PORTA0_FUNC_COL0                0x02
#define PORTA0_FUNC_I2C0_SCL            0x03
#define PORTA0_FUNC_SSP_SCLK            0x04
#define PORTA0_FUNC_PWM0                0x05
#define PORTA0_FUNC_PDM0_SCK            0x06
#define PORTA0_FUNC_I2S_SCLK            0x07
#define PORTA0_FUNC_SDC_CLK             0x08
#define PORTA0_FUNC_CM3_UART_RX         0x09
#define PORTA0_FUNC_PWM1_INV            0x0b
#define PORTA0_FUNC_QSPI1_SCLK          0x0c


#define PORTA1_FUNC_A1                  0x00
#define PORTA1_FUNC_DSP_IO1             0x01
#define PORTA1_FUNC_COL1                0x02
#define PORTA1_FUNC_I2C0_SDA            0x03
#define PORTA1_FUNC_SSP_CSN             0x04
#define PORTA1_FUNC_PWM1                0x05
#define PORTA1_FUNC_PDM0_SDA            0x06
#define PORTA1_FUNC_I2S_FRM             0x07
#define PORTA1_FUNC_SDC_CMD             0x08
#define PORTA1_FUNC_CM3_UART_TX         0x09
#define PORTA1_FUNC_PWM0_INV            0x0b
#define PORTA1_FUNC_QSPI1_CS            0x0c


#define PORTA2_FUNC_A2                  0x00
#define PORTA2_FUNC_DSP_IO2             0x01
#define PORTA2_FUNC_COL2                0x02
#define PORTA2_FUNC_I2C1_SCL            0x03
#define PORTA2_FUNC_SSP_MOSI            0x04
#define PORTA2_FUNC_PWM2                0x05
#define PORTA2_FUNC_PDM1_SCK            0x06
#define PORTA2_FUNC_I2S_MOSI            0x07
#define PORTA2_FUNC_SDC_DAT0            0x08
#define PORTA2_FUNC_PWM3_INV            0x0b
#define PORTA2_FUNC_QSPI1_MISO0         0x0c

#define PORTA3_FUNC_A3                  0x00
#define PORTA3_FUNC_DSP_IO3             0x01
#define PORTA3_FUNC_COL3                0x02
#define PORTA3_FUNC_I2C1_SDA            0x03
#define PORTA3_FUNC_SSP_MISO            0x04
#define PORTA3_FUNC_PWM3                0x05
#define PORTA3_FUNC_PDM1_SDA            0x06
#define PORTA3_FUNC_I2S_MISO            0x07
#define PORTA3_FUNC_SDC_DAT1            0x08
#define PORTA3_FUNC_PWM2_INV            0x0b
#define PORTA3_FUNC_QSPI1_MISO1         0x0c

#define PORTA4_FUNC_A4                  0x00
#define PORTA4_FUNC_DSP_IO4             0x01
#define PORTA4_FUNC_COL4                0x02
#define PORTA4_FUNC_I2C0_SCL            0x03
#define PORTA4_FUNC_SSP_SCLK            0x04
#define PORTA4_FUNC_PWM4                0x05
#define PORTA4_FUNC_PDM0_SCK            0x06
#define PORTA4_FUNC_I2S_SCLK            0x07
#define PORTA4_FUNC_SDC_DAT2            0x08
#define PORTA4_FUNC_CDC_I2S_CLK         0x0a
#define PORTA4_FUNC_PWM5_INV            0x0b
#define PORTA4_FUNC_QSPI1_MISO2         0x0c

#define PORTA5_FUNC_A5                  0x00
#define PORTA5_FUNC_DSP_IO5             0x01
#define PORTA5_FUNC_COL5                0x02
#define PORTA5_FUNC_I2C0_SDA            0x03
#define PORTA5_FUNC_SSP_CSN             0x04
#define PORTA5_FUNC_PWM5                0x05
#define PORTA5_FUNC_PDM0_SDA            0x06
#define PORTA5_FUNC_I2S_FRM             0x07
#define PORTA5_FUNC_SDC_DAT3            0x08
#define PORTA5_FUNC_CDC_I2S_FRM         0x0a
#define PORTA5_FUNC_PWM4_INV            0x0b
#define PORTA5_FUNC_QSPI1_MISO3         0x0c

#define PORTA6_FUNC_A6                  0x00
#define PORTA6_FUNC_DSP_IO6             0x01
#define PORTA6_FUNC_COL6                0x02
#define PORTA6_FUNC_I2C1_SCL            0x03
#define PORTA6_FUNC_SSP_MOSI            0x04
#define PORTA6_FUNC_PWM0                0x05
#define PORTA6_FUNC_PDM1_SCK            0x06
#define PORTA6_FUNC_I2S_MOSI            0x07
#define PORTA6_FUNC_DSP_UART_RXD        0x09
#define PORTA6_FUNC_CDC_I2S_MOSI        0x0a
#define PORTA6_FUNC_PWM1_INV            0x0b
#define PORTA6_FUNC_CM3_UART_RXD        0x0c

#define PORTA7_FUNC_A7                  0x00
#define PORTA7_FUNC_DSP_IO7             0x01
#define PORTA7_FUNC_COL7                0x02
#define PORTA7_FUNC_I2C1_SDA            0x03
#define PORTA7_FUNC_SSP_MISO            0x04
#define PORTA7_FUNC_PWM1                0x05
#define PORTA7_FUNC_PDM1_SDA            0x06
#define PORTA7_FUNC_I2S_MISO            0x07
#define PORTA7_FUNC_DSP_UART_TXD        0x09
#define PORTA7_FUNC_CDC_I2S_MISO        0x0a
#define PORTA7_FUNC_PWM0_INV            0x0b
#define PORTA7_FUNC_CM3_UART_TXD        0x0c


#define PORTB0_FUNC_B0                  0x00
#define PORTB0_FUNC_DSP_IO0             0x01
#define PORTB0_FUNC_COL8                0x02
#define PORTB0_FUNC_I2C1_SCL            0x03
#define PORTB0_FUNC_SSP_SCLK            0x04
#define PORTB0_FUNC_PWM2                0x05
#define PORTB0_FUNC_PDM0_SCK            0x06
#define PORTB0_FUNC_I2S_SCLK            0x07
#define PORTB0_FUNC_CDC_I2S_SCLK        0x09
#define PORTB0_FUNC_PWM3_INV            0x0b


#define PORTB1_FUNC_B1                  0x00
#define PORTB1_FUNC_DSP_IO1             0x01
#define PORTB1_FUNC_COL9                0x02
#define PORTB1_FUNC_I2C1_SDA            0x03
#define PORTB1_FUNC_SSP_CSN             0x04
#define PORTB1_FUNC_PWM3                0x05
#define PORTB1_FUNC_PDM0_SDA            0x06
#define PORTB1_FUNC_I2S_FRM             0x07
#define PORTB1_FUNC_CLK_OUT             0x08
#define PORTB1_FUNC_CDC_I2S_FRM         0x09
#define PORTB1_FUNC_PWM2_INV            0x0b


#define PORTB2_FUNC_B2                  0x00
#define PORTB2_FUNC_DSP_IO2             0x01
#define PORTB2_FUNC_COL10               0x02
#define PORTB2_FUNC_I2C0_SCL            0x03
#define PORTB2_FUNC_SSP_MOSI            0x04
#define PORTB2_FUNC_PWM4                0x05
#define PORTB2_FUNC_PDM1_SCK            0x06
#define PORTB2_FUNC_I2S_MOSI            0x07
#define PORTB2_FUNC_CDC_I2S_MOSI        0x09
#define PORTB2_FUNC_PWM5_INV            0x0b

#define PORTB3_FUNC_B3                  0x00
#define PORTB3_FUNC_DSP_IO3             0x01
#define PORTB3_FUNC_COL11               0x02
#define PORTB3_FUNC_I2C0_SDA            0x03
#define PORTB3_FUNC_SSP_MISO            0x04
#define PORTB3_FUNC_PWM5                0x05
#define PORTB3_FUNC_PDM1_SDA            0x06
#define PORTB3_FUNC_I2S_MISO            0x07
#define PORTB3_FUNC_CLK_OUT             0x08
#define PORTB3_FUNC_CDC_I2S_MISO        0x09
#define PORTB3_FUNC_PWM4_INV            0x0b

#define PORTB4_FUNC_B4                  0x00
#define PORTB4_FUNC_DSP_IO4             0x01
#define PORTB4_FUNC_COL16               0x02
#define PORTB4_FUNC_I2C1_SCL            0x03
#define PORTB4_FUNC_SSP_SCLK            0x04
#define PORTB4_FUNC_PWM0                0x05
#define PORTB4_FUNC_PDM0_SCK            0x06
#define PORTB4_FUNC_I2S_SCLK            0x07
#define PORTB4_FUNC_SARADC0             0x08
#define PORTB4_FUNC_PWM1_INV            0x0b
#define PORTB4_FUNC_CLK_OUT             0x0c

#define PORTB5_FUNC_B5                  0x00
#define PORTB5_FUNC_DSP_IO5             0x01
#define PORTB5_FUNC_COL17               0x02
#define PORTB5_FUNC_I2C1_SDA            0x03
#define PORTB5_FUNC_SSP_CSN             0x04
#define PORTB5_FUNC_PWM1                0x05
#define PORTB5_FUNC_PDM0_SDA            0x06
#define PORTB5_FUNC_I2S_FRM             0x07
#define PORTB5_FUNC_SARADC1             0x08
#define PORTB5_FUNC_PWM0_INV            0x0b
#define PORTB5_FUNC_CLK_OUT             0x0c

#define PORTB6_FUNC_B6                  0x00
#define PORTB6_FUNC_DSP_IO6             0x01
#define PORTB6_FUNC_COL18               0x02
#define PORTB6_FUNC_I2C0_SCL            0x03
#define PORTB6_FUNC_SSP_MOSI            0x04
#define PORTB6_FUNC_PWM2                0x05
#define PORTB6_FUNC_PDM1_SCK            0x06
#define PORTB6_FUNC_I2S_MOSI            0x07
#define PORTB6_FUNC_SARADC2             0x08
#define PORTB6_FUNC_CM3_UART_RXD        0x09
#define PORTB6_FUNC_PWM3_INV            0x0b
#define PORTB6_FUNC_USB_DP              0x0c

#define PORTB7_FUNC_B7                  0x00
#define PORTB7_FUNC_DSP_IO7             0x01
#define PORTB7_FUNC_COL19               0x02
#define PORTB7_FUNC_I2C0_SDA            0x03
#define PORTB7_FUNC_SSP_MISO            0x04
#define PORTB7_FUNC_PWM3                0x05
#define PORTB7_FUNC_PDM1_SDA            0x06
#define PORTB7_FUNC_I2S_MISO            0x07
#define PORTB7_FUNC_CM3_UART_TXD        0x09
#define PORTB7_FUNC_PWM2_INV            0x0b
#define PORTB7_FUNC_USB_DM              0x0c


#define PORTC0_FUNC_C0                  0x00
#define PORTC0_FUNC_DSP_IO0             0x01
#define PORTC0_FUNC_COL12               0x02
#define PORTC0_FUNC_I2C1_SCL            0x03
#define PORTC0_FUNC_SSP_SCLK            0x04
#define PORTC0_FUNC_PWM4                0x05
#define PORTC0_FUNC_PDM0_SCK            0x06
#define PORTC0_FUNC_I2S_SCLK            0x07
#define PORTC0_FUNC_DSP_JTAG_TCK        0x08
#define PORTC0_FUNC_CDC_I2S_SCLK        0x09
#define PORTC0_FUNC_SDC_CLK             0x0A
#define PORTC0_FUNC_PWM5_INV            0x0b
#define PORTC0_FUNC_QSPI1_SCLK          0x0c


#define PORTC1_FUNC_C1                  0x00
#define PORTC1_FUNC_DSP_IO1             0x01
#define PORTC1_FUNC_COL13               0x02
#define PORTC1_FUNC_I2C1_SDA            0x03
#define PORTC1_FUNC_SSP_CSN             0x04
#define PORTC1_FUNC_PWM5                0x05
#define PORTC1_FUNC_PDM0_SDA            0x06
#define PORTC1_FUNC_I2S_FRM             0x07
#define PORTC1_FUNC_DSP_JTAG_TMS        0x08
#define PORTC1_FUNC_CDC_I2S_FRM         0x09
#define PORTC1_FUNC_SDC_CMD             0x0A
#define PORTC1_FUNC_PWM4_INV            0x0b
#define PORTC1_FUNC_QSPI1_CS            0x0c


#define PORTC2_FUNC_C2                  0x00
#define PORTC2_FUNC_DSP_IO2             0x01
#define PORTC2_FUNC_COL14               0x02
#define PORTC2_FUNC_I2C0_SCL            0x03
#define PORTC2_FUNC_SSP_MOSI            0x04
#define PORTC2_FUNC_PWM0                0x05
#define PORTC2_FUNC_PDM1_SCK            0x06
#define PORTC2_FUNC_I2S_MOSI            0x07
#define PORTC2_FUNC_DSP_JTAG_TDO        0x08
#define PORTC2_FUNC_CDC_I2S_MOSI        0x09
#define PORTC2_FUNC_SCD_DAT0            0x0A
#define PORTC2_FUNC_PWM1_INV            0x0b
#define PORTC2_FUNC_QSPI1_MISO0         0x0c

#define PORTC3_FUNC_C3                  0x00
#define PORTC3_FUNC_DSP_IO3             0x01
#define PORTC3_FUNC_COL15               0x02
#define PORTC3_FUNC_I2C0_SDA            0x03
#define PORTC3_FUNC_SSP_MISO            0x04
#define PORTC3_FUNC_PWM1                0x05
#define PORTC3_FUNC_PDM1_SDA            0x06
#define PORTC3_FUNC_I2S_MISO            0x07
#define PORTC3_FUNC_DSP_JTAG_TDI        0x08
#define PORTC3_FUNC_CDC_I2S_MISO        0x09
#define PORTC3_FUNC_SDC_DAT1            0x0A
#define PORTC3_FUNC_PWM0_INV            0x0b
#define PORTC3_FUNC_QSPI1_MISO1         0x0c

#define PORTC4_FUNC_C4                  0x00
#define PORTC4_FUNC_DSP_IO4             0x01
#define PORTC4_FUNC_COL16               0x02
#define PORTC4_FUNC_I2C1_SCL            0x03
#define PORTC4_FUNC_SSP_SCLK            0x04
#define PORTC4_FUNC_PWM2                0x05
#define PORTC4_FUNC_PDM0_SCK            0x06
#define PORTC4_FUNC_I2S_SCLK            0x07
#define PORTC4_FUNC_CLK_OUT             0x08
#define PORTC4_FUNC_SDC_DAT2            0x0a
#define PORTC4_FUNC_PWM3_INV            0x0b
#define PORTC4_FUNC_QSPI1_MISO2         0x0c

#define PORTC5_FUNC_C5                  0x00
#define PORTC5_FUNC_DSP_IO5             0x01
#define PORTC5_FUNC_COL17               0x02
#define PORTC5_FUNC_I2C1_SDA            0x03
#define PORTC5_FUNC_SSP_CSN             0x04
#define PORTC5_FUNC_PWM3                0x05
#define PORTC5_FUNC_PDM0_SDA            0x06
#define PORTC5_FUNC_I2S_FRM             0x07
#define PORTC5_FUNC_CM3_SWV             0x08
#define PORTC5_FUNC_SDC_DAT3            0x0a
#define PORTC5_FUNC_PWM2_INV            0x0b
#define PORTC5_FUNC_QSPI1_MISO3         0x0c

#define PORTC6_FUNC_C6                  0x00
#define PORTC6_FUNC_DSP_IO6             0x01
#define PORTC6_FUNC_COL18               0x02
#define PORTC6_FUNC_I2C0_SCL            0x03
#define PORTC6_FUNC_SSP_MOSI            0x04
#define PORTC6_FUNC_PWM4                0x05
#define PORTC6_FUNC_PDM1_SCK            0x06
#define PORTC6_FUNC_I2S_MOSI            0x07
#define PORTC6_FUNC_CM3_SWCK            0x08
#define PORTC6_FUNC_DSP_UART_RXD        0x09
#define PORTC6_FUNC_CLK_OUT             0x0a
#define PORTC6_FUNC_PWM5_INV            0x0b

#define PORTC7_FUNC_C7                  0x00
#define PORTC7_FUNC_DSP_IO7             0x01
#define PORTC7_FUNC_COL19               0x02
#define PORTC7_FUNC_I2C0_SDA            0x03
#define PORTC7_FUNC_SSP_MISO            0x04
#define PORTC7_FUNC_PWM5                0x05
#define PORTC7_FUNC_PDM1_SDA            0x06
#define PORTC7_FUNC_I2S_MISO            0x07
#define PORTC7_FUNC_CM3_SWD             0x08
#define PORTC7_FUNC_DSP_UART_TXD        0x09
#define PORTC7_FUNC_CLK_OUT             0x0a
#define PORTC7_FUNC_PWM4_INV            0x0b


#define PORTD0_FUNC_D0                  0x00
#define PORTD0_FUNC_DSP_IO0             0x01
#define PORTD0_FUNC_COL0                0x02
#define PORTD0_FUNC_I2C1_SCL            0x03
#define PORTD0_FUNC_SSP_SCLK            0x04
#define PORTD0_FUNC_PWM0                0x05
#define PORTD0_FUNC_PDM0_SCK            0x06
#define PORTD0_FUNC_I2S_SCLK            0x07
#define PORTD0_FUNC_CLK_OUT             0x08
#define PORTD0_FUNC_DSP_UART_RXD        0x09
#define PORTD0_FUNC_PWM1_INV            0x0b


#define PORTD1_FUNC_D1                  0x00
#define PORTD1_FUNC_DSP_IO1             0x01
#define PORTD1_FUNC_COL1                0x02
#define PORTD1_FUNC_I2C1_SDA            0x03
#define PORTD1_FUNC_SSP_CSN             0x04
#define PORTD1_FUNC_PWM1                0x05
#define PORTD1_FUNC_PDM0_SDA            0x06
#define PORTD1_FUNC_I2S_FRM             0x07
#define PORTD1_FUNC_CLK_OUT             0x08
#define PORTD1_FUNC_DSP_UART_TXD        0x09
#define PORTD1_FUNC_PWM0_INV            0x0b


#define PORTD2_FUNC_D2                  0x00
#define PORTD2_FUNC_DSP_IO2             0x01
#define PORTD2_FUNC_COL2                0x02
#define PORTD2_FUNC_I2C0_SCL            0x03
#define PORTD2_FUNC_SSP_MOSI            0x04
#define PORTD2_FUNC_PWM2                0x05
#define PORTD2_FUNC_PDM1_SCK            0x06
#define PORTD2_FUNC_I2S_MOSI            0x07
#define PORTD2_FUNC_I2S_SDC_CLK         0x08
#define PORTD2_FUNC_PWM3_INV            0x0b
#define PORTD2_FUNC_QSPI1_SCLK          0x0C

#define PORTD3_FUNC_D3                  0x00
#define PORTD3_FUNC_DSP_IO3             0x01
#define PORTD3_FUNC_COL3                0x02
#define PORTD3_FUNC_I2C0_SDA            0x03
#define PORTD3_FUNC_SSP_MISO            0x04
#define PORTD3_FUNC_PWM3                0x05
#define PORTD3_FUNC_PDM1_SDA            0x06
#define PORTD3_FUNC_I2S_MISO            0x07
#define PORTD3_FUNC_SDC_CMD             0x08
#define PORTD3_FUNC_PWM2_INV            0x0b
#define PORTD3_FUNC_QSPI1_CS            0x0C

#define PORTD4_FUNC_D4                  0x00
#define PORTD4_FUNC_DSP_IO4             0x01
#define PORTD4_FUNC_COL4                0x02
#define PORTD4_FUNC_I2C1_SCL            0x03
#define PORTD4_FUNC_SSP_SCLK            0x04
#define PORTD4_FUNC_PWM4                0x05
#define PORTD4_FUNC_PDM0_SCK            0x06
#define PORTD4_FUNC_I2S_SCLK            0x07
#define PORTD4_FUNC_SDC_DAT0            0x08
#define PORTD4_FUNC_PWM5_INV            0x0b
#define PORTD4_FUNC_QSPI1_MISO0         0x0c

#define PORTD5_FUNC_D5                  0x00
#define PORTD5_FUNC_DSP_IO5             0x01
#define PORTD5_FUNC_COL5                0x02
#define PORTD5_FUNC_I2C1_SDA            0x03
#define PORTD5_FUNC_SSP_CSN             0x04
#define PORTD5_FUNC_PWM5                0x05
#define PORTD5_FUNC_PDM0_SDA            0x06
#define PORTD5_FUNC_I2S_FRM             0x07
#define PORTD5_FUNC_SDC_DAT1            0x08
#define PORTD5_FUNC_PWM4_INV            0x0b
#define PORTD5_FUNC_QSPI1_MISO1         0x0c

#define PORTD6_FUNC_D6                  0x00
#define PORTD6_FUNC_DSP_IO6             0x01
#define PORTD6_FUNC_COL6                0x02
#define PORTD6_FUNC_I2C0_SCL            0x03
#define PORTD6_FUNC_SSP_MOSI            0x04
#define PORTD6_FUNC_PWM0                0x05
#define PORTD6_FUNC_PDM1_SCK            0x06
#define PORTD6_FUNC_I2S_MOSI            0x07
#define PORTD6_FUNC_SDC_DAT2            0x08
#define PORTD6_FUNC_PWM1_INV            0x0b
#define PORTD6_FUNC_QSPI1_MISO2          0x0c

#define PORTD7_FUNC_D7                  0x00
#define PORTD7_FUNC_DSP_IO7             0x01
#define PORTD7_FUNC_COL7                0x02
#define PORTD7_FUNC_I2C0_SDA            0x03
#define PORTD7_FUNC_SSP_MISO            0x04
#define PORTD7_FUNC_PWM1                0x05
#define PORTD7_FUNC_PDM1_SDA            0x06
#define PORTD7_FUNC_I2S_MISO            0x07
#define PORTD7_FUNC_SDC_DAT3            0x08
#define PORTD7_FUNC_PWM0_INV            0x0b
#define PORTD7_FUNC_QSPI1_MISO3         0x0c



#define GPIO_PA0              (1<<0)
#define GPIO_PA1              (1<<1)
#define GPIO_PA2              (1<<2)
#define GPIO_PA3              (1<<3)
#define GPIO_PA4              (1<<4)
#define GPIO_PA5              (1<<5)
#define GPIO_PA6              (1<<6)
#define GPIO_PA7              (1<<7)

#define GPIO_PB0              (1<<8)
#define GPIO_PB1              (1<<9)
#define GPIO_PB2              (1<<10)
#define GPIO_PB3              (1<<11)
#define GPIO_PB4              (1<<12)
#define GPIO_PB5              (1<<13)
#define GPIO_PB6              (1<<14)
#define GPIO_PB7              (1<<15)

#define GPIO_PC0              (1<<16)
#define GPIO_PC1              (1<<17)
#define GPIO_PC2              (1<<18)
#define GPIO_PC3              (1<<19)
#define GPIO_PC4              (1<<20)
#define GPIO_PC5              (1<<21)
#define GPIO_PC6              (1<<22)
#define GPIO_PC7              (1<<23)

#define GPIO_PD0              (1<<24)
#define GPIO_PD1              (1<<25)
#define GPIO_PD2              (1<<26)
#define GPIO_PD3              (1<<27)
#define GPIO_PD4              (1<<28)
#define GPIO_PD5              (1<<29)
#define GPIO_PD6              (1<<30)
#define GPIO_PD7              (1<<31)


enum system_port_t
{
    GPIO_PORT_A,
    GPIO_PORT_B,
    GPIO_PORT_C,
    GPIO_PORT_D,
};

enum system_port_bit_t
{
    GPIO_BIT_0,
    GPIO_BIT_1,
    GPIO_BIT_2,
    GPIO_BIT_3,
    GPIO_BIT_4,
    GPIO_BIT_5,
    GPIO_BIT_6,
    GPIO_BIT_7,
};

#define GPIO_DIR_IN             1
#define GPIO_DIR_OUT            0

#endif
