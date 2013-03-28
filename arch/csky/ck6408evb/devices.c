/* linux/arch/csky/ck6408ecb/devices.c
 *
 * Copyright (C) 2010 , Hu junshan<junshan_hu@c-sky.com>.
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

#include <linux/spi/spi.h>
#include <linux/spi/flash.h>
#include <linux/spi/eeprom.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/physmap.h>
#include <linux/mtd/partitions.h>

#include <mach/irqs.h>
#include "devices.h"
#include <mach/ck_iomap.h>
#include <mach/board.h>
#include <mach/ckuart.h>
#include <mach/fb.h>
#include <mach/ckspi.h>

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
static struct platform_device *__initdata ck6408evb_uarts[CSKY_MAX_UART]= {
	&csky_device_uart1,
	&csky_device_uart2,
	&csky_device_uart3,
};
struct platform_device *csky_default_console_device;	/* the serial console device */
void __init ck6408evb_set_serial_console(unsigned portnr)
{
	if (portnr < CSKY_MAX_UART)
		csky_default_console_device = ck6408evb_uarts[portnr];
}
#else
void __init ck6408evb_set_serial_console(unsigned portnr) {}
#endif

/* NAND parititon */
static struct mtd_partition default_nand_part[] = {
	[0] = {
		.name	= "Boot Load",
		.size	= 0x100000,
		.offset	= 0,
	},
	[1] = {
		.name	= "Kernel",
		.offset = 0x100000,
		.size	= 0x700000,
	},
	[2] = {
		.name	= "YAFFS2 FS",
		.offset = 0x800000,
		.size	= 0x2000000,
	},
	[3] = {
		.name	= "CSKY flash partition 3",
		.offset	= 0x2800000,
		.size	= 0x1800000,
	},
	[4] = {
	    .name   = "CSKY flash partition 4",
	    .offset = 0x4000000,
	    .size   = 0x1000000,
	},
	[5] = {
	    .name   = "CSKY flash partition 5",
	    .offset = 0x5000000,
	    .size   = 0x1000000,
	},
	[6] = {
	    .name   = "CSKY flash partition 6",
	    .offset = 0x6000000,
	    .size   = 0x1000000,
	}
};

static struct csky_nand_data csky_nand_pdata[] = {
	[0] = {
		.name		= "NAND",
		.nr_chips	= 1,
		.nr_partitions	= ARRAY_SIZE(default_nand_part),
		.partitions	= default_nand_part,
	},
};

static struct resource resources_nand[] = {
	[0] = {
	      .start  = CSKY_NFC_PHYS,
	      .end    = CSKY_NFC_PHYS + CSKY_NFC_SIZE - 1,
	      .flags  = IORESOURCE_MEM,
	},
	[1] = {
	      .start  = CSKY_NFC_BUF0_PHYS,
	      .end    = CSKY_NFC_BUF0_PHYS + CSKY_NFC_BUF_SIZE - 1,
	      .flags  = IORESOURCE_MEM,
	},
};

struct platform_device csky_device_nand = {
	.name   = "csky_nand",
	.id = -1,
		.dev		= {
			.platform_data = &csky_nand_pdata,
		},
	.num_resources  = ARRAY_SIZE(resources_nand),
	.resource   = resources_nand,
};

EXPORT_SYMBOL(csky_device_nand);

/* USB Host Controller */

static struct resource csky_usb_resource[] = {
	[0] = {
	    .start = CSKY_USB_HOST_PHYS,
	    .end   = CSKY_USB_HOST_PHYS + CSKY_USB_HOST_SIZE - 1,
	    .flags = IORESOURCE_MEM,
	},
	[1] = {
	    .start = CSKY_USBH_IRQ,
	    .end   = CSKY_USBH_IRQ,
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
	    .start = CSKY_LCD_PHYS,
	    .end   = CSKY_LCD_PHYS + CSKY_LCD_SIZE - 1,
	    .flags = IORESOURCE_MEM,
	},
	[1] = {
	    .start = CSKY_LCD_IRQ,
	    .end   = CSKY_LCD_IRQ,
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

struct csky_fb_hw ck6408evb_fb_regs = {
	.control = CSKY_LCDCON_OUT_24BIT | CSKY_LCDCON_VBL_RESERVED | \
		CSKY_LCDCON_PBS_16BITS | CSKY_LCDCON_PAS_TFT,
	.timing0 = (CSKY_HBP << 24) | (CSKY_HFP << 16) | \
		((CSKY_HLW & 0x3f) << 10) | CSKY_LCDTIM0_PPL_320,
	.timing1 = (CSKY_VBP << 24) | ((CSKY_VFP << 16)) | \
		((CSKY_VLW & 0x3f) << 10)| CSKY_LCDTIM1_LPP_240,
	.timing2 = CSKY_LCDTIM2_PCP_FAL | CSKY_LCDTIM2_HSP_ACT_LOW | \
	           CSKY_LCDTIM2_VSP_ACT_LOW | CSKY_LCDTIM2_PCD_20,
};

struct csky_fb_display ck6408evb_lcd_cfg __initdata = {
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

struct csky_fb_mach_info ck6408evb_fb_info __initdata = {
	.displays       = &ck6408evb_lcd_cfg,
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

static struct mtd_partition csky_partitions[] = {
        {
            .name = "Bootloader",
            .offset = 0,
            .size = 0x080000
        },
        {
            .name = "kernel",
            .offset = 0x080000,
            .size = 0x0280000,
        },
        {
            .name = "fs",
            .offset = 0x300000,
            .size = 0x500000
        }
};

static struct physmap_flash_data csky_nor_pdata = {
        .width          = 4,
	.parts          = csky_partitions,
        .nr_parts       = ARRAY_SIZE(csky_partitions),
};

static struct resource csky_nor_resource[] = {
        [0] = {
                .start = 0x0, 
                .end   = 0x1000000 - 1,
                .flags = IORESOURCE_MEM,
        }
};

struct platform_device csky_device_nor = {
        .name           = "physmap-flash",
        .id             = 0,
        .num_resources  = ARRAY_SIZE(csky_nor_resource),
        .resource       = csky_nor_resource,
        .dev            = {
                .platform_data = &csky_nor_pdata,
        },
};

EXPORT_SYMBOL(csky_device_nor);

static struct resource ck6408_spi_resources[] = {
        {
               .start = CSKY_SPI_PHYS,
               .end = CSKY_SPI_PHYS + CSKY_SPI_SIZE - 1,
               .flags = IORESOURCE_MEM,
        }, {
               .start = CSKY_SPI_IRQ,
               .end = CSKY_SPI_IRQ,
               .flags = IORESOURCE_IRQ,
        },
};

struct platform_device ck6408_device_spi = {
        .name = "ck6408-spi",
        .id = 0,
        .num_resources = ARRAY_SIZE(ck6408_spi_resources),
        .resource = ck6408_spi_resources,
};

EXPORT_SYMBOL(ck6408_device_spi);

static struct mtd_partition ck6408_spi_flash_partitions[] = {
	{
		.name = "bootloader(spi)",
		.size = 0x080000,
		.offset = 0,
	},
};

static struct flash_platform_data ck6408_spi_flash_data = {
	.name = "m25p80",
	.parts =  ck6408_spi_flash_partitions,
	.nr_parts = ARRAY_SIZE(ck6408_spi_flash_partitions),
	.type = "at26f004",
};

struct ck6408_spi_chip spidev_chip_info = {
	.is_eeprom      = 1,
	.timeout        = 0xffffffff,
};

static struct spi_board_info ck6408_spi_board_info[] __initdata = {
	{
		.modalias = "spidev",
		.max_speed_hz = 100000,
		.bus_num = 0,
		.chip_select = 1,
		.controller_data = &spidev_chip_info,
		.mode = SPI_MODE_0,
	},
};

void __init spi_board_init(void)
{
	spi_register_board_info(ck6408_spi_board_info,
	                  ARRAY_SIZE(ck6408_spi_board_info));
}
