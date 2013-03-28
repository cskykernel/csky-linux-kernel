/*
 *	csky.h -- CKcore CPU sepecific defines
 *
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

#ifndef	_ASM_CSKY_H_
#define	_ASM_CSKY_H_

#ifdef CONFIG_MMU

#define CK_RAM_BASE     CONFIG_RAM_BASE

#else /* !CONFIG_MMU */

#if defined(CONFIG_DMA_UNCACHED_16M)
 #define DMA_UNCACHED_REGION (16 * 1024 * 1024)
 #define DMA_MGU_SIZE  0x2f
#elif defined(CONFIG_DMA_UNCACHED_8M)
 #define DMA_UNCACHED_REGION (8 * 1024 * 1024)
 #define DMA_MGU_SIZE  0x2d
#elif defined(CONFIG_DMA_UNCACHED_4M)
 #define DMA_UNCACHED_REGION (4 * 1024 * 1024)
 #define DMA_MGU_SIZE  0x2b
#elif defined(CONFIG_DMA_UNCACHED_2M)
 #define DMA_UNCACHED_REGION (2 * 1024 * 1024)
 #define DMA_MGU_SIZE  0x29
#elif defined(CONFIG_DMA_UNCACHED_1M)
 #define DMA_UNCACHED_REGION (1024 * 1024)
 #define DMA_MGU_SIZE  0x27
#else
 #define DMA_UNCACHED_REGION (0)
 #define DMA_MGU_SIZE  0x0
#endif

#define CK_RAM_BASE CONFIG_RAM_BASE
#define CK_RAM_END	(CK_RAM_BASE + CONFIG_RAM_SIZE - DMA_UNCACHED_REGION) 
#define CK_UNCACHED_RAM_BASE  CK_RAM_END

#endif /*CONFIG_MMU*/

/*
 *	Define Board Bus clock frequency.
 */
#define CK_BUSCLK	CONFIG_APB_FREQ

/*
 *  Define the CPU Clock
 */
#define CK_CPU_CLOCK   CONFIG_CPU_CLOCK_FREQ

#endif	/* _ASM_CSKY_H_ */
