/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  (C) Copyright 2004, Li Chunqiang (chunqiang_li@c-sky.com)
 *  (C) Copyright 2009, Hu Junshan (junshan_hu@c-sky.com)
 *  (C) Copyright 2009, C-SKY Microsystems Co., Ltd. (www.c-sky.com)
 *  
 */ 
 
#ifndef _ASM_CSKY_SIGCONTEXT_H
#define _ASM_CSKY_SIGCONTEXT_H

struct sigcontext {
	unsigned long  sc_mask; 	/* old sigmask */
	unsigned long  sc_usp;		/* old user stack pointer */
	unsigned long  sc_r1;
	unsigned long  sc_r2;
	unsigned long  sc_r3;
	unsigned long  sc_r4;
	unsigned long  sc_r5;
	unsigned long  sc_r6;
	unsigned long  sc_r7;
	unsigned long  sc_r8;
	unsigned long  sc_r9;
	unsigned long  sc_r10;
	unsigned long  sc_r11;
	unsigned long  sc_r12;
	unsigned long  sc_r13;
	unsigned long  sc_r14;
	unsigned long  sc_r15;
#if defined(CONFIG_CPU_CSKYV2)
	unsigned long  sc_r16;
	unsigned long  sc_r17;
	unsigned long  sc_r18;
	unsigned long  sc_r19;
	unsigned long  sc_r20;
	unsigned long  sc_r21;
	unsigned long  sc_r22;
	unsigned long  sc_r23;
	unsigned long  sc_r24;
	unsigned long  sc_r25;
	unsigned long  sc_r26;
	unsigned long  sc_r27;
	unsigned long  sc_r28;
	unsigned long  sc_r29;
	unsigned long  sc_r30;
	unsigned long  sc_r31;
	unsigned long  sc_rhi;
	unsigned long  sc_rlo;
#endif
	unsigned long  sc_sr;
	unsigned long  sc_pc;
	unsigned long  sc_fsr;
	unsigned long  sc_fesr;
	unsigned long  sc_feinst1;
	unsigned long  sc_feinst2;
	unsigned long  sc_fpregs[32];

};

#endif /* _ASM_CSKY_SIGCONTEXT_H */
