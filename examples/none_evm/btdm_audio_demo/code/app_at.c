#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "bt_api.h"
#include "hf_api.h"
#include "avrcp_api.h"
#include "a2dp_api.h"

#include "os_msg_q.h"
#include "os_mem.h"
#include "os_timer.h"
#include "user_utils.h"

#include "driver_gpio.h"
#include "driver_uart.h"
#include "driver_plf.h"
#include "driver_pmu.h"
#include "driver_codec.h"
#include "driver_ipc.h"
#include "driver_button.h"

#include "user_task.h"
#include "audio_source.h"
#include "native_playback.h"
#include "msbc_playback.h"
#include "user_bt.h"
#include "user_utils.h"
#include "co_log.h"
#include "mp3_tag_decoder.h"
#include "user_fs.h"
#include "user_dsp.h"
#include "app_at.h"
#include "gap_api.h"
#include "comm.h"

#if COPROCESSOR_UART_ENABLE
///port define
#define UART_SLAVE_WAKE_IO          GPIO_PC4
#define UART_SLAVE_WAKE_PIN_PORT   GPIO_PORT_C    //input,used by master to wake up slave
#define UART_SLAVE_WAKE_PIN_BIT     GPIO_BIT_4
#define UART_SLAVE_IND_PIN_PORT    GPIO_PORT_C    //output,used by slave to indicate master that slave has data to send
#define UART_SLAVE_IND_PIN_BIT      GPIO_BIT_5

#define UART_SLAVE_BAUDRATE     BAUD_RATE_115200

enum uart_lp_rx_state_t{
    UART_LP_RX_STATE_IDLE,
    UART_LP_RX_STATE_HEADER,
    UART_LP_RX_STATE_DATA,
};

//co_list_t uart_msg_list;
struct uart_lp_env_t uart_lp_env;

os_timer_t uart_slave_wake_timer;
os_timer_t uart_slave_check_timer;

#endif

os_timer_t timer_btdm_tx_test;
static uint8_t tx_type = 0;
static uint8_t tx_freq = 0;
static uint8_t tx_pwr = 0;

#define AT_RECV_MAX_LEN             32
#if PBAP_FUC == 1
    uint8_t pbap_connect_flag = 0;
#endif
static uint8_t app_at_recv_char;
static uint8_t at_recv_buffer[AT_RECV_MAX_LEN];
static uint8_t at_recv_index = 0;
static uint8_t at_recv_state = 0;

const unsigned char test_sbc[] = \
	"\x9C\x31\x0C\x02\xCA\x87\x77\x65\x7D\xD7\xDD\x7D\xD7\xDD\x7D\xD7\xDD\x7D\xD8\x1D" \
	"\x8A\x19\x0C\xE6\x46\x9D\x01\xDA\x5D\xF1\xD4\x5D\x9C\x31\x0C\x2F\xC5\x54\x10\x13" \
	"\x12\x5C\x84\xE0\x92\x94\x2A\x4E\x15\xC5\x51\x25\x49\x8F\x32\xA4\x40\x75\x6B\x9F" \
	"\xA1\x81\x00\x49\x9C\x31\x0C\x95\xC6\x54\x10\x13\x8F\x6F\x6D\x5C\xC0\xCA\xB1\x5E" \
	"\x8D\x3C\x51\xEE\xCF\x1D\x4C\x24\xD3\x88\xE2\xDB\x6E\x12\xE5\x69\x9C\x31\x0C\x63" \
	"\xC5\x54\x11\x13\xF2\x59\x66\x0B\x57\x84\xF4\x17\x45\x0B\x89\xB5\xEF\x15\x35\x18" \
	"\x5B\x9A\xDE\x13\x74\x2B\x9D\x4A\x9C\x31\x0C\x2F\xC5\x54\x10\x13\xC7\x52\x04\x45" \
	"\x6E\x68\xAB\x41\x11\x64\x6F\x19\x8A\x10\xC6\x85\x1F\x04\x69\x61\x21\xA5\x4E\x75" \
	"\x9C\x31\x0C\x9D\xC5\x54\x01\x23\x49\x21\xF5\xC1\x1D\x42\x31\x53\x64\xD8\x5B\xB5" \
	"\x1D\x45\x15\xE6\x29\xD0\x14\x47\x0A\xEC\x57\xE1\x9C\x31\x0C\x65\xC5\x54\x10\x23" \
	"\x12\x99\x0A\xE8\x85\xF1\x1B\x5A\xE9\xDC\x54\x31\x2A\x6C\x79\xC8\x92\xD1\x42\x4D" \
	"\x94\xAE\x51\xE6\x9C\x31\x0C\x9B\xC5\x54\x00\x13\x5E\x1E\x55\x91\x91\x84\x7C\x5E" \
	"\x61\x73\x51\x94\x9A\x5E\x11\x56\x12\x39\xB5\x9D\x31\x3C\x03\x59\x9C\x31\x0C\x9B" \
	"\xC5\x54\x00\x13\xCD\xAB\xE1\x2A\x14\xD6\xDB\x8A\x34\x1E\x16\x96\xE4\x98\x61\x1B" \
	"\x68\x64\xE1\x56\x96\x20\x1A\x34\x9C\x31\x0C\x56\xC5\x54\x00\x03\xD9\x54\xE5\x2E" \
	"\x1B\xB5\xC8\x63\x95\x41\x4C\xE2\xB2\x62\x95\x5A\x5D\x95\x97\x02\x14\x75\x9D\xE5" \
	"\x9C\x31\x0C\x83\xC5\x44\x00\x13\x7B\x02\x09\x90\x6D\xA8\x61\x12\x96\xAA\x9C\xF4" \
	"\x49\x23\x76\xBF\x5B\xCA\x37\x64\xC1\xCF\x4A\x66\x9C\x31\x0C\xC9\xC5\x44\x00\x23" \
	"\x2A\x66\x45\xD8\x58\xD6\x24\x87\xE5\xDA\x17\x26\x25\x89\x96\xD3\x65\xA0\x31\x9B" \
	"\x0A\xC6\x14\x44\x9C\x31\x0C\x2F\xC5\x54\x10\x13\x40\x5C\x3A\xB2\x43\x51\x56\x5C" \
	"\xF9\x9C\x42\xB5\x6F\x5D\x69\x83\x92\xA5\x87\x4D\x35\x6A\x62\xE5\x9C\x31\x0C\x83" \
	"\xC5\x44\x00\x13\xA0\x4C\xB6\x54\x83\xA4\xB3\x5B\xC6\x41\x44\xB4\xC4\x6A\x91\x35" \
	"\x46\x1A\xCD\x69\x20\x2E\x47\x8A\x9C\x31\x0C\x9B\xC5\x54\x00\x13\xD1\x57\xB0\x30" \
	"\x68\xF9\xCC\x86\x41\x36\x5A\x59\xC2\x54\xF1\x43\x5B\x75\xB3\x53\xF5\x54\x5C\x55" \
	"\x9C\x31\x0C\xC9\xC5\x44\x00\x23\x9F\xA3\x45\x6A\x2C\xB6\x89\x63\x14\x80\x4C\xC2" \
	"\x72\x53\x48\x96\x6C\x56\x5E\x03\xE9\xAA\x5B\xA1\x9C\x31\x0C\x83\xC5\x44\x00\x13" \
	"\x4B\x04\xB5\xB9\x5A\x94\x40\x25\xE6\xC3\x89\x54\x37\x67\x3A\xC7\x48\x15\x38\x68" \
	"\x84\xC5\x56\xC6\x9C\x31\x0C\xD3\xC4\x44\x00\x03\x3C\x19\xC8\xBD\xA5\xA5\x47\x8A" \
	"\xC6\xB1\x64\xA4\x55\x4B\x96\x9F\x44\x24\x67\xAC\x06\x8D\x03\xC4\x9C\x31\x0C\x44" \
	"\xC4\x43\x00\x02\x7B\xAC\x31\x7A\x13\xC6\x8E\x9C\x08\x66\x14\x26\xA1\x9B\x74\x56" \
	"\x94\xE0\xAE\x4A\x99\x49\x55\xD0\x9C\x31\x0C\x42\xC8\x76\x20\x02\xB8\xD9\x8D\x42" \
	"\xD6\xED\xBC\xD8\x4D\x40\xD8\x2D\xC3\x18\x00\x76\xE7\xED\x7E\xD7\xED\x7E\xD7\xED" \
	"\x9C\x31\x0C\xAD\x00\x00\x00\x11\x68\xDA\x85\x4A\xE4\xDD\x3C\x98\x98\x69\x8B\xE0" \
	"\x92\x65\x32\x5D\xC8\x9F\x7D\x9C\xEB\x51\xC5\xA9\x9C\x31\x0C\x29\x76\x42\x20\x11" \
	"\x7B\xB7\xBB\x7B\xB7\xBB\x7B\xB7\xBB\x7B\xB7\xBB\x7B\xB7\xBB\x7B\xB7\xBB\x7B\xB7" \
	"\xBB\x93\xBC\x98\x9C\x31\x0C\x0F\xC9\x76\x20\x13\x8B\x19\x84\xEA\xDA\xCD\x18\xD1" \
	"\xCD\xB0\xDF\xCD\x8E\xD0\xAD\x32\xDC\xAD\xF2\xD7\x0D\x02\xD4\xCD\x9C\x31\x0C\x45" \
	"\xC5\x65\x10\x03\xE1\x3E\x53\x53\x30\x4C\x68\x2E\xED\xCE\xA3\x8B\x0A\x38\x62\xF4" \
	"\x4B\x4A\x24\xA1\x8B\xA3\x3F\x8A\x9C\x31\x0C\x0B\xC5\x64\x20\x03\x9B\x41\x49\x2C" \
	"\xBB\xD3\xF1\x27\xD4\x0A\xC4\x23\xD2\xCE\x6C\x60\xB0\xA3\x5E\xAE\x24\xD2\x34\x6A" \
	"\x9C\x31\x0C\x2F\xC5\x54\x10\x13\x0C\x67\xA5\xEE\x0B\xD5\x32\x51\x88\x95\x9F\x25" \
	"\xA5\x91\xF6\x27\x5B\x16\xF1\x48\xA1\x13\x23\xB1\x9C\x31\x0C\x2F\xC5\x54\x10\x13" \
	"\xC8\x4E\x82\x6E\x80\xE5\x55\x5D\xA0\xD9\x55\x49\x0E\x86\xEA\xE5\x5C\x59\x3D\x61" \
	"\x74\x8A\x6E\xD6\x9C\x31\x0C\x2F\xC5\x54\x10\x13\xAE\x42\xB1\x25\x1A\x34\xEC\x19" \
	"\x40\x1D\x43\x64\xBB\x5E\x75\x7A\x81\x68\x4C\x6C\xD5\xDA\x96\x16\x9C\x31\x0C\x2F" \
	"\xC5\x54\x10\x13\x13\x66\x4A\xDB\x6C\x95\x4A\x11\x96\x7E\x4E\x51\xB5\x53\x74\x24" \
	"\x59\x64\xE7\x49\xE5\x28\x43\x25\x9C\x31\x0C\xE2\xC5\x54\x10\x03\xAE\x9E\x49\x85" \
	"\x61\xD6\x45\x4C\x36\xDC\x56\xC5\x18\x65\xB0\xD2\x1C\xD1\x56\x41\xB6\x73\x5D\xD4" \
	"\x9C\x31\x0C\x2F\xC5\x54\x10\x13\xBC\x94\x14\x23\x58\xC9\xE1\x9A\x68\x31\x62\xE5" \
	"\xA3\x6E\x15\x8F\x42\x71\x41\x5B\x71\xDB\x17\x84\x9C\x31\x0C\x2F\xC5\x54\x10\x13" \
	"\x1F\x55\x45\xC7\x1C\xF5\x62\x51\xF8\x6A\x5D\x45\xC0\x94\xD6\x25\x58\x19\xDA\x5A" \
	"\xD5\x3C\x12\xE1\x9C\x31\x0C\xB2\xC4\x54\x10\x13\x97\x4D\xC2\x97\x83\x01\x3D\x1A" \
	"\xB4\xD8\x58\x29\x27\x84\xE9\xBD\x9D\x09\x6C\x62\x49\x62\xAC\xB5\x9C\x31\x0C\x2F" \
	"\xC5\x54\x10\x13\xC4\x55\x81\x27\x67\x84\xD3\x5B\x35\x46\x42\xD5\x8D\x5D\x75\x9F" \
	"\x93\x94\x3A\x5A\x15\xD6\x58\xB5\x9C\x31\x0C\x2F\xC5\x54\x10\x13\x2E\x64\x95\xB3" \
	"\x6C\xF5\x76\x52\xA6\x5C\x5C\x11\xC5\x16\x34\x2B\x56\xF5\xCA\x4B\x65\x51\x42\xF5" \
	"\x9C\x31\x0C\xE2\xC5\x54\x10\x03\x83\x5D\x09\xA4\x94\x36\x3A\x49\x7A\xD1\x59\x25" \
	"\x37\x64\x70\xA8\x1C\xC5\x7F\x53\x25\x57\x5B\x74\x9C\x31\x0C\x37\xC5\x44\x10\x13" \
	"\xC4\x66\xC4\x2F\x46\x85\xC3\x9B\x99\x5A\x63\x19\x7B\x6C\xA6\xAA\x44\xC1\x39\x58" \
	"\xE4\xCC\x69\x94\x9C\x31\x0C\x9B\xC5\x54\x00\x13\x3F\x54\x45\x9F\x1C\x94\x87\x53" \
	"\x84\x52\x5A\xE5\xC5\x57\x56\x36\x96\x36\xB9\x5B\xA5\x64\x53\x66\x9C\x31\x0C\xF5" \
	"\xC4\x53\x10\x03\x74\x1C\x26\xAC\x85\x51\x3C\x18\x51\xC5\x59\xE9\x49\x44\x45\x96" \
	"\x5C\x59\x8D\x54\x18\x50\x9A\x46\x9C\x31\x0C\x03\xC4\x44\x00\x12\xC2\x57\xD1\x3D" \
	"\x65\xF1\xB0\x1B\x92\x6D\x03\xB5\x6E\x0B\xA4\xAF\x65\xE4\x3E\x57\xE9\xC0\x8A\x39" \
	"\x9C\x31\x0C\xDA\xC4\x54\x30\x12\x51\x64\x38\x8E\x2C\x15\x93\x54\x85\x4E\x19\xC1" \
	"\xC0\x18\x41\x42\x55\xB1\xA8\x4B\x95\x74\x43\xEA\x9C\x31\x0C\x5C\xB9\x87\x31\x22" \
	"\x45\xBB\xE5\x76\x87\xDB\x7D\xB7\xDB\x7D\xB7\xDB\x7D\xB7\xDB\x7D\xB7\xDB\x7D\xB7" \
	"\xDB\x7D\xB7\xDB\x9C\x31\x0C\x8A\x00\x00\x00\x12\x81\x5B\x9D\x7C\xD2\x9A\x8E\xA6" \
	"\x98\x8D\xB4\x29\x76\x29\x22\x8A\x5B\x23\x6D\xA8\x23\x7E\xE4\xE1\x9C\x31\x0C\x4E" \
	"\xC8\x76\x10\x12\x7E\xD7\xED\x7E\xD7\xED\x7E\xD7\xED\x7E\xD8\x0D\x8B\x19\xA0\xE9" \
	"\x1E\x6D\x78\xD1\x2D\x0A\xD6\x8D\x9C\x31\x0C\x9E\xC5\x44\x00\x12\xDB\x5F\xB1\xAC" \
	"\x53\x72\x02\x93\x95\xAE\x5F\xA5\xD8\x66\x81\x0F\x21\x69\x7B\x5E\x45\xF3\x49\xD5" \
	"\x9C\x31\x0C\x37\xC5\x44\x10\x13\x2C\x50\x54\x4B\x1B\xD5\xF8\x2C\xA5\x5A\x50\xA5" \
	"\x22\x18\xC6\xE8\x8E\x85\x8B\x52\x35\x0C\xA5\x96\x9C\x31\x0C\xDC\xC6\x44\x11\x12" \
	"\xC6\xDF\x4D\xB9\x44\xA9\x0B\x03\x0D\x9A\xDE\xCD\xDC\xC7\xA8\x1E\xA1\x4D\x6A\xDD" \
	"\x2D\xEE\x9A\xAE\x9C\x31\x0C\x2A\xC5\x44\x10\x12\x40\x50\xC5\x3E\x5A\xB5\xF0\x2D" \
	"\x36\x6C\x51\x55\x1D\x57\xA9\xDA\x8E\xA4\x9B\x63\x35\x0E\x94\xC5\x9C\x31\x0C\x1F" \
	"\xC5\x44\x01\x02\xB6\x5E\xF6\xC4\x85\xD5\x14\x92\x94\x89\x5E\x15\xE2\x58\xC4\x2C" \
	"\x61\x59\x5C\x5C\x25\xED\x5B\x78\x9C\x31\x0C\x2A\xC5\x44\x10\x12\x51\x51\x34\x37" \
	"\x29\x95\xE3\x1D\x76\x7D\x52\x45\x1C\x56\xB5\xC9\x5E\x74\xA8\x54\x55\x15\x94\x26" \
	"\x9C\x31\x0C\x2A\xC5\x44\x10\x12\xA4\x6E\x56\xCA\x96\xE6\x21\x92\x75\x79\x5D\x25" \
	"\xE1\x59\xA5\x3C\x21\x85\x50\x5B\x25\xE7\x5C\x15\x9C\x31\x0C\x03\xC4\x44\x00\x12" \
	"\x62\x11\xD5\x30\x18\x89\xD7\x1D\xA6\x8C\x23\x26\x1D\x95\xEA\xBA\x8E\x25\xB2\x95" \
	"\x55\x1C\x53\xA1\x9C\x31\x0C\x4B\xC4\x34\x00\x12\x94\x9D\xB6\xCF\xA7\xD1\x2E\x82" \
	"\x55\x6B\x5C\x41\xDF\x5A\x65\x4C\x51\xF9\x48\x5A\x15\xDC\x5C\x68\x9C\x31\x0C\x2A" \
	"\xC5\x44\x10\x12\x72\x52\x95\x2F\x27\xA8\xC9\x1D\x81\x99\x54\x25\x22\x55\x45\xAA" \
	"\x4D\xA5\xBA\x56\x55\x27\x53\x75\x9C\x31\x0C\x9E\xC5\x44\x00\x12\x85\x5C\xF5\xD1" \
	"\x68\xB2\x3B\x92\x74\x60\x5B\x55\xDB\x5A\xF5\x5B\x52\x75\x41\x49\x25\xD3\x5C\x96" \
	"\x9C\x31\x0C\xCB\xC5\x34\x00\x13\x80\x53\x55\x2E\x16\xD5\xBB\x2D\x54\xA3\x55\x05" \
	"\x27\x54\xB5\x9C\x5D\x25\xBF\x57\x35\x32\x53\x55\x9C\x31\x0C\x14\xC5\x43\x00\x12" \
	"\x78\x5C\x25\xCF\x49\x65\x4A\x92\xC4\x57\x5A\x65\xD2\x5B\x55\x69\x53\x25\x3E\x58" \
	"\x55\xC6\x5C\x95\x9C\x31\x0C\x10\xC5\x33\x01\x12\x8C\x24\x43\x32\x36\x44\xAD\x9C" \
	"\xF3\xAA\xB5\xFB\x30\xA4\x7B\x8E\xCC\x93\xC2\x38\x03\x3D\x23\x53\x9C\x31\x0C\x2A" \
	"\xC5\x44\x10\x12\x6D\x5B\x55\xCC\x49\xF5\x57\x93\x24\x50\x59\x95\xCA\x5B\x95\x76" \
	"\x53\xC5\x3D\x57\x95\xBA\x5C\x75\x9C\x31\x0C\xD6\xC5\x34\x00\x12\x95\x55\x05\x36" \
	"\x55\xC5\xA0\x2C\x85\xAF\x46\xC5\x39\x54\x45\x83\x5B\xC5\xC0\x58\xA5\x4A\x53\x95" \
	"\x9C\x31\x0C\x62\xC5\x34\x10\x12\x65\x5A\x75\xC5\x5A\x55\x64\x83\xB5\x4E\x58\xC5" \
	"\xBF\x5B\x95\x81\x54\x85\x40\x57\x05\xAD\x5C\x25\x9C\x31\x0C\x44\xC4\x43\x00\x02" \
	"\x9C\x55\xD5\x3B\x55\x65\x94\x1C\x06\xB3\x97\x75\x43\x54\x48\x79\x1B\x25\xBE\x59" \
	"\x25\x55\x53\xE4\x9C\x31\x0C\x3D\xB9\x87\x41\x12\x3D\xBB\x5B\xF9\xBD\x1B\x5D\xB0" \
	"\x5B\x15\xB7\xDB\xE5\xBD\x1D\x5F\x07\xD2\x7D\xB7\xDB\x7D\xB7\xDB\x9C\x31\x0C\x8A" \
	"\x00\x00\x00\x12\xAA\xF8\x60\x3D\x46\x56\xB5\xC9\xA2\x79\xB8\x16\xE2\x96\x22\x4A" \
	"\x28\x27\x76\x85\x23\x8D\xC8\x0D\x9C\x31\x0C\x07\x65\x31\x00\x11\x7D\xD7\xDD\x7D" \
	"\xD7\xDD\x7D\xD7\xDD\x7D\xD7\xDD\x7D\xD7\xDD\x7D\xD7\xDD\x7D\xD7\xDD\x9D\xDF\x8C" \
	"\x9C\x31\x0C\xFF\xD8\x65\x10\x12\x83\x18\xA4\xAC\xDB\xED\xB2\xD9\x0D\x68\xE4\x8D" \
	"\x3E\xD5\x0D\x74\xE9\xCC\xB6\xDB\xAD\xA6\xD8\x0C\x9C\x31\x0C\x21\xC5\x65\x00\x12" \
	"\x36\xC0\x82\x0A\x43\xAB\x86\x3D\x0B\xF6\xCE\xCB\xB4\xB6\x6C\x24\xB0\x6A\x18\x35" \
	"\x6C\xA4\x9E\x2B\x9C\x31\x0C\x87\xC4\x64\x10\x12\xF6\xBD\xB3\x99\x24\xC4\x16\x20" \
	"\x8C\x2E\x37\x23\xBC\x3E\xCD\xEF\x1C\x43\x7C\xD3\x4A\x0C\x21\x2B\x9C\x31\x0C\x71" \
	"\xC4\x65\x00\x02\x45\x38\xE9\xD0\x3F\x12\xE2\xBA\xA2\x60\xB2\x49\x0C\xC2\x2B\x60" \
	"\x3A\x8B\xE3\x4F\x0B\xD1\x29\x0D\x9C\x31\x0C\x62\xC4\x54\x10\x02\x47\x51\x64\x11" \
	"\x63\x75\x7C\x4C\x11\xEB\x8E\x95\xBC\x57\x55\x34\x81\x16\x1C\x54\xF1\x96\x5D\x46" \
	"\x9C\x31\x0C\x94\xC4\x55\x00\x12\xEE\x9D\xBA\xA1\x95\xB5\x24\x81\x25\x2C\x56\x94" \
	"\xAE\x1E\x08\xE8\x8C\x71\x88\x54\x54\x1A\x21\x85\x9C\x31\x0C\x94\xC4\x55\x00\x12" \
	"\x41\x48\x35\xC3\x2E\x68\xDF\x5B\x12\x6E\x53\x39\x16\x92\x55\x59\x59\xBA\xD2\x4E" \
	"\x65\xD0\x59\x95\x9C\x31\x0C\x20\xC4\x55\x10\x12\x57\x42\x56\x18\x93\x65\x71\x5B" \
	"\x36\xDE\x5E\x31\xBF\x68\x05\x42\x91\xD9\x21\x54\xA5\x89\x6C\x54\x9C\x31\x0C\x94" \
	"\xC4\x55\x00\x12\xE3\x5D\x86\xA8\x86\x94\x32\x61\xC9\x2E\x06\x16\xA1\xAD\x15\xE0" \
	"\x5C\x85\x91\x55\x39\x27\x92\x09\x9C\x31\x0C\x94\xC4\x55\x00\x12\x3F\x57\x89\xB4" \
	"\x0D\x95\xD9\x5B\x55\x7A\x44\x16\x22\x92\x94\x54\x18\xFA\xC3\x4D\xC6\xCE\x5A\x09" \
	"\x9C\x31\x0C\x3C\xC4\x54\x00\x11\x64\x63\x45\x24\x13\x85\x69\xAA\x44\xCF\x5D\x91" \
	"\xBF\x58\xA4\x50\x62\xA0\x27\xA4\x75\x7F\x6B\x65\x9C\x31\x0C\xBD\xC5\x55\x10\x12" \
	"\xD7\x5D\x35\xAC\x97\x46\x40\x52\x64\x31\x55\xB5\x94\x4C\x45\xD6\x5C\x65\x98\x46" \
	"\x06\x35\x52\x86\x9C\x31\x0C\x1B\xC4\x54\x00\x12\x3F\x27\x05\xA7\x1C\xD6\xD3\x5B" \
	"\x79\x84\x64\xF5\x2E\x42\xF5\x51\x28\x54\xB7\x5D\x11\xCA\x5A\x45\x9C\x31\x0C\x3C" \
	"\xC4\x54\x00\x11\x6F\x54\x20\x2E\x23\xB1\x63\x19\x85\xC2\x6D\x05\xBE\x99\x16\x5D" \
	"\x93\x90\x31\x54\x85\x77\x4A\x91\x9C\x31\x0C\x1B\xC4\x54\x00\x12\xC9\x5C\xB4\xAE" \
	"\x57\xD6\x4E\x03\x26\x36\x55\x75\x89\x5B\x76\xCC\x8C\x25\x9D\x66\xB5\x42\x43\x25" \
	"\x9C\x31\x0C\x86\xC5\x54\x00\x12\x41\x56\xA4\x9B\x1C\x04\xCA\x5B\x64\x8B\x55\xB4" \
	"\x3A\x63\x65\x51\x17\xC5\xAA\x6C\x54\xC4\x5A\x76\x9C\x31\x0C\x1B\xC4\x54\x00\x12" \
	"\x79\x54\xE4\x39\x13\xF5\x60\x48\xE5\xB5\x4C\x75\xBB\x59\x65\x69\x84\x55\x39\x54" \
	"\xA1\x70\x59\xD6\x9C\x31\x0C\xCD\xCA\x88\x78\x76\xBB\x5B\xB5\xAB\x58\x35\x5B\x54" \
	"\x29\x75\x48\x31\x7B\x57\xB5\x7B\x57\xB5\x7B\x57\xB5\x7B\x57\xB5";



extern struct user_bt_env_tag *user_bt_env;
extern uint8_t pbap_addr_index;
#include "ff.h"

void btdm_tx_test_func(void *arg)
{
    printf("cnt = %d\r\n",system_get_btdm_active_cnt());
    
    os_timer_start(&timer_btdm_tx_test, 100, false);
    //btdm_enter_tx_testmode(tx_type,tx_freq,0,tx_pwr);

}
void app_at_recv_cmd_A(uint8_t sub_cmd, uint8_t *data)
{
    //struct bd_addr addr;
    //uint8_t tmp_data = 0;
    enum bt_state_t bt_state = bt_get_state();
    bool ret = false;

    switch(sub_cmd)
    {
        case 'A':
            
            GLOBAL_INT_DISABLE();
            
            gpio_set_pin_value(GPIO_PORT_B, GPIO_BIT_0,1);
            co_delay_100us(100);
            gpio_set_pin_value(GPIO_PORT_B, GPIO_BIT_0,0);
            GLOBAL_INT_RESTORE();
            //pskeys_update_app_data();
            printf("OK\r\n");
            break;
        case 'B':
            if((bt_state <= BT_STATE_CONNECTED)&&(true == user_check_native_playback_allowable(USER_EVT_NATIVE_PLAYBACK_NEXT))){
                //ret = native_playback_action_next();
                ret = bt_statemachine(USER_EVT_NATIVE_PLAYBACK_NEXT,NULL);
            }
            if(ret == false){
                uart_send("FAIL\r\n",6);
            }

            break;
        case 'C':
            
            if((bt_state <= BT_STATE_CONNECTED)&&(true == user_check_native_playback_allowable(USER_EVT_NATIVE_PLAYBACK_START))){
                //ret = native_playback_action_play();
                ret = bt_statemachine(USER_EVT_NATIVE_PLAYBACK_START,NULL);
            }
            if(ret == false){
                uart_send("FAIL\r\n",6);
            }

            break;
        case 'D':
            if(true == user_check_native_playback_allowable(USER_EVT_NATIVE_PLAYBACK_STOP)){
                ret = bt_statemachine(USER_EVT_NATIVE_PLAYBACK_STOP,NULL);
            }
            if(ret == true){
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }
            //ret = native_playback_action_pause();
            break;
        case 'E':
            if((bt_state <= BT_STATE_CONNECTED)&&(true == user_check_native_playback_allowable(USER_EVT_NATIVE_PLAYBACK_PREV))){
                //ret = native_playback_action_prev();
                ret = bt_statemachine(USER_EVT_NATIVE_PLAYBACK_PREV,NULL);
            }
            if(ret == false){
                uart_send("FAIL\r\n",6);
            }

            break;
        
        case 'G':
            printf("hello world!!!!\r\n");
            co_delay_100us(3000);
            break;
        case 'H':
            printf("VAL: 0x%08x.\r\n", REG_PL_RD(ascii_strn2val((const char *)&data[0], 16, 8)));
            break;
        case 'I':
            REG_PL_WR(ascii_strn2val((const char *)&data[0], 16, 8), ascii_strn2val((const char *)&data[9], 16, 8));
            printf("OK\r\n");
            break;
            
        case 'J':
            if((user_bt_env->dev[user_bt_env->last_active_dev_index].conFlags&LINK_STATUS_HF_CONNECTED) && (user_bt_env->dev[user_bt_env->last_active_dev_index].active == HF_CALL_NONE)) {
                hf_enable_voice_recognition(user_bt_env->dev[user_bt_env->last_active_dev_index].hf_chan, TRUE);
            }
            uart_send("OK\r\n",4);
            break;
        case 'K':
            if((user_bt_env->dev[user_bt_env->last_active_dev_index].conFlags&LINK_STATUS_HF_CONNECTED) && (user_bt_env->dev[user_bt_env->last_active_dev_index].active == HF_CALL_NONE)) {
                hf_enable_voice_recognition(user_bt_env->dev[user_bt_env->last_active_dev_index].hf_chan, FALSE);
            }
            uart_send("OK\r\n",4);
            break;
        case 'L':
            audio_play_tone(0x80+ascii_strn2val((const char*)&data[0],16,2));
            uart_send("OK\r\n",4);
            break;
        case 'M':
            if(bt_state <= BT_STATE_CONNECTED){
                //audio_codec_func_start(AUDIO_CODEC_FUNC_MIC_LOOP);
                #if DSP_MIC_LOOP
                dsp_open(DSP_LOAD_TYPE_VOICE_ALGO);
                #endif
                bt_statemachine(USER_EVT_MIC_LOOP_START,NULL);
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }
            break;
        case 'N':
            //audio_codec_func_stop(AUDIO_CODEC_FUNC_MIC_LOOP);
            bt_statemachine(USER_EVT_MIC_LOOP_STOP,NULL);
            uart_send("OK\r\n",4);
            break;
        case 'P':
            printf("cnt = %d\r\n",system_get_btdm_active_cnt());
            break;
        case 'Q':
            system_set_btdm_active_cnt(0);
            printf("OK\r\n");
            break;
        case 'S':
            if(bt_state <= BT_STATE_CONNECTED){
                //audio_codec_func_start(AUDIO_CODEC_FUNC_MIC_ONLY);
                #if DSP_MIC_LOOP
                dsp_open(DSP_LOAD_TYPE_VOICE_ALGO);
                #endif
                ret = bt_statemachine(USER_EVT_MIC_ONLY_START,NULL);
            }
            if(ret == true){
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }
            break;
        case 'T':
            //audio_codec_func_stop(AUDIO_CODEC_FUNC_MIC_ONLY);
            bt_statemachine(USER_EVT_MIC_ONLY_STOP,NULL);
            uart_send("OK\r\n",4);
            break;
            
        case 'U':
            {
                uint32_t *ptr = (uint32_t *)(ascii_strn2val((const char *)&data[0], 16, 8) & (~3));
                uint8_t count = ascii_strn2val((const char *)&data[9], 16, 2);
                uint32_t *start = (uint32_t *)((uint32_t)ptr & (~0x0f));
                printf("ptr is 0x%08x, count = %d.\r\n", ptr, count);
                for(uint8_t i=0; i<count;) {
                    if(((uint32_t)start & 0x0c) == 0) {
                        printf("0x%08x: ", start);
                    }
                    if(start < ptr) {
                        printf("        ");
                    }
                    else {
                        i++;
                        printf("%08x", *start);
                    }
                    if(((uint32_t)start & 0x0c) == 0x0c) {
                        printf("\r\n");
                    }
                    else {
                        printf(" ");
                    }
                    start++;
                }
            }
            break;
        case 'V':
            if((bt_get_a2dp_type() == BT_A2DP_TYPE_SRC)&&((bt_state == BT_STATE_CONNECTED)||(bt_state == BT_STATE_MEDIA_PLAYING))){
                ret = audio_source_action_next();
            }
            if(ret == false){
                uart_send("FAIL\r\n",6);
            }
            break;
        case 'W':
            if((bt_get_a2dp_type() == BT_A2DP_TYPE_SRC)&&(bt_state == BT_STATE_CONNECTED)){
                ret = audio_source_action_play();
            }
            if(ret == false){
                uart_send("FAIL\r\n",6);
            }

            break;
        case 'X':
            if((bt_get_a2dp_type() == BT_A2DP_TYPE_SRC)&&(bt_state == BT_STATE_MEDIA_PLAYING)){
                ret = audio_source_action_pause();
            }
            if(ret == false){
                uart_send("FAIL\r\n",6);
            }else{
                uart_send("OK\r\n",4);
            }           
            break;
        case 'Y':
            if((bt_get_a2dp_type() == BT_A2DP_TYPE_SRC)&&((bt_state == BT_STATE_CONNECTED)||(bt_state == BT_STATE_MEDIA_PLAYING))){
                ret = audio_source_action_prev();
            }
            if(ret == false){
                uart_send("FAIL\r\n",6);
            }

            break;
        case 'Z':
            bt_rd_rem_name();
            //fs_prepare_next();
            //fs_prepare_next();
            break;
        case 'y':
            msbc_playback_action_play();
            break;
        case 'z':
            msbc_playback_action_pause();
            break;            
        default:
            break;
    }
}

extern int8_t rxrssi;
void app_at_recv_cmd_B(uint8_t sub_cmd, uint8_t *data)
{
    BtSniffInfo sniff_info; 
    BD_ADDR addr;
    
    switch(sub_cmd)
    {
        case 'A':
            bt_start_inquiry(5,5);
            printf("OK\r\n");
        break;
        case 'B':
            bt_cancel_inquiry();
            printf("OK\r\n");
        break;

        case 'C':
            bt_set_accessible_mode_nc(BAM_GENERAL_ACCESSIBLE,NULL);
            printf("OK\r\n");
        break;
    
        case 'D':
            bt_set_accessible_mode_nc(BAM_NOT_ACCESSIBLE,NULL);
            printf("OK\r\n");
        break;
        
        case 'E':
            sniff_info.maxInterval = 0x320;
            sniff_info.minInterval = 0x320;
            sniff_info.attempt = 0x01;
            sniff_info.timeout = 0x00;
            bt_start_sniff(user_bt_env->dev[0].remDev,(const BtSniffInfo *)&sniff_info);
            printf("OK\r\n");
        break;
        
        case 'F':
            //a2dp_close_stream(user_bt_env->dev[0].pstream);
            //a2dp_force_disconnect(user_bt_env->dev[0].pstream);
            a2dp_remove_datalink();
            //bt_reset();
            printf("OK\r\n");
            //printf("current mode = %d\r\n",bt_get_current_mode(user_bt_env->dev[0].remDev));
        break;
        
        case 'G':
            bt_reconnect(RECONNECT_TYPE_USER_CMD,NULL);
            printf("OK\r\n");
        break;

        case 'H':
            bt_set_spk_volume(BT_VOL_MEDIA,ascii_strn2val((const char*)&data[0],16,2));
            printf("OK\r\n");
        break;

        case 'I':
            bt_set_spk_volume(BT_VOL_HFP,ascii_strn2val((const char*)&data[0],16,2));
            printf("OK\r\n");
        break;
        
        case 'J':
            bt_disconnect();
            printf("OK\r\n");
        break;

        case 'K':
            hf_redial(user_bt_env->dev[0].hf_chan);
            printf("OK\r\n");
        break;
        
        case 'L':
            //hf_hang_up(user_bt_env->dev[0].hf_chan);
            //printf("OK\r\n");
            if(data[2] == '_'){
                ool_write(ascii_strn2val((const char*)&data[0],16,2), ascii_strn2val((const char*)&data[3],16,2));
                printf("\r\nOK\r\n");

            }else{
                printf("\r\n0x%x\r\n",ool_read(ascii_strn2val((const char*)&data[0],16,2)));
            }

        break;
        
        case 'M':
            if(data[8] == '\r') {
            //read uint32
                printf("\r\n0x%x\r\n",REG_PL_RD(ascii_strn2val((const char*)&data[0],16,8)));
                return;
            }else if(data[8] == '_') {
                REG_PL_WR(ascii_strn2val((const char*)&data[0],16,8), ascii_strn2val((const char*)&data[9],16,8));
                printf("\r\nOK\r\n");
            }
        break;

        case 'N':
            #if 0
            addr.A[0] = 0xA7;
            addr.A[1] = 0x50;
            addr.A[2] = 0x58;
            addr.A[3] = 0x16;
            addr.A[4] = 0x52;
            addr.A[5] = 0x1c;
            #else
            addr.A[0] = 0x23;
            addr.A[1] = 0xd2;
            addr.A[2] = 0x57;
            addr.A[3] = 0x16;
            addr.A[4] = 0x52;
            addr.A[5] = 0x1c;

            #endif
            memcpy(&user_bt_env->dev[0].bd,&addr,6);
            a2dp_open_stream(AVDTP_STRM_ENDPOINT_SNK, &addr);
            printf("OK\r\n");
        break;

        case 'O':
            if(bt_get_a2dp_type() == BT_A2DP_TYPE_SINK) {
                bt_set_a2dp_type(BT_A2DP_TYPE_SRC);
            }
            else if(bt_get_a2dp_type() == BT_A2DP_TYPE_SRC) {
                bt_set_a2dp_type(BT_A2DP_TYPE_SINK);
            }
            printf("OK\r\n");
        break;
#if 0
        case 'P':
            a2dp_start_stream(user_bt_env->dev[0].pstream);
            printf("OK\r\n");
        break;
        
        case 'Q':
            a2dp_suspend_stream(user_bt_env->dev[0].pstream);
            printf("OK\r\n");
        break;
#else
        case 'P':
            printf("\r\n0x%x\r\n",REG_PL_RD8(MDM_BASE+ascii_strn2val((const char*)&data[0],16,2)));
            break;
        case 'Q':
            REG_PL_WR8(MDM_BASE+ascii_strn2val((const char*)&data[0],16,2), ascii_strn2val((const char*)&data[3],16,2));
            printf("\r\nOK\r\n");
            break;

#endif
        case 'R':
            if(BT_STATUS_PENDING == hf_enable_voice_recognition(user_bt_env->dev[0].hf_chan, TRUE)){
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }
        break;
        case 'S':
            if(BT_STATUS_PENDING == hf_enable_voice_recognition(user_bt_env->dev[0].hf_chan, FALSE)){
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }
        break;
        case 'T':
            printf("enter bt test mode\r\n");
            bt_enter_bredr_testmode();
            printf("OK\r\n");
        break;
        case 'U':
            if(ascii_strn2val((const char*)&data[0],16,2) == 0){
                printf("enter ble test mode , hci uart: PB6,PB7\r\n");
                co_delay_100us(10);
                bt_enter_ble_testmode(GPIO_PB6,GPIO_PB7);
            }else if(ascii_strn2val((const char*)&data[0],16,2) == 1){
                printf("enter ble test mode , hci uart: PA0,PA1\r\n");
                co_delay_100us(10);
                bt_enter_ble_testmode(GPIO_PA0,GPIO_PA1);
            }else if(ascii_strn2val((const char*)&data[0],16,2) == 2){
                printf("enter ble test mode , hci uart: PA6,PA7\r\n");
                co_delay_100us(10);
                bt_enter_ble_testmode(GPIO_PA6,GPIO_PA7);
            }
        break;
        case 'V':
            printf("enter bt sensitivity test\r\n");
            bt_enter_rx_sensitivity_testmode();
            //avrcp_ct_get_media_info(user_bt_env->dev[0].rcp_chan, 0x41);
            printf("OK\r\n");
        break;
        case 'W':
            {
                uint8_t *data_spp = os_malloc(10);
                memcpy(data_spp,"123456789a",10);
                spp_send_data(data_spp, 10);
                printf("OK\r\n");
            }
        break;
        
        case 'X':
            //printf("rssi = %d\r\n",rxrssi);
            dsp_open(DSP_LOAD_TYPE_VOICE_ALGO);
            //memset(&pskeys.app_data,0,sizeof(struct pskeys_app_t));
            //flash_erase(0x2000, 0x1000);
            
            printf("OK\r\n");
        break;
                
        case 'Y':
            gap_stop_advertising();
            bt_set_accessible_mode_nc(BAM_NOT_ACCESSIBLE,NULL);
            os_timer_init(&timer_btdm_tx_test,btdm_tx_test_func,NULL);
            os_timer_start(&timer_btdm_tx_test, 100, false);
            tx_type = ascii_strn2val((const char*)&data[0],16,2);
            tx_freq = ascii_strn2val((const char*)&data[3],16,2);
            tx_pwr = ascii_strn2val((const char*)&data[6],16,2);
            //btdm_enter_tx_testmode(, ascii_strn2val((const char*)&data[3],16,2), 0, ascii_strn2val((const char*)&data[6],16,2))
            printf("OK\r\n");
        break;
                
        case 'Z':
            dsp_close();
            //pmu_power_off_DSP();
        printf("OK\r\n");
        break;
        default:
            
        break;
    }
}

uint8_t bt_inquiry_flag = false;
uint8_t bt_change_a2dp_flag = 0;
extern uint8_t ipc_mp3_start_flag;
extern uint8_t remote_dev_type;
void app_at_recv_cmd_C(uint8_t sub_cmd, uint8_t *data)
{
    BD_ADDR addr;    
    BtStatus status;
    enum bt_state_t bt_state = bt_get_state();
    uint8_t i = 0;

    switch(sub_cmd)
    {
        case 'A':
            if((bt_state == BT_STATE_HFP_INCOMMING)
                &&(BT_STATUS_PENDING == hf_answer_call(user_bt_env->dev[user_bt_env->last_active_dev_index].hf_chan))){
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }

        break;
        case 'B':
            if(((bt_state == BT_STATE_HFP_INCOMMING)||(bt_state == BT_STATE_HFP_OUTGOING)||(bt_state == BT_STATE_HFP_CALLACTIVE))
                &&(BT_STATUS_PENDING == hf_hang_up(user_bt_env->dev[user_bt_env->last_active_dev_index].hf_chan))){
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }

        break;

        case 'C':
            if(((bt_state == BT_STATE_CONNECTED)||(bt_state == BT_STATE_MEDIA_PLAYING))
                &&(BT_STATUS_PENDING == hf_redial(user_bt_env->dev[user_bt_env->last_active_dev_index].hf_chan))){
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }

        break;
    
        case 'D':
            if((bt_state == BT_STATE_MEDIA_PLAYING)
                &&(BT_STATUS_PENDING == avrcp_set_panel_key(user_bt_env->dev[user_bt_env->last_active_dev_index].rcp_chan, AVRCP_POP_PAUSE,TRUE))){
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }

        break;
        
        case 'E':
            if((bt_state == BT_STATE_CONNECTED)
                &&(BT_STATUS_PENDING == avrcp_set_panel_key(user_bt_env->dev[user_bt_env->last_active_dev_index].rcp_chan, AVRCP_POP_PLAY,TRUE))){
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }

        break;
        
        case 'F':
            if(((bt_state == BT_STATE_CONNECTED)||(bt_state == BT_STATE_MEDIA_PLAYING))
                &&(BT_STATUS_PENDING == avrcp_set_panel_key(user_bt_env->dev[user_bt_env->last_active_dev_index].rcp_chan, AVRCP_POP_FORWARD,TRUE))){
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }
        break;
        
        case 'G':
            if(((bt_state == BT_STATE_CONNECTED)||(bt_state == BT_STATE_MEDIA_PLAYING))
                &&(BT_STATUS_PENDING == avrcp_set_panel_key(user_bt_env->dev[user_bt_env->last_active_dev_index].rcp_chan, AVRCP_POP_BACKWARD,TRUE))){
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }
        break;

        case 'H':
            if((bt_state == BT_STATE_PAIRING)||(bt_state == BT_STATE_IDLE)){
                bt_set_accessible_mode_nc(BAM_NOT_ACCESSIBLE,NULL);
                bt_start_inquiry(10,0);
                bt_inquiry_flag = true;
                uart_send("OK\r\n",4);
            }else {
                uart_send("FAIL\r\n",6);
            }
        break;

        case 'I':
            bt_cancel_inquiry();
            bt_set_accessible_mode_nc(BAM_GENERAL_ACCESSIBLE,NULL);
            bt_inquiry_flag = false;
            uart_send("OK\r\n",4);
        break;

        case 'J':
            if(bt_state != BT_STATE_PAIRING){
                uart_send("FAIL\r\n",6);
            }else{
                if(data[0] == '\r') {
                    if(false == bt_reconnect(RECONNECT_TYPE_USER_CMD,NULL)){
                        uart_send("FAIL\r\n",6);
                    }else{
                        uart_send("OK\r\n",4);
                    }
                }else if(data[12] == '\r'){
                    addr.A[0] = ascii_strn2val((const char*)&data[0],16,2);
                    addr.A[1] = ascii_strn2val((const char*)&data[2],16,2);
                    addr.A[2] = ascii_strn2val((const char*)&data[4],16,2);
                    addr.A[3] = ascii_strn2val((const char*)&data[6],16,2);
                    addr.A[4] = ascii_strn2val((const char*)&data[8],16,2);
                    addr.A[5] = ascii_strn2val((const char*)&data[10],16,2); 
                    if(false == bt_reconnect(RECONNECT_TYPE_USER_CMD,&addr)){
                        uart_send("FAIL\r\n",6);
                    }else{
                        uart_send("OK\r\n",4);
                    }
                }else{
                    uart_send("FAIL\r\n",6);
                }
            }
        break;

        case 'K':
            bt_disconnect();
            uart_send("OK\r\n",4);
        break;
        
        case 'L':
            if(bt_inquiry_flag == true){
				uart_send("+STATE:1\r\n", 10);
            }else{
                if(bt_state == BT_STATE_PAIRING){
					uart_send("+STATE:0\r\n", 10);
                }else if(bt_state == BT_STATE_CONNECTING){
					uart_send("+STATE:2\r\n", 10);
                }else if(bt_state == BT_STATE_CONNECTED){
					uart_send("+STATE:3\r\n", 10);
                }else if((bt_state == BT_STATE_HFP_CALLACTIVE)||(bt_state == BT_STATE_HFP_INCOMMING)||(bt_state == BT_STATE_HFP_OUTGOING)){
					uart_send("+STATE:4\r\n", 10);
                }else if(bt_state == BT_STATE_MEDIA_PLAYING){
					uart_send("+STATE:5\r\n", 10);
                }else{
					uart_send("+STATE:6\r\n", 10);
                }
            }

        break;
        
        case 'M':
            if(user_bt_env->dev[user_bt_env->last_active_dev_index].mode == 2){
                printf("+MODE:1\r\n");
            }else if(user_bt_env->dev[user_bt_env->last_active_dev_index].mode == 0){
                printf("+MODE:0\r\n");
            }
        break;

        case 'N':
            while(data[i] != '\r'){
                i++;
            }
            if(((bt_state == BT_STATE_CONNECTED)||(bt_state == BT_STATE_MEDIA_PLAYING))
                &&(BT_STATUS_PENDING == hf_dial_number(user_bt_env->dev[user_bt_env->last_active_dev_index].hf_chan,data,i))){
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }

        break;

        case 'P':
            codec_eq_param_switch();
        break;

        case 'O':
            if(user_bt_env->access_state == ACCESS_IDLE){
                status = bt_set_a2dp_type(BT_A2DP_TYPE_SRC);
                bt_set_accessible_mode_nc(BAM_GENERAL_ACCESSIBLE,NULL);
                if(status == BT_STATUS_SUCCESS){
                    uart_send("OK\r\n",4);
                }else{
                    uart_send("FAIL\r\n",6);
                }
            }else{
                bt_change_a2dp_flag = 1;
                bt_set_accessible_mode_nc(BAM_NOT_ACCESSIBLE,NULL);
            }

        break;
        
        case 'Q':
            if(user_bt_env->access_state == ACCESS_IDLE){
                status = bt_set_a2dp_type(BT_A2DP_TYPE_SINK);
                bt_set_accessible_mode_nc(BAM_GENERAL_ACCESSIBLE,NULL);
                if(status == BT_STATUS_SUCCESS){
                    uart_send("OK\r\n",4);
                }else{
                    uart_send("FAIL\r\n",6);
                }
            }else{
                bt_change_a2dp_flag = 2;
                bt_set_accessible_mode_nc(BAM_NOT_ACCESSIBLE,NULL);
            }

        break;

        case 'R':
            if((data[0] == 'H')&&(data[1] == 'F')){
                if(true == bt_set_spk_volume(BT_VOL_HFP,ascii_strn2val((const char*)&data[3],16,2))){
                    uart_send("OK\r\n",4);
                }else{
                    uart_send("FAIL\r\n",6);
                }

            }
            else if((data[0] == 'A')&&(data[1] == 'D')){
                if(true == bt_set_spk_volume(BT_VOL_MEDIA,ascii_strn2val((const char*)&data[3],16,2))){
                    uart_send("OK\r\n",4);
                }else{
                    uart_send("FAIL\r\n",6);
                }

            }else{
                uart_send("FAIL\r\n",6);
            }
        break;
            
        case 'S':
            gap_stop_advertising();
            printf("OK\r\n");
            break;
        case 'T':
            gap_start_advertising(0);
            printf("OK\r\n");
            break;
            
        case 'X':
            //gap_stop_advertising();
            if(bt_state == BT_STATE_PAIRING){
                bt_set_accessible_mode_nc(BAM_NOT_ACCESSIBLE,NULL);
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }


            break;
        case 'Y':
            //gap_start_advertising(0);
            if(bt_state == BT_STATE_IDLE){
                bt_set_accessible_mode_nc(BAM_GENERAL_ACCESSIBLE,NULL);
                uart_send("OK\r\n",4);
            }else{
                uart_send("FAIL\r\n",6);
            }

            break;
        case 'Z':
            //bt_stack_reset();
            printf("a2dp type = %d\r\n",bt_get_a2dp_type());
            break;
        default:
        break;
    }
}

os_timer_t req_msbc_timer;
void req_msbc_timer_handler(void *arg)
{
    ipc_msg_send(IPC_MSG_WITHOUT_PAYLOAD, IPC_SUB_MSG_NEED_MORE_MSBC, NULL);
    os_timer_start(&req_msbc_timer, 10, false);
}

static void app_at_recv_cmd_D(uint8_t sub_cmd, uint8_t *data)
{
    switch(sub_cmd) {
        case 'A':
            os_timer_init(&req_msbc_timer, req_msbc_timer_handler, NULL);
            os_timer_start(&req_msbc_timer, 100, false);
            system_set_voice_config();
            codec_adc_reg_config(AUDIO_CODEC_SAMPLE_RATE_ADC_16000, AUDIO_CODEC_ADC_DEST_IPC_DMA, 0x20);
            ipc_config_mic_only_dma();
            dsp_open(DSP_LOAD_TYPE_MSBC_ENCODER);
            break;
        default:
            break;
    }
}

extern uint8_t music_player_open_flag;

void app_at_recv_cmd_F(uint8_t sub_cmd, uint8_t *data)
{
    enum bt_state_t bt_state = bt_get_state();
    uint8_t uart_buf[20];
    switch(sub_cmd)
    {
        case 'A':
            if(false == fs_handle_mp3_info_req(NULL,ascii_strn2val((const char*)&data[0],16,2))){
                uart_send("FAIL\r\n",6);
            }
            break;
        case 'B':
            if(false == fs_handle_mp3_list_req(NULL,ascii_strn2val((const char*)&data[0],16,2))){
                uart_send("FAIL\r\n",6);
            }
            break;
        case 'C':
            co_sprintf((void *)uart_buf,"+MNUM:%02x\r\n",fs_get_mp3_num());
            uart_buf[10] = '\0';
            uart_send(uart_buf,10);
            break;
        case 'D':
            ///get bt addr
            co_sprintf((void *)uart_buf,"+MAC:%02x%02x%02x%02x%02x%02x\r\n",pskeys.local_bdaddr.addr[0],pskeys.local_bdaddr.addr[1], \
                pskeys.local_bdaddr.addr[2],pskeys.local_bdaddr.addr[3],pskeys.local_bdaddr.addr[4],pskeys.local_bdaddr.addr[5]);
            uart_buf[19] = '\0';
            uart_send(uart_buf,19);
            break;
        case 'E':
            ///get sdk version
            uart_send("+VER:1.3.11\r\n",13);
            break;
        case 'F':
            bt_set_user_evt_notify_enable(ascii_strn2val((const char*)&data[0],16,2));
            uart_send("OK\r\n",4);
            //native_playback_action_specified(item1);
        break;
        case 'G':
            if(bt_get_state() >= BT_STATE_CONNECTED){
                bt_set_accessible_mode_nc(BAM_NOT_ACCESSIBLE,NULL);
                bt_disconnect();
            }else{
                bt_set_accessible_mode_nc(BAM_NOT_ACCESSIBLE,NULL);
            }
            gap_stop_advertising();
            pskeys.slp_max_dur = 0x3a0000;
            system_sleep_direct_enable();
            
            uart_send("OK\r\n",4);
        break;

        case 'H':
            audio_play_sbc((void *)test_sbc,sizeof(test_sbc));
            uart_send("OK\r\n",4);
        break;
        
        case 'I':
            audio_stop_sbc();
            uart_send("OK\r\n",4);
        break;

        case 'J':
            remote_dev_type = ascii_strn2val((const char*)&data[0],16,2);
            uart_send("OK\r\n",4);
            break;
            
        case 'K':
            //open music player
            if(music_player_open_flag == 0){
                ///若responder标志位没有被置起，表示是HF端主动连接，此时需要根据pskeys及状态看是否需要建立A2DP连接
                if(((user_bt_env->dev[0].conFlags&LINK_STATUS_AV_CONNECTED)==0)
                    &&(pskeys.enable_profiles & ENABLE_PROFILE_A2DP)
                    &&(bt_a2dp_get_connecting_state() == FALSE)){
                    //printf("a2dp open stream\r\n");
                    a2dp_open_stream(AVDTP_STRM_ENDPOINT_SRC, &user_bt_env->dev[0].bd);
                }
                music_player_open_flag = 1;
            }
            break;
        case 'L':
            //close music player
            if(music_player_open_flag == 1){
                if(user_bt_env->dev[0].conFlags&LINK_STATUS_AVC_CONNECTED){
                    avrcp_disconnect(user_bt_env->dev[0].rcp_chan);
                }
                if(user_bt_env->dev[0].conFlags&LINK_STATUS_AV_CONNECTED){
                    a2dp_close_stream(user_bt_env->dev[0].pstream);
                }
                music_player_open_flag = 0;
            }
            break;
        case 'M':
            //delete linkkey
            memset(&pskeys.app_data,0,sizeof(struct pskeys_app_t));
            flash_erase(0x2000,0x1000);
            uart_send("OK\r\n",4);
            break;
        case 'N':
            printf("local addr=%x,%x,%x,%x,%x,%x\r\n",pskeys.local_bdaddr.addr[0],pskeys.local_bdaddr.addr[1],pskeys.local_bdaddr.addr[2],pskeys.local_bdaddr.addr[3],pskeys.local_bdaddr.addr[4],pskeys.local_bdaddr.addr[5]);
            break;
        case 'O':
            
            uart_send("OK\r\n",4);
            break;
        case 'P':
            
            uart_send("OK\r\n",4);
            break;
        case 'Q':
            uint8_t hid_data[4];
            hid_data[0] = ascii_strn2val((const char*)&data[0],16,2);
            hid_data[1] = ascii_strn2val((const char*)&data[2],16,2);
            hid_data[2] = ascii_strn2val((const char*)&data[4],16,2);
            //hid_data[3] = ascii_strn2val((const char*)&data[6],16,2);
            bt_hid_send_mouse_data(hid_data);
            uart_send("OK\r\n",4);
            break;
        case 'R':
            #include "driver_adc.h"
            adc_init(ADC_CHANNEL_VBAT, 0, 1);
            ool_write(PMU_REG_AUXADC_PWR_EN, ool_read(PMU_REG_AUXADC_PWR_EN)|BIT7);
            adc_start();
            co_delay_100us(10);
            //result = adc_read_data();
            printf("vbat=%d\r\n",triming_bat_cap(ADC_REF_SEL_INTERNAL_1_2v,adc_read_data()));
            adc_stop();
            break;
        case 'S':
            //avrcp_connect(&user_bt_env->dev[0].bd);
            bt_hid_open_connection(&user_bt_env->dev[0].bd);
            uart_send("OK\r\n",4);
            break;
        case 'T':
            //avrcp_connect(&user_bt_env->dev[0].bd);
            extern uint8_t bt_hid_send_ctrl_data(uint8_t data);
            bt_hid_send_ctrl_data(ascii_strn2val((const char*)&data[0],16,2));
            bt_hid_send_ctrl_data(0);
            uart_send("OK\r\n",4);
            break;
        case 'X':
            show_ke_malloc();
            break;
        default:
            break;
    };
}

#if FIXED_FREQ_TX_TEST
struct lld_test_params
{
    /// Type (0: RX | 1: TX)
    uint8_t type;

    /// RF channel, N = (F - 2402) / 2
    uint8_t channel;

    /// Length of test data
    uint8_t data_len;

    /**
     * Packet payload
     * 0x00 PRBS9 sequence "11111111100000111101" (in transmission order) as described in [Vol 6] Part F, Section 4.1.5
     * 0x01 Repeated "11110000" (in transmission order) sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x02 Repeated "10101010" (in transmission order) sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x03 PRBS15 sequence as described in [Vol 6] Part F, Section 4.1.5
     * 0x04 Repeated "11111111" (in transmission order) sequence
     * 0x05 Repeated "00000000" (in transmission order) sequence
     * 0x06 Repeated "00001111" (in transmission order) sequence
     * 0x07 Repeated "01010101" (in transmission order) sequence
     * 0x08-0xFF Reserved for future use
     */
    uint8_t payload;

    /**
     * Tx/Rx PHY
     * For Tx PHY:
     * 0x00 Reserved for future use
     * 0x01 LE 1M PHY
     * 0x02 LE 2M PHY
     * 0x03 LE Coded PHY with S=8 data coding
     * 0x04 LE Coded PHY with S=2 data coding
     * 0x05-0xFF Reserved for future use
     * For Rx PHY:
     * 0x00 Reserved for future use
     * 0x01 LE 1M PHY
     * 0x02 LE 2M PHY
     * 0x03 LE Coded PHY
     * 0x04-0xFF Reserved for future use
     */
    uint8_t phy;
};

#define POWER_MAX 0x3f
os_timer_t ble_tx_test_timer;
static uint8_t curr_ble_test_flag = 0;
static uint8_t test_power = 0;
static uint8_t test_chan = 0;
static uint8_t test_type = 0;

void ble_start_tx_test(uint8_t power,uint8_t chan,uint8_t type)
{

    REG_PL_WR8(0x500210f1,0xc8);  //shall not change if dsp is working

    *(volatile uint32_t *)0x400008d0 = 0x80008000;
    
    //REG_PL_WR8(0x50021051,0x40|power);
    REG_PL_WR8(0x50021081,power);
    
    struct lld_test_params test_params;
    test_params.type = 1;
    test_params.channel = chan/2 - 1;//0-39
    test_params.data_len = 0x25;
    test_params.payload = type;// type
    test_params.phy = 1;
    //lld_test_start(&test_params);
    ((uint8_t (*)(struct lld_test_params*))(0x000558f9))(&test_params);
    curr_ble_test_flag = 1;
    
}

void ble_exit_tx_test(void)
{
    REG_PL_WR8(0x500210f1,0xc8);  //shall not change if dsp is working
    REG_PL_WR8(0x5002100d,0x6f);
    REG_PL_WR8(0x50021002,0x03);
    //REG_PL_WR8(0x5002104b,0xa1);
    
    REG_PL_WR8(0x50021081,0x1f);
    //lld_test_stop();
    ((uint8_t (*)(void))(0x00055b75))();

    curr_ble_test_flag = 0;
}

void ble_tx_test_timer_handler(void *arg)
{
    if(curr_ble_test_flag ==0){
        ble_start_tx_test(test_power, test_chan, test_type);
    }
    else{
        ble_exit_tx_test();
    }
    
    os_timer_start(&ble_tx_test_timer,20,false);
}


static void app_at_recv_cmd_T(uint8_t sub_cmd, uint8_t *data)
{
    uint32_t val_B,temp;
    uint8_t val_A;
    uint8_t power,channel,type;
        
    switch(sub_cmd)
    {
        //ble 1Mbps 调制发�?
        case 'A':
            
            REG_PL_WR8(0x500210f1,0xc8);  //shall not change if dsp is working
            // parameter check
            power = ascii_strn2val((const char*)&data[6],16,2);
            channel = ascii_strn2val((const char *)&data[0], 16, 2);
            type = ascii_strn2val((const char *)&data[3], 16, 2);
            
            if(power > POWER_MAX){
                printf("error power parameter,power shall be in range[0x00,0x3f]....\r\n");
                return;
            }
            if(channel&0x01){
                printf("error channel parameter,channel shall be even....\r\n");
                return;
            }else if((channel == 0)||(channel > 80)){
                printf("error channel parameter,channel shall be in range[0x02,0x50]....\r\n");
                return;
            }
            if(type>7){
                printf("error data type parameter,type shall be in range[0x00,0x07]....\r\n");
                return;
            }
            
            *(volatile uint32_t *)0x400008d0 = 0x80008000;

            //REG_PL_WR8(0x50021051,0x40|power);
            REG_PL_WR8(0x50021081,power);

            struct lld_test_params test_params;
            test_params.type = 1;
            test_params.channel = channel/2 - 1;//0-39
            test_params.data_len = 0x25;
            test_params.payload = ascii_strn2val((const char *)&data[3], 16, 2);// type
            test_params.phy = 1;
            //lld_test_start(&test_params);
            ((uint8_t (*)(struct lld_test_params*))(0x000558f9))(&test_params);
            
            printf("OK\r\n");
        break;
        //单载波发射，非调�?
        case 'B':
            // parameter check
            power = ascii_strn2val((const char*)&data[6],16,2);
            channel = ascii_strn2val((const char *)&data[0], 16, 2);
            type = ascii_strn2val((const char *)&data[3], 16, 2);
            if(power > POWER_MAX){
                printf("error power parameter,power shall be in range[0x00,0x3f]....\r\n");
                return;
            }
            if((channel < 2)||(channel > 80)){
                printf("error channel parameter,channel shall be in range[0x02,0x50]....\r\n");
                return;
            }

            GLOBAL_INT_DISABLE();
            REG_PL_WR8(0x500210f1,0xff);
            REG_PL_WR8(0x50021000,0x00);
            REG_PL_WR8(0x50021001,0xff);

            //信道
            temp = 2400 + channel;
            val_A = temp/18;
            val_B = (temp-val_A*18)*0xffffff/18;
            
            REG_PL_WR8(0x50021023,val_A);
            REG_PL_WR8(0x50021024,(val_B>>16)&0xff);
            REG_PL_WR8(0x50021025,(val_B>>8)&0xff);
            REG_PL_WR8(0x50021026,val_B&0xff);
            
            REG_PL_WR8(0x5002100d,0x7e);
            REG_PL_WR8(0x500210f2,0x00);
            REG_PL_WR8(0x500210f2,0x01);
            
            co_delay_100us(1);
            REG_PL_WR8(0x50021002,0x01);
            REG_PL_WR8(0x50021002,0x81);
            
            //REG_PL_WR8(0x50021051,0x40|power);
            REG_PL_WR8(0x50021081,power);
            //REG_PL_WR8(0x5002104b,0xa9);
            GLOBAL_INT_RESTORE();
            printf("OK\r\n");
        break;

        //停止测试
        case 'C':
            REG_PL_WR8(0x500210f1,0xc8);  //shall not change if dsp is working
            REG_PL_WR8(0x5002100d,0x6f);
            REG_PL_WR8(0x50021002,0x03);
            //REG_PL_WR8(0x5002104b,0xa1);
            
            REG_PL_WR8(0x50021081,0x1f);
            //lld_test_stop();
            ((uint8_t (*)(void))(0x00055b75))();
            printf("OK\r\n");

        break;

        // BR 调制发�?
        case 'D':
            //infinite tx/rx
            *(volatile uint32_t *)0x400004d0 = 0x80008000;

            //bt_rftestfreq_rxfreq_setf(2);
            
        break;

        //EDR 调制发�?
        case 'E':
        break;

        case 'H':
            // parameter check
            power = ascii_strn2val((const char*)&data[6],16,2);
            channel = ascii_strn2val((const char *)&data[0], 16, 2);
            type = ascii_strn2val((const char *)&data[3], 16, 2);
            
            if(power > POWER_MAX){
                printf("error power parameter,power shall be in range[0x00,0x3f]....\r\n");
                return;
            }
            if(channel&0x01){
                printf("error channel parameter,channel shall be even....\r\n");
                return;
            }else if((channel == 0)||(channel > 80)){
                printf("error channel parameter,channel shall be in range[0x02,0x50]....\r\n");
                return;
            }
            if(type>7){
                printf("error data type parameter,type shall be in range[0x00,0x07]....\r\n");
                return;
            }
            
            bt_set_accessible_mode_nc(BAM_NOT_ACCESSIBLE,NULL);
            gap_stop_advertising();

            test_power = power;
            test_chan = channel;
            test_type = type;
            os_timer_init(&ble_tx_test_timer,ble_tx_test_timer_handler,NULL);
            os_timer_start(&ble_tx_test_timer,20,false);
            printf("OK\r\n");
            break;

        default:
            break;

    }       
}
#endif


#if PBAP_FUC == 1
os_timer_t pcconnect_req_timeout_handle_id ={
  NULL  
};
static void pcconnect_req_timeout_handle(void *param)
{
   printf("+PTO");
   pba_abort_client(); 
}
void user_pull_sim_phonebook(uint8_t phonetype,uint16_t list_offset,uint16_t maxlist)  
{
    static      Pbap_Vcard_Filter filter;
    uint16_t    listOffset, maxListCount;
    char        pName[64];     
    listOffset = list_offset;
    maxListCount = maxlist;
    memset(pName,0,sizeof(pName));
    memset(&filter.byte,0,sizeof(Pbap_Vcard_Filter));
    filter.byte[0] = 0x84;
    switch(phonetype)
    {
        case PhoneBookList:
            memcpy(pName,PB_SIM_STORE_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
        case IncomingCallList:
            memcpy(pName,PB_SIM_ICH_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
        case OutgoingCallList:
            memcpy(pName,PB_SIM_OCH_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
        case MissCallList:
            memcpy(pName,PB_SIM_MCH_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
        case CombinedCallList:
            memcpy(pName,PB_SIM_CCH_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
        case SpeedDialList:
            memcpy(pName,PB_SIM_SPD_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
        case FavoriteNumberList:
            memcpy(pName,PB_SIM_FAV_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
        default:
            memcpy(pName,PB_SIM_STORE_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
    }
    PBA_PullPhonebook(pName, &filter, maxListCount, listOffset,VCARD_FORMAT_30);
}

void user_pull_phonebook(uint8_t phonetype,uint16_t list_offset,uint16_t maxlist)
{
    
    static      Pbap_Vcard_Filter filter;
    uint16_t    listOffset, maxListCount;
    char        pName[64];     
    listOffset = list_offset;
    maxListCount = maxlist;
    memset(pName,0,sizeof(pName));
    memset(&filter.byte,0,sizeof(Pbap_Vcard_Filter));
    filter.byte[0] = 0x84;
    switch(phonetype)
    {
        case PhoneBookList:
            memcpy(pName,PB_LOCAL_STORE_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
        case IncomingCallList:
            memcpy(pName,PB_LOCAL_ICH_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
        case OutgoingCallList:
            memcpy(pName,PB_LOCAL_OCH_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
        case MissCallList:
            memcpy(pName,PB_LOCAL_MCH_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
        case CombinedCallList:
            memcpy(pName,PB_LOCAL_CCH_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
        case SpeedDialList:
            memcpy(pName,PB_LOCAL_SPD_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
        case FavoriteNumberList:
            memcpy(pName,PB_LOCAL_FAV_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
        default:
            memcpy(pName,PB_LOCAL_STORE_NAME,sizeof(PB_LOCAL_STORE_NAME));
            break;
    }
    PBA_PullPhonebook(pName, &filter, maxListCount, listOffset,VCARD_FORMAT_30);
}
#if 0
void user_pull_vcarlist(void)
{
    char        searchValue[15];
    uint16_t    listOffset, maxListCount;
    listOffset = 0;
    maxListCount = 0xffff;
    memset(searchValue,0,15);
     
    PBA_PullVcardListing(PB_LOCAL_STORE_NAME,VCARD_SEARCH_ATTRIB_NUMBER,searchValue,VCARD_SORT_ORDER_INDEXED,maxListCount,listOffset);
}
void user_entry_vcard(void)
{
    Pbap_Vcard_Filter filter;
    memset(&filter.byte,0,sizeof(Pbap_Vcard_Filter));
    PBA_PullVcardEntry(PB_LOCAL_STORE_NAME,&filter,VCARD_FORMAT_30);
}
#endif

void app_at_recv_cmd_P(uint8_t cmd,uint8_t *param)
{
    switch(cmd)
    {
        case 'C':
        {
            if(!pbap_connect_flag)
            {
                uint8_t bt_addr[6];
                memcpy(bt_addr,&user_bt_env->dev[pbap_addr_index].bd,6);
                pba_connect(bt_addr);
                //uart_send("OK\r\n",4);
                if(pcconnect_req_timeout_handle_id.timer_id == NULL)
                {
                    os_timer_init(&pcconnect_req_timeout_handle_id,pcconnect_req_timeout_handle,NULL);
                }
                //os_timer_start(&pcconnect_req_timeout_handle_id,5000,false);
            }else{
                uart_send("FAIL\r\n",6);
            }
        }
            break;
        
        case 'P':
        {
            if(pbap_connect_flag)
            {
                uint16_t list_offset = 0,max_list  = 0xffff;
                uint8_t phone_type = 0;
                phone_type = ascii_strn2val((const char*)param,16,2);
                list_offset = ascii_strn2val((const char*)param+3,16,4);
                max_list = ascii_strn2val((const char*)param+8,16,4);           
                //printf("list_offset=%x,max list=%x\r\n",list_offset,max_list);
                user_pull_phonebook(phone_type,list_offset,max_list); 
                //printf("OK");
            }else{
                uart_send("FAIL\r\n",6);
            }
        }
            break;
        case 'S':
        {
            if(pbap_connect_flag)
            {
                uint16_t list_offset = 0,max_list  = 0xffff;
                uint8_t phone_type = 0;
                phone_type = ascii_strn2val((const char*)param,16,2);
                list_offset = ascii_strn2val((const char*)param+3,16,4);
                max_list = ascii_strn2val((const char*)param+8,16,4);
                user_pull_sim_phonebook(phone_type,list_offset,max_list); 
                //printf("OK");
            }else{
                uart_send("FAIL\r\n",6);
            }
        }
            break;
        case 'T':
        {
            if(pbap_connect_flag)
            {
                pba_disconnect();
                //printf("OK");
            }else{
                uart_send("FAIL\r\n",6);
            }
        }
            break;
        default:
            //printf("FAIL");
            break;
    }
}
#endif

void report_att_value(uint8_t fu8_att_index, uint8_t fu8_length, uint8_t *fp8_data)
{
    uint8_t i;
    uint8_t lu8_Buffer[30];

    lu8_Buffer[0] = fu8_att_index;
    lu8_Buffer[1] = fu8_length;

    for (i = 0; i < fu8_length; i++)
    {
        lu8_Buffer[i + 2] = fp8_data[i];
    }

    comm_add_cmd(TYPE_EVENT, TYPE_EVENT_1, fu8_length + 2, lu8_Buffer);
}

void app_at_recv_cmd_L(uint8_t cmd, uint8_t *param)
{
    switch(cmd)
    {
        /* ����BLE�㲥 */
        case TYPE_CMD_L_A:
        {
            gap_start_advertising(0);
        }break;

        /* �ر�BLE�㲥 */
        case TYPE_CMD_L_B:
        {
            gap_stop_advertising();
        }break;

        /* BLE�������� */
        case TYPE_CMD_L_C:
        {
            mac_addr_t Mac_addr;

            Mac_addr.addr[0] = ascii_strn2val((const char*)&param[0], 16, 2);
            Mac_addr.addr[1] = ascii_strn2val((const char*)&param[2], 16, 2);
            Mac_addr.addr[2] = ascii_strn2val((const char*)&param[4], 16, 2);
            Mac_addr.addr[3] = ascii_strn2val((const char*)&param[6], 16, 2);
            Mac_addr.addr[4] = ascii_strn2val((const char*)&param[8], 16, 2);
            Mac_addr.addr[5] = ascii_strn2val((const char*)&param[10], 16, 2);

            gap_start_conn(&Mac_addr, 0, 30, 30, 0, 300);
        }break;

        /* ��ATT����ֵ */
        case TYPE_CMD_L_R:
        {
            co_printf("read att_index: %d", param[0]);
            
            report_att_value(param[0], 5, "\xBD\xAD\x10\x11\x20");
        }break;
        
        /* дATT����ֵ */
        case TYPE_CMD_L_W:
        {
            co_printf("att_index: %d", param[0]);
            co_printf("length %d", param[1]);
            
            for (int i = 0; i < param[1]; i++)
            {
                co_printf("data[%d]: 0x%02X", i, param[2 + i]);
            }
        }break;

        default: break;
    }
}

void app_at_cmd_recv_handler(uint8_t *data, uint16_t length)
{
    switch(data[0])
    {
        case 'A':
            
            //uart_putc_noint('2');
            //printf("%c,%c,%c,%c\r\n",data[1],data[2],data[3],data[4]);
            app_at_recv_cmd_A(data[1], &data[2]);
            break;
        case 'B':
            app_at_recv_cmd_B(data[1], &data[2]);
            break;
        case 'C':
            app_at_recv_cmd_C(data[1], &data[2]);
            break;  
//        case 'D':
//            app_at_recv_cmd_D(data[1], &data[2]);
//            break;
        case 'F':
            app_at_recv_cmd_F(data[1], &data[2]);
            break;
#if PBAP_FUC == 1
        case 'P':
            app_at_recv_cmd_P(data[1],&data[2]);
            break;
#endif
        case 'L':
            app_at_recv_cmd_L(data[1], &data[2]);
            break;
        #if FIXED_FREQ_TX_TEST
        case 'T':
            app_at_recv_cmd_T(data[1], &data[2]);
            break;

        #endif
        default:
            break;
    }
}

#define __RAM_CODE          __attribute__((section("ram_code")))
__RAM_CODE static void app_at_recv_c(uint8_t c)
{
    switch(at_recv_state)
    {
        case 0:
            if(c == 'A')
            {
                at_recv_state++;
            }
            break;
        case 1:
            if(c == 'T')
                at_recv_state++;
            else
                at_recv_state = 0;
            break;
        case 2:
            if(c == '#')
                at_recv_state++;
            else
                at_recv_state = 0;
            break;
        case 3:
            at_recv_buffer[at_recv_index++] = c;
            if((c == '\n')
               ||(at_recv_index >= AT_RECV_MAX_LEN))
            {
                os_event_t at_cmd_event;
                at_cmd_event.event_id = USER_EVT_AT_COMMAND;///event id
                at_cmd_event.param = at_recv_buffer;        ///���ڽ���buffer
                at_cmd_event.param_len = at_recv_index;     ///���ڽ���buffer����
                os_msg_post(user_task_id, &at_cmd_event);   ///������Ϣ
                at_recv_state = 0;
                at_recv_index = 0;
            }
            break;
    }
}

__attribute__((section("ram_code"))) void app_at_uart_recv(void*dummy, uint8_t status)
{
    app_at_recv_c(app_at_recv_char);
    uart0_read_for_hci(&app_at_recv_char, 1, app_at_uart_recv, NULL);
}

#if COPROCESSOR_UART_ENABLE
__attribute__((section("ram_code"))) void app_at_init_app(void)
{
    system_set_port_pull_up(GPIO_PA6, true);
    system_regs->reset_ctrl.cm3_uart_sft_rst = 1;
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_CM3_UART_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_7, PORTA7_FUNC_CM3_UART_TXD);
    
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_6, PORTB6_FUNC_B6);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_7, PORTB7_FUNC_B7);
    uart_init(BAUD_RATE_115200);
    //uart0_read_for_hci(&app_at_recv_char, 1, app_at_uart_recv, NULL);
}
#elif FR5088_COMM_ENABLE
__attribute__((section("ram_code"))) void app_at_init_app(void)
{
    system_set_port_pull_up(GPIO_PB6, true);
    system_regs->reset_ctrl.cm3_uart_sft_rst = 1;
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_A6);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_7, PORTA6_FUNC_A6);
    
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_6, PORTB6_FUNC_CM3_UART_RXD);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_7, PORTB7_FUNC_CM3_UART_TXD);
	
    uart_init(BAUD_RATE_115200);
}
#elif USB_DEVICE_ENABLE 
__attribute__((section("ram_code"))) void app_at_init_app(void)
{
    system_set_port_pull_up(GPIO_PA6, true);
    system_regs->reset_ctrl.cm3_uart_sft_rst = 1;
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_6, PORTA6_FUNC_CM3_UART_RXD);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_7, PORTA7_FUNC_CM3_UART_TXD);
    
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_6, PORTB6_FUNC_B6);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_7, PORTB7_FUNC_B7);
    uart_init(BAUD_RATE_115200);
    uart0_read_for_hci(&app_at_recv_char, 1, app_at_uart_recv, NULL);
}

#else

__attribute__((section("ram_code"))) void app_at_init_app(void)
{
//    uart_putc_noint('A');
    //printf("app_at init app\r\n");
    system_set_port_pull_up(GPIO_PB6, true);
    system_regs->reset_ctrl.cm3_uart_sft_rst = 1;
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_6, PORTB6_FUNC_CM3_UART_RXD);
    system_set_port_mux(GPIO_PORT_B, GPIO_BIT_7, PORTB7_FUNC_CM3_UART_TXD);
    
    //uart_putc_noint('B');
#if FIXED_FREQ_TX_TEST|BLE_TEST_MODE
    uart_init(BAUD_RATE_115200);
#else
    uart_init(BAUD_RATE_115200);
#endif
    //uart_putc_noint('C');
#if !BLE_TEST_MODE
    uart0_read_for_hci(&app_at_recv_char, 1, app_at_uart_recv, NULL);
#endif
    //printf("after app_at init app\r\n");
    //uart_putc_noint('D');
}
#endif


#if COPROCESSOR_UART_ENABLE

#if 0
void uart_slave_wake_timer_func(void *arg)
{
    struct uart_slave_msg_elem_t *elem;
    LOG_INFO("uart_slave_wake_timer_func\r\n");
    if(uart_slave_env.state == UART_SLEEP_WAKING_STATE){
        uart_slave_env.state = UART_SLEEP_STATE;
        system_sleep_enable();
        while(!co_list_is_empty(&uart_slave_env.msg_list)){
            elem = (struct uart_slave_msg_elem_t *)co_list_pop_front(&uart_slave_env.msg_list);
            os_free((void *)elem->data);
            os_free((void *)elem);
        }
        uart_slave_env.msg_total_num = 0;
    }
}

void uart_slave_check_timer_func(void *arg)
{
    LOG_INFO("uart_slave_check_timer_func\r\n");
    if(uart_slave_env.state == UART_WORK_STATE){
        uart_write("+SLEEP\r\n",sizeof("+SLEEP\r\n"));
        co_delay_100us(10);
        uart_slave_env.state = UART_SLEEP_STATE;
        system_sleep_enable();
    }
}

void uart_slave_send_wake_ind(void)
{
    struct uart_slave_msg_elem_t *elem;
    LOG_INFO("uart_slave_send_wake_ind,%d\r\n",uart_slave_env.state);
    if(uart_slave_env.state == UART_SLEEP_WAKING_STATE){
        os_timer_stop(&uart_slave_wake_timer);
        uart_slave_env.state = UART_WORK_STATE;
        os_timer_start(&uart_slave_check_timer,5000,false);
        while(!co_list_is_empty(&uart_slave_env.msg_list)){
            elem = (struct uart_slave_msg_elem_t *)co_list_pop_front(&uart_slave_env.msg_list);
            uart_write(elem->data,elem->data_len);
            os_free((void *)elem->data);
            os_free((void *)elem);
            uart_slave_env.msg_total_num --;
        }
    }
    if(uart_slave_env.state != UART_WORK_STATE){
        system_sleep_disable();
        uart_write("+WAKE\r\n",sizeof("+WAKE\r\n"));
        uart_slave_env.state = UART_WORK_STATE;
        os_timer_start(&uart_slave_check_timer,5000,false);
    }

}

void uart_slave_init(void)
{
    //pmu_set_pin_to_PMU(UART_SLAVE_WAKE_PIN_PORT, 1<<UART_SLAVE_WAKE_PIN_BIT);
    //pmu_set_pin_dir(UART_SLAVE_WAKE_PIN_PORT, 1<<UART_SLAVE_WAKE_PIN_BIT, GPIO_DIR_IN);

    
    pmu_set_pin_to_PMU(UART_SLAVE_IND_PIN_PORT, 1<<UART_SLAVE_IND_PIN_BIT);
    pmu_set_pin_dir(UART_SLAVE_IND_PIN_PORT, 1<<UART_SLAVE_IND_PIN_BIT, GPIO_DIR_OUT);
    pmu_set_gpio_value(UART_SLAVE_IND_PIN_PORT, 1<<UART_SLAVE_IND_PIN_BIT,1);

    os_timer_init(&uart_slave_wake_timer,uart_slave_wake_timer_func,NULL);
    co_list_init(&uart_slave_env.msg_list);
    
    uart_slave_env.state = UART_WORK_STATE;
    //system_sleep_disable();
    //os_timer_init(&uart_slave_gpio_debounce_timer,uart_slave_gpio_debounce_timer_func,NULL);
    os_timer_init(&uart_slave_check_timer,uart_slave_check_timer_func,NULL);
    os_timer_start(&uart_slave_check_timer,5000,false);
    
}

void uart_send_to_master(uint8_t *data, uint8_t len)
{
    struct uart_slave_msg_elem_t *elem;
    if(uart_slave_env.state == UART_WORK_STATE){
        uart_write(data,len);
        os_timer_start(&uart_slave_check_timer,5000,false);
    }
    else if(uart_slave_env.state == UART_SLEEP_STATE){
        system_sleep_disable();
        uart_slave_env.state = UART_SLEEP_WAKING_STATE;
        elem = (struct uart_slave_msg_elem_t *)os_malloc(sizeof(struct uart_slave_msg_elem_t));
        elem->data_len = len;
        elem->data = (uint8_t *)os_malloc(len);
        memcpy(elem->data,data,len);
        co_list_push_back(&uart_slave_env.msg_list, &elem->hdr);
        pmu_set_gpio_value(UART_SLAVE_IND_PIN_PORT, 1<<UART_SLAVE_IND_PIN_BIT, 0);
        os_timer_start(&uart_slave_wake_timer,500,false);
    }
    else if(uart_slave_env.state == UART_SLEEP_WAKING_STATE){
        elem->data_len = len;
        elem->data = (uint8_t *)os_malloc(len);
        memcpy(elem->data,data,len);
        co_list_push_back(&uart_slave_env.msg_list, &elem->hdr);
        os_timer_start(&uart_slave_wake_timer,500,false);
    }
    else{
        ///shutoff state,shall not be here
        //printf("error:data can't send in shutoff state.\r\n");
    }
}
#endif

uint8_t uart_recv[512];

void uart_slave_init(void)
{
    //enable pc4 as wakeup pin
    button_init(GPIO_PC4);
    pmu_set_pin_pull_up(GPIO_PORT_C, 1<<GPIO_BIT_4,true);
    pmu_port_wakeup_func_set(GPIO_PC4);
    
    pmu_set_pin_to_PMU(UART_SLAVE_IND_PIN_PORT, 1<<UART_SLAVE_IND_PIN_BIT);
    pmu_set_pin_dir(UART_SLAVE_IND_PIN_PORT, 1<<UART_SLAVE_IND_PIN_BIT, GPIO_DIR_OUT);
    pmu_set_gpio_value(UART_SLAVE_IND_PIN_PORT, 1<<UART_SLAVE_IND_PIN_BIT,1);

    uart_lp_env.rx_state = UART_LP_RX_STATE_IDLE;
    
}

void uart_recv_callback(void*dummy, uint8_t status)
{
//    struct uart_msg_elem_t *elem = NULL;
    os_event_t uart_tx_ready_event;
//    uint8_t rx_size = 0;
    uint8_t rx_state = uart_lp_env.rx_state;
    //printf("5080 rx:%d,%c,%c\r\n",rx_state,uart_recv[0],uart_recv[6]);
    
    //uart_putc_noint(rx_state);
    switch(rx_state)
    {
        case UART_LP_RX_STATE_IDLE:
            if(memcmp(&uart_recv[0],"AA#OK\r\n",7) == 0){
                ///recv mcu ack,post msg,send buffer data in task
                uart_enable_isr(0, 0);
                pmu_set_gpio_value(UART_SLAVE_IND_PIN_PORT, 1<<UART_SLAVE_IND_PIN_BIT,1);
                uart_tx_ready_event.event_id = USER_EVT_UART_TX_READY;
                uart_tx_ready_event.param = NULL;        
                uart_tx_ready_event.param_len = 0;     
                os_msg_post(user_task_id, &uart_tx_ready_event); 
            }
            break;
        case UART_LP_RX_STATE_HEADER:
            if(memcmp(&uart_recv[0],"ZZ#",3) == 0){
                ///recv mcu data,5 bytes header first
                uart_lp_env.rx_size = ascii_strn2val((const char*)&uart_recv[3],16,4);
                //uart_lp_env.rx_size = (uart_recv[3<<8)|uart_recv[4];
                uart_lp_env.rx_state = UART_LP_RX_STATE_DATA;
                uart_enable_isr(1, 0);
                uart0_read_for_hci(&uart_recv[0], uart_lp_env.rx_size, uart_recv_callback, NULL);
            }
            break;
        case UART_LP_RX_STATE_DATA:
            os_event_t at_cmd_event;
            
            at_cmd_event.event_id = USER_EVT_AT_COMMAND;
            at_cmd_event.param = &uart_recv[3]; //memcpy here? TBD
            at_cmd_event.param_len = uart_lp_env.rx_size-3;
            os_msg_post(user_task_id, &at_cmd_event);

            uart_lp_env.rx_state = UART_LP_RX_STATE_IDLE;
            uart_enable_isr(0,0);
            system_sleep_enable();
            break;
        default:
            break;
    }
    
}

void uart_send_ack(void)
{
    uart_write("AA#OK\r\n",7);
    uart_enable_isr(1, 0);
    uart_lp_env.rx_state = UART_LP_RX_STATE_HEADER;
    uart0_read_for_hci(&uart_recv[0], 7, uart_recv_callback, NULL);
}

void uart_req_tx(void)
{
    pmu_set_gpio_value(UART_SLAVE_IND_PIN_PORT, 1<<UART_SLAVE_IND_PIN_BIT,0);
    uart_enable_isr(1, 0);
    uart_lp_env.rx_state = UART_LP_RX_STATE_IDLE;
    uart0_read_for_hci(&uart_recv[0], 7, uart_recv_callback, NULL);
}

void uart_send(uint8_t *data, uint16_t data_len)
{
    struct uart_msg_elem_t *elem;
    elem = (struct uart_msg_elem_t*)os_malloc(sizeof(struct uart_msg_elem_t));
    elem->data_len = data_len;
    elem->data = (uint8_t *)os_malloc(data_len);
    memcpy(elem->data,data,data_len);
    GLOBAL_INT_DISABLE();
    co_list_push_back(&uart_lp_env.msg_list, &elem->hdr);
    uart_lp_env.pending_num ++;
    GLOBAL_INT_RESTORE();
    if((uart_lp_env.tx_sending == false)&&(uart_lp_env.rx_pending == false)){
        uart_lp_env.tx_sending = true;
        //uart_putc_noint('T');
        uart_req_tx();
    }
}

__attribute__((section("ram_code"))) void pmu_gpio_isr_ram(void)
{
    uint32_t gpio_value = ool_read32(PMU_REG_PORTA_INPUT_VAL);
    
    //printf("gpio new value = 0x%08x.\r\n", gpio_value);
    if((gpio_value&GPIO_PC4) == 0){
        if(uart_lp_env.tx_sending == true){
            uart_lp_env.rx_pending = true;
        }else{
            //ack mcu,enable uart rx isr, set recv threshold to 5(recv header)
            system_sleep_disable();
            uart_send_ack();
        }
    }
    ool_write32(PMU_REG_STATE_MON_IN_A,gpio_value);

}
#elif FR5088_COMM_ENABLE

void uart_send(uint8_t *fp8_Data, uint16_t fu16_Length)
{
    uint8_t lu8_HeadBuffer[8];
    uint8_t lu8_Count;

    uint8_t  lu8_type_index;
    uint8_t *lu8_Data;
    uint8_t  lu8_DataLength;

    if (fp8_Data[0] == '+')
    {
        lu8_Count = 0;

        while (lu8_Count < 8)
        {
            lu8_HeadBuffer[lu8_Count] = fp8_Data[lu8_Count];

            if (lu8_HeadBuffer[lu8_Count] == ':') 
            {
                lu8_Data       = 0;
                lu8_DataLength = 0;

                switch (lu8_Count)
                {
                    case 3: 
                    {
                        if (strncmp((const char *)lu8_HeadBuffer, "+MN", 3) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_MN;
                            lu8_Data       = &fp8_Data[4];
                            lu8_DataLength = fu16_Length - 7;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+PP", 3) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_PP;
                            lu8_Data       = &fp8_Data[4];
                            lu8_DataLength = 4;
                        }
                    }break;

                    case 4:
                    {
                        if (strncmp((const char *)lu8_HeadBuffer, "+INQ", 4) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_INQ;
                            lu8_Data       = &fp8_Data[5];
                            lu8_DataLength = fu16_Length - 7;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+ACC", 4) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_ACC;
                            lu8_Data       = &fp8_Data[5];
                            lu8_DataLength = fu16_Length - 7;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+CLI", 4) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_CLI;
                            lu8_Data       = &fp8_Data[5];
                            lu8_DataLength = fu16_Length - 7;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+VOL", 4) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_VOL;
                            lu8_Data       = &fp8_Data[5];
                            lu8_DataLength = fu16_Length - 7;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+MP3", 4) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_MP3;
                            lu8_Data       = &fp8_Data[5];
                            lu8_DataLength = fu16_Length - 7;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+PEC", 4) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_PEC;
                            lu8_Data       = &fp8_Data[5];
                            lu8_DataLength = fu16_Length - 7;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+PEA", 4) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_PEA;
                            lu8_Data       = &fp8_Data[5];
                            lu8_DataLength = fu16_Length - 7;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+MAC", 4) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_MAC;
                            lu8_Data       = &fp8_Data[5];
                            lu8_DataLength = fu16_Length - 7;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+VER", 4) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_VER;
                            lu8_Data       = &fp8_Data[5];
                            lu8_DataLength = fu16_Length - 7;
                        }
                    }break;

                    case 5:
                    {
                        if (strncmp((const char *)lu8_HeadBuffer, "+DISC", 5) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_DISC;
                            lu8_Data       = &fp8_Data[6];
                            lu8_DataLength = fu16_Length - 8;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+NAME", 5) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_NAME;
                            lu8_Data       = &fp8_Data[6];
                            lu8_DataLength = fu16_Length - 6;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+MNUM", 5) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_MNUM;
                            lu8_Data       = &fp8_Data[6];
                            lu8_DataLength = fu16_Length - 8;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+PROG", 5) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_PROG;
                            lu8_Data       = &fp8_Data[6];
                            lu8_DataLength = fu16_Length - 8;
                        }
                    }break;
					
					case 6:
					{
						if (strncmp((const char *)lu8_HeadBuffer, "+STATE", 6) == 0)
						{
                            lu8_type_index = TYPE_EVENT_STATE;
                            lu8_Data       = &fp8_Data[7];
                            lu8_DataLength = fu16_Length - 9;
						}
					}break;

                    default: break; 
                }

                comm_add_cmd(TYPE_EVENT, lu8_type_index, lu8_DataLength, lu8_Data);

                break;
            }
            else if (lu8_HeadBuffer[lu8_Count] == '\r')
            {
                switch (lu8_Count)
                {
                    case 3: 
                    {
                        if (strncmp((const char *)lu8_HeadBuffer, "+AC", 3) == 0) 
                        {
                            lu8_type_index = TYPE_EVENT_AC;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+AD", 3) == 0) 
                        {
                            lu8_type_index = TYPE_EVENT_AD;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+MS", 3) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_MS;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+MT", 3) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_MT;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+MC", 3) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_MC;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+MN", 3) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_MN;
                        }
                    }break;

                    case 5:
                    {
                        if (strncmp((const char *)lu8_HeadBuffer, "+CONN", 5) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_CONN;
                        }
                        else if (strncmp((const char *)lu8_HeadBuffer, "+RING", 5) == 0)
                        {
                            lu8_type_index = TYPE_EVENT_RING;
                        }
                    }break;

                    default: break; 
                }

                comm_add_cmd(TYPE_EVENT, lu8_type_index, 0, 0);
            }

            lu8_Count++;
        }
    } 
    else 
    {
        if (strcmp((const char *)fp8_Data, "OK\r\n") == 0) 
        {
            comm_add_cmd(TYPE_STATUS, TYPE_STATUS_OK, 0,0);
        }
        else if (strcmp((const char *)fp8_Data, "FAIL\r\n") == 0)
        {
            comm_add_cmd(TYPE_STATUS, TYPE_STATUS_FAIL, 0,0);
        }
    }
}

#else
void uart_send(uint8_t *data, uint16_t data_len)
{
    for(uint16_t i = 0; i < data_len; i++)
    {
        uart_putc_noint(data[i]);
    }
}

#endif

typedef void (*rwip_eif_callback) (void*, uint8_t);

struct uart_txrxchannel
{
    /// call back function pointer
    rwip_eif_callback callback;
};

struct uart_env_tag
{
    /// rx channel
    struct uart_txrxchannel rx;
    uint32_t rxsize;
    uint8_t *rxbufptr;
    void *dummy;
    /// error detect
    uint8_t errordetect;
    /// external wakeup
    bool ext_wakeup;
};

#if COPROCESSOR_UART_ENABLE
__attribute__((section("ram_code"))) void uart_isr_ram(void)
{
    uint8_t int_id;
    uint8_t c;
    rwip_eif_callback callback;
    void *dummy;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART_BASE;
    struct uart_env_tag *uart1_env = (struct uart_env_tag *)0x200015bc;
    int_id = uart_reg->u3.iir.int_id;
    if(int_id == 0x04 || int_id == 0x0c )   /* Receiver data available or Character time-out indication */
    {
        while(uart_reg->lsr & 0x01)
        {
            c = uart_reg->u1.data;
            *uart1_env->rxbufptr++ = c;
            uart1_env->rxsize--;
            
            //uart_putc_noint(uart1_env->rxsize);
            if((uart1_env->rxsize == 0)
               &&(uart1_env->rx.callback))
            {
                uart_reg->u3.fcr.data = 0xf1;
                NVIC_DisableIRQ(UART_IRQn);
                uart_reg->u3.fcr.data = 0x21;
                callback = uart1_env->rx.callback;
                dummy = uart1_env->dummy;
                uart1_env->rx.callback = 0;
                uart1_env->rxbufptr = 0;
                callback(dummy, 0);
                break;
            }

        }
    }else if(int_id == 0x02){
        //tx empty
        
    }
}
#elif FR5088_COMM_ENABLE
// code in comm.c
#else
__attribute__((section("ram_code"))) void uart_isr_ram(void)
{
    uint8_t int_id;
    uint8_t c;
    rwip_eif_callback callback;
    void *dummy;
    volatile struct uart_reg_t *uart_reg = (volatile struct uart_reg_t *)UART_BASE;
    struct uart_env_tag *uart1_env = (struct uart_env_tag *)0x200015bc;

    int_id = uart_reg->u3.iir.int_id;
    //uart_putc_noint('#');
    if(int_id == 0x04 || int_id == 0x0c )   /* Receiver data available or Character time-out indication */
    {
        while(uart_reg->lsr & 0x01)
        {
            c = uart_reg->u1.data;
            *uart1_env->rxbufptr++ = c;
            uart1_env->rxsize--;
            if((uart1_env->rxsize == 0)
               &&(uart1_env->rx.callback))
            {
                uart_reg->u3.fcr.data = 0xf1;
                NVIC_DisableIRQ(UART_IRQn);
                uart_reg->u3.fcr.data = 0x21;
                callback = uart1_env->rx.callback;
                dummy = uart1_env->dummy;
                uart1_env->rx.callback = 0;
                uart1_env->rxbufptr = 0;
                callback(dummy, 0);
                break;
            }
        }
    }
    else if(int_id == 0x06)
    {
        volatile uint32_t line_status = uart_reg->lsr;
    }
}
#endif
