/*
 * This program is used to generate definitions needed by
 * assembly language modules.
 *
 * We use the technique used in the OSF Mach kernel code:
 * generate asm statements containing #defines,
 * compile this file to assembler, and then extract the
 * #defines from the assembly-language output.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation. 
 *
 * Copyright (C) 2009  Hangzhou C-SKY Microsystems co.,ltd.
 * Copyright (C) 2009  Hu junshan<junshan_hu@c-sky.com>
 * 
 */
#include <linux/sched.h>
#include <linux/kernel_stat.h>
#include <linux/kbuild.h>

int main(void)
{
	/* offsets into the task struct */
	DEFINE(TASK_STATE,        offsetof(struct task_struct, state));
	DEFINE(TASK_THREAD_INFO,  offsetof(struct task_struct, stack));
	DEFINE(TASK_FLAGS,        offsetof(struct task_struct, flags));
	DEFINE(TASK_PTRACE,       offsetof(struct task_struct, ptrace));
	DEFINE(TASK_THREAD,       offsetof(struct task_struct, thread));
	DEFINE(TASK_MM,           offsetof(struct task_struct, mm));
	DEFINE(TASK_ACTIVE_MM,    offsetof(struct task_struct, active_mm));

	/* offsets into the thread struct */
	DEFINE(THREAD_KSP,        offsetof(struct thread_struct, ksp));
	DEFINE(THREAD_USP,        offsetof(struct thread_struct, usp));
	DEFINE(THREAD_SR,         offsetof(struct thread_struct, sr));
	DEFINE(THREAD_CRP,        offsetof(struct thread_struct, crp));
	DEFINE(THREAD_ESP0,       offsetof(struct thread_struct, esp0));
	DEFINE(THREAD_FSR,        offsetof(struct thread_struct, fsr));
	DEFINE(THREAD_FESR,       offsetof(struct thread_struct, fesr));
	DEFINE(THREAD_FPREG,      offsetof(struct thread_struct, fp));
	DEFINE(THREAD_DSPHI,      offsetof(struct thread_struct, hi));
	DEFINE(THREAD_DSPLO,      offsetof(struct thread_struct, lo));
	DEFINE(THREAD_DSPCSR,     offsetof(struct thread_struct, dspcsr));

	/* offsets into the thread_info struct */
	DEFINE(TINFO_FLAGS,       offsetof(struct thread_info, flags));
	DEFINE(TINFO_PREEMPT,     offsetof(struct thread_info, preempt_count));
	DEFINE(TINFO_ADDR_LIMIT,  offsetof(struct thread_info, addr_limit));
	DEFINE(TINFO_TP_VALURE,   offsetof(struct thread_info, tp_value));
	DEFINE(TINFO_TASK,        offsetof(struct thread_info, task));
	DEFINE(TINFO_EXEC_DOMAIN, offsetof(struct thread_info, exec_domain));

	/* offsets into the pt_regs */
	DEFINE(PT_PC,             offsetof(struct pt_regs, pc));
	DEFINE(PT_R1,             offsetof(struct pt_regs, r1));
	DEFINE(PT_SYSCALLR1,      offsetof(struct pt_regs, syscallr2));
	DEFINE(PT_SR,             offsetof(struct pt_regs, sr));
	DEFINE(PT_R2,             offsetof(struct pt_regs, r2));
	DEFINE(PT_R3,             offsetof(struct pt_regs, r3));
	DEFINE(PT_R4,             offsetof(struct pt_regs, r4));
	DEFINE(PT_R5,             offsetof(struct pt_regs, r5));
	DEFINE(PT_R6,             offsetof(struct pt_regs, r6));
	DEFINE(PT_R7,             offsetof(struct pt_regs, r7));
	DEFINE(PT_R8,             offsetof(struct pt_regs, r8));
	DEFINE(PT_R9,             offsetof(struct pt_regs, r9));
	DEFINE(PT_R10,            offsetof(struct pt_regs, r10));
	DEFINE(PT_R11,            offsetof(struct pt_regs, r11));
	DEFINE(PT_R12,            offsetof(struct pt_regs, r12));
	DEFINE(PT_R13,            offsetof(struct pt_regs, r13));
	DEFINE(PT_R14,            offsetof(struct pt_regs, r14));
	DEFINE(PT_R15,            offsetof(struct pt_regs, r15));
#if defined(CONFIG_CPU_CSKYV2)
	DEFINE(PT_R16,            offsetof(struct pt_regs, r16));
	DEFINE(PT_R17,            offsetof(struct pt_regs, r17));
	DEFINE(PT_R18,            offsetof(struct pt_regs, r18));
	DEFINE(PT_R19,            offsetof(struct pt_regs, r19));
	DEFINE(PT_R20,            offsetof(struct pt_regs, r20));
	DEFINE(PT_R21,            offsetof(struct pt_regs, r21));
	DEFINE(PT_R22,            offsetof(struct pt_regs, r22));
	DEFINE(PT_R23,            offsetof(struct pt_regs, r23));
	DEFINE(PT_R24,            offsetof(struct pt_regs, r24));
	DEFINE(PT_R25,            offsetof(struct pt_regs, r25));
	DEFINE(PT_R26,            offsetof(struct pt_regs, r26));
	DEFINE(PT_R27,            offsetof(struct pt_regs, r27));
	DEFINE(PT_R28,            offsetof(struct pt_regs, r28));
	DEFINE(PT_R29,            offsetof(struct pt_regs, r29));
	DEFINE(PT_R30,            offsetof(struct pt_regs, r30));
	DEFINE(PT_R31,            offsetof(struct pt_regs, r31));
	DEFINE(PT_RHI,            offsetof(struct pt_regs, rhi));
	DEFINE(PT_RLO,            offsetof(struct pt_regs, rlo));
#endif

	/* offsets into the irq_handler struct */
	DEFINE(IRQ_HANDLER,       offsetof(struct irq_node, handler));
	DEFINE(IRQ_DEVID,         offsetof(struct irq_node, dev_id));
	DEFINE(IRQ_NEXT,          offsetof(struct irq_node, next));

	/* offsets into the irq_cpustat_t struct */
	DEFINE(CPUSTAT_SOFTIRQ_PENDING, offsetof(irq_cpustat_t, __softirq_pending));

	
	/* signal defines */
	DEFINE(SIGSEGV, SIGSEGV);
	DEFINE(SIGTRAP, SIGTRAP);
	
	return 0;
}
