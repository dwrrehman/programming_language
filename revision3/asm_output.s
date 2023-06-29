	.section __TEXT,__text,regular,pure_instructions
	.build_version macos, 13, 0 sdk_version 13, 3
	.globl pasta
	.p2align 2
pasta:
	movz x0, 0x2211
	movk x0, 0x4433, lsl 16
	movk x0, 0x6655, lsl 32
	movk x0, 0x8877, lsl 48
	movz x1, 0x0
	movz x2, 0xe400
	movk x2, 0x540b, lsl 16
	movk x2, 0x2, lsl 32
L_loop:
	movz x3, 0x1
	add x1, x1, x3
	cmp x1, x2
	b.ne L_loop
	cmp x1, x1
	b.eq L_bubbles0
L_bubbles0:
	movz x0, 0x2a
	cmp x1, x1
	b.hs L_bubbles1
L_bubbles1:
	movz x0, 0x2a
	cmp x1, x1
	b.ge L_bubbles2
L_bubbles2:
	movz x0, 0x2a
	cmp x1, x0
	b.lo L_bubbles3
L_bubbles3:
	movz x0, 0x2a
	cmp x1, x0
	b.lt L_bubbles3
	mov x0, #37
	mov x16, #1
	svc 0x80 ; temporary auto-generated exit syscall
.subsections_via_symbols
