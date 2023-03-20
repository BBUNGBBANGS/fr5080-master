#ifndef _USER_FS_H
#define _USER_FS_H

#include <stdint.h>

#define MCU_REQ_MP3_CUR     0x00
#define MCU_REQ_MP3_NEXT    0x01
#define MCU_REQ_MP3_PREV    0x02


/*********************************************************************
 * @fn      fs_init
 *
 * @brief   called at the beginning of program, used to open music directory
 *          and the first MP3 file.
 *
 * @param   None
 *
 * @return  0: success, other: failed
 */
int fs_init(void);

/*********************************************************************
 * @fn      fs_read
 *
 * @brief   read data in current opened MP3 file, new file will be opened if
 *          reach the end of current file.
 *
 * @param   buffer  - buffer used to store read data
 *          length  - how many bytes requested
 *
 * @return  size of read data in bytes
 */
uint32_t fs_read(uint8_t *buffer, uint32_t length);

/**
 * @brief open next music file
 * 
 */
void fs_prepare_next(void);

/**
 * @brief open prev music file
 * 
 */
void fs_prepare_prev(void);


/**
 * @brief open prev specified file
 *
 * @param   mp3_item    mp3 item address
 *
 * @return  NULL
 * 
 */
void fs_prepare_specified(void *mp3_item);

/**
 * @brief handle getting detailed mp3 information requset,uart send related 
 * mp3 detailed info to MCU
 *
 * @param   mp3_item    mp3 item address, if NULL, item = mp3_env.cur_item
 *          direction   0---current mp3, 1---next mp3, 2---prev mp3
 *
 * @return  true --- get mp3 info success, false --- get mp3 info fail
 * 
 */
bool fs_handle_mp3_info_req(void *item, uint8_t direction);

/**
 * @brief handle getting only mp3 file name requset,uart send name and mp3  
 *  item address to MCU
 *
 * @param   mp3_item    mp3 item address, if NULL, item = mp3_env.cur_item
 *          direction   0---current mp3, 1---next mp3, 2---prev mp3
 *
 * @return  true --- get mp3 info success, false --- get mp3 info fail
 * 
 */
bool fs_handle_mp3_list_req(void *item, uint8_t direction);

/**
 * @brief get mp3 total nummeber in sd card
 * 
 */
uint8_t fs_get_mp3_num(void);


/**
 * @brief check if sd card has mp3 file
 *
 * @param   NULL
 * @return  true---has mp3  false --- has no mp3
 */
bool is_fs_has_mp3_item(void);

/**
 * @brief send the current mp3 detail info use uart
 *
 */
void fs_uart_send_mp3_info(void);

#endif  // _USER_FS_H

