/* arch/csky/ck6408evb/include/mach/cknand.h
 *
 * Copyright (c) 2010 Hu Junshan <junshan_hu@c-sky.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * CSKY NAND register definitions
*/

#ifndef __ASM_ARM_REGS_NAND
#define __ASM_ARM_REGS_NAND

#define CSKY_NFC_EN          0x0  // Nand Flash Controller enable
#define CSKY_NFC_IMASK       0x4  // interrupt mask register
#define CSKY_NFC_DEVICE_CFG  0x8  // device configuring register
#define CSKY_NFC_IDR         0xC  // nand flash ID register
#define CSKY_NFC_COLAR       0x10 // column address register
#define CSKY_NFC_ROWAR       0x14 // row address register
#define CSKY_NFC_CMDR        0x18  // command register
#define CSKY_NFC_SR          0x1C  // status register
#define CSKY_NFC_ECC_CODE1   0x20  // 24 bits ECC code for the first 1024 bytes
#define CSKY_NFC_ECC_CODE2   0x24  // 24 bits ECC code for the second 1024 bytes
#define CSKY_NFC_WPR         0x28  // write protect
#define CSKY_NFC_TIMOUT      0x2C  // the max time for operation

// bits of reg NFC_EN
#define CSKY_NFC_EN_ECC          (1 << 2)
#define CSKY_NFC_EN_INT          (1 << 1)
#define CSKY_NFC_EN_EN           (1 << 0)

// bits of reg IMASK
#define CSKY_NFC_IMASK_TO        (1 << 2)
#define CSKY_NFC_IMASK_ECC       (1 << 1)
#define CSKY_NFC_IMASK_CF        (1 << 0)

// bits of reg DEVICE_CFG
#define CSKY_NFC_DEVICE_CFG_CC   (1 << 19)
#define CSKY_NFC_DEVICE_CFG_CALC (0x3 << 12)
#define CSKY_NFC_DEVICE_CFG_RALC (0x3 << 8)
#define CSKY_NFC_DEVICE_CFG_WTH  (1 << 7)
#define CSKY_NFC_DEVICE_CFG_SIZE (1 << 4)
#define CSKY_NFC_DEVICE_CFG_CAP  (0xF << 0)

// bits of reg CMDR
#define CSKY_NFC_CMDR_BUF        (1 << 6)
#define CSKY_NFC_CMDR_RPAGE      0x0
#define CSKY_NFC_CMDR_EBLOCK     0x6
#define CSKY_NFC_CMDR_RSTATUS    0x7
#define CSKY_NFC_CMDR_WPAGE      0x8
#define CSKY_NFC_CMDR_RID        0x9
#define CSKY_NFC_CMDR_RPPAGE     0xE
#define CSKY_NFC_CMDR_RESET      0xF

// bits of reg SR 
#define CSKY_NFC_SR_TO           (1 << 10)
#define CSKY_NFC_SR_ECCMASK      (0xff << 2)
#define CSKY_NFC_SR_NOECCERR0    (0 << 2)
#define CSKY_NFC_SR_ONEECCERR0   (1 << 2)
#define CSKY_NFC_SR_MULECCERR0   (2 << 2)
#define CSKY_NFC_SR_NOECCERR1    (0 << 4)
#define CSKY_NFC_SR_ONEECCERR1   (1 << 4)
#define CSKY_NFC_SR_MULECCERR1   (2 << 4)
#define CSKY_NFC_SR_NOECCERR2    (0 << 6)
#define CSKY_NFC_SR_ONEECCERR2   (1 << 6)
#define CSKY_NFC_SR_MULECCERR2   (2 << 6)
#define CSKY_NFC_SR_NOECCERR3    (0 << 8)
#define CSKY_NFC_SR_ONEECCERR3   (1 << 8)
#define CSKY_NFC_SR_MULECCERR3   (2 << 8)
#define CSKY_NFC_SR_RS           (1 << 1)
#define CSKY_NFC_SR_BUSY         (1 << 0)

// in reg ECC_CODE1
#define	CSKY_ECC_0_BITADDR	     (0x7 << 0)		 /* First Bit Error Address */
#define	CSKY_ECC_0_CHARADDR	     (0x1ff << 3)	 /* First Error Address */
#define CSKY_ECC_1_BITADDR       (0x7 << 12)     /* SECOND Bit Error Address */
#define CSKY_ECC_1_CHARADDR      (0x1ff << 15)   /* SECOND Error Address */
// in reg ECC_CODE2
#define CSKY_ECC_2_BITADDR       (0x7 << 0)      /* Thrid Bit Error Address */
#define CSKY_ECC_2_CHARADDR      (0x1ff << 3)    /* Thrid Error Address */
#define CSKY_ECC_3_BITADDR       (0x7 << 12)     /* Fourth Bit Error Address */
#define CSKY_ECC_3_CHARADDR      (0x1ff << 15)   /* Fourth Error Address */

#endif /* __ASM_ARM_REGS_NAND */
