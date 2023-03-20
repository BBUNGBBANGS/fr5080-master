#ifndef _MP3_TAG_DECODER_H
#include <stdint.h>
#include <stdbool.h>

#define _MP3_TAG_DECODER_H

#define MP3_TAG_HD_SIZE  10
#define MAX_TAG_LEN             255


/* 媒体文件ID3V2标签头结构体 */
struct mp3_id3v2_header_t
{
    char header[3];             /* 必须为"ID3"否则认为标签不存在,3字节 */
    char version[1];            /* 版本号ID3V2.3 就记录3，1字节 */
    char revision[1];           /* 副版本号此版本记录为0，1字节 */
    char flag[1];               /* 存放标志的字节，这个版本只定义了三位，1字节 */
    char size[4];               /* 标签大小，包括标签头的10个字节和所有的标签帧的大小，4字节 */
};
 
 /* 媒体文件ID3V2H标签帧帧头结构体 */
struct mp3_id3v2_frame_header_t
{
    char frameID[4];            /* 用四个字符标识一个帧，说明其内容,4字节 */
    char size[4];               /* 帧内容的大小，不包括帧头，不得小于1，4字节 */
    char flags[2];              /* 存放标志，只定义了6 位，2字节 */
};


struct mp3_frame_header_t
{
unsigned int sync1:8; //同步信息 1
unsigned int error_protection:1; //CRC 校验
unsigned int layer:2; //层
unsigned int version:2; //版本
unsigned int sync2:3; //同步信息 2
unsigned int extension:1; //版权
unsigned int padding:1; //填充空白字
unsigned int sample_rate_index:2; //采样率索引
unsigned int bit_rate_index:4; //位率索引
unsigned int emphasis:2; //强调方式
unsigned int original:1; //原始媒体
unsigned int copyright:1; //版权标志
unsigned int mode_extension:2; //扩展模式,仅用于联合立体声
unsigned int channel_mode:2; //声道模式
};


bool mp3_get_tag_info(void* fpp,uint8_t *tag_buf,uint32_t *size);

uint32_t mp3_get_tag_size(void* fpp);

#endif
