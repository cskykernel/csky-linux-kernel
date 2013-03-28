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

#ifndef __CSKY_ENTRY_H
#define __CSKY_ENTRY_H

//#include <asm/setup.h>
#include <asm/page.h>

/*
 * Stack layout in 'ret_from_exception':
 *      Below describes the stack layout after initial exception entry.
 *      All traps, interrupts and exceptions will set up the stack frame
 *      in this way before starting processing proper.
 *	This allows access to the syscall arguments in registers r1-r5
 *
 *	 0(sp) - pc
 *	 4(sp) - r1
 *	 8(sp) - syscallr2
 *	 C(sp) - sr
 *	10(sp) - r2
 *	14(sp) - r3
 *	18(sp) - r4
 *	1C(sp) - r5
 *	20(sp) - r6
 *	24(sp) - r7
 *	28(sp) - r8
 *	2C(sp) - r9
 *	30(sp) - r10
 *	34(sp) - r11
 *	38(sp) - r12
 *	3C(sp) - r13
 *	40(sp) - r14
 *	44(sp) - r15
 */


/*
 *      Offsets into the stack to get at save registers.
 */
#define LSAVE_R1         0x4
#define LSAVE_SYSCALLR1  0x8
#define LSAVE_R2         0x10
#define LSAVE_R3         0x14
#define LSAVE_R4         0x18
#define LSAVE_R5         0x1C
#define LSAVE_R6         0x20
#define LSAVE_R7         0x24
#define LSAVE_R8         0x28
#define LSAVE_R9         0x2C
#define LSAVE_R10        0x30
#define LSAVE_R11        0x34
#define LSAVE_R12        0x38
#define LSAVE_R13        0x3C
#define LSAVE_R14        0x40
#define LSAVE_R15        0x44
#if defined(CONFIG_CPU_CSKYV2)
#define LSAVE_R16        0x48    
#define LSAVE_R17        0x4C
#define LSAVE_R18        0x50
#define LSAVE_R19        0x54
#define LSAVE_R20        0x58
#define LSAVE_R21        0x5C
#define LSAVE_R22        0x60
#define LSAVE_R23        0x64
#define LSAVE_R24        0x68
#define LSAVE_R25        0x6C
#define LSAVE_R26        0x70
#define LSAVE_R27        0x74
#define LSAVE_R28        0x78
#define LSAVE_R29        0x7C
#define LSAVE_R30        0x80
#define LSAVE_R31        0x84
#define LSAVE_Rhi        0x88
#define LSAVE_Rlo        0x8C
#endif

#define ALLOWINT 	0x40

#ifdef __ASSEMBLY__


#define SAVE_ALL    save_all
#define RESTORE_ALL restore_all
/*
 *      This code creates the normal kernel pt_regs layout on a trap
 *      or interrupt. The only trick here is that we check whether we
 *      came from supervisor mode before changing stack pointers.
 */

.macro	save_all 
#if defined(CONFIG_CPU_CSKYV1)
        mtcr    r1, ss2         /* save original r1 */
        mtcr    r2, ss3         /* save original r2 */
        mfcr    r1, epsr        /* Get original PSR */
        btsti   r1, 31          /* Check if was supervisor */
        bt      1f
        mtcr    r0, ss1         /* save user stack */
        mfcr    r0, ss0         /* Set kernel stack */
        1:
        subi    r0, 28 
        subi    r0, 32
        stw     r1, (r0, 0)
        stw     r2, (r0, 4)
        stw     r3, (r0, 8)
        stw     r4, (r0, 12)
        stw     r5, (r0, 16)
        stw     r6, (r0, 20)
        stw     r7, (r0, 24)
        stw     r8, (r0, 28)
        stw     r9, (r0, 32)
        stw     r10, (r0, 36)
        stw     r11, (r0, 40)
        stw     r12, (r0, 44)
        stw     r13, (r0, 48)
        stw     r14, (r0, 52)
        stw     r15, (r0, 56)

        subi    r0, 12          /* Make room for PC/PSR/r1 */
        mfcr    r1, ss2         /* Save original r1 on stack */
        stw     r1, (r0, 4)	/* Save r1 */
        /*
	 * syscallr1->syscallr2;r1->r2
	 * please refer asm/ptrace.h
	 */
        stw     r2, (r0, 8)     /* Save syscall r2 on stack */
        mfcr    r2, epc         /* Save PC on stack */
        stw     r2, (r0)
        mfcr    r2, ss3         /* restore original r2 */
#elif defined(CONFIG_CPU_CSKYV2)
        subi    r0,  144 
        stw     r1, (r0, 4)
        stw     r2, (r0, 16)
        stw     r3, (r0, 20)
        stw     r4, (r0, 24)
        stw     r5, (r0, 28)
        stw     r6, (r0, 32)
        stw     r7, (r0, 36)
        stw     r8, (r0, 40)
        stw     r9, (r0, 44)
        stw     r10, (r0, 48)
        stw     r11, (r0, 52)
        stw     r12, (r0, 56)
        stw     r13, (r0, 60)
        stw     r14, (r0, 64)
        stw     r15, (r0, 68)
        stw     r16, (r0, 72)
        stw     r17, (r0, 76)
        stw     r18, (r0, 80)
        stw     r19, (r0, 84)
        stw     r20, (r0, 88)
        stw     r21, (r0, 92)
        stw     r22, (r0, 96)
        stw     r23, (r0, 100)
        stw     r24, (r0, 104)
        stw     r25, (r0, 108)
        stw     r26, (r0, 112)
        stw     r27, (r0, 116)
        stw     r28, (r0, 120)
        stw     r29, (r0, 124)
        stw     r30, (r0, 128)
        stw     r31, (r0, 132)
        mfhi    r22
        mflo    r23
        stw     r22, (r0, 136)
        stw     r23, (r0, 140)
        stw     r2, (r0, 8)      /* Save syscall r2 on stack */
        mfcr    r2, epsr        /* Get original PSR */
        stw     r2, (r0, 12)     /* Save psr on stack */
        mfcr    r2, epc         /* Save PC on stack */
        stw     r2, (r0)
        ldw     r2, (r0, 16)      /* restore original r2 */
#endif
.endm

.macro	restore_all
        psrclr	ie              /* Disable interrupt */
        ldw     r2, (r0)        /* Restore PC */
        mtcr    r2, epc         /* Set return PC */    
        ldw     r1, (r0, 4)     /* Get original r1 */
        ldw     r2, (r0, 12)    /* Get saved PSR */
        mtcr    r2, epsr        /* Restore PSR */
        addi    r0, 16   
#if defined(CONFIG_CPU_CSKYV1) 
        btsti   r2, 31          /* Check if returning to user */
        ldm     r2-r15, (r0)    /* Restore registers */       
        addi    r0, 32          /* Increment stack pointer */
        addi    r0, 24 
        bt      1f
        mtcr    r0, ss0         /* Save kernel stack*/
        mfcr    r0, ss1         /* Set  user stack */ 
#elif defined(CONFIG_CPU_CSKYV2)
        ldw     r2, (r0, 120)
        ldw     r3, (r0, 124)
        mthi    r2    
        mtlo    r3
        ldm     r2-r31, (r0)     /* Restore registers */
        addi    r0, 128          /* Increment stack pointer */
#endif
1:
        rte
.endm

#define SAVE_SWITCH_STACK save_switch_stack
#define RESTORE_SWITCH_STACK restore_switch_stack
#define GET_CURRENT(tmp)

.macro	save_switch_stack
#if defined(CONFIG_CPU_CSKYV1)
        subi    r0, 32    
#elif defined(CONFIG_CPU_CSKYV2)
        subi    r0, 68 
#endif
        stw     r8, (r0, 0)
        stw     r9, (r0, 4)
        stw     r10, (r0, 8)
        stw     r11, (r0, 12)
        stw     r12, (r0, 16)
        stw     r13, (r0, 20)
        stw     r14, (r0, 24)
        stw     r15, (r0, 28)
#if defined(CONFIG_CPU_CSKYV2)
        stw     r16, (r0, 32)
        stw     r17, (r0, 36)
        stw     r18, (r0, 40)
        stw     r19, (r0, 44)
        stw     r26, (r0, 48)
        stw     r27, (r0, 52)
        stw     r28, (r0, 56)
        stw     r29, (r0, 60)
        stw     r30, (r0, 64)
#endif
.endm

.macro	restore_switch_stack
#if defined(CONFIG_CPU_CSKYV1)
        ldm     r8-r15, (r0)     /* Restore all registers */
        addi    r0, 32
#elif defined(CONFIG_CPU_CSKYV2)
        ldm     r8-r15, (r0)     /* Restore all registers */
        ldw     r16, (r0, 32)
        ldw     r17, (r0, 36)
        ldw     r18, (r0, 40)
        ldw     r19, (r0, 44)
        ldw     r26, (r0, 48)
        ldw     r27, (r0, 52)
        ldw     r28, (r0, 56)
        ldw     r29, (r0, 60)
        ldw     r30, (r0, 64)
        addi    r0, 68 
#endif
.endm

/*
 * Because kernel don't use FPU and only user program use FPU, we select 
 * coprocessor 15(MMU) when in super-mode. So this macro is called when 
 * CPU enter from user-mode to kernel super-mode except MMU exception.
 */
.macro SET_SMOD_MMU_CP15
#ifdef CONFIG_CPU_HAS_FPU
    cpseti  cp15
#endif
.endm

/*
 * Below, are macros for MMU operating, use them to switch cop, read or write
 * registers of MMU in assemble files. Macro CONFIG_CPU_MMU_V1 means MMU in
 * coprocessor.
 */
/* Coprocessor switch to MMU */
.macro SET_CP_MMU 
#ifdef CONFIG_CPU_MMU_V1 
	cpseti  cp15           
#endif 
.endm

/* MMU registers read operators. */
.macro RD_MIR	rx 
#ifdef CONFIG_CPU_MMU_V1 
	cprcr   \rx, cpcr0      
#else 
	mfcr    \rx, cr<0, 15>
#endif 
.endm

.macro RD_MRR	rx 
#ifdef CONFIG_CPU_MMU_V1 
	cprcr   \rx, cpcr1      
#else 
	mfcr    \rx, cr<1, 15>
#endif 
.endm

.macro RD_MEL0	rx 
#ifdef CONFIG_CPU_MMU_V1 
	cprcr   \rx, cpcr2      
#else 
	mfcr    \rx, cr<2, 15>
#endif 
.endm

.macro RD_MEL1	rx 
#ifdef CONFIG_CPU_MMU_V1 
	cprcr   \rx, cpcr3      
#else 
	mfcr    \rx, cr<3, 15>
#endif 
.endm

.macro RD_MEH	rx 
#ifdef CONFIG_CPU_MMU_V1 
	cprcr   \rx, cpcr4      
#else 
	mfcr    \rx, cr<4, 15>
#endif 
.endm

.macro RD_MCR	rx 
#ifdef CONFIG_CPU_MMU_V1 
	cprcr   \rx, cpcr5      
#else 
	mfcr    \rx, cr<5, 15>
#endif 
.endm

.macro RD_MPR	rx 
#ifdef CONFIG_CPU_MMU_V1 
	cprcr   \rx, cpcr6      
#else 
	mfcr    \rx, cr<6, 15>
#endif 
.endm

.macro RD_MWR	rx 
#ifdef CONFIG_CPU_MMU_V1 
	cprcr   \rx, cpcr7      
#else 
	mfcr    \rx, cr<7, 15>
#endif 
.endm

.macro RD_MCIR	rx 
#ifdef CONFIG_CPU_MMU_V1 
	cprcr   \rx, cpcr8      
#else 
	mfcr    \rx, cr<8, 15>
#endif 
.endm

/* MMU registers write operators. */
.macro WR_MIR	rx 
#ifdef CONFIG_CPU_MMU_V1
	cpwcr   \rx, cpcr0         
#else
	mtcr    \rx, cr<0, 15>
#endif
.endm

.macro WR_MRR	rx 
#ifdef CONFIG_CPU_MMU_V1
	cpwcr   \rx, cpcr1         
#else
	mtcr    \rx, cr<1, 15>
#endif
.endm

.macro WR_MEL0	rx 
#ifdef CONFIG_CPU_MMU_V1
	cpwcr   \rx, cpcr2         
#else
	mtcr    \rx, cr<2, 15>
#endif
.endm
.macro WR_MEL1	rx 
#ifdef CONFIG_CPU_MMU_V1
	cpwcr   \rx, cpcr3         
#else
	mtcr    \rx, cr<3, 15>
#endif
.endm

.macro WR_MEH	rx 
#ifdef CONFIG_CPU_MMU_V1
	cpwcr   \rx, cpcr4         
#else
	mtcr    \rx, cr<4, 15>
#endif
.endm

.macro WR_MCR	rx 
#ifdef CONFIG_CPU_MMU_V1
	cpwcr   \rx, cpcr5         
#else
	mtcr    \rx, cr<5, 15>
#endif
.endm

.macro WR_MPR	rx 
#ifdef CONFIG_CPU_MMU_V1
	cpwcr   \rx, cpcr6         
#else
	mtcr    \rx, cr<6, 15>
#endif
.endm

.macro WR_MWR	rx 
#ifdef CONFIG_CPU_MMU_V1
	cpwcr   \rx, cpcr7         
#else
	mtcr    \rx, cr<7, 15>
#endif
.endm

.macro WR_MCIR	rx 
#ifdef CONFIG_CPU_MMU_V1
	cpwcr   \rx, cpcr8         
#else
	mtcr    \rx, cr<8, 15>
#endif
.endm

/* MMU HINT register, use to adjust {asid, vpn} index */
.macro WR_HINT  rx
#ifdef CONFIG_CPU_MMU_V1
        cpwcr   \rx, cpcr31
#endif
.endm

#else /* C source */

#define STR(X) STR1(X)
#define STR1(X) #X

#define PT_OFF_ORIG_D0	 0x24
#define PT_OFF_FORMATVEC 0x32
#define PT_OFF_SR	 0x2C

#endif


#endif /* __CSKY_ENTRY_H */
	
