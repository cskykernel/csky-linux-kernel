/*
 * linux/arch/csky/mm/mmu.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2011 C-SKY Microsystems co.,ltd.  All rights reserved.
 * Copyright (C) 2011 Hu Junshan(junshan_hu@c-sky.com)
 */
#include <linux/bug.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/types.h>
#include <linux/pagemap.h>
#include <linux/ptrace.h>
#include <linux/mman.h>
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/highmem.h>
#include <linux/swap.h>
#include <linux/proc_fs.h>
#include <linux/pfn.h>

#include <asm/setup.h>
#include <asm/cachectl.h>
#include <asm/dma.h>
#include <asm/kmap_types.h>
#include <asm/system.h>
#include <asm/pgtable.h>
#include <asm/pgalloc.h>
#include <asm/mmu_context.h>
#include <asm/sections.h>
#include <asm/tlb.h>
#include <asm/virtconvert.h>
#include <asm/csky.h>
#include <asm/fixmap.h>

DEFINE_PER_CPU(struct mmu_gather, mmu_gathers);
void pgd_init(unsigned long page)
{
	unsigned long *p = (unsigned long *) page;
	int i;

	for (i = 0; i < USER_PTRS_PER_PGD; i+=8) {
#ifdef CONFIG_MMU_HARD_REFILL
		/* hard refill need fill PA. */
		p[i + 0] = __pa(invalid_pte_table);
		p[i + 1] = __pa(invalid_pte_table);
		p[i + 2] = __pa(invalid_pte_table);
		p[i + 3] = __pa(invalid_pte_table);
		p[i + 4] = __pa(invalid_pte_table);
		p[i + 5] = __pa(invalid_pte_table);
		p[i + 6] = __pa(invalid_pte_table);
		p[i + 7] = __pa(invalid_pte_table);
#else
		p[i + 0] = (unsigned long) invalid_pte_table;
		p[i + 1] = (unsigned long) invalid_pte_table;
		p[i + 2] = (unsigned long) invalid_pte_table;
		p[i + 3] = (unsigned long) invalid_pte_table;
		p[i + 4] = (unsigned long) invalid_pte_table;
		p[i + 5] = (unsigned long) invalid_pte_table;
		p[i + 6] = (unsigned long) invalid_pte_table;
		p[i + 7] = (unsigned long) invalid_pte_table;
#endif
	}
#ifdef CONFIG_MMU_HARD_REFILL
	clear_dcache_range((unsigned long)p, USER_PTRS_PER_PGD);
#endif
}

static void __init fixrange_init (unsigned long start, unsigned long end, pgd_t *pgd_base)
{
#ifdef CONFIG_HIGHMEM
	pgd_t *pgd;
	pud_t *pud;
	pmd_t *pmd;
	pte_t *pte;
	int i, j, k;
	unsigned long vaddr;

	vaddr = start;
	i = __pgd_offset(vaddr);
	j = __pud_offset(vaddr);
	k = __pmd_offset(vaddr);
	pgd = pgd_base + i;

	for ( ; (i < PTRS_PER_PGD) && (vaddr != end); pgd++, i++) {
		pud = (pud_t *)pgd;
		for ( ; (j < PTRS_PER_PUD) && (vaddr != end); pud++, j++) {
			pmd = (pmd_t *)pud;
			for (; (k < PTRS_PER_PMD) && (vaddr != end); pmd++, k++) {
				if (pmd_none(*pmd)) {
					pte = (pte_t *) alloc_bootmem_low_pages(PAGE_SIZE);
#ifdef CONFIG_MMU_HARD_REFILL
/* hard refill need fill PA. */
					set_pmd(pmd, __pmd(__pa(pte)));
#else
					set_pmd(pmd, __pmd((unsigned long)pte));
#endif
					BUG_ON(pte != pte_offset_kernel(pmd, 0));
				}
				vaddr += PMD_SIZE;
			}
			k = 0;
		}
		j = 0;
	}
#endif
}

void __init pagetable_init(void)
{
	unsigned long vaddr;
	pgd_t *pgd_base;
#ifdef CONFIG_HIGHMEM
	pgd_t *pgd;
	pmd_t *pmd;
	pud_t *pud;
	pte_t *pte;
#endif

	memset((void*)0x80100000, 0, 1024*12);
	/* Initialize the entire pgd.  */
	pgd_init((unsigned long)swapper_pg_dir);
	pgd_init((unsigned long)swapper_pg_dir + sizeof(pgd_t ) * USER_PTRS_PER_PGD);

	pgd_base = swapper_pg_dir;

	/*
	 * Fixed mappings:
	 */
	vaddr = __fix_to_virt(__end_of_fixed_addresses - 1) & PMD_MASK;
	fixrange_init(vaddr, 0, pgd_base);

#ifdef CONFIG_HIGHMEM
	/*
	 * Permanent kmaps:
	 */
	vaddr = PKMAP_BASE;
	fixrange_init(vaddr, vaddr + PAGE_SIZE*LAST_PKMAP, pgd_base);

	pgd = swapper_pg_dir + __pgd_offset(vaddr);
	pud = pud_offset(pgd, vaddr);
	pmd = pmd_offset(pud, vaddr);
	pte = pte_offset_kernel(pmd, vaddr);
	pkmap_page_table = pte;
#endif
}

void __init paging_init(void)
{
	unsigned long max_zone_pfns[MAX_NR_ZONES];
	unsigned long lastpfn;
	
	pagetable_init();

#ifdef CONFIG_HIGHMEM
	kmap_init();
#endif

#ifdef CONFIG_ZONE_DMA
	max_zone_pfns[ZONE_DMA] = MAX_DMA_PFN;
#endif
#ifdef CONFIG_ZONE_DMA32
	max_zone_pfns[ZONE_DMA32] = MAX_DMA32_PFN;
#endif
	max_zone_pfns[ZONE_NORMAL] = max_low_pfn;
	lastpfn = max_low_pfn;
#ifdef CONFIG_HIGHMEM
	max_zone_pfns[ZONE_HIGHMEM] = highend_pfn;
	lastpfn = highend_pfn;
	
//	if (cpu_has_dc_aliases && max_low_pfn != highend_pfn) {
//	if (max_low_pfn != highend_pfn) {
//		printk(KERN_WARNING "This processor doesn't support highmem."
//		       " %ldk highmem ignored\n",
//		       (highend_pfn - max_low_pfn) << (PAGE_SHIFT - 10));
//		max_zone_pfns[ZONE_HIGHMEM] = max_low_pfn;
//		lastpfn = max_low_pfn;
//	}
#endif
	free_area_init_nodes(max_zone_pfns);
}

void __init setup_memory(void)
{
	/* nothing to do when cpu has mmu */
}
