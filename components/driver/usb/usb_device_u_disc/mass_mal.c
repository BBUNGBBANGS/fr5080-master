/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <string.h>
#include "user_utils.h"

#include "driver_flash.h"
#include "driver_sdc.h"

#include "mass_mal.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

#define MEM_ASSERT(v) \
do { \
    if (!(v)) {             \
        printf("%s %d \r\n", __FILE__, __LINE__); \
        while (1);   \
    }                   \
} while (0);


/* Private variables ---------------------------------------------------------*/
long long Mass_Memory_Size[MAX_LUN+1];
uint32_t Mass_Block_Size[MAX_LUN+1];
uint32_t Mass_Block_Count[MAX_LUN+1];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
struct
{
    bool start;             //is fast wrtie begins?
    uint32_t last_mem_offset;   //the logic address windows send down to wrtie.
    uint32_t erased_flash_page;     //current erased flash page
    uint32_t page_write_offset;     //current offset in erased flash page
} fast_write = {false,0,0xffffffff,0};

uint8_t SPI_FLASH_BUF[4096];
void SPI_Flash_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead)
{
    //printf("f_r:%x,len:%d\r\n",ReadAddr,NumByteToRead);
    flash_read(ReadAddr,NumByteToRead,pBuffer);
    //show_reg((uint8_t*)pBuffer,10,1);
}

void SPI_Flash_Write_NoCheck(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    //printf("f_w:%x,len:%d\r\n",WriteAddr,NumByteToWrite);
    //show_reg((uint8_t*)pBuffer,10,1);
    flash_write(WriteAddr,NumByteToWrite,pBuffer);
}

void SPI_Flash_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t secremain;
    uint16_t i;

    secpos=WriteAddr/4096;//扇区地址 0~511 for w25x16
    secoff=WriteAddr%4096;//在扇区内的偏移
    secremain=4096-secoff;//扇区剩余空间大小

    if(NumByteToWrite<=secremain)secremain=NumByteToWrite;//不大于4096个字节
    while(1)
    {
        SPI_Flash_Read(SPI_FLASH_BUF,secpos*4096,4096);//读出整个扇区的内容
        for(i=0; i<secremain; i++) //校验数据
        {
            if(SPI_FLASH_BUF[secoff+i]!=0XFF)break;//需要擦除
        }
        if(i<secremain)//需要擦除
        {
            flash_erase(secpos*4096,0x1000);//擦除这个扇区
            for(i=0; i<secremain; i++)     //复制
            {
                SPI_FLASH_BUF[i+secoff]=pBuffer[i];
            }
            SPI_Flash_Write_NoCheck(SPI_FLASH_BUF,secpos*4096,4096);//写入整个扇区
        }
        else
            SPI_Flash_Write_NoCheck(pBuffer,WriteAddr,secremain); //写已经擦除了的,直接写入扇区剩余区间.
        if(NumByteToWrite==secremain)
            break;//写入结束了
        else//写入未结束
        {
            secpos++;//扇区地址增1
            secoff=0;//偏移位置为0

            pBuffer+=secremain;  //指针偏移
            WriteAddr+=secremain;//写地址偏移
            NumByteToWrite-=secremain;              //字节数递减
            if(NumByteToWrite>4096)secremain=4096;  //下一个扇区还是写不完
            else secremain=NumByteToWrite;          //下一个扇区可以写完了
        }
    };
}

void SPI_Flash_Write_dyc(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite)
{
    uint32_t secpos;
    uint16_t secoff;
    uint16_t w_pos = 0;
    uint16_t w_len = 0;
    secpos=WriteAddr/4096;//扇区地址 0~511 for w25x16
    secoff=WriteAddr%4096;//在扇区内的偏移
    while(1)
    {
        if(fast_write.erased_flash_page == 0xffffffff || fast_write.erased_flash_page != secpos || fast_write.page_write_offset > secoff)
        {
            flash_erase(secpos*4096,0x1000);//擦除这个扇区
            fast_write.erased_flash_page = secpos;
            fast_write.page_write_offset = 0;
            //printf("sec_pos:%x\r\n",secpos);
        }
        if(fast_write.page_write_offset < secoff )
            fast_write.page_write_offset = secoff;

        if(NumByteToWrite<=(4096 - fast_write.page_write_offset))
            w_len = NumByteToWrite;
        else
        {
            w_len = (4096 - fast_write.page_write_offset);
            secpos++;
        }
        //printf("off:%d,w_len:%d\r\n",flash_page_offset,w_len);
        SPI_Flash_Write_NoCheck(pBuffer+w_pos,WriteAddr,w_len);
        fast_write.page_write_offset += w_len;
        w_pos += w_len;

        if(w_pos == NumByteToWrite)
            break;
    }
}


/*******************************************************************************
* Function Name  : MAL_Init
* Description    : Initializes the Media on the STM32
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_Init(uint8_t lun)
{
    if(lun == MASS_LUN_IDX_SD_NAND) {
        Mass_Memory_Size[0]= U_DISC0_SIZE;
        Mass_Block_Size[0]= 512;
        //printf("block count is %d.\r\n", sdc_get_blkcnt());
        Mass_Block_Count[0]= sdc_get_blkcnt(); //Mass_Memory_Size[0]/Mass_Block_Size[0];
    }
#if (MAX_LUN == 1)
    else if(lun == MASS_LUN_IDX_SPI_FLASH) {
        Mass_Memory_Size[1]= U_DISC1_SIZE;
        Mass_Block_Size[1]= 512;
        Mass_Block_Count[1]= Mass_Memory_Size[1]/Mass_Block_Size[1];
    }
#endif

    return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_Write
* Description    : Write sectors
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/

extern uint8_t USB_STATUS_REG;
uint16_t MAL_Write(uint8_t lun, uint32_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length)
{
    uint8_t STA;
    switch (lun)
    {
        case 0:
            STA=0;
            /***************For fast flash_write****************/
#if 0
            if(Memory_Offset >= 10*4096 && Memory_Offset < 0x210*4096 && (USB_STATUS_REG & 0x01) )      //page0~7, is file system info. need old flash write fucntion
            {
                if(fast_write.start && (Memory_Offset == fast_write.last_mem_offset + Transfer_Length) )
                {
                    if( (Memory_Offset%4096) !=0 )
                    {
                        fast_write.last_mem_offset = Memory_Offset;
                        SPI_Flash_Write_dyc((uint8_t*)Writebuff, Memory_Offset, Transfer_Length);
                    }
                }
                else
                {
                    SPI_Flash_Write((uint8_t*)Writebuff, Memory_Offset, Transfer_Length);
                    fast_write.start = false;
                    fast_write.last_mem_offset = 0;
                    fast_write.erased_flash_page = 0xffffffff;
                    fast_write.page_write_offset = 0;
                }
                if((Memory_Offset%4096) == 0)
                {
                    fast_write.start = true;
                    fast_write.last_mem_offset = Memory_Offset;
                    SPI_Flash_Write_dyc((uint8_t*)Writebuff, Memory_Offset, Transfer_Length);
                }
            }
            else
#endif
            {
                SPI_Flash_Write((uint8_t*)Writebuff, Memory_Offset + U_DISC0_FLASH_BASE, Transfer_Length);
                fast_write.start = false;
                fast_write.last_mem_offset = 0;
                fast_write.erased_flash_page = 0xffffffff;
                fast_write.page_write_offset = 0;
            }

            //STA=SD_WriteDisk((uint8_t*)Writebuff, Memory_Offset>>9, Transfer_Length>>9);
            break;
        case 1:
            STA=0;
            //printf("f_w:%x,len:%d\r\n",Memory_Offset + 0x60000,Transfer_Length);
            //show_reg((uint8_t*)Writebuff,10,1);
            SPI_Flash_Write((uint8_t*)Writebuff, Memory_Offset + U_DISC1_FLASH_BASE, Transfer_Length);
            break;
        default:
            return MAL_FAIL;
    }
    if(STA!=0)return MAL_FAIL;
    return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_Read
* Description    : Read sectors
* Input          : None
* Output         : None
* Return         : Buffer pointer
*******************************************************************************/
uint16_t MAL_Read(uint8_t lun, uint32_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length)
{
    uint8_t STA;
    switch (lun)
    {
        case 0:
            STA=0;
            SPI_Flash_Read((uint8_t*)Readbuff, Memory_Offset + U_DISC0_FLASH_BASE, Transfer_Length);
            //STA=SD_ReadDisk((uint8_t*)Readbuff, Memory_Offset>>9, Transfer_Length>>9);
            break;
        case 1:
            STA=0;
            //printf("f_r:%x,len:%d\r\n",Memory_Offset + 0x60000,Transfer_Length);
            SPI_Flash_Read((uint8_t*)Readbuff, Memory_Offset + U_DISC1_FLASH_BASE, Transfer_Length);
            //show_reg((uint8_t*)Readbuff,10,1);
            break;
        default:
            return MAL_FAIL;
    }
    if(STA!=0)return MAL_FAIL;
    return MAL_OK;
}

/*******************************************************************************
* Function Name  : MAL_GetStatus
* Description    : Get status
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
uint16_t MAL_GetStatus (uint8_t lun)
{
    switch(lun)
    {
        case 0:
            return MAL_OK;
        case 1:
            return MAL_OK;
        case 2:
            return MAL_FAIL;
        default:
            return MAL_FAIL;
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

