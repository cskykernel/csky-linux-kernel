/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  (C) Copyright 2011, C-SKY Microsystems Co., Ltd. (www.c-sky.com)
 *  Author: Hu junshan <junshan_hu@c-sky.com>
 *  
 */

#ifndef __ASM_CSKY_IRQFLAGS_H
#define __ASM_CSKY_IRQFLAGS_H

#include <asm/entry.h>

#ifdef __KERNEL__

/*
 * CPU interrupt mask handling.
 */

#define raw_local_irq_save(x)					\
	({							\
	__asm__ __volatile__(					\
	"mfcr %0, psr					\n"	\
	"psrclr ie"						\
	: "=r" (x) : :"memory" );				\
	})

#define raw_local_irq_enable()  __asm__("psrset ee, ie	\n" : : :"memory")
#define raw_local_irq_disable() __asm__("psrclr ie	\n" : : :"memory")

/*
 * Save the current interrupt enable state.
 */
#define raw_local_save_flags(x)					\
	({							\
	__asm__ __volatile__(					\
	"mfcr %0, psr			\n"			\
	: "=r" (x) : :"memory" );				\
	})

/*
 * restore saved IRQ state
 */
#define raw_local_irq_restore(x)				\
	__asm__ __volatile__(					\
	"mtcr %0, psr					\n"	\
	: : "r" (x) :"memory" )

static inline int raw_irqs_disabled_flags(unsigned long flags)
{
	return !((flags) & ALLOWINT); 
}

#endif
#endif
