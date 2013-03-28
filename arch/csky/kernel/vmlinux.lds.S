/* ld script to make csky Linux kernel */


#include <asm-generic/vmlinux.lds.h>
#include <asm/page.h>

#ifdef __csky__
OUTPUT_ARCH(csky)    /* define in csky-gcc4.5 */
#else
OUTPUT_ARCH(csky)  /* define in csky-gcc3.2 */
#endif

ENTRY(_start)
	PHDRS {
		text PT_LOAD FLAGS(7);  /* RWX */
		data PT_LOAD FLAGS(7);
		note PT_NOTE FLAGS(4);  /* R__ */
	}

#ifndef __cskyBE__
jiffies = jiffies_64;
#else
jiffies = jiffies_64 + 4;
#endif

SECTIONS
{
	. = PAGE_OFFSET + CONFIG_RAM_BASE;
	_text = .;			/* Text and read-only data */
	.text : {
		HEAD_TEXT
		TEXT_TEXT
		SCHED_TEXT
		LOCK_TEXT
		*(.fixup)
		*(.gnu.warning)
	} :text = 0

	_etext = .;			/* End of text section */

	. = ALIGN(16);		/* Exception table */
	__ex_table : {
		__start___ex_table = .;
		*(__ex_table)
		__stop___ex_table = .;
	}

	RODATA

	.data : {			/* Data */
		DATA_DATA
		CONSTRUCTORS
	}

	. = ALIGN(16);
	.data.cacheline_aligned : { *(.data.cacheline_aligned) }

	.bss : {
		_sbss = . ;
		*(.bss)
		_ebss = . ;
	}		/* BSS */

	_edata = .;			/* End of data section */

	/* will be freed after init */
	. = ALIGN(4096);		/* Init code and data */
	__init_begin = .;
	.init.text : {
		_sinittext = .;
		INIT_TEXT
		_einittext = .;
		__tagtable_begin = .;
		*(.taglist.init)
		__tagtable_end = .;
	} :data

	.init.data : { INIT_DATA }

	. = ALIGN(16);
	.init.setup : {
		__setup_start = .;
		*(.init.setup)
		__setup_end = .;
	}

	.initcall.init : {
		__initcall_start = .;
		INITCALLS
		__initcall_end = .;
	}
	.con_initcall.init : {
		__con_initcall_start = .;
		*(.con_initcall.init)
		__con_initcall_end = .;
	}

	.csky_fixup : {
		__start_fixup = .;
		*(.csky_fixup)
		__stop_fixup = .;
	}

	SECURITY_INIT
#ifdef CONFIG_BLK_DEV_INITRD
	. = ALIGN(8192);
	__initramfs_start = .;
	.init.ramfs : { *(.init.ramfs) }
	__initramfs_end = .;
#endif
	NOTES
	. = ALIGN(8192);
	__init_end = .;

	.data.init_task : { *(.data.init_task) }	/* The initial task and kernel stack */

	_end = . ;

	/* Sections to be discarded */
	/DISCARD/ : {
		EXIT_TEXT
		EXIT_DATA
		*(.exitcall.exit)
	}

	/* Stabs debugging sections.  */
	STABS_DEBUG
	DWARF_DEBUG

	/* These must appear regardless of  .  */
	.gptab.sdata : {
		*(.gptab.data)
		*(.gptab.sdata)
	}

	.gptab.sbss : {
		*(.gptab.bss)
		*(.gptab.sbss)
	}
}
