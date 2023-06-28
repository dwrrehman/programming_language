	.section __TEXT,__text,regular,pure_instructions
	.build_version macos, 13, 0 sdk_version 13, 3
	.globl entrypoint
	.p2align 2
entrypoint:
	add x1, x0, x0
	and x2, x1, x0
	orr x3, x2, x2
	sub x4, x1, x3
	eor x1, x4, x1
otherthingylabel:
	add x0, x0, x0
	eor x0, x0, x0
	mul x2, x4, x3
	mov x0, #37
	mov x16, #1
	svc #0
.subsections_via_symbols
