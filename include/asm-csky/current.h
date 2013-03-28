/*
 * arch/csky/include/asm/current.h
 *
 * Copyright (C) 2009  Hangzhou C-SKY Microsystems
 * 
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 */


#ifndef _CSKY_CURRENT_H
#define _CSKY_CURRENT_H

#include <linux/thread_info.h>

static inline struct task_struct *get_current(void)
{
    return current_thread_info()->task;
}

#define current (get_current())

#endif /* _CSKY_CURRENT_H */
