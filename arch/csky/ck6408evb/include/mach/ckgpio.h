/*
 *
 * This file is subject to the terms and conditions of the GNU General Public
 * License.  See the file "COPYING" in the main directory of this archive
 * for more details.
 *
 * Copyright (C) 2011  Hangzhou C-SKY Microsystems co.,ltd.
 */

#ifndef __CKGPIO_H__
#define __CKGPIO_H__

#define CKPIC_PADR      0x00      // Port A data register
#define CKPIC_PADDR     0x04      // Port A data direction register
#define CKPIC_PBDR      0x0c      // Port B data register
#define CKPIC_PBDDR     0x10      // Port B data direction register
#define CKPIC_INTEN     0x30      // Interrupt enable register
#define CKPIC_INTMASK   0x34      // Interrupt mask register
#define CKPIC_INTTYPE   0x38      // Interrupt level register
#define CKPIC_INTPOLA   0x3c      // Interrupt polarity register
#define CKPIC_INSTATUS  0x40      // Interrupt status of Port A
#define CKPIC_RAWINTST  0x44      // Raw interrupt status of Port A (premasking)
#define CKPIC_PORTAEOI  0x4c      // Port A clear interrupt register
#define CKPIC_EXTPORTA  0x50      // Port A external port register
#define CKPIC_EXTPORTB  0x54      // Port B external port register
#define CKPIC_LS_SYNC   0x60      // Level-sensitive synchronization enable register

/* PIO Definitions for one of the Port, and we do the max size possibilaty. */
#define CK_GPIO_P0     ((unsigned int) 1 << 0)  /* Pin Controlled by P0 */
#define CK_GPIO_P1     ((unsigned int) 1 << 1)  /* Pin Controlled by P1 */
#define CK_GPIO_P2     ((unsigned int) 1 << 2)  /* Pin Controlled by P2 */
#define CK_GPIO_P3     ((unsigned int) 1 << 3)  /* Pin Controlled by P3 */
#define CK_GPIO_P4     ((unsigned int) 1 << 4)  /* Pin Controlled by P4 */
#define CK_GPIO_P5     ((unsigned int) 1 << 5)  /* Pin Controlled by P5 */
#define CK_GPIO_P6     ((unsigned int) 1 << 6)  /* Pin Controlled by P6 */
#define CK_GPIO_P7     ((unsigned int) 1 << 7)  /* Pin Controlled by P7 */
#define CK_GPIO_P8     ((unsigned int) 1 << 8)  /* Pin Controlled by P8 */
#define CK_GPIO_P9     ((unsigned int) 1 << 9)  /* Pin Controlled by P9 */
#define CK_GPIO_P10     ((unsigned int) 1 << 10)  /* Pin Controlled by P10 */
#define CK_GPIO_P11     ((unsigned int) 1 << 11)  /* Pin Controlled by P11 */
#define CK_GPIO_P12     ((unsigned int) 1 << 12)  /* Pin Controlled by P12 */
#define CK_GPIO_P13     ((unsigned int) 1 << 13)  /* Pin Controlled by P13 */
#define CK_GPIO_P14     ((unsigned int) 1 << 14)  /* Pin Controlled by P14 */
#define CK_GPIO_P15     ((unsigned int) 1 << 15)  /* Pin Controlled by P15 */
#define CK_GPIO_P16     ((unsigned int) 1 << 16)  /* Pin Controlled by P16 */
#define CK_GPIO_P17     ((unsigned int) 1 << 17)  /* Pin Controlled by P17 */
#define CK_GPIO_P18     ((unsigned int) 1 << 18)  /* Pin Controlled by P18 */
#define CK_GPIO_P19     ((unsigned int) 1 << 19)  /* Pin Controlled by P19 */
#define CK_GPIO_P20     ((unsigned int) 1 << 20)  /* Pin Controlled by P20 */
#define CK_GPIO_P21     ((unsigned int) 1 << 21)  /* Pin Controlled by P21 */
#define CK_GPIO_P22     ((unsigned int) 1 << 22)  /* Pin Controlled by P22 */
#define CK_GPIO_P23     ((unsigned int) 1 << 23)  /* Pin Controlled by P23 */
#define CK_GPIO_P24     ((unsigned int) 1 << 24)  /* Pin Controlled by P24 */
#define CK_GPIO_P25     ((unsigned int) 1 << 25)  /* Pin Controlled by P25 */
#define CK_GPIO_P26     ((unsigned int) 1 << 26)  /* Pin Controlled by P26 */
#define CK_GPIO_P27     ((unsigned int) 1 << 27)  /* Pin Controlled by P27 */
#define CK_GPIO_P28     ((unsigned int) 1 << 28)  /* Pin Controlled by P28 */
#define CK_GPIO_P29     ((unsigned int) 1 << 29)  /* Pin Controlled by P29 */
#define CK_GPIO_P30     ((unsigned int) 1 << 30)  /* Pin Controlled by P30 */
#define CK_GPIO_P31     ((unsigned int) 1 << 31)  /* Pin Controlled by P31 */

#endif
