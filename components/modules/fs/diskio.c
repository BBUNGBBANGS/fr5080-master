/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"         /* Obtains integer types */
#include "diskio.h"     /* Declarations of disk functions */
#include "mass_mal.h"

#include "driver_sdc.h"

/* Definitions of physical drive number for each drive */
#define DEV_NAND        0   /* Example: Map Ramdisk to physical drive 0 */
#define DEV_FLASH       1   /* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB     2   /* Example: Map USB MSD to physical drive 2 */

#define FLASH_SECTOR_SIZE   512
uint32_t FLASH_SECTOR_COUNT = U_DISC0_SIZE/512;
uint32_t FLASH_SECTOR_COUNT_NAND;
#define FLASH_BLOCK_SIZE    8
extern void SPI_Flash_Write(uint8_t* pBuffer,uint32_t WriteAddr,uint16_t NumByteToWrite);
extern void SPI_Flash_Read(uint8_t* pBuffer,uint32_t ReadAddr,uint16_t NumByteToRead);

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
    BYTE pdrv       /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS stat = 0;

    switch (pdrv)
    {
        case DEV_FLASH :
            return stat;

        case DEV_NAND :
            return stat;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
    BYTE pdrv               /* Physical drive nmuber to identify the drive */
)
{
    DSTATUS stat = 0;
    switch (pdrv)
    {
        case DEV_FLASH :
            // translate the reslut code here
            //SPI_Flash_Init();
            FLASH_SECTOR_COUNT = U_DISC0_SIZE/512;
            return stat;

        case DEV_NAND :
            FLASH_SECTOR_COUNT_NAND = (sdc_get_blkcnt());
            // translate the reslut code here
            return stat;
    }
    return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
    BYTE pdrv,      /* Physical drive nmuber to identify the drive */
    BYTE *buff,     /* Data buffer to store read data */
    LBA_t sector,   /* Start sector in LBA */
    UINT count      /* Number of sectors to read */
)
{
    DRESULT res = DISKIO_RES_OK;

    switch (pdrv)
    {
        case DEV_FLASH :
            // translate the arguments here
            for(; count>0; count--)
            {
                SPI_Flash_Read(buff,sector*FLASH_SECTOR_SIZE + U_DISC0_FLASH_BASE,FLASH_SECTOR_SIZE);
                sector++;
                buff+=FLASH_SECTOR_SIZE;
            }
            return res;

        case DEV_NAND :
            // translate the arguments here
            for(; count>0; count--)
            {
                RES_T ret = sdmem_read_single_block(buff, sector);
                if(ret != RES_SUCCESS)
                    co_printf("r_EEERRR:%x\r\n",ret);
                sector++;
                buff+=FLASH_SECTOR_SIZE;
            }
            return res;
    }

    return DISKIO_RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
    BYTE pdrv,          /* Physical drive nmuber to identify the drive */
    const BYTE *buff,   /* Data to be written */
    LBA_t sector,       /* Start sector in LBA */
    UINT count          /* Number of sectors to write */
)
{
    DRESULT res = DISKIO_RES_OK;

    switch (pdrv)
    {
        case DEV_FLASH :
            // translate the arguments here
            for(; count>0; count--)
            {
                SPI_Flash_Write((uint8_t*)buff,sector*FLASH_SECTOR_SIZE + U_DISC0_FLASH_BASE,FLASH_SECTOR_SIZE);
                sector++;
                buff+=FLASH_SECTOR_SIZE;
            }
            return res;

        case DEV_NAND :
            // translate the arguments here
            for(; count>0; count--)
            {
                RES_T ret = sdmem_write_single_block((uint8_t*)buff, sector);
                if(ret != RES_SUCCESS)
                    co_printf("w_EEERRR:%x\r\n",ret);
                sector++;
                buff+=FLASH_SECTOR_SIZE;
            }
            return res;
    }

    return DISKIO_RES_PARERR;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
    BYTE pdrv,      /* Physical drive nmuber (0..) */
    BYTE cmd,       /* Control code */
    void *buff      /* Buffer to send/receive control data */
)
{
    DRESULT res;

    switch (pdrv)
    {
        case DEV_NAND :

            // Process of the command for the RAM drive
            switch(cmd)
            {
                case CTRL_SYNC:
                    res = DISKIO_RES_OK;
                    break;
                case GET_SECTOR_SIZE:
                    *(WORD*)buff = FLASH_SECTOR_SIZE;
                    res = DISKIO_RES_OK;
                    break;
                case GET_BLOCK_SIZE:
                    *(WORD*)buff = FLASH_BLOCK_SIZE;
                    res = DISKIO_RES_OK;
                    break;
                case GET_SECTOR_COUNT:
                    *(DWORD*)buff = FLASH_SECTOR_COUNT_NAND;
                    res = DISKIO_RES_OK;
                    break;
                case CTRL_TRIM:
                    res = DISKIO_RES_OK;
                    break;
                default:
                    res = DISKIO_RES_PARERR;
                    break;
            }
            return res;
        case DEV_FLASH :
					
            // Process of the command for the RAM drive
            switch(cmd)
            {
                case CTRL_SYNC:
                    res = DISKIO_RES_OK;
                    break;
                case GET_SECTOR_SIZE:
                    *(WORD*)buff = FLASH_SECTOR_SIZE;
                    res = DISKIO_RES_OK;
                    break;
                case GET_BLOCK_SIZE:
                    *(WORD*)buff = FLASH_BLOCK_SIZE;
                    res = DISKIO_RES_OK;
                    break;
                case GET_SECTOR_COUNT:
                    *(DWORD*)buff = FLASH_SECTOR_COUNT;
                    res = DISKIO_RES_OK;
                    break;
                case CTRL_TRIM:
                    res = DISKIO_RES_OK;
                    break;
                default:
                    res = DISKIO_RES_PARERR;
                    break;
            }
            return res;
    }

    return DISKIO_RES_PARERR;
}

DWORD get_fattime (void)
{
    return 0;
}


