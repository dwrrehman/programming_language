	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 13, 0	sdk_version 13, 3
	.globl	_calc                           ; -- Begin function calc
	.p2align	2
_calc:                                  ; @calc
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #80
	.cfi_def_cfa_offset 80
	stp	x24, x23, [sp, #16]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #32]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #48]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #64]             ; 16-byte Folded Spill
	add	x29, sp, #64
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	cbz	x2, LBB0_3
; %bb.1:
	mov	x19, x2
	mov	x20, x1
	mov	x21, x0
	mov	x23, #-3689348814741910324
	movk	x23, #52429
Lloh0:
	adrp	x22, l_.str@PAGE
Lloh1:
	add	x22, x22, l_.str@PAGEOFF
LBB0_2:                                 ; =>This Inner Loop Header: Depth=1
	umulh	x8, x20, x23
	lsr	x8, x8, #2
	add	x8, x8, x8, lsl #2
	sub	x8, x20, x8
	str	x8, [sp]
	mov	x0, x22
	bl	_printf
	add	x20, x20, x21
	subs	x19, x19, #1
	b.ne	LBB0_2
LBB0_3:
	ldp	x29, x30, [sp, #64]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #48]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #32]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #80
	ret
	.loh AdrpAdd	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"%i\n"

.subsections_via_symbols
