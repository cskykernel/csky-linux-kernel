/*
 *
 * linux/arch/csky/entry.S
 *
 * entry.S  contains the system-call and fault low-level handling routines.
 * This also contains the timer-interrupt handler, as well as all interrupts
 * and faults that can result in a task-switch.
 *
 * NOTE: This code handles signal-recognition, which happens every time
 * after a timer-interrupt and after each system call.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file README.legal in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009 Hangzhou C-SKY Microsystems co.,ltd.
 *
 */

#include <linux/linkage.h>
#include <asm/entry.h>
#include <asm/errno.h>
#include <asm/segment.h>
#include <asm/traps.h>
#include <asm/pgtable-bits.h>
#include <asm/unistd.h>
#include <asm/asm-offsets.h>
#include <linux/threads.h>
#include <asm/csky.h>
#include <asm/thread_info.h>

#define PTE_HALF        0
#define PTE_SIZE        4
#define PTE_BIT         2
#define PTEP_INDX_MSK	0xff8
#define PTE_INDX_MSK    0xffc
#define PTE_INDX_SHIFT  10
#define _PGDIR_SHIFT    22
#define THREADSIZE_MASK_BIT 13

.export system_call
.export buserr
.export trap
.export alignment
.export inthandler
.export autohandler
.export fasthandler

.export fastautohandler
.export resume, ret_from_exception
.export ret_from_signal
.export sys_fork, sys_clone
.export sys_sigsuspend, sys_sigreturn
.export sw_usp
.export sw_ksp

.export handle_tlbinvalidl
.export handle_tlbmodified
.export handle_tlbmissinst
.export handle_tlbmissdata
.export tlbinvalidl
.export tlbinvalids
.export tlbmiss
.export readtlbinvalid
.export writetlbinvalid
.export handle_fpe

.import irq_stat
.export sys_call_table

#ifndef CONFIG_MMU_HARD_REFILL
.import pgd_current
#endif

.data
sw_ksp:
.long 0
sw_usp:
.long 0

.text

/*
 * Tlbinvalidl exception handle routine.
 */
#if defined(CONFIG_MMU)
ENTRY(handle_tlbinvalidl)
tlbinvalidl:
    mtcr    r5, ss2
    mtcr    r8, ss3
    mtcr    r4, ss4
#ifdef CONFIG_MMU_HARD_REFILL
    mfcr    r8, cr<29, 15>
    bclri   r8, 0
    bseti   r8, 31
#else
    lrw     r8, (pgd_current)
    ldw     r8, (r8)
#endif
    SET_CP_MMU
    RD_MEH  r5
    mov     r4, r5
    lsri    r4, _PGDIR_SHIFT
    lsli    r4, 2
    addu    r8, r4
    ldw     r8, (r8)
#ifdef CONFIG_MMU_HARD_REFILL
    bseti   r8, 31
#endif

    lsri    r5, PTE_INDX_SHIFT
    lrw     r4, PTE_INDX_MSK
    and     r5, r4
    addu    r8, r5
    ldw     r5, (r8)
    bgeni   r4, 31            /* move 0x80000000 to r4 */
    WR_MCIR r4
    movi    r4, (_PAGE_PRESENT | _PAGE_READ)
    and     r5, r4
    cmpne   r5, r4
    bt      readtlbinvalid   /* PTE not present, jump to fix it. */

    /* PTE present, now make it valid */
    ldw     r5, (r8)
#ifdef CONFIG_CPU_MMU_V1
    bgeni   r4, 7         /* r4 = (_PAGE_VALID | _PAGE_ACCESSED) */
    bseti   r4, 3
#else
    movi    r4, (_PAGE_VALID | _PAGE_ACCESSED)
#endif
    or      r5, r4
    stw     r5, (r8)

    /*
     * Below, fill a jTLB with two PTEs of which we have set one above.
     * When do this, we make sure set Entrylo0 with the low PTE in Page
     * Table, and Entrylo1 with the high one.
     */
    bclri   r8, PTE_BIT
#ifdef CONFIG_CPU_MMU_V1
    ldw     r4, (r8, 4)
    lsri    r4, 6
    WR_MEL1 r4
    ldw     r4, (r8)
    lsri    r4, 6
    WR_MEL0 r4
#else
    ldw     r4, (r8, 4)
    WR_MEL1 r4
    ldw     r4, (r8)
    WR_MEL0 r4
#endif

    bgeni   r4, 29
    WR_MCIR r4
    mfcr    r5, ss2
    mfcr    r8, ss3
    mfcr    r4, ss4
    rte

readtlbinvalid:
    mfcr    r5, ss2
    mfcr    r8, ss3
    mfcr    r4, ss4
    SAVE_ALL

    SET_CP_MMU
    RD_MEH  r5
    bmaski  r8, 12
    andn    r5, r8             /* r8 = !(0xfffff000) */
    mov     r4, r5
    psrset  ee, ie          /* Enable exception & interrupt */
    mov     r2, r0
    movi    r3, 0
    jsri    do_page_fault
    jmpi    ret_from_exception

/*
 * Tlbinvalids exception handle routine.
 */
ENTRY(handle_tlbinvalids)
tlbinvalids:
    mtcr    r5, ss2
    mtcr    r8, ss3
    mtcr    r4, ss4
#ifdef CONFIG_MMU_HARD_REFILL
    mfcr    r8, cr<29, 15>
    bclri   r8, 0
    bseti   r8, 31
#else
    lrw r8, pgd_current
    ldw     r8, (r8)
#endif

    SET_CP_MMU
    RD_MEH  r5
    mov     r4, r5
    lsri    r4, _PGDIR_SHIFT
    lsli    r4, 2
    addu    r8, r4
    ldw     r8, (r8)
#ifdef CONFIG_MMU_HARD_REFILL
    bseti   r8, 31
#endif

    lsri    r5, PTE_INDX_SHIFT
    lrw     r4, PTE_INDX_MSK
    and     r5, r4
    addu    r8, r5
    ldw     r5, (r8)
    bgeni   r4, 31           /* TLB probe command, r4 = 0x80000000 */
    WR_MCIR r4
    movi    r4, (_PAGE_PRESENT | _PAGE_WRITE)
    and     r5, r4
    xor     r5, r4
    cmpnei  r5, 0
    bt      writetlbinvalid  /* PTE not present, jump to fix it. */

    /* PTE resent, set it to be valid. */
    ldw     r5, (r8)

#ifdef CONFIG_CPU_MMU_V1
    /* r4 = (_PAGE_ACCESSED | _PAGE_MODIFIED | _PAGE_VALID | _PAGE_DIRTY) */
    movi    r4, 0x18
    bseti   r4, 7
    bseti   r4, 8
#else
    movi   r4, (_PAGE_ACCESSED | _PAGE_MODIFIED | _PAGE_VALID | _PAGE_DIRTY)
#endif

    or      r5, r4
    stw     r5, (r8)
    /*
     * Below, fill a jTLB with two PTEs of which we have set one above.
     * When do this, we make sure set Entrylo0 with the low PTE in Page
     * Table, and Entrylo1 with the high one.
     */
    bclri   r8, PTE_BIT
#ifdef CONFIG_CPU_MMU_V1
    ldw     r4, (r8,4)
    lsri    r4, 6
    WR_MEL1 r4
    ldw     r4, (r8)
    lsri    r4, 6
    WR_MEL0 r4
#else
    ldw     r4, (r8,4)
    WR_MEL1 r4
    ldw     r4, (r8)
    WR_MEL0 r4
#endif

    bgeni   r4, 29
    WR_MCIR r4

    mfcr    r5, ss2
    mfcr    r8, ss3
    mfcr    r4, ss4
    rte

writetlbinvalid:
    mfcr    r5, ss2
    mfcr    r8, ss3
    mfcr    r4, ss4
    SAVE_ALL

    SET_CP_MMU
    RD_MEH  r5
    bmaski  r8, 12
    andn    r5, r8          /* r8 = !(0xfffff000) */
    mov     r4, r5
    psrset  ee, ie          /* Enable exception & interrupt */
    mov     r2, r0
    movi    r3, 1
    jsri    (do_page_fault)
    jmpi    (ret_from_exception)

/*
 * Tlbmiss exception handle routine.
 */
ENTRY(handle_tlbmiss)
tlbmiss:
    lrw     r2, (pgd_current)
    ldw     r2, (r2)

    SET_CP_MMU
    RD_MEH      r3
#ifdef CONFIG_CPU_MMU_V1
    mov     r4, r3
    lsri    r4, _PGDIR_SHIFT
    ixw       r2, r4
    ldw       r2, (r2)

    lsri    r3, PTE_INDX_SHIFT
    lrw     r4, PTE_INDX_MSK
    and     r3, r4
    addu    r2, r3
    bclri   r2, PTE_BIT
    ldw       r3, (r2)
    lsri      r3, 6
    WR_MEL0 r3
    ldw       r3, (r2, 4)
    lsri      r3, 6
    WR_MEL1 r3
#else
    lsri	r4, r3, _PGDIR_SHIFT
    ixw		r2, r4
    ldw		r2, (r2)

    zext	r3, r3, 21, 12
    ixw		r2, r3
    bclri   r2, PTE_BIT
    ldw     r3, (r2)
    WR_MEL0 r3
    ldw     r3, (r2, 4)
    WR_MEL1 r3
#endif

    bgeni   r3, 28           /* TLB write random command, r5 = 0x10000000 */
    WR_MCIR r3
    rte

/*
 * Tlbmodified exception handle routine.
 */
ENTRY(handle_tlbmodified)
    mtcr    r5, ss2
    mtcr    r8, ss3
    mtcr    r4, ss4
#ifdef CONFIG_MMU_HARD_REFILL
    mfcr    r8, cr<29, 15>
    bclri   r8, 0
    bseti   r8, 31
#else
    lrw     r8, (pgd_current)
    ldw     r8, (r8)
#endif

    SET_CP_MMU
    RD_MEH  r5
    mov     r4, r5
    lsri    r4, _PGDIR_SHIFT
    lsli    r4, 2
    addu    r8, r4
    ldw     r8, (r8)
#ifdef CONFIG_MMU_HARD_REFILL
    bseti   r8, 31
#endif

    lsri    r5, PTE_INDX_SHIFT
    lrw     r4, PTE_INDX_MSK
    and     r5, r4
    addu    r8, r5
    ldw     r5, (r8)
    bgeni   r4, 31      /* TLB probe command, r4 = 0x80000000 */

    WR_MCIR	r4          /* find faulting entry */
    movi    r4, _PAGE_WRITE
    and     r5, r4
    cmpnei  r5, 0
    bf      tlbmodified
    ldw     r5, (r8)

    /*
     * Present and writable bits set, set accessed and dirty bits.
     * r4 = (_PAGE_ACCESSED | _PAGE_MODIFIED | _PAGE_VALID | _PAGE_DIRTY)
     */
#ifdef CONFIG_CPU_MMU_V1
    movi    r4, 0x18
    bseti   r4, 7
    bseti   r4, 8
#else
    movi    r4, (_PAGE_ACCESSED | _PAGE_MODIFIED | _PAGE_VALID | _PAGE_DIRTY)
#endif
    or      r5, r4
    stw     r5, (r8)

    /* Now reload the entry into the tlb. */
    bclri   r8, PTE_BIT
#ifdef CONFIG_CPU_MMU_V1
    ldw     r4, (r8, 4)
    lsri    r4, 6
    WR_MEL1 r4
    ldw     r4, (r8)
    lsri    r4, 6
    WR_MEL0 r4
#else
    ldw     r4, (r8, 4)
    WR_MEL1 r4
    ldw     r4, (r8)
    WR_MEL0 r4
#endif

    bgeni   r4, 29      /* TLB write index command, r9 = 0x20000000. */
    WR_MCIR r4

    mfcr    r5, ss2
    mfcr    r8, ss3
    mfcr    r4, ss4
    rte

tlbmodified:
    mfcr    r5, ss2
    mfcr    r8, ss3
    mfcr    r4, ss4
    SAVE_ALL

    SET_CP_MMU
    RD_MEH  r5
    bmaski  r8, 12
    andn    r5, r8          /* r8 = !(0xfffff000) */
    mov     r4, r5
    psrset  ee, ie          /* Enable exception & interrupt */
    mov     r2, r0
    movi    r3, 1
    jsri    (do_page_fault)
    jmpi    (ret_from_exception)
#else
ENTRY(handle_tlbinvalidl)
ENTRY(handle_tlbinvalids)
ENTRY(handle_tlbmiss)
ENTRY(handle_tlbmodified)
    rte
#endif

/*
 * This function is used to handle access exception.
 */
ENTRY(buserr)
	SAVE_ALL
	SET_SMOD_MMU_CP15
	bmaski  r2, 0                   /* r2 = -1, Not a syscall */
	stw     r2, (r0, LSAVE_SYSCALLR1)
	mov     r2, r0                  /* Stack address is arg[0] */
	jbsr    buserr_c                /* Call C level handler */
	jmpi    ret_from_exception

ENTRY(system_call)
	SAVE_ALL
	SET_SMOD_MMU_CP15
	
	/*
	 * Do not use r2-r7 here, because the arguments are saved in r2-r6
	 * and the syscall number is saved in r1, when the exception is a
	 * systemcall.
	 * Use temp regs instead
	 *
	 * When excuting a trap instruction, the pc does not increase.
	 * The pc should
	 * be increased manully and save in epc register.
	 */
	mfcr    r14, epc                /* Get the trap point */
#if defined(CONFIG_CPU_CSKYV1)
	addi    r14, 2                  /* Increase the epc */
#elif defined(CONFIG_CPU_CSKYV2)
	addi    r14, 4                  /* Increase the epc, because the Instruct "trap x" in CK ISA V2 is 32 bit */
#endif

	mtcr    r14, epc                /* Save return point */
	stw     r14, (r0)               /* Save it in stack*/
	psrset  ee, ie                 /* Enable Exception & interrupt */
	
	/* Stack frame for syscall, origin call set_esp0 */
	mov     r12, r0
	bmaski  r11, 13
	andn    r12, r11
	bgeni   r11, 9
	addi    r11, 32
	addu    r12, r11
	st      r0, (r12, 0)

	lrw     r14, NR_syscalls
	cmphs   r1, r14                 /* Check nr of syscall */
	bt      ret_from_exception
	
	lrw     r14, sys_call_table
	ixw     r14, r1                 /* Index into syscall table */
	ldw     r14, (r14)              /* Get syscall function */
	cmpnei  r14, 0                  /* Check for not null */
	bf      ret_from_exception
	
	mov     r9, r0                  /* Get task pointer */
	bmaski  r10, THREADSIZE_MASK_BIT
	andn    r9, r10                 /* Get thread_info */
	ldw     r8, (r9, TINFO_FLAGS)   /* Get thread_info.flags value */
	btsti   r8, TIF_SYSCALL_TRACE   /* Check if TIF_SYSCALL_TRACE set */
	bt      1f
	
	jsr     r14                     /* Do system call */
	stw     r2, (r0, LSAVE_R2)      /* Save return value */
	jmpi    ret_from_exception

1:
	jbsr    syscall_trace
	
	jsr     r14                    /* Do system call */
	stw     r2, (r0, LSAVE_R2)     /* Save return value */
	
	jbsr     syscall_trace

syscall_exit_work:
	ld       r1, (r0,12)
	btsti    r1, 31
	bt       1f
	
	jmpi     resume_userspace

1:      RESTORE_ALL

ENTRY(ret_from_signal)
	jmpi     ret_from_exception


ENTRY(ret_from_fork)
	jbsr     schedule_tail
	jbsr     ret_from_exception

ENTRY(ret_from_exception)
	ld       r1, (r0,12)
	btsti    r1, 31
	bt       1f
	/*
	 * Load address of current->thread_info, Then get address of task_struct
	 * Get task_needreshed in task_struct
	 */
	mov      r9, r0                       /* Get current stack  pointer */
	bmaski   r10, THREADSIZE_MASK_BIT
	andn     r9, r10                      /* Get thread_info */

resume_userspace:
	ldw      r8, (r9, TINFO_FLAGS)
	cmpnei   r8, 0
	bt       exit_work
1:  RESTORE_ALL
        
exit_work:
	lrw      r2, init_task          /* Address of init_task */
	ldw      r2, (r2)               /* Address of task_struct of task[0]*/
	cmpne    r10, r2
	bf       1b                     /* Exit if task[0] */

	
	mov      r2, r0                 /* Stack address is arg[0] */
	jbsr     set_esp0               /* Call C level */
	btsti    r8, TIF_NEED_RESCHED
	bf       Lsignal_return
	
	lrw      r1, ret_from_exception
	mov      r15, r1                /* Return address in link */
	jmpi     schedule

Lsignal_return:
	mov      r3, r0
	movi     r2, 0
	jbsr     do_signal              /* Call signal handler */
	br       resume_userspace


/*
 * Common trap handler. Standard traps come through here first
 */

ENTRY(trap)
	SAVE_ALL
	SET_SMOD_MMU_CP15

	bmaski   r2, 0                  /* r2 = -1, Not a syscall */
	stw      r2, (r0, LSAVE_SYSCALLR1)
	
	mfcr     r2, psr                /* Get psr register */
	lsri     r2, 16                 /* Get vector in base 8 bits */
	sextb    r2                     /* Fill upper bytes with zero */
	mov      r3, r0                 /* Push Stack pointer arg */
	jbsr     trap_c                 /* Call C-level trap handler */
	jmpi     ret_from_exception

/*
 * Alignment_exception handler.
 */
ENTRY(alignment)
	SAVE_ALL
	SET_SMOD_MMU_CP15
	psrset   ee                     /* Enable Exception */
	bmaski   r2, 0                  /* r2 = -1, Not a syscall */
	stw      r2, (r0, LSAVE_SYSCALLR1)
	mov      r2, r0                 /* Push Stack pointer arg */
	jbsr     alignment_c            /* Call C-level align exception handler */
	jmpi     ret_from_exception

ENTRY(trap1)
#if defined(CONFIG_CPU_CSKYV1)
	mtcr     r0, ss1
	mfcr     r0, ss0
	mtcr     r3, ss4
#elif defined(CONFIG_CPU_CSKYV2)
	subi     r0, 8
	stw      r3, (r0)
#endif
	mfcr     r3, epc                /* Get the trap point */
#if defined(CONFIG_CPU_CSKYV1)
	addi     r3, 2                  /* Increase the epc */
#elif defined(CONFIG_CPU_CSKYV2)
	addi     r3, 4                  /* Increase the epc, because the Instruct "trap x" in CK ISA V2 is 32 bit */
#endif
	mtcr     r3, epc                /* Save return point */
	
	movi     r3, 0x32
	mtcr     r3, cr17
#if defined(CONFIG_CPU_CSKYV1)
	mfcr     r3, ss4
	mtcr     r0, ss0
	mfcr     r0, ss1
#elif defined(CONFIG_CPU_CSKYV2)
	ldw      r3, (r0)
	addi     r0, 8
#endif
	rte
	
/*
 *  This is the generic interrupt handler (for all hardware interrupt
 *  sources). It figures out the vector number and calls the appropriate
 *  interrupt service routine directly. This is for vectored normal
 *  interrupts.
 */

ENTRY(trap2)
	SAVE_ALL
	SET_SMOD_MMU_CP15

	mfcr     r7, epc                /* Get the trap point */
#if defined(CONFIG_CPU_CSKYV1)
	addi     r7, 2                  /* Increase the epc */
#elif defined(CONFIG_CPU_CSKYV2)
	addi     r7, 4                  /* Increase the epc, because the Instruct "trap x" in CK ISA V2 is 32 bit */
#endif
	mtcr     r7, epc                /* Save return point */
	stw      r7, (r0)
	RESTORE_ALL

ENTRY(trap3)                            /* added for get tls*/
#if defined(CONFIG_CPU_CSKYV1)
	mtcr     r0, ss1
	mfcr     r0, ss0
#endif
	subi     r0, 8
	stw      r3, (r0, 0)            /* need to save r3? */

	mfcr     r3, epc                /* Get the trap point */
#if defined(CONFIG_CPU_CSKYV1)
	addi     r3, 2                  /* Increase the epc */
#elif defined(CONFIG_CPU_CSKYV2)
	addi     r3, 4                  /* Increase the epc, because the Instruct "trap x" in CK ISA V2 is 32 bit */
#endif
	mtcr     r3, epc                /* Save return point */
	
	bmaski   r3, (PAGE_SHIFT + 1)   /* kernel stack is 2*page if page is 4k */
	not      r3
	and      r3, r0                 /* thread_info local in bottom of stack */
	
	ldw      r2, (r3, TINFO_TP_VALURE) /* get tls */
	
	ldw      r3, (r0, 0)            /* restore r3 */
	addi     r0, 8

#if defined(CONFIG_CPU_CSKYV1)
	mtcr     r0, ss0
	mfcr     r0, ss1
#endif
	rte
	

/*
 * handle FPU exception.
 */
ENTRY(handle_fpe)
	SAVE_ALL
	/* Clear FPU exception state */
#if defined(CONFIG_CPU_HAS_FPU)
	mfcr      r2, cr15
	bseti     r2, 28
	mtcr      r2, cr15             /* clear the exceptional state */
	cprcr     r2, cpcr4            /* read fesr to check the exception type */
	mov       r3, r0               /* Push Stack pointer arg */
	jbsr      handle_fpe_c         /* Call C-level fpe handler */
	SET_SMOD_MMU_CP15
#endif
	jmpi      ret_from_exception

/*
 * handle interrupt.
 */
ENTRY(inthandler)
	SAVE_ALL
	SET_SMOD_MMU_CP15
	psrset   ee                     /* Enable exceptions */
	
	bmaski   r2, 0                  /* r2 = -1, Not a syscall */
	stw      r2, (r0, LSAVE_SYSCALLR1)
	
	mov     r9, r0                  /* Get current stack  pointer */
	bmaski  r10, THREADSIZE_MASK_BIT
	andn    r9, r10                 /* Get thread_info */

#ifdef CONFIG_PREEMPT
	/*
	 * Get task_struct->stack.preempt_count for current,
	 * and increase 1.
	 */
	ldw      r8, (r9, TINFO_PREEMPT)
	addi     r8, 1
	stw      r8, (r9, TINFO_PREEMPT)
#endif
	mfcr     r2, psr                /* Get PSR register */
	lsri     r2, 16                 /* Get vector in 7 bits */
	sextb    r2                     /* Fill upper bytes with zero */
	subi     r2, 32                 /* Real irq nomber need sub VEC offset(32)*/
	mov      r3, r0                 /* arg[1] is stack pointer */
	jsri     csky_do_IRQ          /* Call handler */

#ifdef CONFIG_PREEMPT
	subi     r8, 1
	stw      r8, (r9, TINFO_PREEMPT)
	cmpnei   r8, 0
	bt       2f
	ldw      r8, (r9, TINFO_FLAGS)
	btsti    r8, TIF_NEED_RESCHED
	bf       2f
1:
	jsri     preempt_schedule_irq   /* irq en/disable is done inside */
	ldw      r7, (r9, TINFO_FLAGS)  /* get new tasks TI_FLAGS */
	btsti    r7, TIF_NEED_RESCHED
	bt       1b                     /* go again */
#endif
2:
	jmpi     ret_from_exception
 
/*
 * This is the auto-vectored interrupt handler (for all hardware interrupt
 * sources). It figures out the vector number and calls the appropriate
 * interrupt service routine directly. This is for auto-vectored normal
 * interrupts only.
 *
 */

ENTRY(autohandler)
	SAVE_ALL
	SET_SMOD_MMU_CP15
	psrset  ee       // enable exception

	bmaski   r2, 0                       /* r2 = -1, Not a syscall */
	stw      r2, (r0, LSAVE_SYSCALLR1)

#ifdef CONFIG_PREEMPT
	mov     r9, r0                       /* Get current stack  pointer */
	bmaski  r10, THREADSIZE_MASK_BIT
	andn    r9, r10                      /* Get thread_info */

	/*
	 * Get task_struct->stack.preempt_count for current,
	 * and increase 1.
	 */
	ldw      r8, (r9, TINFO_PREEMPT)
	addi     r8, 1
	stw      r8, (r9, TINFO_PREEMPT)
#endif

	mov      r2, r0                      /* arg[0] is stack pointer */
	jsri     csky_do_auto_IRQ          /* Call handler */

#ifdef CONFIG_PREEMPT
	subi     r8, 1
	stw      r8, (r9, TINFO_PREEMPT)
	cmpnei   r8, 0
	bt       2f
	ldw      r8, (r9, TINFO_FLAGS)
	btsti    r8, TIF_NEED_RESCHED
	bf       2f
1:
	jsri     preempt_schedule_irq   /* irq en/disable is done inside */
	ldw      r7, (r9, TINFO_FLAGS)  /* get new tasks TI_FLAGS */
	btsti    r7, TIF_NEED_RESCHED
	bt       1b                     /* go again */
#endif
2:
	jmpi     ret_from_exception

/*
 * This is the fast interrupt handler (for certain hardware interrupt
 * sources). Unlike the normal interrupt handler it doesn't bother
 * doing the bottom half handlers.
 *
 */

#ifdef CONFIG_CPU_USE_FIQ
ENTRY(fasthandler)
	mfcr     r2, psr                /* Get PSR register */
	lsri     r2, 16                 /* Get vector in base 8 bits */
	sextb    r2                     /* Fill upper bytes with 0 */
	subi    r2, 32                  /* Real irq nomber need sub VEC offset(32)*/
	jsri     csky_do_FIQ          /* Call handler */
	rfi
#endif

/*
 * This is the fast aotu-vector interrupt handler (for certain hardware
 * interrupt sources). Unlike the normal interrupt handler it doesn't
 * bother doing the bottom half handlers.
 *
 */
ENTRY(fastautohandler)
#ifdef CONFIG_CPU_USE_FIQ
	jsri     csky_do_auto_FIQ     /* Call handler */
#endif
	rfi

ENTRY(sys_fork)
	mov      r2, r0
	jmpi     csky_fork            /* Call fork routine */

ENTRY(sys_clone)
	mov      r2, r0
	jmpi     csky_clone           /* Call clone routine */

ENTRY(sys_vfork)
	mov      r2, r0
	jmpi     csky_vfork           /* Call fork routine */

ENTRY(sys_sigsuspend)
	mov      r2, r0
	jmpi     do_rt_sigsuspend       /* Call sigsuspend routine */

ENTRY(sys_sigreturn)
	mov      r2, r0
	jmpi     do_sigreturn

ENTRY(sys_rt_sigreturn)
	mov      r2, r0
	jmpi     do_rt_sigreturn

ENTRY(sys_rt_sigsuspend)
	mov      r2, r0
	jmpi     do_rt_sigsuspend       /* Call sigsuspend routine */

/*
 * Resume execution of a new process.
 * Register definitions comming in:
 *
 * r2   =  current task
 * r3   =  new task
 */

ENTRY(resume)
	lrw      r5, TASK_THREAD        /* struct_thread offset in task_struct */
	addu     r5, r2                 /* r5 point to thread in task_struct */
	mfcr     r4, psr                /* Save PSR value */
	stw      r4, (r5, THREAD_SR)    /* Save PSR in task struct */
	bclri    r4, 6                  /* Disable interrupts */
	mtcr     r4, psr

	SAVE_SWITCH_STACK

#if defined(CONFIG_CPU_CSKYV1)
	mfcr     r6, ss1                /* Get current usp */
#elif defined(CONFIG_CPU_CSKYV2)
	mfcr     r6, cr<0,1>                /* Get current usp */
#endif
	stw      r6, (r5, THREAD_USP)   /* Save usp in task struct */
	stw      r0, (r5, THREAD_KSP)   /* Save ksp in task struct */

#ifdef CONFIG_CPU_HAS_FPU
	cpseti   1             /* select fpu */
	/* Save FPU control regs task struct */
	cprcr    r6, cpcr2
	cprcr    r7, cpcr4
	stw      r6, (r5, THREAD_FSR)
	stw      r7, (r5, THREAD_FESR)
	/* Save FPU general regs task struct */
	lrw      r10, THREAD_FPREG
	add      r10, r5
	fmfs     r6, fr0
	fmfs     r7, fr1
	fmfs     r8, fr2
	fmfs     r9, fr3
	stw      r6, (r10, 0)
	stw      r7, (r10, 4)
	stw      r8, (r10, 8)
	stw      r9, (r10, 12)
	fmfs     r6, fr4
	fmfs     r7, fr5
	fmfs     r8, fr6
	fmfs     r9, fr7
	stw      r6, (r10, 16)
	stw      r7, (r10, 20)
	stw      r8, (r10, 24)
	stw      r9, (r10, 28)
	fmfs     r6, fr8
	fmfs     r7, fr9
	fmfs     r8, fr10
	fmfs     r9, fr11
	stw      r6, (r10, 32)
	stw      r7, (r10, 36)
	stw      r8, (r10, 40)
	stw      r9, (r10, 44)
	fmfs     r6, fr12
	fmfs     r7, fr13
	fmfs     r8, fr14
	fmfs     r9, fr15
	stw      r6, (r10, 48)
	stw      r7, (r10, 52)
	stw      r8, (r10, 56)
	stw      r9, (r10, 60)
	movi     r11, 64
	add      r10, r11
	fmfs     r6, fr16
	fmfs     r7, fr17
	fmfs     r8, fr18
	fmfs     r9, fr19
	stw      r6, (r10, 0)
	stw      r7, (r10, 4)
	stw      r8, (r10, 8)
	stw      r9, (r10, 12)
	fmfs     r6, fr20
	fmfs     r7, fr21
	fmfs     r8, fr22
	fmfs     r9, fr23
	stw      r6, (r10, 16)
	stw      r7, (r10, 20)
	stw      r8, (r10, 24)
	stw      r9, (r10, 28)
	fmfs     r6, fr24
	fmfs     r7, fr25
	fmfs     r8, fr26
	fmfs     r9, fr27
	stw      r6, (r10, 32)
	stw      r7, (r10, 36)
	stw      r8, (r10, 40)
	stw      r9, (r10, 44)
	fmfs     r6, fr28
	fmfs     r7, fr29
	fmfs     r8, fr30
	fmfs     r9, fr31
	stw      r6, (r10, 48)
	stw      r7, (r10, 52)
	stw      r8, (r10, 56)
	stw      r9, (r10, 60)
#endif

#if  defined(CONFIG_CPU_HAS_DSP) || defined(__CK810__)
	/* Save DSP regs */
	lrw      r10, THREAD_DSPHI
	add      r10, r5
	mfhi     r6
	mflo     r7
	stw      r6, (r10, 0)           /* THREAD_DSPHI */
	stw      r7, (r10, 4)           /* THREAD_DSPLO */
	mfcr     r6, cr14
	stw      r6, (r10, 8)           /* THREAD_DSPCSR */
#endif

	lrw      r5, TASK_THREAD
	addu     r5, r3                 /* Pointer to thread in task_struct */
	
	/* Set up next process to run */
	ldw      r0, (r5, THREAD_KSP)   /* Set next ksp */
	ldw      r6, (r5, THREAD_USP)   /* Set next usp */
#if defined(CONFIG_CPU_CSKYV1)
	mtcr     r6, ss1
#elif defined(CONFIG_CPU_CSKYV2)
	mtcr     r6, cr<0,1>                /* Set current usp */
#endif

#ifdef    CONFIG_CPU_HAS_FPU
	/* Save FPU control regs task struct */
	ldw      r6, (r5, THREAD_FSR)
	ldw      r7, (r5, THREAD_FESR)
	cpwcr    r6, cpcr2
	cpwcr    r7, cpcr4
	/* restore FPU general regs task struct */
	lrw      r10, THREAD_FPREG
	add      r10, r5
	ldw      r6, (r10, 0)
	ldw      r7, (r10, 4)
	ldw      r8, (r10, 8)
	ldw      r9, (r10, 12)
	fmts     r6, fr0
	fmts     r7, fr1
	fmts     r8, fr2
	fmts     r9, fr3
	ldw      r6, (r10, 16)
	ldw      r7, (r10, 20)
	ldw      r8, (r10, 24)
	ldw      r9, (r10, 28)
	fmts     r6, fr4
	fmts     r7, fr5
	fmts     r8, fr6
	fmts     r9, fr7
	ldw      r6, (r10, 32)
	ldw      r7, (r10, 36)
	ldw      r8, (r10, 40)
	ldw      r9, (r10, 44)
	fmts     r6, fr8
	fmts     r7, fr9
	fmts     r8, fr10
	fmts     r9, fr11
	ldw      r6, (r10, 48)
	ldw      r7, (r10, 52)
	ldw      r8, (r10, 56)
	ldw      r9, (r10, 60)
	fmts     r6, fr12
	fmts     r7, fr13
	fmts     r8, fr14
	fmts     r9, fr15
	movi     r11, 64
	add      r10, r11
	ldw      r6, (r10, 0)
	ldw      r7, (r10, 4)
	ldw      r8, (r10, 8)
	ldw      r9, (r10, 12)
	fmts     r6, fr16
	fmts     r7, fr17
	fmts     r8, fr18
	fmts     r9, fr19
	ldw      r6, (r10, 16)
	ldw      r7, (r10, 20)
	ldw      r8, (r10, 24)
	ldw      r9, (r10, 28)
	fmts     r6, fr20
	fmts     r7, fr21
	fmts     r8, fr22
	fmts     r9, fr23
	ldw      r6, (r10, 32)
	ldw      r7, (r10, 36)
	ldw      r8, (r10, 40)
	ldw      r9, (r10, 44)
	fmts     r6, fr24
	fmts     r7, fr25
	fmts     r8, fr26
	fmts     r9, fr27
	ldw      r6, (r10, 48)
	ldw      r7, (r10, 52)
	ldw      r8, (r10, 56)
	ldw      r9, (r10, 60)
	fmts     r6, fr28
	fmts     r7, fr29
	fmts     r8, fr30
	fmts     r9, fr31
#endif

#if  defined(CONFIG_CPU_HAS_DSP) || defined(__CK810__)
	lrw      r10, THREAD_DSPHI
	add      r10, r5
	ldw      r6, (r10, 8)   /* THREAD_DSPCSR */
#if defined(__CK810__)
	mtcr     r6, cr14
#else
	/*
	 * Because bit 0 in CK610's cr14 is read only, we need to restore it by
	 * using special method
	 */
	btsti    r6, 0
	movi     r7, 0xf
	bf       1f
	bmaski   r7, 0           /* old is "lrw r7, 0xffffffff" */
1:
	mthi     r7
	mulua    r7, r7
#endif
	/* Restore DSP regs */
	ldw      r6, (r10, 0)    /* THREAD_DSPHI */
	ldw      r7, (r10, 4)    /* THREAD_DSPLO */
	mthi     r6
	mtlo     r7
#endif

	ldw      r4, (r5, THREAD_SR)    /* Set next PSR */
	mtcr     r4, psr
	
#if  defined(CONFIG_CPU_CSKYV2)
	/* set TLS register (r31) */
	addi     r7, r3, TASK_THREAD_INFO
	ldw      r31, (r7, TINFO_TP_VALURE)
#endif

	RESTORE_SWITCH_STACK

	rts

.data
ALIGN
sys_call_table:
	.long sys_restart_syscall /* 0 old "setup" system call, used for restart*/
	.long sys_exit
	.long sys_fork
	.long sys_read
	.long sys_write
	.long sys_open          /* 5 */
	.long sys_close
	.long sys_waitpid
	.long sys_creat
	.long sys_link
	.long sys_unlink        /* 10 */
	.long sys_execve
	.long sys_chdir
	.long sys_time
	.long sys_mknod
	.long sys_chmod         /* 15 */
	.long sys_chown16
	.long sys_ni_syscall    /* old break syscall holder */
	.long sys_stat
	.long sys_lseek
	.long sys_getpid        /* 20 */
	.long sys_mount
	.long sys_oldumount
	.long sys_setuid16
	.long sys_getuid16
	.long sys_stime         /* 25 */
	.long sys_ptrace
	.long sys_alarm
	.long sys_fstat
	.long sys_pause
	.long sys_utime         /* 30 */
	.long sys_ni_syscall    /* old stty syscall holder */
	.long sys_ni_syscall    /* old gtty syscall holder */
	.long sys_access
	.long sys_nice
	.long sys_ni_syscall    /* 35 old ftime syscall holder */
	.long sys_sync
	.long sys_kill
	.long sys_rename
	.long sys_mkdir
	.long sys_rmdir         /* 40 */
	.long sys_dup
	.long sys_pipe
	.long sys_times
	.long sys_ni_syscall    /* old prof syscall holder */
	.long sys_brk           /* 45 */
	.long sys_setgid16
	.long sys_getgid16
	.long sys_signal
	.long sys_geteuid16
	.long sys_getegid16     /* 50 */
	.long sys_acct
	.long sys_umount        /* recycled never used phys() */
	.long sys_ni_syscall    /* old lock syscall holder */
	.long sys_ioctl
	.long sys_fcntl         /* 55 */
	.long sys_ni_syscall    /* old mpx syscall holder */
	.long sys_setpgid
	.long sys_ni_syscall    /* old ulimit syscall holder */
	.long sys_ni_syscall
	.long sys_umask         /* 60 */
	.long sys_chroot
	.long sys_ustat
	.long sys_dup2
	.long sys_getppid
	.long sys_getpgrp       /* 65 */
	.long sys_setsid
	.long sys_sigaction
	.long sys_sgetmask
	.long sys_ssetmask
	.long sys_setreuid16    /* 70 */
	.long sys_setregid16
	.long sys_sigsuspend
	.long sys_sigpending
	.long sys_sethostname
	.long sys_setrlimit     /* 75 */
	.long sys_old_getrlimit
	.long sys_getrusage
	.long sys_gettimeofday
	.long sys_settimeofday
	.long sys_getgroups16   /* 80 */
	.long sys_setgroups16
	.long old_select
	.long sys_symlink
	.long sys_lstat
	.long sys_readlink      /* 85 */
	.long sys_uselib
	.long sys_swapon
	.long sys_reboot
	.long sys_old_readdir
	.long old_mmap          /* 90 */
	.long sys_munmap
	.long sys_truncate
	.long sys_ftruncate
	.long sys_fchmod
	.long sys_fchown16      /* 95 */
	.long sys_getpriority
	.long sys_setpriority
	.long sys_ni_syscall    /* old profil syscall holder */
	.long sys_statfs
	.long sys_fstatfs       /* 100 */
	.long sys_ni_syscall    /* ioperm for i386 */
	.long sys_socketcall
	.long sys_syslog
	.long sys_setitimer
	.long sys_getitimer     /* 105 */
	.long sys_newstat
	.long sys_newlstat
	.long sys_newfstat
	.long sys_ni_syscall
	.long sys_ni_syscall    /* 110 iopl for i386 */
	.long sys_vhangup
	.long sys_ni_syscall    /* obsolete idle() syscall */
	.long sys_ni_syscall    /* vm86old for i386 */
	.long sys_wait4
	.long sys_swapoff       /* 115 */
	.long sys_sysinfo
	.long sys_ipc
	.long sys_fsync
	.long sys_sigreturn
	.long sys_clone         /* 120 */
	.long sys_setdomainname
	.long sys_newuname
	.long sys_cacheflush    /* modify_ldt for i386 */
	.long sys_adjtimex
	.long sys_mprotect      /* 125 */
	.long sys_sigprocmask
	.long sys_ni_syscall    /* old "create_module" */
	.long sys_init_module
	.long sys_delete_module
	.long sys_ni_syscall    /* 130 - old "get_kernel_syms" */
	.long sys_quotactl
	.long sys_getpgid
	.long sys_fchdir
	.long sys_bdflush
	.long sys_sysfs         /* 135 */
	.long sys_personality
	.long sys_ni_syscall    /* for afs_syscall */
	.long sys_setfsuid16
	.long sys_setfsgid16
	.long sys_llseek        /* 140 */
	.long sys_getdents
	.long sys_select
	.long sys_flock
	.long sys_msync
	.long sys_readv         /* 145 */
	.long sys_writev
	.long sys_getsid
	.long sys_fdatasync
	.long sys_sysctl
	.long sys_mlock         /* 150 */
	.long sys_munlock
	.long sys_mlockall
	.long sys_munlockall
	.long sys_sched_setparam
	.long sys_sched_getparam        /* 155 */
	.long sys_sched_setscheduler
	.long sys_sched_getscheduler
	.long sys_sched_yield
	.long sys_sched_get_priority_max
	.long sys_sched_get_priority_min  /* 160 */
	.long sys_sched_rr_get_interval
	.long sys_nanosleep
	.long sys_mremap
	.long sys_setresuid16
	.long sys_getresuid16   /* 165 */
	.long sys_getpagesize
	.long sys_ni_syscall    /* old sys_query_module */
	.long sys_poll
	.long sys_nfsservctl
	.long sys_setresgid16   /* 170 */
	.long sys_getresgid16
	.long sys_prctl
	.long sys_rt_sigreturn
	.long sys_rt_sigaction
	.long sys_rt_sigprocmask        /* 175 */
	.long sys_rt_sigpending
	.long sys_rt_sigtimedwait
	.long sys_rt_sigqueueinfo
	.long sys_rt_sigsuspend
	.long csky_pread       /* 180 */
	.long csky_pwrite
	.long sys_lchown16;
	.long sys_getcwd
	.long sys_capget
	.long sys_capset        /* 185 */
	.long sys_sigaltstack
	.long sys_sendfile
	.long sys_ni_syscall    /* streams1 */
	.long sys_ni_syscall    /* streams2 */
	.long sys_vfork         /* 190 */
	.long sys_getrlimit
	.long sys_mmap2
	.long sys_truncate64
	.long sys_ftruncate64
	.long sys_stat64        /* 195 */
	.long sys_lstat64
	.long sys_fstat64
	.long sys_chown
	.long sys_getuid
	.long sys_getgid        /* 200 */
	.long sys_geteuid
	.long sys_getegid
	.long sys_setreuid
	.long sys_setregid
	.long sys_getgroups     /* 205 */
	.long sys_setgroups
	.long sys_fchown
	.long sys_setresuid
	.long sys_getresuid
	.long sys_setresgid     /* 210 */
	.long sys_getresgid
	.long sys_lchown
	.long sys_setuid
	.long sys_setgid
	.long sys_setfsuid      /* 215 */
	.long sys_setfsgid
	.long sys_pivot_root
	.long sys_set_thread_area
	.long sys_ni_syscall
	.long sys_getdents64    /* 220 */
	.long sys_gettid
	.long sys_tkill
	.long sys_setxattr
	.long sys_lsetxattr
	.long sys_fsetxattr     /* 225 */
	.long sys_getxattr
	.long sys_lgetxattr
	.long sys_fgetxattr
	.long sys_listxattr
	.long sys_llistxattr    /* 230 */
	.long sys_flistxattr
	.long sys_removexattr
	.long sys_lremovexattr
	.long sys_fremovexattr
	.long sys_futex         /* 235 */
	.long sys_sendfile64
	.long sys_mincore
	.long sys_madvise
	.long sys_fcntl64
	.long sys_readahead     /* 240 */
	.long sys_io_setup
	.long sys_io_destroy
	.long sys_io_getevents
	.long sys_io_submit
	.long sys_io_cancel     /* 245 */
	.long sys_fadvise64
	.long sys_exit_group
	.long sys_lookup_dcookie
	.long sys_epoll_create
	.long sys_epoll_ctl     /* 250 */
	.long sys_epoll_wait
	.long sys_remap_file_pages
	.long sys_set_tid_address
	.long sys_timer_create
	.long sys_timer_settime /* 255 */
	.long sys_timer_gettime
	.long sys_timer_getoverrun
	.long sys_timer_delete
	.long sys_clock_settime
	.long sys_clock_gettime /* 260 */
	.long sys_clock_getres
	.long sys_clock_nanosleep
	.long sys_statfs64
	.long sys_fstatfs64
	.long sys_tgkill        /* 265 */
	.long sys_utimes
	.long sys_fadvise64_64
	.long sys_mbind
	.long sys_get_mempolicy
	.long sys_set_mempolicy /* 270 */
	.long sys_mq_open
	.long sys_mq_unlink
	.long sys_mq_timedsend
	.long sys_mq_timedreceive
	.long sys_mq_notify     /* 275 */
	.long sys_mq_getsetattr
	.long sys_waitid
	.long sys_ni_syscall    /* for sys_vserver */
	.long sys_add_key
	.long sys_request_key   /* 280 */
	.long sys_keyctl
	.long sys_ioprio_set
	.long sys_ioprio_get
	.long sys_inotify_init
	.long sys_inotify_add_watch     /* 285 */
	.long sys_inotify_rm_watch
	.long sys_migrate_pages
	.long sys_openat
	.long sys_mkdirat
	.long sys_mknodat               /* 290 */
	.long sys_fchownat
	.long sys_futimesat
	.long sys_fstatat64
	.long sys_unlinkat
	.long sys_renameat              /* 295 */
	.long sys_linkat
	.long sys_symlinkat
	.long sys_readlinkat
	.long sys_fchmodat
	.long sys_faccessat             /* 300 */
	.long sys_ni_syscall            /* Reserved for pselect6 */
	.long sys_ni_syscall            /* Reserved for ppoll */
	.long sys_unshare
	.long sys_set_robust_list
	.long sys_get_robust_list       /* 305 */
	.long sys_splice
	.long sys_sync_file_range
	.long sys_tee
	.long sys_vmsplice
	.long sys_move_pages            /* 310 */
	.long sys_sched_setaffinity
	.long sys_sched_getaffinity
	.long sys_kexec_load
	.long sys_getcpu
	.long sys_epoll_pwait           /* 315 */
	.long sys_utimensat
	.long sys_signalfd
	.long sys_timerfd_create
	.long sys_eventfd
	.long sys_fallocate             /* 320 */
	.long sys_timerfd_settime
	.long sys_timerfd_gettime
	.long sys_signalfd4
	.long sys_eventfd2
	.long sys_epoll_create1         /* 325 */
	.long sys_dup3
	.long sys_pipe2
	.long sys_inotify_init1

                                                   
