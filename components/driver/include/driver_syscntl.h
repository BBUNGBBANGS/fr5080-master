#ifndef DRIVER_SYSCNTL_H_
#define DRIVER_SYSCNTL_H_

#include <stdint.h>
#include <stdbool.h>

#include "driver_iomux.h"

enum system_clk_t
{
    SYSTEM_SYS_CLK_48M,
    SYSTEM_SYS_CLK_24M,
    SYSTEM_SYS_CLK_16M,
    SYSTEM_SYS_CLK_12M,
};

enum system_dsp_clk_src_sel_t {
    SYSTEM_DSP_SRC_OSC_48M,         // dsp_master_clk = 48M, qspi_ref_clk = 48M
    SYSTEM_DSP_SRC_OSC_48M_DIV,     // dsp_master_clk = 48M/(2*(div+1)), qspi_ref_clk = 48M
    SYSTEM_DSP_SRC_208M,            // dsp_master_clk = 208/(2*(div+1)), qspi_ref_clk = 156M
    SYSTEM_DSP_SRC_312M,            // dsp_master_clk = 312/(2*(div+1)), qspi_ref_clk = 208M
};

enum system_dsp_mem_ctrl_t {
    SYSTEM_DSP_MEM_CTRL_I160_D256 = 0,
    SYSTEM_DSP_MEM_CTRL_I96_D320 = 1,
    SYSTEM_DSP_MEM_CTRL_I64_D352 = 3,
    SYSTEM_DSP_MEM_CTRL_I32_D384 = 7,
    SYSTEM_DSP_MEM_CTRL_I16_D400 = 15,
};

struct syscntl_clock_ctrl_t{
    uint32_t clk_bb_div:3;
    ///000---48M, other--- 48M/(clk_bb_div+1)
    uint32_t clk_lp_sel:1;
    ///0---rc 62.5K, 1---32K from osc24M divider
    uint32_t clk_gpio_sel:1;
    ///0---cm3 master clock,1---deep sleep clock
    uint32_t dsp_clk_sel:2;
    ///00---osc 48M,  01---osc 48M/(2*(dsp_mas_clk_div + 1)),
    ///10---spll 208M/(2*(dsp_mas_clk_div + 1)), 11---spll 312M/(2*(dsp_mas_clk_div + 1))
    uint32_t cm3_qspi_ref_clk_sel:1;
    ///0---48M clock,1---52M clock
    uint32_t out_clk_div:8;
    ///output clock divider
    uint32_t dsp_mas_clk_div:8;
    ///dsp master clock divider
    uint32_t clk_out_sel:1;
    ///0---osc/out_div, 1---32K from osc24M divider
    uint32_t adc_clk_sel:1;
    ///0---high speed clok from clock divider, 1---low speed clock from system lp_clk
    uint32_t adc_clk_div:4;
    ///48M/clkdiv
    uint32_t resv:1;
    uint32_t bb_phase_sel:1; 
    ///0---normal,1---48M clock negedge
};
///@0x50000004
struct syscntl_clock_enable_t{
    uint32_t cm3_mas_cken:1;
    uint32_t cm3_qspi_ref_cken:1;
    uint32_t dsp_mas_cken:1;
    uint32_t dsp_qspi_ref_cken:1;
    uint32_t cpu_bt_cken:1;
    uint32_t cpu_ble_cken:1;
    uint32_t cpu_btdm_cken:1;
    uint32_t cpu_btdm_em_cken:1;
    uint32_t i2s_mas_cken:1;
    uint32_t sbc_mas_cken:1;
    uint32_t efuse_mas_cken:1;
    uint32_t cdc_mas_cken:1;
    uint32_t gpio_mas_cken:1;
    uint32_t auxadc_mas_cken:1;
    uint32_t cm3_uart_cken:1;
    uint32_t dsp_uart_cken:1;
    uint32_t out_cken:1;
    uint32_t usb48_cken:1;
    uint32_t usb12_cken:1;
    uint32_t sdc_cken:1;
    uint32_t mdm_top_cken:1;
    uint32_t mdm_core_cken:1;
    uint32_t tws_mas_cken:1;
    uint32_t rsvd:9;
};
///@0x50000008
struct syscntl_sdc_ctrl_t{
    uint32_t sdc_clk_div:8;
    uint32_t sdc_s_dly:5;
    uint32_t rsvd0:3;
    uint32_t sdc_d_dly:5;
    uint32_t rsvd1:11;
};
///@0x5000000c
struct syscntl_reset_ctrl_t{
    uint32_t dsp_mas_sft_rst:1;
    uint32_t cm3_qspi_ref_sft_rst:1;
    uint32_t dsp_qspi_ref_sft_rst:1;
    uint32_t efuse_mas_sft_rst:1;
    uint32_t auxadc_mas_sft_rst:1;
    uint32_t ssp_sft_rst:1;
    uint32_t i2c0_sft_rst:1;
    uint32_t i2c1_sft_rst:1;
    uint32_t gpio_sft_rst:1;
    uint32_t cm3_uart_sft_rst:1;
    uint32_t dsp_uart_sft_rst:1;
    uint32_t i2s_sft_rst:1;
    uint32_t sbc_sft_rst:1;
    uint32_t frspim_sft_rst:1;
    uint32_t ptc_sft_rst:1;
    uint32_t cdc_sft_rst:1;
    uint32_t pdm_sft_rst:1;
    uint32_t revd:7;
    uint32_t btdm_mas_sft_rst:1;
    uint32_t btdm_crypt_sft_rst:1;
    uint32_t mdm_top_sft_rst:1;
    uint32_t mdm_rf_sft_rst:1;
    uint32_t mdm_core_sft_rst:1;
    uint32_t mdm_reg_sft_rst:1;
    uint32_t dsp_qspi1_sft_rst:1;
    uint32_t sdc_sft_rst:1;
};

///@0x50000010
struct syscntl_cdc_mas_clk_adj_t {
    uint32_t cdc_adj_div:29;
    uint32_t cdc_adj_bp:1;/// codec master clock adjust bypass or not
    uint32_t cdc_adj_new:1;
    uint32_t cdc_adj_dir:1;
};

///@0x50000014
struct syscntl_misc_ctrl_t {
    uint32_t audio_format:1;// 0 -- voice, 1--- dac audio
    uint32_t audio_src:1;   //0--- internal 1---external
    uint32_t audio_dst:1;   //0---normal    1---external
    uint32_t cdc_if_sel:1;
    uint32_t mdm_if_sel:1;
    uint32_t sbc2cdc_en:1;
    uint32_t sbc2iis_en:1;
    uint32_t qspi_source:1;
    uint32_t qspi_burn:1;
    uint32_t rsvd:23;
};

///@0x50000020
struct syscntl_port_mux_t {
    uint32_t porta_mux_sel;
    uint32_t portb_mux_sel;
    uint32_t portc_mux_sel;
    uint32_t portd_mux_sel;
};

///@0x50000030
struct syscntl_pad_ie_ctrl_t {
    /// 1--- enable input,0---disable input
    uint32_t pad_porta_ie:8;
    uint32_t pad_portb_ie:8;
    uint32_t pad_portc_ie:8;
    uint32_t pad_portd_ie:8;
};

///@0x50000034
struct syscntl_pad_len_ctrl_t {
    /// 0--- enable pull down,1---disable pull down
    uint32_t pad_porta_len:8;
    uint32_t pad_portb_len:8;
    uint32_t pad_portc_len:8;
    uint32_t pad_portd_len:8;
};

///@0x50000038
struct syscntl_pad_he_ctrl_t {
    /// 1--- enable pull up,0---disable pull up
    uint32_t pad_porta_he:8;
    uint32_t pad_portb_he:8;
    uint32_t pad_portc_he:8;
    uint32_t pad_portd_he:8;
};

///@0x5000003c
struct syscntl_pad_qspi_ctrl_t{
    uint32_t qspi_ie:6;
    uint32_t resv0:2;
    uint32_t qspi_len:6;
    uint32_t resv1:2;
    uint32_t qspi_he:6;
    uint32_t resv2:10;
};


///@0x50000040
struct syscntl_usbotg_ctrl_t {
    uint32_t usb_phy_adp_cfg:8;
    uint32_t otg_vbusval_i:1;
    uint32_t otg_vbusses_i:1;
    uint32_t otg_vbuslo_i:1;
    uint32_t otg_cid_i:1;
    uint32_t usb_phy_sel:1;
    uint32_t rsvd:3;
    uint32_t pusb_psp:1;
    uint32_t pusb_psm:1;
    uint32_t pusb_pep:1;
    uint32_t pusb_pem:1;
    uint32_t rsvd1:12;
};

///@0x50000048
struct syscntl_dsp_mem_ctrl_t {
    uint32_t mux:4;
    /*
    0000: IRAM = DRAM = 64KB
    0001: IRAM = 48K, DRAM = 80K
    0011: IRAM = 32K, DRAM = 96K
    0111: IRAM = 16K, DRAM = 112K
    1111: IRAM = 0K, DRAM = 128K
    */
    uint32_t reserved:28;
};

///@0x5000004c
struct syscntl_diag_cfg_t{
    /// 00---bt, 01---ble, 10---dm
    uint32_t btdm_cfg0:2;
    uint32_t btdm_cfg1:2;
    uint32_t btdm_cfg2:2;
    uint32_t btdm_cfg3:2;
    uint32_t bbsoc_cfg:4;
    uint32_t rsvd:20;
};

struct system_regs_t  {
    //uint32_t reserved[17];
    struct syscntl_clock_ctrl_t clk_ctrl;           //0x00
    struct syscntl_clock_enable_t clk_enable;       //0x04
    struct syscntl_sdc_ctrl_t sdc_ctrl;             //0x08
    struct syscntl_reset_ctrl_t reset_ctrl;         //0x0c
    struct syscntl_cdc_mas_clk_adj_t cdc_mas_clk_adj;   //0x10
    struct syscntl_misc_ctrl_t misc_ctrl;           //0x14
    uint32_t resv0[2];
    uint32_t port_mux[4];                           //0x20
    struct syscntl_pad_ie_ctrl_t pad_ie;            //0x30
    uint32_t pad_len;                               //0x34
    uint32_t pad_he;                                //0x38
    struct syscntl_pad_qspi_ctrl_t pad_qspi;        //0x3c
    struct syscntl_usbotg_ctrl_t usbotg_ctrl;       //0x40
    uint32_t dsp_reset_vector;                      //0x44
    struct syscntl_dsp_mem_ctrl_t dsp_mem_ctrl;     //0x48
    uint32_t diag_cfg;                              //0x4c, 00---bt, 01---ble, 10---dm
    uint32_t qspi_map_src;                          //0x50, bit[0,21]
    uint32_t qspi_map_len;                          //0x54, bit[0,21]
    uint32_t qspi_map_des;                          //0x58, bit[0,21]
    struct syscntl_reset_ctrl_t reset_ctrl_level;   //0x0c
    uint32_t resv3[4];
    
};

//extern volatile struct syscntl_reg_t *const sys_ctrl;
extern volatile struct system_regs_t * system_regs;

/*
 * PUBLIC FUNCTIONS 
 */

/*********************************************************************
 * @fn      system_get_pclk
 *
 * @brief   get current system clock, the value should be 16M, 24M, 48M.
 *
 * @param   None
 *
 * @return  current system clock.
 */
uint32_t system_get_pclk(void);

/*********************************************************************
 * @fn      system_set_pclk
 *
 * @brief   change current system clock, some peripheral clock settings need
 *          to be reconfig if neccessary.
 *
 * @param   clk - @ref system_clk_t
 *
 * @return  None.
 */
void system_set_pclk(uint8_t clk);

/*********************************************************************
 * @fn      system_get_pclk_config
 *
 * @brief   get current system clock configuration.
 *
 * @param   None
 *
 * @return  current system clock setting, @ref system_clk_t.
 */
uint8_t system_get_pclk_config(void);

/*********************************************************************
 * @fn      system_set_port_pull_up
 *
 * @brief   set pull-up of IOs which are controlled by main digital core,
 *          only effect the pull state of IOs indicated by port parameter.
 *          example usage:
 *          system_set_port_pull((GPIO_PA0 | GPIO_PA1), true)
 *
 * @param   port    - each bit represents one GPIO channel
 *          pull    - 1: enable pull-up, 0: disable pull-up.
 *
 * @return  None.
 */
void system_set_port_pull_up(uint32_t port, uint8_t pull);

/*********************************************************************
 * @fn      system_set_port_pull_down
 *
 * @brief   set pull-up of IOs which are controlled by main digital core,
 *          only effect the pull state of IOs indicated by port parameter.
 *          example usage:
 *          system_set_port_pull((GPIO_PA0 | GPIO_PA1), true)
 *
 * @param   port    - each bit represents one GPIO channel
 *          pull    - 0: enable pull-down, 1: disable pull-down.
 *
 * @return  None.
 */
void system_set_port_pull_down(uint32_t port, uint8_t pull);

/*********************************************************************
 * @fn      system_set_port_mux
 *
 * @brief   set function of IO which is controlled by main digital core,
 *          example usage:
 *          system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PMU_PORT_MUX_KEYSCAN)
 *
 * @param   port    - which group the io belongs to, @ref system_port_t
 *          bit     - the channel number, @ref system_port_bit_t
 *          func    - such as PORTA0_FUNC_I2C0_SCL, PORTA3_FUNC_PDM1_SDA
 *
 * @return  None.
 */
void system_set_port_mux(enum system_port_t port, enum system_port_bit_t bit, uint8_t func);

/*********************************************************************
 * @fn      system_reset_dsp_set
 *
 * @brief   reset and halt dsp core
 *
 * @param   None.
 *
 * @return  None.
 */
void system_reset_dsp_set(void);

/*********************************************************************
 * @fn      system_reset_dsp_release
 *
 * @brief   release dsp core
 *
 * @param   None.
 *
 * @return  None.
 */
void system_reset_dsp_release(void);

/*********************************************************************
 * @fn      system_reset_dsp
 *
 * @brief   reset dsp without halt.
 *
 * @param   None.
 *
 * @return  None.
 */
void system_reset_dsp(void);

/*********************************************************************
 * @fn      system_set_dsp_clk
 *
 * @brief   reset dsp running frequency.
 *
 * @param   src     - clock source, @ref system_dsp_clk_src_sel_t
 *          div     - clock dividor 
 *
 * @return  None.
 */
void system_set_dsp_clk(enum system_dsp_clk_src_sel_t src, uint8_t div);

/*********************************************************************
 * @fn      system_set_dsp_vector
 *
 * @brief   used to set dsp reset vector
 *
 * @param   vector  - new vector
 *
 * @return  None.
 */
void system_set_dsp_vector(uint32_t vector);

/*********************************************************************
 * @fn      system_set_dsp_mem_config
 *
 * @brief   used to set dsp memory configuration
 *
 * @param   ctrl    - @ref system_dsp_mem_ctrl_t
 *
 * @return  None.
 */
void system_set_dsp_mem_config(enum system_dsp_mem_ctrl_t ctrl);

/*********************************************************************
 * @fn      system_set_diagport
 *
 * @brief   used to set diagport,used internal for debug
 *
 * @param   sys_diag    - select map to ip,bt or ble
 *          ip_diag     - ip diagport mux
 *          bt_diag     - bt diagport mux
 *          ble_diag    - ble diagport mux
 *
 * @return  None.
 */
void system_set_diagport(uint32_t sys_diag, uint32_t ip_diag, uint32_t bt_diag, uint32_t ble_diag);

/*********************************************************************
 * @fn      system_sleep_enable
 *
 * @brief   used to enable deep sleep
 *
 * @param   None.
 *
 * @return  None.
 */
void system_sleep_enable(void);

/*********************************************************************
 * @fn      system_sleep_disable
 *
 * @brief   used to disable deep sleep
 *
 * @param   None.
 *
 * @return  None.
 */
void system_sleep_disable(void);

void system_sleep_direct_enable(void);

void system_set_voice_config(void);

void system_set_sbc_config(void);

void system_set_aac_config(void);

#endif
