	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 13, 0	sdk_version 13, 3
	.globl	_main                           ; -- Begin function main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #64
	.cfi_def_cfa_offset 64
	stp	x22, x21, [sp, #16]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #32]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	mov	w20, #65536
Lloh0:
	adrp	x21, l_.str@PAGE
Lloh1:
	add	x21, x21, l_.str@PAGEOFF
Lloh2:
	adrp	x19, l_.str.2@PAGE
Lloh3:
	add	x19, x19, l_.str.2@PAGEOFF
LBB0_1:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_2 Depth 2
	sub	x20, x20, #1
	str	x21, [sp]
	mov	x0, x19
	bl	_printf
	mov	x22, #0
LBB0_2:                                 ;   Parent Loop BB0_1 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	lsr	x8, x20, x22
	mov	w0, #48
	bfxil	w0, w8, #0, #1
	bl	_putchar
	add	x22, x22, #1
	cmp	x22, #64
	b.ne	LBB0_2
; %bb.3:                                ;   in Loop: Header=BB0_1 Depth=1
	mov	w0, #10
	bl	_putchar
	cbnz	x20, LBB0_1
; %bb.4:
Lloh4:
	adrp	x0, l_.str.1@PAGE
Lloh5:
	add	x0, x0, l_.str.1@PAGEOFF
	bl	_puts
	mov	w0, #0
	bl	_exit
	.loh AdrpAdd	Lloh2, Lloh3
	.loh AdrpAdd	Lloh0, Lloh1
	.loh AdrpAdd	Lloh4, Lloh5
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"i = "

l_.str.1:                               ; @.str.1
	.asciz	"[done looping!]"

l_.str.2:                               ; @.str.2
	.asciz	"%s"

.subsections_via_symbols
