/*
 * arch/csky/include/asm/machdep_mm.h
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of
 * this archive for more details.
 *
 * (C) Copyright 2009, C-SKY Microsystems Co., Ltd. (www.c-sky.com)
 *  
 */

#ifndef _CSKY_MACHDEP_H
#define _CSKY_MACHDEP_H

#include <linux/seq_file.h>
#include <linux/interrupt.h>
struct pt_regs;
struct kbd_repeat;
struct mktime;
struct hwclk_time;
struct gendisk;
struct buffer_head;

extern void (*mach_time_init) (irq_handler_t handler);
/* machine dependent keyboard functions */
extern int (*mach_keyb_init) (void);
/* machine dependent irq functions */
extern void (*mach_init_IRQ) (void);
extern unsigned int (*mach_get_auto_irqno) (void);
extern void (*mach_init_FIQ) (void);
extern unsigned int (*mach_get_auto_fiqno) (void);
extern void (*mach_get_model) (char *model);
extern void (*mach_get_hardware_list) (struct seq_file *m);
/* machine dependent timer functions */
extern unsigned long (*mach_gettimeoffset)(void);
extern int (*mach_hwclk)(int, struct rtc_time*);
extern unsigned int (*mach_get_ss)(void);
extern int (*mach_get_rtc_pll)(struct rtc_pll_info *);
extern int (*mach_set_rtc_pll)(struct rtc_pll_info *);
extern int (*mach_set_clock_mmss)(unsigned long);
extern void (*mach_reset)( void );
extern void (*mach_halt)( void );
extern void (*mach_power_off)( void );
extern unsigned long (*mach_hd_init) (unsigned long, unsigned long);
extern void (*mach_hd_setup)(char *, int *);
extern long mach_max_dma_address;
extern void (*mach_heartbeat) (int);

extern void config_BSP(void);
extern void (*mach_tick)(void);
extern void (*mach_trap_init)(void);
extern const char *get_machine_type(void);

#endif /* _CSKY_MACHDEP_H */
