/* linux/arch/csky/ckm5208/devices.h
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
 */

#ifndef __ARCH_CSKY_MACH_CKM5208_DEVICES_H
#define __ARCH_CSKY_MACH_CKM5208_DEVICES_H

extern struct platform_device csky_device_uart1;
extern struct platform_device csky_device_uart2;
extern struct platform_device csky_device_uart3;

extern struct platform_device csky_device_nand;
extern struct platform_device csky_device_usb;
extern struct platform_device csky_device_lcd;

#endif
