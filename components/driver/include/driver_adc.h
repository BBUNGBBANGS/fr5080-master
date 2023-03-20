#ifndef _DRIVER_ADC_H
#define _DRIVER_ADC_H
#include "stdint.h"
/*
 * TYPEDEFS
 */
enum adc_mode_t {
    ADC_MODE_LOOP,
    ADC_MODE_FIXED,
};

enum adc_channel_func_t{
    ADC_CHANNEL_PB4,    //can be used as adkey
    ADC_CHANNEL_PB5,    //can be used as adkey
    ADC_CHANNEL_PB6,
    ADC_CHANNEL_PB7,
    ADC_CHANNEL_RESV0,
    ADC_CHANNEL_RESV1,
    ADC_CHANNEL_VBE,
    ADC_CHANNEL_VBAT,
};

enum adc_fifo_int_type_t {
    ADC_FF_FULL = (1<<0),
    ADC_FF_HFULL = (1<<1),
    ADC_FF_EMPTY = (1<<2),
    ADC_FF_HEMPTY = (1<<3),
    ADC_ERROR = (1<<4),
};

enum adc_fifo_int_status_t {
    ADC_FF_STATUS_FULL = (1<<8),
    ADC_FF_STATUS_HFULL = (1<<9),
    ADC_FF_STATUS_EMPTY = (1<<10),
    ADC_FF_STATUS_HEMPTY = (1<<11),
    ADC_STATUS_ERROR = (1<<12),
};

enum adc_ref_sel_t {
    ADC_REF_SEL_AVDD,
    ADC_REF_SEL_INTERNAL_1_2v,
};
    
/*********************************************************************
 * @fn      adc_init
 *
 * @brief   init AD convert.
 *
 * @param   channel_func    - select channel function,reference enum adc_channel_func_t.
 *                        
 *          fifo_en        -  enable fifo or not
 *          ref_sel        -  //1---1.2v, 0---vddio
 *          
 * @return  None.
 */
void adc_init(enum adc_channel_func_t channel_func, uint8_t fifo_en, enum adc_ref_sel_t ref_sel);

/*********************************************************************
 * @fn      adc_start
 *
 * @brief   start AD convert.
 *
 * @param   None
 *			
 * @return  None.
 */
void adc_start(void);

/*********************************************************************
 * @fn      adc_stop
 *
 * @brief   stop AD convert.
 *
 * @param   None
 *			
 * @return  None.
 */
void adc_stop(void);

/*********************************************************************
 * @fn      adc_enable_fifo
 *
 * @brief   enable adc fifo, fifo should not be enable in LOOP mode.
 *
 * @param   int_type    - represent which interrupts should be enable
 *			
 * @return  None.
 */
void adc_enable_fifo(uint8_t int_type);

/*********************************************************************
 * @fn      adc_disable_fifo
 *
 * @brief   disable adc fifo.
 *
 * @param   None
 *			
 * @return  None.
 */
void adc_disable_fifo(void);

/*********************************************************************
 * @fn      adc_get_fifo_status
 *
 * @brief   get current fifo status.
 *
 * @param   None
 *			
 * @return  status, only bit[12:8] are avaliable.
 */
uint32_t adc_get_fifo_status(void);

/*********************************************************************
 * @fn      is_adc_data_valid
 *
 * @brief   check if data is valid.
 *
 * @param   None
 *			
 * @return  valid or not.
 */
uint8_t is_adc_data_valid(void);

/*********************************************************************
 * @fn      adc_read_data
 *
 * @brief   read fifo value.
 *
 * @param   None.
 *			
 * @return  fifo value.
 */
uint16_t adc_read_data(void);

#endif  // _ADC_H

