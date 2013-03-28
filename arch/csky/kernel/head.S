/*
 * hrad.S -- startup code for csky eval board.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file README.legal in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009 Hangzhou C-SKY Microsystems co.,ltd.
 * Copyright (C) 2009 Hu junshan (junshan_hu@c-sky.com)
 *
 */

#include <linux/init.h>
#include <linux/threads.h>
#include <asm/setup.h>
#include <asm/csky.h>
#include <asm/page.h>
#include <asm/entry.h>
/*
 *	Board setup info.
 */
#define	PHY_MEM_BASE	CK_RAM_BASE	/* Memory base at address 0 */

//Virtual address variable
.export _ramvec
.export swapper_pg_dir
.export invalid_pte_table
.export bootloader_tags

.export _start
.export _stext

.data

_ramvec:
.long 0

bootloader_tags:
.long 0

.align 12
swapper_pg_dir:
.fill 4096
invalid_pte_table:
.fill 4096

.align 10
virtual_vec_base:
.fill 128, 4

__HEAD
/*
 * This is the codes first entry point. This is where it all
 * begins...
 */
_stext:
_start:
	.rept   8
	mov     r0, r0
	.endr
	/* Check if the boot loader actually provided a tag table */
	lrw     r1, ATAG_MAGIC
	cmpne   r1, r2
	bt      no_tag_table
	lrw     r2, bootloader_tags  /* Load address of bootloader_tags */
	// bclri   r2, 31               /* convert to PA, because MMU disable */
	bseti   r3, 31               /* convert to VA */
	stw     r3, (r2)             /* Save the tag table address for later use */

no_tag_table:
	/*
	 * Init psr value, enable exception, disable interrupt and fast interrupt.
	 * psr = 0x80000100
	 */
	bgeni   r1, 31
	bseti   r1, 8
	mtcr    r1, psr

	/*
	 * Enable internal cache.
	 */
#ifdef CONFIG_CSKY_INSTRUCTION_CACHE
	// Invalid instruction cache
	movi    r7, 0x11
	mtcr    r7, cr17
	// Enable instruction cache
	mfcr    r7, cr18
	bseti   r7, 2
	mtcr    r7, cr18
#endif

#ifdef CONFIG_CSKY_DATA_CACHE
	// Invalid Data cache
	movi    r7, 0x12
	mtcr    r7, cr17
	// Enable data cache
	mfcr    r7, cr18
	bseti   r7, 3
	mtcr    r7, cr18
#endif

	bseti   r7, 16   // invalid BHT
	bseti   r7, 17   // invalid BTB
	mtcr    r7, cr17

/*
 * Setup the initial page tables.  We only setup the barest
 * amount which are required to get the kernel running, which
 * generally means mapping in the kernel code.
 *
 * We only map in 2 * PAGE_SIZE of RAM, which should be sufficient in
 * all cases.
 *
 * r5, r6 = physical address of start RAM
 * r7 = Virtual address
 */

__create_page_table:
	/*
	 * Create identity mapping for first MB of kernel to
	 * cater for the MMU enable.  This identity mapping
	 * will be removed by paging_init()
	 */

	lrw     r8, PHY_MEM_BASE  //get physical address of start RAM
	movi    r2, 0
	movi    r5, 0x00000016 //C = 010, D = 1, V = 1, G = 0, pfn = 0x0
	movi    r6, 0x00000056 //C = 010, D = 1, V = 1, G = 0, pfn = 0x1
	movi    r7, 0x00000000 //VPN2 = 0x0, ASID = 0
	bmaski  r9, 13
	andn    r8, r9
	or      r7, r8          // VPN2 is bit 31-12 of phy addr
	lsri    r8, 6           // get bit 31-12 of phy addr
	or      r5, r8          // pfn
	or      r6, r8          // pfn

	// invalid mmu jtlb(ASID)
	bgeni   r1, 27
	WR_MCIR r1

	SET_CP_MMU
	WR_MPR  r2              // page mask = 4k
	WR_MEL0 r5
	WR_MEL1 r6
	WR_MEH  r7
	
	WR_HINT r2              // r2 = 0 use vpn[5:0] index

	// Initial TLB
	bgeni   r2, 28          // TLBWR
	WR_MCIR r2

	/*
	 * Enable  CPU MMU.
	 */
	mfcr    r7, cr18
	bseti   r7, 0
	bclri   r7, 1
	mtcr    r7, cr18
	
	/*
	 * Enable predict for jump, return stack, and cache write back.
	 */
	mfcr    r7, cr18
	bseti   r7, 11                 /* BTB */
	bseti   r7, 6                  /* branch prediction */
	bseti   r7, 5                  /* forecast jump  for ck610  */
	bseti   r7, 4                  /* write back for ck610  */
	mtcr    r7, cr18
	
	jsri __create_vector_table   /* P2V, jump from physic address to virtual address */

	/*
	 * Setup initial vector base table for interrupts and exceptions
	 */
__create_vector_table:
	lrw     r1, virtual_vec_base    /* Load pre-defined vbr */
	lrw     r2, _ramvec             /* Load address of _ramvec */
	stw     r1, (r2)                /* Write vbr setting to it */
	mtcr    r1, vbr                 /* Set vbr register with physical address */

	/*
	 * Zero the bss region.
	 */
	lrw     r1, _sbss               /* Get start of bss */
	lrw     r2, _ebss               /* Get end of bss */
	subu    r2, r1                  /* Calculate size of bss */
	lsri    r2, 2                   /* Size of whole words */

	movi    r3, 0                   /* Set zero value to write */
       
1:
	stw     r3, (r1)                /* Zero next word */
	addi    r1, 4                   /* Increase bss pointer */
	decne   r2                      /* Decrease counter */
	bt      1b                      /* Repeat for all bss */

	/*
	 * Load the current task stack.
	 */
	lrw     r1, init_thread_union   /* Get address of init_thread_union */
	lrw     r2, THREAD_SIZE         /* 8K memory, 2 page for stack */
	add     r1, r2                  /* Add 2 page offset to r1:
	                                          task struct pointer */
	mov     r0, r1                  /* Set current task stack pointer */

	/*
	 * Assember start up done, start code proper.
	 */
	jsri    start_kernel            /* Start Linux kernel */

1:
	br      1b                      /* Should never get here */

