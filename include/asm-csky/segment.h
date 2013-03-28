/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  (C) Copyright 2004, Li Chunqiang (chunqiang_li@c-sky.com)
 *  (C) Copyright 2009, Hu Junshan (junshan_hu@c-sky.com)
 *
 */

#ifndef _CSKY_SEGMENT_H
#define _CSKY_SEGMENT_H

#ifndef __ASSEMBLY__

typedef struct {
	unsigned long seg;
} mm_segment_t;

#define KERNEL_DS		((mm_segment_t) { 0xFFFFFFFF })

#ifdef CONFIG_MMU
#define USER_DS			((mm_segment_t) { 0x80000000UL })
#define get_fs()		(current_thread_info()->addr_limit)
#define set_fs(x)		(current_thread_info()->addr_limit = (x))
#define segment_eq(a,b)	((a).seg == (b).seg)

#else /*CONFIG_MMU*/
#define USER_DS   KERNEL_DS
#define get_fs()		(KERNEL_DS)
static inline void set_fs(mm_segment_t fs)
{
}
#define segment_eq(a,b)	(1)
#endif /*CONFIG_MMU*/

#define get_ds()		(KERNEL_DS)

#endif /* __ASSEMBLY__ */

#endif /* _CSKY_SEGMENT_H */
