/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2009  Hangzhou C-SKY Microsystems co.,ltd.
 */

#ifndef _CSKY_CACHECTL_H
#define _CSKY_CACHECTL_H

/* Definitions for the cacheflush system call.  */

#define FLUSH_SCOPE_LINE    1	/* Flush a cache line */
#define FLUSH_SCOPE_PAGE    2	/* Flush a page */
#define FLUSH_SCOPE_ALL     3	/* Flush the whole cache -- superuser only */

#define FLUSH_CACHE_DATA    1	/* Writeback and flush data cache */
#define FLUSH_CACHE_INSN    2	/* Flush instruction cache */
#define FLUSH_CACHE_BOTH    3	/* Flush both caches */

#endif /* _CSKY_CACHECTL_H */
