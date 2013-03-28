/*
 * linux/arch/csky/kernel/time.c
 *
 * This file contains the csky time handling details.
 * Most of the stuff is located in the machine specific files.
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2006  Hangzhou C-SKY Microsystems co.,ltd.
 * Copyright (C) 2006  Li Chunqiang (chunqiang_li@c-sky.com)
 * Copyright (C) 2009  Hu junshan(junshan_hu@c-sky.com) 
 *
 */

#include <linux/errno.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/param.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/rtc.h>
#include <linux/platform_device.h>

#include <asm/machdep.h>
#include <asm/io.h>
#include <asm/irq_regs.h>

#include <linux/time.h>
#include <linux/timex.h>
#include <linux/profile.h>

#define TICK_SIZE (tick_nsec / 1000)

static inline int set_rtc_mmss(unsigned long nowtime)
{
	if (mach_set_clock_mmss)
		return mach_set_clock_mmss (nowtime);
	return -1;
}


/*
 * timer_interrupt() needs to keep up the real-time clock,
 * as well as call the "do_timer()" routine every clocktick
 */
irqreturn_t  csky_timer_interrupt(int irq, void *dummy)
{
	/* last time the cmos clock got updated */
	static long last_rtc_update=0;

	/* may need to kick the hardware timer */
	if (mach_tick)
		mach_tick();
	write_seqlock(&xtime_lock);	
	do_timer(1);
	write_sequnlock(&xtime_lock);
#ifndef CONFIG_SMP
	update_process_times(user_mode(get_irq_regs()));
#endif
	profile_tick(CPU_PROFILING);

	/*
	 * If we have an externally synchronized Linux clock, then update
	 * CMOS clock accordingly every ~11 minutes. Set_rtc_mmss() has to be
	 * called as close as possible to 500 ms before the new second starts.
	 */
	if ((time_status & STA_UNSYNC) == 0 &&
            xtime.tv_sec > last_rtc_update + 660 &&
            (xtime.tv_nsec / 1000) >= 500000 - ((unsigned) TICK_SIZE) / 2 &&
            (xtime.tv_nsec / 1000) <= 500000 + ((unsigned) TICK_SIZE) / 2) {
		if (set_rtc_mmss(xtime.tv_sec) == 0)
			last_rtc_update = xtime.tv_sec;
		else
			last_rtc_update = xtime.tv_sec - 600; /* do it again in 60 s */
	}

#ifdef CONFIG_HEARTBEAT
	/* use power LED as a heartbeat instead -- much more useful
	   for debugging -- based on the version for PReP by Cort */
	/* acts like an actual heart beat -- ie thump-thump-pause... */
	if (mach_heartbeat) {
		static unsigned cnt = 0, period = 0, dist = 0;

		if (cnt == 0 || cnt == dist)
			mach_heartbeat( 1 );
		else if (cnt == 7 || cnt == dist+7)
			mach_heartbeat( 0 );

		if (++cnt > period) {
			cnt = 0;
		/* The hyperbolic function below modifies the heartbeat period
		 * length in dependency of the current (5min) load. It goes
		 * through the points f(0)=126, f(1)=86, f(5)=51,
		 * f(inf)->30. */
			period = ((672<<FSHIFT)/(5*avenrun[0]+(7<<FSHIFT))) + 30;
			dist = period / 4;
	    	}
	}
#endif /* CONFIG_HEARTBEAT */
	
	return IRQ_HANDLED;
}

void __init time_init(void)
{
	struct rtc_time time;

	if (mach_hwclk) {
		mach_hwclk(0, &time);

		if ((time.tm_year += 1900) < 1970)
			time.tm_year += 100;
		xtime.tv_sec = mktime(time.tm_year, time.tm_mon, time.tm_mday,
		              time.tm_hour, time.tm_min, time.tm_sec);
		xtime.tv_nsec = 0;
	}
	wall_to_monotonic.tv_sec = -xtime.tv_sec;

	mach_time_init(csky_timer_interrupt);
}


/*
 * This version of gettimeofday has near microsecond resolution.
 */
void do_gettimeofday(struct timeval *tv)
{

	unsigned long seq;
	unsigned long usec, sec;
	unsigned long max_ntp_tick = tick_usec - tickadj;

	do {
		seq = read_seqbegin(&xtime_lock);
		usec = mach_gettimeoffset();

		/*
		 * If time_adjust is negative then NTP is slowing the clock
		 * so make sure not to go into next possible interval.
		 * Better to lose some accuracy than have time go backwards..
		 */
		if (unlikely(time_adjust < 0))
			usec = min(usec, max_ntp_tick);

		sec = xtime.tv_sec;
		usec += xtime.tv_nsec/1000;
	} while (read_seqretry(&xtime_lock, seq));

	while (usec >= 1000000) {
		usec -= 1000000;
		sec++;
	}

	tv->tv_sec = sec;
	tv->tv_usec = usec;
}

EXPORT_SYMBOL(do_gettimeofday);

int do_settimeofday(struct timespec *tv)
{
	time_t wtm_sec, sec = tv->tv_sec;
	long wtm_nsec, nsec = tv->tv_nsec;
	unsigned long flags;

	if ((unsigned long)tv->tv_nsec >= NSEC_PER_SEC)
		return -EINVAL;

	write_seqlock_irqsave(&xtime_lock, flags);
	/* This is revolting. We need to set the xtime.tv_nsec
	 * correctly. However, the value in this location is
	 * is value at the last tick.
	 * Discover what correction gettimeofday
	 * would have done, and then undo it!
	 */
	nsec -= 1000 * mach_gettimeoffset();

	wtm_sec  = wall_to_monotonic.tv_sec + (xtime.tv_sec - sec);
	wtm_nsec = wall_to_monotonic.tv_nsec + (xtime.tv_nsec - nsec);

	set_normalized_timespec(&xtime, sec, nsec);
	set_normalized_timespec(&wall_to_monotonic, wtm_sec, wtm_nsec);

	ntp_clear();
	write_sequnlock_irqrestore(&xtime_lock, flags);
	clock_was_set();
	return 0;
}

EXPORT_SYMBOL(do_settimeofday);
