	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 13, 0	sdk_version 13, 3
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	mov	x9, #0
	mov	x8, #0
	mov	w10, #2
	b	LBB0_3
LBB0_1:                                 ;   in Loop: Header=BB0_3 Depth=1
	add	x8, x8, #1
LBB0_2:                                 ;   in Loop: Header=BB0_3 Depth=1
	add	x10, x10, #1
	add	x9, x9, #1
	cmp	x10, #128, lsl #12              ; =524288
	b.eq	LBB0_7
LBB0_3:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_5 Depth 2
	cmp	x10, #3
	b.lo	LBB0_1
; %bb.4:                                ;   in Loop: Header=BB0_3 Depth=1
	mov	x11, #0
LBB0_5:                                 ;   Parent Loop BB0_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	add	x12, x11, #2
	udiv	x13, x10, x12
	msub	x12, x13, x12, x10
	cbz	x12, LBB0_2
; %bb.6:                                ;   in Loop: Header=BB0_5 Depth=2
	add	x11, x11, #1
	cmp	x9, x11
	b.ne	LBB0_5
	b	LBB0_1
LBB0_7:
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	str	x8, [sp]
	mov	w0, #1
	bl	_syscall
	mov	w0, #0
	ldp	x29, x30, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #32
	ret
	.cfi_endproc
                                        ; -- End function
.subsections_via_symbols
