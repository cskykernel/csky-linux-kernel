/*
 * cktimer.h -- CKcore on board TIMER support defines.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * (C) Copyright 2009, Li Chunqiang (chunqiang_li@c-sky.com)
 * (C) Copyright 2004, Kang Sun (sunk@vlsi.zju.edu.cn)
 * (C) Copyright 1999-2003, Greg Ungerer (gerg@snapgear.com)
 * (C) Copyright 2000, Lineo Inc. (www.lineo.com) 
 */
#ifndef	_CSKY_TIMER_H
#define	_CSKY_TIMER_H

#include <asm/addrspace.h>
#include <mach/ck_iomap.h>

/*
 *  Define the Timer Control Register base address.
 */
#ifdef CONFIG_MMU
#define CKTIMER_BASE          KSEG1ADDR(CSKY_TIMER_PHYS)
#else
#define CKTIMER_BASE          CSKY_TIMER_PHYS
#endif

/*
 *  Define the offset(index) in CKTIMER_BASE for the registers
 *  addresses of the timer
 */
#define CKTIMER_TCN1_LDCR     0x00    // Timer1 Load Count register
#define CKTIMER_TCN1_CVR      0x01    // Timer1 Current Value register
#define CKTIMER_TCN1_CR       0x02    // Timer1 Control register
#define CKTIMER_TCN1_EOI      0x03    // Timer1 Interrupt clear register
#define CKTIMER_TCN1_ISR      0x04    // Timer1 Interrupt status.

#define CKTIMER_TCN2_LDCR     0x05    // Timer2 Load Count register
#define CKTIMER_TCN2_CVR      0x06    // Timer2 Current Value register
#define CKTIMER_TCN2_CR       0x07    // Timer2 Control register
#define CKTIMER_TCN2_EOI      0x08    // Timer2 Interrupt clear register
#define CKTIMER_TCN2_ISR      0x09    // Timer2 Interrupt status.

#define CKTIMER_SYS_ISR       0x28    // Interrupts status of all timers
#define CKTIMER_SYS_EOI       0x29    // Read it to clear all active interrupts
#define CKTIMER_SYS_RISR      0x2a    // Unmasked interrupts status register

/*
 *  Bit definitions for the Timer Control Register (Timer CR).
 */
#define CKTIMER_TCR_RCS       0x00000008    // Timer Reference Clock selection
#define CKTIMER_TCR_IM        0x00000004    // Timer Interrupt mask
#define CKTIMER_TCR_MS        0x00000002    // Timer Mode select
#define CKTIMER_TCR_EN        0x00000001    // Timer Enable select

#endif	/* _CSKY_TIMER_H */
