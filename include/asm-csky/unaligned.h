/*
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2010 Hu junshan (junshan_hu@c-sky.com)
 */

#ifndef _ASM_CSKY_UNALIGNED_H
#define _ASM_CSKY_UNALIGNED_H

#include <linux/compiler.h>
#if defined(__cskyBE__)
# include <linux/unaligned/be_struct.h>
# include <linux/unaligned/le_byteshift.h>
# include <linux/unaligned/generic.h>
# define get_unaligned  __get_unaligned_be
# define put_unaligned  __put_unaligned_be
#elif defined(__cskyLE__)
# include <linux/unaligned/le_struct.h>
# include <linux/unaligned/be_byteshift.h>
# include <linux/unaligned/generic.h>
# define get_unaligned  __get_unaligned_le
# define put_unaligned  __put_unaligned_le
#else
#  error "csky, but neither __cskyBE__, nor __cskyLE__???"
#endif

#endif /* _ASM_CSKY_UNALIGNED_H */
