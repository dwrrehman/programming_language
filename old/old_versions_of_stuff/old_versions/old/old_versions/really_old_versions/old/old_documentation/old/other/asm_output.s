	.section __TEXT,__text,regular,pure_instructions
	.build_version macos, 13, 0 sdk_version 13, 3
	.globl _main
	.p2align 2
_main:
	orr x0, x0, x1
	ret
.subsections_via_symbols
