#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "user_utils.h"

#include "driver_uart.h"
#include "driver_syscntl.h"
#include "driver_iomux.h"
#include "driver_gpio.h"
#include "driver_ssp.h"
#include "driver_flash_ssp.h"

#define SUPPORT_SSP_UART_SIMULTANEOUSE          0
#define OPTIMIZE_PROGRAM_SPEED                  1
    #define OPTIMIZE_CRC_CALCULATE              1
    #define OPTIMIZE_PROGRAM_SPEED_DEBUG        0

static uint8_t *boot_send_buffer = (uint8_t *)0x40014000;
static uint8_t *boot_recv_buffer = (uint8_t *)0x40014400;
#if SUPPORT_SSP_UART_SIMULTANEOUSE
static uint8_t *boot_recv_back_buffer = (uint8_t *)0x40014800;
static uint32_t last_recv_address = 0;
static uint16_t last_recv_length = 0;
static uint16_t last_data_write_index = 0;
#endif

#if OPTIMIZE_PROGRAM_SPEED
// used to save received data in UART interrupt
static uint8_t *bool_recv_back_buffer = (uint8_t *)0x40012000;
// used to write received data into flash
static uint8_t *bool_write_flash_buffer;
// current used receive buffer: 0 or 1
static uint8_t bool_recv_back_buffer_index = 0;
volatile static uint16_t expect_recv_length;
static uint32_t last_recv_address = 0;
static uint16_t last_recv_length = 0;
static uint16_t last_data_write_index = 0;
static uint32_t xip_code_crc = 0xffffffff;
#endif

enum storage_type_t
{
    STORAGE_TYPE_NONE,
    STORAGE_TYPE_FLASH,
    STORAGE_TYPE_RAM,
};

enum update_param_opcode_t
{
    UP_OPCODE_GET_TYPE,         // 0
    UP_OPCODE_SEND_TYPE,        // 1
    UP_OPCODE_WRITE,            // 2
    UP_OPCODE_WRITE_ACK,        // 3
    UP_OPCODE_WRITE_RAM,        // 4
    UP_OPCODE_WRITE_RAM_ACK,    // 5
    UP_OPCODE_READ_ENABLE,      // 6
    UP_OPCODE_READ_ENABLE_ACK,  // 7
    UP_OPCODE_READ,             // 8
    UP_OPCODE_READ_ACK,         // 9
    UP_OPCODE_READ_RAM,         // a
    UP_OPCODE_READ_RAM_ACK,     // b
    UP_OPCODE_BLOCK_ERASE,      // c
    UP_OPCODE_BLOCK_ERASE_ACK,  // d
    UP_OPCODE_CHIP_ERASE,       // e
    UP_OPCODE_CHIP_ERASE_ACK,   // f
    UP_OPCODE_DISCONNECT,       // 10
    UP_OPCODE_DISCONNECT_ACK,   // 11
    UP_OPCODE_CHANGE_BANDRATE,  // 12
    UP_OPCODE_CHANGE_BANDRATE_ACK,  // 13
    UP_OPCODE_ERROR,            // 14
    UP_OPCODE_EXECUTE_CODE,     //15
    UP_OPCODE_BOOT_RAM,         //16
    UP_OPCODE_EXECUTE_CODE_END, //17
    UP_OPCODE_BOOT_RAM_ACK,     //18
    UP_OPCODE_CALC_CRC32,       //19
    UP_OPCODE_CALC_CRC32_ACK,   //1a
    UP_OPCODE_MAX,
};

enum update_cmd_proc_result_t
{
    UP_RESULT_CONTINUE,
    UP_RESULT_NORMAL_END,
    UP_RESULT_BOOT_FROM_RAM,
    UP_RESULT_RESET,
};

struct update_param_header_t
{
    uint8_t code;
    uint32_t address;
    uint16_t length;
} __attribute__((packed));

#if (SUPPORT_SSP_UART_SIMULTANEOUSE == 1) || (OPTIMIZE_PROGRAM_SPEED == 1)
/*
 * TYPEDEFS (���Ͷ���)
 */
typedef unsigned int    u32;
typedef unsigned short  u16;
typedef unsigned char   u8;

struct ssp_cr0{
    u32 dss:4;  /* data size select : = DATASIZE - 1*/

    u32 frf:2;  /* frame format */
    
    u32 spo:1;  /* sclk polarity */
    u32 sph:1;  /* sclk phase */
    u32 scr:8;  /* serial clock rate */
    u32 unused:16;
};

struct ssp_cr1{
    u32 rie:1;  /* rx fifo interrupt enable */
    u32 tie:1;  /* tx fifo interrupt enable */
    u32 rorie:1;/* rx fifo overrun interrupt enable */

    u32 lbm:1;  /* loop back mode */
    u32 sse:1;  /* synchronous serial port enable*/

    u32 ms:1;   /* master mode or slave mode */
    u32 sod:1;  /* output disable in slave mode */
    
    u32 unused:25;
};

struct ssp_dr{
    u32 data:8; 
    
    u32 unused:24;
};

struct ssp_sr{
    u32 tfe:1;  /* transmit fifo empty */
    u32 tnf:1;  /* transmit fifo not full */
    u32 rne:1;  /* receive fifo not empty */
    u32 rff:1;  /* receive fifo full */
    u32 bsy:1;  /* ssp busy flag */
    
    u32 unused:27;
};

struct ssp_cpsr{
    u32 cpsdvsr:8;  /* clock prescale divisor 2-254 */
    
    u32 unused:24;
};

struct ssp_ff_int_ctrl {
    u32 rx_ff:8;
    u32 tx_ff:8;

    u32 unused:16;
};

struct ssp{
    struct ssp_cr0 ctrl0;
    struct ssp_cr1 ctrl1; /*is also error clear register*/
    struct ssp_dr data;
    struct ssp_sr status;
    struct ssp_cpsr clock_prescale;
    uint32_t reserved;
    struct ssp_ff_int_ctrl ff_ctrl;
};
#endif

const uint8_t dsp_program_boot_conn_req[] = {'F','R','E','Q','C','H','I','P'};//from embedded to pc, request
const uint8_t dsp_program_boot_conn_ack[] = {'F','R','8','0','1','H','O','K'};//from pc to embedded,ack
const uint8_t dsp_program_boot_conn_success[] = {'o','k'};

const uint16_t app_boot_uart_baud_map[12] = {
    12,24,48,96,144,192,384,576,1152,2304,4608,9216
};

extern void dsp_program_load_data(uint8_t *dest, uint32_t src, uint32_t len);
extern void dsp_program_save_data(uint32_t offset, uint32_t length, uint8_t *buffer);
extern void dsp_program_flash_sector_erase(uint32_t addr);

#if OPTIMIZE_PROGRAM_SPEED
static const unsigned int CRC32_Table[256] =
{
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA,
    0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988,
    0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE,
    0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC,
    0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172,
    0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940,
    0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116,
    0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924,
    0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A,
    0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818,
    0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E,
    0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C,
    0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2,
    0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0,
    0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086,
    0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4,
    0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A,
    0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8,
    0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE,
    0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC,
    0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252,
    0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60,
    0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236,
    0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04,
    0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A,
    0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38,
    0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E,
    0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C,
    0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2,
    0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0,
    0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6,
    0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94,
    0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D
};

static unsigned int _getCRC32(void *buffer, unsigned int bufferLen, unsigned int init_value)
{
    unsigned int crc32Value = init_value;

    unsigned char *pTmpBuffer = buffer;

    while(bufferLen--)
    {
        crc32Value = CRC32_Table[(crc32Value^ *pTmpBuffer++) & 0xff] ^ (crc32Value >> 8);
    }
    return (crc32Value);
}
#endif

static int dsp_program_serial_gets(uint8_t ms, uint8_t *data_buf, uint32_t buf_size)
{
    int i, n=0;
    uint32_t recv_size;

    for(i=0; i<ms; i++)
    {
        co_delay_100us(10);
        recv_size = uart_get_data_nodelay_noint(data_buf+n, buf_size);
        n += recv_size;
        buf_size -= recv_size;
        if(0 == buf_size)
        {
            return n;
        }
    }

    return -1;
}

typedef void (*process_callback_func)(uint8_t *, uint16_t);
typedef uint8_t *(*process_get_buffer)(uint32_t);
static enum update_cmd_proc_result_t dsp_program_process_cmd(uint8_t *data, process_get_buffer get_buffer, process_callback_func callback)
{
    uint32_t req_address, req_length, rsp_length;   //req_length does not include header
    struct update_param_header_t *req_header = (struct update_param_header_t *)data;
    struct update_param_header_t *rsp_header;
    enum update_cmd_proc_result_t result = UP_RESULT_CONTINUE;

    req_address = req_header->address;
    req_length = req_header->length;

    rsp_length = sizeof(struct update_param_header_t);
    
    switch(req_header->code)
    {
        case UP_OPCODE_GET_TYPE:
            rsp_header = (struct update_param_header_t *)get_buffer(sizeof(struct update_param_header_t));
            if(rsp_header != NULL)
            {
                rsp_header->code = UP_OPCODE_SEND_TYPE;
                //rsp_header->address = 0x01;
                rsp_header->address = ssp_flash_init();
            }
            break;
        case UP_OPCODE_WRITE:
            rsp_header = (struct update_param_header_t *)get_buffer(sizeof(struct update_param_header_t));
            if(rsp_header != NULL)
            {
                //app_boot_save_data(req_address, req_length, data + sizeof(struct update_param_header_t));
#if SUPPORT_SSP_UART_SIMULTANEOUSE == 1
                memcpy(boot_recv_back_buffer, data + sizeof(struct update_param_header_t), req_length);
                last_recv_length = req_length;
                last_recv_address = req_address;
#elif OPTIMIZE_PROGRAM_SPEED == 1
                last_recv_length = req_length;
                last_recv_address = req_address;
#else
                ssp_flash_write(req_address, req_length, data + sizeof(struct update_param_header_t));
#endif
                rsp_header->code = UP_OPCODE_WRITE_ACK;
                rsp_header->address = req_address;
                rsp_header->length = req_length;
            }
            break;
        case UP_OPCODE_READ:
            rsp_length += req_length;
            rsp_header = (struct update_param_header_t *)get_buffer(rsp_length);
            if(rsp_header != NULL)
            {
                //app_boot_load_data((uint8_t *)rsp_header + sizeof(struct update_param_header_t), req_address, req_length);
                ssp_flash_read(req_address, req_length, (uint8_t *)rsp_header + sizeof(struct update_param_header_t));
                rsp_header->code = UP_OPCODE_READ_ACK;
                rsp_header->address = req_address;
                rsp_header->length = req_length;
            }
            break;
        case UP_OPCODE_WRITE_RAM:
            rsp_header = (struct update_param_header_t *)get_buffer(rsp_length);
            {
                memcpy((uint8_t *)req_address,data+sizeof(struct update_param_header_t),req_length);
                rsp_header->code = UP_OPCODE_WRITE_RAM_ACK;
                rsp_header->address = req_address;
                rsp_header->length = req_length;
            }
            break;
        case UP_OPCODE_READ_ENABLE:
            rsp_header = (struct update_param_header_t *)get_buffer(rsp_length);
            {
                rsp_header->code = UP_OPCODE_READ_ENABLE_ACK;
                rsp_header->address = req_address;
                rsp_header->length = req_length;
            }
            break;
        case UP_OPCODE_READ_RAM:
            rsp_header = (struct update_param_header_t *)get_buffer(sizeof(struct update_param_header_t)+req_length);
            if(rsp_header != NULL)
            {
                memcpy((uint8_t *)rsp_header + sizeof(struct update_param_header_t), (uint8_t *)req_address, req_length);
                rsp_header->code = UP_OPCODE_READ_RAM_ACK;
                rsp_header->address = req_address;
                rsp_header->length = req_length;
                rsp_length += req_length;
            }
            break;
            
        case UP_OPCODE_BLOCK_ERASE:
            rsp_header = (struct update_param_header_t *)get_buffer(sizeof(struct update_param_header_t));
            if(rsp_header != NULL)
            {
                //app_boot_flash_sector_erase(req_address);
                ssp_flash_erase(req_address, 0x1000);
                rsp_header->code = UP_OPCODE_BLOCK_ERASE_ACK;
                rsp_header->address = req_address;
                rsp_header->length = req_length;
            }
            break;
        case UP_OPCODE_CHIP_ERASE:
            rsp_header = (struct update_param_header_t *)get_buffer(sizeof(struct update_param_header_t));
            if(rsp_header != NULL)
            {
                ssp_flash_chip_erase();
                rsp_header->code = UP_OPCODE_CHIP_ERASE_ACK;
                rsp_header->address = req_address;
                rsp_header->length = req_length;
            }
        break;
        case UP_OPCODE_DISCONNECT:
            rsp_header = (struct update_param_header_t *)get_buffer(sizeof(struct update_param_header_t));
            if(rsp_header != NULL)
            {
                rsp_header->code = UP_OPCODE_DISCONNECT_ACK;
            }
            result = (enum update_cmd_proc_result_t) (req_address & 0xFF);
            break;
        case UP_OPCODE_CHANGE_BANDRATE:
            rsp_header = (struct update_param_header_t *)get_buffer(sizeof(struct update_param_header_t));
            if(rsp_header != NULL)
            {
                rsp_header->code = UP_OPCODE_CHANGE_BANDRATE_ACK;
            }
            callback((uint8_t *)rsp_header, rsp_length);
            uart_init(req_address & 0xFF);
#if OPTIMIZE_PROGRAM_SPEED == 1
            /* set up uart interrupt threshold */
            volatile struct uart_reg_t * const uart_reg = (volatile struct uart_reg_t *)UART_BASE;
            uart_reg->u3.fcr.data = FCR_RX_TRIGGER_10 | FCR_TX_TRIGGER_10 | FCR_FIFO_ENABLE;
#endif
            break;
        case UP_OPCODE_EXECUTE_CODE:
#if OPTIMIZE_PROGRAM_SPEED && OPTIMIZE_CRC_CALCULATE
            xip_code_crc = xip_code_crc ^ 0xffffffff;
            *(volatile uint32_t *)0x2000f004 = xip_code_crc;
            uart_write((void *)"CRC",3);
#else
            (*(void(*)(void))req_address)();
#endif
            rsp_header = (struct update_param_header_t *)get_buffer(sizeof(struct update_param_header_t));
            if(rsp_header != NULL)
            {
                rsp_header->code = UP_OPCODE_EXECUTE_CODE_END;
#if OPTIMIZE_PROGRAM_SPEED && OPTIMIZE_CRC_CALCULATE
                rsp_header->address = xip_code_crc;
#endif
            }
            break;
        default:
            break;
    }

    if(req_header->code != UP_OPCODE_CHANGE_BANDRATE)
    {
        callback((uint8_t *)rsp_header, rsp_length);
    }

    return result;

}

static uint8_t *dsp_program_get_buffer(uint32_t length)
{
    return (uint8_t *)&boot_send_buffer[0];
}

static void dsp_program_send_rsp(uint8_t *buffer, uint16_t length)
{
    uart_write(buffer, length);
}

#if (SUPPORT_SSP_UART_SIMULTANEOUSE == 1) || (OPTIMIZE_PROGRAM_SPEED == 1)
static uint8_t ssp_flash_read_status_reg(void)
{
    uint8_t buffer[2] = {0x00, 0x00};

    ssp_put_data(FLASH_READ_STATUS_REG_OPCODE);
    ssp_put_data(0xff);
    ssp_enable();
    ssp_wait_busy_bit();
    ssp_get_data(&buffer[0]);
    ssp_get_data(&buffer[1]);
    ssp_disable();
    return buffer[1];
}

static void flash_poll_busy_bit(void)
{
    volatile uint16_t i;

    while(ssp_flash_read_status_reg()&0x03)
    {
        //delay
        for(i=0; i<1000; i++);
    }
}

void ssp_flash_write_enable(void)
{
    uint8_t dummy;
    ssp_put_data(FLASH_WRITE_ENABLE_OPCODE);
    ssp_enable();
    ssp_wait_busy_bit();
    ssp_disable();
    ssp_get_data(&dummy);
}

static void ssp_flash_read_sub(uint32_t offset, uint32_t length, uint8_t *buffer)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP_BASE;
    uint32_t j;
    
    ssp->data.data = 0x03;
    ssp->data.data = offset >> 16;
    ssp->data.data = offset >> 8;
    ssp->data.data = offset ;
    for(j=0; j<length; j++) {
        ssp->data.data = 0xff;
    }
    gpio_set_dir(GPIO_PORT_A, GPIO_BIT_1, GPIO_DIR_OUT);
    gpio_porta_write(gpio_porta_read() & 0xfd);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_A1);
    ssp_enable();
    while(ssp->status.bsy);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_SSP_CSN);
    ssp_disable();
    
    for(j=0; j<4; j++) {
        volatile uint8_t dummy = ssp->data.data;
    }
    for(j=0; j<length; j++) {
        *buffer++ = ssp->data.data;
    }
}

static uint8_t ssp_flash_read_(uint32_t offset, uint32_t length, uint8_t *buffer)
{
#define FLASH_READ_SINGLE_PACKET_LEN    (128-4)
    uint32_t read_times;
    uint8_t last_bytes;
    uint32_t i;

    read_times = length/FLASH_READ_SINGLE_PACKET_LEN;
    last_bytes = length%FLASH_READ_SINGLE_PACKET_LEN;

    for(i=0; i<read_times; i++)
    {
        ssp_flash_read_sub(offset, FLASH_READ_SINGLE_PACKET_LEN, buffer);
        offset += FLASH_READ_SINGLE_PACKET_LEN;
        buffer += FLASH_READ_SINGLE_PACKET_LEN;
    }
    if(last_bytes != 0)
    {
        ssp_flash_read_sub(offset, last_bytes, buffer);
    }

    return 0;
}

/* write data to flash with page boundary */
void ssp_flash_write_page(uint32_t dest, uint32_t length, uint8_t *ptr)
{
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP_BASE;
    /* write enable */
    ssp_flash_write_enable();

    /* execute write */
    gpio_set_dir(GPIO_PORT_A, GPIO_BIT_1, GPIO_DIR_OUT);
    gpio_porta_write(gpio_porta_read() & 0xfd);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_A1);
    ssp_put_data(FLASH_PAGE_PROGRAM_OPCODE);
    ssp_put_data(dest >> 16);
    ssp_put_data(dest >> 8);
    ssp_put_data(dest);
    ssp_enable();
    for(uint32_t i=0; i<length; i++) {
        while(ssp->status.tnf == 0);
        ssp->data.data = *ptr++;
    }
    while(ssp->status.bsy);
    system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_SSP_CSN);
    ssp_disable();
    
    ssp_clear_rx_fifo();
    
    /* poll status */
    uint8_t status;
    do {
        co_delay_100us(1);
        status = ssp_flash_read_status_reg();
    } while(status&0x03);
    
#if OPTIMIZE_PROGRAM_SPEED && OPTIMIZE_CRC_CALCULATE
    ssp_flash_read_(dest, length, (void *)0x40011000);
    xip_code_crc = _getCRC32((void *)0x40011000, length, xip_code_crc);
#endif
}
#endif

static void dsp_program_host_comm_loop(void)
{
    enum update_cmd_proc_result_t result = UP_RESULT_CONTINUE;
    struct update_param_header_t *req_header = (struct update_param_header_t *)&boot_recv_buffer[0]; //this address is useless after cpu running into this function
#if SUPPORT_SSP_UART_SIMULTANEOUSE == 1
    uint8_t *uart_recv_ptr = (void *)(((uint8_t *)req_header)+sizeof(struct update_param_header_t));
    uint16_t recv_length, expect_length;
    uint8_t *flash_write_ptr = (void *)boot_recv_back_buffer;
    volatile struct ssp * const ssp = (volatile struct ssp *)SSP_BASE;
#endif
#if OPTIMIZE_PROGRAM_SPEED == 1
    
#endif

    while(result == UP_RESULT_CONTINUE)
    {
#if SUPPORT_SSP_UART_SIMULTANEOUSE == 1
        if(last_recv_length) {
            uart_get_data_noint((uint8_t *)req_header, sizeof(struct update_param_header_t));
            
            expect_length = req_header->length;
            if((req_header->length != 0)
               &&(req_header->code != UP_OPCODE_READ)
               &&(req_header->code != UP_OPCODE_READ_RAM))
            {
                uart_recv_ptr = (void *)(((uint8_t *)req_header)+sizeof(struct update_param_header_t));
                flash_write_ptr = (void *)boot_recv_back_buffer;
                
                ssp_flash_write_enable();

                gpio_set_dir(GPIO_PORT_A, GPIO_BIT_1, GPIO_DIR_OUT);
                gpio_porta_write(gpio_porta_read() & 0xfd);
                system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_A1);
                
                ssp_put_data(FLASH_PAGE_PROGRAM_OPCODE);
                ssp_put_data(last_recv_address >> 16);
                ssp_put_data(last_recv_address >> 8);
                ssp_put_data(last_recv_address);
                ssp_enable();
                
                while(expect_length) {
                    recv_length = uart_get_data_nodelay_noint(uart_recv_ptr, 16);
                    if(expect_length < recv_length) {
                        // not expected to be here
                        while(1);
                    }
                    else {
                        expect_length -= recv_length;
                        uart_recv_ptr += recv_length;
                        if(expect_length == 0) {
                            while(last_recv_length) {
                                while(ssp->status.tnf == 0);
                                ssp->data.data = *flash_write_ptr++;
                                last_recv_length--;
                            }
                            while(ssp->status.bsy);
                            system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_SSP_CSN);
                            ssp_disable();
                        }
                    }
                    if(last_recv_length) {
                        if(ssp->status.tfe == 1) {
                            uint8_t flash_single_write_length;
                            if(last_recv_length > 16) {
                                flash_single_write_length = 16;
                            }
                            else {
                                flash_single_write_length = last_recv_length;
                            }
                            for(uint8_t i=0; i<flash_single_write_length; i++) {
                                ssp->data.data = *flash_write_ptr++;
                            }
                            last_recv_length -= flash_single_write_length;
                        }
                    }
                    else {
                        if(ssp->status.bsy) {
                        }
                        else {
                            system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_SSP_CSN);
                            ssp_disable();
                        }
                    }
                }

                ssp_clear_rx_fifo();
                flash_poll_busy_bit();
            }

            if(last_recv_length) {
                ssp_flash_write(last_recv_address, last_recv_length, boot_recv_back_buffer);
                last_recv_length = 0;
            }
        }
        else {
#endif
#if OPTIMIZE_PROGRAM_SPEED == 1
        if(last_recv_length) {
            uart_get_data_noint((uint8_t *)req_header, sizeof(struct update_param_header_t));
            
            expect_recv_length = req_header->length;
            if((req_header->length != 0)
               &&(req_header->code != UP_OPCODE_READ)
               &&(req_header->code != UP_OPCODE_READ_RAM))
            {
                if(bool_recv_back_buffer_index == 0) {
                    bool_recv_back_buffer = (uint8_t *)0x40012000+4*1024;
                    bool_recv_back_buffer_index = 1;
                }
                else {
                    bool_recv_back_buffer = (uint8_t *)0x40012000;
                    bool_recv_back_buffer_index = 0;
                }
                if(req_header->code == UP_OPCODE_WRITE) {
                    if(expect_recv_length >= 16) {
                        /* enable UART interrupt to receive data */
                        NVIC_EnableIRQ(UART_IRQn);
                    }
                    else {
                        uart_get_data_noint(bool_recv_back_buffer, expect_recv_length);
                        expect_recv_length = 0;
                    }
                }
                else {
                uart_get_data_noint(((uint8_t *)req_header)+sizeof(struct update_param_header_t), req_header->length);
            }
                
                if(bool_recv_back_buffer_index == 1) {
                    bool_write_flash_buffer = (uint8_t *)0x40012000;
                }
                else {
                    bool_write_flash_buffer = (uint8_t *)0x40012000+4*1024;
                }
                /* start program data to flash */
                while(last_recv_length > 256) {
                    ssp_flash_write_page(last_recv_address, 256, bool_write_flash_buffer);
                    last_recv_address += 256;
                    bool_write_flash_buffer += 256;
                    last_recv_length -= 256;
                }
                if(last_recv_length) {
                    ssp_flash_write_page(last_recv_address, last_recv_length, bool_write_flash_buffer);
                    last_recv_address += last_recv_length;
                    bool_write_flash_buffer += last_recv_length;
                    last_recv_length -= last_recv_length;
                }
                
                if(req_header->code == UP_OPCODE_WRITE) {
                    /* wait for all uart data is received */
                    while(expect_recv_length > 0);
                    NVIC_DisableIRQ(UART_IRQn);
                }
            }
        }
        else {
#endif
            uart_get_data_noint((uint8_t *)req_header, sizeof(struct update_param_header_t));
            if((req_header->length != 0)
               &&(req_header->code != UP_OPCODE_READ)
               &&(req_header->code != UP_OPCODE_READ_RAM))
            {
#if OPTIMIZE_PROGRAM_SPEED == 1
                if(req_header->code != UP_OPCODE_WRITE) {
                uart_get_data_noint(((uint8_t *)req_header)+sizeof(struct update_param_header_t), req_header->length);
            }
                else {
                    uart_get_data_noint(bool_recv_back_buffer, req_header->length);
                }
#else
                uart_get_data_noint(((uint8_t *)req_header)+sizeof(struct update_param_header_t), req_header->length);
#endif
            }
#if SUPPORT_SSP_UART_SIMULTANEOUSE == 1
        }
#endif
#if OPTIMIZE_PROGRAM_SPEED == 1
        }
#endif
        result = dsp_program_process_cmd((uint8_t *)req_header, dsp_program_get_buffer, dsp_program_send_rsp);
    }
    if(result == UP_RESULT_RESET){
        co_delay_100us(30000);//delay 1s
        platform_reset(0);
    }
    while(1);
}


#if OPTIMIZE_PROGRAM_SPEED == 1
__attribute__((section("ram_code"))) static void uart_isr_dsp_program(void)
{
    uint8_t int_id;
    volatile struct uart_reg_t * const uart_reg = (volatile struct uart_reg_t *)UART_BASE;

    int_id = uart_reg->u3.iir.int_id;

#if OPTIMIZE_PROGRAM_SPEED_DEBUG
    REG_PL_WR(GPIO_PORTB_DATA, REG_PL_RD(GPIO_PORTB_DATA) | 0x10);
#endif
    if(int_id == 0x04 || int_id == 0x0c ) { /* Receiver data available or Character time-out indication */        
        for(uint8_t i=0; i<16; i++) {
						while((uart_reg->lsr & 0x01) == 0){};
            *bool_recv_back_buffer++ = uart_reg->u1.data;
        }
        expect_recv_length -= 16;
        if(expect_recv_length < 16) {
            while(expect_recv_length > 0) {
                while((uart_reg->lsr & 0x01) == 0);
                *bool_recv_back_buffer++ = uart_reg->u1.data;
                expect_recv_length--;
            }
        }
    }
    else if(int_id == 0x06) {
        volatile uint32_t line_status = uart_reg->lsr;
    }
#if OPTIMIZE_PROGRAM_SPEED_DEBUG
    REG_PL_WR(GPIO_PORTB_DATA, REG_PL_RD(GPIO_PORTB_DATA) & 0xef);
#endif
}
#endif

void dsp_program(void)
{
    uint8_t buffer[sizeof(dsp_program_boot_conn_ack)];
    uint8_t do_handshake = 1;
    uint8_t retry_count = 1;
#if OPTIMIZE_PROGRAM_SPEED == 1
    uint32_t uart_isr_tmp;
    volatile struct uart_reg_t * const uart_reg = (volatile struct uart_reg_t *)UART_BASE;
#endif

    while(retry_count) {
        uart_write((uint8_t *)dsp_program_boot_conn_req, sizeof(dsp_program_boot_conn_req));
        if(dsp_program_serial_gets(100, buffer, sizeof(dsp_program_boot_conn_ack))==sizeof(dsp_program_boot_conn_ack)) {
            if(memcmp(buffer, dsp_program_boot_conn_ack, sizeof(dsp_program_boot_conn_ack)) != 0) {
                do_handshake = 0;
            }
            else {
                break;
            }
        }
        else {
            do_handshake = 0;
        }

        retry_count--;
    }

    if(do_handshake) {
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PORTA0_FUNC_SSP_SCLK);
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_SSP_CSN);
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_SSP_MOSI);
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_SSP_MISO);
        #if 0
        system_set_port_mux(GPIO_PORT_D, GPIO_BIT_6, PORTD6_FUNC_D6);
        system_set_port_mux(GPIO_PORT_D, GPIO_BIT_7, PORTD7_FUNC_D7);
        gpio_set_dir(GPIO_PORT_D, GPIO_BIT_6, GPIO_DIR_OUT);
        gpio_set_dir(GPIO_PORT_D, GPIO_BIT_7, GPIO_DIR_OUT);
        gpio_set_pin_value(GPIO_PORT_D, GPIO_BIT_6, 1);
        gpio_set_pin_value(GPIO_PORT_D, GPIO_BIT_7, 1);
        #else
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_4, PORTA4_FUNC_A4);
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_5, PORTA5_FUNC_A5);
        gpio_set_dir(GPIO_PORT_A, GPIO_BIT_4, GPIO_DIR_OUT);
        gpio_set_dir(GPIO_PORT_A, GPIO_BIT_5, GPIO_DIR_OUT);
        gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_4, 1);
        gpio_set_pin_value(GPIO_PORT_A, GPIO_BIT_5, 1);
        #endif
        ssp_init(8, 0, 0, 1000000, 2, 0);
        
#if OPTIMIZE_PROGRAM_SPEED == 1
        /* set up uart interrupt threshold */
        uart_reg->u3.fcr.data = FCR_RX_TRIGGER_10 | FCR_TX_TRIGGER_10 | FCR_FIFO_ENABLE;
        /* set uart interrupt handle */
        uart_isr_tmp = *(volatile uint32_t *)(0x20000000+(16+12)*4);
        *(volatile uint32_t *)(0x20000000+(16+12)*4) = (uint32_t)uart_isr_dsp_program;
        GLOBAL_INT_START();
#if OPTIMIZE_PROGRAM_SPEED_DEBUG == 1
        system_set_port_mux(GPIO_PORT_B, GPIO_BIT_4, PORTB4_FUNC_B4);
        gpio_set_dir(GPIO_PORT_B, GPIO_BIT_4, GPIO_DIR_OUT);
        gpio_set_pin_value(GPIO_PORT_B, GPIO_BIT_4, 0);
#endif
#endif
        uart_write((uint8_t *)dsp_program_boot_conn_success, sizeof(dsp_program_boot_conn_success));
        //external flash write protect
        ssp_flash_protect_disable();
        dsp_program_host_comm_loop();
        ssp_flash_protect_enable();
        memset((void *)0x40010000, 0, 24*1024);
        
#if OPTIMIZE_PROGRAM_SPEED == 1
#if OPTIMIZE_PROGRAM_SPEED_DEBUG == 1
        gpio_set_dir(GPIO_PORT_B, GPIO_BIT_4, GPIO_DIR_IN);
#endif
        /* reset uart interrupt handle */
        *(volatile uint32_t *)(0x20000000+(16+12)*4) = uart_isr_tmp;
        GLOBAL_INT_STOP();
#endif
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_0, PORTA0_FUNC_A0);
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_1, PORTA1_FUNC_A1);
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_2, PORTA2_FUNC_A2);
        system_set_port_mux(GPIO_PORT_A, GPIO_BIT_3, PORTA3_FUNC_A3);
        //gpio_set_dir(GPIO_PORT_D, GPIO_BIT_6, GPIO_DIR_IN);
        //gpio_set_dir(GPIO_PORT_D, GPIO_BIT_7, GPIO_DIR_IN);
        gpio_set_dir(GPIO_PORT_A, GPIO_BIT_4, GPIO_DIR_IN);
        gpio_set_dir(GPIO_PORT_A, GPIO_BIT_5, GPIO_DIR_IN);

    }
}

