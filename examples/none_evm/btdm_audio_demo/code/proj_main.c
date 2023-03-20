/*
 * INCLUDE FILES
 ****************************************************************************************
 */
#include <stdio.h>
#include <string.h>

#include "co_log.h"

#include "gap_api.h"
#include "gatt_api.h"
#include "bt_api.h"
#include "hf_api.h"
#include "a2dp_api.h"
#include "avrcp_api.h"
#include "spp_api.h"
#include "pbap_api.h"
#include "ota_service.h"

#include "os_timer.h"
#include "os_mem.h"
#include "os_msg_q.h"
#include "os_task.h"
#include "user_utils.h"
#include "ipc_load_code.h"
#include "dsp_program.h"
#include "fr8000_burn.h"

#include "driver_uart.h"
#include "driver_flash.h"
#include "driver_syscntl.h"
#include "driver_pmu.h"
#include "driver_button.h"
#include "driver_ipc.h"
#include "driver_gpio.h"
#include "driver_triming.h"
#include "driver_codec.h"
#include "driver_sdc.h"
#include "driver_pwm.h"
#include "usb_audio.h"

#include "app_user_bt.h"
#include "user_task.h"
#include "user_bt.h"
#include "user_fs.h"
#include "user_dsp.h"
#include "app_at.h"
#include "ble_simple_peripheral.h"
#include "audio_source.h"
#include "mass_mal.h"
#include "usbdev.h"
#include "ff.h"
#include "native_playback.h"
#include "memory.h"
#include "comm.h"
#include "user_hid.h"

//__attribute__((section("compile_date_sec")))const uint8 compile_date[]  = __DATE__;
//__attribute__((section("compile_time_sec")))const uint8 compile_time[]  = __TIME__;
extern const uint8 compile_date[];
extern const uint8 compile_time[];

#if (!COPROCESSOR_UART_ENABLE && !FR5088_COMM_ENABLE)
__attribute__((section("ram_code"))) void pmu_gpio_isr_ram(void)
{
    uint32_t gpio_value = ool_read32(PMU_REG_PORTA_INPUT_VAL);
    //printf("gpio = %x\r\n",gpio_value);
    button_toggle_detected(gpio_value);
   
    ool_write32(PMU_REG_STATE_MON_IN_A,gpio_value);
}
#endif

//extern int ff_test_main5 (void);
uint8_t tog_flag = 0;
extern     os_timer_t bt_reconnect_timer;
extern os_timer_t bt_delay_reconnect_timer;

os_timer_t onkey_timer;
os_timer_t bt_test_reconnect_timer;

void bt_test_reconnect_timer_func(void *arg)
{
    uart_putc_noint('T');
    
    os_timer_start(&bt_test_reconnect_timer,1000,false);
}

void onkey_timer_func(void *arg)
{
    if(REG_PL_RD(0x50000078)&BIT12){
        printf("onkey pressed\r\n");
    }else{
        printf("onkey released\r\n");
        pmu_clear_isr_state(PMU_INT_ONKEY);
        ool_write(PMU_REG_INT_EN_1, ((ool_read(PMU_REG_INT_EN_1) | ((PMU_INT_ONKEY)))));
    }
}

void proj_button_evt_func(struct button_msg_t *msg)
{
    enum bt_state_t bt_state;
    extern struct user_bt_env_tag *user_bt_env;
    
    bt_state = bt_get_state();
    
    LOG_INFO("index = %x,type = %x, cnt = %x\r\n",msg->button_index,msg->button_type,msg->button_cnt);
    if(msg->button_index == BUTTON_FN12){
        if(msg->button_type == BUTTON_SHORT_PRESSED){
            if(bt_state == BT_STATE_HFP_INCOMMING){
                hf_answer_call(user_bt_env->dev[0].hf_chan);
            }else if(bt_state == BT_STATE_HFP_CALLACTIVE){
                hf_hang_up(user_bt_env->dev[0].hf_chan);
            }else if(bt_state == BT_STATE_CONNECTED){
                //printf("%x,%x\r\n",user_bt_env->dev[0].rcp_chan,user_bt_env->dev[1].rcp_chan);
                avrcp_set_panel_key(user_bt_env->dev[0].rcp_chan, AVRCP_POP_PLAY,TRUE);
            }else if(bt_state == BT_STATE_MEDIA_PLAYING){
                avrcp_set_panel_key(user_bt_env->dev[0].rcp_chan, AVRCP_POP_PAUSE,TRUE);
            }
        }else if(msg->button_type == BUTTON_LONG_PRESSED){
            if((bt_state == BT_STATE_HFP_INCOMMING)||(BT_STATE_HFP_OUTGOING)){
                hf_hang_up(user_bt_env->dev[0].hf_chan);
            }
        }else if(msg->button_type == BUTTON_MULTI_PRESSED){
            if(bt_state == BT_STATE_CONNECTED){
                hf_redial(user_bt_env->dev[0].hf_chan);
            }
        }
    }else if(msg->button_index == BUTTON_FN13){
        if(msg->button_type == BUTTON_SHORT_PRESSED){
            #if 0
            if(bt_state <= BT_STATE_STANDBY){
                bt_set_a2dp_type(!bt_get_a2dp_type());
            }
            #else
            //os_malloc(4000);
            if(tog_flag == 1){
                //printf("power off\r\n");
                tog_flag = 0;
                bt_set_accessible_mode_nc(BAM_NOT_ACCESSIBLE,NULL);
                os_timer_stop(&bt_delay_reconnect_timer);
                os_timer_stop(&bt_reconnect_timer);
                bt_disconnect();
            }else{
                
                //printf("power on\r\n");
                tog_flag = 1;
                //bt_set_accessible_mode_nc(BAM_GENERAL_ACCESSIBLE,NULL);
                bt_reconnect(RECONNECT_TYPE_USER_CMD,NULL);
            }
        
            #endif
        }else if(msg->button_type == BUTTON_LONG_PRESSED){
            #if 0
            if(bt_state >= BT_STATE_CONNECTED){
                bt_disconnect();
            }
            #else
            //REG_PL_RD(0x60000000);

            #endif
        }
    }
#if COPROCESSOR_UART_ENABLE
    else if(msg->button_index == BUTTON_FN20){
        if(msg->button_type == BUTTON_PRESSED){
            //PC4
            //uart_slave_send_wake_ind();
        }
    }
#endif
}

/*********************************************************************
 * @fn      user_custom_parameters
 *
 * @brief   initialize several parameters, this function will be called 
 *          at the beginning of the program. Note that the parameters 
 *          changed here will overlap changes in uart tools 
 *
 * @param   None. 
 *       
 *
 * @return  None.
 */
void user_custom_parameters(void)
{
    uint8_t i = 0;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART_BASE;
    uint8_t temp_div;
    
    retry_handshake();
    system_set_port_pull_up(GPIO_PB6, true);
    #if 0
    uart_reg->lcr.divisor_access = 1;
    temp_div =  uart_reg->u1.dll.data;
    uart_reg->lcr.divisor_access = 0;
    system_regs->reset_ctrl.cm3_uart_sft_rst = 1;
    
    if(temp_div == 1){
        uart_init(BAUD_RATE_921600);
    }else{
        uart_init(BAUD_RATE_115200);
    }
    #else
    system_regs->clk_ctrl.clk_bb_div = 0;
    system_regs->reset_ctrl.cm3_uart_sft_rst = 1;
    uart_init(BAUD_RATE_921600);

    #endif
    dsp_program();
        
#if FR5088D_V1_0
    fr8000_program();
    fr8000_reset();
#endif

    LOG_INFO("\r\n Complile Date: ");
    for(i = 0; i < 11; i++){
        LOG_INFO("%c",compile_date[i]);
    }
    LOG_INFO("\r\n");
    LOG_INFO("Ccomplile Time: ");
    for(i = 0; i < 8; i++){
        LOG_INFO("%c",compile_time[i]);
    }
    LOG_INFO("\r\n");
    //printf("mem = %x,%x,%x,%x\r\n",REG_PL_RD(0x40010000),REG_PL_RD(0x40010004),REG_PL_RD(0x40010008),REG_PL_RD(0x4001000c));
#if 0
    /* these parameters can be set for constum application  */
    pskeys.local_bdaddr.addr[0] = 0x09;
    pskeys.local_bdaddr.addr[1] = 0x0a;
    pskeys.local_bdaddr.addr[2] = 0x03;
    pskeys.local_bdaddr.addr[3] = 0x0a;
    pskeys.local_bdaddr.addr[4] = 0x05;
    pskeys.local_bdaddr.addr[5] = 0x08;

    memcpy(pskeys.localname,"FR5080_D",strlen("FR5080_D")+1);

    pskeys.hf_vol = 0x20;
    pskeys.a2dp_vol = 0x20;
    pskeys.hf_vol = 0x20;

    pskeys.enable_profiles = ENABLE_PROFILE_HF|ENABLE_PROFILE_A2DP|ENABLE_PROFILE_SPP;  //enable hfp,a2dp profile
#endif

#if BLE_TEST_MODE
    ///for ble test
    pskeys.system_options = 0xe2;
    pskeys.tws_mode = 2;
#endif
    pskeys.twext = 50;
    pskeys.twosc = 50;
    mac_addr_t addr;
    memcpy(addr.addr,pskeys.local_bdaddr.addr,6);
//    addr.addr[5] |= 0xC0;  //private static
    gap_address_set(&addr);
    ble_set_addr_type(BLE_ADDR_TYPE_PUBLIC);
}

__attribute__((section("ram_code"))) void user_entry_before_sleep_imp(void)
{
    uart_putc_noint_no_wait('s');
    
    //pmu_set_gpio_value(GPIO_PORT_B, 1<<GPIO_BIT_0, 0);
}

__attribute__((section("ram_code"))) void user_entry_after_sleep_imp(void)
{
    /* IO mux have to be reassigned afte wake up from sleep mode */
    app_at_init_app();
    #if 0
    //system_set_dsp_clk(SYSTEM_DSP_SRC_OSC_48M, 0);
    system_set_port_pull_up(GPIO_PC6, true);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_6, PORTC6_FUNC_DSP_UART_RXD);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_7, PORTC7_FUNC_DSP_UART_TXD);
    
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_0, PORTC0_FUNC_PDM0_SCK);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_1, PORTC1_FUNC_PDM0_SDA);
    //pmu_set_gpio_value(GPIO_PORT_B, 1<<GPIO_BIT_0, 1);
    #endif
    uart_putc_noint_no_wait('w');
#if DSP_USE_XIP
#if FR5086D_V1_0
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PORTA0_FUNC_QSPI1_SCLK);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_QSPI1_CS);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_QSPI1_MISO0);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_QSPI1_MISO1);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_QSPI1_MISO2);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_QSPI1_MISO3);
#endif

#if FR5086DQ
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_0, PORTC0_FUNC_QSPI1_SCLK);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_1, PORTC1_FUNC_QSPI1_CS);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_2, PORTC2_FUNC_QSPI1_MISO0);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_3, PORTC3_FUNC_QSPI1_MISO1);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_4, PORTC4_FUNC_QSPI1_MISO2);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_5, PORTC5_FUNC_QSPI1_MISO3);
#endif
#endif

#if SD_DEVICE_ENABLE == 1
        system_set_port_pull_down(GPIO_PC0|GPIO_PC1|GPIO_PC2, false);
        system_set_port_pull_up(GPIO_PC0|GPIO_PC1|GPIO_PC2, true);
        system_set_port_mux(GPIO_PORT_C, GPIO_BIT_0, PORTC0_FUNC_SDC_CLK);
        system_set_port_mux(GPIO_PORT_C, GPIO_BIT_1, PORTC1_FUNC_SDC_CMD);
        system_set_port_mux(GPIO_PORT_C, GPIO_BIT_2, PORTC2_FUNC_SCD_DAT0);
        system_regs->reset_ctrl.sdc_sft_rst = 1;
        system_regs->sdc_ctrl.sdc_clk_div = 0;
        //MAL_Init(MASS_LUN_IDX_SD_NAND);
        
#endif  // SD_DEVICE_ENABLE == 1
}

#if HID_ENABLE == 1
#include "bt_hid_api.h"

/****************************************************************************
 * HID SDP Entries
 ****************************************************************************/

/*---------------------------------------------------------------------------
 * SDP ServiceClassIDList
 */
static const uint8_t HidSrcClassId0[] = {
    SDP_ATTRIB_HEADER_8BIT(3),                  /* Data Element Sequence, 3 bytes */
    SDP_UUID_16BIT(SC_HUMAN_INTERFACE_DEVICE),  /* Human Interface Device */
};

/*---------------------------------------------------------------------------
 * SDP Protocol Descriptor List object registered by HID.
 * 
 * Value of the protocol descriptor list for the HID Profile.
 * This structure is a ROM'able representation of the RAM structure.
 * This structure is copied into a RAM structure used to register the 
 * service.
 */
static const uint8_t HidProtoDescList0[] = {
    SDP_ATTRIB_HEADER_8BIT(13),  /* Data element sequence, 13 bytes */

    /* Each element of the list is a Protocol descriptor which is a
     * data element sequence. The first element is L2CAP which only
     * has a UUID element.
     */
    SDP_ATTRIB_HEADER_8BIT(6),    /* Data element sequence for L2CAP, 6 bytes */     
    SDP_UUID_16BIT(PROT_L2CAP),      /* L2CAP UUID */
    SDP_UINT_16BIT(0x0011), /* HID PSM */

    /* Next protocol descriptor in the list is HID.
     */
    SDP_ATTRIB_HEADER_8BIT(3),   /* Data element sequence for AVDTP, 2 bytes */
    SDP_UUID_16BIT(PROT_HIDP)    /* HID UUID */
};

/*---------------------------------------------------------------------------
 * SDP AdditionalProtocol Descriptor List object registered by HID.
 * 
 * Value of the additional protocol descriptor list for the HID.
 * This structure is a ROM'able representation of the RAM structure.
 * During HCRP_ServerRegisterSDP, this structure is copied into the RAM 
 * structure and used to register the client or server.
 */
static const uint8_t HcrpAddProtoDescList0[] = {
    SDP_ATTRIB_HEADER_8BIT(15),  /* Data element sequence, 15 bytes  */

    /* Each element of this list is a protocol descriptor list.  For
     * HID, there is only one list.
     */

    SDP_ATTRIB_HEADER_8BIT(13),  /* Data element sequence, 13 bytes */

    /* Each element of the list is a Protocol descriptor which is a
     * data element sequence. The first element is L2CAP which has
     * a UUID element and a PSM.  The second element is HID.
     */
    SDP_ATTRIB_HEADER_8BIT(6),   /* Data element sequence for L2CAP */     
    SDP_UUID_16BIT(PROT_L2CAP),  /* Uuid16 L2CAP                    */
    SDP_UINT_16BIT(0x0013),  /* L2CAP PSM (varies)         */

    /* The next protocol descriptor is for HCRP. It contains one element
     * which is the UUID.
     */
    SDP_ATTRIB_HEADER_8BIT(3),   /* Data element sequence for HCRP Control */
    SDP_UUID_16BIT(PROT_HIDP)    /* Uuid16 HCRP Channel (varies)           */
};

/*---------------------------------------------------------------------------
 * SDP Language Base Attribute ID List.  
 *
 * Only defines the English language.  This must be modified if other language
 * support is required.
 */
static const uint8_t HidLangBaseAttrIdList0[] = {
    SDP_ATTRIB_HEADER_8BIT(9),

    /* Elements of this list occur in triplets that describe the Language
     * Base Attribute ID.
     */
    SDP_UINT_16BIT(0x656e),      /* English "en" */
    SDP_UINT_16BIT(0x006a),      /* UTF-8 endoding */
    SDP_UINT_16BIT(0x0100)       /* Primary Language Base ID */

    /* Additional languages may be defined.  Each language is defined by a
     * "triplet" of 16-bit elements.
     * 
     * The first element of the triplet defines the language.  The language is
     * encoded according to ISO 639:1988 (E/F):  "Code for the representation
     * of names of languages."  
     *
     * The second element describes the character encoding.  Values for
     * character encoding can be found in IANA's database2, and have the values
     * that are referred to as MIBEnum values. The recommended character
     * encoding is UTF-8.
     *
     * The third element of each triplet contains an attribute ID that serves as
     * the base attribute ID for the natural language in the service record.
     *
     * The length of this attribute must be modified to include the length
     * of any additional triplets.
     */
};

/*---------------------------------------------------------------------------
 * HID Language ID Base List.
 *
 * Defines how Bluetooth strings are mapped to HID LANGID and string 
 * indices.  For a complete description, see section 7.11.7 of the
 * Bluetooth HID Profile specification.  The default value is a sample
 * that supports only United States English.
 */
static const uint8_t HidLangIdBaseList0[] = {
    SDP_ATTRIB_HEADER_8BIT(8),   /* Data element sequence */

    /* Each element of the list is a data element sequence describing a HID
     * language ID base.  Only one element is included.
     */
    SDP_ATTRIB_HEADER_8BIT(6),   /* Data element sequence. */
    SDP_UINT_16BIT(0x0409),      /* Language = English (United States) */
    SDP_UINT_16BIT(0x0100)       /* Bluetooth String Offset */

    /* Additional languages may be defined.  Each language is defined by
     * a data element sequence with 2 elements.
     *
     * The first element defines the language and is encoded according to the
     * the "Universal Serial Bus Language Identifiers (LANGIDs)" specification.
     *
     * The second element defines the Bluetooth base attribute ID as is defined
     * in the Language Base Attribute ID List (see HidLangBaseAttrIdList above).
     *
     * Headers must be added along with the actual element (HID Language ID
     * base) data.  Also,  the length in the first header must be adjusted.
     */
};

/*---------------------------------------------------------------------------
 * SDP Public Browse Group.
 */
static const uint8_t HidBrowseGroup0[] = {
    SDP_ATTRIB_HEADER_8BIT(3),               /* 3 bytes */
    SDP_UUID_16BIT(SC_PUBLIC_BROWSE_GROUP)   /* Public Browse Group */
};

/*---------------------------------------------------------------------------
 * SDP Profile Descriptor List
 */
static const uint8_t HidProfileDescList0[] = {
    SDP_ATTRIB_HEADER_8BIT(8),   /* Data element sequence, 8 bytes */
    SDP_ATTRIB_HEADER_8BIT(6),   /* Data element sequence for ProfileDescriptor,
                                  * 6 bytes.
                                  */
    SDP_UUID_16BIT(SC_HUMAN_INTERFACE_DEVICE),
    SDP_UINT_16BIT(0x0100)
};

/*---------------------------------------------------------------------------
 * Device Release Number.
 */
static const uint8_t HidDeviceRelease0[] = {
    SDP_UINT_16BIT(HID_DEVICE_RELEASE)
};

/*---------------------------------------------------------------------------
 * Parser Version.
 */
static const uint8_t HidParserVersion0[] = {
    SDP_UINT_16BIT(HID_PARSER_VERSION)
};

/*---------------------------------------------------------------------------
 * Device Subclass.
 */
static const uint8_t HidDeviceSubclass0[] = {
    SDP_UINT_8BIT(HID_DEVICE_SUBCLASS)
};

/*---------------------------------------------------------------------------
 * Country Code.
 */
static const uint8_t HidCountrycode0[] = {
    SDP_UINT_8BIT(HID_COUNTRY_CODE)
};

/*---------------------------------------------------------------------------
 * Virtual Cable.
 */
static const uint8_t HidVirtualCable0[] = {
    SDP_BOOL(HID_VIRTUAL_CABLE)
};

/*---------------------------------------------------------------------------
 * Initiate Reconnect.
 */
static const uint8_t HidReconnectInitiate0[] = {
    SDP_BOOL(HID_RECONNECT_INITIATE)
};

/*---------------------------------------------------------------------------
 * Descriptor List.
 */
uint8_t HidDescriptorList_app0[] = {
    SDP_ATTRIB_HEADER_8BIT(HID_DESCRIPTOR_LEN + 6), /* Data element sequence */

    /* Each element of the list is a HID descriptor which is a
     * data element sequence.
     */
    SDP_ATTRIB_HEADER_8BIT(HID_DESCRIPTOR_LEN + 4),
    SDP_UINT_8BIT(HID_DESCRIPTOR_TYPE),  /* Report Descriptor Type */

    /* Length of the HID descriptor */
    SDP_TEXT_8BIT(HID_DESCRIPTOR_LEN),

    /* The actual descriptor is defined in hid.h or in overide.h */
    HID_DESCRIPTOR

    /* Addition descriptors may be added, but the header must be added
     * along with the actual descriptor data.  Also, all lengths in the 
     * headers above must be adjusted.
     */
};

/*---------------------------------------------------------------------------
 * Profile Version
 */
static const uint8_t HidProfileVersion0[] = {
    SDP_UINT_16BIT(0x0100)
};

/*---------------------------------------------------------------------------
 * Boot Device
 */
static const uint8_t HidBootDevice0[] = {
    SDP_BOOL(HID_BOOT_DEVICE)
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  ServiceName
 *
 * This is the English string.  Other languguages can be defined.
 */
static const uint8_t HidServiceName0[] = {
    SDP_TEXT_8BIT(9),          /* Null terminated text string */
    'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd', '\0'
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  ServiceDescription
 *
 * This is the English string.  Other languguages can be defined.
 */
static const uint8_t HidServiceDescription0[] = {
    SDP_TEXT_8BIT(9),          /* Null terminated text string */
    'K', 'e', 'y', 'b', 'o', 'a', 'r', 'd', '\0'
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  ProviderName
 *
 * This is the English string.  Other languguages can be defined.
 */
static const uint8_t HidProviderName0[] = {
    SDP_TEXT_8BIT(9),          /* Null terminated text string */
    'F', 'r', 'e', 'q', 'c', 'h', 'i', 'p', '\0'
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  HidSdpDisable
 */
static const uint8_t HidSdpDisable0[] = {
    SDP_BOOL(0)
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  HidBatteryPower
 */
static const uint8_t HidBatteryPower0[] = {
    SDP_BOOL(HID_BATTERY_POWER)
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  HidRemoteWake
 */
static const uint8_t HidRemoteWake0[] = {
    SDP_BOOL(HID_REMOTE_WAKE)
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  HidSupervisionTimeout
 */
static const uint8_t HidSupervisionTimeout0[] = {
    SDP_UINT_16BIT(HID_SUPERVISION_TIMEOUT)
};

/*---------------------------------------------------------------------------
 * * OPTIONAL *  HidNormallyConnectable
 */
static const uint8_t HidNormallyConnectable0[] = {
    SDP_BOOL(HID_NORMALLY_CONNECTABLE)
};

/*---------------------------------------------------------------------------
 * HID attributes.
 *
 * This is a ROM template for the RAM structure used to register the
 * HID SDP record.
 */
static const SdpAttribute HidSdpAttributes0[] = {
    
    /* Mandatory SDP Attributes */

    /* HID class ID List attribute */
    SDP_ATTRIBUTE(AID_SERVICE_CLASS_ID_LIST, HidSrcClassId0), 
    /* HID protocol descriptor list attribute */
    SDP_ATTRIBUTE(AID_PROTOCOL_DESC_LIST, HidProtoDescList0),
    /* Public Browse Group Service */
    SDP_ATTRIBUTE(AID_BROWSE_GROUP_LIST, HidBrowseGroup0), 
    /* Language Base ID List */
    SDP_ATTRIBUTE(AID_LANG_BASE_ID_LIST, HidLangBaseAttrIdList0),
    /* HID profile descriptor list attribute */
    SDP_ATTRIBUTE(AID_BT_PROFILE_DESC_LIST, HidProfileDescList0),
    /* HID additional protocol descriptor list attribute */
    SDP_ATTRIBUTE(AID_ADDITIONAL_PROT_DESC_LISTS, HcrpAddProtoDescList0),

    /* Optional Human readable attributes.  The strings provided are English.
     * Other languages can be added.  Each language should have a different
     * Language Base Attribute ID (defined in HidLangBaseAttrIdList).  This ID
     * is added to the universal attribute ID for Service Name, Service
     * Description, and Provider Name.
     */

    /* HID Service Name in English */
    SDP_ATTRIBUTE((AID_SERVICE_NAME + 0x0100), HidServiceName0),
    /* HID Service Description in English */
    SDP_ATTRIBUTE((AID_SERVICE_DESCRIPTION + 0x0100), HidServiceDescription0),
    /* HID Provider Name in English*/
    SDP_ATTRIBUTE((AID_PROVIDER_NAME + 0x0100), HidProviderName0),

    /* Mandatory HID attributes */

    /* Device release number */
    SDP_ATTRIBUTE(AID_HID_DEVICE_RELEASE, HidDeviceRelease0),
    /* HID parser version */
    SDP_ATTRIBUTE(AID_HID_PARSER_VERSION, HidParserVersion0),
    /* Device subclass */
    SDP_ATTRIBUTE(AID_HID_DEVICE_SUBCLASS, HidDeviceSubclass0),
    /* Country Code */
    SDP_ATTRIBUTE(AID_HID_COUNTRY_CODE, HidCountrycode0),
    /* Virtual Cable */
    SDP_ATTRIBUTE(AID_HID_VIRTUAL_CABLE, HidVirtualCable0),
    /* Device initiates reconnect */
    SDP_ATTRIBUTE(AID_HID_RECONNECT_INIT, HidReconnectInitiate0),
    /* HID descriptor list */
    SDP_ATTRIBUTE(AID_HID_DESCRIPTOR_LIST, HidDescriptorList_app0),
    /* Language ID Base List */
    SDP_ATTRIBUTE(AID_HID_LANG_ID_BASE_LIST, HidLangIdBaseList0),

    /* Optional HID attibutes */

    /* SDP Disable/Enable */
    SDP_ATTRIBUTE(AID_HID_SDP_DISABLE, HidSdpDisable0),
    /* Battery powered */
    SDP_ATTRIBUTE(AID_HID_BATTERY_POWER, HidBatteryPower0),
    /* Device support of remote wakeup */
    SDP_ATTRIBUTE(AID_HID_REMOTE_WAKE, HidRemoteWake0),

    /* Mandatory HID attribute */

    /* HID profile version*/
    SDP_ATTRIBUTE(AID_HID_PROFILE_VERSION, HidProfileVersion0),

    /* Optional HID attributes */

    /* Recommended supervision timeout */
    SDP_ATTRIBUTE(AID_HID_SUPERV_TIMEOUT, HidSupervisionTimeout0),
    /* Device connectability */
    SDP_ATTRIBUTE(AID_HID_NORM_CONNECTABLE, HidNormallyConnectable0),

    /* Mandatory HID attribute */

    /* Support for boot protocol */
    SDP_ATTRIBUTE(AID_HID_BOOT_DEVICE, HidBootDevice0),
};

/*---------------------------------------------------------------------------
 * ServiceClassIDList
 */
static const uint8_t DeviceIdSrcClassId[] = {
    SDP_ATTRIB_HEADER_8BIT(3),          /* Data Element Sequence, 3 bytes */
    SDP_UUID_16BIT(SC_PNP_INFO),        /* Plug and Play Information */
};

/*---------------------------------------------------------------------------
 * Specification Id
 */
static const uint8_t DeviceIdSpecId[] = {
    SDP_UINT_16BIT(HID_DEVID_SPEC_ID)
};

/*---------------------------------------------------------------------------
 * Vendor Id
 */
static const uint8_t DeviceIdVendorId[] = {
    SDP_UINT_16BIT(HID_DEVID_VENDOR_ID)
};

/*---------------------------------------------------------------------------
 * Product Id
 */
static const uint8_t DeviceIdProductId[] = {
    SDP_UINT_16BIT(HID_DEVID_PRODUCT_ID)
};

/*---------------------------------------------------------------------------
 * Version
 */
static const uint8_t DeviceIdVersion[] = {
    SDP_UINT_16BIT(HID_DEVICE_RELEASE)
};

/*---------------------------------------------------------------------------
 * Primary Record
 */
static const uint8_t DeviceIdPrimaryRec[] = {
    SDP_BOOL(TRUE)
};

/*---------------------------------------------------------------------------
 * Vendor ID Source
 */
static const uint8_t DeviceIdVendorIdSrc[] = {
    SDP_UINT_16BIT(HID_DEVID_VENDOR_ID_SRC)
};
#endif

void user_entry_before_stack_init(void)
{    
    //*(uint8_t *)(0x2000014e) = 0x73;//bit3 --- ssp enable

    //power on dsp first, system dsp clk controls the actual opening
    ool_write(PMU_REG_DLDO_CTRL_0, ool_read(PMU_REG_DLDO_CTRL_0)|0x20);//dldo bypass
    ool_write(PMU_REG_DSP_ISO, 0x80);       //dsp isolation enable
    ool_write(PMU_REG_LDO_DSP_CTRL, 0xc2);  //dsp power on,bit6
    co_delay_10us(2);
    ool_write(PMU_REG_DLDO_CTRL_0, ool_read(PMU_REG_DLDO_CTRL_0)&0xdf);

    /* initalize uart for AT command usage */
    app_at_init_app();

    LOG_INFO("user_entry_before_stack_init\r\n");
    
    /* register callback functions for BT mode */
    bt_me_set_cb_func(bt_me_evt_func);
    bt_hf_set_cb_func(bt_hf_evt_func);
    bt_a2dp_set_cb_func(bt_a2dp_evt_func);
    bt_avrcp_set_cb_func(bt_avrcp_evt_func);
    bt_spp_set_cb_func(bt_spp_evt_func);
    bt_pbap_client_set_cb_func(PbaClientCallback);
    bt_hid_set_cb_func(bt_hid_evt_func);
    triming_init();
    pmu_set_ioldo_voltage(PMU_ALDO_VOL_3_3);

#if HID_ENABLE == 1
    hid_sdp_attribute = (void *)HidSdpAttributes0;
    hid_sdp_attribute_num = 24;
    device_id_sdp_attribute = (void *)DeviceIdSdpAttributes0;
    device_id_sdp_attribute_num = 7;
#endif
    //connect phone with hfp, connect earphone with a2dp source
    //bt_set_hf_source_mode(1);

} 

void user_entry_after_stack_init(void)
{ 
    LOG_INFO("user_entry_after_stack_init\r\n");
    //local drift set t0 200
    //*(uint32_t *)0x200008a8 = 0x000100c8;

    /* create a user task */
    user_task_init();
    
#if (!BLE_TEST_MODE) & (!FIXED_FREQ_TX_TEST)&(!BT_TEST_MODE)
    /* BLE GAP initialization */
    simple_peripheral_init();

    /* register OTA service */
    ota_gatt_add_service();
#endif


    /* intialize variables used in audio source mode */
    audio_source_init();
    /* intialize variables used in native playback mode */
    native_playback_init();
    
#if MP3_SRC_LOCAL == 0
    /* 
     * In MCU+FR508x application, MCU send raw MP3 data to FR508x for local
     * playback or a2dp source. This function is used to initial variables used
     * when system working in this mode.
     */
    app_bt_audio_play_init();
#endif

    /* disable system enter deep sleep mode for debug */
    system_sleep_disable();

    #if BLE_TEST_MODE|FIXED_FREQ_TX_TEST|BT_TEST_MODE
    uart_init(BAUD_RATE_115200);
    #endif
    
    /* try to reconnect last connected BT device */
    ///reconnect only in sink mode
    if(pskeys.app_data.misc_flag == 0){
        //if(bt_reconnect(RECONNECT_TYPE_POWER_ON,NULL) == false){
            //bt_set_accessible_mode_nc(BAM_GENERAL_ACCESSIBLE,NULL);
        //}
    }

#if 1
    /* configure audio input source from analog MIC */
    ipc_set_audio_inout_type(IPC_MIC_TYPE_ANLOG_MIC, IPC_SPK_TYPE_CODEC, IPC_MEDIA_TYPE_BT);
#else
    /* configure audio input source from PDM */
    ipc_set_audio_inout_type(IPC_MIC_TYPE_PDM,IPC_SPK_TYPE_CODEC,IPC_MEDIA_TYPE_BT);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_0, PORTC0_FUNC_PDM0_SCK);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_1, PORTC1_FUNC_PDM0_SDA);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_2, PORTC2_FUNC_PDM1_SCK);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_3, PORTC3_FUNC_PDM1_SDA);
#endif

    /* 
     * in MCU+FR508x application, PDM is connected both to MCU and FR508x, this pin is used to 
     * notify MCU release PDM when SCO is connect between FR508x and phone.
     */
#if 1
    pmu_set_pin_to_PMU(GPIO_PORT_B, 1<<GPIO_BIT_0);
    //pmu_set_pin_pull_up(GPIO_PORT_B,1<<GPIO_BIT_0,true);
    pmu_set_pin_dir(GPIO_PORT_B, 1<<GPIO_BIT_0,GPIO_DIR_OUT);
    pmu_set_gpio_value(GPIO_PORT_B, 1<<GPIO_BIT_0, 0);
#else
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_0, PORTB0_FUNC_B0);
    gpio_set_dir(GPIO_PORT_B, GPIO_BIT_0, GPIO_DIR_OUT);
    gpio_set_pin_value(GPIO_PORT_B, GPIO_BIT_0,0);
#endif

    system_set_pclk(SYSTEM_SYS_CLK_48M);

#if 0
    //PB1 for bt sleep led control
    pmu_set_pin_to_PMU(GPIO_PORT_B, 1<<GPIO_BIT_1);
    pmu_set_pin_pull_up(GPIO_PORT_B,1<<GPIO_BIT_1,true);
    pmu_set_pin_dir(GPIO_PORT_B, 1<<GPIO_BIT_1,GPIO_DIR_OUT);
    pmu_set_gpio_value(GPIO_PORT_B, 1<<GPIO_BIT_1, 0);
#endif

#if 0
    /* 
     * button module demonstration
     * driver_button is a software module to generate different button event, @ref button_type_t
     */
    /* set PORTB4 and PORTB5  used in button module */
    button_init(BUTTON_FN12|BUTTON_FN13);
    
    /* 
     * enable monitoring PORTB4 and PORTB5 state by PMU. once the level of PORTB4 and PORTB5 is changed, 
     * an interrupt named pmu_gpio_isr_ram is generated.
     */
    pmu_set_pin_pull_up(GPIO_PORT_B,1<<GPIO_BIT_4,true);
    pmu_set_pin_pull_up(GPIO_PORT_B,1<<GPIO_BIT_5,true);
    pmu_port_wakeup_func_set(BUTTON_FN12|BUTTON_FN13);
    
    /* set a callback to receive button event */
    bt_button_set_cb_func(proj_button_evt_func);
    
#endif

#if COPROCESSOR_UART_ENABLE

    //bt_button_set_cb_func(proj_button_evt_func);
    uart_slave_init();

#endif

#if FR5088_COMM_ENABLE

	comm_peripheral_init();

    /* Port D0 FR8000 reset */
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_0, PORTD0_FUNC_D0);
    gpio_set_dir(GPIO_PORT_D, GPIO_BIT_0, GPIO_DIR_OUT);
    gpio_set_pin_value(GPIO_PORT_D, GPIO_BIT_0, 0);
    co_delay_100us(10);
    gpio_set_pin_value(GPIO_PORT_D, GPIO_BIT_0, 1);
    
#endif  // FR5088_COMM_ENABLE

#if USB_DEVICE_ENABLE == 1
    system_set_port_pull_up(GPIO_PA6, true);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_CM3_UART_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_7, PORTA7_FUNC_CM3_UART_TXD);

    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_6, PORTB6_FUNC_USB_DP);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_7, PORTB7_FUNC_USB_DM);
    system_set_port_pull_down(GPIO_PB6 | GPIO_PB7, false);
    system_set_port_pull_up(GPIO_PB6, true);
    system_set_port_pull_up(GPIO_PB7, false);
    
    usbdev_init();

    Memory_init();
#endif  // USB_DEVICE_ENABLE == 1

#if USB_AUDIO_ENABLE == 1
    system_set_port_pull_up(GPIO_PC6, true);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_6, PORTC6_FUNC_DSP_UART_RXD);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_7, PORTC7_FUNC_DSP_UART_TXD);

    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_6, PORTB6_FUNC_USB_DP);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_7, PORTB7_FUNC_USB_DM);
    system_set_port_pull_down(GPIO_PB6 | GPIO_PB7, false);

    usb_device_init();
    usb_audio_init();
	
    NVIC_SetPriority(USBMCU_IRQn, 0);
    NVIC_EnableIRQ(USBMCU_IRQn);
	
    system_set_port_pull_up(GPIO_PB6, true);
    system_set_port_pull_up(GPIO_PB7, false);
	
	codec_usb_audio_adc_dac_config(AUDIO_CODEC_SAMPLE_RATE_ADC_48000 | AUDIO_CODEC_SAMPLE_RATE_DAC_48000, AUDIO_CODEC_DAC_SRC_USER, AUDIO_CODEC_ADC_DEST_USER, 0x3b);

    NVIC_EnableIRQ(CDC_IRQn);

    MIC_Packet  = 0;
    MIC_RxCount = 0;
    MIC_TxCount = 0;
    MIC_Start   = false;

    Speaker_Packet  = 0;
    Speaker_RxCount = 0;
    Speaker_TxCount = 0;
    Speaker_Start   = false;

    while(1);
#endif

#if SD_DEVICE_ENABLE == 1
    system_set_port_pull_down(GPIO_PC0|GPIO_PC1|GPIO_PC2, false);
    system_set_port_pull_up(GPIO_PC0|GPIO_PC1|GPIO_PC2, true);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_0, PORTC0_FUNC_SDC_CLK);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_1, PORTC1_FUNC_SDC_CMD);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_2, PORTC2_FUNC_SCD_DAT0);
    system_regs->reset_ctrl.sdc_sft_rst = 1;
    system_regs->sdc_ctrl.sdc_clk_div = 0;

    sdc_init(0, 0, 7, 0);
    sdc_bus_set_clk(0);
    MAL_Init(MASS_LUN_IDX_SD_NAND);
#if MP3_SRC_SDCARD == 1
    fs_init();
    //ff_test_main5();
#endif  // MP3_SRC_SDCARD == 1
#endif  // SD_DEVICE_ENABLE == 1

#if DSP_USE_XIP

#if FR5086D_V1_0
//    system_set_port_pull_up(GPIO_PD0, true);
//    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_0, PORTD0_FUNC_DSP_UART_RXD);
//    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_1, PORTD1_FUNC_DSP_UART_TXD);
    system_set_port_pull_up(GPIO_PC6, true);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_6, PORTC6_FUNC_DSP_UART_RXD);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_7, PORTC7_FUNC_DSP_UART_TXD);
    
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PORTA0_FUNC_QSPI1_SCLK);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_QSPI1_CS);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_QSPI1_MISO0);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_QSPI1_MISO1);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_QSPI1_MISO2);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_QSPI1_MISO3);
//    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_6, PORTD6_FUNC_QSPI1_MISO2);
//    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_QSPI1_MISO3);
    
    system_set_port_pull_up((GPIO_PB1|GPIO_PB2|GPIO_PB3), true);
    system_set_port_pull_down((GPIO_PB1|GPIO_PB2|GPIO_PB3), false);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_0, PORTB0_FUNC_DSP_IO0);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_1, PORTB1_FUNC_DSP_IO1);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_2, PORTB2_FUNC_DSP_IO2);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_3, PORTB3_FUNC_DSP_IO3);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_5, PORTC5_FUNC_DSP_IO5);
    
//    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_1, PORTB1_FUNC_B1);
//    gpio_set_dir(GPIO_PORT_B, GPIO_BIT_1, GPIO_DIR_OUT);
//    gpio_portb_write(gpio_portb_read() | 0x02);
#endif  // FR5086D_V1_0

#if FR5088D_V1_0
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_0, PORTC0_FUNC_QSPI1_SCLK);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_1, PORTC1_FUNC_QSPI1_CS);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_QSPI1_MISO0);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_QSPI1_MISO1);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_QSPI1_MISO2);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_QSPI1_MISO3);
#endif  // FR5086D_V1_0

#if FR5082DM_V1_0
    system_set_port_pull_up(GPIO_PC6, true);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_6, PORTC6_FUNC_DSP_UART_RXD);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_7, PORTC7_FUNC_DSP_UART_TXD);
    
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PORTA0_FUNC_QSPI1_SCLK);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_QSPI1_CS);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_QSPI1_MISO0);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_QSPI1_MISO1);
//    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_QSPI1_MISO2);
//    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_QSPI1_MISO3);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_6, PORTD6_FUNC_QSPI1_MISO2);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_QSPI1_MISO3);
    
    system_set_port_pull_up((GPIO_PB1|GPIO_PB2|GPIO_PB3), true);
    system_set_port_pull_down((GPIO_PB1|GPIO_PB2|GPIO_PB3), false);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_0, PORTB0_FUNC_DSP_IO0);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_1, PORTB1_FUNC_DSP_IO1);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_2, PORTB2_FUNC_DSP_IO2);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_3, PORTB3_FUNC_DSP_IO3);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_5, PORTC5_FUNC_DSP_IO5);
#endif  // FR5082DM_V1_0

#if FR5086_JET
    system_set_port_pull_up(GPIO_PC6, true);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_6, PORTC6_FUNC_DSP_UART_RXD);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_7, PORTC7_FUNC_DSP_UART_TXD);
    
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PORTA0_FUNC_QSPI1_SCLK);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_QSPI1_CS);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_QSPI1_MISO0);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_QSPI1_MISO1);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_QSPI1_MISO2);
    system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_QSPI1_MISO3);
    
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_0, PORTB0_FUNC_DSP_IO0);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_2, PORTB2_FUNC_DSP_IO2);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_3, PORTB3_FUNC_DSP_IO3);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_5, PORTC5_FUNC_DSP_IO5);
#endif  // FR5086_JET

#if FR5086D_V2_0
    system_set_port_pull_up(GPIO_PC6, true);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_6, PORTC6_FUNC_DSP_UART_RXD);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_7, PORTC7_FUNC_DSP_UART_TXD);
    
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PORTA0_FUNC_QSPI1_SCLK);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_QSPI1_CS);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_QSPI1_MISO0);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_QSPI1_MISO1);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_QSPI1_MISO2);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_QSPI1_MISO3);
    
    system_set_port_pull_up((GPIO_PB1|GPIO_PB2|GPIO_PB3), true);
    system_set_port_pull_down((GPIO_PB1|GPIO_PB2|GPIO_PB3), false);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_0, PORTB0_FUNC_DSP_IO0);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_1, PORTB1_FUNC_DSP_IO1);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_2, PORTB2_FUNC_DSP_IO2);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_3, PORTB3_FUNC_DSP_IO3);
#endif  // FR5086D_V2_0

#if FR5086DQ

    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_0, PORTC0_FUNC_QSPI1_SCLK);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_1, PORTC1_FUNC_QSPI1_CS);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_2, PORTC2_FUNC_QSPI1_MISO0);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_3, PORTC3_FUNC_QSPI1_MISO1);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_4, PORTC4_FUNC_QSPI1_MISO2);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_5, PORTC5_FUNC_QSPI1_MISO3);
    
#endif

#else   // DSP_USE_XIP
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_0, PORTB0_FUNC_DSP_IO0);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_1, PORTB1_FUNC_DSP_IO1);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_2, PORTB2_FUNC_DSP_IO2);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_3, PORTB3_FUNC_DSP_IO3);
    
    system_set_port_pull_up(GPIO_PC6, true);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_6, PORTC6_FUNC_DSP_UART_RXD);
    system_set_port_mux(GPIO_PORT_C, GPIO_BIT_7, PORTC7_FUNC_DSP_UART_TXD);
#endif  // DSP_USE_XIP

#if FIXED_FREQ_TX_TEST
    printf("/**************************************************/\r\n");
    printf("Entering FR5080 TX mode\r\n");
    printf("AT cmd description:\r\n");
    printf("AT#TAaa_bb_cc  --- Modulating Transmit,\r\n");
    printf("'aa' --- channel [0x02,0x50],shall be even\r\n");
    printf("'bb' --- packet type,default:0x00,other type refer to spec\r\n");
    printf("'cc' --- transmit power,[0x00,0x3f]\r\n");
            
    printf("AT#TBaa_bb_cc  --- Carrier Transmit,\r\n");
    printf("'aa' --- channel, [0x02,0x50]\r\n");
    printf("'bb' --- not used,set 0x00 \r\n");
    printf("'cc' --- transmit power,[0x00,0x3f]\r\n");

    printf("AT#TC --- exit test mode\r\n");
    printf("/**************************************************/\r\n");
    printf("Please enter at cmd\r\n");
#endif
    //os_timer_init(&onkey_timer,onkey_timer_func,NULL);
    //ool_write(PMU_REG_INT_EN_1, ((ool_read(PMU_REG_INT_EN_1) | ((PMU_INT_ONKEY)))));

#if 0
    REG_PL_WR(0x50000020,0xdddddddd);
    REG_PL_WR(0x50000024,0x99dddddd);
    REG_PL_WR(0x5000004c,0x04);
    REG_PL_WR(0x40000450,0x83);
    REG_PL_WR(0x40000850,0x8300);
#endif
    //os_timer_init(&bt_test_reconnect_timer,bt_test_reconnect_timer_func,NULL);
    //os_timer_start(&bt_test_reconnect_timer,1000,false);
    //bt_set_a2dp_decorder_type(A2DP_DEC_TYPE_SBC_DSP);
}

 void pmu_onkey_isr_ram(void)
{
    printf("onkey isr\r\n");
    
    system_sleep_direct_enable();
    /*
    if(cur_flag == 0){
        bt_statemachine(USER_EVT_NATIVE_PLAYBACK_START,NULL);
        cur_flag = 1;
    }
    else{
        bt_statemachine(USER_EVT_NATIVE_PLAYBACK_STOP,NULL);
        cur_flag = 0;
    }*/
    os_timer_start(&onkey_timer,500,0);
}

void bt_notify_error_func(uint8_t error)
{
    printf("do nothing\r\n");
}

#if 0
void bt_notify_name_func(uint8_t *addr,uint8_t *name,uint8_t name_len)
{
    printf("addr :%x,%x\r\n",addr[0],addr[1]);
    printf("len=%d,name: %c,%c,%c,%c\r\n",name_len,name[0],name[1],name[2],name[3]);
}

bool bt_user_confirm_func(BD_ADDR *addr)
{
    printf("confirm\r\n");
    return true;
}

#endif


#if USB_DEVICE_ENABLE == 1
uint8_t usb_in_flag = 0; // 0 --- usb out, 1 --- usb in, 2 --- usb data transfer, 3 --- usb no data transfer

void usb_user_notify(enum      usb_action_t flag)
{
    //printf("usb flag = %d\r\n",flag);
    if(usb_in_flag == flag){
        return;
    }
    else{
        usb_in_flag = flag;
        if(flag == USB_ACTION_IN){
            system_sleep_disable();
        }
        else if(flag == USB_ACTION_OUT){
            system_sleep_enable();
        }
        else if(flag == USB_ACTION_DATA_TRANSFER){
            gap_stop_advertising();
            bt_set_accessible_mode_nc(BAM_NOT_ACCESSIBLE,NULL);
            bt_disconnect();
        }
        else if(flag == USB_ACTION_IDLE){
            gap_start_advertising(0);
            bt_set_accessible_mode_nc(BAM_GENERAL_ACCESSIBLE,NULL);
        }
    }

}

#endif

