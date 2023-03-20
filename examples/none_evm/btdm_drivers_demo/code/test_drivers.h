#ifndef _TEST_DRIVERS_H
#define _TEST_DRIVERS_H

#define TEST_DRIVER_I2S 		0
#define TEST_DRIVER_ADC         0
    #define TEST_ADC_VBAT       0
    #define TEST_ADC_IO         0
#define TEST_DRIVER_RTC         0
#define TEST_DRIVER_SPI         0
#define TEST_DRIVER_ADKEY       0
#define TEST_DRIVER_PWM         1

void test_drivers(void);
#endif
