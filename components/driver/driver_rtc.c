#include "driver_pmu.h"

extern void fr_mul_64(uint32_t *low, uint32_t *high, uint32_t mul1, uint32_t mul2);
extern uint32_t fr_div_64(uint32_t low, uint32_t high, uint32_t div);

void rtc_init(void)
{
    ool_write(PMU_REG_RST_EN,ool_read(PMU_REG_RST_EN)&(~PMU_RST_RTC_ENN));
    ool_write(PMU_REG_RST_EN,ool_read(PMU_REG_RST_EN)| PMU_RST_RTC_ENN);

    ool_write(PMU_REG_CLK_EN,ool_read(PMU_REG_CLK_EN)|PMU_CLK_RTC_EN);
    ool_write32(PMU_REG_RTC_UPD_VAL0,0);
    ool_write(PMU_REG_RTC_CTRL,PMU_RTC_UPD_EN);
    co_delay_10us(7);
    ool_write(PMU_REG_RTC_CTRL,0);

    ool_write(PMU_REG_INT_EN_1,ool_read(PMU_REG_INT_EN_1)|PMU_INT_ALMA);
}

void rtc_start_alarm(uint32_t count_ms)
{
    uint32_t tmp_high,tmp_low;
    uint32_t tmp,cnt;

    fr_mul_64(&tmp_low, &tmp_high, pmu_get_rc_clk(), count_ms);
    cnt = fr_div_64(tmp_low, tmp_high, 1000);
    
    tmp = ool_read32(PMU_REG_RTC_UPD_VAL0);
    ool_write32(PMU_REG_RTC_ALMA_VAL0, cnt+tmp);

    ool_write(PMU_REG_RTC_CTRL,PMU_RTC_ALMA_EN);
}

void rtc_stop_alarm(void)
{
    ool_write(PMU_REG_RTC_CTRL,0);;
}

__attribute__((section("ram_code"))) void rtc_isr_ram(void)
{
    printf("rtc isr.\r\n");
}

