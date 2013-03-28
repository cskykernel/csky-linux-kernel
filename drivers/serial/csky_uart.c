/*
 *  csky_serial.c -- serial driver for CSKY Board UARTS.
 *
 *  Based on drivers/serial/8250.c by Russell King.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  (C) Copyright 2011, C-SKY Microsystems Co., Ltd. (www.c-sky.com)
 *  Author: Hu Junshan (junshan_hu@c-sky.com)
 *
 */
#include <linux/hrtimer.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/console.h>
#include <linux/tty.h>
#include <linux/tty_flip.h>
#include <linux/serial.h>
#include <linux/serial_core.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/irq.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/ckcore.h>
#include <mach/ckuart.h> 

#if defined (CONFIG_SERIAL_CSKY_BAUDRATE)
	#define CONSOLE_BAUD_RATE 	CONFIG_SERIAL_CSKY_BAUDRATE
#else
	#define CONSOLE_BAUD_RATE   115200 
#endif

/*
 *      Local per-uart structure.
 */
struct csky_uart {
	struct uart_port        port;
	unsigned char           ier;
	unsigned char           lcr;
	unsigned char           mcr;
	unsigned int            lsr_break_flag;
};

static inline unsigned int serial_in(struct csky_uart *up, int offset)
{
	offset <<= 2;
	return readl(up->port.membase + offset);
}

static inline void serial_out(struct csky_uart *up, int offset, int value)
{
	offset <<= 2;
	writel(value, up->port.membase + offset);
}

static void csky_enable_ms(struct uart_port *port)
{
	struct csky_uart *up = (struct csky_uart *)port;

	up->ier |= CSKY_UART_IER_EDSSI;
	serial_out(up, CSKY_UART_IER, up->ier);
}

static void csky_stop_tx(struct uart_port *port)
{
	struct csky_uart *up = (struct csky_uart *)port;

	if (up->ier & CSKY_UART_IER_ETHEI) {
		up->ier &= ~CSKY_UART_IER_ETHEI;
		serial_out(up, CSKY_UART_IER, up->ier);
	}
}

static void csky_stop_rx(struct uart_port *port)
{
	struct csky_uart *up = (struct csky_uart *)port;

	up->ier &= ~CSKY_UART_IER_ELSI;
	up->port.read_status_mask &= ~CSKY_UART_LSR_DR;
	serial_out(up, CSKY_UART_IER, up->ier);
}

static void csky_start_tx(struct uart_port *port)
{
	struct csky_uart *up = (struct csky_uart *)port;

	if (!(up->ier & CSKY_UART_IER_ETHEI)) {
		up->ier |= CSKY_UART_IER_ETHEI;
		serial_out(up, CSKY_UART_IER, up->ier);
	}
}

static void csky_rx_chars(struct csky_uart *up, int *status)
{
	struct tty_struct *tty = up->port.state->port.tty;
	unsigned int ch, flag;
	int max_count = 256;
	do {
		ch = serial_in(up, CSKY_UART_RBR);
		flag = TTY_NORMAL;
		up->port.icount.rx++;

		if (unlikely(*status & (CSKY_UART_LSR_BI | CSKY_UART_LSR_PERR |
				       CSKY_UART_LSR_FERR | CSKY_UART_LSR_OVRERR))) {
			/*
			 * For statistics only
			 */
			if (*status & CSKY_UART_LSR_BI) {
				*status &= ~(CSKY_UART_LSR_FERR | CSKY_UART_LSR_PERR);
				up->port.icount.brk++;
				/*
				 * We do the SysRQ and SAK checking
				 * here because otherwise the break
				 * may get masked by ignore_status_mask
				 * or read_status_mask.
				 */
				if (uart_handle_break(&up->port))
					goto ignore_char;
			} else if (*status & CSKY_UART_LSR_PERR)
				up->port.icount.parity++;
			else if (*status & CSKY_UART_LSR_FERR)
				up->port.icount.frame++;
			if (*status & CSKY_UART_LSR_OVRERR)
				up->port.icount.overrun++;

			/*
			 * Mask off conditions which should be ignored.
			 */
			*status &= up->port.read_status_mask;

#ifdef CONFIG_SERIAL_CSKY_CONSOLE
			if (up->port.line == up->port.cons->index) {
				/* Recover the break flag from console xmit */
				*status |= up->lsr_break_flag;
				up->lsr_break_flag = 0;
			}
#endif
			if (*status & CSKY_UART_LSR_BI) {
				flag = TTY_BREAK;
			} else if (*status & CSKY_UART_LSR_PERR)
				flag = TTY_PARITY;
			else if (*status & CSKY_UART_LSR_FERR)
				flag = TTY_FRAME;
		}

		if (uart_handle_sysrq_char(&up->port, ch))
			goto ignore_char;

		uart_insert_char(&up->port, *status, CSKY_UART_LSR_OVRERR, ch, flag);

	ignore_char:
		*status = serial_in(up, CSKY_UART_LSR);
	} while ((*status & CSKY_UART_LSR_DR) && (max_count-- > 0));
	tty_flip_buffer_push(tty);
}

static void csky_tx_chars(struct csky_uart *up)
{
	struct circ_buf *xmit = &up->port.state->xmit;	
	int count;
	
	if (up->port.x_char) {
		/* Send special char - probably flow control */
		serial_out(up, CSKY_UART_THR, up->port.x_char);
		up->port.icount.tx++;
		up->port.x_char = 0;
		return;
	}
	if (uart_circ_empty(xmit) || uart_tx_stopped(&up->port)) {
		csky_stop_tx(&up->port);
		return;
	}

	count = up->port.fifosize / 2;
	do {
		serial_out(up, CSKY_UART_THR, xmit->buf[xmit->tail]);
		xmit->tail = (xmit->tail + 1) & (UART_XMIT_SIZE - 1);
		up->port.icount.tx++;
		if (uart_circ_empty(xmit))
			break;
	} while (--count > 0);

	if (uart_circ_chars_pending(xmit) < WAKEUP_CHARS)
		uart_write_wakeup(&up->port);

	if (uart_circ_empty(xmit))
		csky_stop_tx(&up->port);
}


static inline void check_modem_status(struct csky_uart *up)
{
	int status;
	
	status = serial_in(up, CSKY_UART_MSR);

	if ((status & 0x0F) == 0)
		return;

	if (status & CSKY_UART_MSR_TERI)
		up->port.icount.rng++;
	if (status & CSKY_UART_MSR_DDSR)
		up->port.icount.dsr++;
	if (status & CSKY_UART_MSR_DDCD)
		uart_handle_dcd_change(&up->port, status & CSKY_UART_MSR_DCD);
	if (status & CSKY_UART_MSR_DCTS)
		uart_handle_cts_change(&up->port, status & CSKY_UART_MSR_CTS);

	wake_up_interruptible(&up->port.state->port.delta_msr_wait);
}

static irqreturn_t csky_interrupt(int irq, void *data)
{
	struct csky_uart *up = data;
	unsigned int iir, lsr;

	iir = serial_in(up, CSKY_UART_IIR);
	if (iir & CSKY_UART_IIR_NONE)
		return IRQ_NONE;
	lsr = serial_in(up, CSKY_UART_LSR); 
	if (lsr & CSKY_UART_LSR_DR)
		csky_rx_chars(up, &lsr);
	check_modem_status(up);
	if (lsr & CSKY_UART_LSR_THRE)
		csky_tx_chars(up);
	return IRQ_HANDLED;
}

static unsigned int csky_tx_empty(struct uart_port *port)
{
	struct csky_uart *up = (struct csky_uart *)port;
	unsigned long flags;
	unsigned int ret;

	spin_lock_irqsave(&up->port.lock, flags);
	ret = serial_in(up, CSKY_UART_LSR) & CSKY_UART_LSR_TEMT ? TIOCSER_TEMT : 0;
	spin_unlock_irqrestore(&up->port.lock, flags);

	return ret;
}

static unsigned int csky_get_mctrl(struct uart_port *port)
{
	struct csky_uart *up = (struct csky_uart *)port;
	unsigned char status;
	unsigned int ret;

	status = serial_in(up, CSKY_UART_MSR);

	ret = 0;
	if (status & CSKY_UART_MSR_DCD)
		ret |= TIOCM_CAR;
	if (status & CSKY_UART_MSR_RI)
		ret |= TIOCM_RNG;
	if (status & CSKY_UART_MSR_DSR)
		ret |= TIOCM_DSR;
	if (status & CSKY_UART_MSR_CTS)
		ret |= TIOCM_CTS;
	return ret;
}

static void csky_set_mctrl(struct uart_port *port, unsigned int mctrl)
{
	struct csky_uart *up = (struct csky_uart *)port;
	unsigned char mcr = 0;

	if (mctrl & TIOCM_RTS)
		mcr |= CSKY_UART_MCR_RTS;
	if (mctrl & TIOCM_DTR)
		mcr |= CSKY_UART_MCR_DTR;
	if (mctrl & TIOCM_OUT1)
		mcr |= CSKY_UART_MCR_OUT1;
	if (mctrl & TIOCM_OUT2)
		mcr |= CSKY_UART_MCR_OUT2;
	if (mctrl & TIOCM_LOOP)
		mcr |= CSKY_UART_MCR_LB;

	mcr |= up->mcr;
	serial_out(up, CSKY_UART_MCR, mcr);
}

static void csky_break_ctl(struct uart_port *port, int break_state)
{
	struct csky_uart *up = (struct csky_uart *)port;
	unsigned long flags;

	spin_lock_irqsave(&up->port.lock, flags);
	if (break_state == -1)
		up->lcr |= CSKY_UART_LCR_BC;
	else
		up->lcr &= ~CSKY_UART_LCR_BC;
	serial_out(up, CSKY_UART_LCR, up->lcr);
	spin_unlock_irqrestore(&up->port.lock, flags);

}

static int csky_startup(struct uart_port *port)
{
	struct csky_uart *up = (struct csky_uart *)port;
	unsigned long flags;

	up->mcr = 0;
		
	if (request_irq(up->port.irq, csky_interrupt, 0, "CSKY UART", up))
         printk(KERN_ERR "CSKY: unable to attach UART %d "
                "interrupt vector=%d\n", port->line, port->irq);

	/*
	 * Clear the FIFO buffers and disable them.
	 * (they will be reenabled in set_termios())
	 */
	 
	serial_out(up, CSKY_UART_FCR, CSKY_UART_FCR_FIFOE);
	serial_out(up, CSKY_UART_FCR, CSKY_UART_FCR_FIFOE |
		CSKY_UART_FCR_RFIFOR | CSKY_UART_FCR_XFIFOR);
	serial_out(up, CSKY_UART_FCR, 0);

	/*
	 * Clear the interrupt registers.
	 */
	(void) serial_in(up, CSKY_UART_LSR);
	(void) serial_in(up, CSKY_UART_RBR);
	(void) serial_in(up, CSKY_UART_IIR);
	(void) serial_in(up, CSKY_UART_MSR);

	while ( serial_in(up, CSKY_UART_USR) & CSKY_UART_USR_BUSY);	
    
	/*
	 * Now, initialize the UART
	 */
	serial_out(up, CSKY_UART_LCR, CSKY_UART_LCR_DLAEN | CSKY_UART_LCR_WLEN8);
	
	spin_lock_irqsave(&up->port.lock, flags);
	up->port.mctrl |= TIOCM_OUT2;
	csky_set_mctrl(&up->port, up->port.mctrl);
	spin_unlock_irqrestore(&up->port.lock, flags);
  
	/*
	 * Finally, enable interrupts.  Note: Modem status interrupts
	 * are set via set_termios(), which will be occurring imminently
	 * anyway, so we don't enable them here.
	 */
	 
	/* Enable RX interrupts now */
	up->ier = CSKY_UART_IER_ELSI | CSKY_UART_IER_ERBFI;        //|  CSKY_UART_IER_ERDAI
	serial_out(up, CSKY_UART_IER, up->ier);

	/*
	 * And clear the interrupt registers again for luck.
	 */
	(void) serial_in(up, CSKY_UART_LSR);
	(void) serial_in(up, CSKY_UART_RBR);
	(void) serial_in(up, CSKY_UART_IIR);
	(void) serial_in(up, CSKY_UART_MSR);

	return 0;
}

static void csky_shutdown(struct uart_port *port)
{
	struct csky_uart *up = (struct csky_uart *)port;
	unsigned long flags;
	
	free_irq(up->port.irq, up);
	
	/* Disable all interrupts now */
	up->ier = 0;
	serial_out(up, CSKY_UART_IER, 0);

	spin_lock_irqsave(&up->port.lock, flags);
	up->port.mctrl &= ~TIOCM_OUT2;
    csky_set_mctrl(&up->port, up->port.mctrl);	
	spin_unlock_irqrestore(&up->port.lock, flags);
	
	/*
	 * Disable break condition and FIFOs
	 */
	serial_out(up, CSKY_UART_LCR, serial_in(up, CSKY_UART_LCR) & ~CSKY_UART_LCR_BC);
	serial_out(up, CSKY_UART_FCR, CSKY_UART_FCR_FIFOE |
				  CSKY_UART_FCR_RFIFOR |
				  CSKY_UART_FCR_XFIFOR);
	serial_out(up, CSKY_UART_FCR, 0);
}

static void csky_set_termios(struct uart_port *port, struct ktermios *termios,
        struct ktermios *old)
{
	struct csky_uart *up = (struct csky_uart *)port;
	unsigned char cval, fcr = 0;
	unsigned long flags;
	unsigned int baud, baudclk;

	switch (termios->c_cflag & CSIZE) {
	case CS5:
		cval = CSKY_UART_LCR_WLEN5;
		break;
	case CS6:
		cval = CSKY_UART_LCR_WLEN6;
		break;
	case CS7:
		cval = CSKY_UART_LCR_WLEN7;
		break;
	default:
	case CS8:
		cval = CSKY_UART_LCR_WLEN8;
		break;
	}

	if (termios->c_cflag & CSTOPB)
		cval |= CSKY_UART_LCR_STOP;
	if (termios->c_cflag & PARENB)
		cval |= CSKY_UART_LCR_PEN;
	if (!(termios->c_cflag & PARODD))
		cval |= CSKY_UART_LCR_ESP;

	/*
	 * Ask the core to calculate the divisor for us.
	 */
	baud = uart_get_baud_rate(port, termios, old, 0, 115200*8);
	baudclk = (unsigned long)((((CK_BUSCLK * 2) /
        (baud << CSKY_UART_DIV)) + 1) / 2);

	fcr = CSKY_UART_FCR_FIFOE | CSKY_UART_FCR_RT1;  //FIXME:

	/*
	 * Ok, we're now changing the port state.  Do it with
	 * interrupts disabled.
	 */
	spin_lock_irqsave(&port->lock, flags);

	/*
	 * Ensure the port will be enabled.
	 * This is required especially for serial console.
	 */
	up->ier |= 0;       //UART_IER_UUE
	
	/*
	 * Update the per-port timeout.
	 */
	uart_update_timeout(port, termios->c_cflag, baud);

	up->port.read_status_mask = CSKY_UART_LSR_OVRERR | CSKY_UART_LSR_THRE | CSKY_UART_LSR_DR;

	if (termios->c_iflag & INPCK)
		up->port.read_status_mask |= CSKY_UART_LSR_FERR | CSKY_UART_LSR_PERR;
	if (termios->c_iflag & (BRKINT | PARMRK))
		up->port.read_status_mask |= CSKY_UART_LSR_BI;

	/*
	 * Characters to ignore
	 */
	up->port.ignore_status_mask = 0;
	if (termios->c_iflag & IGNPAR)
		up->port.ignore_status_mask |= CSKY_UART_LSR_PERR | CSKY_UART_LSR_FERR;
	if (termios->c_iflag & IGNBRK) {
		up->port.ignore_status_mask |= CSKY_UART_LSR_BI;
		/*
		 * If we're ignoring parity and break indicators,
		 * ignore overruns too (for real raw support).
		 */
		if (termios->c_iflag & IGNPAR)
			up->port.ignore_status_mask |= CSKY_UART_LSR_OVRERR;
	}

	/*
	 * ignore all characters if CREAD is not set
	 */
	if ((termios->c_cflag & CREAD) == 0)
		up->port.ignore_status_mask |= CSKY_UART_LSR_DR;

	/*
	 * CTS flow control flag and modem status interrupts
	 */
	up->ier &= ~CSKY_UART_IER_EDSSI;
	if (UART_ENABLE_MS(&up->port, termios->c_cflag))
		up->ier |= CSKY_UART_IER_EDSSI;

	serial_out(up, CSKY_UART_IER, up->ier);

	if (termios->c_cflag & CRTSCTS)
		up->mcr |= CSKY_UART_MCR_AFCE;
	else
		up->mcr &= ~CSKY_UART_MCR_AFCE;
	
	while ( serial_in(up, CSKY_UART_USR) & CSKY_UART_USR_BUSY);

	serial_out(up, CSKY_UART_LCR, cval | CSKY_UART_LCR_DLAEN);/* set DLAB */
	serial_out(up, CSKY_UART_DLL, (baudclk & 0xff));		/* LS of divisor */
	serial_out(up, CSKY_UART_DLH, baudclk >> 8);
	serial_out(up, CSKY_UART_LCR, cval);		/* reset DLAB */
	up->lcr = cval;					/* Save LCR */
	csky_set_mctrl(&up->port, up->port.mctrl);
	serial_out(up, CSKY_UART_FCR, fcr);
	spin_unlock_irqrestore(&port->lock, flags);
}

static void csky_config_port(struct uart_port *port, int flags)
{
	port->type = PORT_16550;
}

static const char *csky_type(struct uart_port *port)
{
	return (port->type == PORT_16550) ? "CSKY UART(simple 16550)" : NULL;
}

static int csky_request_port(struct uart_port *port)
{
	/* UARTs always present */
	return 0;
}

static void csky_release_port(struct uart_port *port)
{
	/* Nothing to release... */
}

static int csky_verify_port(struct uart_port *port, struct serial_struct *ser)
{
	if ((ser->type != PORT_UNKNOWN) && (ser->type != PORT_16550))
		return -EINVAL;
	return 0;
}

/*
 *      Define the basic serial functions we support.
 */
static struct uart_ops csky_uart_ops = {
        .tx_empty       = csky_tx_empty,
        .get_mctrl      = csky_get_mctrl,
        .set_mctrl      = csky_set_mctrl,
        .start_tx       = csky_start_tx,
        .stop_tx        = csky_stop_tx,
        .stop_rx        = csky_stop_rx,
        .enable_ms      = csky_enable_ms,
        .break_ctl      = csky_break_ctl,
        .startup        = csky_startup,
        .shutdown       = csky_shutdown,
        .set_termios    = csky_set_termios,
        .type           = csky_type,
        .request_port   = csky_request_port,
        .release_port   = csky_release_port,
        .config_port    = csky_config_port,
        .verify_port    = csky_verify_port,
};

static struct csky_uart csky_ports[CSKY_MAX_UART];

/****************************************************************************/
/*                          Serial Console                                  */
/****************************************************************************/
#ifdef CONFIG_SERIAL_CSKY_CONSOLE

static int __init early_csky_setup(void)
{
        struct uart_port *port;
        struct resource *resource;
	struct platform_device *devs;
	
	devs = csky_default_console_device;
	
	if(csky_default_console_device)
	{
		port = &csky_ports[devs->id].port;
		port->line = devs->id;
		port->type = PORT_16550;

		resource = platform_get_resource(devs, IORESOURCE_MEM, 0);
		if (unlikely(!resource))
	 	   return -ENXIO;
		port->mapbase = resource->start;
		port->membase = ioremap(resource->start, 
			resource->end - resource->start + 1);
		port->iotype = SERIAL_IO_MEM;
		port->uartclk = CK_BUSCLK;
		port->flags = ASYNC_BOOT_AUTOCONF;
		port->ops = &csky_uart_ops;
	}

        return 0;
}

/*
 *	Setup for console. Argument comes from the boot command line.
 */
static int __init csky_console_setup(struct console *co, char *options)
{
	struct uart_port *port;
	int baud = CONFIG_SERIAL_CSKY_BAUDRATE;
	int bits = 8;
	int parity = 'n';
	int flow = 'n';
	
	if ((co->index < 0) || (co->index >= CSKY_MAX_UART))
		co->index = 0;
	port = &csky_ports[co->index].port;
	if (port->membase == 0)
		 return -ENODEV;
	
	if (options)
		uart_parse_options(options, &baud, &parity, &bits, &flow);
	
	return uart_set_options(port, co, baud, parity, bits, flow);	
}

#define BOTH_EMPTY (CSKY_UART_LSR_TEMT | CSKY_UART_LSR_THRE)

/*
 *	Wait for transmitter & holding register to empty
 */
static inline void wait_for_xmitr(struct csky_uart *up)
{
	unsigned int status, tmout = 10000;

	/* Wait up to 10ms for the character(s) to be sent. */
	do {
		status = serial_in(up, CSKY_UART_LSR);

		if (status & CSKY_UART_LSR_BI)
			up->lsr_break_flag = CSKY_UART_LSR_BI;

		if (--tmout == 0)
			break;
		udelay(1);
	} while ((status & BOTH_EMPTY) != BOTH_EMPTY);

	/* Wait up to 1s for flow control if necessary */
	if (up->port.flags & UPF_CONS_FLOW) {
		tmout = 1000000;
		while (--tmout && 
		    ((serial_in(up, CSKY_UART_MSR) & CSKY_UART_MSR_CTS) == 0))
			udelay(1);
	}
}


/*
 *	Output a single character, using UART polled mode.
 *	This is used for console output.
 */
static void csky_console_putc(struct uart_port *port, int ch)
{

	struct csky_uart *up = (struct csky_uart *)port;

	wait_for_xmitr(up);
	serial_out(up, CSKY_UART_THR, ch);
}

/*
 * csky_console_write is registered for printk output.
 */
static void csky_console_write(struct console *co, const char *s, unsigned int count)
{
	struct csky_uart *up = &csky_ports[co->index];
	unsigned int ier;

	/*
	 *	First save the IER then disable the interrupts
	 */
	ier = serial_in(up, CSKY_UART_IER);
	
	serial_out(up, CSKY_UART_IER, 0);

	uart_console_write(&up->port, s, count, csky_console_putc);

	/*
	 *	Finally, wait for transmitter to become empty
	 *	and restore the IER
	 */
	wait_for_xmitr(up);
	serial_out(up, CSKY_UART_IER, ier);
}

static struct uart_driver csky_driver;

struct console csky_console = {
	.name   = "ttyS",
	.write  = csky_console_write,
	.device = uart_console_device,
	.setup  = csky_console_setup,
	.early_setup = early_csky_setup,
	.flags  = CON_PRINTBUFFER,
	.index  = -1,
	.data   = &csky_driver,	
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

#define CSKY_CONSOLE     (&csky_console)

#else

#define CSKY_CONSOLE     NULL

#endif //#ifdef CONFIG_SERIAL_CSKY_CONSOLE

/*
 *      Define the csky UART driver structure.
 */
static struct uart_driver csky_driver = {
        .owner          = THIS_MODULE,
        .driver_name    = "csky",
        .dev_name       = "ttyS",
        .major          = TTY_MAJOR,
        .minor          = 64,
        .nr             = CSKY_MAX_UART,
        .cons           = CSKY_CONSOLE,
};

static int __devinit csky_probe(struct platform_device *pdev)
{
	struct csky_uart *cport;
	struct resource *mmres, *irqres;
	int ret;

	mmres = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	irqres = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (!mmres || !irqres)
		return -ENODEV;	

	cport = kzalloc(sizeof(struct csky_uart), GFP_KERNEL);
	if (!cport)
		return -ENOMEM;
	
	cport->port.type = PORT_16550;
	cport->port.iotype = SERIAL_IO_MEM;
	cport->port.mapbase = mmres->start;
	cport->port.irq = irqres->start;
	cport->port.fifosize = 32;
	cport->port.ops = &csky_uart_ops;
	cport->port.line = pdev->id;
	cport->port.dev = &pdev->dev;
	cport->port.flags = ASYNC_BOOT_AUTOCONF;
	cport->port.uartclk = CK_BUSCLK;

	cport->port.membase = ioremap(mmres->start, mmres->end - mmres->start + 1);
	if (!cport->port.membase) {
		ret = -ENOMEM;
		goto err_free;
	}	

	csky_ports[pdev->id] = *cport; 
	
	uart_add_one_port(&csky_driver, &cport->port);
	platform_set_drvdata(pdev, cport);

	return 0;

err_free:
	kfree(cport);
	return ret;
}
static int csky_remove(struct platform_device *pdev)
{
	struct csky_uart *cport = platform_get_drvdata(pdev);	

	platform_set_drvdata(pdev, NULL);

	uart_remove_one_port(&csky_driver, &cport->port);
	kfree(cport);

	return 0;
}
static struct platform_driver csky_platform_driver = {
        .probe          = csky_probe,
        .remove         = __devexit_p(csky_remove),
        .driver         = {
                .name   = "cskyuart",
                .owner  = THIS_MODULE,
        },
};
static int __init csky_init(void)
{
        int rc;

        printk("C-SKY internal UART serial driver\n");

        rc = uart_register_driver(&csky_driver);
        if (rc)
                return rc;
        rc = platform_driver_register(&csky_platform_driver);
        if (rc != 0)
		uart_unregister_driver(&csky_driver);

		return rc;
}

static void __exit csky_exit(void)
{
        platform_driver_unregister(&csky_platform_driver);
        uart_unregister_driver(&csky_driver);
}

module_init(csky_init);
module_exit(csky_exit);

MODULE_AUTHOR("Hu junshan <junshan_hu@c-sky.com>");
MODULE_DESCRIPTION("C-sky SOC internal UART driver");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:cskyuart");
