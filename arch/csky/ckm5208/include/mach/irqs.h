/* arch/csky/ckm5208/include/mach/irqs.h
 *
 * Copyright (c) 20010 Simtec Electronics
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_MACH_IRQS_H
#define __ASM_MACH_IRQS_H

#define CSKY_GPIOA0_IRQ      0
#define CSKY_GPIOA1_IRQ      1
#define CSKY_GPIOA2_IRQ      2
#define CSKY_GPIOA3_IRQ      3
#define CSKY_GPIOA4_IRQ      4
#define CSKY_GPIOA5_IRQ      5
#define CSKY_GPIOA6_IRQ      6
#define CSKY_GPIOA7_IRQ      7
#define CSKY_TIMER0_IRQ      8
#define CSKY_TIMER1_IRQ      9
#define CSKY_RTC_IRQ         10
#define CSKY_WTD_IRQ         11
#define CSKY_SSI_IRQ         12
#define CSKY_IIC_IRQ         13
#define CSKY_UART0_IRQ       14
#define CSKY_UART1_IRQ       15
#define CSKY_DMAC_IRQ        16
#define CSKY_LCDC_IRQ        17
#define CSKY_MAC_IRQ         18
#define CSKY_POWM_IRQ        19
#define CSKY_SCI_IRQ         20
#define CSKY_UART2_IRQ       21
#define CSKY_UART3_IRQ       22
#define CSKY_TIMER2_IRQ      23
#define CSKY_TIMER3_IRQ      24
#define CSKY_TIMER4_IRQ      25
#define CSKY_TIMER5_IRQ      26
#define CSKY_USB_IRQ         28


#define NR_IRQS              32
#define FIQ_START            32  /* first vector number of fast interrupt */ 
#define NR_FIQS              32

#endif   /* __ASM_MACH_IRQS_H */
