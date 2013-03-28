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

//the value is physical address
.export	phy_ramstart
.export	phy_ramend

//Virtual address variable
.export _ramvec
.export _start
.export _stext
.export bootloader_tags

.data

/*
 *	Set up the usable of RAM stuff. Size of RAM is determined then
 *	an initial stack set up at the end.
 */
phy_rambase:
.long	0
phy_ramstart:
.long	0
phy_ramend:
.long	0
_ramvec:
.long	0

bootloader_tags:
.long   0

.align 10
virtual_vec_base:
.fill	128, 4

__HEAD
/*
 *	This is the codes first entry point. This is where it all
 *	begins...
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
	stw     r3, (r2)             /* Save the tag table address for later use */

no_tag_table:
	/*
     * Init psr value, enable exception, disable interrupt and fast interrupt.
     * psr = 0x80000100
     */
	bgeni   r1, 31
	bseti   r1, 8	
	mtcr    r1, psr                 /* Set psr register */

	/*
	 *	Enable internal cache.
	 */
#ifdef CONFIG_CSKY_INSTRUCTION_CACHE
	//Invalid instruction cache
	movi    r7, 0x11
	mtcr    r7, cr17
	//Enable instruction cache
	mfcr    r7, cr18
	bseti   r7, 2
	mtcr    r7, cr18
#endif

#ifdef CONFIG_CSKY_DATA_CACHE
	//Invalid Data cache
	movi    r7, 0x12
	mtcr    r7, cr17
	//Enable data cache
	mfcr    r7, cr18
	bseti   r7, 3
	mtcr    r7, cr18
#endif

	/* 
	 * Setup MGU.
	 * priority setting, ff -- read and write
	 */
	lrw	r7, 0xff02
	mtcr	r7, cr19

  	/* The first block */
	movi	r10, 0
	mtcr	r10, cr21
	lrw	r7, 0x3f /*baseaddr: 0x0; size: 4G*/
	mtcr	r7, cr20
	
	/* The second block */
	movi    r13, 1
	mtcr	r13, cr21
	lrw     r10, CONFIG_RAM_SIZE 
	ff1     r10
	rsubi   r10, 31                /* Get Size + 1 */
	bgenr   r11, r10
	subi    r11, 1
	lrw     r12, CK_RAM_BASE      /* Get base address of RAM */
	andn    r12, r11               /* Align by Size */ 
	subi    r10, 1                 /* Get Size */
	lsli    r10, 1               
	addi    r10, 1
	add     r12, r10
	mtcr	r12, cr20
	
	/* The third block */
	movi	r10, 2
	mtcr	r10, cr21
	lrw     r7, CK_UNCACHED_RAM_BASE 
	movi    r10, DMA_MGU_SIZE
	add     r7, r10
	mtcr	r7, cr20
	
	/*
	 * Enable MGU
	 */
	mfcr    r7, cr18
	bseti   r7, 0
	bclri   r7, 1
	mtcr    r7, cr18
	
	/*
	 * Setup physical memory layout variables. This is the way that
	 * the memory info is passed to the higher level kernel.
	 * Also set up an initial kernel stack at limit of memory.
	 */
	lrw     r1, CK_RAM_BASE         /* Load pre-defined base */
	lrw     r2, phy_ramstart        /* Load address of phy_rambase */
	stw     r1, (r2)                /* Write the ram base address to it */

	/*
	 *	Determine size of RAM, then set up initial stack.
	 */
	lrw     r1, CK_RAM_END          /* Load ram end */
	lrw     r2, phy_ramend          /* Load address of phy_ramend */
	stw     r1, (r2)    		    /* Set end ram addr */

	/*
	 * Enable predict for jump, return stack, and cache write back.
	 */
	mfcr    r7 , cr18
	bseti   r7 , 6
	bseti   r7 , 5                  /* forecast jump  for ck610  */
	bseti   r7 , 4                  /* write back for ck610  */	
	mtcr    r7 , cr18 
	
	/*
	 *	Setup initial vector base table for interrupts and exceptions
	 */
__create_vector_table:
	lrw     r1, virtual_vec_base    /* Load pre-defined vbr */
	lrw     r2, _ramvec             /* Load address of _ramvec */
	stw     r1, (r2)                /* Write vbr setting to it */
	mtcr    r1, vbr                 /* Set vbr register with physical address */



	/*
	 *	Zero out the bss region.
	 */
	lrw	    r1, _sbss               /* Get start of bss */
	lrw	    r2, _ebss               /* Get end of bss */
	subu    r2, r1                  /* Calculate size of bss */
	lsri    r2, 2                   /* Size of whole words */

	movi    r3, 0                   /* Set zero value to write */
       
1:
	stw     r3, (r1)                /* Zero next word */
	addi    r1, 4                   /* Increase bss pointer */
	decne   r2                      /* Decrease counter */
	bt      1b                      /* Repeat for all bss */

	/*
	 *	Load the current task stack.
	 */
	lrw     r1, init_thread_union   /* Get address of init_thread_union */
	lrw     r2, THREAD_SIZE         /* 8K memory, 2 page for stack */
	add     r1, r2                  /* Add 2 page offset to r1: 
	                                          task struct pointer */
	mov     r0, r1                  /* Set current task stack pointer */

	/*
	 *	Assember start up done, start code proper.
	 */
	jsri	start_kernel			/* Start Linux kernel */

1:
	br      1b				        /* Should never get here */

