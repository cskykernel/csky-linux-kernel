/*
 * arch/csky/ckm5208/timer.c --- timer handles
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2004, Kang Sun (sunk@vlsi.zju.edu.cn)
 * Copyright (C) 2004, Li Chunqiang (chunqiang_li@c-sky.com)
 * Copyright (C) 2009, Hu Junshan (junshan_hu@c-sky.com)
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/param.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/rtc.h>
#include <asm/irq.h>
#include <asm/traps.h>
#include <asm/csky.h>
#include <mach/cktimer.h>
#include <asm/delay.h>

extern irqreturn_t csky_timer_interrupt(int irq, void *dummy);

static struct irqaction csky_timer_irq = {
        .name           = "Ckcore Timer Tick",
        .flags          = IRQF_DISABLED,
        .handler        = csky_timer_interrupt,
};

/*
 *  Clears the interrupt from timer1.
 */
void csky_tick(void)
{
	volatile unsigned long	*timerp;
	unsigned long	temp;

	/* Ack and Clear the interrupt from timer1 */
	timerp = (volatile unsigned long *) (CKTIMER_BASE);
	temp = timerp[CKTIMER_TCN1_EOI];
}

/*
 * Initial the time controller of c-sky. We use timer1 to system time.
 */
void __init csky_timer_init(irq_handler_t handler)
{
	volatile unsigned long	*timerp;

	timerp = (volatile unsigned long *) (CKTIMER_BASE);

	/* 
	 * Set up TIMER 1 as poll clock
	 * Set timer control register, 8 bits, clock = APB bus clock
  	 * The work mode of timer1 is user-defined running mode
	 * Enable timer1 and tiemr1 interrupt
	 */
	timerp[CKTIMER_TCN1_CR] = CKTIMER_TCR_MS | CKTIMER_TCR_EN;

	/*
	 *  set the init value of timer1 load counter, 24bits
	 *  when the counter overflow, interrupt occurs
	 */
	timerp[CKTIMER_TCN1_LDCR] = (CK_BUSCLK /  HZ);

	/*
	 * set interrupt controller for timer
	 * the number of timer interrupt is 8
	 */
	setup_irq(CSKY_TIMER0_IRQ, &csky_timer_irq);
}


unsigned long csky_timer_offset(void)
{
	volatile unsigned long  *timerp;
	unsigned long       trr, tcn, offset;

	timerp = (volatile unsigned long *) (CKTIMER_BASE);
	tcn = timerp[CKTIMER_TCN1_CVR];
	trr = timerp[CKTIMER_TCN1_LDCR];
	
	tcn = trr - tcn;
	
	offset = ((tcn * (1000000 / HZ)) / trr);
	return offset;
}

/*
 *  set or get the real time clock. But now, we don't support.
 */
int csky_hwclk(int set, struct rtc_time *t)
{
	t->tm_year = 1980;
	t->tm_mon  = 1;
	t->tm_mday = 1;
	t->tm_hour = 0;
	t->tm_min  = 0;
	t->tm_sec  = 0;	

	return 0;
}

