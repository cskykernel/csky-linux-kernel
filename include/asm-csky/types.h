/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1994, 1995, 1996, 1999 by Ralf Baechle
 * Copyright (C) 1999 Silicon Graphics, Inc.
 * Copyright (C) 2004, Li Chunqiang (chunqiang_li@c-sky.com)
 * Copyright (C) 2009, Hu Junshan (junshan_hu@c-sky.com)
 * Copyright (C) 2009, C-SKY Microsystems Co., Ltd. (www.c-sky.com)
 */
#ifndef _ASM_TYPES_H
#define _ASM_TYPES_H

/*
 * We don't use int-l64.h for the kernel anymore but still use it for
 * userspace to avoid code changes.
 */
#define _CSKY_SZLONG 32
#include <asm-generic/int-ll64.h>

#ifndef __ASSEMBLY__

typedef unsigned short umode_t;

#endif /* __ASSEMBLY__ */

/*
 * These aren't exported outside the kernel to avoid name space clashes
 */
#ifdef __KERNEL__

#ifndef BITS_PER_LONG
#define BITS_PER_LONG _CSKY_SZLONG
#endif

#ifndef __ASSEMBLY__

#if (defined(CONFIG_HIGHMEM) && defined(CONFIG_64BIT_PHYS_ADDR)) \
    || defined(CONFIG_64BIT)
typedef u64 dma_addr_t;
#else
typedef u32 dma_addr_t;
#endif
typedef u64 dma64_addr_t;

/*
 * Don't use phys_t.  You've been warned.
 */
#ifdef CONFIG_64BIT_PHYS_ADDR
typedef unsigned long long phys_t;
#else
typedef unsigned long phys_t;
#endif

#endif /* __ASSEMBLY__ */

#endif /* __KERNEL__ */

#endif /* _ASM_TYPES_H */
