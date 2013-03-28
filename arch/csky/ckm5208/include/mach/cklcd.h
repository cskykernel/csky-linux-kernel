/* arch/csky/ckm5208/include/mach/cklcd.h
 *
 * Copyright (C) 2011 C-SKY Microsystems Co., Ltd.
 * Author: Hu junshan <junshan_hu@c-sky.com> 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef _CSKY_CKM5208LCD_H_
#define _CSKY_CKM5208LCD_H_

#include <asm/delay.h>
#include <mach/ck_iomap.h>

#define     CSKY_LCD_CONTROL            0x000
#define     CSKY_LCD_TIMING0            0x004
#define     CSKY_LCD_TIMING1            0x008
#define     CSKY_LCD_TIMING2            0x00c
#define     CSKY_LCD_PBASE              0x010
#define     CSKY_LCD_PCURR              0x018
#define     CSKY_LCD_INT_STAT           0x020
#define     CSKY_LCD_INT_MASK           0x024

#define     CSKY_LCD_DP1_2              0x028
#define     CSKY_LCD_DP4_7              0x02c
#define     CSKY_LCD_DP3_5              0x030
#define     CSKY_LCD_DP2_3              0x034
#define     CSKY_LCD_DP5_7              0x038
#define     CSKY_LCD_DP3_4              0x03c
#define     CSKY_LCD_DP4_5              0x040
#define     CSKY_LCD_DP6_7              0x044

#define     CSKY_LCD_PALETTE_BASE       0x800	
#define     CSKY_LCD_PALETTE_ENTRIES_NUM 256

#define	    CSKY_LCD_INT_BASE		CSKY_LCD_INT_STAT

/* Bits macro define */

#define     CSKY_LCDCON_LEN             1
#define     CSKY_LCDCON_CMS_COLOR       1 << 1
#define     CSKY_LCDCON_PAS_TFT	        1 << 3
#define     CSKY_LCDCON_PBS_4BITS       0
#define     CSKY_LCDCON_PBS_8BITS       1 << 5
#define     CSKY_LCDCON_PBS_16BITS      2 << 5
#define     CSKY_LCDCON_BES_BIG         1 << 8
#define     CSKY_LCDCON_VBL_1CYCLES     0
#define     CSKY_LCDCON_VBL_4CYCLES     2 << 9
#define     CSKY_LCDCON_VBL_RESERVED    3 << 9
#define	    CSKY_LCDCON_WML_8WORD       1 << 11
#define	    CSKY_LCDCON_OUT_24BIT       1 << 12

#define     CSKY_LCDTIM0_HSW_16         15 << 10 // 15+1 pixel clock periods
#define     CSKY_LCDTIM0_PPL_320        19 << 4  // pixels-per-line = 16 * (19 + 1)

#define     CSKY_LCDTIM1_EFW_2          2 << 16
#define     CSKY_LCDTIM1_VSW_2          2 << 10
#define     CSKY_LCDTIM1_LPP_240        239      // Lines per panel = 239 + 1 

#define     CSKY_LCDTIM2_PCD_20         9        //pixel clock divisor = 2 * (9 + 1)
#define     CSKY_LCDTIM2_PCD_MASK       0xff
#define     CSKY_LCDTIM2_CLKS_EXT       1 << 8
#define     CSKY_LCDTIM2_VSP_ACT_LOW	1 << 9
#define     CSKY_LCDTIM2_HSP_ACT_LOW	1 << 10
#define     CSKY_LCDTIM2_PCP_FAL        1 << 11
#define     CSKY_LCDTIM2_OEP_ACT_LOW	1 << 12			

#define     CSKY_LCDINT_STAT_LDD        1
#define     CSKY_LCDINT_STAT_BAU        1 << 1
#define     CSKY_LCDINT_STAT_BER        1 << 2
#define     CSKY_LCDINT_STAT_LFU        1 << 3 
#define     CSKY_LCDINT_MASK_LDD        1
#define     CSKY_LCDINT_MASK_BAU        1 << 1
#define     CSKY_LCDINT_MASK_BER        1 << 2
#define     CSKY_LCDINT_MASK_LFU        1 << 3             

#define     CSKY_LCD_SOURCE_CLK         40000000 // To do better

/* GPIO configure for LCD */
//Interface mode
#define CSKY_SYNC_MODE           0x409d
#define CSKY_DE_MODE             0x609d
#define CSKY_RGB_RGB             0x204

// H/V Sync Backporch
#define CSKY_HLW             0x0f
#define CSKY_HBP             0x00
#define CSKY_HFP             0x00
#define CSKY_VLW             0x02
#define CSKY_VBP             0x00
#define CSKY_VFP             0x02

#endif
