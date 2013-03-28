/*
 *  linux/arch/csky/include/asm/cache.h
 *
 * Copyright (C) 2009  Hangzhou C-SKY Microsystems.
 * Copyright (C) 2006 by Li Chunqiang (chunqiang_li@c-sky.com)
 * Copyright (C) 2009 by Ye yun (yun_ye@c-sky.com) 
 */

#ifndef __ARCH_CSKY_CACHE_H
#define __ARCH_CSKY_CACHE_H

/* bytes per L1 cache line */
#define    L1_CACHE_SHIFT    4
/* this need to be at least 1 */
#define    L1_CACHE_BYTES    (1 << L1_CACHE_SHIFT)

#define __cacheline_aligned
#define ____cacheline_aligned

#define SMP_CACHE_BYTES		L1_CACHE_BYTES
#define ARCH_SLAB_MINALIGN 8

#endif  /* __ARCH_CSKY_CACHE_H */
