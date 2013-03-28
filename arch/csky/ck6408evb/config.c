/*
 * linux/arch/csky/ck6408evb/config.c
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 1999-2002, Greg Ungerer (gerg@snapgear.com)
 * Copyright (C) 2000, Lineo (www.lineo.com)
 * Copyright (C) 2004, Kang Sun (sunk@vlsi.zju.edu.cn)
 * Copyright (C) 2004, Li Chunqiang (chunqiang_li@c-sky.com)
 * Copyright (C) 2009, Hu Junshan (junshan_hu@c-sky.com)
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/param.h>
#include <linux/init.h>
#include <linux/rtc.h>
#include <linux/platform_device.h>
#include <asm/machdep.h>
#include <asm/csky.h>
#include <asm/delay.h>
#include <asm/mmu_context.h>

#include <mach/fb.h>
#include "devices.h"

extern void csky_tick(void);
extern int csky_hwclk(int set, struct rtc_time *t);
extern void __init csky_timer_init(irq_handler_t handler);
extern unsigned long csky_timer_offset(void); 
extern void __init csky_init_IRQ(void); 
extern unsigned int csky_get_auto_irqno(void); 
extern void __init ck6408evb_set_serial_console(unsigned portnr);
extern void __init spi_board_init(void);
#ifdef CONFIG_CPU_USE_FIQ
extern void __init csky_init_FIQ(void); 
#endif

static struct platform_device *devices[] __initdata = {
	&csky_device_uart1,
	&csky_device_uart2,
	&csky_device_uart3,
	&csky_device_nand,
	&csky_device_usb,
	&csky_device_lcd,
	&csky_device_nor,
	&ck6408_device_spi,
};

static int __init board_devices_init(void)
{
	csky_fb_set_platdata(&ck6408evb_fb_info); //configure mach_info for LCD
	spi_board_init();
	platform_add_devices(devices, ARRAY_SIZE(devices));

	return 0;
}


void __init config_BSP(void)
{
	printk("C-SKY CK6408EVB Board port by Hu junshan,  junshan_hu@c-sky.com\n");

	mach_time_init = csky_timer_init;
	mach_tick = csky_tick;
	mach_hwclk = csky_hwclk;
	mach_init_IRQ = csky_init_IRQ;
	mach_get_auto_irqno = csky_get_auto_irqno;
	mach_gettimeoffset = csky_timer_offset;
#ifdef CONFIG_CPU_USE_FIQ
	mach_init_FIQ = csky_init_FIQ;
#endif

	ck6408evb_set_serial_console(0);
}

const char *get_machine_type(void)
{
	return "CK6408EVB";
}

arch_initcall(board_devices_init);
