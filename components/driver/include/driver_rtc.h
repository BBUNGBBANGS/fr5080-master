#ifndef _DRIVER_RTC_H
#define _DRIVER_RTC_H
#include "stdint.h"

void rtc_init(void);
void rtc_start_alarm(uint32_t count_ms);
void rtc_stop_alarm(void);


#endif
