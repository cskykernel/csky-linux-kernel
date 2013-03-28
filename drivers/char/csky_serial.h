/*
 *  csky_serial.h -- serial driver header file for 
 *  CK1000-EVB and CK6408EVB Board UARTS.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  (C) Copyright 2004, Li Chunqiang (chunqiang_li@c-sky.com)
 *  (C) Copyright 2009, Hu Junshan (junshan_hu@c-sky.com)
 *  (C) Copyright 2009, C-SKY Microsystems Co., Ltd. (www.c-sky.com) 
 */
 
#ifndef _CSKY_SERIAL_H_
#define _CSKY_SERIAL_H_

#include <linux/serial.h>

#ifdef __KERNEL__

/*
 *	Define a local serial stats structure.
 */

struct csky_stats {
	unsigned int	rx;
	unsigned int	tx;
	unsigned int	rxbreak;
	unsigned int	rxframing;
	unsigned int	rxparity;
	unsigned int	rxoverrun;
};


/*
 * This is our internal structure for each serial port's state.
 * Each serial port has one of these structures associated with it.
 */

struct csky_serial {
	int					magic;
	unsigned int		addr;			/* UART memory address */
	int					irq;
	int					flags; 			/* defined in tty.h */
	int					type; 			/* UART type */
	struct tty_struct 	*tty;
	unsigned char		imr;			/* Software control register */
	unsigned int		baud;
	int					sigs;
	int					custom_divisor;
	int					x_char;	        /* xon/xoff character */
	int					baud_base;
	int					close_delay;
	unsigned short		closing_wait;
	unsigned short		closing_wait2;
	unsigned long		event;
	int					line;
	int					count;	        /* # of fd on device */
	int					blocked_open;   /* # of blocked opens */
	long				session;        /* Session of opening process */
	long				pgrp;           /* pgrp of opening process */
	unsigned char 		*xmit_buf;
	int					xmit_head;
	int					xmit_tail;
	int					xmit_cnt;
	struct csky_stats	stats;
	struct work_struct	tqueue;
	struct work_struct	tqueue_hangup;
	wait_queue_head_t	open_wait;
	wait_queue_head_t	close_wait;
};

#endif /* __KERNEL__ */

#endif /* _CSKY_SERIAL_H_ */
