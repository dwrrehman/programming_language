	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 13, 0	sdk_version 13, 3
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	mov	x8, #0
	mov	x0, #0
	mov	w9, #2
	b	LBB0_3
LBB0_1:                                 ;   in Loop: Header=BB0_3 Depth=1
	add	x0, x0, #1
LBB0_2:                                 ;   in Loop: Header=BB0_3 Depth=1
	add	x9, x9, #1
	add	x8, x8, #1
	cmp	x9, #64, lsl #12                ; =262144
	b.eq	LBB0_7
LBB0_3:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_5 Depth 2
	cmp	x9, #3
	b.lo	LBB0_1
; %bb.4:                                ;   in Loop: Header=BB0_3 Depth=1
	mov	x10, #0
LBB0_5:                                 ;   Parent Loop BB0_3 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	add	x11, x10, #2
	udiv	x12, x9, x11
	msub	x11, x12, x11, x9
	cbz	x11, LBB0_2
; %bb.6:                                ;   in Loop: Header=BB0_5 Depth=2
	add	x10, x10, #1
	cmp	x8, x10
	b.ne	LBB0_5
	b	LBB0_1
LBB0_7:
	stp	x29, x30, [sp, #-16]!           ; 16-byte Folded Spill
	.cfi_def_cfa_offset 16
	mov	x29, sp
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	bl	_printbinary
Lloh0:
	adrp	x0, l_.str.1@PAGE
Lloh1:
	add	x0, x0, l_.str.1@PAGEOFF
	bl	_puts
	mov	w0, #0
	bl	_exit
	.loh AdrpAdd	Lloh0, Lloh1
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function printbinary
_printbinary:                           ; @printbinary
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #48
	.cfi_def_cfa_offset 48
	stp	x20, x19, [sp, #16]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	x19, x0
Lloh2:
	adrp	x8, l_.str@PAGE
Lloh3:
	add	x8, x8, l_.str@PAGEOFF
	str	x8, [sp]
Lloh4:
	adrp	x0, l_.str.2@PAGE
Lloh5:
	add	x0, x0, l_.str.2@PAGEOFF
	bl	_printf
	mov	x20, #0
LBB1_1:                                 ; =>This Inner Loop Header: Depth=1
	lsr	x8, x19, x20
	mov	w0, #48
	bfxil	w0, w8, #0, #1
	bl	_putchar
	add	x20, x20, #1
	cmp	x20, #64
	b.ne	LBB1_1
; %bb.2:
	mov	w0, #10
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #48
	b	_putchar
	.loh AdrpAdd	Lloh4, Lloh5
	.loh AdrpAdd	Lloh2, Lloh3
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"total prime count = "

l_.str.1:                               ; @.str.1
	.asciz	"[done looping!]"

l_.str.2:                               ; @.str.2
	.asciz	"%s"

.subsections_via_symbols
