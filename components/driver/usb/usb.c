#include <stdint.h>

#include "otg_reg.h"
#include "usb.h"

uint8_t usbdev_event_flag;

#if USB_DEVICE_ENABLE == 1
void usb_read_fifo(uint8_t *p_des, uint32_t size, uint32_t fifo_addr)
{
    uint32_t *ptr;
    uint32_t s;

    s = ((uint32_t)p_des) & 0x3;
    if(size > s)
        size -= s;
    while(s--)
        *p_des++ = REGB(fifo_addr);

    s = size / 4;
    ptr = (uint32_t *)p_des;
    while(s--)
        *ptr++ = REGW(fifo_addr);

    s = size % 4;
    p_des = (uint8_t *)ptr;
    while(s--)
        *p_des++ = REGB(fifo_addr);
}
#endif

void usb_read_fifo32(uint32_t *p_des, uint32_t size, uint32_t fifo_addr)
{
    while(size--)
        *p_des++ = REGW(fifo_addr);
}

#if USB_DEVICE_ENABLE == 1
void usb_write_fifo(uint8_t *p_src, uint32_t size, uint32_t fifo_addr)
{
    uint32_t *ptr = (uint32_t *)p_src;
    uint8_t *c;
    int s;
    if(size == 0)
        return;
    s = size >> 2;
    while (s--)
        REGW(fifo_addr) = *ptr++;

    s = size & 3;
    if (s)
    {
        c = (uint8_t *)ptr;
        while (s--)
            REGB(fifo_addr) = *c++;
    }
}
#endif

/*******************************************************************************
* Function Name  : USB_SIL_Write
* Description    : Write a buffer of data to a selected endpoint.
* Input          : - bEpAddr: The address of the non control endpoint.
*                  - pBufferPointer: The pointer to the buffer of data to be written
*                    to the endpoint.
*                  - wBufferSize: Number of data to be written (in bytes).
* Output         : None.
* Return         : Status.
*******************************************************************************/
uint32_t USB_SIL_Write(uint8_t bEpAddr, uint8_t* pBufferPointer, uint32_t wBufferSize)
{
    /* Use the memory interface function to write to the selected endpoint */
    //UserToPMABufferCopy(pBufferPointer, GetEPTxAddr(bEpAddr & 0x7F), wBufferSize);

    /* Update the data length in the control register */
    //SetEPTxCount((bEpAddr & 0x7F), wBufferSize);
    
    REGB(OTG_INDEX) = (bEpAddr & 0x7F);
    usb_write_fifo(pBufferPointer, wBufferSize, OTG_EP1FIFO);
    return 0;
}

