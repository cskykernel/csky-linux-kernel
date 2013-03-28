/*
 *	ckspi.h -- for CK6408EVB Board internal SPI support defines.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  (C) Copyright 2012, Hu Junshan (junshan_hu@c-sky.com)
 *  (C) Copyright 2012, C-SKY Microsystems Co., Ltd. (www.c-sky.com) 
 */

#ifndef _CK6408EVB_SPI_H_
#define	_CK6408EVB_SPI_H_

#include <asm/addrspace.h>
#include <mach/ck_iomap.h>
#include <mach/irqs.h>

/*
 *  Define the CK6408EVB Board SPI register set addresses.
 *	Access as array with index.
 */
#define CK6408EVB_SPI_CTRLR0    0x00    /* Control Register 0 */
#define CK6408EVB_SPI_CTRLR1    0x04    /* Control Register 1  */
#define CK6408EVB_SPI_SPIENR    0x08    /* SPI Enable Register */
#define CK6408EVB_SPI_SER       0x10    /* Slave Enable Register */
#define CK6408EVB_SPI_BAUDR     0x14    /* Baud Rate Select */
#define CK6408EVB_SPI_TXFTLR    0x18    /* Transmit FIFO Threshold Level */
#define CK6408EVB_SPI_RXFTLR    0x1c    /* Receive FIFO Threshold Level */
#define CK6408EVB_SPI_TXFLR     0x20    /* Transmit FIFO Level Register */
#define CK6408EVB_SPI_RXFLR     0x24    /* Receive FIFO Level Register */
#define CK6408EVB_SPI_SR        0x28    /* Status Register */
#define CK6408EVB_SPI_IMR       0x2c    /* Interrupt Mask Register */
#define CK6408EVB_SPI_ISR       0x30    /* Interrupt Status Register */
#define CK6408EVB_SPI_RISR      0x34    /* Raw Interrupt Status Register */
#define CK6408EVB_SPI_TXOICR    0x38    /* Transmit FIFO Overflow Interrupt */
#define CK6408EVB_SPI_RXOICR    0x3c    /* Receive FIFO Overflow Interrupt */
#define CK6408EVB_SPI_RXUICR    0x40    /* Receive FIFO Underflow Interrupt */
#define CK6408EVB_SPI_MSTICR    0x44    /* MultiMaster Interrupt Clr Register*/
#define CK6408EVB_SPI_ICR       0x48    /* Interrupt Clear Register */
#define CK6408EVB_SPI_DMACR     0x4c    /* DMA Control Register */
#define CK6408EVB_SPI_DMATDLR   0x50    /* DMA Transmit Data Level */
#define CK6408EVB_SPI_DMARDLR   0x54    /* DMA Receive Data Level */
#define CK6408EVB_SPI_IDR       0x58    /* Identification Register */
#define CK6408EVB_SPI_DR        0x60    /* Data Register */
#define CK6408EVB_SPI_WR        0xc0    /* Receive or transmit mode */

/*
 *  Define bit flags in Control Register 0
 */
#define CK6408EVB_SPI_CLR0_TRM_MASK  (3 << 8)
#define CK6408EVB_SPI_CLR0_TMODRXTX  (0 << 8)  /* Transmit & Receive */
#define CK6408EVB_SPI_CLR0_TMODTX    (1 << 8)  /* Transmit Only */
#define CK6408EVB_SPI_CLR0_TMODRX    (2 << 8)  /* Receive Only */
#define CK6408EVB_SPI_CLR0_TMODEEEPROM   (3 << 8)  /* EEPROM mode */
#define CK6408EVB_SPI_CLR0_SCPOL     (1 << 7)  /* Serial Clock Polarity. */
#define CK6408EVB_SPI_CLR0_SCPH      (1 << 6)  /* Serial Clock Phase. */
#define CK6408EVB_SPI_CLR0_DFS8BIT   (7 << 0)  /* 8bit serial data transfer */
#define CK6408EVB_SPI_CLR0_DFS16BIT  (15 << 0) /* 16bit serial data transfer */

/*
 *  Define bit flags in SPI Enable Register 
 */
#define CK6408EVB_SPI_SPI_EN  1 /* SPI enable */
#define CK6408EVB_SPI_SPI_DIS 0 /* SPI disable */

#define CK6408EVB_SPI_FIFO_LV          0x02

/*
 *  Define bit flags in Slave Enable Register
 */
#define CK6408EVB_SPI_SENR  1 /* Slave Select Enable Flag. */
#define CK6408EVB_SPI_SDSR  0 /* Slave Select Disable Flag. */

#define CK6408EVB_SPI_SR_DCOL        (1 << 6)  /* Data Collision Error. */
#define CK6408EVB_SPI_SR_TXE         (1 << 5)  /* Transmission Error. */
#define CK6408EVB_SPI_SR_RFF         (1 << 4)  /* Receive FIFO Full. */
#define CK6408EVB_SPI_SR_RFNE        (1 << 3)  /* Receive FIFO Not Empty. */
#define CK6408EVB_SPI_SR_TFE         (1 << 2)  /* Transmit FIFO Empty. */
#define CK6408EVB_SPI_SR_TFNF        (1 << 1)  /* Transmit FIFO Not Full. */
#define CK6408EVB_SPI_SR_BUSY        (1 << 0)  /* SPI Busy Flag. */

#define CK6408EVB_SPI_INT_RR	(1 << 4) /* Receive data ready interrupt */
#define CK6408EVB_SPI_INT_TE	(1 << 0) /* Transmit FIFO empty interrupt */

/* spi_board_info.controller_data for SPI slave devices,
 * copied to spi_device.controller_data ... mostly for eeprom device.
 */
struct ck6408_spi_chip {
        u32 is_eeprom;
        u32 dma_mode;
        u32 timeout;
};

#endif	/* _CK6408EVB_SPI_H_  */
