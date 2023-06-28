	.section __TEXT,__text,regular,pure_instructions
	.build_version macos, 13, 0 sdk_version 13, 3
	.globl startingthingy
	.p2align 2
startingthingy:
	movz x0, 0x5
	movz x1, 0x6
	movz x2, 0x7
	movz x2, 0x8
	add x0, x1, x0
	sub x1, x2, x2
	movz x16, 0x1b0
	svc 0x80
	add x2, x2, x3
	add x0, x0, x2
	mov x0, #37
	mov x16, #1
	svc 0x80 ;generated
.subsections_via_symbols
