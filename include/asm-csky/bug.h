/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009  Hangzhou C-SKY Microsystems co.,ltd.
 */

#ifndef _CSKY_BUG_H
#define _CSKY_BUG_H

#ifdef CONFIG_BUG

#define BUG()                                                 \
	do {                                                      \
		printk("kernel BUG at %s:%d!\n", __FILE__, __LINE__); \
		__asm__ __volatile__("bkpt");                         \
	} while (0)
#endif /* CONFIG_BUG */

#define HAVE_ARCH_BUG

#include <asm-generic/bug.h>

#endif /* _CSKY_BUG_H */
