#include <stdint.h>
#include <stdio.h>

#include "type_lib.h"
#include "driver_uart.h"
#include "driver_i2s.h"
#include "driver_syscntl.h"
#include "driver_plf.h"
#include "test_drivers.h"

#if TEST_DRIVER_I2S
uint16_t pcm_sin_wave_1k[]={0x5175,0x0000,0xae8a,0x8ccc,0xae8a,0xffff,0x5175,0x7333};
void test_i2s(void)
{
    printf("test i2s.");
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PORTA0_FUNC_I2S_SCLK);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_I2S_FRM);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_I2S_MOSI);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_I2S_MISO);
    
    i2s_init_(I2S_DIR_TX, 2000000, 8000, I2S_MODE_MASTER);//i2s tx, sample rate = 8000, i2s clk = 2000000, master mode
    i2s_start_();
    system_regs->misc_ctrl.audio_format = 1;
    system_regs->misc_ctrl.audio_dst = 1;
    NVIC_SetPriority(I2S_IRQn, 2);
    NVIC_EnableIRQ(I2S_IRQn);
}

__attribute__((section("ram_code")))void i2s_isr_ram(void)
{
    uint8_t i;
    uint32_t last;

    last = REG_PL_RD(I2S_REG_STATUS);
    if(last & bmSTATUS_TXFFHEMPTY) {
        for(i=0; i<32; i++) {
            REG_PL_WR(I2S_REG_DATA, pcm_sin_wave_1k[i%8]);
        }
    }
    
}
#endif


#if TEST_DRIVER_ADC
#include "driver_adc.h"
#include "driver_pmu.h"
#include "user_utils.h"
void test_adc(void)
{
    printf("test adc\r\n");
    #if TEST_ADC_IO
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_4, PORTB4_FUNC_SARADC0);
    pmu_set_ioldo_voltage(PMU_ALDO_VOL_3_3);
    adc_init(ADC_CHANNEL_PB4,0,0);
    #elif TEST_ADC_VBAT
    pmu_set_ioldo_voltage(PMU_ALDO_VOL_3_3);
    adc_init(ADC_CHANNEL_VBAT,0,0);
    #endif

   
    ool_write(PMU_REG_AUXADC_PWR_EN, ool_read(PMU_REG_AUXADC_PWR_EN)|BIT7);
    adc_start();
    
    while(is_adc_data_valid() == 1){
        co_delay_100us(10000);
        adc_stop();
        adc_start();
        co_delay_100us(1);
        printf("adc value = %x\r\n",adc_read_data());
    }
}
#endif

#if TEST_DRIVER_ADKEY
#include "driver_adc.h"
#include "driver_pmu.h"
uint8_t test_adkey_step = 0;
void test_adkey(void)
{
    printf("test adc\r\n");
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_4, PORTB4_FUNC_SARADC0);
    pmu_set_ioldo_voltage(PMU_ALDO_VOL_3_3);
    adc_init(ADC_CHANNEL_PB4,0,0);
    ool_write(PMU_REG_AUXADC_PWR_EN, ool_read(PMU_REG_AUXADC_PWR_EN)|BIT7);//power on auxadc
    adc_start();
    
    ool_write(PMU_REG_ADKEY_CTRL,ool_read(PMU_REG_ADKEY_CTRL)|0x01);//adkey pb4 pull up
    ool_write(PMU_REG_AULOD_CTRL_0,ool_read(PMU_REG_AULOD_CTRL_0)|0x40); //enable adkey pb4 detect
    ool_write(PMU_REG_INT_EN_2,ool_read(PMU_REG_INT_EN_2)|0x02); //enable adkey pb4 interrupt
}

void test_vbat_and_adkey(void)
{
    system_set_pclk(SYSTEM_SYS_CLK_48M);
    //system_regs->clk_ctrl.adc_clk_div = 1;
    
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_4, PORTB4_FUNC_SARADC0);
    pmu_set_ioldo_voltage(PMU_ALDO_VOL_3_3);
    adc_init(ADC_CHANNEL_VBAT,0,0);
    ool_write(PMU_REG_AUXADC_PWR_EN, ool_read(PMU_REG_AUXADC_PWR_EN)|BIT7);
    adc_start();
    while(is_adc_data_valid() == 0);
    printf("A vbat adc value = %x\r\n",adc_read_data());
    adc_stop();

    //enable pb4 adkey interrupt
    ool_write(PMU_REG_ADKEY_CTRL,ool_read(PMU_REG_ADKEY_CTRL)|0x01);//adkey pb4 pull up
    ool_write(PMU_REG_AULOD_CTRL_0,ool_read(PMU_REG_AULOD_CTRL_0)|0x40); //enable adkey pb4 detect
    ool_write(PMU_REG_INT_EN_2,ool_read(PMU_REG_INT_EN_2)|0x02); //enable adkey pb4 interrupt
    while(test_adkey_step == 0);

    ool_write(PMU_REG_INT_EN_2,ool_read(PMU_REG_INT_EN_2)&0xfd); //enable adkey pb4 interrupt
    adc_init(ADC_CHANNEL_VBAT,0,0);
    adc_start();
    while(is_adc_data_valid() == 0);
    printf("B vbat adc value = %x\r\n",adc_read_data());
    adc_stop();
    ool_write(PMU_REG_INT_EN_2,ool_read(PMU_REG_INT_EN_2)|0x02); //enable adkey pb4 interrupt
}
#include "os_timer.h"
os_timer_t test_timer;
void test_timer_func(void *arg)
{
    uint16_t adc_val;
    adc_init(ADC_CHANNEL_PB4,0,0);
    adc_start();
    adc_read_data();
    while(is_adc_data_valid() == 0);
    
    adc_val = adc_read_data();
    printf("adkey 0 isr = %x\r\n",adc_val);
    adc_stop();
    
    if(adc_val > 0x300){
        ool_write(PMU_REG_INT_EN_2,ool_read(PMU_REG_INT_EN_2)|0x02); //enable adkey pb4 interrupt
    }else{
        os_timer_start(&test_timer,30,0);
    }
}
__attribute__((section("ram_code"))) void pmu_adkey0_isr_ram(void)
{
    
    ool_write(PMU_REG_INT_EN_2,ool_read(PMU_REG_INT_EN_2)&0xfd); //disenable adkey pb4 interrupt
    
    adc_init(ADC_CHANNEL_PB4,0,0);
    adc_start();
    while(is_adc_data_valid() == 0);

    printf("adkey 0 isr = %x\r\n",adc_read_data());
    //ool_write(PMU_REG_INT_EN_2,ool_read(PMU_REG_INT_EN_2)|0x02); //enable adkey pb4 interrupt
    test_adkey_step = 1;
    
    os_timer_init(&test_timer,test_timer_func,NULL);
    os_timer_start(&test_timer,30,0);
    
    adc_stop();
}

#endif

#if TEST_DRIVER_RTC
#include "driver_rtc.h"
uint8_t rtc_wait_isr_flag = 0;
void test_rtc(void) 
{
    printf("test rtc\r\n");
    rtc_init();
    printf("start 2s rtc,rc clk=%d\r\n",pmu_get_rc_clk());
    rtc_start_alarm(2000);
    
    while(rtc_wait_isr_flag == 0);
    printf("start 4s rtc\r\n");
    rtc_start_alarm(4000);
    rtc_wait_isr_flag = 0;
    while(rtc_wait_isr_flag == 0);
    rtc_stop_alarm();
    printf("rtc test end\r\n");
    
}

__attribute__((section("ram_code"))) void rtc_isr_ram()
{
    rtc_wait_isr_flag = 1;
}

#endif

#if TEST_DRIVER_SPI
#include "driver_ssp.h"

__attribute__((section("ram_code"))) void ssp_isr(void)
{
    uint32_t isr_status;
    uint8_t data[10];
    isr_status = ssp_get_isr_status();
    if(isr_status & SSP_INT_RX_FF) {
        printf("spi rx: \r\n");
        ssp_get_data_from_fifo(&data[0], 6);
        ssp_clear_isr_status(SSP_INT_RX_FF);
        for(uint8_t i = 0;i<6;i++){
            printf("%x,",data[i]);
        }
    }

    if(isr_status & SSP_INT_TX_FF) {
        
        printf("spi tx: \r\n");
        ssp_clear_isr_status(SSP_INT_TX_FF);
    }
}

void test_spi(void)
{
    system_set_port_pull_up(GPIO_PD0,true);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_0, 0x04);
    
  
    ssp_init(8, SSP_FRAME_MOTO, 1, 2250000, 2, 0);
    ssp_set_clk_type(1, 1);
    
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, 0x04);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, 0x04);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, 0x04);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, 0x04);

    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_0, 0);

    ssp_slave_output_disable();
    
    ssp_enable_interrupt(SSP_INT_RX_FF);
    ssp_set_ff_int_ctrl(6, 6);
    ssp_enable();

    NVIC_EnableIRQ(SSP_IRQn);
}

#endif

#if TEST_DRIVER_PWM
#include "driver_pwm.h"
void test_pwm(void)
{
    /*1k wave,high level occupy 70%*/
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_0, PORTC0_FUNC_PWM4);
    pwm_init(PWM_CHANNEL_4,1000,70);
    pwm_start(PWM_CHANNEL_4);
}

#endif

void test_drivers(void)
{
    printf("test 5080 drivers start.\r\n");
    
    #if TEST_DRIVER_I2S
        test_i2s();
    #endif
    #if TEST_DRIVER_ADC
        test_adc();
    #endif
    #if TEST_DRIVER_ADKEY
        //test_adkey();
        test_vbat_and_adkey();
    #endif
    #if TEST_DRIVER_RTC
        test_rtc();
    #endif
    #if TEST_DRIVER_SPI
        test_spi();
    #endif
    #if TEST_DRIVER_PWM
        test_pwm();
    #endif
    
}
