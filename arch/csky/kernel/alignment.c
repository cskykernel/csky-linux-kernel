/*
 *  arch/arm/kernel/alignment.c  - handle alignment exceptions for CSKY CPU.
 *
 *  Copyright (C) 2011, C-SKY Microsystems Co., Ltd. (www.c-sky.com)
 *  Copyright (C) 2011, Hu Junshan (junshan_hu@c-sky.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/compiler.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/proc_fs.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/uaccess.h>

#include <asm/unaligned.h>

extern void die_if_kernel(char *, struct pt_regs *, long);

#ifdef CONFIG_SOFT_HANDMISSALIGN

static unsigned long ai_user;
static unsigned long ai_sys;
static unsigned long ai_skipped;
static unsigned long ai_half;
static unsigned long ai_word;
static unsigned long ai_qword;
static int ai_usermode;

#define UM_WARN		(1 << 0)
#define UM_FIXUP	(1 << 1)
#define UM_SIGNAL	(1 << 2)

#ifdef CONFIG_PROC_FS
static const char *usermode_action[] = {
	"ignored",
	"warn",
	"fixup",
	"fixup+warn",
	"signal",
	"signal+warn"
};

static int
proc_alignment_read(char *page, char **start, off_t off, int count, int *eof,
		    void *data)
{
	char *p = page;
	int len;

	p += sprintf(p, "User:\t\t%lu\n", ai_user);
	p += sprintf(p, "System:\t\t%lu\n", ai_sys);
	p += sprintf(p, "Skipped:\t%lu\n", ai_skipped);
	p += sprintf(p, "Half:\t\t%lu\n", ai_half);
	p += sprintf(p, "Word:\t\t%lu\n", ai_word);
	p += sprintf(p, "Qword:\t\t%lu\n", ai_qword);
	p += sprintf(p, "User faults:\t%i (%s)\n", ai_usermode,
			usermode_action[ai_usermode]);

	len = (p - page) - off;
	if (len < 0)
		len = 0;

	*eof = (len <= count) ? 1 : 0;
	*start = page + off;

	return len;
}

static int proc_alignment_write(struct file *file, const char __user *buffer,
				unsigned long count, void *data)
{
	char mode;

	if (count > 0) {
		if (get_user(mode, buffer))
			return -EFAULT;
		if (mode >= '0' && mode <= '5')
			ai_usermode = mode - '0';
	}
	return count;
}

#endif /* CONFIG_PROC_FS */

#ifdef  __cskyBE__
#define BE		1
#define FIRST_BYTE_16	"rotri	%1, 8\n"
#define FIRST_BYTE_32	"rotri	%1, 24\n"
#define NEXT_BYTE	"rotri  %1, 24\n"
#else
#define BE		0
#define FIRST_BYTE_16
#define FIRST_BYTE_32
#define NEXT_BYTE	"lsri   %1, 8\n"
#endif

#define __get8_unaligned_check(val,addr,err)		\
	__asm__(					\
	"1:	ldb	%1, (%2)\n"			\
	"	addi	%2, 1\n"			\
	"	br	3f\n"				\
	"2:	movi	%0, 1\n"			\
	"	br	3f\n"				\
	"	.section __ex_table,\"a\"\n"		\
	"	.align	2\n"				\
	"	.long	1b, 2b\n"			\
	"	.previous\n"				\
	"3:\n"						\
	: "=r" (err), "=r" (val), "=r" (addr)		\
	: "0" (err), "2" (addr))

#define get16_unaligned_check(val,addr)				\
	do {							\
		unsigned int err = 0, v, a = addr;		\
		__get8_unaligned_check(v,a,err);		\
		val =  v << ((BE) ? 8 : 0);			\
		__get8_unaligned_check(v,a,err);		\
		val |= v << ((BE) ? 0 : 8);			\
		if (err)					\
			goto fault;				\
	} while (0)

#define get32_unaligned_check(val,addr)				\
	do {							\
		unsigned int err = 0, v, a = addr;		\
		__get8_unaligned_check(v,a,err);		\
		val =  v << ((BE) ? 24 :  0);			\
		__get8_unaligned_check(v,a,err);		\
		val |= v << ((BE) ? 16 :  8);			\
		__get8_unaligned_check(v,a,err);		\
		val |= v << ((BE) ?  8 : 16);			\
		__get8_unaligned_check(v,a,err);		\
		val |= v << ((BE) ?  0 : 24);			\
		if (err)					\
			goto fault;				\
	} while (0)

#define put16_unaligned_check(val,addr)				\
	do {							\
		unsigned int err = 0, v = val, a = addr;	\
		__asm__( FIRST_BYTE_16				\
		"1:	stb	%1, (%2)\n"			\
		"	addi	%2, 1\n"			\
			NEXT_BYTE				\
		"2:	stb	%1, (%2)\n"			\
		"	br	4f\n"				\
		"3:	movi	%0, 1\n"			\
		"	br	4f\n"				\
		"	.section __ex_table,\"a\"\n"		\
		"	.align	2\n"				\
		"	.long	1b, 3b\n"			\
		"	.long	2b, 3b\n"			\
		"	.previous\n"				\
		"4:\n"						\
		: "=r" (err), "=r" (v), "=r" (a)		\
		: "0" (err), "1" (v), "2" (a));			\
		if (err)					\
			goto fault;				\
	} while (0)

#define put32_unaligned_check(val,addr)				\
	do {							\
		unsigned int err = 0, v = val, a = addr;	\
		__asm__( FIRST_BYTE_32				\
		"1:	stb	%1, (%2)\n"			\
		"	addi	%2, 1\n"			\
			NEXT_BYTE				\
		"2:	stb	%1, (%2)\n"			\
		"	addi	%2, 1\n"			\
			NEXT_BYTE				\
		"3:	stb	%1, (%2)\n"			\
		"	addi	%2, 1\n"			\
			NEXT_BYTE				\
		"4:	stb	%1, (%2)\n"			\
		"	br	6f\n"				\
		"5:	movi	%0, 1\n"			\
		"	br	6f\n"				\
		"	.section __ex_table,\"a\"\n"		\
		"	.align	2\n"				\
		"	.long	1b, 5b\n"			\
		"	.long	2b, 5b\n"			\
		"	.long	3b, 5b\n"			\
		"	.long	4b, 5b\n"			\
		"	.previous\n"				\
		"6:\n"						\
		: "=r" (err), "=r" (v), "=r" (a)		\
		: "0" (err), "1" (v), "2" (a));			\
		if (err)					\
			goto fault;				\
	} while (0)


static int alignment_handle_ldhsth(unsigned long addr, unsigned long instr, struct pt_regs *regs)
{
	unsigned int regz = (instr >> 8) & 0xf;

	ai_half += 1;

	if (instr & 0x1000)  // store
	{
		long dataregz;
		if(regz == 0) {
			__asm__ __volatile__("mfcr %0, ss0 \n\r"
					     :"=r"(dataregz));
		} else if(regz == 1){
			dataregz = regs->r1;
		} else
		{
			dataregz = *((long *)regs + (regz + 2));
		}

		put16_unaligned_check(dataregz, addr);
	}
	else
	{
		unsigned short val;
		long * datapt = NULL;
		get16_unaligned_check(val, addr);
		if(regz == 0) {
			goto fault;  // SP(r0) need not handle
		} else if(regz == 1){
			regs->r1 = (long)val;
		} else
		{
			datapt = (long *)regs + (regz + 2);
			*datapt = (long)val;
		}
	}
	return 1;

fault:
	return 0;
}

static int alignment_handle_ldwstw(unsigned long addr, unsigned long instr, struct pt_regs *regs)
{
	unsigned int regz = (instr >> 8) & 0xf;

	ai_word += 1;
	
	if (instr & 0x1000)  // store
	{
		long dataregz;
		if(regz == 0) {
			__asm__ __volatile__("mfcr %0, ss0 \n\r"
						:"=r"(dataregz));
		} else if(regz == 1){
			dataregz = regs->r1;
		} else
		{
			dataregz = *((long *)regs + (regz + 2));
		}
	
		put32_unaligned_check(dataregz, addr);
	}
	else
	{
		long val;
		long * datapt = NULL;
		get32_unaligned_check(val, addr);
		if(regz == 0) {
			goto fault;  // SP(r0) need not handle;
		} else if(regz == 1){
			regs->r1 = val;
		} else
		{
			datapt = (long *)regs + (regz + 2);
			*datapt = val;
		}
	}
	return 1;

fault:
	return 0;
}

static int alignment_handle_ldqstq(unsigned long addr, unsigned long instr, struct pt_regs *regs)
{
	ai_qword += 1;
	
	if (instr & 0x1000)  // store
	{
		long dataregz = regs->r4;

		put32_unaligned_check(dataregz, addr);
		addr += 1;
		dataregz = regs->r5;
		put32_unaligned_check(dataregz, addr);
		addr += 1;
		dataregz = regs->r6;
		put32_unaligned_check(dataregz, addr);
		addr += 1;
		dataregz = regs->r7;
		put32_unaligned_check(dataregz, addr);
	}
	else
	{
		long val;
		get32_unaligned_check(val, addr);
		regs->r4 = val;
		addr += 1;
		get32_unaligned_check(val, addr);
		regs->r5 = val;
		addr += 1;
		get32_unaligned_check(val, addr);
		regs->r6 = val;
		addr += 1;
		get32_unaligned_check(val, addr);
		regs->r7 = val;
	}
	return 1;

fault:
	return 0;
}

asmlinkage void alignment_c(struct pt_regs *regs)
{
	int err;
	unsigned long instrptr, srcaddr;
	unsigned int fault;
	u16 tinstr = 0;
	int (*handler)(unsigned long addr, unsigned long instr, struct pt_regs *regs);
	mm_segment_t fs;

	instrptr = instruction_pointer(regs);

	fs = get_fs();
	set_fs(KERNEL_DS);
	fault = __get_user(tinstr, (u16 *)(instrptr & ~1));
	set_fs(fs);
	if (fault) {
		goto bad_or_fault;
	}

	if(((short)(regs->pc) & 1))
		goto bad_or_fault;

	if (user_mode(regs))
		goto user;
	
	ai_sys += 1;

fixup:
	regs->pc +=2;

	if(tinstr & 0x8000)   // ld/st
	{
		int imm4 = (tinstr >> 4) & 0xf;
		int regx = tinstr & 0xf;
		if(regx == 0) {
			__asm__ __volatile__("mfcr %0, ss0 \n\r"
				                 :"=r"(srcaddr));
		} else if(regx == 1){
			srcaddr = (unsigned long )regs->r1;
		} else
		{
			srcaddr = *((int*)regs + (regx + 2));
		}
		
		if(tinstr & 0x4000)   // ldh/sth
		{
			srcaddr += imm4 << 1;
			handler = alignment_handle_ldhsth;
		}
		else    // ldw/stw
		{
			srcaddr += imm4 << 2;
			handler = alignment_handle_ldwstw;
		}
	}
	else if ((tinstr & 0x60) == 0x40){ // ldq/stq
		int regx = tinstr & 0xf;
		if(regx == 0) {
			__asm__ __volatile__("mfcr %0, ss0 \n\r"
				                 :"=r"(srcaddr));
		} else if(regx == 1){
			srcaddr = (unsigned long )regs->r1;
		} else
		{
			srcaddr = *((int *)regs + (regx + 2));
		}
		handler = alignment_handle_ldqstq;
	}
	else {
		// FIXME: sourse reg of ldm/stm is r0(stack pointer). It may
		//     lead to unrecover exception if r0 unalign. So ignore it.
		goto bad_or_fault;
	}
	
	if (!handler)
		goto bad_or_fault;

	err = handler(srcaddr, tinstr, regs);
	if (!err)
	{
		regs->pc -=2;
		goto bad_or_fault;
	}

	return;

bad_or_fault:
	ai_skipped += 1;
	if(fixup_exception(regs)) {
	    return;
	}
	die_if_kernel("Kernel mode Alignment exception", regs, 0);
	return;

user:
	ai_user += 1;

	if (ai_usermode & UM_WARN)
		printk("Alignment trap: %s(pid=%d) PC=0x%x Ins=0x%x\n",
			current->comm, current->pid, (unsigned int)regs->pc, tinstr);

	if (ai_usermode & UM_FIXUP)
		goto fixup;

	if (ai_usermode & UM_SIGNAL)
		force_sig(SIGBUS, current);

	return;
}

/*
 * This needs to be done after sysctl_init, otherwise sys/ will be
 * overwritten.  Actually, this shouldn't be in sys/ at all since
 * it isn't a sysctl, and it doesn't contain sysctl information.
 * We now locate it in /proc/cpu/alignment instead.
 */
static int __init alignment_init(void)
{
#ifdef CONFIG_PROC_FS
	struct proc_dir_entry *res;

	res = proc_mkdir("cpu", NULL);
	if (!res)
		return -ENOMEM;

	res = create_proc_entry("alignment", S_IWUSR | S_IRUGO, res);
	if (!res)
		return -ENOMEM;

	res->read_proc = proc_alignment_read;
	res->write_proc = proc_alignment_write;
#endif

	ai_usermode = UM_FIXUP;

	return 0;
}

fs_initcall(alignment_init);

#else /* !CONFIG_SOFT_HANDMISSALIGN */

asmlinkage void alignment_c(struct pt_regs *regs)
{
	int sig;
	siginfo_t info;

	sig = SIGBUS;
	info.si_code = BUS_ADRALN;
	info.si_signo = sig;
	info.si_errno = 0;
	info.si_addr = (void *)regs->pc;
	if (user_mode(regs)){
		force_sig_info(sig, &info, current);
        return;
	}
	
	if(fixup_exception(regs)) {
	    return;
	}
	die_if_kernel("Kernel mode Alignment exception", regs, 0);
	return;
}

#endif /* CONFIG_SOFT_HANDMISSALIGN */
