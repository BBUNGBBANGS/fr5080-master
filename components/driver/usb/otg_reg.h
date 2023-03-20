#ifndef _OTG_REG_H_
#define _OTG_REG_H_

#define OTG_BASE_ADDR       0x40020000
// OTG - Common USB registers
#define OTG_FADDR           (OTG_BASE_ADDR+(0x00))              // FAddr Function address register.
#define OTG_POWER           (OTG_BASE_ADDR+(0x01))              // Power Power management register.
#define OTG_INTRTX1         (OTG_BASE_ADDR+(0x02))              // IntrTx1 Interrupt register for Endpoint 0 plus Tx Endpoint 1 to 7.
#define OTG_INTRTX2         (OTG_BASE_ADDR+(0x03))              // IntrTx2 Interrupt register for Tx EndpoUINT32s 8 to 15.
#define OTG_INTRRX1         (OTG_BASE_ADDR+(0x04))              // IntrRx1 Interrupt register for Rx EndpoUINT32s 1 to 7.
#define OTG_INTRRX2         (OTG_BASE_ADDR+(0x05))              // IntrRx2 Interrupt register for Rx EndpoUINT32s 8 to 15.
#define OTG_INTRUSB         (OTG_BASE_ADDR+(0x06))              // IntrUSB Interrupt register for common USB UINT32errupts.
#define OTG_INTRTX1E        (OTG_BASE_ADDR+(0x07))              // IntrTx1E Interrupt enable register for IntrTx1.
#define OTG_INTRTX2E        (OTG_BASE_ADDR+(0x08))              // IntrTx2E Interrupt enable register for IntrTx2.
#define OTG_INTRRX1E        (OTG_BASE_ADDR+(0x09))              // IntrRx1E Interrupt enable register for IntrRx1.
#define OTG_INTRRX2E        (OTG_BASE_ADDR+(0x0A))              // IntrRx2E Interrupt enable register for IntrRx2.
#define OTG_INTRUSBE        (OTG_BASE_ADDR+(0x0B))              // IntrUSBE Interrupt enable register for IntrUSB.
#define OTG_FRAME1          (OTG_BASE_ADDR+(0x0C))              // Frame1 Frame number bits 0 to 7.
#define OTG_FRAME2          (OTG_BASE_ADDR+(0x0D))              // Frame2 Frame number bits 8 to 10.
#define OTG_INDEX           (OTG_BASE_ADDR+(0x0E))              // Index Index register for selecting the endpoUINT32 status and control registers.
#define OTG_DEVCTL          (OTG_BASE_ADDR+(0x0F))              // DevCtl USB device control register.
// OTG - Indexed registers
#define OTG_TXMAXP          (OTG_BASE_ADDR+(0x10))              // TxMaxP Maximum packet size for peripheral IN endpoint. (Index register set to select EndpoUINT32s 1 每 15 only)
#define OTG_CSR0            (OTG_BASE_ADDR+(0x11))              // CSR0 Main Control Status register for Endpoint 0. (Index register set to select Endpoint 0)
#define OTG_TXCSR1          (OTG_BASE_ADDR+(0x11))              // TxCSR1 Control Status register 1 for peripheral IN endpoUINT32. (Index register set to select EndpoUINT32s 1 每 15)
#define OTG_CSR02           (OTG_BASE_ADDR+(0x12))              // CSR02 Subsidiary Control Status register for Endpoint 0, containing FlushFIFO bit. (Index register set to select Endpoint 0)
#define OTG_TXCSR2          (OTG_BASE_ADDR+(0x12))              // TxCSR2 Control Status register 2 for peripheral IN endpoUINT32. (Index register set to select EndpoUINT32s 1 每 15 only)
#define OTG_RXMAXP          (OTG_BASE_ADDR+(0x13))              // RxMaxP Maximum packet size for peripheral OUT endpoUINT32. (Index register set to select EndpoUINT32s 1 每 15 only)
#define OTG_RXCSR1          (OTG_BASE_ADDR+(0x14))              // RxCSR1 Control Status register 1 for peripheral OUT endpoUINT32. (Index register set to select EndpoUINT32s 1 每 15 only)
#define OTG_RXCSR2          (OTG_BASE_ADDR+(0x15))              // RxCSR2 Control Status register 2 for peripheral OUT endpoUINT32. (Index register set to select EndpoUINT32s 1 每 15 only)
#define OTG_COUNT0          (OTG_BASE_ADDR+(0x16))              // Count0 Number of received bytes in Endpoint 0 FIFO. (Index register set to select Endpoint 0)
#define OTG_RXCOUNT1        (OTG_BASE_ADDR+(0x16))              // RxCount1 Number of bytes in peripheral OUT endpoUINT32 FIFO (lower byte). (Index register set to select Endpoint 1 每 15)
#define OTG_RXCOUNT2        (OTG_BASE_ADDR+(0x17))              // RxCount2 Number of bytes in peripheral OUT endpoUINT32 FIFO (upper byte). (Index register set to select Endpoint 1 每 15 only)
// OTG - Indexed registers - Host mode
#define OTG_TXTYPE          (OTG_BASE_ADDR+(0x18))              // Sets the transaction protocol and peripheral Endpoint number for the host OUT endpoUINT32. (Index register set to select Endpoint 1 每 15 only)
#define OTG_NAKLIMIT0       (OTG_BASE_ADDR+(0x19))              // Sets the NAK response timeout on Endpoint 0. (Index register set to select Endpoint 0)
#define OTG_TXINTERVAL      (OTG_BASE_ADDR+(0x19))              // Sets the polling UINT32erval for an OUT Interrupt endpoUINT32, in ms. (Index register set to select EndpoUINT32s 1 每 15 only)
#define OTG_RXTYPE          (OTG_BASE_ADDR+(0x1A))              // Sets the transaction protocol and peripheral endpoUINT32 number for the host IN endpoUINT32. (Index register set to select Endpoint 1 每 15 only)
#define OTG_RXINTERVAL      (OTG_BASE_ADDR+(0x1B))              // Sets the polling UINT32erval for an IN Interrupt endpoUINT32, in ms. (Index register set to select EndpoUINT32s 1 每 15 only)
// OTG - Indexed registers
#define OTG_TXFIFO1         (OTG_BASE_ADDR+(0x1C))
#define OTG_TXFIFO2         (OTG_BASE_ADDR+(0x1D))
#define OTG_RXFIFO1         (OTG_BASE_ADDR+(0x1E))
#define OTG_RXFIFO2         (OTG_BASE_ADDR+(0x1F))
// OTG - FIFOs
#define OTG_EP0FIFO         (OTG_BASE_ADDR+(0x20))
#define OTG_EP1FIFO         (OTG_BASE_ADDR+(0x24))
#define OTG_EP2FIFO         (OTG_BASE_ADDR+(0x28))
#define OTG_EP3FIFO         (OTG_BASE_ADDR+(0x2C))

#define OTG_FFBITS          (OTG_BASE_ADDR+(0x1F))
#define OTG_DMA_INTR        (OTG_BASE_ADDR+0x200)               // OTGC Indicates pending UINT32errupts
#define OTG_DMA_CNTL1       (OTG_BASE_ADDR+0x204)               // OTGC DMA Channel 1 Control:
#define OTG_DMA_ADDR1       (OTG_BASE_ADDR+0x208)               // OTGC DMA Channel 1 AHB Memory Address (32 bits)
#define OTG_DMA_COUNT1      (OTG_BASE_ADDR+0x20C)               // OTGC DMA Channel 1 Byte Count (32 bits)

#ifndef REGB
#define REGB(a)             *((volatile uint8_t *)(a))
#endif

#ifndef REGW
#define REGW(a)             *((volatile uint32_t *)(a))
#endif

#endif /* _OTG_REG_H_ */

