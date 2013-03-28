/*
 * arch/csky/include/asm/mmu.h
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of
 * this archive for more details.
 *
 * (C) Copyright 2009, C-SKY Microsystems Co., Ltd. (www.c-sky.com)
 *  
 */

#ifndef __ASM_MMU_H
#define __ASM_MMU_H

#ifdef CONFIG_MMU
	typedef unsigned long mm_context_t;
#else
	typedef struct {
		unsigned long		end_brk;
	} mm_context_t;
#endif 

#endif /* __ASM_MMU_H */
