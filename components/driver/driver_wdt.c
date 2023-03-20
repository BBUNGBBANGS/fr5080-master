/**
 * Copyright (c) 2019, Freqchip
 *
 * All rights reserved.
 *
 *
 */

/*
 * INCLUDES
 */
#include <stdint.h>

#include "co_log.h"
#include "driver_plf.h"
#include "user_utils.h"
#include "driver_plf.h"
#include "driver_wdt.h"

/*
 * MACROS
 */

/*********************************************************************
 * @fn      wdt_init
 *
 * @brief   init watchdog before enable this mudule.
 *
 * @param   action      - the next action after watchdog timer expired.
 *          delay_s     - how many seconds does the timer set
 *
 * @return  None.
 */
void wdt_init(uint8_t delay_s)
{
    uint32_t rc_clk = pmu_get_rc_clk();

    ool_write32(PMU_REG_WD_VALUE0, delay_s * rc_clk );

    ool_write(PMU_REG_RST_EN, ool_read(PMU_REG_RST_EN) | PMU_RST_WD_RST_EN );
}

/*********************************************************************
 * @fn      wdt_feed
 *
 * @brief   feed the watchdog.
 *
 * @param   None.
 *
 * @return  None.
 */
void wdt_feed(void)
{
    GLOBAL_INT_DISABLE();
    ool_write(PMU_REG_WD_VALUE3, ool_read(PMU_REG_WD_VALUE3) | PMU_WD_CLR );
    co_delay_10us(5);
    ool_write(PMU_REG_WD_VALUE3, ool_read(PMU_REG_WD_VALUE3) & (~PMU_WD_CLR) );
    GLOBAL_INT_RESTORE();
}

/*********************************************************************
 * @fn      wdt_start
 *
 * @brief   start the watchdog after init this mudle.
 *
 * @param   None.
 *
 * @return  None.
 */
void wdt_start(void)
{
    wdt_feed();
    ool_write(PMU_REG_WD_VALUE3, ool_read(PMU_REG_WD_VALUE3) | PMU_WD_EN );
}

/*********************************************************************
 * @fn      wdt_stop
 *
 * @brief   stop the watchdog.
 *
 * @param   None.
 *
 * @return  None.
 */
void wdt_stop(void)
{
    ool_write(PMU_REG_WD_VALUE3, ool_read(PMU_REG_WD_VALUE3) & (~ PMU_WD_EN) );
}
