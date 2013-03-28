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
 
#ifndef _CSKY_SHM_H
#define _CSKY_SHM_H

/* format of page table entries that correspond to shared memory pages
   currently out in swap space (see also mm/swap.c):
   bits 0-1 (PAGE_PRESENT) is  = 0
   bits 8..2 (SWP_TYPE) are = SHM_SWP_TYPE
   bits 31..9 are used like this:
   bits 15..9 (SHM_ID) the id of the shared memory segment
   bits 30..16 (SHM_IDX) the index of the page within the shared memory segment
                    (actually only bits 25..16 get used since SHMMAX is so low)
   bit 31 (SHM_READ_ONLY) flag whether the page belongs to a read-only attach
*/

#define SHM_ID_SHIFT	9
#define _SHM_ID_BITS	7
#define SHM_ID_MASK		((1<<_SHM_ID_BITS)-1)

#define SHM_IDX_SHIFT	(SHM_ID_SHIFT+_SHM_ID_BITS)
#define _SHM_IDX_BITS	15
#define SHM_IDX_MASK	((1<<_SHM_IDX_BITS)-1)

#endif /* _CSKY_SHM_H */
