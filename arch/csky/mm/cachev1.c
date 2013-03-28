/*
 * linux/arck/csky/mm/ckcache.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006 Hangzhou C-SKY Microsystems co.,ltd.
 * Copyright (C) 2006  Li Chunqiang (chunqiang_li@c-sky.com)
 * Copyright (C) 2009  Ye Yun (yun_ye@c-sky.com)
 */

#include <linux/mm.h>
#include <linux/sched.h>

void _flush_cache_all(void)
{
	int value = 0x33;
	__asm__ __volatile__("mtcr %0,cr17\n\t"
			     "sync\n\t"
			     : :"r" (value));
}

void ___flush_cache_all(void)
{
	int value = 0x33;
	__asm__ __volatile__("mtcr %0,cr17\n\t"
			     "sync\n\t"
			     : :"r" (value));
}

void _flush_cache_mm(struct mm_struct *mm)
{
	int value = 0x33;
	__asm__ __volatile__("mtcr %0,cr17\n\t"
			     "sync\n\t"
			     : :"r" (value));
}

void _flush_cache_page(struct vm_area_struct *vma, unsigned long page)
{
	int value = 0x33;
	__asm__ __volatile__("mtcr %0,cr17\n\t"
			     "sync\n\t"
			     : :"r" (value));
}

void _flush_icache_page(struct vm_area_struct *vma, struct page *page)
{
	int value = 0x11;
	
	__asm__ __volatile__("mtcr %0,cr17\n\t"
			     "sync\n\t"
			     : :"r" (value));
}

void _flush_cache_range(struct vm_area_struct *mm, unsigned long start, unsigned long end)
{
	int value = 0x33;
	__asm__ __volatile__("mtcr %0,cr17\n\t"
			     "sync\n\t"
			     : :"r" (value));
}

void _flush_cache_sigtramp(unsigned long addr)
{
	int value = 0x33;
	__asm__ __volatile__("mtcr %0,cr17\n\t"
			     "sync\n\t"
			     : :"r" (value));
}

void _flush_icache_all(void)
{
	int value = 0x11;
	
	__asm__ __volatile__("mtcr %0,cr17\n\t"
			     "sync\n\t"
			     : :"r" (value));
}

void _flush_dcache_page(struct page * page)
{ 
        int value = 0x32;
	
        __asm__ __volatile__("mtcr %0,cr17\n\t"
			     "sync\n\t"
                             : :"r" (value));
}

void _flush_dcache_all(void)
{ 
        int value = 0x32;
	
        __asm__ __volatile__("mtcr %0,cr17\n\t"
			     "sync\n\t"
                             : :"r" (value));
}

void _flush_icache_range(unsigned long start, unsigned long end)
{
	int value = 0x11;
	
	__asm__ __volatile__("mtcr %0,cr17\n\t"
			     "sync\n\t"
			     : :"r" (value));
}

void _clear_dcache_all(void)
{
        int value = 0x22;
	
        __asm__ __volatile__("mtcr %0,cr17\n\t"
			     "sync\n\t"
                             : :"r" (value));
}

void _clear_dcache_range(unsigned long start, unsigned long end)
{
	int value = 0x22;

	__asm__ __volatile__("mtcr %0,cr17\n\t"
			     "sync\n\t"
	                     : :"r" (value));
}
