/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009  Hangzhou C-SKY Microsystems co.,ltd.
 */

#ifndef _CSKY_BYTEORDER_H
#define _CSKY_BYTEORDER_H

#include <linux/compiler.h>
#include <asm/types.h>

static inline __attribute_const__ __u32 ___arch__swab32(__u32 x)
{

	__u32 t;

	t = x ^ ((x << 16) | (x >> 16)); /* eor r1,r0,r0,ror #16 */
	x = (x << 24) | (x >> 8);               /* mov r0,r0,ror #8      */
	t &= ~0x00FF0000;                       /* bic r1,r1,#0x00FF0000 */
	x ^= (t >> 8);                          /* eor r0,r0,r1,lsr #8   */

	return x;
}

#define __arch__swab32(x) ___arch__swab32(x)

#if !defined(__STRICT_ANSI__) || defined(__KERNEL__)
#  define __BYTEORDER_HAS_U64__
#  define __SWAB_64_THRU_32__
#endif

#if defined(__cskyBE__)
#include <linux/byteorder/big_endian.h>
#elif defined(__cskyLE__)
#include <linux/byteorder/little_endian.h>
#else
# error "csky, but neither __cskyBE__, nor __cskyLE__???"
#endif

#endif /* _CSKY_BYTEORDER_H */
