	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 11, 0	sdk_version 11, 1
	.globl	_main
	.p2align	4, 0x90
_main:
	mov $5, %rax
	retq

