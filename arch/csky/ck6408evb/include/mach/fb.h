/* arch/csky/ck6408evb/include/mach/fb.h
 *
 * Copyright (c) 2010 CSKY Ltd.
 *
 * Inspired by pxafb.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_CSKY_FB_H
#define __ASM_CSKY_FB_H

#include <mach/cklcd.h>

struct csky_fb_hw {
    unsigned long   control;
    unsigned long   timing0;
    unsigned long   timing1;
    unsigned long   timing2;
};

/* LCD description */
struct csky_fb_display {
    /* LCD type */
    unsigned type;

    /* Screen size */
    unsigned short width; //What's difference with xres???
    unsigned short height;

    /* Screen info */
    unsigned short xres;
    unsigned short yres;
    unsigned short bpp;

    unsigned pixclock;      /* pixclock in picoseconds */
    unsigned short left_margin;  /* value in pixels (TFT) or HCLKs (STN) */
    unsigned short right_margin; /* value in pixels (TFT) or HCLKs (STN) */
    unsigned short hsync_len;    /* value in pixels (TFT) or HCLKs (STN) */
    unsigned short upper_margin;    /* value in lines (TFT) or 0 (STN) */
    unsigned short lower_margin;    /* value in lines (TFT) or 0 (STN) */
    unsigned short vsync_len;   /* value in lines (TFT) or 0 (STN) */

    /* lcd configuration registers */
    unsigned long   timing2;
};

struct csky_fb_mach_info {

    struct csky_fb_display *displays; /* attached diplays info */
    unsigned num_displays;          /* number of defined displays */
    unsigned default_display;
};



extern void __init csky_fb_set_platdata(struct csky_fb_mach_info *pd);
extern void InitLCDReg(void);
extern struct csky_fb_hw ck6408evb_fb_regs;
extern struct csky_fb_mach_info ck6408evb_fb_info;

#endif /* __ASM_CSKY_FB_H */

