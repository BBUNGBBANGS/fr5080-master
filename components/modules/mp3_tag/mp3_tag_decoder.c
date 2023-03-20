#include "stdio.h"
#include "string.h"
#include "ff.h"
#include "mp3_tag_decoder.h"
#include "co_log.h"



uint16_t MPEG1[5] = {1152,1152,1152,384,1152};
uint16_t MPEG2[5] = {1152,576,1152,384,1152};

uint16_t MPEG1_rate[4] = {44100,48000,32000,8000};
uint16_t MPEG2_rate[4] = {22050,24000,16000,8000};
uint16_t MPEG25_rate[4] = {11025,12000,8000,8000};

extern void fr_mul_64(uint32_t *low, uint32_t *high, uint32_t mul1, uint32_t mul2);
extern uint32_t fr_div_64(uint32_t low, uint32_t high, uint32_t div);

static uint32_t ld_dword (uint8_t* ptr)	/* Load a 4-byte big-endian word */
{
	uint32_t rv;

	rv = ptr[0];
	rv = rv << 8 | ptr[1];
	rv = rv << 8 | ptr[2];
	rv = rv << 8 | ptr[3];
	return rv;
}

/*buffer order
*  buf length --            2bytes
*  mp3_total_size --        4bytes
*  mp3 total duration       4bytes
*  mp3 title/artist/album
*/
bool mp3_get_tag_info(void* fpp,uint8_t *tag_buf,uint32_t *size)
{
    uint32_t tag_size = 0;
    uint32_t read_bytes = 0;
    uint32_t fbody_size = 0;
    uint8_t counter = 0;
    uint16_t save_index = 10;
//    int8_t *dummy_buf;
    uint8_t temp_buf[4];
    uint32_t frame_cnt = 0;
    uint32_t mp3_total_len = 0;
    uint32_t duration = 0;
    uint32_t sample_cnt = 0;
    uint32_t sample_rate = 0;
    FIL *fp = (FIL*)fpp;
    
    struct mp3_id3v2_header_t id3v2_hd;
    struct mp3_id3v2_frame_header_t id3v2_frame_hd;
    struct mp3_frame_header_t mp3_frame_hd;
    uint32_t n;

    f_lseek(fp, 0);
    f_read(fp, (void *)&id3v2_hd, MP3_TAG_HD_SIZE, &n);
    if(n != MP3_TAG_HD_SIZE)
    {
        return false;
    }
    
    if(strncmp(id3v2_hd.header,"ID3",3) != 0)
    {
        return false;
    }
    
    /* 计算ID3V2标签的大小，包括标签头的10字节大小 */
    tag_size =     (((id3v2_hd.size)[0] & 0x7F) << 21) |
                                (((id3v2_hd.size)[1] & 0x7F) << 14) |
                                (((id3v2_hd.size)[2] & 0x7F) << 7) |
                                ((id3v2_hd.size)[3] & 0x7F);
    LOG_INFO("tag size=%x\r\n",tag_size);
    *size = tag_size;
    while(read_bytes < (tag_size - 10))
    {
        f_read(fp, (void *)&id3v2_frame_hd, MP3_TAG_HD_SIZE, &n);
        /*
        uint8_t *ptr = (uint8_t *)&id3v2_frame_hd;
        printf("frame hd:\r\n");
        for(uint16_t j=0;j<n;j++)
        {
            printf("%02x ",*(ptr+j));
        }*/
        
        if(n != MP3_TAG_HD_SIZE)
        {
            return false;
        }
        fbody_size =    ((id3v2_frame_hd.size)[0] << 24) |
                    ((id3v2_frame_hd.size)[1] << 16) |
                    ((id3v2_frame_hd.size)[2] << 8) |
                    (id3v2_frame_hd.size)[3];
                    
        read_bytes += (MP3_TAG_HD_SIZE + fbody_size);

        if (fbody_size == 0)
        {
            ++counter;
            
            if (counter >= 5)                   //当程序获取标签帧的帧体连续5次都五内容 
            {                               //程序就判断后续已没有信息标签帧，随即中止循环
                //printf("No more frame!\n");
                break;                      
            }                               
            continue;
        }
        else
        {
            counter = 0;    
        }
        
        //f_read(fp, (void *)&id3v2_frame_hd, fbody_size, &n);
        
        //printf("body size = %x,%x,%s\r\n",fbody_size,read_bytes,id3v2_frame_hd.frameID);
        //标题,作者,专辑信息保留
        if((strncmp(id3v2_frame_hd.frameID,"TIT2",4) == 0)
            ||(strncmp(id3v2_frame_hd.frameID,"TPE1",4) == 0)
            ||(strncmp(id3v2_frame_hd.frameID,"TALB",4) == 0))
        {
            //超出返回最大长度
            if((save_index + MP3_TAG_HD_SIZE + fbody_size) >= MAX_TAG_LEN)
            {
                //printf("excced size\r\n");
                return false;
            }
            
            memcpy(&tag_buf[save_index],(void *)&id3v2_frame_hd,MP3_TAG_HD_SIZE);
            f_read(fp, (void *)&tag_buf[save_index + MP3_TAG_HD_SIZE], fbody_size, &n);
            save_index += MP3_TAG_HD_SIZE + fbody_size;
        }
        else
        {
            f_lseek(fp,MP3_TAG_HD_SIZE+read_bytes);
            //f_read(fp, (void *)&tag_buf[save_index + 4 + MP3_TAG_HD_SIZE], fbody_size, &n);
        }
        
    }

    //big endian
    //memcpy(&tag_buf[0],(void *)&save_index,4);
    tag_buf[1] = save_index&0xff;
    tag_buf[0] = (save_index>>8)&0xff;

    
    //tag_buf[7] = tag_size&0xff;
    //tag_buf[6] = (tag_size>>8)&0xff;
    //tag_buf[5] = (tag_size>>16)&0xff;
    //tag_buf[4] = (tag_size>>24)&0xff;
    
    f_lseek(fp, tag_size+MP3_TAG_HD_SIZE);
    f_read(fp, (void *)&mp3_frame_hd, 4, &n);
    //printf("%x,%x,%x\r\n",mp3_frame_hd.bit_rate_index,mp3_frame_hd.sample_rate_index,mp3_frame_hd.sync2);

    if(mp3_frame_hd.version == 0)
    {
        //MPEG2.5
        sample_rate = MPEG25_rate[mp3_frame_hd.sample_rate_index];
        sample_cnt = MPEG2[mp3_frame_hd.layer];
    }
    else if(mp3_frame_hd.version == 2)
    {
        //MPEG2        
        sample_rate = MPEG2_rate[mp3_frame_hd.sample_rate_index];
        sample_cnt = MPEG2[mp3_frame_hd.layer];
    }
    else
    {
        //MPEG1
        sample_rate = MPEG1_rate[mp3_frame_hd.sample_rate_index];
        sample_cnt = MPEG1[mp3_frame_hd.layer];
    }

    f_lseek(fp, tag_size+MP3_TAG_HD_SIZE+4+32);
    
    f_read(fp, (void *)&temp_buf[0], 4, &n);
    if(memcmp(temp_buf,"Info",4) == 0)
    {
       
        f_read(fp, (void *)&temp_buf[0], 4, &n);
        
        f_read(fp, (void *)&frame_cnt, 4, &n);
        f_read(fp, (void *)&mp3_total_len, 4, &n);
        LOG_INFO("sample cnt = %x,%x,frame_cnt = %x,total_len = %x\r\n",sample_cnt,sample_rate,frame_cnt,mp3_total_len);
    }
    
    //memcpy(&tag_buf[4],(void *)&frame_cnt,4);
    memcpy(&tag_buf[2],(void *)&mp3_total_len,4);

    frame_cnt = ld_dword((uint8_t *)&frame_cnt);

    uint32_t tmp_low,tmp_high;
    fr_mul_64(&tmp_low, &tmp_high, sample_cnt*1000, frame_cnt);    
    duration = fr_div_64(tmp_low, tmp_high, sample_rate);
    //duration = (sample_cnt*1000*frame_cnt)/sample_rate;
    LOG_INFO("duration = %d\r\n",duration);

    tag_buf[9] = duration&0xff;
    tag_buf[8] = (duration>>8)&0xff;
    tag_buf[7] = (duration>>16)&0xff;
    tag_buf[6] = (duration>>24)&0xff;
    
    f_lseek(fp, tag_size+MP3_TAG_HD_SIZE);
    return true;
}

uint32_t mp3_get_tag_size(void* fpp)
{
    uint32_t tag_size = 0;
    FIL *fp = (FIL*)fpp;
    struct mp3_id3v2_header_t id3v2_hd;
    uint32_t n;

    f_lseek(fp, 0);
    f_read(fp, (void *)&id3v2_hd, MP3_TAG_HD_SIZE, &n);
    if(n != MP3_TAG_HD_SIZE)
    {
     return 0;
    }

    if(strncmp(id3v2_hd.header,"ID3",3) != 0)
    {
     return 0;
    }

    /* 计算ID3V2标签的大小，包括标签头的10字节大小 */
    tag_size =     (((id3v2_hd.size)[0] & 0x7F) << 21) |
                             (((id3v2_hd.size)[1] & 0x7F) << 14) |
                             (((id3v2_hd.size)[2] & 0x7F) << 7) |
                             ((id3v2_hd.size)[3] & 0x7F);

    return (tag_size + MP3_TAG_HD_SIZE);
    
}
