#ifndef _DRIVER_TRIMING_H
#define _DRIVER_TRIMING_H

#include <stdint.h>

#include "driver_pmu.h"
#include "driver_adc.h"

/*********************************************************************
 * @fn      triming_bat_cap
 *
 * @brief   convert sample value to actual battery capacity
 *          Usage:
 *          ool_write(PMU_REG_AUXADC_PWR_EN, ool_read(PMU_REG_AUXADC_PWR_EN)|BIT7);
 *          adc_init(ADC_CHANNEL_VBAT, 0, ADC_REF_SEL_INTERNAL_1_2v);
 *          adc_start();
 *          co_delay_100us(10);
 *          uint16_t adc_value = adc_read_data();
 *          printf("battery cap is %d.\r\n", triming_bat_cap(ADC_REF_SEL_INTERNAL_1_2v, adc_value));
 *          adc_stop();
 *
 * @param   ref_sel      - reference selection when execute ADC sample.
 *          sample value - 
 *
 * @return  actual battery capacity, unit mV.
 */
uint16_t triming_bat_cap(enum adc_ref_sel_t ref_sel, uint16_t sample_value);

/*********************************************************************
 * @fn      triming_set_charge
 *
 * @brief   set charge relevant parameters, charge current and charge cut off value.
 *          Usage: triming_set_charge(CHG_CUR_100MA, 4200)
 *
 * @param   cur             - charge current configuration, @ref charge_current_t.
 *          charge_cut_off_value - charge cut off voltage, unit: mV
 *
 * @return  None.
 */
void triming_set_charge(enum charge_current_t cur, uint16_t charge_cut_off_value);

/*********************************************************************
 * @fn      triming_adkey_value
 *
 * @brief   caculate reference value for specified pull up and pull down resistance in ADKey application.
 *          This function should be called after triming_init is callded.
 *          Usage: 
 *              ref_value = triming_adkey_value(10000, 3000, ADC_REF_SEL_AVDD);
 *          Then this key is supposed to be pressed when sample value is between ref_value-%10*ref_value to
 *          ref_value+%10*ref_value.
 *
 * @param   pull_up_res_value   - pull up resistance value (internal pull up resistance is 10k).
 *          adkey_res_value     - pull down value when key is pressed.
 *          ref_sel             - reference selection for ADC core, @ref adc_ref_sel_t
 *
 * @return  reference value for pressed key judgement.
 */
uint16_t triming_adkey_value(uint32_t pull_up_res_value, uint32_t adkey_res_value, enum adc_ref_sel_t ref_sel);

/*********************************************************************
 * @fn      triming_get_avdd_param
 *
 * @brief   get triming parameter when avdd is used as ADC referance.
 *          Usage:
 *              uint16_t ref_value, ref_offset;
 *              triming_get_avdd_param(&ref_value, &ref_offset);
 *              adc_init(ADC_CHANNEL_PB4, 0, ADC_REF_SEL_AVDD);
 *              adc_start();
 *              co_delay_100us(10);
 *              value = adc_read_data();
 *              printf("adc voltage = %d\r\n",((value*ref_value)>>10)+ref_offset);
 *              adc_stop();
 *
 * @param   avdd_value  - .
 *          avdd_offset _ .
 *
 * @return  None.
 */
void triming_get_avdd_param(uint16_t *avdd_value, uint16_t *avdd_offset);

/*********************************************************************
 * @fn      triming_get_internal_ref_param  
 *
 * @brief   get triming parameter when avdd is used as ADC referance.
 *          Usage:
 *              uint16_t ref_value, ref_offset;
 *              triming_get_internal_ref_param(&ref_value, &ref_offset);
 *              adc_init(ADC_CHANNEL_PB4, 0, ADC_REF_SEL_INTERNAL_1_2v);
 *              adc_start();
 *              co_delay_100us(10);
 *              value = adc_read_data();
 *              printf("adc value = %d\r\n",((value*ref_value)>>10)+ref_offset);
 *              adc_stop();
 *
 * @param   int_ref_value  - .
 *          int_ref_offset _ .
 *
 * @return  None.
 */
void triming_get_internal_ref_param(uint16_t *int_ref_value, uint16_t *int_ref_offset);

/*********************************************************************
 * @fn      triming_init
 *
 * @brief   generate triming value for ADC calibration, this function should be called before ADC module usage.
 *
 * @param   None.
 *
 * @return  None.
 */
void triming_init(void);

#endif  // _DRIVER_TRIMINGH

