#ifndef _DRIVER_PMU_REGS_H
#define _DRIVER_PMU_REGS_H

#include "co_math.h"

#define PMU_REG_ADKEY_CTRL          0x00
#define PMU_REG_AULOD_CTRL_0        0x01
#define PMU_REG_AULOD_CTRL_1        0x02
#define PMU_REG_AUBUCK_CTRL_0       0x03
#define PMU_REG_AUBUCK_CTRL_1       0x04
#define PMU_REG_AUBUCK_CTRL_2       0x05
#define PMU_REG_AUBUCK_CTRL_3       0x06
#define PMU_REG_AUBUCK_CTRL_4       0x07
#define PMU_REG_AUBUCK_CTRL_5       0x08
#define PMU_REG_CHG_CTRL_0          0x0b
#define PMU_REG_CHG_CTRL_1          0x0c
#define PMU_REG_CHG_CTRL_2          0x0d
#define PMU_REG_CHG_CTRL_3          0x0e
#define PMU_REG_DLDO_CTRL_0         0x11
#define PMU_REG_DLDO_CTRL_1         0x12
#define PMU_REG_LDO_DSP_CTRL        0x13
#define PMU_REG_IOLDO_CTRL          0x14
#define PMU_REG_PKVDD_CTRL_0        0x16
#define PMU_REG_PKVDD_CTRL_1        0x17
#define PMU_REG_BG_TRIM_CTRL        0x1A
#define PMU_REG_AUXADC_PWR_EN       0x1E
#define PMU_REG_SYSBUCK_CTRL_0      0x1F
#define PMU_REG_SYSBUCK_CTRL_1      0x20
#define PMU_REG_SYSBUCK_CTRL_2      0x21
#define PMU_REG_SYSBUCK_CTRL_3      0x22
#define PMU_REG_SYSBUCK_CTRL_4      0x23
#define PMU_REG_VDDHA_SEL           0x25
#define PMU_REG_XTAL32K_CTRL_0      0x26
#define PMU_REG_XTAL32K_CTRL_1      0x27
#define PMU_REG_OSC24M_CTRL         0x28

/**************************************************************************/
/* analog delay relevant registers */
#define PMU_REG_POWER_ON_DLY        0x30
#define PMU_REG_SYS_BUCK_PO_DLY     0x31
#define PMU_REG_SYS_RSTN_PO_DLY     0x32
#define PMU_REG_AULDO_PO_DLY        0x33
#define PMU_REG_DLDO_PO_DLY         0x34
#define PMU_REG_IOLDO_PO_DLY        0x35
#define PMU_REG_PKSTPD_PO_DLY       0x36
#define PMU_REG_VDDHA_PO_DLY        0x37
#define PMU_REG_RF_LATCH_PO_DLY     0x38
#define PMU_REG_BUCK_WFS_PO_DLY     0x39
#define PMU_REG_MEM_PDVDD_PO_DLY    0x3A
#define PMU_REG_MEM_PKVDD_PO_DLY    0x3B
#define PMU_REG_GPIO_PDVDD_PO_DLY   0x3C
#define PMU_REG_GPIO_PKVDD_PO_DLY   0x3D
#define PMU_REG_MEM_ISO_PO_DLY      0x3E
#define PMU_REG_AU_BUCK_PO_DLY      0x3F

#define PMU_REG_POWER_OFF_DLY       0x40
#define PMU_REG_SYS_BUCK_POF_DLY    0x41
#define PMU_REG_SYS_RSTN_POF_DLY    0x42
#define PMU_REG_AULDO_POF_DLY       0x43
#define PMU_REG_DLDO_POF_DLY        0x44
#define PMU_REG_IOLDO_POF_DLY       0x45
#define PMU_REG_PKSTPD_POF_DLY      0x46
#define PMU_REG_VDDHA_POF_DLY       0x47
#define PMU_REG_RF_LATCH_POF_DLY    0x48
#define PMU_REG_BUCK_WFS_POF_DLY    0x49
#define PMU_REG_MEM_PDVDD_POF_DLY   0x4A
#define PMU_REG_MEM_PKVDD_POF_DLY   0x4B
#define PMU_REG_GPIO_PDVDD_POF_DLY  0x4C
#define PMU_REG_GPIO_PKVDD_POF_DLY  0x4D
#define PMU_REG_MEM_ISO_POF_DLY     0x4E
#define PMU_REG_AU_BUCK_POF_DLY     0x4F

#define PMU_REG_VDDHA_OK_PO_DLY     0x56
#define PMU_REG_PMU_ISO_PO_DLY      0x57
#define PMU_REG_GPIO_ISO_PO_DLY     0x58
#define PMU_REG_MEM_CLK_PO_DLY      0x59
#define PMU_REG_MEM_PKRET_PO_DLY    0x5A

#define PMU_REG_VDDHA_OK_POF_DLY    0x5B
#define PMU_REG_PMU_ISO_POF_DLY     0x5C
#define PMU_REG_GPIO_ISO_POF_DLY    0x5D
#define PMU_REG_MEM_CLK_POF_DLY     0x5E
#define PMU_REG_MEM_PKRET_POF_DLY   0x5F
/**************************************************************************/


/**************************************************************************/
/*BT sleep ctrl*/
#define PMU_REG_BB_WKUP_CTRL        0x50
#define PMU_BB_WKUP_EX_DSB          0x01    //disable 外部中断唤醒bt sleep timer
#define PMU_BB_WKUP_CORE_EN         0x02    //使能来自Core的外部中断唤??
#define PMU_BB_WKUP_PMU_EN          0x04    //使能来自PMU的中断唤??
#define PMU_BB_WKUP_TIMER_EN        0x08    //使能bt sleep timer控制pmu controller进入睡眠及唤??
#define PMU_BB_WKUP_OSC_EN          0x10    //使能bt sleep timer内部的osc动态信号产??
#define PMU_BB_MASCLK_ALWAYS_ON     0x20    //bt master clock always enable
#define PMU_BB_WKUP_LP_REG          0x40    //register bit to generate wakeup lp interrupt
#define PMU_BB_SLP_DUR_SEL          0x80    //1: 通过51~54读取的是设定的睡眠时间，0: 通过51~54读取实际睡眠时间
/**************************************************************************/

/**************************************************************************/
/*BT sleep time*/
#define PMU_REG_BB_SLEEP_VAL0       0x51
#define PMU_REG_BB_SLEEP_VAL1       0x52
#define PMU_REG_BB_SLEEP_VAL2       0x53
#define PMU_REG_BB_SLEEP_VAL3       0x54
/**************************************************************************/

/**************************************************************************/
/*osc and external wakeup dummy time*/
#define PMU_REG_BB_TW_OSC           0x55    //value * 4 dummy time clock
#define PMU_REG_BB_TW_EXT           0x92    //value * 4 dummy time clock
/**************************************************************************/

/**************************************************************************/
/*led related*/
///data pattern led0 data pattern[31:0] -- {led0_cfg3,led0_cfg2,led0_cfg1,led0_cfg0}
#define PMU_REG_LED0_CFG0           0x60
#define PMU_REG_LED0_CFG1           0x61
#define PMU_REG_LED0_CFG2           0x62
#define PMU_REG_LED0_CFG3           0x63

#define PMU_REG_LED1_CFG0           0x64
#define PMU_REG_LED1_CFG1           0x65
#define PMU_REG_LED1_CFG2           0x66
#define PMU_REG_LED1_CFG3           0x67

#define PMU_REG_LED2_CFG0           0x68
#define PMU_REG_LED2_CFG1           0x69
#define PMU_REG_LED2_CFG2           0x6a
#define PMU_REG_LED2_CFG3           0x6b

#define PMU_REG_LED_CTL             0x6c
#define PMU_REG_LED_STS             0x6d
/**************************************************************************/

/**************************************************************************/
/*RTC related*/
#define PMU_REG_RTC_CTRL            0x6F
#define PMU_RTC_ALMA_EN             0x01
#define PMU_RTC_ALMB_EN             0x02
#define PMU_RTC_ALMA_CLR            0x04
#define PMU_RTC_ALMB_CLR            0x08
#define PMU_RTC_UPD_EN              0x10
#define PMU_RTC_UPD_RD              0x80    //1--- update,0---read in {0x78--0x7b}

#define PMU_REG_RTC_ALMA_VAL0       0x70
#define PMU_REG_RTC_ALMA_VAL1       0x71
#define PMU_REG_RTC_ALMA_VAL2       0x72
#define PMU_REG_RTC_ALMA_VAL3       0x73

#define PMU_REG_RTC_ALMB_VAL0       0x74
#define PMU_REG_RTC_ALMB_VAL1       0x75
#define PMU_REG_RTC_ALMB_VAL2       0x76
#define PMU_REG_RTC_ALMB_VAL3       0x77

#define PMU_REG_RTC_UPD_VAL0        0x78
#define PMU_REG_RTC_UPD_VAL1        0x79
#define PMU_REG_RTC_UPD_VAL2        0x7A
#define PMU_REG_RTC_UPD_VAL3        0x7B
/**************************************************************************/

/**************************************************************************/
/*WatchDog related*/
#define PMU_REG_WD_VALUE0           0x7C
#define PMU_REG_WD_VALUE1           0x7D
#define PMU_REG_WD_VALUE2           0x7E
#define PMU_REG_WD_VALUE3           0x7F    //bit[7:4] is for value, bit0 for en, bit1 for clear
#define PMU_WD_EN                   0x01
#define PMU_WD_CLR                  0x02
/**************************************************************************/

/**************************************************************************/
/*GPIO config*/
#define PMU_REG_PORTA_IE            0x80
#define PMU_REG_PORTB_IE            0x81
#define PMU_REG_PORTA_OEN           0x82
#define PMU_REG_PORTB_OEN           0x83
#define PMU_REG_PORTA_HE            0x84
#define PMU_REG_PORTB_HE            0x85
#define PMU_REG_PORTA_LEN           0x86
#define PMU_REG_PORTB_LEN           0x87
#define PMU_REG_PORTA_SEL           0x88    //0: PMU, 1: core logic
#define PMU_REG_PORTB_SEL           0x89
#define PMU_REG_PORTA_OUTPUT_SEL    0x8a
#define PMU_REG_PORTA_OUTPUT_VAL    0x8b
#define PMU_REG_PORTB_OUTPUT_SEL    0x8c
#define PMU_REG_PORTB_OUTPUT_VAL    0x8d
/**************************************************************************/

/**************************************************************************/
/*
 * REG90控制针对整个系统的睡眠和唤醒，主要用于系统处于关机状态的唤醒
 * REG91控制针对小timer的唤醒，主要用于系统处于低功耗状态的唤醒
 */
#define PMU_REG_DEEP_SLEEP_EN       0x90
#define PMU_DEEP_SLEEP_BT_EN        0x01    //使能写baseband寄存器进入睡眠
#define PMU_DEEP_SLEEP_CPU_EN       0x02    //使能CPU写PMU寄存器进入睡眠
#define PMU_DEEP_SLEEP_HD_EN        0x04    //使能超低电压进睡眠
#define PMU_DEEP_SLEEP_ONKEY_EN     0x08    //使能onkey进入睡眠
#define PMU_DEEP_SLEEP_BT_WK_EN     0x10    //使能BT小timer唤醒
#define PMU_DEEP_SLEEP_PMU_INT_WK_EN    0x20    //使能pmu中断唤醒
#define PMU_DEEP_SLEEP_MON_CHG_WK_EN    0x40    //使能被检测的对象发生变化唤醒
#define PMU_DEEP_SLEEP_ONKEY_WK_EN      0x80    //使能onkey唤醒

#define PMU_REG_BT_TIMER_WK_SRC     0x91
#define PMU_BT_TIMER_WK_MON         0x01    //允许被检测对象唤醒小timer，然后通过小timer再唤醒系统
#define PMU_BT_TIMER_WK_PMU_INT     0x02    //允许pmu中断唤醒小timer，然后通过小timer再唤醒系统
#define PMU_PMU_CPU_INT_EN          0x04    //控制PMU给大数字的中断使能
#define PMU_PMU_GPIO_INT_EN         0x08    //控制gpio中断enable
#define PMU_BT_TIMER_MON_CLR        0x10    //清除monitor的标志位
#define PMU_ANA_TYPE                0x20
#define PMU_INT_STORE               0x40
#define PMU_INT_CLR_RD              0x80
/**************************************************************************/

/**************************************************************************/
/*PMU interrupt enable*/
#define PMU_REG_INT_EN_1            0x93
#define PMU_INT_ONKEY_OFF           0x01
#define PMU_INT_ALMA                0x02
#define PMU_INT_ONKEY               0x04
#define PMU_INT_BAT_FULL            0x08
#define PMU_INT_AC_OK               0x10
#define PMU_INT_AC_OFF              0x20
#define PMU_INT_LVD                 0x40
#define PMU_INT_OTD                 0x80

#define PMU_REG_INT_EN_2            0x94
#define PMU_INT_CAL_DONE            0x0100
#define PMU_INT_ADKEY_0             0x0200
#define PMU_INT_ADKEY_1             0x0400
#define PMU_INT_ONE_WIRE            0x0800
#define PMU_INT_ULV_POWOFF          0x1000
#define PMU_INT_LED_DONE_0          0x2000
#define PMU_INT_LED_DONE_1          0x4000
#define PMU_INT_GPIO                0x8000
/**************************************************************************/

/**************************************************************************/
/*pmu interrupt clear and status*/
#define PMU_REG_INT_CLR_1           0x95
#define PMU_REG_INT_CLR_2           0x96
#define PMU_REG_INT_STATUS_1        0x95
#define PMU_REG_INT_STATUS_2        0x96
/**************************************************************************/

/***********************************************************************/
///charger related
#define PMU_REG_ANALOG_DETECT_EN    0x97    //使能检查来自analog的信号
#define PMU_BAT_FULL_DETECT_EN      0x01
#define PMU_AC_OK_DETECT_EN         0x02
#define PMU_AC_OFF_DETECT_EN        0x04
#define PMU_LVD_DETECT_EN           0x08
#define PMU_OTD_DETECT_EN           0x10
#define PMU_CHARGING_DETECT_EN      0x20
#define PMU_ONKEY_HIGH_DETECT_EN    0x40

///debounce related
#define PMU_REG_BAT_DEBOUNCE_LEN    0x98
#define PMU_REG_AC_OK_DEBOUNCE_LEN  0x99
#define PMU_REG_LVD_DEBOUNCE_LEN    0x9A
#define PMU_REG_OTD_DEBOUNCE_LEN    0x9B
#define PMU_REG_CHARGING_DEBOUNCE_LEN  0x9C
#define PMU_REG_ONKEY_DEBOUNCE_LEN  0x9D

#define PMU_REG_ANALOG_STATUS       0x9E    // raw analog status
#define PMU_ANA_STATUS_BAT_FULL     0x01
#define PMU_ANA_STATUS_ACOK         0x02
#define PMU_ANA_STATUS_ACOFF        0x04
#define PMU_ANA_STATUS_LVD          0x08
#define PMU_ANA_STATUS_OTD          0x10
#define PMU_ANA_STATUS_ONKEY        0x20
#define PMU_ANA_STATUS_CHARGING     0x40
#define PMU_ANA_STATUS_POWOFF       0x80
/************************************************************************/

/************************************************************************/
///gpio wakeup related
#define PMU_REG_STATE_MON_IN_A      0xA0    ///current gpio status,software store
#define PMU_REG_STATE_MON_IN_B      0xA1
#define PMU_REG_STATE_MON_IN_C      0xA2
#define PMU_REG_STATE_MON_IN_D      0xA3

#define PMU_REG_STATE_MON_IN        0xA4
#define PMU_MON_SRC_OTD             0x04
#define PMU_MON_SRC_LVD             0x08
#define PMU_MON_SRC_AC_OFF          0x10
#define PMU_MON_SRC_AC_OK           0x20
#define PMU_MON_SRC_BAT_FULL        0x40
#define PMU_MON_SRC_ONKEY           0x80

#define PMU_REG_DSP_ISO             0xAA

#define PMU_REG_PORTC_IE            0xc0
#define PMU_REG_PORTD_IE            0xc1
#define PMU_REG_PORTC_OEN           0xc2
#define PMU_REG_PORTD_OEN           0xc3
#define PMU_REG_PORTC_HE            0xc4
#define PMU_REG_PORTD_HE            0xc5
#define PMU_REG_PORTC_LEN           0xc6
#define PMU_REG_PORTD_LEN           0xc7
#define PMU_REG_PORTC_SEL           0xc8    //0: PMU, 1: core logic
#define PMU_REG_PORTD_SEL           0xc9
#define PMU_REG_PORTC_OUTPUT_SEL    0xca
#define PMU_REG_PORTC_OUTPUT_VAL    0xcb
#define PMU_REG_PORTD_OUTPUT_SEL    0xcc
#define PMU_REG_PORTD_OUTPUT_VAL    0xcd

#define PMU_REG_STATE_MON_SEL_A     0xd0    ///sel gpio trig wakeup
#define PMU_REG_STATE_MON_SEL_B     0xd1
#define PMU_REG_STATE_MON_SEL_C     0xd2
#define PMU_REG_STATE_MON_SEL_D     0xd3
#define PMU_REG_STATE_MON_SEL       0xd4

#define PMU_REG_PORTA_INPUT_VAL     0xd6
#define PMU_REG_PORTB_INPUT_VAL     0xd7
#define PMU_REG_PORTC_INPUT_VAL     0xd8
#define PMU_REG_PORTD_INPUT_VAL     0xd9
/***************************************************************************/

#define PMU_REG_MEM_PKVDD_EN        0xA7
#define PMU_REG_MEM_RET_EN          0xA8
#define PMU_REG_MEM_ISO_EN          0xA9

#define PMU_REG_POWER_OFF_CTRL_0   0xAD
#define PMU_REG_POWER_OFF_CTRL_1    0xAE
#define PMU_REG_POWER_OFF_CTRL_2    0xAF

/**************************************************************************/
/*CLK_EN_CTL*/
#define PMU_REG_CLK_EN              0xb0
#define PMU_CLK_RC_EN               0x01    //控制可以gating的RC时钟的源开关
#define PMU_CLK_BT_LP_SEL           0x02    //0: rc clk; 1: osc32K
#define PMU_CLK_RTC_SEL             0x04
#define PMU_CLK_LED_SEL             0x08
#define PMU_CLK_CAL_EN              0x10
#define PMU_CLK_BT_LP_EN            0x20
#define PMU_CLK_RTC_EN              0x40
#define PMU_CLK_LED_EN              0x80
/**************************************************************************/

/**************************************************************************/
/*reset enn*/
#define PMU_REG_RST_EN              0xb1    
#define PMU_RST_CAL_ENN             0x01    //0有效
#define PMU_RST_BT_LP_ENN           0x02    //bt lowpower模块复位,//0有效
#define PMU_RST_RTC_ENN             0x04    //0有效
#define PMU_RST_LED_ENN             0x08    //0有效
#define PMU_RST_ONKEY_RST_EN        0x10    //关闭onkey的长按键复位功能,1有效
#define PMU_RST_WD_RST_EN           0x20    //看门狗 1有效
#define PMU_RST_PIN_RST_EN          0x40    //外部引脚 1有效
#define PMU_RST_VCHG_RST_EN         0x80    //插入充电器复位 1有效
/**************************************************************************/


/**************************************************************************/
#define PMU_REG_ONKEY_ON_OFF_TIME   0xb2    //配置onkey开关机时间
#define PMU_TONKEY_ON_MASK          0x0f    //0---80ms, other--- 500ms* bit[3:0]
#define PMU_TONKEY_OFF_MASK         0xf0    //0---80ms, other--- 500ms* bit[3:0]
/**************************************************************************/

/**************************************************************************/
#define PMU_REG_POWER_MAGIC         0xb3    //PMU状态标志位
#define PMU_FIRST_MAGIC             0xE7    //第一次开机
#define PMU_SHUTDOWN_MAGIC          0xC3    //关机状态
#define PMU_WORKING_MAGIC           0x81    //工作状态，包括睡眠状态
/**************************************************************************/

/**************************************************************************/
#define PMU_REG_ONKEY_CRTL          0xb4    ///pmu onkey ctrl reg
#define PMU_ONKEY_MODE_SWITCH       0x01
#define PMU_ONKEY_CLR               0x02
#define PMU_ONKEY_RST_EN            0x04    ///使能onkey长按复位的功能
#define PMU_OOLKEY_CLR              0x08
#define PMU_RC_DIV_EN               0x10
/**************************************************************************/

#define PMU_REG_POWER_CTRL_0        0xB6
#define PMU_REG_POWER_CTRL_1        0xB7
#define PMU_REG_POWER_CTRL_2        0xB8

/**************************************************************************/
///calibration related
#define PMU_REG_CALI_CNTL           0xBB    //calibration 控制寄存器
#define PMU_CALI_EN                 0x01

#define PMU_REG_CALI_REG_0          0xBC
#define PMU_REG_CALI_REG_1          0xBD
#define PMU_REG_CALI_REG_2          0xBE
#define PMU_REG_CALI_REG_3          0xBF
/**************************************************************************/

#endif

