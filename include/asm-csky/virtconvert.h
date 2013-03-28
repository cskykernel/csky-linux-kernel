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
  
#ifndef __CSKY_VIRT_CONVERT__
#define __CSKY_VIRT_CONVERT__

/*
 * Macros used for converting between virtual and physical mappings.
 */

#ifdef __KERNEL__
#include <linux/compiler.h>
#include <linux/mmzone.h>
#include <asm/setup.h>
#include <asm/page.h>


static inline unsigned long mm_vtop(volatile void * address)
{
	return __pa(address);
}

static inline void * mm_ptov(unsigned long address)
{
	return (void *)__va(address);
}
static inline unsigned long virt_to_phys(volatile void * address)
{
	return (unsigned long)address - PAGE_OFFSET;
}

static inline void * phys_to_virt(unsigned long address)
{
	return (void *)(address + PAGE_OFFSET);
}

#define virt_to_bus virt_to_phys
#define bus_to_virt phys_to_virt

#define page_to_phys(page)      (page_to_pfn(page) << PAGE_SHIFT)

#endif

#endif /* __CSKY_VIRT_CONVERT__ */
