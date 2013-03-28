/*
 * linux/arch/csky/kernel/ints.c -- General interrupt handling code
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file COPYING in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1999  Greg Ungerer (gerg@snapgear.com)
 * Copyright (C) 1998  D. Jeff Dionne <jeff@ArcturusNetworks.com>
 *                     Kenneth Albanowski <kjahds@kjahds.com>,
 * Copyright (C) 2000  Lineo Inc. (www.lineo.com)
 * Copyright (C) 2004  Kang Sun <sunk@vlsi.zju.edu.cn>
 * Copyright (C) 2009  Hu Junshan (junshan_hu@c-sky.com)
 *
 */
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/signal.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/kernel_stat.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/random.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/kallsyms.h>
#include <linux/list.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/rtc.h>

#include <asm/atomic.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/traps.h>
#include <asm/page.h>
#include <asm/machdep.h>

/* The number of spurious interrupts */
volatile unsigned int num_spurious;

#ifdef CONFIG_CPU_USE_FIQ
extern void init_FIQ(void);
extern int show_fiq_list(struct seq_file *, void *);
#endif
extern e_vector *_ramvec;
void set_evector(int vecnum, void (*handler)(void))
{
	if (vecnum >= 0 && vecnum <= 255)
		_ramvec[vecnum] = handler;
}

unsigned long irq_err_count;

/*
 * Generic, controller-independent functions:
 */

int show_interrupts(struct seq_file *p, void *v)
{
	int i = *(loff_t *) v, j;
	struct irqaction * action;
	unsigned long flags;

	if (i == 0) {
		seq_printf(p, "           ");
		for_each_online_cpu(j)
			seq_printf(p, "CPU%d       ", j);
		seq_putc(p, '\n');
	}

	if (i < NR_IRQS) {
		spin_lock_irqsave(&irq_desc[i].lock, flags);
		action = irq_desc[i].action;
		if (!action)
			goto skip;
		seq_printf(p, "%3d: ", i);
#ifndef CONFIG_SMP
		seq_printf(p, "%10u ", kstat_irqs(i));
#else
		for_each_online_cpu(j)
			seq_printf(p, "%10u ", kstat_cpu(j).irqs[i]);
#endif
		seq_printf(p, " %14s", irq_desc[i].chip->name);
		seq_printf(p, "  %s", action->name);

		for (action=action->next; action; action = action->next)
			seq_printf(p, ", %s", action->name);

		seq_putc(p, '\n');
skip:
		spin_unlock_irqrestore(&irq_desc[i].lock, flags);
	} else if (i == NR_IRQS) {
		seq_putc(p, '\n');
#ifdef CONFIG_CPU_USE_FIQ
		show_fiq_list(p, v);
#endif
		seq_printf(p, "ERR: %10u\n",(unsigned int)irq_err_count);
	}
	return 0;
}


/* Handle bad interrupts */
static struct irq_desc bad_irq_desc = {
	.handle_irq = handle_bad_irq,
	.lock = __SPIN_LOCK_UNLOCKED(bad_irq_desc.lock),
};

/*
 * do_IRQ handles all hardware IRQ's.  Decoded IRQs should not
 * come via this function.  Instead, they should provide their
 * own 'handler'
 */
asmlinkage void csky_do_IRQ(unsigned int irq, struct pt_regs *regs)
{
	struct pt_regs *old_regs = set_irq_regs(regs);

	irq_enter();

	/*
	 * Some hardware gives randomly wrong interrupts.  Rather
	 * than crashing, do something sensible.
	 */
	if (irq >= NR_IRQS) {
		handle_bad_irq(irq, &bad_irq_desc);
	} else
		generic_handle_irq(irq);

	irq_exit();
	set_irq_regs(old_regs);
}

asmlinkage void  csky_do_auto_IRQ(struct pt_regs *regs)
{
	unsigned int irq;
	
	if(mach_get_auto_irqno) {
		irq = mach_get_auto_irqno();
	}
	else {
		printk("Error: cant get irq number from auto IRQ!");
		return;
	}
		
	csky_do_IRQ(irq, regs);
}

/*
 * void init_IRQ(void)
 *
 * Parameters:	None
 *
 * Returns:	Nothing
 *
 * This function should be called during kernel startup to initialize
 * the IRQ handling routines.
 */

void __init init_IRQ(void)
{
	int i;
	
	for (i = 0; i < NR_IRQS; i++)
		set_irq_noprobe(i);


	if (mach_init_IRQ)
		mach_init_IRQ();
#ifdef CONFIG_CPU_USE_FIQ
	init_FIQ();
#endif
}



