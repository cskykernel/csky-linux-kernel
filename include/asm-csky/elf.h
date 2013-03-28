#ifndef __ASMCSKY_ELF_H
#define __ASMCSKY_ELF_H

/*
 * ELF register definitions..
 */

#include <asm/ptrace.h>
#include <asm/user.h>

/* CSKY Relocations */
#define R_CSKY_NONE               0
#define R_CSKY_32                 1
#define R_CSKY_PCIMM8BY4          2
#define R_CSKY_PCIMM11BY2         3
#define R_CSKY_PCIMM4BY2          4
#define R_CSKY_PC32               5
#define R_CSKY_PCRELJSR_IMM11BY2  6
#define R_CSKY_GNU_VTINHERIT      7
#define R_CSKY_GNU_VTENTRY        8
#define R_CSKY_RELATIVE           9
#define R_CSKY_COPY               10
#define R_CSKY_GLOB_DAT           11
#define R_CSKY_JUMP_SLOT          12

typedef unsigned long elf_greg_t;
#define EM_CSKY 39

#define ELF_NGREG (sizeof(struct user_regs_struct) / sizeof(elf_greg_t))

typedef elf_greg_t elf_gregset_t[ELF_NGREG];

typedef struct user_cskyfp_struct elf_fpregset_t;

/*
 * This is used to ensure we don't load something for the wrong architecture.
 */
#define elf_check_arch(x) ((x)->e_machine == EM_CSKY)

/*
 * These are used to set parameters in the core dumps.
 */
#define ELF_CLASS	ELFCLASS32
#ifdef  __cskyBE__
#define ELF_DATA	ELFDATA2MSB
#else
#define ELF_DATA    ELFDATA2LSB
#endif
#define ELF_ARCH	EM_CSKY

#define ELF_PLAT_INIT(_r, load_addr)	_r->r2 = 0

#define USE_ELF_CORE_DUMP
#define ELF_EXEC_PAGESIZE	4096

/* This is the location that an ET_DYN program is loaded if exec'ed.  Typical
   use of this is to invoke "./ld.so someprog" to test out a new version of
   the loader.  We need to make sure that it is out of the way of the program
   that it will "exec", and that there is sufficient room for the brk.  */

#define ELF_ET_DYN_BASE         0x00000000UL

#if defined(CONFIG_CPU_CSKYV1)
#define ELF_CORE_COPY_REGS(pr_reg, regs)        \
	pr_reg[0] = regs->pc;                   \
	pr_reg[1] = regs->r1;                   \
	pr_reg[2] = regs->syscallr2;            \
	pr_reg[3] = regs->sr;                   \
	pr_reg[4] = regs->r2;                   \
	pr_reg[5] = regs->r3;                   \
	pr_reg[6] = regs->r4;                   \
	pr_reg[7] = regs->r5;                   \
	pr_reg[8] = regs->r6;                   \
	pr_reg[9] = regs->r7;                   \
	pr_reg[10] = regs->r8;                  \
	pr_reg[11] = regs->r9;                  \
	pr_reg[12] = regs->r10;                 \
	pr_reg[13] = regs->r11;                 \
	pr_reg[14] = regs->r12;                 \
	pr_reg[15] = regs->r13;                 \
	pr_reg[16] = regs->r14;                 \
	pr_reg[17] = regs->r15;                 \
	pr_reg[18] = rdusp();
#else
#define ELF_CORE_COPY_REGS(pr_reg, regs)        \
	pr_reg[0] = regs->pc;                   \
	pr_reg[1] = regs->r1;                   \
	pr_reg[2] = regs->syscallr2;            \
	pr_reg[3] = regs->sr;                   \
	pr_reg[4] = regs->r2;                   \
	pr_reg[5] = regs->r3;                   \
	pr_reg[6] = regs->r4;                   \
	pr_reg[7] = regs->r5;                   \
	pr_reg[8] = regs->r6;                   \
	pr_reg[9] = regs->r7;                   \
	pr_reg[10] = regs->r8;                  \
	pr_reg[11] = regs->r9;                  \
	pr_reg[12] = regs->r10;                 \
	pr_reg[13] = regs->r11;                 \
	pr_reg[14] = regs->r12;                 \
	pr_reg[15] = regs->r13;                 \
	pr_reg[16] = regs->r14;                 \
	pr_reg[17] = regs->r15;                 \
	pr_reg[18] = regs->r16;                 \
	pr_reg[19] = regs->r17;                 \
	pr_reg[20] = regs->r18;                 \
	pr_reg[21] = regs->r19;                 \
	pr_reg[22] = regs->r20;                 \
	pr_reg[23] = regs->r21;                 \
	pr_reg[24] = regs->r22;                 \
	pr_reg[25] = regs->r23;                 \
	pr_reg[26] = regs->r24;                 \
	pr_reg[27] = regs->r25;                 \
	pr_reg[28] = regs->r26;                 \
	pr_reg[29] = regs->r27;                 \
	pr_reg[30] = regs->r28;                 \
	pr_reg[31] = regs->r29;                 \
	pr_reg[32] = regs->r30;                 \
	pr_reg[33] = regs->r31;                 \
	pr_reg[34] = rdusp(); 
#endif

/* This yields a mask that user programs can use to figure out what
   instruction set this cpu supports.  */

#define ELF_HWCAP	(0)

/* This yields a string that ld.so will use to load implementation
   specific libraries for optimization.  This is more specific in
   intent than poking at uname or /proc/cpuinfo.  */

#define ELF_PLATFORM  (NULL)

#define SET_PERSONALITY(ex, ibcs2) set_personality((ibcs2)?PER_SVR4:PER_LINUX)

#endif
