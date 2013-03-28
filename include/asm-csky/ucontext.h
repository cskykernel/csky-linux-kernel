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
  
#ifndef _CSKY_UCONTEXT_H
#define _CSKY_UCONTEXT_H

struct ucontext {
	unsigned long     uc_flags;
	struct ucontext   *uc_link;
	stack_t	          uc_stack;
	struct sigcontext uc_mcontext;
	sigset_t          uc_sigmask;	/* mask last for extensibility */
};

#endif
