/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009  Hangzhou C-SKY Microsystems co.,ltd.
 */

#ifndef __ASM_CSKY_DIV64_H
#define __ASM_CSKY_DIV64_H

#if 0
#include <linux/types.h>

/* We're not 64-bit, but... */
/* n = n / base; return rem; */

/* Error!  */
#define do_div(n,base)                                          \
({                                                              \
        int __res;                                              \
        __res = ((unsigned long)n) % (unsigned int)base;        \
        n = ((unsigned long)n) / (unsigned int)base;            \
        __res;                                                  \
})
#else
#include <asm-generic/div64.h>
#endif

#endif /*__ASM_CSKY_DIV64_H*/
