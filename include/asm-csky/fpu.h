/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009  Hangzhou C-SKY Microsystems co.,ltd.
 */

#ifndef __CSKY_FPU_H
#define __CSKY_FPU_H

/*
 * Define the fesr bit for fpe handle. 
 */
#define  FPE_ILLE  (1 << 16)    /* Illegal instruction  */
#define  FPE_FEC   (1 << 7)     /* Input float-point arithmetic exception */
#define  FPE_IDC   (1 << 5)     /* Input denormalized exception */
#define  FPE_IXC   (1 << 4)     /* Inexact exception */
#define  FPE_UFC   (1 << 3)     /* Underflow exception */
#define  FPE_OFC   (1 << 2)     /* Overflow exception */
#define  FPE_DZC   (1 << 1)     /* Divide by zero exception */
#define  FPE_IOC   (1 << 0)     /* Invalid operation exception */

#ifdef CONFIG_OPEN_FPU_IDE
#define IDE_STAT   (1 << 5)
#else 
#define IDE_STAT   0
#endif

#ifdef CONFIG_OPEN_FPU_IXE
#define IXE_STAT   (1 << 4)
#else 
#define IXE_STAT   0
#endif

#ifdef CONFIG_OPEN_FPU_UFE
#define UFE_STAT   (1 << 3)
#else
#define UFE_STAT   0
#endif

#ifdef CONFIG_OPEN_FPU_OFE
#define OFE_STAT   (1 << 2)
#else
#define OFE_STAT   0
#endif

#ifdef CONFIG_OPEN_FPU_DZE
#define DZE_STAT   (1 << 1)
#else
#define DZE_STAT   0
#endif

#ifdef CONFIG_OPEN_FPU_IOE
#define IOE_STAT   (1 << 0)
#else
#define IOE_STAT   0
#endif

/* enable and init FPU */
static inline void init_fpu(void)
{
	unsigned long flg;
	unsigned long cpwr, fcr;

	cpwr = 0xf0000007; // set for reg CPWR(cp15): ie, ic, ec, rp, wp, en = 1 
	fcr = (IDE_STAT | IXE_STAT | UFE_STAT | OFE_STAT | DZE_STAT | IOE_STAT);
	local_save_flags(flg);	

	__asm__ __volatile__("cpseti  1 \n\t"
	                     "mtcr    %0, cr15 \n\t"
	                     "cpwcr   %1, cpcr1 \n\t"
	                     ::"r"(cpwr), "b"(fcr)
	                     );
	local_irq_restore(flg);
}
 
static inline void save_fp_to_thread(unsigned long  * fpregs, 
	                  unsigned long * fsr, unsigned long * fesr)
{
	unsigned long flg;
	unsigned long tmp1, tmp2, tmp3, tmp4;
	unsigned long fctl1, fctl2;
	
	local_save_flags(flg);	
    
	__asm__ __volatile__("cpseti 1 \n\t"
	                     "cprcr  %1, cpcr2 \n\t"
	                     "cprcr  %2, cpcr4 \n\t"
	                     "fmfs   %3, fr0 \n\t"
	                     "fmfs   %4, fr1 \n\t"
	                     "fmfs   %5, fr2 \n\t"
	                     "fmfs   %6, fr3 \n\t"
	                     "stw    %3, (%0, 0) \n\t"
	                     "stw    %4, (%0, 4) \n\t"
	                     "stw    %5, (%0, 8) \n\t"
	                     "stw    %6, (%0, 12) \n\t"
	                     "fmfs   %3, fr4 \n\t"
	                     "fmfs   %4, fr5 \n\t"
	                     "fmfs   %5, fr6 \n\t"
	                     "fmfs   %6, fr7 \n\t"
	                     "stw    %3, (%0, 16) \n\t"
	                     "stw    %4, (%0, 20) \n\t"
	                     "stw    %5, (%0, 24) \n\t"
	                     "stw    %6, (%0, 28) \n\t"
	                     "fmfs   %3, fr8 \n\t"
	                     "fmfs   %4, fr9 \n\t"
	                     "fmfs   %5, fr10 \n\t"
	                     "fmfs   %6, fr11 \n\t" 
	                     "stw    %3, (%0, 32) \n\t"
	                     "stw    %4, (%0, 36) \n\t"
	                     "stw    %5, (%0, 40) \n\t"
	                     "stw    %6, (%0, 44) \n\t"
	                     "fmfs   %3, fr12 \n\t" 
	                     "fmfs   %4, fr13 \n\t" 
	                     "fmfs   %5, fr14 \n\t"
	                     "fmfs   %6, fr15 \n\t"
	                     "stw    %3, (%0, 48) \n\t"
	                     "stw    %4, (%0, 52) \n\t"
	                     "stw    %5, (%0, 56) \n\t"
	                     "stw    %6, (%0, 60) \n\t"
	                     "addi   %0,32 \n\t"
	                     "addi   %0,32 \n\t"
	                     "fmfs   %3, fr16 \n\t"
	                     "fmfs   %4, fr17 \n\t"
	                     "fmfs   %5, fr18 \n\t"
	                     "fmfs   %6, fr19 \n\t"
	                     "stw    %3, (%0, 0) \n\t"
	                     "stw    %4, (%0, 4) \n\t"
	                     "stw    %5, (%0, 8) \n\t"
	                     "stw    %6, (%0, 12) \n\t"
	                     "fmfs   %3, fr20 \n\t"
	                     "fmfs   %4, fr21 \n\t"
	                     "fmfs   %5, fr22 \n\t"
	                     "fmfs   %6, fr23 \n\t"
	                     "stw    %3, (%0, 16) \n\t"
	                     "stw    %4, (%0, 20) \n\t"
	                     "stw    %5, (%0, 24) \n\t"
	                     "stw    %6, (%0, 28) \n\t"
	                     "fmfs   %3, fr24 \n\t"
	                     "fmfs   %4, fr25 \n\t"
	                     "fmfs   %5, fr26 \n\t"
	                     "fmfs   %6, fr27 \n\t"
	                     "stw    %3, (%0, 32) \n\t"
	                     "stw    %4, (%0, 36) \n\t"
	                     "stw    %5, (%0, 40) \n\t"
	                     "stw    %6, (%0, 44) \n\t"
	                     "fmfs   %3, fr28 \n\t"
	                     "fmfs   %4, fr29 \n\t"
	                     "fmfs   %5, fr30 \n\t"
	                     "fmfs   %6, fr31 \n\t"
	                     "stw    %3, (%0, 48) \n\t"
	                     "stw    %4, (%0, 52) \n\t"
	                     "stw    %5, (%0, 56) \n\t"
	                     "stw    %6, (%0, 60) \n\t"
	                     :"+a"(fpregs), "=a"(fctl1), "=a"(fctl2), "=a"(tmp1), 
	                       "=a"(tmp2),"=a"(tmp3),"=a"(tmp4));
	*fsr = fctl1;
	*fesr = fctl2;
	
	local_irq_restore(flg);
}

#endif /* __CSKY_FPU_H */
