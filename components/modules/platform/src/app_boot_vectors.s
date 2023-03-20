;/*****************************************************************************
; * @file:    startup_MPS_CM3.s
; * @purpose: CMSIS Cortex-M3 Core Device Startup File 
; *           for the ARM 'Microcontroller Prototyping System' 
; * @version: V1.01
; * @date:    19. Aug. 2009
; *------- <<< Use Configuration Wizard in Context Menu >>> ------------------
; *
; * Copyright (C) 2008-2009 ARM Limited. All rights reserved.
; * ARM Limited (ARM) is supplying this software for use with Cortex-M3 
; * processor based microcontrollers.  This file can be freely distributed 
; * within development tools that are supporting such ARM based processors. 
; *
; * THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
; * OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
; * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
; * ARM SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
; * CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
; *
; *****************************************************************************/

; <h> Stack Configuration
;   <o> Stack Size (in Bytes) <0x0-0xFFFFFFFF:8>
; </h>

                import Reset_Handler
                import NMI_Handler
                ;import HardFault_Handler
                import MemManage_Handler
                import BusFault_Handler
                import UsageFault_Handler
                import DebugMon_Handler
                import PendSV_Handler
                import SysTick_Handler
                import rwble_isr
                import rwip_isr_imp
                import rwbt_isr
				import rwbt_isr_imp
                import pmu_isr_imp
                import sbc_isr
                import uart_isr_ram
                import cdc_isr
                import ipc_isr
                import sbc_isr_imp
                import cdc_isr_imp
                import ipc_isr_ram
				import rwble_isr_imp

                AREA    STACK, NOINIT, READWRITE, ALIGN=3

                PRESERVE8
                THUMB

; Vector Table Mapped to Address 0 at Reset

                AREA    RESET, DATA, READONLY
                
__initial_sp    EQU     0x40004000

                DCD     __initial_sp ;  ?迆3?那??‘o車辰??～?迆MAINo‘那y?迆??那1車?EXMEM℅??aSTACK㏒?米豕os?a??o車㏒???handle thead?∩MSP谷豕?a0x0800xxxx?D米??米(?迄?Y車??∫????∩車D?)
                DCD     Reset_Handler             ; Reset Handler
                DCD     NMI_Handler               ; NMI Handler
                DCD     HardFault_Handler_Ram     ; Hard Fault Handler
                DCD     MemManage_Handler         ; MPU Fault Handler
                DCD     BusFault_Handler          ; Bus Fault Handler
                DCD     UsageFault_Handler        ; Usage Fault Handler
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     0                         ; Reserved
                DCD     SVC_Handler               ; SVCall Handler
                DCD     DebugMon_Handler          ; Debug Monitor Handler
                DCD     0                         ; Reserved
                DCD     PendSV_Handler            ; PendSV Handler
                DCD     SysTick_Handler   		  ; SysTick Handler
                    
                DCD     rwip_isr_imp              ; 0
                DCD     rwbt_isr_imp              ; 1
                DCD     rwble_isr_imp             ; 2
                DCD     0                         ; 3
                DCD     0                         ; 4
                DCD     USB_IRQHandler            ; 5
                DCD     0                         ; 6
                DCD     0                         ; 7
                DCD     timer0_isr_ram            ; 8
                DCD     timer1_isr_ram            ; 9
                DCD     0                         ; 10
                DCD     0                   	  ; 11
                DCD     uart_isr_ram              ; 12
                DCD     exti_isr_ram              ; 13
                DCD     i2s_isr_ram               ; 14
                DCD     0                         ; 15
                DCD     pmu_isr_imp               ; 16
                DCD     0                         ; 17
                DCD     0                         ; 18
                DCD     0                         ; 19
                DCD     0                         ; 20
                DCD     0                         ; 21
                DCD     0                         ; 22
                DCD     0                         ; 23
                DCD     ipc_isr_ram               ; 24
                DCD     ssp_isr                   ; 25
                DCD     sbc_isr_imp               ; 26
                DCD     0                         ; 27
                DCD     cdc_isr_imp               ; 28
                DCD     0                         ; 29
                DCD     0                         ; 30
                DCD     0                         ; 31

                AREA    |.text|, CODE, READONLY
HardFault_Handler_Ram   PROC
                IMPORT  HardFault_Handler_C
                TST LR, #4			;test bit[2] is 0 ,then exe EQ branch, MSP as sp
                ITE EQ
                MRSEQ R0, MSP
                MRSNE R0, PSP
                B HardFault_Handler_C
                ENDP

SVC_Handler     PROC
                IMPORT  prv_call_svc_pc
                IMPORT  vPortSVCHandler
                IMPORT  svc_exception_handler
                TST     LR, #4			;test bit[2] is 0 ,then exe EQ branch, MSP as sp
                ITE     EQ
                MRSEQ   R3, MSP
                MRSNE   R3, PSP
                LDR     R0, [R3, #0x18]     ;r0 = return_address
                LDR     R2, =prv_call_svc_pc
                ADD     R2, R2, #1
                CMP     R0, R2
                BEQ     vPortSVCHandler
                
                PUSH    {LR, R3}
SVC_Handler_1
                LDR     R1, [R3, #0x14]     ;r1 = lr
                LDR     R2, =svc_exception_handler
                LDR     R2, [R2, #0]
                BLX     R2
SVC_Handler_2
                POP     {LR, R3}
                STR     R0, [R3, #0x18]
                BX      LR
                ENDP
                    
Default_Handler PROC
                EXPORT  USB_IRQHandler  [WEAK]
                EXPORT  i2s_isr         [WEAK]
                EXPORT  ssp_isr         [WEAK]
                EXPORT  exti_isr_ram    [WEAK]
                EXPORT  i2s_isr_ram     [WEAK]
                EXPORT  timer0_isr_ram  [WEAK]
                EXPORT  timer1_isr_ram  [WEAK]
USB_IRQHandler
i2s_isr
ssp_isr
exti_isr_ram
i2s_isr_ram
timer0_isr_ram
timer1_isr_ram
                B       .
                NOP
                ENDP
                    
                END
