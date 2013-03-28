/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright 1992, Linus Torvalds.
 * Copyright (C) 2009 Hangzhou C-SKY Microsystems co.,ltd. 
 */

#ifndef _CSKY_BITOPS_H
#define _CSKY_BITOPS_H

#ifndef _LINUX_BITOPS_H
#error only <linux/bitops.h> can be included directly
#endif

#include <linux/compiler.h>
#include <linux/irqflags.h>
#include <linux/types.h>
#include <asm/bug.h>
#include <asm/byteorder.h>             
#include <asm/system.h>

extern int printk(const char *fmt, ...);
static inline int test_and_set_bit(int nr, volatile void * addr)
{
        int     mask, retval;
        volatile unsigned int *a = (volatile unsigned int *) addr;
        unsigned long flags;

        a += nr >> 5;
        mask = 1 << (nr & 0x1f);
        raw_local_irq_save(flags);
        retval = (mask & *a) != 0;
        *a |= mask;
        raw_local_irq_restore(flags);

        return retval;
}
static inline int test_and_set_bit_lock(unsigned long nr,
        volatile unsigned long *addr)
{
	return test_and_set_bit(nr,addr);
}
static inline int __test_and_set_bit(int nr, volatile void * addr)
{
        int     mask, retval;
        volatile unsigned int *a = (volatile unsigned int *) addr;

        a += nr >> 5;
        mask = 1 << (nr & 0x1f);
        retval = (mask & *a) != 0;
        *a |= mask;
        return retval;
}

static inline void set_bit(int nr, volatile void * addr)
{
        volatile unsigned int *a = (volatile unsigned int *) addr;
        int     mask;
        unsigned long flags;

        a += nr >> 5;
        mask = 1 << (nr & 0x1f);
        raw_local_irq_save(flags);
        *a |= mask;
        raw_local_irq_restore(flags);
}
static inline  void __set_bit(int nr, volatile void * addr)
{
        volatile unsigned int *a = (volatile unsigned int *) addr;
        int     mask;

        a += nr >> 5;
        mask = 1 << (nr & 0x1f);
        *a |= mask;
}

static inline int test_and_clear_bit(int nr, volatile void * addr)
{
        int     mask, retval;
        volatile unsigned int *a = (volatile unsigned int *) addr;
        unsigned long flags;

        a += nr >> 5;
        mask = 1 << (nr & 0x1f);
        raw_local_irq_save(flags);
        retval = (mask & *a) != 0;
        *a &= ~mask;
        raw_local_irq_restore(flags);

        return retval;
}

static inline int __test_and_clear_bit(int nr, volatile void * addr)
{
        int     mask, retval;
        volatile unsigned int *a = (volatile unsigned int *) addr;

        a += nr >> 5; 
        mask = 1 << (nr & 0x1f);
        retval = (mask & *a) != 0;
        *a &= ~mask;
        return retval;
}

/*
 * clear_bit() doesn't provide any barrier for the compiler.
 */
#define smp_mb__before_clear_bit()      barrier()
#define smp_mb__after_clear_bit()       barrier()


static inline void clear_bit(int nr, volatile void * addr)
{       
        volatile unsigned int *a = (volatile unsigned int *) addr;
        int     mask;
        unsigned long flags;

        a += nr >> 5;
        mask = 1 << (nr & 0x1f);
        raw_local_irq_save(flags);
        *a &= ~mask;
        raw_local_irq_restore(flags);
}

#define __clear_bit(nr,vaddr) clear_bit(nr,vaddr)

static inline int test_and_change_bit(int nr, volatile void * addr)
{
        int     mask, retval;
        volatile unsigned int *a = (volatile unsigned int *) addr;
        unsigned long flags;

        a += nr >> 5;
        mask = 1 << (nr & 0x1f);
        raw_local_irq_save(flags);
        retval = (mask & *a) != 0;
        *a ^= mask;
        raw_local_irq_restore(flags);

        return retval;
}

static inline int __test_and_change_bit(int nr, volatile void * addr)
{
        int     mask, retval;
        volatile unsigned int *a = (volatile unsigned int *) addr;

        a += nr >> 5;
        mask = 1 << (nr & 0x1f);
        retval = (mask & *a) != 0;
        *a ^= mask;
        return retval;
}

static inline void change_bit(int nr, volatile void * addr)
{
        int mask, flags;
        volatile unsigned long *ADDR = (volatile unsigned long *) addr;

        ADDR += nr >> 5;
        mask = 1 << (nr & 31);
       	raw_local_irq_save(flags);
        *ADDR ^= mask;
        raw_local_irq_restore(flags);
}

static inline void __change_bit(int nr, volatile void * addr)
{
        int mask;
        volatile unsigned long *ADDR = (volatile unsigned long *) addr;

        ADDR += nr >> 5;
        mask = 1 << (nr & 31);
        *ADDR ^= mask;
}

static inline int __test_bit(int nr, volatile void * addr)
{
        volatile unsigned int *a = (volatile unsigned int *) addr;
        int     mask;

        a += nr >> 5;
        mask = 1 << (nr & 0x1f);
        return ((mask & *a) != 0);
}

static inline int test_bit(int nr, const volatile unsigned long *vaddr)
{
        return (vaddr[nr >> 5] & (1UL << (nr & 31))) != 0;
}

/*
 * ffs: find first bit set. This is defined the same way as
 * the libc and compiler builtin ffs routines, therefore
 * differs in spirit from the above ffz (man ffs).
 */
static inline int ffs(int x)
{
	if(!x)
	{
		return 0;
	}
        __asm__ __volatile__ (
	                      "brev %0\n\t"
                              "ff1  %0\n\t"
	                      "addi %0, 1\n\t"  
                              : "=r"(x)  
                              : "0"(x));
        return x;
}

static inline int __ffs(unsigned int x)
{
	__asm__ __volatile__ (
	                      "brev %0\n\t"
	                      "ff1  %0\n\t"
	                      : "=r"(x)
	                      : "0"(x));
	return x;	
}

/*
 * ffz = Find First Zero in word. Undefined if no zero exists,
 * so code should check against ~0UL first..
 */
static inline  unsigned long ffz(unsigned long word)
{
	unsigned long result = ~word;
	__asm__ __volatile__ (
                              "brev %0\n\t"
                              "ff1  %0\n\t"
                              : "=r"(result)
                              : "0"(result));
        return result; 
}

static inline int find_next_zero_bit (const unsigned long * addr, 
                             int size, int offset)
{
        const unsigned long *p = addr + (offset >> 5);
        int result = offset & ~31UL;
        int tmp;
        
        if (offset >= size)
                return size;
        size -= result;
        offset &= 31UL;
        if (offset) {
                tmp = *(p++);
                tmp |= ~0UL >> (32-offset);
                if (size < 32)
                        goto found_first;
                if (~tmp)
                        goto found_middle;
                size -= 32;
                result += 32;
        }
        while (size & ~31UL) {
                if (~(tmp = *(p++)))
                        goto found_middle;
                result += 32;
                size -= 32;
        }
        if (!size)
                return result;
        tmp = *p;

found_first:
	tmp |= ~0UL << size;
        if (tmp == ~0UL)        /* Are any bits zero? */
                return result + size;   /* Nope. */

found_middle:
        return result + ffz(tmp);
}

/*
 * Find the first cleared bit in a memory region.
 */
static inline int find_first_zero_bit(const unsigned long *addr, int size)
{
        const unsigned long *p = addr;
        int result = 0;
        int tmp;

        while (size & ~(32 - 1)) {
                if (~(tmp = *(p++)))
                        goto found;
                result += 32;
                size -= 32;
        }
        if (!size)
                return result;

        tmp = (*p) | (~0UL << size);
        if (tmp == ~0UL)        /* Are any bits zero? */
                return result + size;   /* Nope. */
found:
        return result + ffz(tmp);
}
/*
 * find_next_bit - find the next set bit in a memory region
 * @addr: The address to base the search on
 * @offset: The bitnumber to start searching at
 * @size: The maximum size to search
 */
static inline int find_next_bit (const unsigned long * addr, 
                               int size, int offset)
{
        const unsigned long *p = addr + (offset >> 5);
        int result = offset & ~31UL;
        int tmp;

        if (offset >= size)
                return size;
        size -= result;
        offset &= 31UL;
        if (offset) {
                tmp = *(p++);
                tmp &= (~0UL << offset);
                if (size < 32)
                        goto found_first;
                if (tmp)
                        goto found_middle;
                size -= 32;
                result += 32;
        }
        while (size & ~31UL) {
                if ((tmp = *(p++)))
                        goto found_middle;
                result += 32;
                size -= 32;
        }
        if (!size)
                return result;
        tmp = *p;

found_first:
        tmp &= (~0UL >> (32 - size));
	if (tmp == 0UL)         /* Are any bits set? */
                return result + size;   /* Nope. */	
found_middle:
        return result + __ffs(tmp);
}

/*
 * find_first_bit - find the first set bit in a memory region
 * @addr: The address to start the search at
 * @size: The maximum size to search
 *
 * Returns the bit-number of the first set bit, not the number of the byte
 * containing a bit.
 */
static inline int find_first_bit (const unsigned long * addr, int size)
{
        const unsigned long *p = addr;
        int result = 0;
        int tmp;

        while (size & ~31UL) {
                if ((tmp = *(p++)))
                        goto found;
                result += 32;
                size -= 32;
        }
        if (!size)
                return result;

        tmp = (*p) & (~0UL >> (32 - size));
        if (tmp == 0UL)         /* Are any bits set? */
                return result + size;   /* Nope. */
found:
        return result + __ffs(tmp);
}
/*
 * fls: find last bit set.
 */
static inline int fls(int x)
{
	__asm__ __volatile__(
	                     "ff1 %0\n\t"
	                     :"=r" (x)
	                     :"0" (x));
	
	return (32 - x); 
}

static inline int __fls(int x)
{
        return fls(x) - 1;
}

/*
 * __clear_bit_unlock - Clears a bit in memory
 * @nr: Bit to clear
 * @addr: Address to start counting from
 *
 * __clear_bit() is non-atomic and implies release semantics before the memory
 * operation. It can be used for an unlock if no other CPUs can concurrently
 * modify other bits in the word.
 */
static inline void __clear_bit_unlock(unsigned long nr, 
                                volatile unsigned long *addr)
{
        smp_mb();
        __clear_bit(nr, addr);
}
/*
 * clear_bit_unlock - Clears a bit in memory
 * @nr: Bit to clear
 * @addr: Address to start counting from
 *
 * clear_bit() is atomic and implies release semantics before the memory
 * operation. It can be used for an unlock.
 */
static inline void clear_bit_unlock(unsigned long nr, 
                            volatile unsigned long *addr)
{
        smp_mb__before_clear_bit();
        clear_bit(nr, addr);
}



/* Bitmap functions for the minix filesystem.  */

#define minix_find_first_zero_bit(addr,size) find_first_zero_bit(addr,size)
#define minix_test_and_set_bit(nr,addr) test_and_set_bit(nr,addr)
#define minix_set_bit(nr,addr) set_bit(nr,addr)
#define minix_test_bit(nr,addr) test_bit(nr,addr)

/* Bitmap functions for the ext2 filesystem. */
#define ext2_set_bit(nr, addr)                  __test_and_set_bit((nr) ^ 24, (unsigned long *)(addr))
#define ext2_set_bit_atomic(lock, nr, addr)     test_and_set_bit((nr) ^ 24, (unsigned long *)(addr))
#define ext2_clear_bit(nr, addr)                __test_and_clear_bit((nr) ^ 24, (unsigned long *)(addr))
#define ext2_clear_bit_atomic(lock, nr, addr)   test_and_clear_bit((nr) ^ 24, (unsigned long *)(addr))


static inline int ext2_test_bit(int nr, const volatile void * addr)
{
        int                     mask;
        const volatile unsigned char    *ADDR = (const unsigned char *) addr;

        ADDR += nr >> 3;
        mask = 1 << (nr & 0x07);
        return ((mask & *ADDR) != 0);
}

#define ext2_find_first_zero_bit(addr, size) \
        ext2_find_next_zero_bit((addr), (size), 0)
    
static inline unsigned long ext2_find_next_zero_bit(void *addr, 
                            unsigned long size, unsigned long offset)
{               
        unsigned long *p = ((unsigned long *) addr) + (offset >> 5);
        unsigned long result = offset & ~31UL;
        unsigned long tmp; 
                        
        if (offset >= size)
                return size;
        size -= result;
        offset &= 31UL;
        if(offset) {
                /* We hold the little endian value in tmp, but then the
                 * shift is illegal. So we could keep a big endian value
                 * in tmp, like this:
                 *
                 * tmp = __swab32(*(p++));
                 * tmp |= ~0UL >> (32-offset);
                 *
                 * but this would decrease preformance, so we change the
                 * shift:
                 */
                tmp = *(p++);
                tmp |= __swab32(~0UL >> (32-offset));
                if(size < 32)
                        goto found_first;
                if(~tmp)
                        goto found_middle;
                size -= 32;
                result += 32;
        }
        while(size & ~31UL) {
                if(~(tmp = *(p++)))
                        goto found_middle;
                result += 32;
                size -= 32;
        }
        if(!size)
                return result;
        tmp = *p;

found_first:
        /* tmp is little endian, so we would have to swab the shift,
         * see above. But then we have to swab tmp below for ffz, so
         * we might as well do this here.
         */
        return result + ffz(__swab32(tmp) | (~0UL << size));
found_middle:
        return result + ffz(__swab32(tmp));
}
#include <asm-generic/bitops/fls64.h>


#ifdef __KERNEL__

#include <asm-generic/bitops/sched.h>
#include <asm-generic/bitops/hweight.h>

#endif /* __KERNEL__ */


#endif
