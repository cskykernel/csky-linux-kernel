/*
 * arch/csky/ckm5208/fiq.c ---FIQ vector handles
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006 Hangzhou C-SKY Microsystems co.,ltd.
 * Copyright (C) 2009 by Hu Junshan <junshan_hu@c-sky.com> 
 */
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <asm/fiq.h>
#include <mach/ckpic.h>

/*
 *  Unmask the fast interrupt and the fiq number is fiqno.
 */
static void
csky_fiq_startup(unsigned int fiqno)
{
	((volatile unsigned long *)(CKPIC_BASE))[CKPIC_FIER] |=
                                           (1 << (fiqno - FIQ_START));
}

/*
 *  Mask the fast interrupt and the fiq number is fiqno.
 */
static void
csky_fiq_shutdown(unsigned int fiqno)
{
	((volatile unsigned long *)(CKPIC_BASE))[CKPIC_FIER] &=
                                           ~(1 << (fiqno - FIQ_START));
}

static void
csky_fiq_enable(unsigned int fiqno)
{
	((volatile unsigned long *)(CKPIC_BASE))[CKPIC_FIER] |=
                                           (1 << (fiqno - FIQ_START));
}

static void
csky_fiq_diasble(unsigned int fiqno)
{
	((volatile unsigned long *)(CKPIC_BASE))[CKPIC_FIER] &=
                                                ~(1 << (fiqno - FIQ_START));
}

struct fiq_controller csky_fiq_chip = {
        .name           = "csky_fiq",
        .startup        = csky_fiq_startup,
        .shutdown       = csky_fiq_shutdown,
        .enable         = csky_fiq_enable,
        .disable        = csky_fiq_diasble
};

/*
 *  Initial the fast interrupt controller of c-sky.
 */
void __init csky_init_FIQ(void)
{
	int i;

	for (i = FIQ_START; i < FIQ_START + 32; i++)
		set_fiq_controller(i, &csky_fiq_chip);
}

