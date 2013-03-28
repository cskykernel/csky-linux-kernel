/*
 * linux/arch/csky/kernel/ptrace.c
 *
 * Taken from linux/kernel/ptrace.c and modified for csky.
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of
 * this archive for more details.
 *
 * Copyright (C) 2006  Hangzhou C-SKY Microsystems co.,ltd.
 * Copyright (C) 2006  Li Chunqiang (chunqiang_li@c-sky.com)
 * Copyright (C) 2009  Hu junshan<junshan_hu@c-sky.com>
 *
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/smp.h>
#include <linux/smp_lock.h>
#include <linux/errno.h>
#include <linux/ptrace.h>
#include <linux/user.h>
#include <linux/signal.h>

#include <asm/uaccess.h>
#include <asm/page.h>
#include <asm/pgtable.h>
#include <asm/system.h>
#include <asm/processor.h>
#include <asm/asm-offsets.h>

/*
 * does not yet catch signals sent when the child dies.
 * in exit.c or in signal.c.
 */

/* sets the trace bits. */
#define TRACE_MODE_SI      1 << 14
#define TRACE_MODE_RUN     0
#define TRACE_MODE_JMP     0x11 << 14
#define TRACE_MODE_MASK    ~(0x11 << 14)

/*
 * PT_xxx is the stack offset at which the register is  saved. 
 * Notice that usp has no stack-slot and needs to be treated
 * specially (see get_reg/put_reg below). 
 */
static int regoff[] = {
	-1,          PT_R1,  PT_R2,  PT_R3,	
	PT_R4,  	 PT_R5,  PT_R6,  PT_R7,
	PT_R8,  	 PT_R9,  PT_R10, PT_R11,
	PT_R12,		 PT_R13, PT_R14, PT_R15,
	PT_SR,		 PT_PC,
#if defined(CONFIG_CPU_CSKYV2)
	PT_R16,      PT_R17,  PT_R18,  PT_R19,
	PT_R20,          PT_R21,  PT_R22,  PT_R23,
	PT_R24,          PT_R25,  PT_R26,  PT_R27,
	PT_R28,          PT_R29,  PT_R30,  PT_R31,
	PT_RHI,      PT_RLO,
#endif
};

/*
 * Get contents of register REGNO in task TASK.
 */
static inline long get_reg(struct task_struct *task, int regno)
{
	unsigned long *addr;

	if (regno == REGNO_USP)
		addr = &task->thread.usp;
	else if (regno < sizeof(regoff)/sizeof(regoff[0]))
		addr = (unsigned long *)(task->thread.esp0 + regoff[regno]);
	else
		return 0;
	return *addr;
}

/*
 * Write contents of register REGNO in task TASK.
 */
static inline int put_reg(struct task_struct *task, int regno,
			  unsigned long data)
{
	unsigned long *addr;

	if (regno == REGNO_USP)
		addr = &task->thread.usp;
	else if (regno < sizeof(regoff)/sizeof(regoff[0]))
		addr = (unsigned long *) (task->thread.esp0 + regoff[regno]);
	else
		return -1;
	*addr = data;
	return 0;
}
/*
 * Make sure the single step bit is not set.
 */
static inline void singlestep_disable(struct task_struct *child)
{
	unsigned long tmp; 
	tmp = (get_reg(child, REGNO_SR) & TRACE_MODE_MASK) | TRACE_MODE_RUN;
	put_reg(child, REGNO_SR, tmp);
	/* FIXME maybe wrong here: if clear flag of TIF_DELAYED_TRACE? */
}
/*
 * Make sure the single step bit is set.
 */
static inline void singlestep_enable(struct task_struct *child)
{
	unsigned long tmp;
    tmp = (get_reg(child, REGNO_SR) & TRACE_MODE_MASK) | TRACE_MODE_SI;
    put_reg(child, REGNO_SR, tmp);
	/* FIXME maybe wrong here: if set flag of TIF_DELAYED_TRACE? */

}

int ptrace_getfpregs(struct task_struct *child, void __user *data)
{

	if (!access_ok(VERIFY_WRITE, data, sizeof(struct user_cskyfp_struct)))
		return -EIO;

	if(copy_to_user(data, &child->thread.fsr, 
		                sizeof(struct user_cskyfp_struct)))
		return -EFAULT;

	return 0;
}

int ptrace_setfpregs(struct task_struct *child, void __user *data)
{
	if (!access_ok(VERIFY_READ, data, sizeof(struct user_cskyfp_struct)))
		return -EIO;
	
	if(copy_from_user(&child->thread.fsr, data,
                        sizeof(struct user_cskyfp_struct)))
        return -EFAULT;
	return 0;
}

/*
 * Called by kernel/ptrace.c when detaching..
 *
 * Make sure the single step bit is not set.
 */
void ptrace_disable(struct task_struct *child)
{
	singlestep_disable(child);
}

extern void _flush_cache_all(void);
/*
 * Handle the requests of ptrace system call.
 *
 * INPUT:
 * child   - process being traced.
 * request - request type.
 * addr    - address of data that this request to read from or write to.
 * data    - address of data that this request to read to or write from.
 *
 * RETURN: 
 * 0       - success
 * others  - fail
 */
long arch_ptrace(struct task_struct *child, long request, long addr, long data)
{
	unsigned long tmp;
	int i, ret = 0;

	switch (request) {
	/* when I and D space are separate, these will need to be fixed. */
	case PTRACE_PEEKTEXT:   /* read word at location addr. */
	case PTRACE_PEEKDATA:
		ret = generic_ptrace_peekdata(child, addr, data);
		break;

	/* read the word at location addr in the USER area. */
	case PTRACE_PEEKUSR:
		if (addr & 3)
			goto out_eio;
		addr >>= 2;     /* temporary hack. */

		if (addr >= 0 && addr < 18) {
			tmp = get_reg(child, addr);
		}else if(addr == 18) {
			tmp = child->thread.hi;
		}else if(addr == 19) {
			tmp = child->thread.lo;
		}else if(addr >= 20 && addr <= 51) {
			tmp = child->thread.fp[addr - 20];
		} else
			break;
		ret = put_user(tmp, (unsigned long *)data);
		break;

	/* when I and D space are separate, this will have to be fixed. */
	case PTRACE_POKETEXT:   /* write the word at location addr. */
	case PTRACE_POKEDATA:
		_flush_cache_all ();
		ret = generic_ptrace_pokedata(child, addr, data);
		break;

	case PTRACE_POKEUSR:  /* write the word at location addr in the USER area */
		if (addr & 3)
			goto out_eio;
		addr >>= 2;     /* temporary hack. */

		if (addr >= 0 && addr < 18) {
			if (put_reg(child, addr, data)) /*** should protect 'psr'? ***/
				goto out_eio;
		}else if(addr == 18) {
			child->thread.hi = data;
		}else if(addr == 19) {
			child->thread.lo = data;
		}else if(addr >= 20 && addr <= 51) {
			child->thread.fp[addr - 20] =data;
		} else
			goto out_eio;
    	break;

	case PTRACE_SYSCALL:  /* continue and stop at next (return from) syscall */
	case PTRACE_CONT:     /* restart after signal. */
		_flush_cache_all ();
		if (!valid_signal(data))
			goto out_eio;

		if (request == PTRACE_SYSCALL)
			set_tsk_thread_flag(child, TIF_SYSCALL_TRACE);
		else
			clear_tsk_thread_flag(child, TIF_SYSCALL_TRACE);
		child->exit_code = data;
		singlestep_disable(child);
		wake_up_process(child);
		break;

	/*
	 * make the child exit.  Best I can do is send it a sigkill.
	 * perhaps it should be put in the status that it wants to
	 * exit.
	 */
	case PTRACE_KILL:
		if (child->exit_state == EXIT_ZOMBIE) /* already dead */
			break;
		child->exit_code = SIGKILL;
		singlestep_disable(child);
		wake_up_process(child);
		break;

	case PTRACE_SINGLESTEP: /* set the trap flag. */
		if (!valid_signal(data))
			goto out_eio;
		_flush_cache_all ();
		clear_tsk_thread_flag(child, TIF_SYSCALL_TRACE);
		singlestep_enable(child);
			
		child->exit_code = data;
		/* give it a chance to run. */
		wake_up_process(child);
		break;

	case PTRACE_GETREGS:    /* Get all gp regs from the child. */
		for (i = 0; i < 18; i++) {
			tmp = get_reg(child, i);
			ret = put_user(tmp, (unsigned long *)data);
			if (ret)
				break;
			data += sizeof(long);
		}
		break;

	case PTRACE_SETREGS:    /* Set all gp regs in the child. */
		for (i = 0; i < 18; i++) {
			ret = get_user(tmp, (unsigned long *)data);
			if (ret)
				break;
			put_reg(child, i, tmp);
			data += sizeof(long); 
		}
		break;

	case PTRACE_GETFPREGS:
		ret = ptrace_getfpregs(child, (void  __user *) data);
		break;

	case PTRACE_SETFPREGS:
		ret = ptrace_setfpregs(child, (void __user *) data);
		break;

	default:
		ret = ptrace_request(child, request, addr, data);
		break;
	}

	return ret;
out_eio:
	return -EIO;
}

/*
 * If process's system calls is traces, do some corresponding handles in this
 * fuction before entering system call function and after exiting system call
 * fuction.
 */
asmlinkage void syscall_trace(void)
{
	ptrace_notify(SIGTRAP | ((current->ptrace & PT_TRACESYSGOOD)
                                 ? 0x80 : 0));
	/*
	 * this isn't the same as continuing with a signal, but it will do
	 * for normal use.  strace only continues with a signal if the
	 * stopping signal is not SIGTRAP.  -brl
	 */
	if (current->exit_code) {
		send_sig(current->exit_code, current, 1);
		current->exit_code = 0;
	}

}
