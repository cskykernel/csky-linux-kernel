/*
 *  arch/csky/include/asm/fiq.h
 *
 * Support for FIQ on CK-CPU architectures.
 * Written by Hu Junsahn <junshan_hu@c-sky.com>, 2010
 */

#ifndef __CSKY_FIQ_H
#define __CSKY_FIQ_H

#include <mach/irqs.h>

struct fiq_handler {
	int (*handler)(int);
	unsigned int fiqno;
	const char *name;
	void *dev_id;
};

struct fiq_controller {
	const char *name;
	void (*startup)(unsigned int fiq);
	void (*shutdown)(unsigned int fiq);
	void (*enable)(unsigned int fiq);
	void (*disable)(unsigned int fiq);
};

#define local_fiq_enable()                     \
        __asm__ __volatile__ ("psrset fe")

#define local_fiq_disable()                     \
        __asm__ __volatile__ ("psrclr fe")

#ifdef CONFIG_FIQ_STACK_SIZE
#define FIQ_STACK_SIZE  (CONFIG_FIQ_STACK_SIZE / 4)
#else
#define FIQ_STACK_SIZE  1024 
#endif

extern int set_fiq_controller(unsigned int fiq, struct fiq_controller *chip); 
extern void enable_fiq(unsigned int fiq);
extern void disable_fiq(unsigned int fiq);
extern int claim_fiq(unsigned int fiq, int (*handler)(int), 
           const char *devname, void *dev_id);
extern void release_fiq(unsigned int fiq, void *dev_id);

#endif
