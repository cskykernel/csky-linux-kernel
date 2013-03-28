/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Delay routines, using a pre-computed "loops_per_second" value.
*
 * Copyright (C) 1994 Hamish Macdonald
 * Copyright (C) 2009  Hangzhou C-SKY Microsystems co.,ltd.
 */

#ifndef _CSKY_DELAY_H
#define _CSKY_DELAY_H

#include <asm/param.h>

extern __inline__ void __delay(unsigned long loops)
{
	__asm__ __volatile__ (  "1: declt  %0\n\t"
                                "bf   1b"
                              : "=r" (loops)
	                          : "0" (loops) );
}

/*
 * Use only for very small delays ( < 1 msec).  Should probably use a
 * lookup table, really, as the multiplications take much too long with
 * short delays.  This is a "reasonable" implementation, though (and the
 * first constant multiplications gets optimized away if the delay is
 * a constant)
 */

extern unsigned long loops_per_jiffy;

extern __inline__ void udelay(unsigned long usecs)
{
	unsigned long loops_per_usec;

	loops_per_usec = (loops_per_jiffy * HZ) / 1000000;

	__delay(usecs * loops_per_usec);
}

#endif /* defined(_CSKY_DELAY_H) */
