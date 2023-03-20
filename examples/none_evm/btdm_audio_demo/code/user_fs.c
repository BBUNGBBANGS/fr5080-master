/*----------------------------------------------------------------------/
/ Allocate a contiguous area to the file
/-----------------------------------------------------------------------/
/ This function checks if the file is contiguous with desired size.
/ If not, a block of contiguous sectors is allocated to the file.
/ If the file has been opened without FA_WRITE flag, it only checks if
/ the file is contiguous and returns the resulut.
/-----------------------------------------------------------------------/
/ This function can work with FatFs R0.09 - R0.11a.
/ It is incompatible with R0.12+. Use f_expand function instead.
/----------------------------------------------------------------------*/

/* Declarations of FatFs internal functions accessible from applications.
/  This is intended to be used for disk checking/fixing or dirty hacks :-) */

#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "ff.h"
#include "co_printf.h"
#include "co_log.h"
#include "app_at.h"
#include "user_utils.h"
#include "os_mem.h"
#include "mp3_tag_decoder.h"
#include "user_fs.h"
#include "user_bt.h"

#define IsLower(c)		((c) >= 'a' && (c) <= 'z')
#define ToUpper(c)      (IsLower(c)?c-0x20:c)

#define FS_REC_LVL      3       //search  sub-directory  level

//static FATFS fs;
static FIL fil;
//static char  search[256] = {0};
//static char  file_ext[4] = {'M', 'P', '3', };
//static DIR dir;
//static FILINFO f_info;

#define FILE_TYPE_CATALOG       0
#define FILE_TYPE_MP3           1

struct mp3_env_t{
    struct list_entry_t file_list;
    struct list_entry_t mp3_list;
    uint8_t total_file_num;
    uint8_t total_mp3_num;
    //uint8_t cur_mp3_index;
    struct file_info_item_t *cur_item;
    FIL* mp3_fp;
};

struct file_info_item_t{
    struct list_entry_t node;
    struct file_info_item_t *parent;
    uint8_t type;
    uint8_t *name;
    uint8_t name_len;
};

struct mp3_env_t mp3_env;
struct list_entry_t *last_dir;
uint8_t tag_buf[256+9];

uint8_t loop_num = 0;

uint32_t mp3_read_size = 0;
uint32_t mp3_read_total_size = 0;

bool ff_scan_mp3_files(uint8_t * path, uint8_t rec_lvl)
{
    FRESULT fr;
    DIR dir;
    FILINFO f_info;
    BYTE i;
    static bool ret = false;
    
    //char *path_new;
    uint16_t info_len;
    struct file_info_item_t *item;
    struct file_info_item_t *parent_dir;
    
    LOG_INFO("scan mp3,path=%s\r\n",path);
    fr = f_opendir(&dir,(const TCHAR*)path); //打开一个目录
    if (fr != FR_OK) {
        //printf("ret1\r\n");
        return false;
    }
    loop_num++;
    //LOG_INFO("loop num = %d,%d\r\n",loop_num,rec_lvl);
    if(loop_num >= rec_lvl){
        //printf("close %s\r\n",path);
        loop_num--;
        f_closedir(&dir);
        for(int16_t k=strlen((void *)path)-1; k>=0; k--)
        {
            if(path[k] != '/')
                continue;
            else{
                path[k] = '\0';
                break;
            }
        }
        
        last_dir = &parent_dir->parent->node;
        return true;
    }
    //catalog_head = file_head->parent;
    if(path == '\0'){
        parent_dir = (struct file_info_item_t *)get_head_list(&mp3_env.file_list);
    }else{
        parent_dir = (struct file_info_item_t *)last_dir;
        //catalog_head = (struct file_info_item_t *)&catalog_head->parent->node;
    }
    while(1)
    {
        fr = f_readdir(&dir, &f_info);                   //读取目录下的一个文件
        if (fr != FR_OK || !f_info.fname[0])   /* End of directory? */
        {
            //ret = false;
            break;
        }
        
        //printf("name:%s\r\n", f_info.fname);
        if (f_info.fattrib & AM_DIR){
            //printf("dir:%s\r\n");
            
            i = strlen((void *)path);
            /*
            if((i==0) || (memcmp(path,path_save,i) !(= 0)){
                memset(&path_save[0],'\0',256);
            }
            
            //printf("path-save temp=%s\r\n",path_save);
            path_save[i] = '/';
            
            memcpy(&path_save[i+1],f_info.fname,strlen(f_info.fname));
            */
            info_len = strlen(f_info.fname);
            path[i] = '/';
            memcpy(&path[i+1],f_info.fname,info_len);
            path[i+1+info_len] = '\0';
            
            item = (struct file_info_item_t *)os_malloc(sizeof(struct file_info_item_t));
            item->type = FILE_TYPE_CATALOG;
            item->name = os_malloc(info_len+1);
            item->name_len = info_len + 1;
            memcpy(item->name,f_info.fname,info_len);
            item->name[info_len] = '\0';
            item->parent = parent_dir;
            insert_tail_list(&mp3_env.file_list, &item->node);
            mp3_env.total_file_num ++;
            last_dir = &item->node;
            //printf("dir path=%s,fname=%s,%x,%x\r\n",path,f_info.fname,item,item->parent);
            ff_scan_mp3_files(path,rec_lvl);
        }
        else{
            i = strlen(f_info.fname);
            //printf("fname:%s,%d\r\n", f_info.fname,i);
            if((f_info.fname[i-4] == '.')
                &&(ToUpper(f_info.fname[i-3]) == 'M')
                &&(ToUpper(f_info.fname[i-2]) == 'P')
                &&(ToUpper(f_info.fname[i-1]) == '3')){
                LOG_INFO("path:%s,mp3:%s\r\n",path,f_info.fname);
                item = (struct file_info_item_t *)os_malloc(sizeof(struct file_info_item_t));
                item->type = FILE_TYPE_MP3;
                item->name = os_malloc(i+1);
                item->name_len = i + 1;
                memcpy(item->name,f_info.fname,i);
                item->name[i] = '\0';
                item->parent = parent_dir;
                
                insert_tail_list(&mp3_env.mp3_list, &item->node);
                
                //printf("path:%s,mp3:%s,%x\r\n",path,f_info.fname,item);
                mp3_env.total_mp3_num ++;
                ret = true;
            }
        }
            
    }
    //printf("close dir,path=%s,len=%d\r\n",path,strlen(path));
    f_closedir(&dir);
    
    for(int16_t k=strlen((void *)path)-1; k>=0; k--)
    {
        if(path[k] != '/')
            continue;
        else{
            path[k] = '\0';
            break;
        }
    }
    last_dir = &parent_dir->parent->node;
    //mp3_env.cur_item = (struct file_info_item_t *)get_next_node(&mp3_env.mp3_list);
    //printf("after close,path=%s\r\n",path);
    loop_num--;
    //fr = f_chdir()
    return ret;
}

#if 0
void ff_get_all_mp3_file(void)
{
    uint8_t i,k=0;
    int8_t j;
    struct file_info_item_t *item,*item_tmp;
    uint8_t *name;
    uint8_t len[4] = {0};
    uint8_t path[4][16];
    uint8_t path_total[60];
    
    item = (struct file_info_item_t *)get_next_node(&mp3_env.mp3_list);
    printf("ff_get_all_mp3_file,num=%d\r\n",mp3_env.total_mp3_num);
    for(i=0; i<mp3_env.total_mp3_num; i++){
        j = 0;
        item_tmp = item;
        name = item_tmp->parent->name;
        path_total[0] = '\0';
        while(strlen(name) != 0){
            len[j] = strlen(name) + 1;
            //printf("name = %s,len=%d\r\n",name,len[j]);
            path[j][0] = '/';
            memcpy(&path[j][1],name,strlen(name));
            item_tmp = item_tmp->parent;
            name = item_tmp->parent->name;
            j++;
        }
        
        if(j>0){
            k = 0;
            for(j=j-1;j>=0;j--){
                memcpy(path_total+k,&path[j][0],len[j]);
                k += len[j];
            }
            path_total[k] = '\0';
        } 

        printf("mp3:%s,par=%s\r\n",item->name,path_total);
        item = (struct file_info_item_t *)get_next_node(&item->node);
    } 
}
#endif


void ff_get_cur_mp3_path(uint8_t *path_total)
{
    uint16_t t_len,k=0;
    int8_t j;
    struct file_info_item_t *item,*item_tmp;
    uint8_t *name;
    uint8_t len[FS_REC_LVL] = {0};
    uint8_t path[FS_REC_LVL][FF_LFN_BUF+1];
    //uint8_t path_total[60];

    item = mp3_env.cur_item;
    //printf("ff_get_cur_mp3_path,%s,%x\r\n",item->name,item->parent);
    j = 0;
    item_tmp = item;
    name = item_tmp->parent->name;
    path_total[0] = '\0';
    while(strlen((void *)name) != 0){
        len[j] = strlen((void *)name) + 1;
        //printf("name = %s,len=%d\r\n",name,len[j]);
        path[j][0] = '/';
        memcpy(&path[j][1],name,strlen((void *)name));
        item_tmp = item_tmp->parent;
        name = item_tmp->parent->name;
        j++;
    }
    
    if(j>0){
        k = 0;
        for(j=j-1;j>=0;j--){
            memcpy(path_total+k,&path[j][0],len[j]);
            k += len[j];
        }
        path_total[k] = '\0';
    } 
    
    t_len = strlen((void *)path_total);
    path_total[t_len] = '/';
    //printf("cur name = %s,%x\r\n",mp3_env.cur_item->name,mp3_env.cur_item);
    memcpy(path_total+t_len+1,mp3_env.cur_item->name,strlen((void *)mp3_env.cur_item->name));
   

    printf("mp3:%s,par=%s\r\n",item->name,path_total);

}

void ff_get_prev_node(void)
{
    struct file_info_item_t *item;

    item = mp3_env.cur_item;
    /*
    if(mp3_env.cur_mp3_index == 0){
        mp3_env.cur_mp3_index = mp3_env.total_mp3_num - 1;
        item = (struct file_info_item_t *)get_prior_node(&item->node);
    }else{
        mp3_env.cur_mp3_index --;
    }
    
    */
    mp3_env.cur_item = (struct file_info_item_t *)get_prior_node(&item->node);
    if((uint32_t)mp3_env.cur_item == (uint32_t)&mp3_env.mp3_list){
        mp3_env.cur_item = (struct file_info_item_t *)get_prior_node(&mp3_env.cur_item->node);
        
    }
}

void ff_get_next_node(void)
{
    struct file_info_item_t *item;

    item = mp3_env.cur_item;
    #if 0
    mp3_env.cur_mp3_index ++;
    if(mp3_env.cur_mp3_index >= mp3_env.total_mp3_num){
        mp3_env.cur_mp3_index = 0;
        item = (struct file_info_item_t *)get_next_node(&item->node);
    }
    #endif
    mp3_env.cur_item = (struct file_info_item_t *)get_next_node(&item->node);
    if((uint32_t)mp3_env.cur_item == (uint32_t)&mp3_env.mp3_list){
        mp3_env.cur_item = (struct file_info_item_t *)get_next_node(&mp3_env.cur_item->node);
        
    }

}

int fs_init(void)
{
    /* open music directory and the first mp3 file */
    FRESULT fr;
    struct file_info_item_t *item_head;
    uint8_t path[FF_LFN_BUF*4+1] = {'\0'};
    static FATFS fs;

    /* Open or create a file to write */
    fr = f_mount(&fs, "", 0);
    if (fr)
    {
        return 1;
    }

    init_list_head(&mp3_env.mp3_list);
    init_list_head(&mp3_env.file_list);
    item_head = (struct file_info_item_t *)os_malloc(sizeof(struct file_info_item_t));
    item_head->type = FILE_TYPE_CATALOG;

    item_head->name = os_malloc(1);
    item_head->name[0] = '\0';
    item_head->name_len = 1;
    item_head->parent = NULL;
    insert_head_list(&mp3_env.file_list, &item_head->node);
    //printf("first file node %x\r\n",item_head);
    mp3_env.total_mp3_num = 0;
    mp3_env.total_file_num = 1;
    //mp3_env.cur_mp3_index = 0;
    mp3_env.cur_item = NULL;
    mp3_env.mp3_fp = NULL;
    last_dir = &item_head->node;

    if(false == ff_scan_mp3_files(path, FS_REC_LVL)){
        return 1;
    }
    LOG_INFO("find mp3,num = %d\r\n",mp3_env.total_file_num);
    mp3_env.cur_item = (struct file_info_item_t *)get_next_node(&mp3_env.mp3_list);
    ff_get_cur_mp3_path(path);

    fr = f_open(&fil, (void *)path, FA_READ);
    LOG_INFO("file open:%s:0x%02x\r\n", path, fr);
    if (fr)
    {
        return 1;
    }

    mp3_env.mp3_fp = &fil;
    
    //fs_uart_send_mp3_info();
    f_lseek(mp3_env.mp3_fp, mp3_get_tag_size(&fil));
    
    return 0;
}

uint8_t fs_get_mp3_num(void)
{
    return mp3_env.total_mp3_num;
}

uint32_t fs_read(uint8_t *buffer, uint32_t length)
{    
    if(mp3_env.mp3_fp)
    {
        uint32_t n;
        f_read(mp3_env.mp3_fp, buffer, length, &n);
        return n;
    }
    else
    {
        return 0;
    }
}

void fs_prepare_specified(void *mp3_item)
{
    FRESULT fr;
    uint8_t path[FF_LFN_BUF*FS_REC_LVL+1] = {'\0'};
    f_close(mp3_env.mp3_fp);
    mp3_env.mp3_fp = NULL;
    
    //ff_get_next_file("music", true, search, file_ext);
    //ff_get_next_node();
    mp3_env.cur_item = (struct file_info_item_t *)mp3_item;
    ff_get_cur_mp3_path(path);

    fr = f_open(&fil, (void *)path, FA_READ);
    LOG_INFO("file open:%s:0x%02x\r\n", path, fr);
    
    mp3_env.mp3_fp = &fil;
    f_lseek(mp3_env.mp3_fp, mp3_get_tag_size(&fil));
    mp3_read_size = 0;
    mp3_read_total_size = 0;
}

void fs_prepare_next(void)
{
    FRESULT fr;
    uint8_t path[FF_LFN_BUF*FS_REC_LVL+1] = {'\0'};

    f_close(mp3_env.mp3_fp);
    mp3_env.mp3_fp = NULL;
    
    //ff_get_next_file("music", true, search, file_ext);
    ff_get_next_node();
    ff_get_cur_mp3_path(path);

    fr = f_open(&fil, (void *)path, FA_READ);
    LOG_INFO("file open:%s:0x%02x\r\n", path, fr);
    
    mp3_env.mp3_fp = &fil;
    f_lseek(mp3_env.mp3_fp, mp3_get_tag_size(&fil));
    mp3_read_size = 0;
    mp3_read_total_size = 0;
}

void fs_prepare_prev(void)
{
    FRESULT fr;
    uint8_t path[FF_LFN_BUF*FS_REC_LVL+1] = {'\0'};
    
    f_close(mp3_env.mp3_fp);
    mp3_env.mp3_fp = NULL;
    
    ff_get_prev_node();
    ff_get_cur_mp3_path(path);
    
    fr = f_open(&fil, (void *)path, FA_READ);
    LOG_INFO("file open:%s:0x%02x\r\n", path, fr);
    
    mp3_env.mp3_fp = &fil;
    f_lseek(mp3_env.mp3_fp, mp3_get_tag_size(&fil));
    mp3_read_size = 0;
    mp3_read_total_size = 0;

}

extern uint8_t user_evt_notify_enable;

void fs_uart_send_mp3_info(void)
{
    uint32_t tag_len;
    uint32_t tag_size;
    if(user_evt_notify_enable&USER_EVT_NOTIFY_MP3_INFO){
        tag_buf[0] = '+';
        tag_buf[1] = 'M';
        tag_buf[2] = 'P';
        tag_buf[3] = '3';
        tag_buf[4] = ':';
      
        tag_buf[8] = ((uint32_t)mp3_env.cur_item)&0xff;
        tag_buf[7] = ((uint32_t)mp3_env.cur_item>>8)&0xff;
        tag_buf[6] = ((uint32_t)mp3_env.cur_item>>16)&0xff;
        tag_buf[5] = ((uint32_t)mp3_env.cur_item>>24)&0xff;

        
        //memcpy(&tag_buf[5],(uint8_t *)mp3_env.cur_item,4);
        if(true == mp3_get_tag_info(mp3_env.mp3_fp, &tag_buf[9],&tag_size))
        {
        
            //printf("test2: %x\r\rn",mp3_env.cur_item);
            tag_len = (tag_buf[9]<<8)|tag_buf[10];
            //printf("tag_len:%d,%x\r\n",tag_len+9,mp3_env.mp3_fp);
            
            uart_send(&tag_buf[0],tag_len+9);

            //uart_send("MP3 info\r\n",10);

            //mp3_read_size = 0;
            //mp3_read_total_size = 0;
        }
        else
        {
            //printf("no mp3 info\r\n");
            uart_send(&tag_buf[0],9);
        }

    }

    //set read point to current file location
    f_lseek(mp3_env.mp3_fp, tag_size + MP3_TAG_HD_SIZE + mp3_read_total_size);
}

bool fs_handle_mp3_list_req(void *item, uint8_t direction)
{
    uint16_t len;

    if(mp3_env.cur_item == NULL){
        return false;
    }
    if(item != NULL){
        mp3_env.cur_item = (struct file_info_item_t *)item;
    }
    
    if(direction == MCU_REQ_MP3_NEXT){
        fs_prepare_next();
    }else if(direction == MCU_REQ_MP3_PREV){
        fs_prepare_prev();
    }
    
    tag_buf[0] = '+';
    tag_buf[1] = 'N';
    tag_buf[2] = 'A';
    tag_buf[3] = 'M';
    tag_buf[4] = 'E';
    tag_buf[5] = ':';
    
    tag_buf[9] = ((uint32_t)mp3_env.cur_item)&0xff;
    tag_buf[8] = ((uint32_t)mp3_env.cur_item>>8)&0xff;
    tag_buf[7] = ((uint32_t)mp3_env.cur_item>>16)&0xff;
    tag_buf[6] = ((uint32_t)mp3_env.cur_item>>24)&0xff;
    
    len = strlen((void *)mp3_env.cur_item->name) + 1;
    tag_buf[10] = (len>>8)&0xff;
    tag_buf[11] = len&0xff;
    memcpy(&tag_buf[12],mp3_env.cur_item->name,len);
    
    uart_send(tag_buf, len+12);
    
    for(uint16_t i=0; i<len+12; i++)
    {
        printf("%02x ",tag_buf[i]);
    }
    printf("\r\n");
    return true;
}
bool fs_handle_mp3_info_req(void *item, uint8_t direction)
{
    //uint8_t path[FF_LFN_BUF*FS_REC_LVL+1] = {'\0'};
    //FRESULT fr;
    
    if(mp3_env.cur_item == NULL){
        return false;
    }

    if(item != NULL){
        mp3_env.cur_item = (struct file_info_item_t *)item;
    }
    
    if(direction == MCU_REQ_MP3_NEXT){
        fs_prepare_next();
    }else if(direction == MCU_REQ_MP3_PREV){
        fs_prepare_prev();
    }

    fs_uart_send_mp3_info();

    //fs_prepare_next();
    //f_close(&fil);
    return true;
}

bool is_fs_has_mp3_item(void)
{
    if(mp3_env.cur_item != NULL){
        return true;
    }else{
        return false;
    }
}
