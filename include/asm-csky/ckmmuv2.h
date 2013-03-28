/*
 * linux/arch/csky/include/asm/ckmmuv2.h
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009  Hangzhou C-SKY Microsystems.
 * Copyright (C) 2006 by Li Chunqiang (chunqiang_li@c-sky.com)
 * Copyright (C) 2009 by Ye yun (yun_ye@c-sky.com) 
 */
#ifndef _ASM_CKMMUV2_H
#define _ASM_CKMMUV2_H

#include <linux/linkage.h>

#define CSKY_TLB_SIZE   128 
/*
 * The following macros are especially useful for __asm__
 * inline assembler.
 */
#ifndef __STR
#define __STR(x) #x
#endif
#ifndef STR
#define STR(x) __STR(x)
#endif

/*
 * Default page size for a given kernel configuration
 */
#define PM_DEFAULT_MASK	PM_4K
/*
 * Values used for computation of new tlb entries
 */
#define PL_4K		12
#define PL_16K		14
#define PL_64K		16
#define PL_256K		18
#define PL_1M		20
#define PL_4M		22
#define PL_16M		24
#define PL_64M		26
#define PL_256M		28

static inline int  read_mmu_index(void)
{
	int __res;
	__asm__ __volatile__("mfcr %0,cr<0, 15>\n\t"
					:"=r" (__res));
	return   __res;
}

static inline void  write_mmu_index(int value)
{	
	__asm__ __volatile__("mtcr %0,cr<0, 15>\n\t"
					: :"r" (value));
}

static inline int  read_mmu_entrylo0(void)
{
	int __res;
	__asm__ __volatile__("mfcr %0,cr<2, 15>\n\t"
					:"=r" (__res));
	return   __res;
}

static inline void  write_mmu_entrylo0(int value)
{	
	__asm__ __volatile__("mtcr %0,cr<2, 15>\n\t"
					: :"r" (value));
}

static inline int  read_mmu_entrylo1(void)
{
	int __res;

	__asm__ __volatile__("mfcr %0,cr<3, 15>\n\t"
					:"=r" (__res));
	return   __res;
}

static inline void  write_mmu_entrylo1(int value)
{	
	__asm__ __volatile__("mtcr %0,cr<3, 15>\n\t"
					: :"r" (value));
}

static inline int  read_mmu_pagemask(void)
{
	int __res;

	__asm__ __volatile__("mfcr %0,cr<6, 15>\n\t"
					:"=r" (__res));
	return   __res;
}

static inline void  write_mmu_pagemask(int value)
{	
	__asm__ __volatile__("mtcr %0,cr<6, 15>\n\t"
					: :"r" (value));
}

static inline int  read_mmu_entryhi(void)
{
	int __res;

	__asm__ __volatile__("mfcr %0,cr<4, 15>\n\t"
					:"=r" (__res));
	return   __res;
}

static inline void  write_mmu_entryhi(int value)
{	
	__asm__ __volatile__("mtcr %0,cr<4, 15>\n\t"
					: :"r" (value));
}

/*
 * TLB operations.
 */
static inline void tlb_probe(void)
{
	int value = 0x80000000;

	__asm__ __volatile__("mtcr %0,cr<8, 15>\n\t"
					: :"r" (value));
}

static inline void tlb_read(void)
{
	int value = 0x40000000;

	__asm__ __volatile__("mtcr %0,cr<8, 15>\n\t"
					: :"r" (value));
}

static inline void tlb_write_indexed(void)
{
	int value = 0x20000000;

	__asm__ __volatile__("mtcr %0,cr<8,15>\n\t"
					: :"r" (value));
}

static inline void tlb_write_random(void)
{
	int value = 0x10000000;

	__asm__ __volatile__("mtcr %0,cr<8, 15>\n\t"
					: :"r" (value));
}

static inline void tlb_invalid_all(void)
{
	int value = 0x04000000;

	__asm__ __volatile__("mtcr %0,cr<8, 15>\n\t"
					: :"r" (value));
}

#endif /* _ASM_CKMMUV2_H */

