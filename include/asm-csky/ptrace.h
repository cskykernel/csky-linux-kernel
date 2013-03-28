/*
 * arch/csky/include/asm/ptrace.h
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of
 * this archive for more details.
 *  
 * (C) Copyright 2009, C-SKY Microsystems Co., Ltd. (www.c-sky.com)
 *  
 */

#ifndef _CSKY_PTRACE_H
#define _CSKY_PTRACE_H

#define REGNO_R0   0
#define REGNO_R1   1
#define REGNO_R2   2
#define REGNO_R3   3
#define REGNO_R4   4
#define REGNO_R5   5
#define REGNO_R6   6
#define REGNO_R7   7
#define REGNO_R8   8
#define REGNO_R9   9
#define REGNO_R10  10
#define REGNO_R11  11
#define REGNO_R12  12
#define REGNO_R13  13
#define REGNO_R14  14
#define REGNO_R15  15
#define REGNO_SR   16
#define REGNO_PC   17
#if defined(CONFIG_CPU_CSKYV2)
#define REGNO_R16  18
#define REGNO_R17  19
#define REGNO_R18  20
#define REGNO_R19  21
#define REGNO_R20  22
#define REGNO_R21  23
#define REGNO_R22  24
#define REGNO_R23  25
#define REGNO_R24  26
#define REGNO_R25  27
#define REGNO_R26  28
#define REGNO_R27  29
#define REGNO_R28  30
#define REGNO_R29  31
#define REGNO_R30  32
#define REGNO_R31  33
#define REGNO_RHI  34
#define REGNO_RLO  35
#endif
#define REGNO_USP  REGNO_R0

#ifndef __ASSEMBLY__

/* this struct defines the way the registers are stored on the
   stack during a system call. */
struct pt_regs {
         unsigned long   pc;
         long            r1;
         /*
          * When syscall fails, we must recover syscall arg (r2, modified when 
          * syscall return), Modified by Li Chunqiang  20050626.
          */
         long            syscallr2;	
         unsigned long   sr;
         long            r2;
         long            r3;
         long            r4;
         long            r5;
         long            r6;
         long            r7;
         long            r8;
         long            r9;
         long            r10;
         long            r11;
         long            r12;
         long            r13;
         long            r14;
         long            r15;
#if defined(CONFIG_CPU_CSKYV2)
         long            r16;
         long            r17;
         long            r18;
         long            r19;
         long            r20;
         long            r21;
         long            r22;
         long            r23;
         long            r24;
         long            r25;
         long            r26;
         long            r27;
         long            r28;
         long            r29;
         long            r30;
         long            r31;
         long            rhi;
         long            rlo;
#endif
};

/*
 * This is the extended stack used by the context
 * switcher: it's pushed after the normal "struct pt_regs".
 */
struct switch_stack {
         unsigned long   r8;
         unsigned long   r9;
         unsigned long   r10;
         unsigned long   r11;
         unsigned long   r12;
         unsigned long   r13;
         unsigned long   r14;
         unsigned long   r15;
#if defined(CONFIG_CPU_CSKYV2)
         unsigned long   r16;
         unsigned long   r17;
         unsigned long   r18;
         unsigned long   r19;
         unsigned long   r26;
         unsigned long   r27;
         unsigned long   r28;
         unsigned long   r29;
         unsigned long   r30;
#endif
};

/* Arbitrarily choose the same ptrace numbers as used by the Sparc code. */
#define PTRACE_GETREGS            12
#define PTRACE_SETREGS            13
#define PTRACE_GETFPREGS          14
#define PTRACE_SETFPREGS          15

#ifdef __KERNEL__

#ifndef PS_S
#define PS_S            0x80000000              /* Supervisor Mode */
#define PS_TM           0x0000c000              /* Trace mode */
#endif

#define user_mode(regs) (!((regs)->sr & PS_S))
#define instruction_pointer(regs) ((regs)->pc)
#define profile_pc(regs) instruction_pointer(regs)
#define user_stack(regs) (sw_usp)
extern void show_regs(struct pt_regs *);
#endif /* __KERNEL__ */
#endif /* __ASSEMBLY__ */
#endif /* _CSKY_PTRACE_H */
