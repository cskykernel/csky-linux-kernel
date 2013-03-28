/* arch/csky/ck6408evb/include/mach/ck_iomap.h
 *
 * Copyright (C) 2010 C-SKY Microsystems Co., Ltd.
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
 *
 * The MSM peripherals are spread all over across 768MB of physical
 * space, which makes just having a simple IO_ADDRESS macro to slide
 * them into the right virtual location rough.  Instead, we will
 * provide a master phys->virt mapping for peripherals here.
 *
 */

#ifndef __ASM_ARCH_CK_IOMAP_H
#define __ASM_ARCH_CK_IOMAP_H

/* AHB arbiter */
#define CSKY_AHB_PHYS           0x10000000   /* Base address of AHB */
#define CSKY_AHB_SIZE           0x1000       /* AHB Reg size is 4k */

/* MMCA */
#define CSKY_MMCA_PHYS          0x10001000   /* Base address of MMCA */
#define CSKY_MMCA_SIZE          0x1000       /* MMCA Reg size is 4k */

/* Power Management */
#define CSKY_PM_PHYS            0x10002000
#define CSKY_PM_SIZE            0x1000

/* DMA controller */
#define CSKY_DMA_PHYS           0x10003000
#define CSKY_DMA_SIZE           0x1000

/* LCDC controller */
#define CSKY_LCDC_PHYS          0x10004000
#define CSKY_LCDC_SIZE          0x1000

/* USB device controller */
#define CSKY_USBD_PHYS          0x10005000
#define CSKY_USBD_SIZE          0x1000

/* MAC controller -registers */
#define CSKY_MACC_PHYS          0x10006000
#define CSKY_MACC_SIZE          0x1000

/* MAC controller -buffer descriptor */
#define CSKY_MAC_BUF_PHYS       0x10007400
#define CSKY_MAC_BUF_SIZE       0x1000

/* NFC controller */
#define CSKY_NFC_PHYS           0x10008000   
#define CSKY_NFC_SIZE           0x1000      

#define CSKY_NFC_BUF0_PHYS      0x10009000   /* Base address of NFC BUF1 */
#define CSKY_NFC_BUF1_PHYS      0x1000A000   /* Base address of NFC BUF2 */ 
#define CSKY_NFC_BUF_SIZE       0x1000       /* NFC BUF size is 4k */

/* USB host controller */
#define CSKY_USB_HOST_PHYS      0x1000b000
#define CSKY_USB_HOST_SIZE      0x1000

/* Interrupt controller*/
#define CSKY_INT_PHYS           0x10010000
#define CSKY_INT_SIZE           0x1000

/* Timer */
#define CSKY_TIMER_PHYS         0x10011000
#define CSKY_TIMER_SIZE         0x1000
 
/* Real Time clock */
#define CSKY_RTC_PHYS           0x10012000
#define CSKY_RTC_SIZE           0x1000

/* Watchdog */
#define CSKY_WTD_PHYS           0x10013000
#define CSKY_WTD_SIZE           0x1000

/* PWM */
#define CSKY_PWM_PHYS           0x10014000
#define CSKY_PWM_SIZE           0x1000

/* UART */
#define CSKY_UART0_PHYS         0x10015000   /* Base address of UART0 */
#define CSKY_UART1_PHYS         0x10016000   /* Base address of UART1 */
#define CSKY_UART2_PHYS         0x10017000   /* Base address of UART2 */
#define CSKY_UART_SIZE          0x1000       /* UART Reg size is 4k */

/* I2C bus controller */
#define CSKY_IIC_PHYS           0x10018000
#define CSKY_IIC_SIZE           0x1000

/* GPIO controller */
#define CSKY_GPIO_PHYS          0x10019000
#define CSKY_GPIO_SIZE          0x1000 

/* SPI Maste */
#define CSKY_SPI_PHYS           0x1001A000
#define CSKY_SPI_SIZE           0x1000 

/* AC97 */
#define CSKY_AC97_PHYS           0x1001B000
#define CSKY_AC97_SIZE           0x1000 

/* SDHC */
#define CSKY_SDHC_PHYS           0x1001C000
#define CSKY_SDHC_SIZE           0x1000

/* LCD */
#define CSKY_LCD_BASE             0x10004000
#define CSKY_LCD_PHYS             0x10004000
#define CSKY_LCD_SIZE             0x1000

/* GPIO */
#define CSKY_GPIO_BASE            0x10019000
#define CSKY_GPIO_SIZE            0x1000

#endif /* __ASM_ARCH_CK_IOMAP_H */
