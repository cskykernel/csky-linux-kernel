/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009  Hangzhou C-SKY Microsystems co.,ltd.
 */

#ifndef _CSKY_DMA_H
#define _CSKY_DMA_H 
 
//#define	DMA_DEBUG	1

#include <asm/csky.h>
 
#define MAX_DMA_CHANNELS 8
 
/* under 2.4 it is actually needed by the new bootmem allocator */
#define MAX_DMA_ADDRESS PAGE_OFFSET

/* These are in kernel/dma.c: */
extern int request_dma(unsigned int dmanr, const char *device_id);
extern void free_dma(unsigned int dmanr);	/* release it again */
 
#endif /* _CSKY_DMA_H */
