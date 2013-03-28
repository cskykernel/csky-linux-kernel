/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  (C) Copyright 2009, C-SKY Microsystems Co., Ltd. (www.c-sky.com)
 *  
 */ 
  
#ifndef __ARCH_CSKY_SOCKIOS_H_
#define __ARCH_CSKY_SOCKIOS_H_

/* Socket-level I/O control calls. */
#define FIOSETOWN 		0x8901
#define SIOCSPGRP		0x8902
#define FIOGETOWN		0x8903
#define SIOCGPGRP		0x8904
#define SIOCATMARK		0x8905
#define SIOCGSTAMP		0x8906		/* Get stamp */
#define SIOCGSTAMPNS	0x8907		/* Get stamp (timespec) */

#endif /* __ARCH_CSKY_SOCKIOS_H_ */
