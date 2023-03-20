/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MASS_MAL_H
#define __MASS_MAL_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define MASS_LUN_IDX_SD_NAND           0
#define MASS_LUN_IDX_SPI_FLASH      1
#define MAL_OK   0
#define MAL_FAIL 1
#define MAX_LUN  0      //2∏ˆø…“∆∂Ø¥≈≈Ã SDø®+FLASH 

#define U_DISC0_FLASH_BASE  0x60000      //u disc 0, flash base
#define U_DISC0_SIZE        0x10000      //u disc 0, storage size
#define U_DISC1_FLASH_BASE  0x70000      //u disc 1, flash base
#define U_DISC1_SIZE        0x10000      //u disc 1, storage size

extern long long Mass_Memory_Size[MAX_LUN+1];
extern uint32_t Mass_Block_Size[MAX_LUN+1];
extern uint32_t Mass_Block_Count[MAX_LUN+1];

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

uint16_t MAL_Init (uint8_t lun);
uint16_t MAL_GetStatus (uint8_t lun);
uint16_t MAL_Read(uint8_t lun, uint32_t Memory_Offset, uint32_t *Readbuff, uint16_t Transfer_Length);
uint16_t MAL_Write(uint8_t lun, uint32_t Memory_Offset, uint32_t *Writebuff, uint16_t Transfer_Length);
#endif /* __MASS_MAL_H */

