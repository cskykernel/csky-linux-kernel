/*
 *  include/asm/checksum_mm.h
 *
 *  Copyright (C) 2009  Hangzhou C-SKY Microsystems
 */

#ifndef _CSKY_CHECKSUM_H
#define _CSKY_CHECKSUM_H

#include <linux/in6.h>
/*
 * computes the checksum of a memory block at buff, length len,
 * and adds in "sum" (32-bit)
 *
 * returns a 32-bit number suitable for feeding into itself
 * or csum_tcpudp_magic
 *
 * this function must be called with even lengths, except
 * for the last fragment, which may be odd
 *
 * it's best to have buff aligned on a 32-bit boundary
 */
 __wsum csum_partial(const void * buff, int len, __wsum sum);

/*
 * the same as csum_partial, but copies from src while it
 * checksums
 *
 * here even more important to align src and dst on a 32-bit (or even
 * better 64-bit) boundary
 */

__wsum csum_partial_copy(const void *src, void *dst, int len, __wsum sum);

/*
 * the same as csum_partial_copy, but copies from user space.
 *
 * here even more important to align src and dst on a 32-bit (or even
 * better 64-bit) boundary
 */

extern __wsum csum_partial_copy_from_user(const void __user *src, void *dst,
						int len, __wsum sum, int *csum_err);

#define csum_partial_copy_nocheck(src, dst, len, sum)	\
	csum_partial_copy((src), (dst), (len), (sum))

extern __sum16 ip_fast_csum(const void *iph, unsigned int ihl);

/*
 *	Fold a partial checksum
 */

static inline __sum16 csum_fold(__wsum csum)
{
	u32 tmp;
	__asm__ __volatile__("mov     %1, %0 \n\t"
	                     "rori    %0, 16 \n\t"
                         "addu    %0, %1 \n\t"
	                     "lsri    %0, 16 \n\t"
                         :"=r"(csum), "=r"(tmp)
                         :"0"(csum));
	return (__force __sum16)~csum;
}

/*
 * computes the checksum of the TCP/UDP pseudo-header
 * returns a 16-bit checksum, already complemented
 */

static inline __wsum
csum_tcpudp_nofold(__be32 saddr, __be32 daddr, unsigned short len,
		unsigned short proto, __wsum sum)
{
	__asm__ __volatile__("clrc  \n\t"
	                     "addc    %0, %1 \n\t"
	                     "addc    %0, %2 \n\t"
	                     "addc    %0, %3 \n\t"
	                     "inct    %0 \n\t"
	                     :"=r"(sum)
	                     :"r"((__force u32)saddr),
	                      "r"((__force u32)daddr),
#ifdef __cskyBE__
	                      "r"(proto + len),
#else
	                      "r"((proto + len) << 8),
#endif
	                      "0" ((__force unsigned long)sum)
	                     :"cc");	
	return sum;
}

static inline __sum16
csum_tcpudp_magic(__be32 saddr, __be32 daddr, unsigned short len,
		  unsigned short proto, __wsum sum)
{
	return csum_fold(csum_tcpudp_nofold(saddr, daddr, len, proto, sum));
}

/*
 * this routine is used for miscellaneous IP-like checksums, mainly
 * in icmp.c
 */

extern __sum16 ip_compute_csum(const void * buff, int len);

#define _HAVE_ARCH_IPV6_CSUM
static __inline__ __sum16
csum_ipv6_magic(const struct in6_addr *saddr, const struct in6_addr *daddr,
		__u32 len, unsigned short proto, __wsum sum) 
{
        sum += saddr->in6_u.u6_addr32[0];
        sum += saddr->in6_u.u6_addr32[1];
        sum += saddr->in6_u.u6_addr32[2];
        sum += saddr->in6_u.u6_addr32[3];
        sum += daddr->in6_u.u6_addr32[0];
        sum += daddr->in6_u.u6_addr32[1];
        sum += daddr->in6_u.u6_addr32[2];
        sum += daddr->in6_u.u6_addr32[3];
        sum += (len + proto);
	return csum_fold(sum);
}

#endif /* _CSKY_CHECKSUM_H */
