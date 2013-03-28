/*
 *	ckuart.h -- for CK5208 Board internal UART support defines.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  (C) Copyright 2011, C-SKY Microsystems Co., Ltd. (www.c-sky.com) 
 */

#ifndef _CKM5208_CKUART_H_
#define	_CKM5208_CKUART_H_

#include <asm/addrspace.h>
#include <mach/ck_iomap.h>
#include <mach/irqs.h>

/*
 *      Define the base address of the UARTS within the MBAR address
 *      space.
 */
#ifdef CONFIG_MMU
#define CSKY_UART_BASE0  KSEG1ADDR(CSKY_UART0_PHYS) /* Base address of UART0 */
#define CSKY_UART_BASE1  KSEG1ADDR(CSKY_UART1_PHYS) /* Base address of UART1 */
#define CSKY_UART_BASE2  KSEG1ADDR(CSKY_UART2_PHYS) /* Base address of UART2 */
#else
#define CSKY_UART_BASE0  (CSKY_UART0_PHYS) /* Base address of UART0 */
#define CSKY_UART_BASE1  (CSKY_UART1_PHYS) /* Base address of UART1 */
#define CSKY_UART_BASE2  (CSKY_UART2_PHYS) /* Base address of UART2 */
#endif
/*
 *  Define the CKM5208 Board UART register set addresses.
 *	Access as array with index.
 */
#define CSKY_UART_RBR    0x00    /* Receive Buffer Register (32 bits, R) */
#define CSKY_UART_THR    0x00    /* Transmit Holding Register (32 bits, W) */
#define CSKY_UART_DLL    0x00    /* Divisor Latch(Low)  (32 bits, R/W) */
#define CSKY_UART_IER    0x01    /* Interrupt Enable Register (32 bits, R/W) */
#define CSKY_UART_DLH    0x01    /* Divisor Latch(High) (32 bits, R/W) */
#define CSKY_UART_IIR    0x02    /* Interrupt Identity Register (32 bits, R) */
#define CSKY_UART_FCR    0x02    /* fifo Countrol Register (32 bits, W) */
#define CSKY_UART_LCR    0x03    /* Line Control Register (32 bits, R/W) */
#define CSKY_UART_MCR    0x04    /* Modem Control Register (32 bits, W) */
#define CSKY_UART_LSR    0x05    /* Line Status Register (32 bits, R) */
#define CSKY_UART_MSR    0x06    /* Modem Status Register (32 bits, R/W) */
#define CSKY_UART_USR    0x1f    /* UART Status Register (32 bits, R/W) */

/*
 *  Define bit flags in Interrupt Enable Register (IER)
 */
#define CSKY_UART_IER_PTIME  0x80  /* En the generation of THRE Interrupt */
#define CSKY_UART_IER_EDSSI  0x08  /* En Modem Status Interrupt */
#define CSKY_UART_IER_ELSI   0x04  /* En Receiver Line Status Interrupt */
#define CSKY_UART_IER_ETHEI  0x02  /* En Transmitter Hold Reg Empty Interrupt */
#define CSKY_UART_IER_ERDAI  0x01  /* En Received Data Available Interrupt */
#define CSKY_UART_IER_ETBEI  0x02  /* En Transmitter Hold Reg Empty Interrupt */
#define CSKY_UART_IER_ERBFI  0x01  /* En Received Data Available Interrupt */

/*
 *  Define bit flags in Interrupt Identity Register (IIR)
 */
#define CSKY_UART_IIR_FIFODIS  0x00    /* FIFO disable(7:6 bits) */
#define CSKY_UART_IIR_FIFOEN   0xc0    /* FIFO enable(7:6 bits) */

/* 
 * Interrupt ID (3:0 bits) 
 * 0000 = modem status 
 * 0001 = no interrupt pending 
 * 0010 = THR empty 
 * 0100 = received data available 
 * 0110 = receiver line status 
 * 0111 = busy detect 
 * 1100 = character timeouti
 */
#define CSKY_UART_IIR_NONE      0x01   /* no interrupt pending */
#define CSKY_UART_IIR_RLS       0x06   /* receiver line status interrupt */
#define CSKY_UART_IIR_RDA       0x04   /* Receive data available interrupt */
#define CSKY_UART_IIR_CTOUT     0x0c   /* character time out interrupt */
#define CSKY_UART_IIR_THRE      0x02   /* transmitter holding register empty */
#define CSKY_UART_IIR_MS        0x00   /* modem status interrupt */

/*
 *  Define bit flags FIFO Control Register (FCR)
 */
#define CSKY_UART_FCR_RT0       0x00   /* 0x00 = 1 character in the FIFO, trig 
											receive interrupt(RCVR trig) */
#define CSKY_UART_FCR_RT1       0x40   /* 0x01 = FIFO 1/4 full, trig receive 
											interrupt */
#define CSKY_UART_FCR_RT2       0x80   /* 0x10 = FIFO 1/2 full, trig receive 
											interrupt */
#define CSKY_UART_FCR_RT3       0xc0   /* 0x11 = FIFO 2 less than full, trig the
											receive interrupt */
#define CSKY_UART_FCR_TET0      0x00   /* 0x00 = FIFO empty, trig THRE interrupt											(TX empty trigger) */
#define CSKY_UART_FCR_TET1      0x10   /* 0x01 = 2 characters in the FIFO, trigI
											THRE interrupt */
#define CSKY_UART_FCR_TET2      0x20   /* 0x10 = FIFO 1/4 full, trig THRE 
											interrupt */
#define CSKY_UART_FCR_TET3      0x30   /* 0x11 = FIFO 1/2 full, trig THRE 
											interrupt */
#define CSKY_UART_FCR_DMAM      0x08   /* DMA mode */
#define CSKY_UART_FCR_XFIFOR    0x04   /* XMIT FIFO reset */
#define CSKY_UART_FCR_RFIFOR    0x02   /* RCVR FIFO reset */
#define CSKY_UART_FCR_FIFOE     0x01   /* FIFO enable */

/*
 *  Define bit flags in Line Control Register (LCR).
 */
#define CSKY_UART_LCR_DLAEN     0x80    /* Divisor Latch Access bit */
#define CSKY_UART_LCR_BC        0x40    /* Break Control */
#define CSKY_UART_LCR_SP        0x20    /* Stick Parity */
#define CSKY_UART_LCR_ESP       0x10    /* Even Parity */
#define CSKY_UART_LCR_PEN       0x08    /* Parity Enable */
#define CSKY_UART_LCR_STOP      0x04    /* Specify the number of generated stop
											 bits */
#define CSKY_UART_LCR_WLEN5     0x00    /* Data Word Length (5 bits) */
#define CSKY_UART_LCR_WLEN6     0x01    /* Data Word Length (6 bits) */
#define CSKY_UART_LCR_WLEN7     0x02    /* Data Word Length (7 bits) */
#define CSKY_UART_LCR_WLEN8     0x03    /* Data Word Length (8 bits) */

/*
 *  Define bit flags in the Line Status Register (LSR)
 */
#define CSKY_UART_LSR_FIFOERR   0x80    /* Receive FIFO Error */
#define CSKY_UART_LSR_TEMT      0x40    /* Transmitter Empty */
#define CSKY_UART_LSR_THRE      0x20    /* Transmit Holding Register Empty */
#define CSKY_UART_LSR_BI        0x10    /* Break Interrupt */
#define CSKY_UART_LSR_FERR      0x08    /* Frame Error */
#define CSKY_UART_LSR_PERR      0x04    /* Parity Error */
#define CSKY_UART_LSR_OVRERR    0x02    /* Overrun Error */
#define CSKY_UART_LSR_DR        0x01    /* Data Ready */

#define CSKY_UART_LSR_RXERR (CSKY_UART_SR_FERR | CSKY_UART_SR_OVRERR \
								| CSKY_UART_LSR_PERR)

/*
 *  Define  bit flags in the Modem Control register (MCR)
 */
#define CSKY_UART_MCR_SIRE      0x40    /* SIR Mode Enable */
#define CSKY_UART_MCR_AFCE      0x20    /* Auto Flow Control Enable */
#define CSKY_UART_MCR_LB        0x10    /* Loopback */
#define CSKY_UART_MCR_OUT2      0x08    /* Auxiliary output 2 */
#define CSKY_UART_MCR_OUT1      0x04    /* Auxiliary ouput 1 */
#define CSKY_UART_MCR_RTS       0x02    /* Request to send */
#define CSKY_UART_MCR_DTR       0x01    /* Data Terminal Ready */

/*
 *  Define bit flags in the Modem Status Register (MSR)
 */
#define CSKY_UART_MSR_DCD       0x80    /* Data Carrier Detect */
#define CSKY_UART_MSR_RI        0x40    /* Ring In */
#define CSKY_UART_MSR_DSR       0x20    /* Data Set Ready */
#define CSKY_UART_MSR_CTS       0x10    /* Clear To Send*/
#define CSKY_UART_MSR_DDCD      0x08    /* Delt Carrier Detect */
#define CSKY_UART_MSR_TERI      0x04    /* Trailing Edge Ring Indicator */
#define CSKY_UART_MSR_DDSR      0x02    /* Delta Data Set Ready */
#define CSKY_UART_MSR_DCTS      0x01    /* Delta Clear to send */

/*
 *  Define bit flags in the UART Status Register (USR)
 */
#define CSKY_UART_USR_RFF       0x10    /* Receive FIFO full */
#define CSKY_UART_USR_RFNE      0x08    /* Receive FIFO not empty */
#define CSKY_UART_USR_TFE       0x04    /* Transmit FIFO empty */
#define CSKY_UART_USR_TFNE      0x02    /* Transmit FIFO not full */
#define CSKY_UART_USR_BUSY      0x01    /* UART busy */

#define CSKY_UART_DIV           0x04    /* Baud divisor(Right shift) */

#define CSKY_MAX_UART	4
extern struct platform_device *csky_default_console_device;
/*
 *  The base number of IRQ interrupt for C-SKY UART.
 */
#define IRQBASE          CSKY_UART0_IRQ 

#endif	/* _CKM5208_CKUART_H_  */
