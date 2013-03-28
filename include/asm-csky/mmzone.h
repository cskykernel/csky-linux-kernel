/*
 * arch/csky/include/asm/mmzone.h
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License.  See the file COPYING in the main directory of
 * this archive for more details.
 *
 * (C) Copyright 2009, C-SKY Microsystems Co., Ltd. (www.c-sky.com)
 *  
 */

#ifndef _ASM_CSKY_MMZONE_H_
#define _ASM_CSKY_MMZONE_H_
#include <asm/page.h>
#include <mmzone.h>
extern pg_data_t pg_data_map[];

#define NODE_DATA(nid)		(&pg_data_map[nid])
#define NODE_MEM_MAP(nid)	(NODE_DATA(nid)->node_mem_map)

#endif /* _ASM_CSKY_MMZONE_H_ */
