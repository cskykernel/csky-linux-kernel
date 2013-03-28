	.section .piggydata,"a"
	.globl	input_data
input_data:
	.incbin	"arch/csky/boot/compressed/piggy.gz"
	.globl	input_data_end
input_data_end:
