/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  (C) Copyright 2004, Li Chunqiang (chunqiang_li@c-sky.com)
 *  (C) Copyright 2009, Hu Junshan (junshan_hu@c-sky.com)
 *  (C) Copyright 2011, Dou Shaobin (shaobin_dou@c-sky.com)
 *  (C) Copyright 2011, C-SKY Microsystems Co., Ltd. (www.c-sky.com)
 *  
 */ 
 
#ifndef _CSKY_TLB_H
#define _CSKY_TLB_H

#include <asm/cacheflush.h>

/*
 * csky doesn't need any special per-pte or per-vma handling, except
 * we need to flush cache for area to be unmapped.
 */
#ifdef CONFIG_MMU
#define tlb_start_vma(tlb, vma)                 \
    do {                            \
        if (!tlb->fullmm)               \
            flush_cache_range(vma, vma->vm_start, vma->vm_end); \
    }  while (0)

#ifdef CONFIG_MMU_HARD_REFILL
/*
 * FIXME: may be use function flush_tlb_range like other arch.
 */
#define tlb_end_vma(tlb, vma)       \
    do {                            \
        if (!tlb->fullmm)               \
            clear_dcache_range(vma->vm_start, vma->vm_end - vma->vm_start); \
    }  while (0)
#else
#define tlb_end_vma(tlb, vma)	                    do { } while (0)
#endif
#define __tlb_remove_tlb_entry(tlb, ptep, address)	do { } while (0)

/*
 * .. because we flush the whole mm when it
 * fills up.
 */
#define tlb_flush(tlb)		flush_tlb_mm((tlb)->mm)

#include <asm-generic/tlb.h>

#else /*CONFIG_MMU*/
#define tlb_flush(tlb)	((void) tlb) 
#endif

#endif /* _CSKY_TLB_H */
