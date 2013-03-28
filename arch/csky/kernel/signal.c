/*
 * linux/arch/csky/kernel/signal.c
 *
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * 1997-12-01  Modified for POSIX.1b signals by Andreas Schwab
 *
 * mathemu support by Roman Zippel
 * (Note: fpstate in the signal context is completly ignored for the emulator
 *         and the internal floating point format is put on stack)
 *
 * ++roman (07/09/96): implemented signal stacks (specially for tosemu on
 * Atari :-) Current limitation: Only one sigstack can be active at one time.
 * If a second signal with SA_ONSTACK set arrives while working on a sigstack,
 * SA_ONSTACK is ignored. This behaviour avoids lots of trouble with nested
 * signal handlers!
 *
 * Copyright (C) 2009 Hangzhou C-SKY Microsystems co.,ltd.
 */

#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/signal.h>
#include <linux/syscalls.h>
#include <linux/errno.h>
#include <linux/wait.h>
#include <linux/ptrace.h>
#include <linux/unistd.h>
#include <linux/stddef.h>
#include <linux/highuid.h>
#include <linux/personality.h>
#include <linux/tty.h>
#include <linux/binfmts.h>

#include <asm/setup.h>
#include <asm/uaccess.h>
#include <asm/pgtable.h>
#include <asm/traps.h>
#include <asm/ucontext.h>

#define _BLOCKABLE (~(sigmask(SIGKILL) | sigmask(SIGSTOP)))


asmlinkage int do_signal(sigset_t *oldset, struct pt_regs *regs);

/*
 * Atomically swap in the new signal mask, and wait for a signal.
 */
asmlinkage int do_sigsuspend(struct pt_regs *regs)
{
	old_sigset_t mask = regs->r2; //FIXME: perhaps not correct here!
	sigset_t saveset;

	mask &= _BLOCKABLE;
	saveset = current->blocked;
	siginitset(&current->blocked, mask);
	recalc_sigpending( );

	regs->r2 = -EINTR;
	while (1) {
		current->state = TASK_INTERRUPTIBLE;
		schedule();
		if (do_signal(&saveset, regs))
			return regs->r2;
	}
}

asmlinkage int do_rt_sigsuspend(struct pt_regs *regs)
{
	/*FIXME: perhaps not correct here!*/
	sigset_t *unewset = (sigset_t *)regs->r2;
	size_t sigsetsize = (size_t)regs->r3;
	sigset_t saveset, newset;

	/* XXX: Don't preclude handling different sized sigset_t's.  */
	if (sigsetsize != sizeof(sigset_t))
		return -EINVAL;

	if (copy_from_user(&newset, unewset, sizeof(newset)))
		return -EFAULT;
	sigdelsetmask(&newset, ~_BLOCKABLE);

	saveset = current->blocked;
	current->blocked = newset;
	recalc_sigpending( );

	regs->r2 = -EINTR;
	while (1) {
		current->state = TASK_INTERRUPTIBLE;
		schedule();
		if (do_signal(&saveset, regs))
			return regs->r2;
	}
}

asmlinkage int sys_sigaction(int sig, const struct old_sigaction *act,
		struct old_sigaction *oact)
{
	struct k_sigaction new_ka, old_ka;
	int ret;

	if (act) {
		old_sigset_t mask;
		if (verify_area(VERIFY_READ, act, sizeof(*act)) ||
				__get_user(new_ka.sa.sa_handler, &act->sa_handler) ||
				__get_user(new_ka.sa.sa_restorer, &act->sa_restorer))
			return -EFAULT;
		__get_user(new_ka.sa.sa_flags, &act->sa_flags);
		__get_user(mask, &act->sa_mask);
		siginitset(&new_ka.sa.sa_mask, mask);
	}

	ret = do_sigaction(sig, act ? &new_ka : NULL, oact ? &old_ka : NULL);

	if (!ret && oact) {
		if (verify_area(VERIFY_WRITE, oact, sizeof(*oact)) ||
				__put_user(old_ka.sa.sa_handler, &oact->sa_handler) ||
				__put_user(old_ka.sa.sa_restorer, &oact->sa_restorer))
			return -EFAULT;
		__put_user(old_ka.sa.sa_flags, &oact->sa_flags);
		__put_user(old_ka.sa.sa_mask.sig[0], &oact->sa_mask);
	}

	return ret;
}

asmlinkage int sys_sigaltstack(const stack_t *uss, stack_t *uoss)
{
	return do_sigaltstack(uss, uoss, rdusp());
}


/*
 * Do a signal return; undo the signal stack.
 *
 * Keep the return code on the stack quadword aligned!
 * That makes the cache flush below easier.
 */

struct sigframe {
	int sig;
	int code;
	struct sigcontext *psc;
	short retcode[4];
	unsigned long extramask[_NSIG_WORDS-1];
	struct sigcontext sc;
};

struct rt_sigframe {
	int sig;
	struct siginfo *pinfo;
	void *puc;
	short retcode[4];
	struct siginfo info;
	struct ucontext uc;
};

typedef struct fpregset {
	int f_fsr;
	int f_fesr;
	int f_feinst1;
	int f_feinst2;
	int f_fpregs[32];
} fpregset_t;

static inline int restore_fpu_state(struct sigcontext *sc)
{
	int err = 0;
#ifdef CONFIG_CPU_HAS_FPU
	fpregset_t fpregs;
	unsigned long flg;
	unsigned long tmp1, tmp2, tmp3, tmp4;
	unsigned long fctl1, fctl2;
	int * fpgr;

	if (__copy_from_user(&fpregs, &sc->sc_fsr, sizeof(fpregs))) {
		err = 1;
		goto out;
	}

	local_irq_save(flg);
	fctl1 = fpregs.f_fsr;
	fctl2 = fpregs.f_fesr;
	fpgr = &(fpregs.f_fpregs[0]);
	__asm__ __volatile__("cpseti 1\n\r"
			"cpwcr  %5, cpcr2 \n\r"
			"cpwcr  %6, cpcr4 \n\r"
			"ldw    %0, (%4, 0) \n\r"
			"ldw    %1, (%4, 4) \n\r"
			"ldw    %2, (%4, 8) \n\r"
			"ldw    %3, (%4, 12) \n\r"
			"fmts   %0, fr0 \n\r"
			"fmts   %1, fr1 \n\r"
			"fmts   %2, fr2 \n\r"
			"fmts   %3, fr3 \n\r"
			"ldw    %0, (%4, 16) \n\r"
			"ldw    %1, (%4, 20) \n\r"
			"ldw    %2, (%4, 24) \n\r"
			"ldw    %3, (%4, 28) \n\r"
			"fmts   %0, fr4 \n\r"
			"fmts   %1, fr5 \n\r"
			"fmts   %2, fr6 \n\r"
			"fmts   %3, fr7 \n\r"
			"ldw    %0, (%4, 32) \n\r"
			"ldw    %1, (%4, 36) \n\r"
			"ldw    %2, (%4, 40) \n\r"
			"ldw    %3, (%4, 44) \n\r"
			"fmts   %0, fr8 \n\r"
			"fmts   %1, fr9 \n\r"
			"fmts   %2, fr10 \n\r"
			"fmts   %3, fr11 \n\r"
			"ldw    %0, (%4, 48) \n\r"
			"ldw    %1, (%4, 52) \n\r"
			"ldw    %2, (%4, 56) \n\r"
			"ldw    %3, (%4, 60) \n\r"
			"fmts   %0, fr12 \n\r"
			"fmts   %1, fr13 \n\r"
			"fmts   %2, fr14 \n\r"
			"fmts   %3, fr15 \n\r"
			"addi   %4, 32 \n\r"
			"addi   %4, 32 \n\r"
			"ldw    %0, (%4, 0) \n\r"
			"ldw    %1, (%4, 4) \n\r"
			"ldw    %2, (%4, 8) \n\r"
			"ldw    %3, (%4, 12) \n\r"
			"fmts   %0, fr16 \n\r"
			"fmts   %1, fr17 \n\r"
			"fmts   %2, fr18 \n\r"
			"fmts   %3, fr19 \n\r"
			"ldw    %0, (%4, 16) \n\r"
			"ldw    %1, (%4, 20) \n\r"
			"ldw    %2, (%4, 24) \n\r"
			"ldw    %3, (%4, 28) \n\r"
			"fmts   %0, fr20 \n\r"
			"fmts   %1, fr21 \n\r"
			"fmts   %2, fr22 \n\r"
			"fmts   %3, fr23 \n\r"
			"ldw    %0, (%4, 32) \n\r"
			"ldw    %1, (%4, 36) \n\r"
			"ldw    %2, (%4, 40) \n\r"
			"ldw    %3, (%4, 44) \n\r"
			"fmts   %0, fr24 \n\r"
			"fmts   %1, fr25 \n\r"
			"fmts   %2, fr26 \n\r"
			"fmts   %3, fr27 \n\r"
			"ldw    %0, (%4, 48) \n\r"
			"ldw    %1, (%4, 52) \n\r"
			"ldw    %2, (%4, 56) \n\r"
			"ldw    %3, (%4, 60) \n\r"
			"fmts   %0, fr28 \n\r"
			"fmts   %1, fr29 \n\r"
			"fmts   %2, fr30 \n\r"
			"fmts   %3, fr31 \n\r"
			:"=r"(tmp1), "=r"(tmp2),"=r"(tmp3),"=r"(tmp4),
		"+r"(fpgr), "+r"(fctl1), "+r"(fctl2) );
	local_irq_restore(flg);
out:
#endif
	return err;
}

static inline int restore_sigframe(struct pt_regs *regs, struct sigcontext *usc, int *pr2)
{
	int err = 0;
	unsigned long usp;

	/* Always make any pending restarted system calls return -EINTR */
	current_thread_info()->restart_block.fn = do_no_restart_syscall;

	/* restore passed registers */
	err |= __get_user(regs->r1, &usc->sc_r1);
	err |= __get_user(regs->r2, &usc->sc_r2);
	err |= __get_user(regs->r3, &usc->sc_r3);
	err |= __get_user(regs->r4, &usc->sc_r4);
	err |= __get_user(regs->r5, &usc->sc_r5);
	err |= __get_user(regs->r6, &usc->sc_r6);
	err |= __get_user(regs->r7, &usc->sc_r7);
	err |= __get_user(regs->r8, &usc->sc_r8);
	err |= __get_user(regs->r9, &usc->sc_r9);
	err |= __get_user(regs->r10, &usc->sc_r10);
	err |= __get_user(regs->r11, &usc->sc_r11);
	err |= __get_user(regs->r12, &usc->sc_r12);
	err |= __get_user(regs->r13, &usc->sc_r13);
	err |= __get_user(regs->r14, &usc->sc_r14);
	err |= __get_user(regs->r15, &usc->sc_r15);
#if defined(CONFIG_CPU_CSKYV2)
	err |= __get_user(regs->r16, &usc->sc_r16);
	err |= __get_user(regs->r17, &usc->sc_r17);
	err |= __get_user(regs->r18, &usc->sc_r18);
	err |= __get_user(regs->r19, &usc->sc_r19);
	err |= __get_user(regs->r20, &usc->sc_r20);
	err |= __get_user(regs->r21, &usc->sc_r21);
	err |= __get_user(regs->r22, &usc->sc_r22);
	err |= __get_user(regs->r23, &usc->sc_r23);
	err |= __get_user(regs->r24, &usc->sc_r24);
	err |= __get_user(regs->r25, &usc->sc_r25);
	err |= __get_user(regs->r26, &usc->sc_r26);
	err |= __get_user(regs->r27, &usc->sc_r27);
	err |= __get_user(regs->r28, &usc->sc_r28);
	err |= __get_user(regs->r29, &usc->sc_r29);
	err |= __get_user(regs->r30, &usc->sc_r30);
	err |= __get_user(regs->r31, &usc->sc_r31);
	err |= __get_user(regs->rhi, &usc->sc_rhi);
	err |= __get_user(regs->rlo, &usc->sc_rlo);
#endif
	err |= __get_user(regs->sr, &usc->sc_sr);
	err |= __get_user(regs->pc, &usc->sc_pc);
	err |= __get_user(usp, &usc->sc_usp);
	regs->syscallr2 = -1;
	wrusp(usp);

	err |= restore_fpu_state(usc);

	*pr2 = regs->r2;
	return err;
}

asmlinkage int do_sigreturn(struct pt_regs *regs)
{
	unsigned long usp = rdusp();
	struct sigframe *frame = (struct sigframe *)usp;
	sigset_t set;
	int r2;

	if (verify_area(VERIFY_READ, frame, sizeof(*frame)))
		goto badframe;
	if (__get_user(set.sig[0], &frame->sc.sc_mask) ||
			(_NSIG_WORDS > 1 &&
			 __copy_from_user(&set.sig[1], &frame->extramask,
				 sizeof(frame->extramask))))
		goto badframe;

	sigdelsetmask(&set, ~_BLOCKABLE);
	spin_lock_irq(&current->sighand->siglock);
	current->blocked = set;
	recalc_sigpending( );
	spin_unlock_irq(&current->sighand->siglock);

	if (restore_sigframe(regs, &frame->sc, &r2))
		goto badframe;
	return r2;

badframe:
	force_sig(SIGSEGV, current);
	return 0;
}

asmlinkage int do_rt_sigreturn(struct pt_regs *regs)
{
	unsigned long usp = rdusp();
	struct rt_sigframe *frame = (struct rt_sigframe *)usp;
	sigset_t set;
	int r2;

	if (verify_area(VERIFY_READ, frame, sizeof(*frame)))
		goto badframe;
	if (__copy_from_user(&set, &frame->uc.uc_sigmask, sizeof(set)))
		goto badframe;

	sigdelsetmask(&set, ~_BLOCKABLE);
	spin_lock_irq(&current->sighand->siglock);
	current->blocked = set;
	recalc_sigpending( );
	spin_unlock_irq(&current->sighand->siglock);

	if (restore_sigframe(regs, &frame->uc.uc_mcontext, &r2))
		goto badframe;

	if (do_sigaltstack(&frame->uc.uc_stack, NULL, usp) == -EFAULT)
		goto badframe;

	return r2;

badframe:
	force_sig(SIGSEGV, current);
	return 0;
}

/*
 * Set up a signal frame.
 */

static inline int save_fpu_state(struct sigcontext *sc, struct pt_regs *regs)
{
	int err = 0;
#ifdef CONFIG_CPU_HAS_FPU
	fpregset_t fpregs;
	unsigned long flg;
	unsigned long tmp1, tmp2, tmp3, tmp4;
	int * fpgr;

	local_irq_save(flg);
	fpgr = &(fpregs.f_fpregs[0]);
	__asm__ __volatile__("cpseti 1\n\r"
			"cprcr  %5, cpcr2 \n\r"
			"cprcr  %6, cpcr4 \n\r"
			"cprcr  %7, cpcr5 \n\r"
			"btsti  %6, 30 \n\r"
			"bf     1f  \n\r"
			"cprcr  %8, cpcr6 \n\r"
			"1:         \n"
			"fmfs   %0, fr0 \n\r"
			"fmfs   %1, fr1 \n\r"
			"fmfs   %2, fr2 \n\r"
			"fmfs   %3, fr3 \n\r"
			"stw    %0, (%4, 0) \n\r"
			"stw    %1, (%4, 4) \n\r"
			"stw    %2, (%4, 8) \n\r"
			"stw    %3, (%4, 12) \n\r"
			"fmfs   %0, fr4 \n\r"
			"fmfs   %1, fr5 \n\r"
			"fmfs   %2, fr6 \n\r"
			"fmfs   %3, fr7 \n\r"
			"stw    %0, (%4, 16) \n\r"
			"stw    %1, (%4, 20) \n\r"
			"stw    %2, (%4, 24) \n\r"
			"stw    %3, (%4, 28) \n\r"
			"fmfs   %0, fr8 \n\r"
			"fmfs   %1, fr9 \n\r"
			"fmfs   %2, fr10 \n\r"
			"fmfs   %3, fr11 \n\r"
			"stw    %0, (%4, 32) \n\r"
			"stw    %1, (%4, 36) \n\r"
			"stw    %2, (%4, 40) \n\r"
			"stw    %3, (%4, 44) \n\r"
			"fmfs   %0, fr12 \n\r"
			"fmfs   %1, fr13 \n\r"
			"fmfs   %2, fr14 \n\r"
			"fmfs   %3, fr15 \n\r"
			"stw    %0, (%4, 48) \n\r"
			"stw    %1, (%4, 52) \n\r"
			"stw    %2, (%4, 56) \n\r"
			"stw    %3, (%4, 60) \n\r"
			"addi   %4,32 \n\r"
			"addi   %4,32 \n\r"
			"fmfs   %0, fr16 \n\r"
			"fmfs   %1, fr17 \n\r"
			"fmfs   %2, fr18 \n\r"
			"fmfs   %3, fr19 \n\r"
			"stw    %0, (%4, 0) \n\r"
			"stw    %1, (%4, 4) \n\r"
			"stw    %2, (%4, 8) \n\r"
			"stw    %3, (%4, 12) \n\r"
			"fmfs   %0, fr20 \n\r"
			"fmfs   %1, fr21 \n\r"
			"fmfs   %2, fr22 \n\r"
			"fmfs   %3, fr23 \n\r"
			"stw    %0, (%4, 16) \n\r"
			"stw    %1, (%4, 20) \n\r"
			"stw    %2, (%4, 24) \n\r"
			"stw    %3, (%4, 28) \n\r"
			"fmfs   %0, fr24 \n\r"
			"fmfs   %1, fr25 \n\r"
			"fmfs   %2, fr26 \n\r"
			"fmfs   %3, fr27 \n\r"
			"stw    %0, (%4, 32) \n\r"
			"stw    %1, (%4, 36) \n\r"
			"stw    %2, (%4, 40) \n\r"
			"stw    %3, (%4, 44) \n\r"
			"fmfs   %0, fr28 \n\r"
			"fmfs   %1, fr29 \n\r"
			"fmfs   %2, fr30 \n\r"
			"fmfs   %3, fr31 \n\r"
			"stw    %0, (%4, 48) \n\r"
			"stw    %1, (%4, 52) \n\r"
			"stw    %2, (%4, 56) \n\r"
			"stw    %3, (%4, 60) \n\r"
			:"=a"(tmp1), "=a"(tmp2),"=a"(tmp3),"=a"(tmp4),
		"+a"(fpgr), "=a"(fpregs.f_fsr),
		"=a"(fpregs.f_fesr), "=a"(fpregs.f_feinst1),
		"=b"(fpregs.f_feinst2));

	local_irq_restore(flg);

	err |= copy_to_user(&sc->sc_fsr, &fpregs, sizeof(fpregs));
#endif
	return err;
}

static inline int setup_sigframe(struct sigcontext *sc, struct pt_regs *regs,
		unsigned long mask)
{
	int err = 0;

	err |= __put_user(mask, &sc->sc_mask);
	err |= __put_user(rdusp(), &sc->sc_usp);
	err |= __put_user(regs->r1, &sc->sc_r1);
	err |= __put_user(regs->r2, &sc->sc_r2);
	err |= __put_user(regs->r3, &sc->sc_r3);
	err |= __put_user(regs->r4, &sc->sc_r4);
	err |= __put_user(regs->r5, &sc->sc_r5);
	err |= __put_user(regs->r6, &sc->sc_r6);
	err |= __put_user(regs->r7, &sc->sc_r7);
	err |= __put_user(regs->r8, &sc->sc_r8);
	err |= __put_user(regs->r9, &sc->sc_r9);
	err |= __put_user(regs->r10, &sc->sc_r10);
	err |= __put_user(regs->r11, &sc->sc_r11);
	err |= __put_user(regs->r12, &sc->sc_r12);
	err |= __put_user(regs->r13, &sc->sc_r13);
	err |= __put_user(regs->r14, &sc->sc_r14);
	err |= __put_user(regs->r15, &sc->sc_r15);
#if defined(CONFIG_CPU_CSKYV2)
	err |= __put_user(regs->r16, &sc->sc_r16);
	err |= __put_user(regs->r17, &sc->sc_r17);
	err |= __put_user(regs->r18, &sc->sc_r18);
	err |= __put_user(regs->r19, &sc->sc_r19);
	err |= __put_user(regs->r20, &sc->sc_r20);
	err |= __put_user(regs->r21, &sc->sc_r21);
	err |= __put_user(regs->r22, &sc->sc_r22);
	err |= __put_user(regs->r23, &sc->sc_r23);
	err |= __put_user(regs->r24, &sc->sc_r24);
	err |= __put_user(regs->r25, &sc->sc_r25);
	err |= __put_user(regs->r26, &sc->sc_r26);
	err |= __put_user(regs->r27, &sc->sc_r27);
	err |= __put_user(regs->r28, &sc->sc_r28);
	err |= __put_user(regs->r29, &sc->sc_r29);
	err |= __put_user(regs->r30, &sc->sc_r30);
	err |= __put_user(regs->r31, &sc->sc_r31);
	err |= __put_user(regs->rhi, &sc->sc_rhi);
	err |= __put_user(regs->rlo, &sc->sc_rlo);
#endif
	err |= __put_user(regs->sr, &sc->sc_sr);
	err |= __put_user(regs->pc, &sc->sc_pc);
	err |= save_fpu_state(sc, regs);

	return err;
}

static inline void push_cache (unsigned long vaddr)
{
	int value = 0x33;

	__asm__ __volatile__("mtcr %0,cr17\n\t"
			"sync\n\t"
			: :"r" (value));
}

static inline void *get_sigframe(struct k_sigaction *ka, struct pt_regs *regs, size_t frame_size)
{
	unsigned long usp;

	/* Default to using normal stack.  */
	usp = rdusp();

	/* This is the X/Open sanctioned signal stack switching.  */
	if (ka->sa.sa_flags & SA_ONSTACK) {
		if (!on_sig_stack(usp))
			usp = current->sas_ss_sp + current->sas_ss_size;
	}
	return (void *)((usp - frame_size) & -8UL);
}

static void setup_frame (int sig, struct k_sigaction *ka,
		sigset_t *set, struct pt_regs *regs)
{
	struct sigframe *frame;
	int fsize = 0;
	int err = 0;
	unsigned long retcode;

	if (fsize < 0) {
#ifdef DEBUG
		printk ("setup_frame: Unknown frame format %#x\n", regs->format);
#endif
		goto give_sigsegv;
	}

	frame = get_sigframe(ka, regs, sizeof(*frame) + fsize);

	if (fsize) {
		err |= copy_to_user (frame + 1, regs + 1, fsize);
	}

	err |= __put_user((current_thread_info()->exec_domain
				&& current_thread_info()->exec_domain->signal_invmap
				&& sig < 32
				? current_thread_info()->exec_domain->signal_invmap[sig]
				: sig),
			&frame->sig);

	err |= __put_user(&frame->sc, &frame->psc);

	if (_NSIG_WORDS > 1)
		err |= copy_to_user(frame->extramask, &set->sig[1],
				sizeof(frame->extramask));

	err |= setup_sigframe(&frame->sc, regs, set->sig[0]);

	retcode = (unsigned long)(frame->retcode);

#if defined(CONFIG_CPU_CSKYV1)
	/* movi r2,r2 (ignore); movi #__NR_sigreturn,r1; trap #0 */
	err |= __put_user(0x6000 + (__NR_sigreturn << 4) + 1,
			(frame->retcode + 0));
	err |= __put_user(0x08, (frame->retcode + 1));
#else
	/*
	 * movi #__NR_sigreturn,r1; trap #0
	 * FIXME: For CSKY V2 ISA, we mast write instruction in half word, because
	 *  the CPU load instruction by half word and ignore endian format. So the
	 *  high half word in 32 bit instruction mast local in low address.
	 */
	err |= __put_user(0x9C00 + 1, (frame->retcode + 0));
	err |= __put_user(__NR_sigreturn, (frame->retcode + 1));
	err |= __put_user(0xC000, (frame->retcode + 2));
	err |= __put_user(0x2020, (frame->retcode + 3));
#endif

	if (err)
		goto give_sigsegv;

	push_cache ((unsigned long) &frame->retcode);

	/* Set up registers for signal handler */
	wrusp ((unsigned long) frame);
	regs->pc = (unsigned long) ka->sa.sa_handler;
	/*
	 * Return to user space. Maybe it must return to user space
	 * (if so, we comment this line)
	 */
	if (!(regs->sr & 0x80000000))
	{
		regs->r15 = retcode;
	}
	regs->r2=sig;
	return;

give_sigsegv:
	if (sig == SIGSEGV)
		ka->sa.sa_handler = SIG_DFL;
	force_sig(SIGSEGV, current);
}

static void setup_rt_frame (int sig, struct k_sigaction *ka, siginfo_t *info,
		sigset_t *set, struct pt_regs *regs)
{
	struct rt_sigframe *frame;
	int fsize = 0;
	int err = 0;
	unsigned long retcode;

	if (fsize < 0) {
#ifdef DEBUG
		printk ("setup_rt_frame: Unknown frame format %#x\n",
				regs->format);
#endif
		goto give_sigsegv;
	}

	frame = get_sigframe(ka, regs, sizeof(*frame));


	err |= __put_user((current_thread_info()->exec_domain
				&& current_thread_info()->exec_domain->signal_invmap
				&& sig < 32
				? current_thread_info()->exec_domain->signal_invmap[sig]
				: sig),
			&frame->sig);
	err |= __put_user(&frame->info, &frame->pinfo);
	err |= __put_user(&frame->uc, &frame->puc);
	err |= copy_siginfo_to_user(&frame->info, info);

	/* Create the ucontext.  */
	err |= __put_user(0, &frame->uc.uc_flags);
	err |= __put_user(0, &frame->uc.uc_link);
	err |= __put_user((void *)current->sas_ss_sp,
			&frame->uc.uc_stack.ss_sp);
	err |= __put_user(sas_ss_flags(rdusp()),
			&frame->uc.uc_stack.ss_flags);
	err |= __put_user(current->sas_ss_size, &frame->uc.uc_stack.ss_size);
	err |= setup_sigframe(&frame->uc.uc_mcontext, regs, 0);
	err |= copy_to_user (&frame->uc.uc_sigmask, set, sizeof(*set));

	/* Set up to return from userspace.  */
	retcode = (unsigned long)frame->retcode;

#if defined(CONFIG_CPU_CSKYV1)
	/* movi r1,127; addi r1,33, addi r1,(#__NR_sigreturn-127-33); trap #0 */
	err |= __put_user(0x6000 + (127 << 4)+1, (frame->retcode + 0));
	err |= __put_user(0x2000 + (31  << 4)+1, (frame->retcode + 1));
	err |= __put_user(0x2000 + ((__NR_rt_sigreturn-127-33)  << 4)+1,
			(frame->retcode + 2));
	err |= __put_user(0x08, (frame->retcode + 3));
#else
	/*
	 * movi #__NR_rt_sigreturn,r1; trap #0
	 * FIXME: For CSKY V2 ISA, we mast write instruction in half word, because
	 *  the CPU load instruction by half word and ignore endian format. So the
	 *  high half word in 32 bit instruction mast local in low address.
	 */
	err |= __put_user(0x9C00 + 1, (frame->retcode + 0));
	err |= __put_user(__NR_rt_sigreturn, (frame->retcode + 1));
	err |= __put_user(0xC000, (frame->retcode + 2));
	err |= __put_user(0x2020, (frame->retcode + 3));
#endif

	if (err)
		goto give_sigsegv;

	push_cache ((unsigned long) &frame->retcode);

	/* Set up registers for signal handler */
	wrusp ((unsigned long) frame);
	regs->pc = (unsigned long) ka->sa.sa_handler;
	/*
	 * Return to user space. Maybe it must return to user space
	 * (if so, we comment this line)
	 */
	if (!(regs->sr & 0x80000000))
	{
		regs->r15 = retcode; //frame->retcode;
	}

adjust_stack:
	regs->r2 = sig; /* first arg is signo */
	regs->r3 = (unsigned long)(&(frame->info)); /* second arg is (siginfo_t*) */
	regs->r4 = (unsigned long)(&(frame->uc));/* third arg pointer to ucontext */
	return;

give_sigsegv:
	if (sig == SIGSEGV)
		ka->sa.sa_handler = SIG_DFL;
	force_sig(SIGSEGV, current);
	goto adjust_stack;
}

static inline void handle_restart(struct pt_regs *regs, struct k_sigaction *ka, int has_handler)
{
	switch (regs->r2) {
		case -ERESTARTNOHAND:
		case -ERESTART_RESTARTBLOCK:
			if (!has_handler)
				goto do_restart;
			regs->r2 = -EINTR;
			break;

		case -ERESTARTSYS:
			if (has_handler && !(ka->sa.sa_flags & SA_RESTART)) {
				regs->r2 = -EINTR;
				break;
			}
			/* fallthrough */
		case -ERESTARTNOINTR:
do_restart:
			regs->r2 = regs->syscallr2;
#if defined(CONFIG_CPU_CSKYV1)
			regs->pc -= 2;
#else
			regs->pc -= 4;
#endif
			break;
	}
}

void ptrace_signal_deliver(struct pt_regs *regs, void *cookie)
{
	if (regs->syscallr2 < 0)
		return;
	switch (regs->r2) {
		case -ERESTARTNOHAND:
		case -ERESTARTSYS:
		case -ERESTARTNOINTR:
			regs->r2 = regs->syscallr2;
			regs->syscallr2 = -1;
#if defined(CONFIG_CPU_CSKYV1)
			regs->pc -= 2;
#else
			regs->pc -= 4;
#endif
			break;
	}
}



/*
 * OK, we're invoking a handler
 */
static void handle_signal(int sig, struct k_sigaction *ka, siginfo_t *info,
		sigset_t *oldset, struct pt_regs *regs)
{
	/* are we from a system call? */
	if (regs->syscallr2 >= 0)
		/* If so, check system call restarting.. */
		handle_restart(regs, ka, 1);

	/* set up the stack frame */
	if (ka->sa.sa_flags & SA_SIGINFO)
		setup_rt_frame(sig, ka, info, oldset, regs);
	else
		setup_frame(sig, ka, oldset, regs);

	if (ka->sa.sa_flags & SA_ONESHOT)
		ka->sa.sa_handler = SIG_DFL;

	if (!(ka->sa.sa_flags & SA_NODEFER)) {
		spin_lock_irq(&current->sighand->siglock);
		sigorsets(&current->blocked,&current->blocked,&ka->sa.sa_mask);
		sigaddset(&current->blocked,sig);
		recalc_sigpending( );
		spin_unlock_irq(&current->sighand->siglock);
	}
}

/*
 * Note that 'init' is a special process: it doesn't get signals it doesn't
 * want to handle. Thus you cannot kill init even with a SIGKILL even by
 * mistake.
 *
 * Note that we go through the signals twice: once to check the signals
 * that the kernel can handle, and then we build all the user-level signal
 * handling stack-frames in one go after that.
 */
asmlinkage int do_signal(sigset_t *oldset, struct pt_regs *regs)
{
	siginfo_t info;
	struct k_sigaction ka;
	int signr;

	current->thread.esp0 = (unsigned long) regs;

	if (!oldset)
		oldset = &current->blocked;
	signr = get_signal_to_deliver(&info, &ka, regs, NULL);
	if (signr > 0) {
		/* Whee!  Actually deliver the signal.  */
		handle_signal(signr, &ka, &info, oldset, regs);
		return 1;
	}
	/* Did we come from a system call? */
	if (regs->syscallr2 >= 0)
		/* Restart the system call - no handlers present */
		handle_restart(regs, NULL, 0);

	return 0;
}

