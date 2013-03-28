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
  
#ifndef _ASM_CSKY_THREAD_INFO_H
#define _ASM_CSKY_THREAD_INFO_H

#ifndef __ASSEMBLY__

#include <asm/types.h>
#include <asm/page.h>

struct thread_info {
	struct task_struct	  *task;	     /* main task structure */
	unsigned long		  flags;
	struct exec_domain	  *exec_domain;  /* execution domain */
	int					  preempt_count; /* 0 => preemptable, <0 => BUG */
	unsigned long         tp_value;      /* thread pointer */
	mm_segment_t          addr_limit;
	
	struct restart_block  restart_block;
	struct pt_regs		  *regs;
};

#define INIT_THREAD_INFO(tsk)				\
{											\
	.task		= &tsk,						\
	.exec_domain	= &default_exec_domain,	\
	.preempt_count  = 1,	\
	.addr_limit     = KERNEL_DS,     		\
	.restart_block = {						\
		.fn = do_no_restart_syscall,		\
	},										\
}

/* THREAD_SIZE should be 8k, so handle differently for 4k and 8k machines */
#define THREAD_SIZE_ORDER (13 - PAGE_SHIFT)

#define init_thread_info	(init_thread_union.thread_info)
#define init_stack		(init_thread_union.stack)

static inline struct thread_info *current_thread_info(void)
{
    unsigned long sp;
    
    __asm__ __volatile__(
                        "mov %0, r0\n\t"
                        :"=r"(sp));

    return (struct thread_info *)(sp & ~(THREAD_SIZE - 1));
}

#endif /* !__ASSEMBLY__ */

#define PREEMPT_ACTIVE		0x4000000

/* entry.S relies on these definitions!
 * bits 0-7 are tested at every exception exit
 * bits 8-15 are also tested at syscall exit
 */
#define TIF_SIGPENDING          6       /* signal pending */
#define TIF_NEED_RESCHED        7       /* rescheduling necessary */
#define TIF_DELAYED_TRACE       14      /* single step a syscall */
#define TIF_SYSCALL_TRACE       15      /* syscall trace active */
#define TIF_MEMDIE              16
#define TIF_FREEZE              17      /* thread is freezing for suspend */

#endif	/* _ASM_CSKY_THREAD_INFO_H */
