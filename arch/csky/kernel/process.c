/*
 * linux/arch/csky/kernel/process.c
 * This file handles the architecture-dependent parts of process handling..
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006  Hangzhou C-SKY Microsystems co.,ltd.
 * Copyright (C) 2006  Li Chunqiang (chunqiang_li@c-sky.com)
 * Copyright (C) 2009  Hu junshan<junshan_hu@c-sky.com>
 *
 */

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/tick.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/smp.h>
#include <linux/smp_lock.h>
#include <linux/stddef.h>
#include <linux/unistd.h>
#include <linux/ptrace.h>
#include <linux/slab.h>
#include <linux/user.h>
#include <linux/init_task.h>
#include <linux/reboot.h>
#include <linux/mqueue.h>
#include <linux/rtc.h>

#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/traps.h>
#include <asm/machdep.h>
#include <asm/setup.h>
#include <asm/pgtable.h>
#include <asm/user.h>
#include <asm/fpu.h>

struct cpuinfo_csky cpu_data[NR_CPUS];

/*
 * Initial task structure. Make this a per-architecture thing,
 * because different architectures tend to have different
 * alignment requirements and potentially different initial
 * setup.
 */
static struct fs_struct init_fs = INIT_FS;
static struct signal_struct init_signals = INIT_SIGNALS(init_signals);
static struct sighand_struct init_sighand = INIT_SIGHAND(init_sighand);
struct mm_struct init_mm = INIT_MM(init_mm);

EXPORT_SYMBOL(init_mm);

union thread_union init_thread_union
__attribute__((section(".data.init_task"), aligned(THREAD_SIZE)))
       = { INIT_THREAD_INFO(init_task) };

/* initial task structure */
struct task_struct init_task = INIT_TASK(init_task);

EXPORT_SYMBOL(init_task);

asmlinkage void ret_from_fork(void);

/*
 * Return saved PC from a blocked thread
 */
unsigned long thread_saved_pc(struct task_struct *tsk)
{
	struct switch_stack *sw = (struct switch_stack *)tsk->thread.ksp;

	return sw->r15;
}
/*
 * The idle loop on an csky
 */
static void default_idle(void)
{
	__asm__ __volatile__(
//	                     "wait \n\t"
	                     "mov   r0, r0 \n\t"
	                     ::);
}

void (*idle)(void) = default_idle;

/*
 * The idle thread. There's no useful work to be
 * done, so just try to conserve power and have a
 * low exit latency (ie sit in a loop waiting for
 * somebody to say that they'd like to reschedule)
 */
void cpu_idle(void)
{
	/* endless idle loop with no priority at all */
	while (1) {
		while (!need_resched())
			idle();
		preempt_enable_no_resched();
		schedule();
		preempt_disable();
	}
}

void machine_restart(char * __unused)
{
	/* close mmu */
	asm volatile (
		"mfcr 	r7, cr18;"
		"bclri 	r7, 0;"
		"bclri 	r7, 1;"
		"mtcr   r7, cr18;"
		:
		:
		:"r7"
	);

	/* jmp */
	((void (*)(u32 WakeTime,u32 GpioMask,u32 GpioData,u32 key))0x00100000)
			(*(unsigned int *) (0x00103000 - 0x10),
			*(unsigned int *) (0x00103000 - 0xc),
			*(unsigned int *) (0x00103000 - 0x8),
			*(unsigned int *) (0x00103000 - 0x4));
}

void machine_halt(void)
{
	if (mach_halt)
		mach_halt();
	for (;;);
}

void machine_power_off(void)
{
	if (mach_power_off)
		mach_power_off();
	for (;;);
}

void (*pm_power_off)(void) = machine_power_off;
EXPORT_SYMBOL(pm_power_off);

void show_regs(struct pt_regs * regs)
{
	printk("\n");
	printk("PC: %08lx  Status: %04lx    %s\n",
	        regs->pc, regs->sr, print_tainted());
	printk("origin_r2: %08lx  r1: %08lx  r2: %08lx  r3: %08lx\n",
	       regs->syscallr2, regs->r1, regs->r2, regs->r3);
	printk("r4: %08lx  r5: %08lx  r6: %08lx\n",
	       regs->r4, regs->r5, regs->r6);
	printk("r7: %08lx  r8: %08lx  r9: %08lx\n",
	       regs->r7, regs->r8, regs->r9);
	printk("r10: %08lx  r11: %08lx  r12: %08lx\n",
	       regs->r10, regs->r11, regs->r12);
	printk("r13: %08lx  r14: %08lx  r15: %08lx\n",
	       regs->r13, regs->r14, regs->r15);
#if defined(CONFIG_CPU_CSKYV2)
	printk("r16: %08lx  r17: %08lx  r18: %08lx\n",
	       regs->r16, regs->r17, regs->r18);
	printk("r19: %08lx  r20: %08lx  r21: %08lx\n",
	       regs->r19, regs->r20, regs->r21);
	printk("r22: %08lx  r23: %08lx  r24: %08lx\n",
	       regs->r22, regs->r23, regs->r24);
	printk("r25: %08lx  r26: %08lx  r27: %08lx\n",
	       regs->r25, regs->r26, regs->r27);
	printk("r28: %08lx  r29: %08lx  r30: %08lx\n",
	       regs->r28, regs->r29, regs->r30);
	printk("r31: %08lx  rhi: %08lx  rlo: %08lx\n",
	       regs->r31, regs->rhi, regs->rlo);
#endif
	if (!(regs->sr & PS_S))
		printk("USP: %08lx\n", rdusp());
}

/*
 * Create a kernel thread
 */
int kernel_thread(int (*fn)(void *), void * arg, unsigned long flags)
{
	long retval;
	long clone_arg;
	clone_arg = flags | CLONE_VM ;
	
	set_fs (KERNEL_DS);
	__asm__ __volatile__ (
			"mov	r7, %2\n\t"    /* Save function for later */
			"mov	r3, r0\n\t"    /* Syscall arg[2] = sp */
			"mov	r2, %3\n\t"    /* Syscall arg[1] = flags */
			"movi   r1, %4\n\t"    /* Syscall arg[0] = __NR_clone */
			"trap   0\n\t"         /* Linux CKcore system call */
			"cmpne	r0, r3\n\t"    /* Child or Parent */
			"bf	    1f\n\t"        /* Parent ... jump */
			"mov	r2, %1\n\t"    /* Push argument */
			"jsr	r7\n\t"        /* Call fn */
			"movi	r1, %5\n\t"    /* Exit */
			"trap	0\n\t"
			"1:\n\t"
			"mov    %0, r2"        /* return value */
			: "=r" (retval)
			: "r" (arg),
			  "r" (fn),
			  "r" (clone_arg),
			  "i" (__NR_clone),
			  "i" (__NR_exit)
			: "r1", "r2", "r3", "r7");

	return retval;
}

EXPORT_SYMBOL(kernel_thread);

void flush_thread(void)
{

}

/*
 * "csky_fork()".. By the time we get here, the
 * non-volatile registers have also been saved on the
 * stack. We do some ugly pointer stuff here.. (see
 * also copy_thread)
 */

asmlinkage int csky_fork(struct pt_regs *regs)
{
#ifdef CONFIG_MMU
	return do_fork(SIGCHLD, rdusp(), regs, 0,NULL,NULL);
#else
	/* can not support in nommu mode */
	return(-EINVAL);
#endif

}

asmlinkage int csky_vfork(struct pt_regs *regs)
{
	return do_fork(CLONE_VFORK | CLONE_VM | SIGCHLD, rdusp(), regs, 0,NULL,NULL);
}

asmlinkage int csky_clone(struct pt_regs *regs)
{
	unsigned long clone_flags;
	unsigned long newsp;
	int __user *parent_tidptr, *child_tidptr;

	/* syscall2 puts clone_flags in r2 and usp in r3 */
	clone_flags = regs->r2;
	newsp = regs->r3;
	parent_tidptr = (int __user *)regs->r4;
	child_tidptr = (int __user *)regs->r5;
	if (!newsp)
		newsp = rdusp();
	return do_fork(clone_flags, newsp, regs, 0, parent_tidptr, child_tidptr);
}

int copy_thread(int nr, unsigned long clone_flags,
		unsigned long usp, unsigned long unused,
		struct task_struct * p, struct pt_regs * regs)
{
	struct thread_info *ti = task_thread_info(p);
	struct pt_regs * childregs;
	struct switch_stack * childstack;

	childregs = (struct pt_regs *) (task_stack_page(p) + THREAD_SIZE) - 1;

	 /*FIXME: There's may not be preempt_disable and preempt_enable!*/
	preempt_disable();
#ifdef CONFIG_CPU_HAS_FPU
	save_fp_to_thread(p->thread.fp, &p->thread.fsr, &p->thread.fesr);
#endif
#if defined(CONFIG_CPU_HAS_DSP) || defined(__CK810__)
	__asm__ __volatile__("mfhi    %0 \n\r"
		                 "mflo    %1 \n\r"
		                 :"=r"(p->thread.hi), "=r"(p->thread.lo)
		                 : );
#endif
	preempt_enable();

	*childregs = *regs;
	/*Return 0 for subprocess when return from fork(),vfork(),clone()*/
	childregs->r2 = 0;

	childstack = ((struct switch_stack *) childregs) - 1;
	memset(childstack, 0, sizeof(struct switch_stack));
	childstack->r15 = (unsigned long)ret_from_fork;

	p->thread.usp = usp;
	p->thread.ksp = (unsigned long)childstack;
	
	if (clone_flags & CLONE_SETTLS)
		ti->tp_value = regs->r3;

	return 0;
}

/* Fill in the fpu structure for a core dump.  */

int dump_fpu (struct pt_regs *regs, struct user_cskyfp_struct *fpu)
{
	memcpy(fpu, &current->thread.fsr, sizeof(*fpu));
	return 1;
}

EXPORT_SYMBOL(dump_fpu);

/*
 * fill in the user structure for a core dump..
 *
 * this function is not used in linux 2.6
 */
void dump_thread(struct pt_regs * regs, struct user * dump)
{
}

asmlinkage int __sys_execve(char __user *name, char __user * __user *argv, char __user * __user *envp, struct pt_regs *regs)
{
	/* struct pt_regs *regs; */ /* commended by sunkang 2004.11.18 */
	int error;
	char * filename;

	lock_kernel();
	filename = getname(name);
	error = PTR_ERR(filename);
	if (IS_ERR(filename))
		goto out;
	error = do_execve(filename, argv, envp, regs);
	putname(filename);
out:
	unlock_kernel();
	return error;
}
/*
 * sys_execve() executes a new program.
 */

asmlinkage int sys_execve(char __user *name, char __user * __user *argv, char __user * __user *envp)
{
	register struct pt_regs *regs __asm__ ("r5");
	__asm__ __volatile__("mov %0, r0\n\t"  \
	                     "addi %0, 8\n\t"  \
	                     : "=r"(regs)      \
	                     :);
	return __sys_execve(name, argv, envp, regs);
}

/*
 * These bracket the sleeping functions..
 */

unsigned long get_wchan(struct task_struct *p)
{
	unsigned long esp, pc;
	unsigned long stack_page;
	int count = 0;
	if (!p || p == current || p->state == TASK_RUNNING)
		return 0;

	stack_page = (unsigned long)p;
	esp = p->thread.esp0;
	do {
		if (esp < stack_page+sizeof(struct task_struct) ||
		    esp >= 8184+stack_page)
			return 0;
		/*FIXME: There's may be error here!*/
		pc = ((unsigned long *)esp)[1];
		/* FIXME: This depends on the order of these functions. */
		if (!in_sched_functions(pc))
			return pc;
		esp = *(unsigned long *) esp;
	} while (count++ < 16);
	return 0;
}
