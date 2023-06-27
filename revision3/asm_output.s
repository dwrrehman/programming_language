	.section __TEXT,__text,regular,pure_instructions
	.build_version macos, 13, 0 sdk_version 13, 3
	.globl pasta
	.p2align 2
pasta:
	add xzr, xzr, xzr
	mov x0, #37
	mov x16, #1
	svc #0
.subsections_via_symbols
