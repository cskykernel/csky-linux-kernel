/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009  Hangzhou C-SKY Microsystems co.,ltd.
 */

#ifndef __ARCH_CSKY_ATOMIC__
#define __ARCH_CSKY_ATOMIC__

#include <linux/compiler.h>
#include <linux/irqflags.h>
#include <linux/types.h>
/*
 * Atomic operations that C can't guarantee us.  Useful for
 * resource counting etc..
 */

typedef struct { volatile int counter; } atomic_t;

#define ATOMIC_INIT(i)	{ (i) }

#define atomic_read(v)		((v)->counter)
#define atomic_set(v, i)	(((v)->counter) = i)
#define atomic_xchg(v, new) (xchg(&((v)->counter), (new)))

static inline void atomic_add(int i, atomic_t *v)
{
        unsigned long flags;

 	raw_local_irq_save(flags);       
        v->counter += i;
        raw_local_irq_restore(flags);
}

static __inline__ void atomic_sub(int i, atomic_t *v) 
{
        unsigned long flags;

        raw_local_irq_save(flags);
        v->counter -= i;
        raw_local_irq_restore(flags);    
}



static __inline__ void atomic_inc(atomic_t *v)
{
        atomic_add(1, v);
}
static inline int atomic_inc_and_test(atomic_t *v)
{
	unsigned long flags, result;

        raw_local_irq_save(flags);
	v->counter += 1;
        result = (v->counter == 0);
        raw_local_irq_restore(flags);
        return (result);
}
/****  attantion maybe bugs****/
static inline int atomic_add_negative(int i, atomic_t *v)
{
	unsigned long flags, result;

	raw_local_irq_save(flags);
        v->counter += i;
        result = (v->counter < 0);
        raw_local_irq_restore(flags);
        return (result);

}

/*
#define atomic_add_negative(i,v) (atomic_add_return(i, v) < 0)
*/

static inline int atomic_cmpxchg(atomic_t *v, int old, int new)
{
        unsigned long flags;
        int prev;

        raw_local_irq_save(flags);
        prev = atomic_read(v);
        if (prev == old)
                atomic_set(v, new);
        raw_local_irq_restore(flags);
        return prev;
}
static __inline__ int atomic_add_unless(atomic_t *v, int a, int u)
{
        int c, old;
        c = atomic_read(v);
        for (;;) {
                if (unlikely(c == (u)))
                        break;
                old = atomic_cmpxchg((v), c, c + (a));
                if (likely(old == c))
                        break;
                c = old;
        }
        return c != (u);
}

static __inline__ void atomic_dec(atomic_t *v)
{
        atomic_sub(1, v);
}

static __inline__ int atomic_dec_and_test(atomic_t *v)
{
	unsigned long flags, result;
	
        raw_local_irq_save(flags);
	v->counter -= 1;
        result = (v->counter == 0);
        raw_local_irq_restore(flags);
        
        return (result);
}

static __inline__ void  atomic_clear_mask(int mask, atomic_t *v) 
{
        unsigned long flags;

        raw_local_irq_save(flags);
        v->counter &= ~mask;
        raw_local_irq_restore(flags);
}


static __inline__ void atomic_set_mask(int mask, atomic_t *v) 
{
        unsigned long flags;

        raw_local_irq_save(flags);
        v->counter |= mask;
        raw_local_irq_restore(flags);
}

#define atomic_inc_not_zero(v) atomic_add_unless((v), 1, 0)

/* Atomic operations are already serializing */
#define smp_mb__before_atomic_dec()    barrier()
#define smp_mb__after_atomic_dec() barrier()
#define smp_mb__before_atomic_inc()    barrier()
#define smp_mb__after_atomic_inc() barrier()

extern __inline__ int atomic_add_return(int i, atomic_t * v)
{
	unsigned long temp, flags;

	raw_local_irq_save(flags); 
	temp = v->counter;
	temp += i;
	v->counter = temp;
	raw_local_irq_restore(flags);

	return temp;
}

extern __inline__ int atomic_sub_return(int i, atomic_t * v)
{
	unsigned long temp, flags;

	raw_local_irq_save(flags);
	temp = v->counter;
	temp -= i;
	v->counter = temp;
	raw_local_irq_restore(flags);

	return temp;
}

#define atomic_dec_return(v) atomic_sub_return(1,(v))
#define atomic_inc_return(v) atomic_add_return(1,(v))

#define atomic_sub_and_test(i,v) (atomic_sub_return((i), (v)) == 0)
#define atomic_dec_and_test(v) (atomic_sub_return(1, (v)) == 0)

#include <asm-generic/atomic.h>
#endif /* __ARCH_CSKY_ATOMIC __ */
