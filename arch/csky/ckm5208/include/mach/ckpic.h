/*
 * ckpic.h -- Programmable Interrupt Controller support.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006  Hangzhou C-SKY Microsystems co.,ltd.
 * Copyright (C) 2006  Li Chunqiang (chunqiang_li@c-sky.com)
 * Copyright (C) 2009  Hu Junshan (junshan_hu@c-sky.com)
 */

#ifndef	_CSKY_PIC_H_
#define	_CSKY_PIC_H_

#include <asm/addrspace.h>
#include <mach/ck_iomap.h>

/*
 *	Define the base address of the C-SKY interrupt controller
 */
#ifdef CONFIG_MMU
#define CKPIC_BASE       KSEG1ADDR(CSKY_INT_PHYS)
#else
#define CKPIC_BASE       CSKY_INT_PHYS
#endif

/*
 *	Define the offset(Index) of the Programmable Interrupt Controller registers
 */
#define CKPIC_ICR        0x00     // PIC interrupt control register(High 16bits)
#define CKPIC_ISR        0x00     // PIC interrupt status register(Low 16bits)
#define CKPIC_IFR        0x02     // PIC interrupt force register
#define CKPIC_IPR        0x03     // PIC interrupt pending register
#define CKPIC_NIER       0x04     // PIC normal interrupt enable register
#define CKPIC_NIPR       0x05     // PIC normal interrupt pending register
#define CKPIC_FIER       0x06     // PIC fast interrupt enable register
#define CKPIC_FIPR       0x07     // PIC fast interrupt pending register

#define CKPIC_PR0        0x10     // PIC prior register 0(High-High 8 bit)
#define CKPIC_PR1        0x10     // PIC prior register 1(High-Low 8 bit)
#define CKPIC_PR2        0x10     // PIC prior register 2(Low-High 8 bit)
#define CKPIC_PR3        0x10     // PIC prior register 3(Low-Low 8 bit)
#define CKPIC_PR4        0x11     // PIC prior register 4(High-High 8 bit)
#define CKPIC_PR5        0x11     // PIC prior register 5(High-Low 8 bit)
#define CKPIC_PR6        0x11     // PIC prior register 6(Low-High 8 bit)
#define CKPIC_PR7        0x11     // PIC prior register 7(Low-Low 8 bit)
#define CKPIC_PR8        0x12     // PIC prior register 8(High-High 8 bit)
#define CKPIC_PR9        0x12     // PIC prior register 9(High-Low 8 bit)
#define CKPIC_PR10       0x12     // PIC prior register 10(Low-High 8 bit)
#define CKPIC_PR11       0x12     // PIC prior register 11(Low-Low 8 bit)
#define CKPIC_PR12       0x13     // PIC prior register 12(High-High 8 bit)
#define CKPIC_PR13       0x13     // PIC prior register 13(High-Low 8 bit)
#define CKPIC_PR14       0x13     // PIC prior register 14(Low-High 8 bit)
#define CKPIC_PR15       0x13     // PIC prior register 15(Low-Low 8 bit)
#define CKPIC_PR16       0x14     // PIC prior register 16(High-High 8 bit)
#define CKPIC_PR17       0x14     // PIC prior register 17(High-Low 8 bit)
#define CKPIC_PR18       0x14     // PIC prior register 18(Low-High 8 bit)
#define CKPIC_PR19       0x14     // PIC prior register 19(Low-Low 8 bit)
#define CKPIC_PR20       0x15     // PIC prior register 20(High-High 8 bit)
#define CKPIC_PR21       0x15     // PIC prior register 21(High-Low 8 bit)
#define CKPIC_PR22       0x15     // PIC prior register 22(Low-High 8 bit)
#define CKPIC_PR23       0x15     // PIC prior register 23(Low-Low 8 bit)
#define CKPIC_PR24       0x16     // PIC prior register 24(High-High 8 bit)
#define CKPIC_PR25       0x16     // PIC prior register 25(High-Low 8 bit)
#define CKPIC_PR26       0x16     // PIC prior register 26(Low-High 8 bit)
#define CKPIC_PR27       0x16     // PIC prior register 27(Low-Low 8 bit)
#define CKPIC_PR28       0x17     // PIC prior register 28(High-High 8 bit)
#define CKPIC_PR29       0x17     // PIC prior register 29(High-Low 8 bit)
#define CKPIC_PR30       0x17     // PIC prior register 30(Low-High 8 bit)
#define CKPIC_PR31       0x17     // PIC prior register 31(Low-Low 8 bit)

/*
 *  Bit Definition for the PIC Interrupt control register
 */
#define CKPIC_ICR_AVE    0x80000000 // F/N auto-interrupt requests enable
#define CKPIC_ICR_FVE    0x40000000 // F interrupt request to have vector number
#define CKPIC_ICR_ME     0x20000000 // Interrupt masking enable
#define	CKPIC_ICR_MFI	 0x10000000	// Masking of fast interrupt rquests enable

/*
 * Bit definition for the PIC Normal Interrup Enable register
 */
#define CKPIC_NIER_NIE(x)   (1 << x)   // Prioity=x normal interrupt enable bit

/*
 * Bit definition for the PIC Fast Interrup Enable register
 */
#define CKPIC_FIER_FIE(x)   (1 << x)   // Prioity=x Fast interrupt enable bit

#define csky_irq_wake NULL

#endif /* _CSKY_PIC_H_ */
