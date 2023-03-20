#ifndef _DRIVER_IPC_H
#define _DRIVER_IPC_H

/*
 * INCLUDES
 */
#include <stdint.h>

/*
 * TYPEDEFS
 */

enum ipc_msg_type_t {

    IPC_MSG_LOAD_CODE,
    IPC_MSG_LOAD_CODE_DONE,
    IPC_MSG_EXEC_USER_CODE,
    IPC_MSG_DSP_READY = 10,

};

enum ipc_dir_t {
    IPC_DIR_MCU2DSP,
    IPC_DIR_DSP2MCU,
};

enum ipc_int_t {
    IPC_MSGIN00_INT = (1<<0),
    IPC_MSGIN01_INT = (1<<1),
    IPC_MSGIN10_INT = (1<<2),
    IPC_MSGIN11_INT = (1<<3),
    IPC_MSGOUT00_INT = (1<<4),
    IPC_MSGOUT01_INT = (1<<5),
    IPC_MSGOUT10_INT = (1<<6),
    IPC_MSGOUT11_INT = (1<<7),
    IPC_MSG_INT_ALL = 0xff,
};

enum ipc_int_status_t {
    IPC_MSGIN00_STATUS = (1<<8),
    IPC_MSGIN01_STATUS = (1<<9),
    IPC_MSGIN10_STATUS = (1<<10),
    IPC_MSGIN11_STATUS = (1<<11),
    IPC_MSGOUT00_STATUS = (1<<12),
    IPC_MSGOUT01_STATUS = (1<<13),
    IPC_MSGOUT10_STATUS = (1<<14),
    IPC_MSGOUT11_STATUS = (1<<15),
};

struct ipc_msg_t {
    uint32_t length:10;
    uint32_t format:4;
    uint32_t tog:1;
    uint32_t tag:1;
    uint32_t reserved:16;
};

enum ipc_mic_type_t{
    IPC_MIC_TYPE_ANLOG_MIC,     //local ana mic
    IPC_MIC_TYPE_PDM,           //local pdm
    IPC_MIC_TYPE_I2S,           //external I2S input
};

enum ipc_spk_type_t{
    IPC_SPK_TYPE_CODEC,         //local spk
    IPC_SPK_TYPE_I2S,           //external I2S output
};

enum ipc_media_type_t{
    IPC_MEDIA_TYPE_BT,          //audio from remote BT devices
    IPC_MEDIA_TYPE_LOCAL,       //audio from local flash or received from other MCU
};


typedef void (*ipc_tx_callback)(uint8_t chn);
typedef void (*ipc_rx_callback)(struct ipc_msg_t *msg, uint8_t chn);

/*********************************************************************
 * @fn      ipc_init
 *
 * @brief   used to init ipc controller.
 *
 * @param   ints_en     - which interrupt source should be enable.
 *          callback    - callback function for rx-message
 *
 * @return  None.
 */
void ipc_init(uint32_t ints_en, ipc_rx_callback callback);

/*********************************************************************
 * @fn      ipc_msg_send
 *
 * @brief   send message without payload.
 *
 * @param   msg         - message type.
 *          sub_msg     - this field will be set in ipc_msg_length segment.
 *          callback    - callback function for tx done
 *
 * @return  None.
 */
void ipc_msg_send(enum ipc_msg_type_t msg, uint16_t sub_msg, ipc_tx_callback callback);

/*********************************************************************
 * @fn      ipc_msg_with_payload_send
 *
 * @brief   send message with payload. total length of header and payload should
 *          not be larger than 1023.
 *
 * @param   msg         - message type.
 *          header      - message header, these data will be copy into the beginning of
 *                        exchange memory.
 *          header_length - header length
 *          payload     - message payload, these data will be copy following header
 *          payload_length - payload length
 *          callback    - callback function for tx done
 *
 * @return  None.
 */
void ipc_msg_with_payload_send(enum ipc_msg_type_t msg,
                                            void *header,
                                            uint16_t header_length,
                                            uint8_t *payload,
                                            uint16_t payload_length,
                                            ipc_tx_callback callback);

/*********************************************************************
 * @fn      ipc_msg_flush
 *
 * @brief   flush ipc msg list,when power off,shall call this function first.
 *
 * @param   None.
 *
 * @return  None.
 */
void ipc_msg_flush(void);

/*********************************************************************
 * @fn      ipc_clear_sending_msg
 *
 * @brief   remove msg has not been accpeted by DSP.
 *
 * @param   chn     - the number of channel which will be cleared..
 *
 * @return  None.
 */
void ipc_clear_sending_msg(uint8_t chn);

/*********************************************************************
 * @fn      ipc_alloc_channel
 *
 * @brief   allocate a channel for following transmit.
 *
 * @param   length      - the length of message. If this value is 0,
 *                      - channel 2 and 3 are also available.
 *
 * @return  allocated channel number.
 */
uint8_t ipc_alloc_channel(uint16_t length);

/*********************************************************************
 * @fn      ipc_alloc_channel
 *
 * @brief   free an allocated channel after finish transmit.
 *
 * @param   chn     - the number of channel which will be free.
 *			
 * @return  None.
 */
void ipc_free_channel(uint8_t chn);

/*********************************************************************
 * @fn      ipc_insert_msg
 *
 * @brief   start a message or command transmit.
 *
 * @param   chn      - the number of channel will be used.
 *          format   - contains message type or command
 *          length   - contains length of message
 *          callback - This function will be called after this message
 *                     or command is transmitted.
 *			
 * @return  None.
 */
void ipc_insert_msg(uint8_t chn, uint8_t format, uint16_t length, ipc_tx_callback callback);

/*********************************************************************
 * @fn      ipc_clear_msg
 *
 * @brief   used to notify the other core a specified message has been 
 *          processed
 *
 * @param   chn     - the index of message.
 *			
 * @return  None.
 */
void ipc_clear_msg(uint8_t chn);

/*********************************************************************
 * @fn      ipc_get_buffer_offset
 *
 * @brief   get buffer base address of share memory according transmit direction
 *          and ipc channel index.
 *
 * @param   type    - the direction of tramsmit, reference @ipc_dir_t.
 *          index   - the index of message.
 *			
 * @return  None.
 */
uint8_t *ipc_get_buffer_offset(enum ipc_dir_t type, uint8_t index);

/*********************************************************************
 * @fn      ipc_set_audio_inout_type
 *
 * @brief   set audio input&output type.
 *
 * @param   mic_type   - mic input type, reference @ipc_mic_type_t.
 *			spk_type   - spk output type, reference @ipc_spk_type_t.
 *          media_type   - media source type, reference @ipc_media_type_t.
 *
 * @return  None.
 */
void ipc_set_audio_inout_type(enum ipc_mic_type_t mic_type,enum ipc_spk_type_t spk_type,enum ipc_media_type_t media_type);

/*********************************************************************
 * @fn      ipc_get_mic_type
 *
 * @brief   get current mic type.
 *
 * @param   None
 *
 * @return  reference @ipc_mic_type_t.
 */
enum ipc_mic_type_t ipc_get_mic_type(void);

/*********************************************************************
 * @fn      ipc_get_spk_type
 *
 * @brief   get current spk type.
 *
 * @param   None
 *
 * @return  reference @ipc_spk_ype_t.
 */
enum ipc_spk_type_t ipc_get_spk_type(void);

/*********************************************************************
 * @fn      ipc_get_media_type
 *
 * @brief   get current media source type.
 *
 * @param   None.
 *
 * @return  reference @ipx_media_type_t.
 */
enum ipc_media_type_t ipc_get_media_type(void);

/*********************************************************************
 * @fn      ipc_config_voice_dma
 *
 * @brief   When SCO is created, call this function to create IPC DMA channel for
 *          voice. 4 type voice data will transfer through IPC DMA in this mode:
 *          1. sco_data in (data from baseband to DSP)
 *          2. sco_data out (data from DSP to baseband)
 *          3. mic in (data from codec to DSP)
 *          4. spk out (data from DSP to codec)
 *
 * @param   None.
 *
 * @return  None.
 */
void ipc_config_voice_dma(void);

/*********************************************************************
 * @fn      ipc_config_media_dma
 *
 * @brief   When playing music (decoded in DSP) in local speaker, call this function
 *          to create IPC DMA channel for media PCM transfer. 1 type media data will 
 *          transfer through IPC DMA in this mode:
 *          1. PCM data out (data from DSP to codec)
 *
 * @param   stereo  - 1: stereo, 0: mono.
 *
 * @return  None.
 */
void ipc_config_media_dma(uint8_t stereo);

/*********************************************************************
 * @fn      ipc_config_mic_only_dma
 *
 * @brief   Used for voice recognition, etc. 1 type media data will transfer through 
 *          IPC DMA in this mode:
 *          1. mic in or pdm in (data from codec to DSP) 
 *
 * @param   None.
 *
 * @return  None.
 */
void ipc_config_mic_only_dma(void);

/*********************************************************************
 * @fn      ipc_config_mic_loop_dma
 *
 * @brief   Used for voice recognition, local speaker out. 2 type voice data will transfer through 
 *          IPC DMA in this mode:
 *          1. mic in or pdm in (data from codec to DSP) 
 *          2. spk out (data from DSP to codec)
 *
 * @param   None.
 *
 * @return  None.
 */
void ipc_config_mic_loop_dma(void);


/*********************************************************************
 * @fn      ipc_config_media_dma_disable
 *
 * @brief   Used to disconnect the routing between ipc dma and codec or i2s.
 *
 * @param   None.
 *
 * @return  None.
 */
void ipc_config_media_dma_disable(void);

void ipc_config_reset_dma(void);

/*********************************************************************
 * @fn      ipc_set_audio_inout_type
 *
 * @brief   used to initialize variables which indicate audio route, different selections
 *          mean different configurations will be used in corresponsing working mode.
 *
 * @param   mic_type    - PDM, analog MIC or I2S.
 *          spk_type    - codec or I2S.
 *          media_type  - reserved for future usage.
 *
 * @return  None.
 */
void ipc_set_audio_inout_type(enum ipc_mic_type_t mic_type,enum ipc_spk_type_t spk_type,enum ipc_media_type_t media_type);

#endif  // _DRIVER_IPC_H

