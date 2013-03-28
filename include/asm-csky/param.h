/*
 * arch/csky/include/asm/param.h
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of
 * this archive for more details.
 *
 * (C) Copyright 2009, C-SKY Microsystems Co., Ltd. (www.c-sky.com)
 *  
 */

#ifndef _CSKY_PARAM_H
#define _CSKY_PARAM_H

#ifdef __KERNEL__
# define HZ             CONFIG_HZ   /* Internal kernel timer frequency */
# define USER_HZ        100         /* .. some user interfaces are in "ticks" */
# define CLOCKS_PER_SEC (USER_HZ)   /* like times() */
#endif

#ifndef HZ
#if defined(CONFIG_CK6408EVB)
#define	HZ 100
#endif
#endif


#define EXEC_PAGESIZE	4096

#ifndef NGROUPS
#define NGROUPS		32
#endif

#ifndef NOGROUP
#define NOGROUP		(-1)
#endif

#define MAXHOSTNAMELEN	64	/* max length of hostname */


#endif /* _CSKY_PARAM_H */
