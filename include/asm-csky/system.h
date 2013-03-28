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
  
#ifndef _CSKY_SYSTEM_H
#define _CSKY_SYSTEM_H

#include <linux/linkage.h>
#include <asm/segment.h>
#include <asm/entry.h>
#include <linux/irqflags.h>

#define prepare_to_switch()	do { } while(0)

#ifdef __KERNEL__

#ifdef CONFIG_CPU_HAS_FPU
//FIXME: maybe death cycle auch as FPU error! 	
#define _is_fpu_busying() {                     \
	unsigned long fsr, flags;                   \
                                                \
	__asm__ __volatile__("mfcr    %0, psr\n\r"  \
		                 "cpseti  1   \n\r"     \
		                 "1:		"           \
		                 "cprsr   %1\n\r"       \
		                 "btsti   %1, 31\n\r"   \
		                 "bt      1b\n\r"       \
		                 "mtcr    %0, psr\n\r"  \
		                 :"=r"(flags), "=r"(fsr)\
	                    );                      \
}
#else
#define _is_fpu_busying()  do { } while(0)
#endif

/*
 * switch_to(n) should switch tasks to task ptr, first checking that
 * ptr isn't the current task, in which case it does nothing.  This
 * also clears the TS-flag if the task we switched to has used the
 * math co-processor latest.
 *
 * syscall stores these registers itself and none of them are used
 * by syscall after the function in the syscall has been called.
 *
 * pass 'prev' in r2, 'next' in r3  and 'last' actually equal to 'prev'.
 */

asmlinkage void resume(void);
#define switch_to(prev,next,last) { 			\
  _is_fpu_busying();                            \
  register void *_prev __asm__ ("r2") = (prev); \
  register void *_next __asm__ ("r3") = (next); \
  register void *_last __asm__ ("r2") ;			\
  __asm__ __volatile__(							\
			"jbsr  resume \n\t"          		\
		       : "=r" (_last)				    \
		       : "r" (_prev),				    \
			 "r" (_next)				       	\
		        );	                        	\
  (last) = _last; 								\
}

/*
 * Force strict CPU ordering.
 * Not really required on csky...
 */
#define nop()  asm volatile ("nop"::)
#define mb()   asm volatile (""   : : :"memory")
#define rmb()  asm volatile (""   : : :"memory")
#define wmb()  asm volatile (""   : : :"memory")
#define set_mb(var, value)     do { var = value; mb(); } while (0)
#define set_wmb(var, value)    do { var = value; wmb(); } while (0)

#ifdef CONFIG_SMP
#define smp_mb()	mb()
#define smp_rmb()	rmb()
#define smp_wmb()	wmb()
#else
#define smp_mb()	barrier()
#define smp_rmb()	barrier()
#define smp_wmb()	barrier()
#endif

#define smp_read_barrier_depends()      ((void)0)

#define xchg(ptr,x) ((__typeof__(*(ptr)))__xchg((unsigned long)(x),(ptr),sizeof(*(ptr))))
#define tas(ptr) (xchg((ptr),1))

struct __xchg_dummy { unsigned long a[100]; };
#define __xg(x) ((volatile struct __xchg_dummy *)(x))

static inline unsigned long __xchg(unsigned long x, volatile void * ptr, 
									int size)
{
  unsigned long tmp, flags;

  local_irq_save(flags);  

  switch (size) {
  case 1:
    __asm__ __volatile__
    ("ldb %0, %2\n\t"
     "stb %1, %2"
     : "=&r" (tmp) : "r" (x), "m" (*__xg(ptr)) : "memory");
    break;
  case 2:
    __asm__ __volatile__
    ("ldh %0, %2\n\t"
     "sth %1, %2"
    : "=&r" (tmp) : "r" (x), "m" (*__xg(ptr)) : "memory");
    break;
  case 4:
    __asm__ __volatile__
    ("ldw %0, %2\n\t"
     "stw %1,%2"
    : "=&r" (tmp) : "r" (x), "m" (*__xg(ptr)) : "memory");
    break;
  }
  local_irq_restore(flags);
  return tmp;
}

#define HARD_RESET_NOW()

#define arch_align_stack(x) (x)

#endif /* __KERNEL__ */

#endif /* _CSKY_SYSTEM_H */
