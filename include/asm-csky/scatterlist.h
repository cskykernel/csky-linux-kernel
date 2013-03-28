/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  (C) Copyright 2004, Li Chunqiang (chunqiang_li@c-sky.com)
 *  (C) Copyright 2009, Hu Junshan (junshan_hu@c-sky.com)
 *  (C) Copyright 2009, C-SKY Microsystems Co., Ltd. (www.c-sky.com)
 *
 */

#ifndef _CSKY_SCATTERLIST_H
#define _CSKY_SCATTERLIST_H

#include <linux/types.h>

struct scatterlist {
#ifdef CONFIG_DEBUG_SG
	unsigned long sg_magic;
#endif
	unsigned long page_link;
	unsigned int offset;
	unsigned int length;

	dma_addr_t dma_address;	/* A place to hang host-specific addresses at. */
};

/* This is bogus and should go away. */
#define ISA_DMA_THRESHOLD (0x00ffffff)

#define sg_dma_address(sg)	((sg)->dma_address)
#define sg_dma_len(sg)		((sg)->length)

#endif /* !(_CSKY_SCATTERLIST_H) */
