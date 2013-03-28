/* linux/arch/csky/ckm5208/devices.c
 *
 * Copyright (C) 2011 C-SKY Microsystems Co., Ltd.
 * Author: Hu junshan <junshan_hu@c-sky.com> 
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/platform_device.h>

#include <mach/irqs.h>
#include "devices.h"
#include <mach/ck_iomap.h>
#include <mach/ckuart.h>
#include <mach/fb.h>

static struct resource resources_uart1[] = {
	{
		.start  = CSKY_UART0_IRQ,
		.end    = CSKY_UART0_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.start  = CSKY_UART0_PHYS,
		.end    = CSKY_UART0_PHYS + CSKY_UART_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
};

static struct resource resources_uart2[] = {
	{
		.start  = CSKY_UART1_IRQ,
		.end    = CSKY_UART1_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.start  = CSKY_UART1_PHYS,
		.end    = CSKY_UART1_PHYS + CSKY_UART_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
};

static struct resource resources_uart3[] = {
	{
		.start  = CSKY_UART2_IRQ,
		.end    = CSKY_UART2_IRQ,
		.flags  = IORESOURCE_IRQ,
	},
	{
		.start  = CSKY_UART2_PHYS,
		.end    = CSKY_UART2_PHYS + CSKY_UART_SIZE - 1,
		.flags  = IORESOURCE_MEM,
	},
};

struct platform_device csky_device_uart1 = {
	.name   = "cskyuart",
	.id = 0,
	.num_resources  = ARRAY_SIZE(resources_uart1),
	.resource   = resources_uart1,
};

struct platform_device csky_device_uart2 = {
	.name   = "cskyuart",
	.id = 1,
	.num_resources  = ARRAY_SIZE(resources_uart2),
	.resource   = resources_uart2,
};

struct platform_device csky_device_uart3 = {
	.name   = "cskyuart",
	.id = 2,
	.num_resources  = ARRAY_SIZE(resources_uart3),
	.resource   = resources_uart3,
};

#ifdef CONFIG_SERIAL_CSKY_CONSOLE
static struct platform_device *__initdata ckm5208_uarts[CSKY_MAX_UART]= {
	&csky_device_uart1,
	&csky_device_uart2,
	&csky_device_uart3,
};
struct platform_device *csky_default_console_device;	/* the serial console device */
void __init ckm5208_set_serial_console(unsigned portnr)
{
	if (portnr < CSKY_MAX_UART)
		csky_default_console_device = ckm5208_uarts[portnr];
}
#else
void __init ckm5208_set_serial_console(unsigned portnr) {}
#endif

/* USB Host Controller */

static struct resource csky_usb_resource[] = {
	[0] = {
	    .start = CSKY_USB_HOST_PHYS,
	    .end   = CSKY_USB_HOST_PHYS + CSKY_USB_HOST_SIZE - 1,
	    .flags = IORESOURCE_MEM,
	},
	[1] = {
	    .start = CSKY_USB_IRQ,
	    .end   = CSKY_USB_IRQ,
	    .flags = IORESOURCE_IRQ,
	}
};

static u64 csky_device_usb_dmamask = 0xffffffffUL;

struct platform_device csky_device_usb = {
	.name          = "csky-ohci",
	.id            = -1,
	.num_resources = ARRAY_SIZE(csky_usb_resource),
	.resource      = csky_usb_resource,
	.dev           = {
	    .dma_mask          = &csky_device_usb_dmamask,
	    .coherent_dma_mask = 0xffffffffUL
	}
};

EXPORT_SYMBOL(csky_device_usb);

/* LCD Controller */
static struct resource csky_lcd_resource[] = {
	[0] = {
	    .start = CSKY_LCDC_PHYS,
	    .end   = CSKY_LCDC_PHYS + CSKY_LCDC_SIZE - 1,
	    .flags = IORESOURCE_MEM,
	},
	[1] = {
	    .start = CSKY_LCDC_IRQ,
	    .end   = CSKY_LCDC_IRQ,
	    .flags = IORESOURCE_IRQ,
	}
};

static u64 csky_device_lcd_dmamask = 0xffffffffUL;

struct platform_device csky_device_lcd = {
	.name          = "csky-lcd",
	.id            = -1,
	.num_resources = ARRAY_SIZE(csky_lcd_resource),
	.resource      = csky_lcd_resource,
	.dev           = {
	    .dma_mask          = &csky_device_lcd_dmamask,
	    .coherent_dma_mask = 0xffffffffUL
	}
};
EXPORT_SYMBOL(csky_device_lcd);

struct csky_fb_hw ckm5208_fb_regs = {
	.control = CSKY_LCDCON_OUT_24BIT | CSKY_LCDCON_VBL_RESERVED | \
	           CSKY_LCDCON_PBS_16BITS | CSKY_LCDCON_PAS_TFT,
	.timing0 = CSKY_LCDTIM0_HSW_16 | CSKY_LCDTIM0_PPL_320,
	.timing1 = CSKY_LCDTIM1_EFW_2 | CSKY_LCDTIM1_VSW_2 | \
	           CSKY_LCDTIM1_LPP_240,
	.timing2 = CSKY_LCDTIM2_PCP_FAL | CSKY_LCDTIM2_HSP_ACT_LOW | \
	           CSKY_LCDTIM2_VSP_ACT_LOW | CSKY_LCDTIM2_PCD_20,
};


struct csky_fb_display ckm5208_lcd_cfg __initdata = {
	.timing2        = CSKY_LCDTIM2_PCP_FAL | CSKY_LCDTIM2_HSP_ACT_LOW | \
	                  CSKY_LCDTIM2_VSP_ACT_LOW | CSKY_LCDTIM2_PCD_20,

	.type           = CSKY_LCDCON_PAS_TFT,

	.width          = 320,
	.height         = 240,
	
	.pixclock       = 500000, // HCLK 40 MHz, divisor 20
	.xres           = 320,
	.yres           = 240,
	.bpp            = 32,     // if 24, bpp 888 and 8 dummy
	.left_margin    = 0,
	.right_margin   = 0,
	.hsync_len      = 16,     // unit is pixel clock period
	.upper_margin   = 0,
	.lower_margin   = 2,      // unit is line clock period
	.vsync_len      = 2,
};

struct csky_fb_mach_info ckm5208_fb_info __initdata = {
	.displays       = &ckm5208_lcd_cfg,
	.num_displays   = 1,
	.default_display = 0,
};

/* Set platform data(csky_fb_mach_info) for csky_device_lcd */
void __init csky_fb_set_platdata(struct csky_fb_mach_info *pd)
{
	struct csky_fb_mach_info *npd;

	npd = kmalloc(sizeof(*npd), GFP_KERNEL);
	if (npd) {
		memcpy(npd, pd, sizeof(*npd));
		csky_device_lcd.dev.platform_data = npd;
	} else {
		printk(KERN_ERR "no memory for LCD platform data\n");
	}
}

