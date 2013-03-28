/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006 Hangzhou C-SKY Microsystems co.,ltd.
 * Copyright (C) 2006  Li Chunqiang (chunqiang_li@c-sky.com)
 * Copyright (C) 2009  Ye Yun (yun_ye@c-sky.com)
 */
#ifndef __ASM_CACHEFLUSH_MM_H
#define __ASM_CACHEFLUSH_MM_H

#include <linux/compiler.h>
#include <asm/string.h>

/* 
 * Cache flushing:
 *  - flush_cache_all() flushes entire cache
 *  - flush_cache_mm(mm) flushes the specified mm context's cache lines
 *  - flush_cache_page(mm, vmaddr) flushes a single page
 *  - flush_cache_range(mm, start, end) flushes a range of pages
 *  - flush_page_to_ram(page) write back kernel page to ram
 *  - flush_icache_range(start, end) flush a range of instructions
 *
 * CK640 specific flush operations:
 *
 *  - flush_cache_sigtramp() flush signal trampoline
 *  - flush_icache_all() flush the entire instruction cache
 */
struct mm_struct;
struct vm_area_struct;
struct page;

extern void _flush_cache_all(void);
extern void _flush_cache_mm(struct mm_struct *mm);
extern void _flush_cache_range(struct vm_area_struct *mm, 
                           unsigned long start, unsigned long end);
extern void _flush_cache_page(struct vm_area_struct *vma, unsigned long page);
extern void _flush_dcache_page(struct page * page);
extern void _flush_icache_range(unsigned long start, unsigned long end);
extern void _flush_icache_page(struct vm_area_struct *vma, struct page *page);

extern void _flush_cache_sigtramp(unsigned long addr);
extern void _flush_icache_all(void);
extern void _flush_dcache_all(void);
extern void _clear_dcache_all(void);
extern void _clear_dcache_range(unsigned long start, unsigned long end);
extern void flush_dcache_page(struct page *);

#define flush_cache_all()		_flush_cache_all()
#define flush_cache_mm(mm)		_flush_cache_mm(mm)
#define flush_cache_range(mm,start,end)	_flush_cache_range(mm,start,end)
#define flush_cache_page(vma,page,pfn)	_flush_cache_page(vma, page)
#define flush_page_to_ram(page)		do { } while (0)

#ifdef  CONFIG_MMU
#define flush_icache_range(start, end)	_flush_icache_range(start,end)
#else
#define flush_icache_range(start, end)  _flush_cache_all()
#endif
#define flush_icache_user_range(vma, page, addr, len) \
					_flush_icache_page((vma), (page))
#define flush_icache_page(vma, page) 	_flush_icache_page(vma, page)

#define flush_icache_all()		_flush_icache_all()
#define flush_dcache_all()		_flush_dcache_all()
#define clear_dcache_all()		_clear_dcache_all()
#define clear_dcache_range(start, end)  _clear_dcache_range(start,end)

#define flush_cache_dup_mm(mm)                  flush_cache_mm(mm) 
#define flush_dcache_mmap_lock(mapping)         do { } while (0)
#define flush_dcache_mmap_unlock(mapping)       do { } while (0)

#define flush_cache_vmap(start, end)            flush_cache_all()
#define flush_cache_vunmap(start, end)          flush_cache_all()

static inline void copy_to_user_page(struct vm_area_struct *vma,
                                     struct page *page, unsigned long vaddr,
                                     void *dst, void *src, int len)
{
	flush_cache_page(vma, vaddr, page_to_pfn(page));
	memcpy(dst, src, len);
	flush_icache_user_range(vma, page, vaddr, len);
}
static inline void copy_from_user_page(struct vm_area_struct *vma,
                                       struct page *page, unsigned long vaddr,
                                       void *dst, void *src, int len)
{
	flush_cache_page(vma, vaddr, page_to_pfn(page));
	memcpy(dst, src, len);
}

#endif /* __ASM_CACHEFLUSH_MM_H */
