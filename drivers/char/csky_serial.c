/*
 *  csky_serial.c -- serial driver for CK1000-EVB and CK6408EVB Board UARTS.
 *
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
 
#include <linux/module.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/signal.h>
#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/wait.h>
#include <linux/interrupt.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/major.h>
#include <linux/string.h>
#include <linux/fcntl.h>
#include <linux/mm.h>
#include <linux/kernel.h>
#include <linux/serial.h>
#include <linux/serialP.h>
#include <linux/serial_reg.h>
#ifdef CONFIG_LEDMAN
#include <linux/ledman.h>
#endif
#include <linux/console.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/seq_file.h>
#include <linux/ptrace.h>
#include <linux/delay.h>
#include <linux/irqflags.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <asm/segment.h>
#include <asm/bitops.h>
#include <asm/delay.h>
#include <asm/uaccess.h>

#include <mach/ckuart.h>
#include "csky_serial.h"

#define serial_isroot() (capable(CAP_SYS_ADMIN))

/*
 *	the only event we use
 */
#undef  RS_EVENT_WRITE_WAKEUP
#define RS_EVENT_WRITE_WAKEUP 0
#undef  RS_EVENT_READ_WAKEUP
#define RS_EVENT_READ_WAKEUP  1

struct timer_list csky_timer_struct;

/*
 *	Default console baud rate,  we use this as the default
 *	for all ports so init can just open /dev/console and
 *	keep going.  Perhaps one day the cflag settings for the
 *	console can be used instead.
 */
#if defined (CONFIG_CSKY_SERIAL_BAUDRATE)
	#define CONSOLE_BAUD_RATE 	CONFIG_CSKY_SERIAL_BAUDRATE
#else
	#define CONSOLE_BAUD_RATE   115200 
#endif

#if (CONSOLE_BAUD_RATE == 1200)
	#define DEFAULT_CBAUD   B1200
#elif (CONSOLE_BAUD_RATE == 2400)
	#define DEFAULT_CBAUD   B2400
#elif (CONSOLE_BAUD_RATE == 4800)
	#define DEFAULT_CBAUD   B4800
#elif (CONSOLE_BAUD_RATE == 9600)
	#define DEFAULT_CBAUD   B9600
#elif (CONSOLE_BAUD_RATE == 19200)
	#define DEFAULT_CBAUD   B19200
#elif (CONSOLE_BAUD_RATE == 38400)
	#define DEFAULT_CBAUD   B38400
#elif (CONSOLE_BAUD_RATE == 57600)
    #define DEFAULT_CBAUD   B57600
#elif (CONSOLE_BAUD_RATE == 115200)
    #define DEFAULT_CBAUD   B115200
#else
    #define DEFAULT_CBAUD   B115200
#endif

#if defined (CONFIG_CSKY_SERIAL_CONSOLE_UART0)
	#define CONSOLE_UART_PORT 	CSKY_UART_BASE0
#elif defined (CONFIG_CSKY_SERIAL_CONSOLE_UART1)
	#define CONSOLE_UART_PORT 	CSKY_UART_BASE1
#elif defined (CONFIG_CSKY_SERIAL_CONSOLE_UART2)
	#define CONSOLE_UART_PORT 	CSKY_UART_BASE2
#elif defined (CONFIG_CSKY_SERIAL_CONSOLE_UART3)
	#define CONSOLE_UART_PORT 	CSKY_UART_BASE3
#else
	#define CONSOLE_UART_PORT 	CSKY_UART_BASE0
#endif

int csky_console_inited = 0;
int csky_console_port   = -1;
int csky_console_baud   = CONSOLE_BAUD_RATE;
int csky_console_cbaud  = DEFAULT_CBAUD;

/*
 *	Driver data structures.
 */
struct tty_driver	*csky_serial_driver;

/* serial subtype definitions */
#define SERIAL_TYPE_NORMAL	1
#define SERIAL_TYPE_CALLOUT	2
  
/* number of characters left in xmit buffer before we ask for more */
#define WAKEUP_CHARS 256

/*
 * Debugging...
 */
#undef SERIAL_DEBUG_OPEN
#undef SERIAL_DEBUG_FLOW

/*
 *	Configuration table, UARTs to look for at startup.
 */
static struct csky_serial csky_table[] = {
  { 0, (CSKY_UART_BASE0), IRQBASE,     ASYNC_BOOT_AUTOCONF },  /* ttyS0 */
  { 1, (CSKY_UART_BASE1), IRQBASE+1,   ASYNC_BOOT_AUTOCONF }, 
  { 2, (CSKY_UART_BASE2), IRQBASE+2,   ASYNC_BOOT_AUTOCONF },
//  { 3, (CSKY_UART_BASE3), IRQBASE+3,   ASYNC_BOOT_AUTOCONF },
};

#define	NR_PORTS	(sizeof(csky_table) / sizeof(struct csky_serial))

static struct ktermios		*csky_serial_termios[NR_PORTS];
static struct ktermios		*csky_serial_termios_locked[NR_PORTS];

/*
 * This is used to figure out the divisor speeds and the timeouts.
 */
static int csky_baud_table[] = {
	0, 50, 75, 110, 134, 150, 200, 300, 600, 1200, 1800, 2400, 4800,
	9600, 19200, 38400, 57600, 115200, 230400, 460800, 0
};
#define CSKY_BAUD_TABLE_SIZE \
			(sizeof(csky_baud_table)/sizeof(csky_baud_table[0]))

#ifndef MIN
#define MIN(a,b)	((a) < (b) ? (a) : (b))
#endif

#ifdef CONFIG_MAGIC_SYSRQ
/*
 *	Magic system request keys. Used for debugging...
 */
extern int	magic_sysrq_key(int ch);
#endif

/*
 * tmp_buf is used as a temporary buffer by serial_write.  We need to
 * lock it in case the copy_from_user blocks while swapping in a page,
 * and some other program tries to do a serial write at the same time.
 * Since the lock will only come under contention when the system is
 * swapping and available memory is low, it makes sense to share one
 * buffer across all the serial ports, since it significantly saves
 * memory if large numbers of serial ports are open.
 */
static DECLARE_MUTEX(csky_tmp_buf_sem);

/*
 *	Forward declarations...
 */
static void	csky_change_speed(struct csky_serial *info);
static void     csky_wait_until_sent(struct tty_struct *tty, int timeout);


static inline int serial_paranoia_check(struct csky_serial *info,
					char *name, const char *routine)
{
#ifdef SERIAL_PARANOIA_CHECK
	static const char *badmagic =
		"Warning: bad magic number for serial struct (%d, %d) in %s\n";
	static const char *badinfo =
		"Warning: null csky_serial for (%d, %d) in %s\n";

	if (!info) {
		printk(badinfo, name, routine);
		return 1;
	}
	if (info->magic != SERIAL_MAGIC) {
		printk(badmagic, name, routine);
		return 1;
	}
#endif
	return 0;
}

/*
 *	Sets or clears DTR and RTS on the requested line.
 */
static void csky_setsignals(struct csky_serial *info, int dtr, int rts)
{
	volatile unsigned long	*uartp;
	unsigned long		flags;

	local_irq_save(flags);	
	if (rts >= 0) {
		uartp = (volatile unsigned long *) info->addr;
		if (rts) {
			info->sigs |= TIOCM_RTS;
		} else {
			info->sigs &= ~TIOCM_RTS;
		}
	}
	local_irq_restore(flags);
	return;
}

/*
 *	Gets values of serial signals.
 */
static int csky_getsignals(struct csky_serial *info)
{
	volatile unsigned long	*uartp;
	unsigned long		flags;
	int			sigs;

	local_irq_save(flags);
	uartp = (volatile unsigned long *) info->addr;
	sigs = 0;

	local_irq_restore(flags);
	return(sigs);
}

/*
 * csky_stop() and csky_start()
 *
 * This routines are called before setting or resetting tty->stopped.
 * They enable or disable transmitter interrupts, as necessary.
 */
static void csky_stop(struct tty_struct *tty)
{
	volatile unsigned long	*uartp;
	struct csky_serial	*info = (struct csky_serial *)tty->driver_data;

	if (serial_paranoia_check(info, tty->name, "csky_stop"))
		return;
	
	uartp = (volatile unsigned long *) info->addr;
}

static void csky_start(struct tty_struct *tty)
{
	volatile unsigned long	*uartp;
	struct csky_serial	*info =
				(struct csky_serial *)tty->driver_data;
	
	if (serial_paranoia_check(info, tty->name, "csky_start"))
		return;

	uartp = (volatile unsigned long *) info->addr;
}

/*
 * ----------------------------------------------------------------------
 *
 * Here starts the interrupt handling routines.  All of the following
 * subroutines are declared as inline and are folded into
 * csky_uart_interrupt().  They were separated out for readability's sake.
 *
 * Note: csky_uart_interrupt() is a "fast" interrupt, which means that it
 * runs with interrupts turned off.  People who may want to modify
 * csky_uart_interrupt() should try to keep the interrupt handler as fast as
 * possible.  After you are done making modifications, it is not a bad
 * idea to do:
 * 
 * gcc -S -DKERNEL -Wall -Wstrict-prototypes -O6 -fomit-frame-pointer serial.c
 *
 * and look at the resulting assemble code in serial.s.
 *
 *                              - Ted Ts'o (tytso@mit.edu), 7-Mar-93
 * -----------------------------------------------------------------------
 */

static inline void receive_chars(struct csky_serial *info,
									 volatile unsigned long *uartp)
{
	struct tty_struct	*tty = info->tty;
	unsigned char		status, ch, flag;
        int oe = 0;

	if (!tty)
		return;

#if defined(CONFIG_LEDMAN)
	ledman_cmd(LEDMAN_CMD_SET, info->line ? LEDMAN_COM2_RX : LEDMAN_COM1_RX);
#endif

	while ((status = (unsigned char)uartp[CSKY_UART_LSR])
						 & CSKY_UART_LSR_DR) {

		ch = (unsigned char)uartp[CSKY_UART_RBR];
		info->stats.rx++;

#ifdef CONFIG_MAGIC_SYSRQ
		if (csky_console_inited && 
				(info->line == csky_console_port)) {
			if (magic_sysrq_key(ch))
				continue;
		}
#endif

		if (status & CSKY_UART_LSR_BI) {
			info->stats.rxbreak++;
			flag = TTY_BREAK;
		} else if (status & CSKY_UART_LSR_PERR) {
			info->stats.rxparity++;
			flag = TTY_PARITY;
		} else if (status & CSKY_UART_LSR_OVRERR) {
			info->stats.rxoverrun++;
			flag = TTY_OVERRUN;
		} else if (status & CSKY_UART_LSR_FERR) {
			info->stats.rxframing++;
			flag = TTY_FRAME;
		} else {
			flag = 0;
		}
		if (status & UART_LSR_OE) {
			/*
			 * Overrun is special, since it's
			 * reported immediately, and doesn't
			 * affect the current character
			 */
			 oe = 1;
		}
		tty_insert_flip_char(tty, ch, flag);
	}
	if (oe == 1)
		tty_insert_flip_char(tty, 0, TTY_OVERRUN);

	tty_flip_buffer_push(tty);
	return;
}


/*
 * This is the serial driver's generic interrupt routine
 */
static irqreturn_t csky_uart_interrupt(int irq, void *dev_id)
{
	struct csky_serial	*info;
	unsigned long		isr;
	volatile unsigned long  *uartp;

	info = &csky_table[(irq - IRQBASE)];
	uartp = (volatile unsigned long *)info->addr;

	isr = uartp[CSKY_UART_IIR];

	if (isr & CSKY_UART_IIR_RDA){
		receive_chars(info, uartp);
	}
	return IRQ_HANDLED;
}

/*
 * Here ends the serial interrupt routines.
 */
static void do_softint(struct work_struct *work)
{
	struct csky_serial	*info = container_of(work, struct csky_serial, tqueue);
	struct tty_struct	*tty;
	
	tty = info->tty;
	if (!tty)
		return;

	if (test_and_clear_bit(RS_EVENT_WRITE_WAKEUP, &info->event)) 
		tty_wakeup(tty);
}

/*
 *	Change of state on a DCD line.
 */
void csky_modem_change(struct csky_serial *info, int dcd)
{
	if (info->count == 0)
		return;

	if (info->flags & ASYNC_CHECK_CD) {
		if (dcd) {
			wake_up_interruptible(&info->open_wait);
		} else {
			schedule_work(&info->tqueue_hangup);
		}
	}
}

#if defined(CSKY_HAVEDCD0) || defined(CSKY_HAVEDCD1)

unsigned int	csky_olddcd0;

/*
 * This subroutine is called when the RS_TIMER goes off. It is used
 * to monitor the state of the DCD lines - since they have no edge
 * sensors and interrupt generators.
 */
static void csky_timer(unsigned long arg)
{
	unsigned int dcd;

	dcd = csky_getppdcd(0);
	if (dcd != csky_olddcd0)
		csky_modem_change(&csky_table[0], dcd);
	csky_olddcd0 = dcd;

	/* Re-arm timer */
	csky_timer_struct.expires = jiffies + HZ/25;
	add_timer(&csky_timer_struct);
}

#endif	/* CSKY_HAVEDCD0 || CSKY_HAVEDCD1 */


/*
 * This routine is called from the scheduler tqueue when the interrupt
 * routine has signalled that a hangup has occurred.  The path of
 * hangup processing is:
 *
 * 	serial interrupt routine -> (scheduler tqueue) ->
 * 	do_serial_hangup() -> tty->hangup() -> csky_hangup()
 * 
 */
static void do_serial_hangup(struct work_struct *work)
{
	struct csky_serial	*info = container_of(work, struct csky_serial, tqueue_hangup);
	struct tty_struct	*tty;
	
	tty = info->tty;
	if (!tty)
		return;

	tty_hangup(tty);
}

/*
 * ---------------------------------------------------------------
 * Low level utility subroutines for the serial driver:  routines to
 * figure out the appropriate timeout for an interrupt chain, routines
 * to initialize and startup a serial port, and routines to shutdown a
 * serial port.  Useful stuff like that.
 * ---------------------------------------------------------------
 */
static int startup(struct csky_serial * info)
{
	volatile unsigned long	*uartp;
	unsigned long		flags;
	
	if (info->flags & ASYNC_INITIALIZED)
		return 0;

	if (!info->xmit_buf) {
		info->xmit_buf = (unsigned char *) get_zeroed_page(GFP_KERNEL);
		if (!info->xmit_buf)
			return -ENOMEM;
	}

	local_irq_save(flags);	

#ifdef SERIAL_DEBUG_OPEN
	printk("starting up ttyS%d (irq %d)...\n", info->line, info->irq);
#endif

	/*
	 *	Reset UART, get it into known state...
	 */
	uartp = (volatile unsigned long *) info->addr;

 	/* Cause no reset interface on ckcore uart, i have to reset it manually */
	/* select DLR */
	while (uartp[CSKY_UART_USR] & CSKY_UART_USR_BUSY);

	uartp[CSKY_UART_LCR] = CSKY_UART_LCR_DLAEN | 
						CSKY_UART_LCR_WLEN8;
	uartp[CSKY_UART_DLL] = 0;       			/* reset DLL to 0 */
	uartp[CSKY_UART_DLH] = 0;       			/* reset DLH to 0 */
	uartp[CSKY_UART_LCR] = CSKY_UART_LCR_WLEN8; /* reset LCR to 0x03 */
	uartp[CSKY_UART_IER] = 0;       			/* reset IER to 0 */

	csky_setsignals(info, 1, 1);

	if (info->tty)
		clear_bit(TTY_IO_ERROR, &info->tty->flags);
	info->xmit_cnt = info->xmit_head = info->xmit_tail = 0;

	/*
	 * and set the speed of the serial port
	 */
	csky_change_speed(info);

	/*
	 * Lastly enable the UART transmitter and receiver, and
	 * interrupt enables.
	 */
	info->imr = CSKY_UART_IER_ERDAI;
	uartp[CSKY_UART_IER] = (unsigned long)(info->imr);

	info->flags |= ASYNC_INITIALIZED;
	local_irq_restore(flags);
	return 0;
}

/*
 * This routine will shutdown a serial port; interrupts are disabled, and
 * DTR is dropped if the hangup on close termio flag is on.
 */
static void shutdown(struct csky_serial * info)
{
	volatile unsigned long	*uartp;
	unsigned long		flags;

	if (!(info->flags & ASYNC_INITIALIZED))
		return;

#ifdef SERIAL_DEBUG_OPEN
	printk("Shutting down serial port %d (irq %d)....\n", info->line,
	       info->irq);
#endif
	
	uartp = (volatile unsigned long *) info->addr;
	local_irq_save(flags);	

	uartp[CSKY_UART_IER] = 0;  /* mask all interrupts & reset the uart */

	if (!info->tty || (info->tty->termios->c_cflag & HUPCL))
		csky_setsignals(info, 0, 0);

	if (info->xmit_buf) {
		free_page((unsigned long) info->xmit_buf);
		info->xmit_buf = 0;
	}

	if (info->tty)
		set_bit(TTY_IO_ERROR, &info->tty->flags);
	
	info->flags &= ~ASYNC_INITIALIZED;
	local_irq_restore(flags);
}


/*
 * This routine is called to set the UART divisor registers to match
 * the specified baud rate for a serial port.
 */
static void csky_change_speed(struct csky_serial *info)
{
	volatile unsigned long	*uartp;
	unsigned long		baudclk, cflag;
	unsigned long		flags;
	int			i;

	if (!info->tty || !info->tty->termios)
		return;
	cflag = info->tty->termios->c_cflag;
	if (info->addr == 0)
		return;

	i = cflag & CBAUD;
	if (i & CBAUDEX) {
		i &= ~CBAUDEX;
		if (i < 1 || i > 4)
			info->tty->termios->c_cflag &= ~CBAUDEX;
		else
			i += 15;
	}
	if (i == 0) {
		csky_setsignals(info, 0, -1);
		return;
	}

	/* Caculate the baudrate clock */
	baudclk = (unsigned long)((((CK_BUSCLK * 2) / 
		(csky_baud_table[i] << CSKY_UART_DIV)) + 1) / 2);
	info->baud = csky_baud_table[i];
	uartp = (volatile unsigned long *) info->addr;

	local_irq_save(flags);	

	while (uartp[CSKY_UART_USR] & CSKY_UART_USR_BUSY);
	uartp[CSKY_UART_LCR]  = CSKY_UART_LCR_DLAEN |
					 CSKY_UART_LCR_WLEN8; /* select DLAR */
#ifdef __ckcoreBE__
	uartp[CSKY_UART_DLL]  = 
			(unsigned long)(*((unsigned char *)(&baudclk) + 3));
	if(baudclk & 0xff00){
		uartp[CSKY_UART_DLH] = (
			unsigned long)(*((unsigned char *)(&baudclk) + 2));
	}
#else	
	uartp[CSKY_UART_DLL]  = baudclk & 0xff;
	uartp[CSKY_UART_DLH] = (baudclk >> 8) & 0xff; 
#endif	
	uartp[CSKY_UART_LCR] = CSKY_UART_LCR_WLEN8; /* Odd parity and 8 bits char */
	uartp[CSKY_UART_FCR] = CSKY_UART_FCR_RT0; 	/* 1 byte FIFO */

	csky_setsignals(info, 1, -1);

	local_irq_restore(flags);
	return;
}

static void csky_flush_chars(struct tty_struct *tty)
{
	volatile unsigned long *uartp;
	struct csky_serial	*info = 
			(struct csky_serial *)tty->driver_data;
	unsigned long		flags;
        unsigned long           i;

	if (serial_paranoia_check(info, tty->name, "csky_flush_chars")){
		return;
	}
	/*
	 * re-enable receiver interrupt
	 */
	
	if (info->xmit_cnt <= 0 || tty->stopped || tty->hw_stopped ||
	    !info->xmit_buf)
		return;

	/* Enable transmitter */
	uartp = (volatile unsigned long *) info->addr;

	local_irq_save(flags);	

	/* 
	 * Cause transmitter ready interrupt is not provided,
	 * we did not perform uart wrting in an asynchronized
	 * way. The write procedure is in csky_wirte. 
	 */
	if (info->x_char){
		/* Send special char - probably flow control */
		uartp[CSKY_UART_THR] = (unsigned long)(info->x_char);
		info->x_char = 0;
		info->stats.tx++;
	}

	if ((info->xmit_cnt <= 0) || info->tty->stopped) {
		return;
	}
	/* use do-while for the case of LSR_THRE bit */
	while (1) {
		for (i = 0; i < 0x10000; i++) {
			if (uartp[CSKY_UART_LSR] & 
						CSKY_UART_LSR_THRE){
				break;
			}
		}
		uartp[CSKY_UART_THR] = 
			(unsigned long)(info->xmit_buf[info->xmit_tail++]);
		info->xmit_tail = info->xmit_tail & (SERIAL_XMIT_SIZE - 1);
		if (--info->xmit_cnt <= 0) {
			break;
		}
	}
	local_irq_restore(flags);	
}

static int csky_write(struct tty_struct * tty,
		    const unsigned char *buf, int count)
{
	volatile unsigned long	*uartp;
	struct csky_serial	*info = (struct csky_serial *)tty->driver_data;
	unsigned long			flags;
	int				c, total = 0;
	unsigned long			i;


	if (serial_paranoia_check(info, tty->name, "csky_write"))
		return 0;

	if (!tty || !info->xmit_buf)
		return 0;
	
	local_save_flags(flags);
	while (1) {
		local_irq_disable();		
		c = MIN(count, MIN(SERIAL_XMIT_SIZE - info->xmit_cnt - 1,
				   SERIAL_XMIT_SIZE - info->xmit_head));

		if (c <= 0) {
			local_irq_restore(flags);
			break;
		}

		memcpy(info->xmit_buf + info->xmit_head, buf, c);

		info->xmit_head = (info->xmit_head + c) & (SERIAL_XMIT_SIZE-1);
		info->xmit_cnt += c;
		local_irq_restore(flags);

		buf += c;
		count -= c;
		total += c;
	}

	if (!tty->stopped) {
		uartp = (volatile unsigned long *) info->addr;
		local_irq_disable();

		/* 
		 * Cause transmitter ready interrupt is not provided,
		 * we did not perform uart wrting in an asynchronized
		 * way. The write procedure is in csky_wirte.
		 */
  		if (info->x_char){
			/* Send special char - probably flow control */
			uartp[CSKY_UART_THR] = (unsigned long)(info->x_char);
			info->x_char = 0;
			info->stats.tx++;
		}
       
		if ((info->xmit_cnt <= 0) || info->tty->stopped)
		{
			return 0;
		}
		/* use do-while for the case of LSR_THRE bit */
		while (1)
		{
			for (i = 0; i < 0x10000; i++){
				if (uartp[CSKY_UART_LSR] & 
					CSKY_UART_LSR_THRE) {
					break;
				}
			}
			uartp[CSKY_UART_THR] = 
					(unsigned long)(info->xmit_buf[info->xmit_tail++]);
			info->xmit_tail = info->xmit_tail & (SERIAL_XMIT_SIZE - 1);
			if (--info->xmit_cnt <= 0) {
				break;
			}
		}
		local_irq_restore(flags);	
	}

	return total;
}

static int csky_write_room(struct tty_struct *tty)
{
	struct csky_serial *info = (struct csky_serial *)tty->driver_data;
	int	ret;
				
	if (serial_paranoia_check(info, tty->name, "csky_write_room"))
		return 0;
	ret = SERIAL_XMIT_SIZE - info->xmit_cnt - 1;
	if (ret < 0)
		ret = 0;
	return ret;
}

static int csky_chars_in_buffer(struct tty_struct *tty)
{
	struct csky_serial *info = (struct csky_serial *)tty->driver_data;
					
	if (serial_paranoia_check(info, tty->name, "csky_chars_in_buffer"))
		return 0;

	return (info->xmit_cnt);
}

static void csky_flush_buffer(struct tty_struct *tty)
{
	struct csky_serial	*info = (struct csky_serial *)tty->driver_data;
	unsigned long		flags;
				
	if (serial_paranoia_check(info, tty->name, "csky_flush_buffer"))
		return;

	local_irq_save(flags);	
	info->xmit_cnt = info->xmit_head = info->xmit_tail = 0;
	local_irq_restore(flags);
        
        tty_wakeup(tty);
}

/*
 * ------------------------------------------------------------
 * csky_throttle()
 * 
 * This routine is called by the upper-layer tty layer to signal that
 * incoming characters should be throttled.
 * ------------------------------------------------------------
 */
static void csky_throttle(struct tty_struct * tty)
{
	struct csky_serial *info = (struct csky_serial *)tty->driver_data;
#ifdef SERIAL_DEBUG_THROTTLE
	printk("csky_throttle line=%d chars=%d\n", info->line,
	       tty->ldisc.chars_in_buffer(tty));
#endif

	if (serial_paranoia_check(info, tty->name, "csky_throttle"))
		return;
	
	if (I_IXOFF(tty)) {
		/* Force STOP_CHAR (xoff) out */
		volatile unsigned long	*uartp;
		info->x_char = STOP_CHAR(tty);
		uartp = (volatile unsigned long *) info->addr;
	}

	/* Turn off RTS line */
	if (tty->termios->c_cflag & CRTSCTS)
		csky_setsignals(info, -1, 0);
}

static void csky_unthrottle(struct tty_struct * tty)
{
	struct csky_serial *info = (struct csky_serial *)tty->driver_data;
#ifdef SERIAL_DEBUG_THROTTLE
	printk("csky_unthrottle line=%d chars=%d\n", info->line,
	       tty->ldisc.chars_in_buffer(tty));
#endif

	if (serial_paranoia_check(info, tty->name, "csky_unthrottle"))
		return;
	
	if (I_IXOFF(tty)) {
		if (info->x_char)
			info->x_char = 0;
		else {
			/* Force START_CHAR (xon) out */
			volatile unsigned long	*uartp;
			info->x_char = START_CHAR(tty);
			uartp = (volatile unsigned long *) info->addr;
		}
	}

	/* Assert RTS line */
	if (tty->termios->c_cflag & CRTSCTS)
		csky_setsignals(info, -1, 1);
}

/*
 * ------------------------------------------------------------
 * csky_ioctl() and friends
 * ------------------------------------------------------------
 */

static int get_serial_info(struct csky_serial * info,
			   struct serial_struct * retinfo)
{
	struct serial_struct tmp;
  
	if (!retinfo)
		return -EFAULT;
	memset(&tmp, 0, sizeof(tmp));
	tmp.type = info->type;
	tmp.line = info->line;
	tmp.port = info->addr;
	tmp.irq = info->irq;
	tmp.flags = info->flags;
	tmp.baud_base = info->baud_base;
	tmp.close_delay = info->close_delay;
	tmp.closing_wait = info->closing_wait;
	tmp.custom_divisor = info->custom_divisor;
	copy_to_user(retinfo,&tmp,sizeof(*retinfo));
	return 0;
}

static int set_serial_info(struct csky_serial * info,
			   struct serial_struct * new_info)
{
	struct serial_struct new_serial;
	struct csky_serial old_info;
	int 			retval = 0;

	if (!new_info)
		return -EFAULT;
	copy_from_user(&new_serial,new_info,sizeof(new_serial));
        old_info = *info;

	if (!serial_isroot()) {
		if ((new_serial.baud_base != info->baud_base) ||
		    (new_serial.type != info->type) ||
		    (new_serial.close_delay != info->close_delay) ||
		    ((new_serial.flags & ~ASYNC_USR_MASK) !=
		     (info->flags & ~ASYNC_USR_MASK)))
			return -EPERM;
		info->flags = ((info->flags & ~ASYNC_USR_MASK) |
			       (new_serial.flags & ASYNC_USR_MASK));
		info->custom_divisor = new_serial.custom_divisor;
		goto check_and_exit;
	}

	if (info->count > 1)
		return -EBUSY;

	/*
	 * OK, past this point, all the error checking has been done.
	 * At this point, we start making changes.....
	 */

	info->baud_base = new_serial.baud_base;
	info->flags = ((info->flags & ~ASYNC_FLAGS) |
			(new_serial.flags & ASYNC_FLAGS));
	info->type = new_serial.type;
	info->close_delay = new_serial.close_delay;
	info->closing_wait = new_serial.closing_wait;

check_and_exit:
	retval = startup(info);
	return retval;
}

/*
 * get_lsr_info - get line status register info
 *
 * Purpose: Let user call ioctl() to get info when the UART physically
 * 	    is emptied.  On bus types like RS485, the transmitter must
 * 	    release the bus after transmitting. This must be done when
 * 	    the transmit shift register is empty, not be done when the
 * 	    transmit holding register is empty.  This functionality
 * 	    allows an RS485 driver to be written in user space. 
 */
static int get_lsr_info(struct csky_serial * info, unsigned int *value)
{
	volatile unsigned long 	*uartp;
	unsigned long		flags;
	unsigned char		status;

	uartp = (volatile unsigned long *) info->addr;
	local_irq_save(flags);	
	status = (uartp[CSKY_UART_LSR] & CSKY_UART_LSR_THRE) ? TIOCSER_TEMT : 0;
	local_irq_restore(flags);

	put_user(status,value);
	return 0;
}

/*
 * This routine sends a break character out the serial port.
 */
static void send_break(	struct csky_serial * info, int duration)
{
  // do not support this function in CK6408EVB board
}

static int csky_ioctl(struct tty_struct *tty, struct file * file,
		    unsigned int cmd, unsigned long arg)
{
	struct csky_serial * info = (struct csky_serial *)tty->driver_data;
	unsigned int val;
	int retval, error;
	int dtr, rts;

	if (serial_paranoia_check(info, tty->name, "csky_ioctl"))
		return -ENODEV;

	if ((cmd != TIOCGSERIAL) && (cmd != TIOCSSERIAL) &&
	    (cmd != TIOCSERCONFIG) && (cmd != TIOCSERGWILD)  &&
	    (cmd != TIOCSERSWILD) && (cmd != TIOCSERGSTRUCT)) {
		if (tty->flags & (1 << TTY_IO_ERROR))
		    return -EIO;
	}
	
	switch (cmd) {
		case TCSBRK:	/* SVID version: non-zero arg --> no break */
			retval = tty_check_change(tty);
			if (retval)
				return retval;
			tty_wait_until_sent(tty, 0);
			if (!arg)
				send_break(info, HZ/4);	/* 1/4 second */
			return 0;
		case TCSBRKP:	/* support for POSIX tcsendbreak() */
			retval = tty_check_change(tty);
			if (retval)
				return retval;
			tty_wait_until_sent(tty, 0);
			send_break(info, arg ? arg*(HZ/10) : HZ/4);
			return 0;
		case TIOCGSOFTCAR:
			error = verify_area(VERIFY_WRITE, (void *) arg,sizeof(long));
			if (error)
				return error;
			put_user(C_CLOCAL(tty) ? 1 : 0,
				    (unsigned long *) arg);
			return 0;
		case TIOCSSOFTCAR:
			get_user(arg, (unsigned long *) arg);
			tty->termios->c_cflag =
				((tty->termios->c_cflag & ~CLOCAL) |
				 (arg ? CLOCAL : 0));
			return 0;
		case TIOCGSERIAL:
			error = verify_area(VERIFY_WRITE, (void *) arg,
						sizeof(struct serial_struct));
			if (error)
				return error;
			return get_serial_info(info,
					       (struct serial_struct *) arg);
		case TIOCSSERIAL:
			return set_serial_info(info,
					       (struct serial_struct *) arg);
		case TIOCSERGETLSR: /* Get line status register */
			error = verify_area(VERIFY_WRITE, (void *) arg,
				sizeof(unsigned int));
			if (error)
				return error;
			else
			    return get_lsr_info(info, (unsigned int *) arg);

		case TIOCSERGSTRUCT:
			error = verify_area(VERIFY_WRITE, (void *) arg,
						sizeof(struct csky_serial));
			if (error)
				return error;
			copy_to_user((struct mcf_serial *) arg,
				    info, sizeof(struct csky_serial));
			return 0;
			
		case TIOCMGET:
			if ((error = verify_area(VERIFY_WRITE, (void *) arg,
                            sizeof(unsigned int))))
                                return(error);
			val = csky_getsignals(info);
			put_user(val, (unsigned int *) arg);
			break;

                case TIOCMBIS:
			if ((error = verify_area(VERIFY_WRITE, (void *) arg,
                            sizeof(unsigned int))))
				return(error);

			get_user(val, (unsigned int *) arg);
			rts = (val & TIOCM_RTS) ? 1 : -1;
			dtr = (val & TIOCM_DTR) ? 1 : -1;
			csky_setsignals(info, dtr, rts);
			break;

                case TIOCMBIC:
			if ((error = verify_area(VERIFY_WRITE, (void *) arg,
                            sizeof(unsigned int))))
				return(error);
			get_user(val, (unsigned int *) arg);
			rts = (val & TIOCM_RTS) ? 0 : -1;
			dtr = (val & TIOCM_DTR) ? 0 : -1;
			csky_setsignals(info, dtr, rts);
			break;

                case TIOCMSET:
			if ((error = verify_area(VERIFY_WRITE, (void *) arg,
                            sizeof(unsigned int))))
				return(error);
			get_user(val, (unsigned int *) arg);
			rts = (val & TIOCM_RTS) ? 1 : 0;
			dtr = (val & TIOCM_DTR) ? 1 : 0;
			csky_setsignals(info, dtr, rts);
			break;

#ifdef TIOCSET422
		case TIOCSET422:
			get_user(val, (unsigned int *) arg);
			break;
		case TIOCGET422:
			put_user(val, (unsigned int *) arg);
			break;
#endif

		default:
			return -ENOIOCTLCMD;
		}
	return 0;
}

static void csky_set_termios(struct tty_struct *tty, 
								struct ktermios *old_termios)
{
	struct csky_serial *info = (struct csky_serial *)tty->driver_data;

	if (tty->termios->c_cflag == old_termios->c_cflag)
		return;

	csky_change_speed(info);

	if ((old_termios->c_cflag & CRTSCTS) &&
	    !(tty->termios->c_cflag & CRTSCTS)) {
		tty->hw_stopped = 0;
		csky_setsignals(info, -1, 1);
	}
}

/*
 * ------------------------------------------------------------
 * csky_close()
 * 
 * This routine is called when the serial port gets closed.  First, we
 * wait for the last remaining data to be sent.  Then, we unlink its
 * S structure from the interrupt chain if necessary, and we free
 * that IRQ if nothing is left in the chain.
 * ------------------------------------------------------------
 */
static void csky_close(struct tty_struct *tty, struct file * filp)
{
	volatile unsigned long	*uartp;
	struct csky_serial	*info = (struct csky_serial *)tty->driver_data;
	unsigned long		flags;

	if (!info || serial_paranoia_check(info, tty->name, "csky_close"))
		return;
	
	local_irq_save(flags);	
	
	if (tty_hung_up_p(filp)) {
		local_irq_restore(flags);
		return;
	}
	
#ifdef SERIAL_DEBUG_OPEN
	printk("csky_close ttyS%d, count = %d\n", info->line, info->count);
#endif
	if ((tty->count == 1) && (info->count != 1)) {
		/*
		 * Uh, oh.  tty->count is 1, which means that the tty
		 * structure will be freed.  Info->count should always
		 * be one in these conditions.  If it's greater than
		 * one, we've got real problems, since it means the
		 * serial port won't be shutdown.
		 */
		printk("csky_close: bad serial port count; tty->count is 1, "
		       "info->count is %d\n", info->count);
		info->count = 1;
	}
	if (--info->count < 0) {
		printk("csky_close: bad serial port count for ttyS%d: %d\n",
		       info->line, info->count);
		info->count = 0;
	}
	if (info->count) {
		local_irq_restore(flags);	
		return;
	}
	info->flags |= ASYNC_CLOSING;

	/*
	 * Now we wait for the transmit buffer to clear; and we notify 
	 * the line discipline to only process XON/XOFF characters.
	 */
	tty->closing = 1;
	if (info->closing_wait != ASYNC_CLOSING_WAIT_NONE)
		tty_wait_until_sent(tty, info->closing_wait);

	/*
	 * At this point we stop accepting input.  To do this, we
	 * disable the receive line status interrupts, and tell the
	 * interrupt driver to stop checking the data ready bit in the
	 * line status register.
	 */
	info->imr &= ~CSKY_UART_IER_ERDAI;
	uartp = (volatile unsigned long *) info->addr;
	uartp[CSKY_UART_IER] = (unsigned long)(info->imr);

	shutdown(info);
	csky_flush_buffer(tty);

        tty_ldisc_flush(tty);
        tty->closing = 0;
	info->event = 0;
	info->tty = 0;
	info->imr = 0;
//#warning "This is not and has never been valid so fix it"       
#if 0
	if (tty->ldisc.num != ldiscs[N_TTY].num) {
                if (tty->ldisc.close)
                        (tty->ldisc.close)(tty);
                tty->ldisc = ldiscs[N_TTY];
                tty->termios->c_line = N_TTY;
                if (tty->ldisc.open)
                        (tty->ldisc.open)(tty);
        }
#endif
	if (info->blocked_open) {
		if (info->close_delay) {
		msleep_interruptible(jiffies_to_msecs(info->close_delay));
                }
		wake_up_interruptible(&info->open_wait);
	}
	info->flags &= ~(ASYNC_NORMAL_ACTIVE | ASYNC_CLOSING);
	wake_up_interruptible(&info->close_wait);
	local_irq_restore(flags);
}

/*
 * csky_wait_until_sent() --- wait until the transmitter is empty
 */
static void
csky_wait_until_sent(struct tty_struct *tty, int timeout)
{

}
		
/*
 * csky_hangup() --- called by tty_hangup() when a hangup is signaled.
 */
void csky_hangup(struct tty_struct *tty)
{
	struct csky_serial * info = (struct csky_serial *)tty->driver_data;
	
	if (serial_paranoia_check(info, tty->name, "csky_hangup"))
		return;
	
	csky_flush_buffer(tty);
	shutdown(info);
	info->event = 0;
	info->count = 0;
	info->flags &= ~ASYNC_NORMAL_ACTIVE;
	info->tty = 0;
	wake_up_interruptible(&info->open_wait);
}

/*
 * ------------------------------------------------------------
 * csky_open() and friends
 * ------------------------------------------------------------
 */
static int block_til_ready(struct tty_struct *tty, struct file * filp,
			   struct csky_serial *info)
{
	DECLARE_WAITQUEUE(wait, current);
	int		retval;
	int		do_clocal = 0;

	/*
	 * If the device is in the middle of being closed, then block
	 * until it's done, and then try again.
	 */
	if (info->flags & ASYNC_CLOSING) {
		if (filp->f_flags & O_NONBLOCK)
			return -EAGAIN;
		interruptible_sleep_on(&info->close_wait);
#ifdef SERIAL_DO_RESTART
		if (info->flags & ASYNC_HUP_NOTIFY)
			return -EAGAIN;
		else
			return -ERESTARTSYS;
#else
		return -EAGAIN;
#endif
	}
	
	/*
	 * If non-blocking mode is set, or the port is not enabled,
	 * then make the check up front and then exit.
	 */
	if ((filp->f_flags & O_NONBLOCK) ||
	    (tty->flags & (1 << TTY_IO_ERROR))) {
		info->flags |= ASYNC_NORMAL_ACTIVE;
		return 0;
	}

	if (tty->termios->c_cflag & CLOCAL)
		do_clocal = 1;
	
	/*
	 * Block waiting for the carrier detect and the line to become
	 * free (i.e., not in use by the callout).  While we are in
	 * this loop, info->count is dropped by one, so that
	 * csky_close() knows when to free things.  We restore it upon
	 * exit, either normal or abnormal.
	 */
	retval = 0;
	add_wait_queue(&info->open_wait, &wait);
#ifdef SERIAL_DEBUG_OPEN
	printk("block_til_ready before block: ttyS%d, count = %d\n",
	       info->line, info->count);
#endif
	info->count--;
	info->blocked_open++;
	while (1) {
		local_irq_disable();
		if (tty->termios->c_cflag & CBAUD)
			csky_setsignals(info, 1, 1);
		local_irq_enable();
		current->state = TASK_INTERRUPTIBLE;
		if (tty_hung_up_p(filp) ||
		    !(info->flags & ASYNC_INITIALIZED)) {
#ifdef SERIAL_DO_RESTART
			if (info->flags & ASYNC_HUP_NOTIFY)
				retval = -EAGAIN;
			else
				retval = -ERESTARTSYS;	
#else
			retval = -EAGAIN;
#endif
			break;
		}
		if (!(info->flags & ASYNC_CLOSING) &&
		    (do_clocal || (csky_getsignals(info) & TIOCM_CD)))
			break;
		if (signal_pending(current)) {
			retval = -ERESTARTSYS;
			break;
		}
#ifdef SERIAL_DEBUG_OPEN
		printk("block_til_ready blocking: ttyS%d, count = %d\n",
		       info->line, info->count);
#endif
		schedule();
	}
	current->state = TASK_RUNNING;
	remove_wait_queue(&info->open_wait, &wait);
	if (!tty_hung_up_p(filp))
		info->count++;
	info->blocked_open--;
#ifdef SERIAL_DEBUG_OPEN
	printk("block_til_ready after blocking: ttyS%d, count = %d\n",
	       info->line, info->count);
#endif
	if (retval)
		return retval;
	info->flags |= ASYNC_NORMAL_ACTIVE;
	return 0;
}	

/*
 * This routine is called whenever a serial port is opened. It
 * enables interrupts for a serial port, linking in its structure into
 * the IRQ chain.   It also performs the serial-specific
 * initialization for the tty structure.
 */
int csky_open(struct tty_struct *tty, struct file * filp)
{
	struct csky_serial	*info;
	int 			retval, line;

	line = tty->index;
 	if ((line < 0) || (line >= NR_PORTS))
		return -ENODEV;
	info = csky_table + line;
	if (serial_paranoia_check(info, tty->name, "csky_open"))
		return -ENODEV;
#ifdef SERIAL_DEBUG_OPEN
//	printk("csky_open %s%d, count = %d\n", tty->driver.name, info->line,
//	       info->count);
#endif
	info->count++;
	tty->driver_data = info;
	info->tty = tty;

	/*
	 * Start up serial port
	 */
	retval = startup(info);
	if (retval)
		return retval;

	retval = block_til_ready(tty, filp, info);
	if (retval) {
#ifdef SERIAL_DEBUG_OPEN
		printk("csky_open returning after block_til_ready with %d\n",
		       retval);
#endif
		return retval;
	}

#ifdef SERIAL_DEBUG_OPEN
	printk("csky_open %s successful...\n", tty->name);
#endif
	return 0;
}

/*
 * Based on the line number set up the internal interrupt stuff.
 */
static void csky_irqinit(struct csky_serial *info)
{
	switch (info->line) {
	case 0:
		break;
	case 1:
		break;
	case 2:
		break;
	case 3:
		break;
	default:
		printk("SERIAL: don't know how to handle UART %d interrupt\n",
				info->line);
		return;
	}
	if (request_irq(info->irq, csky_uart_interrupt, IRQF_DISABLED,
		"CK6408EVB Board UART", NULL))
	{
		printk("SERIAL: Unable to attach CK6408EVB UART %d interrupt "
			"vector=%d\n", info->line, info->irq);
	}
	return;
}


char *csky_drivername = "C-SKY Board internal UART serial driver version 2.0\n";


/*
 * Serial stats reporting...
 */
int csky_readproc(char *page, char **start, off_t off, int count,
		         int *eof, void *data)
{
	struct csky_serial	*info;
	char			str[20];
	int			len, sigs, i;

	len = sprintf(page, csky_drivername);
	for (i = 0; (i < NR_PORTS); i++) {
		info = &csky_table[i];
		len += sprintf((page + len), "%d: port:%x irq=%d baud:%d ",
			i, info->addr, info->irq, info->baud);
		if (info->stats.rx || info->stats.tx)
			len += sprintf((page + len), "tx:%d rx:%d ",
			info->stats.tx, info->stats.rx);
		if (info->stats.rxframing)
			len += sprintf((page + len), "fe:%d ",
			info->stats.rxframing);
		if (info->stats.rxoverrun)
			len += sprintf((page + len), "oe:%d ",
			info->stats.rxoverrun);

		str[0] = str[1] = 0;
		if ((sigs = csky_getsignals(info))) {
			if (sigs & TIOCM_RTS)
				strcat(str, "|RTS");
			if (sigs & TIOCM_CTS)
				strcat(str, "|CTS");
			if (sigs & TIOCM_DTR)
				strcat(str, "|DTR");
			if (sigs & TIOCM_CD)
				strcat(str, "|CD");
		}

		len += sprintf((page + len), "%s\n", &str[1]);
	}

	return(len);
}


/* Finally, routines used to initialize the serial driver. */

static void show_serial_version(void)
{
	printk(csky_drivername);
}

static const struct tty_operations serial_ops = {
	.open = csky_open,
	.close = csky_close,
	.write = csky_write,
	.flush_chars = csky_flush_chars,
	.write_room = csky_write_room,
	.chars_in_buffer = csky_chars_in_buffer,
	.flush_buffer = csky_flush_buffer,
	.ioctl = csky_ioctl,
	.throttle = csky_throttle,
	.unthrottle = csky_unthrottle,
	.set_termios = csky_set_termios,
	.stop = csky_stop,
	.start = csky_start,
	.hangup = csky_hangup,
	.wait_until_sent = csky_wait_until_sent,
	.tiocmget = NULL,
	.tiocmset = NULL,
};

/* csky_init inits the driver */
static int __init
csky_init(void)
{
	struct csky_serial	*info;
	unsigned long			flags;
	int	   	i;

	/* Setup base handler, and timer table. */
#if defined(MCF_HAVEDCD0) || defined(MCF_HAVEDCD1)
	init_timer(&mcfrs_timer_struct);
	mcfrs_timer_struct.function = mcfrs_timer;
	mcfrs_timer_struct.data = 0;
	mcfrs_timer_struct.expires = jiffies + HZ/25;
	add_timer(&mcfrs_timer_struct);
	mcfrs_olddcd0 = mcf_getppdcd(0);
	mcfrs_olddcd1 = mcf_getppdcd(1);
#endif /* MCF_HAVEDCD0 || MCF_HAVEDCD1 */

	show_serial_version();

	/* Initialize the tty_driver structure */
	csky_serial_driver = alloc_tty_driver(NR_PORTS);
	if (!csky_serial_driver)
		return -ENOMEM;
	
	csky_serial_driver->owner = THIS_MODULE;
	csky_serial_driver->driver_name = "serial";
	csky_serial_driver->name = "ttyS";
	csky_serial_driver->major = TTY_MAJOR;
	csky_serial_driver->minor_start = 64;
	csky_serial_driver->type = TTY_DRIVER_TYPE_SERIAL;
	csky_serial_driver->subtype = SERIAL_TYPE_NORMAL;
	csky_serial_driver->init_termios = tty_std_termios;

	csky_serial_driver->init_termios.c_cflag =
		csky_console_cbaud | CS8 | CREAD | HUPCL | CLOCAL;
	csky_serial_driver->flags = TTY_DRIVER_REAL_RAW;
	csky_serial_driver->termios = csky_serial_termios;
	csky_serial_driver->termios_locked = csky_serial_termios_locked;

	tty_set_operations(csky_serial_driver, &serial_ops);

	if (tty_register_driver(csky_serial_driver)) {
		put_tty_driver(csky_serial_driver);
		printk(KERN_ERR "Couldn't register serial driver\n");
		return -ENOMEM;
	 }
	
	local_irq_save(flags);

	/*
	 *	Configure all the attached serial ports.
	 */
	for (i = 0, info = csky_table; (i < NR_PORTS); i++, info++) {
		info->magic = SERIAL_MAGIC;
		info->line = i;
		info->tty = 0;
		info->custom_divisor = 16;
		info->close_delay = 50;
		info->closing_wait = 3000;
		info->x_char = 0;
		info->event = 0;
		info->count = 0;
		info->blocked_open = 0;
		INIT_WORK(&info->tqueue, do_softint);
                INIT_WORK(&info->tqueue_hangup, do_serial_hangup);		
		init_waitqueue_head(&info->open_wait);
		init_waitqueue_head(&info->close_wait);

		info->imr = 0;
		csky_setsignals(info, 0, 0);
		csky_irqinit(info);

		printk("%s%d at 0x%04x (irq = %d)", 
			csky_serial_driver->name,
			info->line, info->addr, info->irq);
		printk(" is a builtin C-SKY UART %d\n", info->line);
	}

	local_irq_restore(flags);
	return 0;
}

module_init(csky_init);

/****************************************************************************/
/*                          Serial Console                                  */
/****************************************************************************/
#ifdef CONFIG_CSKY_SERIAL_CONSOLE

/*
 *	Quick and dirty UART initialization, for console output.
 */
void csky_init_console(void)
{
	volatile unsigned long 	*uartp;
	unsigned long		divisor;

	/* Reset UART, get it into known state... */
	uartp = (volatile unsigned long *) CONSOLE_UART_PORT;

	/* set DLA bit in LCR and set word length to 8 bits */
	uartp[CSKY_UART_LCR] = CSKY_UART_LCR_DLAEN | CSKY_UART_LCR_WLEN8;

	/* calculate the baudrate clock */
	divisor = (unsigned long )((((CK_BUSCLK * 2) / 
		(CONSOLE_BAUD_RATE << CSKY_UART_DIV)) + 1) / 2);

#ifdef __ckcoreBE__
	/* set low byte divisor */
	uartp[CSKY_UART_DLL] = (unsigned long)(*((unsigned char *)(&divisor) + 3));
	/* set high byte divisor */
	if(divisor & 0xff00) {
	    uartp[CSKY_UART_DLH] = 
			(unsigned long)(*((unsigned char *)(&divisor) + 2));
	}  
#else	
	/* set low byte divisor */
	uartp[CSKY_UART_DLL] = divisor & 0xff;
	/* set high byte divisor */
	uartp[CSKY_UART_DLH] = (divisor >> 8) & 0xff;
#endif	
	/* clear CLA bit of LCR, and set CKUART_LCR_PEN and CKUART_LCR_WLEN8 */
	uartp[CSKY_UART_LCR] = CSKY_UART_LCR_WLEN8;
	
	/* set FIFO control 1 byte */
	uartp[CSKY_UART_FCR] = CSKY_UART_FCR_RT0;

	csky_console_inited++;
	return;
}

/*
 *	Setup for console. Argument comes from the boot command line.
 */
int csky_console_setup(struct console *cp, char *arg)
{
	int	i, n = CONSOLE_BAUD_RATE;

	if (!cp)
		return(-1);

	if (!strncmp(cp->name, "ttyS", 4))
	{
		csky_console_port = cp->index;
	}
	else if (!strncmp(cp->name, "cua", 3))
	{
		csky_console_port = cp->index;
	}
	else
	{
		return(-1);
	}

	if (arg)
	{
		n = simple_strtoul(arg,NULL,0);
	}
	for (i = 0; i < CSKY_BAUD_TABLE_SIZE; i++)
	{
		if (csky_baud_table[i] == n)
		{
			break;
		}
	}
	if (i < CSKY_BAUD_TABLE_SIZE) {
		csky_console_baud = n;
		csky_console_cbaud = 0;
		if (i > 15) {
			csky_console_cbaud |= CBAUDEX;
			i -= 15;
		}
		csky_console_cbaud |= i;
	}
	csky_init_console(); /* make sure baud rate changes */
	return(0);
}


static struct tty_driver *csky_console_device(struct console *c, int *index)
{
	*index = c -> index;
        return csky_serial_driver;
}

/*
 *	Output a single character, using UART polled mode.
 *	This is used for console output.
 */
void csky_put_char(char ch)
{
	volatile unsigned long	*uartp;
	unsigned long		flags;
	int			i;

	uartp = (volatile unsigned long *) CONSOLE_UART_PORT;

	local_irq_save(flags);
	
	/* Waiting for Transmitter Holding register empty(THRE) */
	for (i = 0; (i < 0x10000); i++) {
		if (uartp[CSKY_UART_LSR] & CSKY_UART_LSR_THRE)
			break;
	}

	uartp[CSKY_UART_THR] = (unsigned long)ch;
	local_irq_restore(flags);

	return;
}

/*
 * csky_console_write is registered for printk output.
 */
void csky_console_write(struct console *cp, const char *p, unsigned len)
{
	if (!csky_console_inited)
		csky_init_console();
	while (len-- > 0) {
		if (*p == '\n')
			csky_put_char('\r');
		csky_put_char(*p++);
	}
}

/*
 * declare our consoles
 */
struct console csky_console = {
	name:		"ttyS",
	write:		csky_console_write,
	read:		NULL,
	device:		csky_console_device,
	unblank:	NULL,
	setup:		csky_console_setup,
	flags:		CON_PRINTBUFFER,
	index:		-1,
	cflag:		0,
	next:		NULL
};

/*
 * Register console.
 */
static int __init csky_console_init(void)
{
 	register_console(&csky_console);
        return 0;
}

console_initcall(csky_console_init);

#endif //#ifdef CONFIG_CSKY_SERIAL_CONSOLE

MODULE_LICENSE("GPL");
