#include <stdio.h>
#include <stdint.h>
#include "driver_plf.h"
void HardFault_Handler_C(unsigned int* hardfault_args)
{
    printf("Crash, dump regs:\r\n");
    printf("PC    = 0x%08X\r\n",hardfault_args[6]);
    printf("LR    = 0x%08X\r\n",hardfault_args[5]);
    /**/
    printf("R0    = 0x%.8X\r\n",hardfault_args[0]);
    printf("R1    = 0x%.8X\r\n",hardfault_args[1]);
    printf("R2    = 0x%.8X\r\n",hardfault_args[2]);
    printf("R3    = 0x%.8X\r\n",hardfault_args[3]);
    printf("R12   = 0x%.8X\r\n",hardfault_args[4]);
    printf("PSR   = 0x%.8X\r\n",hardfault_args[7]);
    printf("BFAR  = 0x%.8X\r\n",*(unsigned int*)0xE000ED38);
    printf("CFSR  = 0x%.8X\r\n",*(unsigned int*)0xE000ED28);
    printf("HFSR  = 0x%.8X\r\n",*(unsigned int*)0xE000ED2C);
    printf("DFSR  = 0x%.8X\r\n",*(unsigned int*)0xE000ED30);
    printf("AFSR  = 0x%.8X\r\n",*(unsigned int*)0xE000ED3C);
    printf("SHCSR = 0x%.8X\r\n",SCB->SHCSR);
    
    printf("dump sp stack[sp sp-512]:\r\n");
    uint16_t i = 0;
    do
    {
        printf("0x%08X,",*(hardfault_args++));
        i++;
        if(i%4 == 0)
            printf("\r\n");
    }
    while(i<128);
    while(1);
}


