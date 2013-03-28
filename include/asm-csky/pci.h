/*
 * arch/csky/include/asm/pci.h
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of
 * this archive for more details.
 *
 * (C) Copyright 2009, C-SKY Microsystems Co., Ltd. (www.c-sky.com)
 *  
 */

#ifndef _ASM_CSKY_PCI_H
#define _ASM_CSKY_PCI_H

#include <asm-generic/pci-dma-compat.h>

/* The PCI address space does equal the physical memory
 * address space.  The networking and block device layers use
 * this boolean for bounce buffer decisions.
 */
#define PCI_DMA_BUS_IS_PHYS	(1)

/*
 *  These are pretty much arbitary with the CoMEM implementation.
 *  We have the whole address space to ourselves.
 */
#define PCIBIOS_MIN_IO          0x100
#define PCIBIOS_MIN_MEM         0x00010000

#endif /* _ASM_CSKY_PCI_H */
