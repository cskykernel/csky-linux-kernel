/*
 * linux/arch/csky/kernel/traps.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1993 1994 by Hamish Macdonald
 * Copyright (C) 2006  Hangzhou C-SKY Microsystems co.,ltd.
 * Copyright (C) 2006  Li Chunqiang (chunqiang_li@c-sky.com)
 * Copyright (C) 2009  Hu junshan <junshan_hu@c-sky.com>
 */

#include <linux/sched.h>
#include <linux/signal.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/module.h>
#include <linux/user.h>
#include <linux/string.h>
#include <linux/linkage.h>
#include <linux/init.h>
#include <linux/ptrace.h>
#include <linux/kallsyms.h>
#include <linux/rtc.h>

#include <asm/setup.h>
#include <asm/fpu.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/traps.h>
#include <asm/pgalloc.h>
#include <asm/machdep.h>
#include <asm/siginfo.h>

#include <asm/mmu_context.h>

extern void csky_tlb_init(void);
extern e_vector *_ramvec;
void show_registers(struct pt_regs *fp);
/* assembler routines */
asmlinkage void buserr(void);
asmlinkage void alignment(void);
asmlinkage void trap(void);
asmlinkage void trap1(void);
asmlinkage void trap2(void);
asmlinkage void trap3(void);
asmlinkage void system_call(void);
asmlinkage void inthandler(void);
asmlinkage void autohandler(void);
asmlinkage void fastautohandler(void);

asmlinkage void handle_tlbinvalidl(void);
asmlinkage void handle_tlbinvalids(void);
asmlinkage void handle_tlbmodified(void);
asmlinkage void handle_tlbmiss(void);
asmlinkage void handle_fpe(void);

/*
 * this must be called very early as the kernel might
 * use some instruction that are emulated on the 060
 */
void __init base_trap_init(void)
{

}

void __init per_cpu_trap_init(void)
{
#if defined(CONFIG_MMU)
	unsigned int cpu = smp_processor_id();

	csky_tlb_init();
	cpu_data[cpu].asid_cache = ASID_FIRST_VERSION;
	current_cpu_data.asid_cache = ASID_FIRST_VERSION;
	TLBMISS_HANDLER_SETUP();
#endif

	atomic_inc(&init_mm.mm_count);
	current->active_mm = &init_mm;
	BUG_ON(current->mm);
}

void __init trap_init (void)
{
	int i;

	per_cpu_trap_init();

	if (mach_trap_init)
		mach_trap_init();
	/*
	 *      There is a common trap handler and common interrupt
	 *      handler that handle almost every vector. We treat
	 *      the system call and bus error special, they get their
	 *      own first level handlers.
	 */
	for(i = 1; (i <= 31); i++)
	{
		_ramvec[i] = trap;
	}
	for(; (i < 128); i++)
	{
		_ramvec[i] = inthandler;
	}
	_ramvec[VEC_ACCESS] = buserr;
	_ramvec[VEC_ALIGN] = alignment;
	_ramvec[VEC_TRAP1]=trap1;
	_ramvec[VEC_TRAP2]=trap2;
	_ramvec[VEC_TRAP3]=trap3;
	_ramvec[VEC_SYS] = system_call;
	_ramvec[VEC_AUTOVEC] = autohandler;
	_ramvec[VEC_FAUTOVEC] = (void *)((unsigned int)fastautohandler | 1);

	_ramvec[VEC_TLBINVALIDL] = handle_tlbinvalidl;
	_ramvec[VEC_TLBINVALIDS] = handle_tlbinvalids;
	_ramvec[VEC_TLBMODIFIED] = handle_tlbmodified;
	_ramvec[VEC_TLBMISS] = handle_tlbmiss;
	_ramvec[VEC_TLBMISS] = (void *)((unsigned int)handle_tlbmiss | 1);
	_ramvec[VEC_FPE] = handle_fpe;
}

void die_if_kernel(char *,struct pt_regs *,int);
asmlinkage int do_page_fault(struct pt_regs *regs, unsigned long address,
		unsigned long error_code);

asmlinkage void trap_c(int vector, struct frame *fp);
asmlinkage void handle_fpe_c(unsigned long fesr, struct pt_regs * regs);

asmlinkage void buserr_c(struct frame *fp)
{
	printk("%s(%d): Bus Error Trap\n", __FILE__, __LINE__);
	show_registers((struct pt_regs *) fp);
	/* Only set esp0 if coming from user mode */
	if (user_mode(&fp->ptregs)){
		current->thread.esp0 = (unsigned long) fp;
	} else{
		die_if_kernel("Kernel mode BUS error", (struct pt_regs *)fp, 0);
	}
	force_sig(SIGSEGV, current);
}

int kstack_depth_to_print = 48;

/* MODULE_RANGE is a guess of how much space is likely to be
   vmalloced.  */
#define MODULE_RANGE (8*1024*1024)

void show_trace(unsigned long *stack)
{
	unsigned long *endstack;
	unsigned long addr;
	int i;

	printk("Call Trace:");
	addr = (unsigned long)stack + THREAD_SIZE - 1;
	endstack = (unsigned long *)(addr & -THREAD_SIZE);
	i = 0;
	while (stack + 1 <= endstack) {
		addr = *stack++;
		/*
		 * If the address is either in the text segment of the
		 * kernel, or in the region which contains vmalloc'ed
		 * memory, it *may* be the address of a calling
		 * routine; if so, print it so that someone tracing
		 * down the cause of the crash will be able to figure
		 * out the call path that was taken.
		 */
		if (__kernel_text_address(addr)) {
#ifndef CONFIG_KALLSYMS
			if (i % 5 == 0)
				printk("\n       ");
#endif
			printk(" [<%08lx>] %pS\n", addr, (void *)addr);
			i++;
		}
	}
	printk("\n");
}

void show_stack(struct task_struct *task, unsigned long *stack)
{
	unsigned long *p;
	unsigned long *endstack;
	int i;

	if (!stack) {
		if (task)
			stack = (unsigned long *)task->thread.esp0;
		else
			stack = (unsigned long *)&stack;
	}
	endstack = (unsigned long *)(((unsigned long)stack + THREAD_SIZE - 1) & -THREAD_SIZE);

	printk("Stack from %08lx:", (unsigned long)stack);
	p = stack;
	for (i = 0; i < kstack_depth_to_print; i++) {
		if (p + 1 > endstack)
			break;
		if (i % 8 == 0)
			printk("\n       ");
		printk(" %08lx", *p++);
	}
	printk("\n");
	show_trace(stack);
}

/*
 * The architecture-independent backtrace generator
 */
void dump_stack(void)
{
	unsigned long stack;

	show_trace(&stack);
}
EXPORT_SYMBOL(dump_stack);

void bad_super_trap (int vector, struct frame *fp)
{
	console_verbose();

	printk("Kernel: Bad trap from supervisor state, vector = %d\n", vector);
	printk("Current process id is %d\n", current->pid);
	show_registers((struct pt_regs *)fp);
	panic("Trap from supervisor state\n");
}

asmlinkage void trap_c(int vector, struct frame *fp)
{
	int sig;
	siginfo_t info;

	/* send the appropriate signal to the user program */
	switch (vector) {
		case VEC_ZERODIV:
			sig = SIGFPE;
			break;

		case VEC_TRACE:  /* ptrace single step */
			info.si_code = TRAP_TRACE;
			sig = SIGTRAP;
			break;

			/* fp->ptregs.sr &= ~PS_T */
		case VEC_BREAKPOINT: /* breakpoint */
			info.si_code = TRAP_BRKPT;
			sig = SIGTRAP;
			break;

		case VEC_TRAP1:    /* gdb server breakpoint */
			fp->ptregs.pc -= 2;
			sig = SIGTRAP;
			break;

		default:
			sig = SIGILL;
			break;
	}
	send_sig(sig, current, 0);
}

asmlinkage void handle_fpe_c(unsigned long fesr, struct pt_regs * regs)
{
	int sig;
	siginfo_t info;

	if(fesr & FPE_ILLE){
		info.si_code = ILL_ILLOPC;
		sig = SIGILL;
	}
	else if(fesr & FPE_IDC){
		info.si_code = ILL_ILLOPN;
		sig = SIGILL;
	}
	else if(fesr & FPE_FEC){
		sig = SIGFPE;
		if(fesr & FPE_IOC){
			info.si_code = FPE_FLTINV;
		}
		else if(fesr & FPE_DZC){
			info.si_code = FPE_FLTDIV;
		}
		else if(fesr & FPE_UFC){
			info.si_code = FPE_FLTUND;
		}
		else if(fesr & FPE_OFC){
			info.si_code = FPE_FLTOVF;
		}
		else if(fesr & FPE_IXC){
			info.si_code = FPE_FLTRES;
		}
		else {
			info.si_code = __SI_FAULT;
		}
	}
	else {
		info.si_code = __SI_FAULT;
		sig = SIGFPE;
	}
	info.si_signo = SIGFPE;
	info.si_errno = 0;
	info.si_addr = (void *)regs->pc;
	force_sig_info(sig, &info, current);
}

asmlinkage void set_esp0 (unsigned long ssp)
{
	current->thread.esp0 = ssp;
}

void show_trace_task(struct task_struct *tsk)
{
	/* DAVIDM: we can do better, need a proper stack dump */
	printk("STACK ksp=0x%lx, usp=0x%lx\n", tsk->thread.ksp, tsk->thread.usp);
}

void die_if_kernel (char *str, struct pt_regs *fp, int nr)
{
	if (!(fp->sr & PS_S))
		return;

	console_verbose();
	printk("%s: %08x\n",str,nr);
	show_registers(fp);
	add_taint(TAINT_DIE);
	do_exit(SIGSEGV);
}

/*
 *      Generic dumping code. Used for panic and debug.
 */
void show_registers(struct pt_regs *fp)
{
	unsigned long   *sp;
	unsigned char   *tp;
	int             i;

	printk("\nCURRENT PROCESS:\n\n");
	printk("COMM=%s PID=%d\n", current->comm, current->pid);

	if (current->mm) {
		printk("TEXT=%08x-%08x DATA=%08x-%08x BSS=%08x-%08x\n",
				(int) current->mm->start_code,
				(int) current->mm->end_code,
				(int) current->mm->start_data,
				(int) current->mm->end_data,
				(int) current->mm->end_data,
				(int) current->mm->brk);
		printk("USER-STACK=%08x  KERNEL-STACK=%08x\n\n",
				(int) current->mm->start_stack,
				(int) (((unsigned long) current) + 2 * PAGE_SIZE));
	}

	printk("PC: 0x%08lx\n", fp->pc);
	printk("PSR: 0x%08lx\n", (long) fp->sr);
	/*
	 * syscallr1->syscallr2
	 * please refer asm/ptrace.h
	 */
	printk("r1:  0x%08lx    syscallr2: 0x%08lx    r2: 0x%08lx    "
			"r3: 0x%08lx\n", fp->r1, fp->syscallr2, fp->r2, fp->r3);
	printk("r4:  0x%08lx    r5:  0x%08lx    r6:  0x%08lx    r7:  0x%08lx\n",
			fp->r4, fp->r5, fp->r6, fp->r7);
	printk("r8:  0x%08lx    r9:  0x%08lx    r10: 0x%08lx    r11: 0x%08lx\n",
			fp->r8, fp->r9, fp->r10, fp->r11);
	printk("r12: 0x%08lx    r13: 0x%08lx    r14: 0x%08lx    r15: 0x%08lx\n",
			fp->r12, fp->r13, fp->r14, fp->r15);
#if defined(CONFIG_CPU_CSKYV2)
	printk("r16: 0x%08lx    r17: 0x%08lx    r18: 0x%08lx    r19: 0x%08lx\n",
			fp->r16, fp->r17, fp->r18, fp->r19);
	printk("r20: 0x%08lx    r21: 0x%08lx    r22: 0x%08lx    r23: 0x%08lx\n",
			fp->r20, fp->r21, fp->r22, fp->r23);
	printk("r24: 0x%08lx    r25: 0x%08lx    r26: 0x%08lx    r27: 0x%08lx\n",
			fp->r24, fp->r25, fp->r26, fp->r27);
	printk("r28: 0x%08lx    r29: 0x%08lx    r30: 0x%08lx    r31: 0x%08lx\n",
			fp->r28, fp->r29, fp->r30, fp->r31);
	printk("hi 0x%08lx     lo: 0x%08lx \n",
			fp->rhi, fp->rlo);
#endif


	printk("\nCODE:");
	tp = ((unsigned char *) fp->pc) - 0x20;
	tp += ((int)tp % 4) ? 2 : 0;
	for (sp = (unsigned long *) tp, i = 0; (i < 0x40);  i += 4) {
		if ((i % 0x10) == 0)
			printk("\n%08x: ", (int) (tp + i));
		printk("%08x ", (int) *sp++);
	}
	printk("\n");

	printk("\nKERNEL STACK:");
	tp = ((unsigned char *) fp) - 0x40;
	for (sp = (unsigned long *) tp, i = 0; (i < 0xc0); i += 4) {
		if ((i % 0x10) == 0)
			printk("\n%08x: ", (int) (tp + i));
		printk("%08x ", (int) *sp++);
	}
	printk("\n");

	return;
}

#ifdef TRAP_DBG_INTERRUPT

asmlinkage void dbginterrupt_c(struct frame *fp)
{
	extern void show_registers(struct pt_regs *fp);
	printk("%s(%d): BUS ERROR TRAP\n", __FILE__, __LINE__);
	show_registers((struct pt_regs *) fp);
	asm("bkpt");
}

#endif

