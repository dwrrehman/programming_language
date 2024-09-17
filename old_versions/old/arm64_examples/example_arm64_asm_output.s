	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 13, 0	sdk_version 13, 3
	.section	__TEXT,__literal16,16byte_literals
	.p2align	4                               ; -- Begin function main
lCPI0_0:
	.quad	0                               ; 0x0
	.quad	1                               ; 0x1
lCPI0_1:
	.quad	2                               ; 0x2
	.quad	3                               ; 0x3
lCPI0_2:
	.quad	4                               ; 0x4
	.quad	5                               ; 0x5
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	stp	x28, x27, [sp, #-96]!           ; 16-byte Folded Spill
	.cfi_def_cfa_offset 96
	stp	x26, x25, [sp, #16]             ; 16-byte Folded Spill
	stp	x24, x23, [sp, #32]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #48]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #64]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #80]             ; 16-byte Folded Spill
	add	x29, sp, #80
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	.cfi_offset w25, -72
	.cfi_offset w26, -80
	.cfi_offset w27, -88
	.cfi_offset w28, -96
	mov	w9, #4208
Lloh0:
	adrp	x16, ___chkstk_darwin@GOTPAGE
Lloh1:
	ldr	x16, [x16, ___chkstk_darwin@GOTPAGEOFF]
	blr	x16
	sub	sp, sp, #1, lsl #12             ; =4096
	sub	sp, sp, #112
Lloh2:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh3:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh4:
	ldr	x8, [x8]
	stur	x8, [x29, #-96]
	cmp	w0, #1
	b.le	LBB0_109
; %bb.1:
	ldr	x19, [x1, #8]
Lloh5:
	adrp	x1, l_.str.131@PAGE
Lloh6:
	add	x1, x1, l_.str.131@PAGEOFF
	mov	x0, x19
	bl	_fopen
	cbz	x0, LBB0_110
; %bb.2:
	mov	x20, x0
	mov	x1, #0
	mov	w2, #2
	bl	_fseek
	mov	x0, x20
	bl	_ftell
	mov	x21, x0
	add	x0, x0, #1
	mov	w1, #1
	bl	_calloc
	mov	x22, x0
	mov	x0, x20
	mov	x1, #0
	mov	w2, #0
	bl	_fseek
	mov	x0, x22
	mov	w1, #1
	mov	x2, x21
	mov	x3, x20
	bl	_fread
	mov	x0, x20
	bl	_fclose
	stp	x19, x21, [sp]
Lloh7:
	adrp	x0, l_.str.133@PAGE
Lloh8:
	add	x0, x0, l_.str.133@PAGEOFF
	bl	_printf
	str	x22, [sp]
Lloh9:
	adrp	x0, l_.str.134@PAGE
Lloh10:
	add	x0, x0, l_.str.134@PAGEOFF
	bl	_printf
	stp	xzr, xzr, [sp, #80]
	stp	xzr, xzr, [sp, #64]
	add	x2, sp, #80
	add	x3, sp, #88
	add	x4, sp, #64
	add	x5, sp, #72
	mov	x0, x22
	mov	x1, x21
	bl	_parse
	ldp	x28, x20, [sp, #80]
	ldr	x19, [sp, #64]
	mov	x0, x28
	mov	x1, x20
	mov	x2, x19
	bl	_print_instructions
	ldr	x25, [sp, #72]
	mov	x0, x19
	mov	x1, x25
	bl	_print_dictionary
	mov	w0, #32768
	bl	_malloc
	mov	x23, x0
	cbz	x25, LBB0_11
; %bb.3:
	mov	x21, #0
	mov	x26, #0
	add	x22, x19, #16
Lloh11:
	adrp	x24, l_.str.139@PAGE
Lloh12:
	add	x24, x24, l_.str.139@PAGEOFF
LBB0_4:                                 ; =>This Inner Loop Header: Depth=1
	ldr	x8, [x22]
	cmp	x8, #2
	b.ne	LBB0_6
; %bb.5:                                ;   in Loop: Header=BB0_4 Depth=1
	ldur	x8, [x22, #-16]
	str	x8, [sp]
	mov	x0, x24
	bl	_printf
	str	x21, [x23, x26, lsl #3]
	add	x26, x26, #1
LBB0_6:                                 ;   in Loop: Header=BB0_4 Depth=1
	add	x21, x21, #1
	add	x22, x22, #40
	cmp	x25, x21
	b.ne	LBB0_4
; %bb.7:
	str	x26, [sp]
Lloh13:
	adrp	x0, l_.str.140@PAGE
Lloh14:
	add	x0, x0, l_.str.140@PAGEOFF
	bl	_printf
	cbz	x26, LBB0_12
; %bb.8:
	mov	x21, #0
	mov	w22, #40
Lloh15:
	adrp	x24, l_.str.141@PAGE
Lloh16:
	add	x24, x24, l_.str.141@PAGEOFF
LBB0_9:                                 ; =>This Inner Loop Header: Depth=1
	ldr	x8, [x23, x21, lsl #3]
	mul	x8, x8, x22
	ldr	x8, [x19, x8]
	stp	x21, x8, [sp]
	mov	x0, x24
	bl	_printf
	add	x21, x21, #1
	cmp	x26, x21
	b.ne	LBB0_9
; %bb.10:
	str	wzr, [sp, #28]                  ; 4-byte Folded Spill
	b	LBB0_13
LBB0_11:
	str	xzr, [sp]
Lloh17:
	adrp	x0, l_.str.140@PAGE
Lloh18:
	add	x0, x0, l_.str.140@PAGEOFF
	bl	_printf
	mov	x26, #0
LBB0_12:
	mov	w8, #1
	str	w8, [sp, #28]                   ; 4-byte Folded Spill
LBB0_13:
Lloh19:
	adrp	x0, l_str.194@PAGE
Lloh20:
	add	x0, x0, l_str.194@PAGEOFF
	bl	_puts
Lloh21:
	adrp	x0, l_str.195@PAGE
Lloh22:
	add	x0, x0, l_str.195@PAGEOFF
	bl	_puts
	stp	x20, x28, [sp, #48]             ; 16-byte Folded Spill
	stp	x26, x23, [sp, #32]             ; 16-byte Folded Spill
	cbz	x20, LBB0_44
; %bb.14:
	mov	x22, #0
	add	x23, x28, #72
	add	x26, x28, #8
	str	x25, [sp, #16]                  ; 8-byte Folded Spill
LBB0_15:                                ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_23 Depth 2
                                        ;     Child Loop BB0_18 Depth 2
	mov	w0, #10
	bl	_putchar
	str	x22, [sp]
Lloh23:
	adrp	x0, l_.str.144@PAGE
Lloh24:
	add	x0, x0, l_.str.144@PAGEOFF
	bl	_printf
	mov	w8, #152
	madd	x27, x22, x8, x28
	ldr	q0, [x27, #96]
	str	q0, [sp, #192]
	ldr	q0, [x27, #112]
	str	q0, [sp, #208]
	ldr	q0, [x27, #128]
	str	q0, [sp, #224]
	ldr	x8, [x27, #144]
	str	x8, [sp, #240]
	ldr	q0, [x27, #32]
	str	q0, [sp, #128]
	ldr	q0, [x27, #48]
	str	q0, [sp, #144]
	ldr	q0, [x27, #64]
	str	q0, [sp, #160]
	ldr	q0, [x27, #80]
	str	q0, [sp, #176]
	ldr	q0, [x27]
	str	q0, [sp, #96]
	ldr	q0, [x27, #16]
	str	q0, [sp, #112]
	add	x0, sp, #96
	mov	x1, x19
	bl	_print_instruction
	ldr	x21, [x27]
	cbz	x21, LBB0_30
; %bb.16:                               ;   in Loop: Header=BB0_15 Depth=1
	and	x9, x21, #0xfffffffffffffff8
Lloh25:
	adrp	x8, _arity@PAGE
Lloh26:
	add	x8, x8, _arity@PAGEOFF
	ldr	x8, [x8, x21, lsl #3]
	cmp	x9, #16
	b.ne	LBB0_22
; %bb.17:                               ;   in Loop: Header=BB0_15 Depth=1
	mov	x21, #0
	cmp	x8, #1
	csinc	x24, x8, xzr, hi
LBB0_18:                                ;   Parent Loop BB0_15 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldr	x8, [x26, x21, lsl #3]
	mov	w9, #40
	madd	x1, x8, x9, x19
	ldr	x8, [x1, #32]
	cmn	x8, #1
	ccmp	x21, #0, #0, eq
	csel	x9, x22, x8, eq
	str	x9, [x23, x21, lsl #3]
	cbz	x21, LBB0_21
; %bb.19:                               ;   in Loop: Header=BB0_18 Depth=2
	cmn	x8, #1
	b.eq	LBB0_104
; %bb.20:                               ;   in Loop: Header=BB0_18 Depth=2
	ldr	x9, [sp, #56]                   ; 8-byte Folded Reload
	mov	w10, #152
	madd	x20, x8, x10, x9
	str	x22, [x20, #136]
	ldr	x9, [x1]
	stp	x9, x8, [sp]
Lloh27:
	adrp	x0, l_.str.146@PAGE
Lloh28:
	add	x0, x0, l_.str.146@PAGEOFF
	bl	_printf
	ldr	q0, [x20, #96]
	str	q0, [sp, #192]
	ldr	q0, [x20, #112]
	str	q0, [sp, #208]
	ldr	q0, [x20, #128]
	str	q0, [sp, #224]
	ldr	x8, [x20, #144]
	str	x8, [sp, #240]
	ldr	q0, [x20, #32]
	str	q0, [sp, #128]
	ldr	q0, [x20, #48]
	str	q0, [sp, #144]
	ldr	q0, [x20, #64]
	str	q0, [sp, #160]
	ldr	q0, [x20, #80]
	str	q0, [sp, #176]
	ldr	q0, [x20]
	str	q0, [sp, #96]
	ldr	q0, [x20, #16]
	str	q0, [sp, #112]
	add	x0, sp, #96
	mov	x1, x19
	bl	_print_instruction
LBB0_21:                                ;   in Loop: Header=BB0_18 Depth=2
	add	x21, x21, #1
	cmp	x24, x21
	b.ne	LBB0_18
	b	LBB0_29
LBB0_22:                                ;   in Loop: Header=BB0_15 Depth=1
	mov	x25, #0
	cmp	x8, #1
	csinc	x24, x8, xzr, hi
	mov	x28, x23
LBB0_23:                                ;   Parent Loop BB0_15 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	cmp	x21, #35
	b.ne	LBB0_25
; %bb.24:                               ;   in Loop: Header=BB0_23 Depth=2
	cmp	x25, #1
	b.eq	LBB0_29
LBB0_25:                                ;   in Loop: Header=BB0_23 Depth=2
	ldur	x8, [x28, #-64]
	mov	w9, #40
	madd	x1, x8, x9, x19
	ldr	x8, [x1, #32]
	cmn	x8, #1
	ccmp	x25, #0, #0, eq
	csel	x9, x22, x8, eq
	str	x9, [x28]
	cbz	x25, LBB0_28
; %bb.26:                               ;   in Loop: Header=BB0_23 Depth=2
	cmn	x8, #1
	b.eq	LBB0_105
; %bb.27:                               ;   in Loop: Header=BB0_23 Depth=2
	ldr	x9, [sp, #56]                   ; 8-byte Folded Reload
	mov	w10, #152
	madd	x20, x8, x10, x9
	str	x22, [x20, #136]
	ldr	x9, [x1]
	stp	x9, x8, [sp]
Lloh29:
	adrp	x0, l_.str.146@PAGE
Lloh30:
	add	x0, x0, l_.str.146@PAGEOFF
	bl	_printf
	ldr	q0, [x20, #96]
	str	q0, [sp, #192]
	ldr	q0, [x20, #112]
	str	q0, [sp, #208]
	ldr	q0, [x20, #128]
	str	q0, [sp, #224]
	ldr	x8, [x20, #144]
	str	x8, [sp, #240]
	ldr	q0, [x20, #32]
	str	q0, [sp, #128]
	ldr	q0, [x20, #48]
	str	q0, [sp, #144]
	ldr	q0, [x20, #64]
	str	q0, [sp, #160]
	ldr	q0, [x20, #80]
	str	q0, [sp, #176]
	ldr	q0, [x20]
	str	q0, [sp, #96]
	ldr	q0, [x20, #16]
	str	q0, [sp, #112]
	add	x0, sp, #96
	mov	x1, x19
	bl	_print_instruction
LBB0_28:                                ;   in Loop: Header=BB0_23 Depth=2
	add	x25, x25, #1
	add	x28, x28, #8
	cmp	x24, x25
	b.ne	LBB0_23
LBB0_29:                                ;   in Loop: Header=BB0_15 Depth=1
	ldr	x28, [sp, #56]                  ; 8-byte Folded Reload
	mov	w8, #152
	madd	x8, x22, x8, x28
	ldr	x9, [x8, #8]
	str	x22, [x8, #128]
	mov	w8, #40
	madd	x8, x9, x8, x19
	str	x22, [x8, #32]
	ldr	x8, [x8]
	stp	x8, x22, [sp]
Lloh31:
	adrp	x0, l_.str.147@PAGE
Lloh32:
	add	x0, x0, l_.str.147@PAGEOFF
	bl	_printf
	str	x22, [sp]
Lloh33:
	adrp	x0, l_.str.148@PAGE
Lloh34:
	add	x0, x0, l_.str.148@PAGEOFF
	bl	_printf
	ldr	q0, [x27, #96]
	str	q0, [sp, #192]
	ldr	q0, [x27, #112]
	str	q0, [sp, #208]
	ldr	q0, [x27, #128]
	str	q0, [sp, #224]
	ldr	x8, [x27, #144]
	str	x8, [sp, #240]
	ldr	q0, [x27, #32]
	str	q0, [sp, #128]
	ldr	q0, [x27, #48]
	str	q0, [sp, #144]
	ldr	q0, [x27, #64]
	str	q0, [sp, #160]
	ldr	q0, [x27, #80]
	str	q0, [sp, #176]
	ldr	q0, [x27]
	str	q0, [sp, #96]
	ldr	q0, [x27, #16]
	str	q0, [sp, #112]
	add	x0, sp, #96
	mov	x1, x19
	bl	_print_instruction
Lloh35:
	adrp	x0, l_str.197@PAGE
Lloh36:
	add	x0, x0, l_str.197@PAGEOFF
	bl	_puts
	mov	x0, x19
	ldr	x25, [sp, #16]                  ; 8-byte Folded Reload
	mov	x1, x25
	bl	_print_dictionary
Lloh37:
	adrp	x0, l_str.198@PAGE
Lloh38:
	add	x0, x0, l_str.198@PAGEOFF
	bl	_puts
LBB0_30:                                ;   in Loop: Header=BB0_15 Depth=1
	add	x22, x22, #1
	add	x23, x23, #152
	add	x26, x26, #152
	ldr	x20, [sp, #48]                  ; 8-byte Folded Reload
	cmp	x22, x20
	b.ne	LBB0_15
; %bb.31:
Lloh39:
	adrp	x0, l_str.196@PAGE
Lloh40:
	add	x0, x0, l_str.196@PAGEOFF
	bl	_puts
	mov	x0, x28
	mov	x1, x20
	mov	x2, x19
	bl	_print_instructions
	add	x21, x28, #136
	mov	w22, #40
	mov	x23, x20
Lloh41:
	adrp	x24, l_.str.152@PAGE
Lloh42:
	add	x24, x24, l_.str.152@PAGEOFF
LBB0_32:                                ; =>This Inner Loop Header: Depth=1
	ldr	x8, [x21]
	cmn	x8, #1
	b.ne	LBB0_35
; %bb.33:                               ;   in Loop: Header=BB0_32 Depth=1
	ldur	x8, [x21, #-8]
	cmn	x8, #1
	b.eq	LBB0_35
; %bb.34:                               ;   in Loop: Header=BB0_32 Depth=1
	ldur	x8, [x21, #-128]
	mul	x8, x8, x22
	ldr	x8, [x19, x8]
	str	x8, [sp]
	mov	x0, x24
	bl	_printf
	ldur	q0, [x21, #-40]
	str	q0, [sp, #192]
	ldur	q0, [x21, #-24]
	str	q0, [sp, #208]
	ldur	q0, [x21, #-8]
	str	q0, [sp, #224]
	ldr	x8, [x21, #8]
	str	x8, [sp, #240]
	ldur	q0, [x21, #-104]
	str	q0, [sp, #128]
	ldur	q0, [x21, #-88]
	str	q0, [sp, #144]
	ldur	q0, [x21, #-72]
	str	q0, [sp, #160]
	ldur	q0, [x21, #-56]
	str	q0, [sp, #176]
	ldur	q0, [x21, #-136]
	str	q0, [sp, #96]
	ldur	q0, [x21, #-120]
	str	q0, [sp, #112]
	add	x0, sp, #96
	mov	x1, x19
	bl	_print_instruction
LBB0_35:                                ;   in Loop: Header=BB0_32 Depth=1
	add	x21, x21, #152
	subs	x23, x23, #1
	b.ne	LBB0_32
; %bb.36:
	mov	x27, x25
	mov	w0, #32768
	bl	_malloc
	mov	x24, x0
	mov	x21, #0
	mov	x22, #0
	mov	x23, x28
Lloh43:
	adrp	x25, l_.str.153@PAGE
Lloh44:
	add	x25, x25, l_.str.153@PAGEOFF
LBB0_37:                                ; =>This Inner Loop Header: Depth=1
	ldr	x8, [x23]
	cmp	x8, #36
	b.ne	LBB0_39
; %bb.38:                               ;   in Loop: Header=BB0_37 Depth=1
	str	x21, [sp]
	mov	x0, x25
	bl	_printf
	str	x21, [x24, x22, lsl #3]
	add	x22, x22, #1
LBB0_39:                                ;   in Loop: Header=BB0_37 Depth=1
	add	x21, x21, #1
	add	x23, x23, #152
	cmp	x20, x21
	b.ne	LBB0_37
; %bb.40:
Lloh45:
	adrp	x0, l_str.199@PAGE
Lloh46:
	add	x0, x0, l_str.199@PAGEOFF
	bl	_puts
	cbz	x22, LBB0_45
; %bb.41:
	mov	x21, #0
Lloh47:
	adrp	x25, l_.str.155@PAGE
Lloh48:
	add	x25, x25, l_.str.155@PAGEOFF
	mov	w23, #152
LBB0_42:                                ; =>This Inner Loop Header: Depth=1
	str	x21, [sp]
	mov	x0, x25
	bl	_printf
	ldr	x8, [x24, x21, lsl #3]
	madd	x8, x8, x23, x28
	ldr	q0, [x8, #96]
	str	q0, [sp, #192]
	ldr	q0, [x8, #112]
	str	q0, [sp, #208]
	ldr	q0, [x8, #128]
	str	q0, [sp, #224]
	ldr	x9, [x8, #144]
	str	x9, [sp, #240]
	ldr	q0, [x8, #32]
	str	q0, [sp, #128]
	ldr	q0, [x8, #48]
	str	q0, [sp, #144]
	ldr	q0, [x8, #64]
	str	q0, [sp, #160]
	ldr	q0, [x8, #80]
	str	q0, [sp, #176]
	ldr	q0, [x8]
	str	q0, [sp, #96]
	ldr	q0, [x8, #16]
	str	q0, [sp, #112]
	add	x0, sp, #96
	mov	x1, x19
	bl	_print_instruction
	add	x21, x21, #1
	cmp	x22, x21
	b.ne	LBB0_42
; %bb.43:
	mov	w21, #0
	b	LBB0_46
LBB0_44:
Lloh49:
	adrp	x0, l_str.196@PAGE
Lloh50:
	add	x0, x0, l_str.196@PAGEOFF
	bl	_puts
Lloh51:
	adrp	x0, l_str.192@PAGE
Lloh52:
	add	x0, x0, l_str.192@PAGEOFF
	bl	_puts
Lloh53:
	adrp	x0, l_str.194@PAGE
Lloh54:
	add	x0, x0, l_str.194@PAGEOFF
	bl	_puts
	mov	w0, #32768
	bl	_malloc
	mov	x24, x0
Lloh55:
	adrp	x0, l_str.199@PAGE
Lloh56:
	add	x0, x0, l_str.199@PAGEOFF
	bl	_puts
	mov	x22, #0
	mov	w21, #1
	b	LBB0_47
LBB0_45:
	mov	w21, #1
LBB0_46:
	mov	x25, x27
LBB0_47:
Lloh57:
	adrp	x0, l_str.200@PAGE
Lloh58:
	add	x0, x0, l_str.200@PAGEOFF
	bl	_puts
Lloh59:
	adrp	x8, lCPI0_0@PAGE
Lloh60:
	ldr	q0, [x8, lCPI0_0@PAGEOFF]
Lloh61:
	adrp	x8, lCPI0_1@PAGE
Lloh62:
	ldr	q1, [x8, lCPI0_1@PAGEOFF]
	stp	q0, q1, [sp, #96]
Lloh63:
	adrp	x8, lCPI0_2@PAGE
Lloh64:
	ldr	q0, [x8, lCPI0_2@PAGEOFF]
	str	q0, [sp, #128]
	mov	w8, #16
	str	x8, [sp, #144]
	tbnz	w21, #0, LBB0_53
; %bb.48:
	mov	x8, #0
	add	x9, x28, #72
	mov	w10, #152
	add	x11, sp, #96
LBB0_49:                                ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_50 Depth 2
	mov	x12, #0
	ldr	x13, [x24, x8, lsl #3]
	madd	x13, x13, x10, x9
LBB0_50:                                ;   Parent Loop BB0_49 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldr	x14, [x13, x12]
	madd	x14, x14, x10, x28
	ldr	x15, [x14, #64]!
	cmn	x15, #1
	b.ne	LBB0_102
; %bb.51:                               ;   in Loop: Header=BB0_50 Depth=2
	ldr	x15, [x11, x12]
	str	x15, [x14]
	add	x12, x12, #8
	cmp	x12, #56
	b.ne	LBB0_50
; %bb.52:                               ;   in Loop: Header=BB0_49 Depth=1
	add	x8, x8, #1
	cmp	x8, x22
	b.ne	LBB0_49
LBB0_53:
Lloh65:
	adrp	x0, l_str.201@PAGE
Lloh66:
	add	x0, x0, l_str.201@PAGEOFF
	bl	_puts
	mov	x0, x28
	mov	x1, x20
	mov	x2, x19
	bl	_print_instructions
Lloh67:
	adrp	x0, l_str.202@PAGE
Lloh68:
	add	x0, x0, l_str.202@PAGEOFF
	bl	_puts
	mov	w0, #31
	mov	w1, #8
	bl	_calloc
	cbz	x20, LBB0_69
; %bb.54:
	mov	x8, #0
	add	x9, x28, #136
	mov	w10, #152
	movi.2d	v0, #0000000000000000
LBB0_55:                                ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_57 Depth 2
                                        ;     Child Loop BB0_64 Depth 2
	madd	x11, x8, x10, x28
	ldr	x12, [x11, #64]!
	cmn	x12, #1
	b.ne	LBB0_68
; %bb.56:                               ;   in Loop: Header=BB0_55 Depth=1
	str	xzr, [x0, #240]
	stp	q0, q0, [x0, #208]
	stp	q0, q0, [x0, #176]
	stp	q0, q0, [x0, #144]
	stp	q0, q0, [x0, #112]
	stp	q0, q0, [x0, #80]
	stp	q0, q0, [x0, #48]
	stp	q0, q0, [x0, #16]
	madd	x15, x8, x10, x28
	str	q0, [x0]
	add	x12, x15, #128
	mov	x13, x20
	mov	x14, x9
	ldr	x15, [x15, #136]
LBB0_57:                                ;   Parent Loop BB0_55 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldur	x16, [x14, #-8]
	cmp	x15, x16
	b.ls	LBB0_59
; %bb.58:                               ;   in Loop: Header=BB0_57 Depth=2
	ldr	x16, [x14]
	ldr	x17, [x12]
	cmp	x16, x17
	cset	w16, hi
	b	LBB0_60
LBB0_59:                                ;   in Loop: Header=BB0_57 Depth=2
	mov	x16, #0
LBB0_60:                                ;   in Loop: Header=BB0_57 Depth=2
	ldur	x17, [x14, #-72]
	cmn	x17, #1
	b.eq	LBB0_62
; %bb.61:                               ;   in Loop: Header=BB0_57 Depth=2
	str	x16, [x0, x17, lsl #3]
LBB0_62:                                ;   in Loop: Header=BB0_57 Depth=2
	add	x14, x14, #152
	subs	x13, x13, #1
	b.ne	LBB0_57
; %bb.63:                               ;   in Loop: Header=BB0_55 Depth=1
	mov	x12, #0
LBB0_64:                                ;   Parent Loop BB0_55 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldr	x13, [x0, x12, lsl #3]
	cbz	x13, LBB0_66
; %bb.65:                               ;   in Loop: Header=BB0_64 Depth=2
	add	x12, x12, #1
	cmp	x12, #31
	b.ne	LBB0_64
	b	LBB0_103
LBB0_66:                                ;   in Loop: Header=BB0_55 Depth=1
	cmp	x12, #30
	b.hi	LBB0_103
; %bb.67:                               ;   in Loop: Header=BB0_55 Depth=1
	str	x12, [x11]
LBB0_68:                                ;   in Loop: Header=BB0_55 Depth=1
	add	x8, x8, #1
	cmp	x8, x20
	b.ne	LBB0_55
LBB0_69:
Lloh69:
	adrp	x0, l_str.203@PAGE
Lloh70:
	add	x0, x0, l_str.203@PAGEOFF
	bl	_puts
	mov	x0, x28
	mov	x1, x20
	mov	x2, x19
	bl	_print_instructions
Lloh71:
	adrp	x0, l_str.204@PAGE
Lloh72:
	add	x0, x0, l_str.204@PAGEOFF
	bl	_puts
	cbz	x25, LBB0_111
; %bb.70:
Lloh73:
	adrp	x0, l_.str.164@PAGE
Lloh74:
	add	x0, x0, l_.str.164@PAGEOFF
Lloh75:
	adrp	x1, l_.str.165@PAGE
Lloh76:
	add	x1, x1, l_.str.165@PAGEOFF
	bl	_fopen
	mov	x23, x0
	ldr	x8, [x19]
	str	x8, [sp]
Lloh77:
	adrp	x1, l_.str.166@PAGE
Lloh78:
	add	x1, x1, l_.str.166@PAGEOFF
	bl	_fprintf
	cbz	x20, LBB0_100
; %bb.71:
	mov	x27, #0
	mov	w21, #152
	mov	w22, #40
Lloh79:
	adrp	x26, l_.str.182@PAGE
Lloh80:
	add	x26, x26, l_.str.182@PAGEOFF
Lloh81:
	adrp	x25, l_.str.169@PAGE
Lloh82:
	add	x25, x25, l_.str.169@PAGEOFF
LBB0_72:                                ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_73 Depth 2
Lloh83:
	adrp	x0, l_str.206@PAGE
Lloh84:
	add	x0, x0, l_str.206@PAGEOFF
	bl	_puts
	madd	x28, x27, x21, x28
	ldr	q0, [x28, #96]
	str	q0, [sp, #192]
	ldr	q0, [x28, #112]
	str	q0, [sp, #208]
	ldr	q0, [x28, #128]
	str	q0, [sp, #224]
	ldr	x8, [x28, #144]
	str	x8, [sp, #240]
	ldr	q0, [x28, #32]
	str	q0, [sp, #128]
	ldr	q0, [x28, #48]
	str	q0, [sp, #144]
	ldr	q0, [x28, #64]
	str	q0, [sp, #160]
	ldr	q0, [x28, #80]
	str	q0, [sp, #176]
	ldr	q0, [x28]
	str	q0, [sp, #96]
	ldr	q0, [x28, #16]
	str	q0, [sp, #112]
	add	x0, sp, #96
	mov	x1, x19
	bl	_print_instruction
	ldp	x24, x21, [sp, #32]             ; 16-byte Folded Reload
	ldr	w8, [sp, #28]                   ; 4-byte Folded Reload
	tbnz	w8, #0, LBB0_76
LBB0_73:                                ;   Parent Loop BB0_72 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldr	x8, [x21]
	madd	x9, x8, x22, x19
	ldr	x9, [x9, #24]
	cmp	x9, x27
	b.ne	LBB0_75
; %bb.74:                               ;   in Loop: Header=BB0_73 Depth=2
	mul	x20, x8, x22
	ldr	x8, [x19, x20]
	stp	x8, x27, [sp]
Lloh85:
	adrp	x0, l_.str.168@PAGE
Lloh86:
	add	x0, x0, l_.str.168@PAGEOFF
	bl	_printf
	ldr	x8, [x19, x20]
	str	x8, [sp]
	mov	x0, x23
	mov	x1, x25
	bl	_fprintf
LBB0_75:                                ;   in Loop: Header=BB0_73 Depth=2
	add	x21, x21, #8
	subs	x24, x24, #1
	b.ne	LBB0_73
LBB0_76:                                ;   in Loop: Header=BB0_72 Depth=1
	ldr	x0, [x28]
	cmp	x0, #21
	b.hi	LBB0_79
; %bb.77:                               ;   in Loop: Header=BB0_72 Depth=1
	ldr	x20, [sp, #56]                  ; 8-byte Folded Reload
	mov	w21, #152
	madd	x8, x27, x21, x20
	add	x8, x8, #8
Lloh87:
	adrp	x11, lJTI0_0@PAGE
Lloh88:
	add	x11, x11, lJTI0_0@PAGEOFF
	adr	x9, LBB0_78
	ldrh	w10, [x11, x0, lsl #1]
	add	x9, x9, x10, lsl #2
	br	x9
LBB0_78:                                ;   in Loop: Header=BB0_72 Depth=1
	ldr	q0, [x28, #96]
	str	q0, [sp, #192]
	ldr	q0, [x28, #112]
	str	q0, [sp, #208]
	ldr	q0, [x28, #128]
	str	q0, [sp, #224]
	ldr	x8, [x28, #144]
	str	x8, [sp, #240]
	ldr	q0, [x28, #32]
	str	q0, [sp, #128]
	ldr	q0, [x28, #48]
	str	q0, [sp, #144]
	ldr	q0, [x28, #64]
	str	q0, [sp, #160]
	ldr	q0, [x28, #80]
	str	q0, [sp, #176]
	ldr	q0, [x28]
	str	q0, [sp, #96]
	ldr	q0, [x28, #16]
	str	q0, [sp, #112]
	add	x2, sp, #96
Lloh89:
	adrp	x0, l_.str.45@PAGE
Lloh90:
	add	x0, x0, l_.str.45@PAGEOFF
	b	LBB0_87
LBB0_79:                                ;   in Loop: Header=BB0_72 Depth=1
	cmp	x0, #35
	mov	w21, #152
	b.eq	LBB0_93
; %bb.80:                               ;   in Loop: Header=BB0_72 Depth=1
	cmp	x0, #36
	ldr	x28, [sp, #56]                  ; 8-byte Folded Reload
	b.ne	LBB0_107
; %bb.81:                               ;   in Loop: Header=BB0_72 Depth=1
Lloh91:
	adrp	x0, l_.str.170@PAGE
Lloh92:
	add	x0, x0, l_.str.170@PAGEOFF
	mov	w1, #10
	mov	w2, #1
	mov	x3, x23
	bl	_fwrite
	b	LBB0_99
LBB0_82:                                ;   in Loop: Header=BB0_72 Depth=1
	ldr	q0, [x28, #96]
	str	q0, [sp, #192]
	ldr	q0, [x28, #112]
	str	q0, [sp, #208]
	ldr	q0, [x28, #128]
	str	q0, [sp, #224]
	ldr	x8, [x28, #144]
	str	x8, [sp, #240]
	ldr	q0, [x28, #32]
	str	q0, [sp, #128]
	ldr	q0, [x28, #48]
	str	q0, [sp, #144]
	ldr	q0, [x28, #64]
	str	q0, [sp, #160]
	ldr	q0, [x28, #80]
	str	q0, [sp, #176]
	ldr	q0, [x28]
	str	q0, [sp, #96]
	ldr	q0, [x28, #16]
	str	q0, [sp, #112]
	add	x2, sp, #96
Lloh93:
	adrp	x0, l_.str.172@PAGE
Lloh94:
	add	x0, x0, l_.str.172@PAGEOFF
	b	LBB0_87
LBB0_83:                                ;   in Loop: Header=BB0_72 Depth=1
	ldr	q0, [x28, #96]
	str	q0, [sp, #192]
	ldr	q0, [x28, #112]
	str	q0, [sp, #208]
	ldr	q0, [x28, #128]
	str	q0, [sp, #224]
	ldr	x8, [x28, #144]
	str	x8, [sp, #240]
	ldr	q0, [x28, #32]
	str	q0, [sp, #128]
	ldr	q0, [x28, #48]
	str	q0, [sp, #144]
	ldr	q0, [x28, #64]
	str	q0, [sp, #160]
	ldr	q0, [x28, #80]
	str	q0, [sp, #176]
	ldr	q0, [x28]
	str	q0, [sp, #96]
	ldr	q0, [x28, #16]
	str	q0, [sp, #112]
	add	x2, sp, #96
Lloh95:
	adrp	x0, l_.str.47@PAGE
Lloh96:
	add	x0, x0, l_.str.47@PAGEOFF
	b	LBB0_87
LBB0_84:                                ;   in Loop: Header=BB0_72 Depth=1
	ldr	q0, [x28, #96]
	str	q0, [sp, #192]
	ldr	q0, [x28, #112]
	str	q0, [sp, #208]
	ldr	q0, [x28, #128]
	str	q0, [sp, #224]
	ldr	x8, [x28, #144]
	str	x8, [sp, #240]
	ldr	q0, [x28, #32]
	str	q0, [sp, #128]
	ldr	q0, [x28, #48]
	str	q0, [sp, #144]
	ldr	q0, [x28, #64]
	str	q0, [sp, #160]
	ldr	q0, [x28, #80]
	str	q0, [sp, #176]
	ldr	q0, [x28]
	str	q0, [sp, #96]
	ldr	q0, [x28, #16]
	str	q0, [sp, #112]
	add	x2, sp, #96
Lloh97:
	adrp	x0, l_.str.171@PAGE
Lloh98:
	add	x0, x0, l_.str.171@PAGEOFF
	b	LBB0_87
LBB0_85:                                ;   in Loop: Header=BB0_72 Depth=1
	ldr	q0, [x28, #96]
	str	q0, [sp, #192]
	ldr	q0, [x28, #112]
	str	q0, [sp, #208]
	ldr	q0, [x28, #128]
	str	q0, [sp, #224]
	ldr	x8, [x28, #144]
	str	x8, [sp, #240]
	ldr	q0, [x28, #32]
	str	q0, [sp, #128]
	ldr	q0, [x28, #48]
	str	q0, [sp, #144]
	ldr	q0, [x28, #64]
	str	q0, [sp, #160]
	ldr	q0, [x28, #80]
	str	q0, [sp, #176]
	ldr	q0, [x28]
	str	q0, [sp, #96]
	ldr	q0, [x28, #16]
	str	q0, [sp, #112]
	add	x2, sp, #96
Lloh99:
	adrp	x0, l_.str.49@PAGE
Lloh100:
	add	x0, x0, l_.str.49@PAGEOFF
	b	LBB0_87
LBB0_86:                                ;   in Loop: Header=BB0_72 Depth=1
	ldr	q0, [x28, #96]
	str	q0, [sp, #192]
	ldr	q0, [x28, #112]
	str	q0, [sp, #208]
	ldr	q0, [x28, #128]
	str	q0, [sp, #224]
	ldr	x8, [x28, #144]
	str	x8, [sp, #240]
	ldr	q0, [x28, #32]
	str	q0, [sp, #128]
	ldr	q0, [x28, #48]
	str	q0, [sp, #144]
	ldr	q0, [x28, #64]
	str	q0, [sp, #160]
	ldr	q0, [x28, #80]
	str	q0, [sp, #176]
	ldr	q0, [x28]
	str	q0, [sp, #96]
	ldr	q0, [x28, #16]
	str	q0, [sp, #112]
	add	x2, sp, #96
Lloh101:
	adrp	x0, l_.str.50@PAGE
Lloh102:
	add	x0, x0, l_.str.50@PAGEOFF
LBB0_87:                                ;   in Loop: Header=BB0_72 Depth=1
	mov	x1, x23
	mov	x3, x20
	mov	x4, x19
	bl	_generate_operation
	mov	x28, x20
	b	LBB0_99
LBB0_88:                                ;   in Loop: Header=BB0_72 Depth=1
	ldr	x8, [x8]
	madd	x9, x8, x22, x19
	ldr	x9, [x9, #24]
	cmn	x9, #1
	b.eq	LBB0_106
; %bb.89:                               ;   in Loop: Header=BB0_72 Depth=1
	mul	x8, x8, x22
	ldr	x20, [x19, x8]
	ldr	x28, [sp, #56]                  ; 8-byte Folded Reload
	madd	x8, x27, x21, x28
	ldp	x9, x8, [x8, #80]
	madd	x9, x9, x21, x28
	ldr	x9, [x9, #64]
	madd	x8, x8, x21, x28
	ldr	x8, [x8, #64]
	stp	x9, x8, [sp]
	mov	x0, x23
Lloh103:
	adrp	x1, l_.str.173@PAGE
Lloh104:
	add	x1, x1, l_.str.173@PAGEOFF
	bl	_fprintf
	str	x20, [sp]
	mov	x0, x23
Lloh105:
	adrp	x1, l_.str.174@PAGE
Lloh106:
	add	x1, x1, l_.str.174@PAGEOFF
	b	LBB0_92
LBB0_90:                                ;   in Loop: Header=BB0_72 Depth=1
	ldr	x8, [x8]
	madd	x9, x8, x22, x19
	ldr	x9, [x9, #24]
	cmn	x9, #1
	b.eq	LBB0_106
; %bb.91:                               ;   in Loop: Header=BB0_72 Depth=1
	mul	x8, x8, x22
	ldr	x20, [x19, x8]
	ldr	x28, [sp, #56]                  ; 8-byte Folded Reload
	madd	x8, x27, x21, x28
	ldp	x9, x8, [x8, #80]
	madd	x9, x9, x21, x28
	ldr	x9, [x9, #64]
	madd	x8, x8, x21, x28
	ldr	x8, [x8, #64]
	stp	x9, x8, [sp]
	mov	x0, x23
Lloh107:
	adrp	x1, l_.str.173@PAGE
Lloh108:
	add	x1, x1, l_.str.173@PAGEOFF
	bl	_fprintf
	str	x20, [sp]
	mov	x0, x23
Lloh109:
	adrp	x1, l_.str.175@PAGE
Lloh110:
	add	x1, x1, l_.str.175@PAGEOFF
LBB0_92:                                ;   in Loop: Header=BB0_72 Depth=1
	bl	_fprintf
	b	LBB0_99
LBB0_93:                                ;   in Loop: Header=BB0_72 Depth=1
	ldr	x9, [sp, #56]                   ; 8-byte Folded Reload
	madd	x8, x27, x21, x9
	ldr	x24, [x8, #64]
	cmp	x24, #32
	b.hs	LBB0_108
; %bb.94:                               ;   in Loop: Header=BB0_72 Depth=1
	madd	x8, x27, x21, x9
	ldr	x8, [x8, #16]
	madd	x8, x8, x22, x19
	ldr	x9, [x8, #8]
	str	x9, [sp, #96]
	ldr	x0, [x8]
	add	x1, sp, #96
	bl	_string_to_number
	mov	x28, x0
	ldr	x8, [sp, #96]
	stp	x0, x8, [sp]
Lloh111:
	adrp	x0, l_.str.122@PAGE
Lloh112:
	add	x0, x0, l_.str.122@PAGEOFF
	bl	_printf
	ubfx	x21, x28, #16, #16
Lloh113:
	adrp	x0, l_.str.185@PAGE
Lloh114:
	add	x0, x0, l_.str.185@PAGEOFF
	mov	w1, #6
	mov	w2, #1
	mov	x3, x23
	bl	_fwrite
	str	x24, [sp]
	mov	x0, x23
	mov	x20, x26
	mov	x1, x26
	bl	_fprintf
	and	w8, w28, #0xffff
	str	x8, [sp]
	mov	x0, x23
Lloh115:
	adrp	x1, l_.str.186@PAGE
Lloh116:
	add	x1, x1, l_.str.186@PAGEOFF
	bl	_fprintf
	cbz	w21, LBB0_98
; %bb.95:                               ;   in Loop: Header=BB0_72 Depth=1
	ubfx	x26, x28, #32, #16
Lloh117:
	adrp	x0, l_.str.187@PAGE
Lloh118:
	add	x0, x0, l_.str.187@PAGEOFF
	mov	w1, #6
	mov	w2, #1
	mov	x3, x23
	bl	_fwrite
	str	x24, [sp]
	mov	x0, x23
	mov	x1, x20
	bl	_fprintf
                                        ; kill: def $w21 killed $w21 killed $x21 def $x21
	str	x21, [sp]
	mov	x0, x23
Lloh119:
	adrp	x1, l_.str.188@PAGE
Lloh120:
	add	x1, x1, l_.str.188@PAGEOFF
	bl	_fprintf
	cbz	w26, LBB0_98
; %bb.96:                               ;   in Loop: Header=BB0_72 Depth=1
Lloh121:
	adrp	x0, l_.str.187@PAGE
Lloh122:
	add	x0, x0, l_.str.187@PAGEOFF
	mov	w1, #6
	mov	w2, #1
	mov	x3, x23
	bl	_fwrite
	str	x24, [sp]
	mov	x0, x23
	mov	x1, x20
	bl	_fprintf
	mov	x8, x26
	str	x8, [sp]
	mov	x0, x23
Lloh123:
	adrp	x1, l_.str.189@PAGE
Lloh124:
	add	x1, x1, l_.str.189@PAGEOFF
	bl	_fprintf
	lsr	x21, x28, #48
	cbz	x21, LBB0_98
; %bb.97:                               ;   in Loop: Header=BB0_72 Depth=1
Lloh125:
	adrp	x0, l_.str.187@PAGE
Lloh126:
	add	x0, x0, l_.str.187@PAGEOFF
	mov	w1, #6
	mov	w2, #1
	mov	x3, x23
	bl	_fwrite
	str	x24, [sp]
	mov	x0, x23
	mov	x1, x20
	bl	_fprintf
	str	x21, [sp]
	mov	x0, x23
Lloh127:
	adrp	x1, l_.str.190@PAGE
Lloh128:
	add	x1, x1, l_.str.190@PAGEOFF
	bl	_fprintf
LBB0_98:                                ;   in Loop: Header=BB0_72 Depth=1
	ldr	x28, [sp, #56]                  ; 8-byte Folded Reload
	mov	w21, #152
	mov	x26, x20
LBB0_99:                                ;   in Loop: Header=BB0_72 Depth=1
	add	x27, x27, #1
	ldr	x8, [sp, #48]                   ; 8-byte Folded Reload
	cmp	x27, x8
	b.ne	LBB0_72
LBB0_100:
Lloh129:
	adrp	x0, l_.str.177@PAGE
Lloh130:
	add	x0, x0, l_.str.177@PAGEOFF
	mov	w20, #1
	mov	w1, #13
	mov	w2, #1
	mov	x3, x23
	bl	_fwrite
	str	x20, [sp]
Lloh131:
	adrp	x1, l_.str.178@PAGE
Lloh132:
	add	x1, x1, l_.str.178@PAGEOFF
	mov	x0, x23
	bl	_fprintf
Lloh133:
	adrp	x0, l_.str.179@PAGE
Lloh134:
	add	x0, x0, l_.str.179@PAGEOFF
	mov	w1, #50
	mov	w2, #1
	mov	x3, x23
	bl	_fwrite
Lloh135:
	adrp	x0, l_.str.180@PAGE
Lloh136:
	add	x0, x0, l_.str.180@PAGEOFF
	mov	w1, #25
	mov	w2, #1
	mov	x3, x23
	bl	_fwrite
	mov	x0, x23
	bl	_fclose
Lloh137:
	adrp	x0, l_.str.135@PAGE
Lloh138:
	add	x0, x0, l_.str.135@PAGEOFF
	bl	_system
	add	x0, sp, #96
	mov	w1, #4096
	bl	_bzero
	ldr	x8, [x19]
Lloh139:
	adrp	x9, l_.str@PAGE
Lloh140:
	add	x9, x9, l_.str@PAGEOFF
	stp	x8, x9, [sp]
Lloh141:
	adrp	x2, l_.str.137@PAGE
Lloh142:
	add	x2, x2, l_.str.137@PAGEOFF
	add	x0, sp, #96
	mov	w1, #4096
	bl	_snprintf
Lloh143:
	adrp	x19, l_.str.136@PAGE
Lloh144:
	add	x19, x19, l_.str.136@PAGEOFF
	mov	x0, x19
	bl	_puts
	mov	x0, x19
	bl	_system
Lloh145:
	adrp	x0, l_.str.138@PAGE
Lloh146:
	add	x0, x0, l_.str.138@PAGEOFF
	bl	_system
	add	x0, sp, #96
	bl	_puts
	add	x0, sp, #96
	bl	_system
	ldur	x8, [x29, #-96]
Lloh147:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh148:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh149:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB0_112
; %bb.101:
	mov	w0, #0
	add	sp, sp, #1, lsl #12             ; =4096
	add	sp, sp, #112
	ldp	x29, x30, [sp, #80]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #64]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #48]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #32]             ; 16-byte Folded Reload
	ldp	x26, x25, [sp, #16]             ; 16-byte Folded Reload
	ldp	x28, x27, [sp], #96             ; 16-byte Folded Reload
	ret
LBB0_102:
	bl	_main.cold.6
LBB0_103:
	bl	_main.cold.5
LBB0_104:
	mov	x0, x27
	mov	x2, x19
	bl	_main.cold.7
LBB0_105:
	mov	x0, x27
	mov	x2, x19
	bl	_main.cold.8
LBB0_106:
	bl	_abort
LBB0_107:
	bl	_main.cold.3
LBB0_108:
	bl	_main.cold.4
LBB0_109:
	bl	_repl
LBB0_110:
	bl	_main.cold.1
LBB0_111:
	bl	_main.cold.2
LBB0_112:
	bl	___stack_chk_fail
	.loh AdrpLdrGotLdr	Lloh2, Lloh3, Lloh4
	.loh AdrpLdrGot	Lloh0, Lloh1
	.loh AdrpAdd	Lloh5, Lloh6
	.loh AdrpAdd	Lloh9, Lloh10
	.loh AdrpAdd	Lloh7, Lloh8
	.loh AdrpAdd	Lloh11, Lloh12
	.loh AdrpAdd	Lloh13, Lloh14
	.loh AdrpAdd	Lloh15, Lloh16
	.loh AdrpAdd	Lloh17, Lloh18
	.loh AdrpAdd	Lloh21, Lloh22
	.loh AdrpAdd	Lloh19, Lloh20
	.loh AdrpAdd	Lloh23, Lloh24
	.loh AdrpAdd	Lloh25, Lloh26
	.loh AdrpAdd	Lloh27, Lloh28
	.loh AdrpAdd	Lloh29, Lloh30
	.loh AdrpAdd	Lloh37, Lloh38
	.loh AdrpAdd	Lloh35, Lloh36
	.loh AdrpAdd	Lloh33, Lloh34
	.loh AdrpAdd	Lloh31, Lloh32
	.loh AdrpAdd	Lloh41, Lloh42
	.loh AdrpAdd	Lloh39, Lloh40
	.loh AdrpAdd	Lloh43, Lloh44
	.loh AdrpAdd	Lloh45, Lloh46
	.loh AdrpAdd	Lloh47, Lloh48
	.loh AdrpAdd	Lloh55, Lloh56
	.loh AdrpAdd	Lloh53, Lloh54
	.loh AdrpAdd	Lloh51, Lloh52
	.loh AdrpAdd	Lloh49, Lloh50
	.loh AdrpLdr	Lloh63, Lloh64
	.loh AdrpAdrp	Lloh61, Lloh63
	.loh AdrpLdr	Lloh61, Lloh62
	.loh AdrpAdrp	Lloh59, Lloh61
	.loh AdrpLdr	Lloh59, Lloh60
	.loh AdrpAdd	Lloh57, Lloh58
	.loh AdrpAdd	Lloh67, Lloh68
	.loh AdrpAdd	Lloh65, Lloh66
	.loh AdrpAdd	Lloh71, Lloh72
	.loh AdrpAdd	Lloh69, Lloh70
	.loh AdrpAdd	Lloh77, Lloh78
	.loh AdrpAdd	Lloh75, Lloh76
	.loh AdrpAdd	Lloh73, Lloh74
	.loh AdrpAdd	Lloh81, Lloh82
	.loh AdrpAdd	Lloh79, Lloh80
	.loh AdrpAdd	Lloh83, Lloh84
	.loh AdrpAdd	Lloh85, Lloh86
	.loh AdrpAdd	Lloh87, Lloh88
	.loh AdrpAdd	Lloh89, Lloh90
	.loh AdrpAdd	Lloh91, Lloh92
	.loh AdrpAdd	Lloh93, Lloh94
	.loh AdrpAdd	Lloh95, Lloh96
	.loh AdrpAdd	Lloh97, Lloh98
	.loh AdrpAdd	Lloh99, Lloh100
	.loh AdrpAdd	Lloh101, Lloh102
	.loh AdrpAdd	Lloh105, Lloh106
	.loh AdrpAdd	Lloh103, Lloh104
	.loh AdrpAdd	Lloh109, Lloh110
	.loh AdrpAdd	Lloh107, Lloh108
	.loh AdrpAdd	Lloh115, Lloh116
	.loh AdrpAdd	Lloh113, Lloh114
	.loh AdrpAdd	Lloh111, Lloh112
	.loh AdrpAdd	Lloh119, Lloh120
	.loh AdrpAdd	Lloh117, Lloh118
	.loh AdrpAdd	Lloh123, Lloh124
	.loh AdrpAdd	Lloh121, Lloh122
	.loh AdrpAdd	Lloh127, Lloh128
	.loh AdrpAdd	Lloh125, Lloh126
	.loh AdrpLdrGotLdr	Lloh147, Lloh148, Lloh149
	.loh AdrpAdd	Lloh145, Lloh146
	.loh AdrpAdd	Lloh143, Lloh144
	.loh AdrpAdd	Lloh141, Lloh142
	.loh AdrpAdd	Lloh139, Lloh140
	.loh AdrpAdd	Lloh137, Lloh138
	.loh AdrpAdd	Lloh135, Lloh136
	.loh AdrpAdd	Lloh133, Lloh134
	.loh AdrpAdd	Lloh131, Lloh132
	.loh AdrpAdd	Lloh129, Lloh130
	.cfi_endproc
	.section	__TEXT,__const
	.p2align	1
lJTI0_0:
	.short	(LBB0_106-LBB0_78)>>2
	.short	(LBB0_107-LBB0_78)>>2
	.short	(LBB0_107-LBB0_78)>>2
	.short	(LBB0_107-LBB0_78)>>2
	.short	(LBB0_78-LBB0_78)>>2
	.short	(LBB0_82-LBB0_78)>>2
	.short	(LBB0_83-LBB0_78)>>2
	.short	(LBB0_84-LBB0_78)>>2
	.short	(LBB0_85-LBB0_78)>>2
	.short	(LBB0_86-LBB0_78)>>2
	.short	(LBB0_107-LBB0_78)>>2
	.short	(LBB0_107-LBB0_78)>>2
	.short	(LBB0_107-LBB0_78)>>2
	.short	(LBB0_107-LBB0_78)>>2
	.short	(LBB0_107-LBB0_78)>>2
	.short	(LBB0_107-LBB0_78)>>2
	.short	(LBB0_107-LBB0_78)>>2
	.short	(LBB0_107-LBB0_78)>>2
	.short	(LBB0_107-LBB0_78)>>2
	.short	(LBB0_107-LBB0_78)>>2
	.short	(LBB0_88-LBB0_78)>>2
	.short	(LBB0_90-LBB0_78)>>2
                                        ; -- End function
	.section	__TEXT,__text,regular,pure_instructions
	.p2align	2                               ; -- Begin function repl
_repl:                                  ; @repl
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #448
	.cfi_def_cfa_offset 448
	stp	x28, x27, [sp, #352]            ; 16-byte Folded Spill
	stp	x26, x25, [sp, #368]            ; 16-byte Folded Spill
	stp	x24, x23, [sp, #384]            ; 16-byte Folded Spill
	stp	x22, x21, [sp, #400]            ; 16-byte Folded Spill
	stp	x20, x19, [sp, #416]            ; 16-byte Folded Spill
	stp	x29, x30, [sp, #432]            ; 16-byte Folded Spill
	add	x29, sp, #432
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	.cfi_offset w25, -72
	.cfi_offset w26, -80
	.cfi_offset w27, -88
	.cfi_offset w28, -96
	add	x24, sp, #176
Lloh150:
	adrp	x0, l_.str.1@PAGE
Lloh151:
	add	x0, x0, l_.str.1@PAGEOFF
	bl	_puts
	str	xzr, [sp, #240]
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [x24, #32]
	stp	q0, q0, [sp, #176]
	add	x1, sp, #176
	mov	w0, #0
	bl	_tcgetattr
	ldp	q0, q1, [x24, #32]
	stp	q0, q1, [sp, #128]
	ldr	x8, [sp, #240]
	str	x8, [sp, #160]
	ldp	q1, q0, [sp, #176]
	stp	q1, q0, [sp, #96]
	ldr	x8, [sp, #120]
	mov	x9, #-265
	and	x8, x8, x9
	str	x8, [sp, #120]
	add	x2, sp, #96
	mov	w0, #0
	mov	w1, #2
	bl	_tcsetattr
	stp	xzr, xzr, [sp, #88]
	stp	xzr, xzr, [sp, #72]
LBB1_1:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB1_2 Depth 2
                                        ;       Child Loop BB1_24 Depth 3
                                        ;     Child Loop BB1_40 Depth 2
	str	xzr, [sp, #176]
	strb	wzr, [sp, #175]
	str	x24, [sp]
	mov	w0, #0
	mov	w1, #29800
	movk	w1, #16392, lsl #16
	bl	_ioctl
	mov	w0, #1
Lloh152:
	adrp	x1, l_.str.11@PAGE
Lloh153:
	add	x1, x1, l_.str.11@PAGEOFF
	mov	w2, #1
	bl	_write
	mov	x26, #0
	mov	x28, #0
	mov	x27, #0
	mov	x21, #0
	mov	x25, #0
	mov	w24, #0
	mov	w8, #0
	mov	w20, #0
	str	xzr, [sp, #64]                  ; 8-byte Folded Spill
	mov	x19, #0
LBB1_2:                                 ;   Parent Loop BB1_1 Depth=1
                                        ; =>  This Loop Header: Depth=2
                                        ;       Child Loop BB1_24 Depth 3
	mov	x22, x20
	mov	x20, x8
	add	x1, sp, #175
	mov	w0, #0
	mov	w2, #1
	bl	_read
	ldrb	w8, [sp, #175]
	cmp	w8, #127
	b.ne	LBB1_9
; %bb.3:                                ;   in Loop: Header=BB1_2 Depth=2
	cbz	x28, LBB1_15
; %bb.4:                                ;   in Loop: Header=BB1_2 Depth=2
	sub	x22, x28, #1
	ldrb	w8, [x19, x22]
	cmp	w8, #9
	b.eq	LBB1_21
; %bb.5:                                ;   in Loop: Header=BB1_2 Depth=2
	cmp	w8, #10
	b.ne	LBB1_22
; %bb.6:                                ;   in Loop: Header=BB1_2 Depth=2
	sub	x25, x25, #1
	ldrh	w24, [x26, x25, lsl #1]
Lloh154:
	adrp	x0, l_.str.13@PAGE
Lloh155:
	add	x0, x0, l_.str.13@PAGEOFF
	bl	_printf
	cbz	w24, LBB1_8
; %bb.7:                                ;   in Loop: Header=BB1_2 Depth=2
	str	x24, [sp]
Lloh156:
	adrp	x0, l_.str.14@PAGE
Lloh157:
	add	x0, x0, l_.str.14@PAGEOFF
	bl	_printf
LBB1_8:                                 ;   in Loop: Header=BB1_2 Depth=2
Lloh158:
	adrp	x8, ___stdoutp@GOTPAGE
Lloh159:
	ldr	x8, [x8, ___stdoutp@GOTPAGEOFF]
Lloh160:
	ldr	x0, [x8]
	bl	_fflush
	b	LBB1_30
LBB1_9:                                 ;   in Loop: Header=BB1_2 Depth=2
	cmp	w8, #116
	b.ne	LBB1_12
; %bb.10:                               ;   in Loop: Header=BB1_2 Depth=2
	cmp	w20, #114
	b.ne	LBB1_12
; %bb.11:                               ;   in Loop: Header=BB1_2 Depth=2
	cmp	w22, #100
	b.eq	LBB1_31
LBB1_12:                                ;   in Loop: Header=BB1_2 Depth=2
	cmp	w8, #9
	b.eq	LBB1_16
; %bb.13:                               ;   in Loop: Header=BB1_2 Depth=2
	cmp	w8, #10
	b.ne	LBB1_17
; %bb.14:                               ;   in Loop: Header=BB1_2 Depth=2
	add	x22, x25, #1
	lsl	x1, x22, #1
	mov	x0, x26
	bl	_realloc
	mov	x26, x0
	strh	w24, [x0, x25, lsl #1]
	add	x1, sp, #175
	mov	w0, #1
	mov	w2, #1
	bl	_write
	mov	w24, #0
	mov	x25, x22
	b	LBB1_18
LBB1_15:                                ;   in Loop: Header=BB1_2 Depth=2
	mov	x22, #0
	b	LBB1_30
LBB1_16:                                ;   in Loop: Header=BB1_2 Depth=2
	and	w8, w24, #0x7
	mov	w9, #8
	sub	w22, w9, w8
	add	w8, w24, w22
	and	w8, w8, #0xffff
	ldrh	w9, [sp, #178]
	udiv	w10, w8, w9
	msub	w24, w10, w9, w8
	mov	w0, #1
Lloh161:
	adrp	x1, l_.str.12@PAGE
Lloh162:
	add	x1, x1, l_.str.12@PAGEOFF
	mov	x2, x22
	bl	_write
	add	x23, x21, #1
	ldr	x0, [sp, #64]                   ; 8-byte Folded Reload
	mov	x1, x23
	bl	_realloc
	str	x0, [sp, #64]                   ; 8-byte Folded Spill
	strb	w22, [x0, x21]
	mov	x21, x23
	b	LBB1_18
LBB1_17:                                ;   in Loop: Header=BB1_2 Depth=2
	ldrh	w9, [sp, #178]
	cmp	w9, w24, uxth
	csel	w9, w24, wzr, hi
	and	w8, w8, #0xc0
	cmp	w8, #128
	cinc	w24, w9, ne
	add	x1, sp, #175
	mov	w0, #1
	mov	w2, #1
	bl	_write
LBB1_18:                                ;   in Loop: Header=BB1_2 Depth=2
	add	x22, x28, #1
	cmp	x22, x27
	b.lo	LBB1_20
; %bb.19:                               ;   in Loop: Header=BB1_2 Depth=2
	lsl	x8, x27, #2
	add	x27, x8, #4
	mov	x0, x19
	mov	x1, x27
	bl	_realloc
	mov	x19, x0
LBB1_20:                                ;   in Loop: Header=BB1_2 Depth=2
	ldrb	w8, [sp, #175]
	strb	w8, [x19, x28]
	b	LBB1_30
LBB1_21:                                ;   in Loop: Header=BB1_2 Depth=2
	sub	x21, x21, #1
	ldr	x8, [sp, #64]                   ; 8-byte Folded Reload
	ldrb	w2, [x8, x21]
	sub	w24, w24, w2
	mov	w0, #1
Lloh163:
	adrp	x1, l_.str.15@PAGE
Lloh164:
	add	x1, x1, l_.str.15@PAGEOFF
	b	LBB1_29
LBB1_22:                                ;   in Loop: Header=BB1_2 Depth=2
	and	w8, w8, #0xc0
	cmp	w8, #128
	b.ne	LBB1_26
; %bb.23:                               ;   in Loop: Header=BB1_2 Depth=2
	sub	x8, x19, #2
LBB1_24:                                ;   Parent Loop BB1_1 Depth=1
                                        ;     Parent Loop BB1_2 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	ldrb	w9, [x8, x28]
	and	w9, w9, #0xc0
	sub	x28, x28, #1
	cmp	w9, #128
	b.eq	LBB1_24
; %bb.25:                               ;   in Loop: Header=BB1_2 Depth=2
	sub	x22, x28, #1
LBB1_26:                                ;   in Loop: Header=BB1_2 Depth=2
	tst	w24, #0xffff
	b.eq	LBB1_28
; %bb.27:                               ;   in Loop: Header=BB1_2 Depth=2
	sub	w24, w24, #1
	mov	w0, #1
Lloh165:
	adrp	x1, l_.str.17@PAGE
Lloh166:
	add	x1, x1, l_.str.17@PAGEOFF
	mov	w2, #3
	b	LBB1_29
LBB1_28:                                ;   in Loop: Header=BB1_2 Depth=2
	ldrh	w8, [sp, #178]
	sub	w24, w8, #1
	mov	w0, #1
Lloh167:
	adrp	x1, l_.str.16@PAGE
Lloh168:
	add	x1, x1, l_.str.16@PAGEOFF
	mov	w2, #1
LBB1_29:                                ;   in Loop: Header=BB1_2 Depth=2
	bl	_write
LBB1_30:                                ;   in Loop: Header=BB1_2 Depth=2
	ldrb	w8, [sp, #175]
	mov	x28, x22
	b	LBB1_2
LBB1_31:                                ;   in Loop: Header=BB1_1 Depth=1
	sub	x8, x28, #2
	cmp	x28, #1
	csel	x26, x8, x28, hi
	strb	wzr, [x19, x26]
	stp	x26, x19, [sp]
Lloh169:
	adrp	x0, l_.str.2@PAGE
Lloh170:
	add	x0, x0, l_.str.2@PAGEOFF
	bl	_printf
	mov	x0, x19
Lloh171:
	adrp	x1, l_.str.3@PAGE
Lloh172:
	add	x1, x1, l_.str.3@PAGEOFF
	bl	_strcmp
	cbz	w0, LBB1_108
; %bb.32:                               ;   in Loop: Header=BB1_1 Depth=1
	mov	x0, x19
Lloh173:
	adrp	x1, l_.str.4@PAGE
Lloh174:
	add	x1, x1, l_.str.4@PAGEOFF
	bl	_strcmp
	cbz	w0, LBB1_108
; %bb.33:                               ;   in Loop: Header=BB1_1 Depth=1
	mov	x0, x19
Lloh175:
	adrp	x1, l_.str.5@PAGE
Lloh176:
	add	x1, x1, l_.str.5@PAGEOFF
	bl	_strcmp
	cbz	w0, LBB1_102
; %bb.34:                               ;   in Loop: Header=BB1_1 Depth=1
	mov	x0, x19
Lloh177:
	adrp	x1, l_.str.6@PAGE
Lloh178:
	add	x1, x1, l_.str.6@PAGEOFF
	bl	_strcmp
	cbz	w0, LBB1_102
; %bb.35:                               ;   in Loop: Header=BB1_1 Depth=1
	mov	x0, x19
Lloh179:
	adrp	x1, l_.str.8@PAGE
Lloh180:
	add	x1, x1, l_.str.8@PAGEOFF
	bl	_strcmp
	cbz	w0, LBB1_103
; %bb.36:                               ;   in Loop: Header=BB1_1 Depth=1
	mov	x0, x19
Lloh181:
	adrp	x1, l_.str.9@PAGE
Lloh182:
	add	x1, x1, l_.str.9@PAGEOFF
	bl	_strcmp
	add	x24, sp, #176
	cbz	w0, LBB1_104
; %bb.37:                               ;   in Loop: Header=BB1_1 Depth=1
	mov	x0, x19
Lloh183:
	adrp	x1, l_.str.10@PAGE
Lloh184:
	add	x1, x1, l_.str.10@PAGEOFF
	bl	_strcmp
	cbz	w0, LBB1_105
; %bb.38:                               ;   in Loop: Header=BB1_1 Depth=1
	add	x2, sp, #88
	add	x3, sp, #96
	add	x4, sp, #72
	add	x5, sp, #80
	mov	x0, x19
	mov	x1, x26
	bl	_parse
	ldp	x0, x28, [sp, #88]
	ldr	x19, [sp, #72]
	str	x0, [sp, #64]                   ; 8-byte Folded Spill
	mov	x1, x28
	mov	x2, x19
	bl	_print_instructions
	mov	w0, #65536
	bl	_malloc
	adrp	x8, _execute.variables@PAGE
	str	x0, [x8, _execute.variables@PAGEOFF]
Lloh185:
	adrp	x25, l_.str.85@PAGE
Lloh186:
	add	x25, x25, l_.str.85@PAGEOFF
	mov	w27, #152
Lloh187:
	adrp	x21, lJTI1_0@PAGE
Lloh188:
	add	x21, x21, lJTI1_0@PAGEOFF
	cbz	x28, LBB1_107
; %bb.39:                               ;   in Loop: Header=BB1_1 Depth=1
	mov	x23, #0
LBB1_40:                                ;   Parent Loop BB1_1 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	str	x23, [sp]
	mov	x0, x25
	bl	_printf
	ldr	x8, [sp, #64]                   ; 8-byte Folded Reload
	madd	x20, x23, x27, x8
	ldr	q0, [x20, #96]
	str	q0, [x24, #96]
	ldr	q0, [x20, #112]
	str	q0, [x24, #112]
	ldr	q0, [x20, #128]
	str	q0, [x24, #128]
	ldr	x8, [x20, #144]
	str	x8, [sp, #320]
	ldr	q0, [x20, #32]
	str	q0, [x24, #32]
	ldr	q0, [x20, #48]
	str	q0, [x24, #48]
	ldr	q0, [x20, #64]
	str	q0, [x24, #64]
	ldr	q0, [x20, #80]
	str	q0, [x24, #80]
	ldr	q0, [x20]
	str	q0, [sp, #176]
	ldr	q0, [x20, #16]
	str	q0, [sp, #192]
	add	x0, sp, #176
	mov	x1, x19
	bl	_print_instruction
	ldr	x9, [x20]
	sub	x8, x9, #1
	cmp	x8, #37
	b.hi	LBB1_112
; %bb.41:                               ;   in Loop: Header=BB1_40 Depth=2
	ldp	x26, x22, [x20, #8]
	ldr	x20, [x20, #24]
	adr	x9, LBB1_42
	ldrh	w10, [x21, x8, lsl #1]
	add	x9, x9, x10, lsl #2
	br	x9
LBB1_42:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh189:
	adrp	x0, l_.str.86@PAGE
Lloh190:
	add	x0, x0, l_.str.86@PAGEOFF
	bl	_printf
Lloh191:
	adrp	x8, _execute.variables@PAGE
Lloh192:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x10, [x8, x20, lsl #3]
	lsl	x9, x9, x10
	b	LBB1_84
LBB1_43:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh193:
	adrp	x0, l_.str.87@PAGE
Lloh194:
	add	x0, x0, l_.str.87@PAGEOFF
	b	LBB1_45
LBB1_44:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh195:
	adrp	x0, l_.str.88@PAGE
Lloh196:
	add	x0, x0, l_.str.88@PAGEOFF
LBB1_45:                                ;   in Loop: Header=BB1_40 Depth=2
	bl	_printf
Lloh197:
	adrp	x8, _execute.variables@PAGE
Lloh198:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x10, [x8, x20, lsl #3]
	lsr	x9, x9, x10
	b	LBB1_84
LBB1_46:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh199:
	adrp	x0, l_.str.89@PAGE
Lloh200:
	add	x0, x0, l_.str.89@PAGEOFF
	bl	_printf
Lloh201:
	adrp	x8, _execute.variables@PAGE
Lloh202:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x10, [x8, x20, lsl #3]
	add	x9, x10, x9
	b	LBB1_84
LBB1_47:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh203:
	adrp	x0, l_.str.90@PAGE
Lloh204:
	add	x0, x0, l_.str.90@PAGEOFF
	bl	_printf
Lloh205:
	adrp	x8, _execute.variables@PAGE
Lloh206:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x10, [x8, x20, lsl #3]
	eor	x9, x10, x9
	b	LBB1_84
LBB1_48:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh207:
	adrp	x0, l_.str.91@PAGE
Lloh208:
	add	x0, x0, l_.str.91@PAGEOFF
	bl	_printf
Lloh209:
	adrp	x8, _execute.variables@PAGE
Lloh210:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x10, [x8, x20, lsl #3]
	and	x9, x10, x9
	b	LBB1_84
LBB1_49:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh211:
	adrp	x0, l_.str.92@PAGE
Lloh212:
	add	x0, x0, l_.str.92@PAGEOFF
	bl	_printf
Lloh213:
	adrp	x8, _execute.variables@PAGE
Lloh214:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x10, [x8, x20, lsl #3]
	orr	x9, x10, x9
	b	LBB1_84
LBB1_50:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh215:
	adrp	x0, l_.str.93@PAGE
Lloh216:
	add	x0, x0, l_.str.93@PAGEOFF
	bl	_printf
Lloh217:
	adrp	x8, _execute.variables@PAGE
Lloh218:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x10, [x8, x20, lsl #3]
	sub	x9, x9, x10
	b	LBB1_84
LBB1_51:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh219:
	adrp	x0, l_.str.94@PAGE
Lloh220:
	add	x0, x0, l_.str.94@PAGEOFF
	bl	_printf
Lloh221:
	adrp	x8, _execute.variables@PAGE
Lloh222:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x10, [x8, x20, lsl #3]
	mul	x9, x10, x9
	b	LBB1_84
LBB1_52:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh223:
	adrp	x0, l_.str.97@PAGE
Lloh224:
	add	x0, x0, l_.str.97@PAGEOFF
	b	LBB1_55
LBB1_53:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh225:
	adrp	x0, l_.str.98@PAGE
Lloh226:
	add	x0, x0, l_.str.98@PAGEOFF
	b	LBB1_57
LBB1_54:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh227:
	adrp	x0, l_.str.99@PAGE
Lloh228:
	add	x0, x0, l_.str.99@PAGEOFF
LBB1_55:                                ;   in Loop: Header=BB1_40 Depth=2
	bl	_printf
Lloh229:
	adrp	x8, _execute.variables@PAGE
Lloh230:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x10, [x8, x20, lsl #3]
	udiv	x9, x9, x10
	b	LBB1_84
LBB1_56:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh231:
	adrp	x0, l_.str.100@PAGE
Lloh232:
	add	x0, x0, l_.str.100@PAGEOFF
LBB1_57:                                ;   in Loop: Header=BB1_40 Depth=2
	bl	_printf
Lloh233:
	adrp	x8, _execute.variables@PAGE
Lloh234:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x10, [x8, x20, lsl #3]
	udiv	x11, x9, x10
	msub	x9, x11, x10, x9
	b	LBB1_84
LBB1_58:                                ;   in Loop: Header=BB1_40 Depth=2
	mov	w8, #40
	madd	x21, x26, x8, x19
	ldr	x8, [x21, #24]
	stp	x22, x20, [sp, #8]
	str	x8, [sp]
Lloh235:
	adrp	x0, l_.str.101@PAGE
Lloh236:
	add	x0, x0, l_.str.101@PAGEOFF
	b	LBB1_61
LBB1_59:                                ;   in Loop: Header=BB1_40 Depth=2
	mov	w8, #40
	madd	x21, x26, x8, x19
	ldr	x8, [x21, #24]
	stp	x22, x20, [sp, #8]
	str	x8, [sp]
Lloh237:
	adrp	x0, l_.str.103@PAGE
Lloh238:
	add	x0, x0, l_.str.103@PAGEOFF
	b	LBB1_64
LBB1_60:                                ;   in Loop: Header=BB1_40 Depth=2
	mov	w8, #40
	madd	x21, x26, x8, x19
	ldr	x8, [x21, #24]
	stp	x22, x20, [sp, #8]
	str	x8, [sp]
Lloh239:
	adrp	x0, l_.str.104@PAGE
Lloh240:
	add	x0, x0, l_.str.104@PAGEOFF
LBB1_61:                                ;   in Loop: Header=BB1_40 Depth=2
	bl	_printf
	ldr	x8, [x21, #24]
Lloh241:
	adrp	x21, lJTI1_0@PAGE
Lloh242:
	add	x21, x21, lJTI1_0@PAGEOFF
	cmn	x8, #1
	b.eq	LBB1_106
; %bb.62:                               ;   in Loop: Header=BB1_40 Depth=2
Lloh243:
	adrp	x9, _execute.variables@PAGE
Lloh244:
	add	x9, x9, _execute.variables@PAGEOFF
	ldr	x10, [x9, x22, lsl #3]
	ldr	x9, [x9, x20, lsl #3]
	sub	x8, x8, #1
	cmp	x10, x9
	csel	x23, x8, x23, lo
	b	LBB1_85
LBB1_63:                                ;   in Loop: Header=BB1_40 Depth=2
	mov	w8, #40
	madd	x21, x26, x8, x19
	ldr	x8, [x21, #24]
	stp	x22, x20, [sp, #8]
	str	x8, [sp]
Lloh245:
	adrp	x0, l_.str.105@PAGE
Lloh246:
	add	x0, x0, l_.str.105@PAGEOFF
LBB1_64:                                ;   in Loop: Header=BB1_40 Depth=2
	bl	_printf
	ldr	x8, [x21, #24]
Lloh247:
	adrp	x21, lJTI1_0@PAGE
Lloh248:
	add	x21, x21, lJTI1_0@PAGEOFF
	cmn	x8, #1
	b.eq	LBB1_106
; %bb.65:                               ;   in Loop: Header=BB1_40 Depth=2
Lloh249:
	adrp	x9, _execute.variables@PAGE
Lloh250:
	add	x9, x9, _execute.variables@PAGEOFF
	ldr	x10, [x9, x22, lsl #3]
	ldr	x9, [x9, x20, lsl #3]
	sub	x8, x8, #1
	cmp	x10, x9
	csel	x23, x23, x8, lo
	b	LBB1_85
LBB1_66:                                ;   in Loop: Header=BB1_40 Depth=2
	mov	w8, #40
	madd	x21, x26, x8, x19
	ldr	x8, [x21, #24]
	stp	x22, x20, [sp, #8]
	str	x8, [sp]
Lloh251:
	adrp	x0, l_.str.107@PAGE
Lloh252:
	add	x0, x0, l_.str.107@PAGEOFF
	bl	_printf
	ldr	x8, [x21, #24]
Lloh253:
	adrp	x21, lJTI1_0@PAGE
Lloh254:
	add	x21, x21, lJTI1_0@PAGEOFF
	cmn	x8, #1
	b.eq	LBB1_106
; %bb.67:                               ;   in Loop: Header=BB1_40 Depth=2
Lloh255:
	adrp	x9, _execute.variables@PAGE
Lloh256:
	add	x9, x9, _execute.variables@PAGEOFF
	ldr	x10, [x9, x22, lsl #3]
	ldr	x9, [x9, x20, lsl #3]
	sub	x8, x8, #1
	cmp	x10, x9
	csel	x23, x8, x23, eq
	b	LBB1_85
LBB1_68:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x26, x22, [sp]
Lloh257:
	adrp	x0, l_.str.109@PAGE
Lloh258:
	add	x0, x0, l_.str.109@PAGEOFF
	bl	_printf
Lloh259:
	adrp	x8, _execute.variables@PAGE
Lloh260:
	add	x8, x8, _execute.variables@PAGEOFF
	str	x23, [x8, x26, lsl #3]
	ldr	x23, [x8, x22, lsl #3]
	b	LBB1_85
LBB1_69:                                ;   in Loop: Header=BB1_40 Depth=2
	mov	w20, #40
	madd	x8, x22, x20, x19
	ldr	x8, [x8, #24]
	stp	x26, x8, [sp]
Lloh261:
	adrp	x0, l_.str.108@PAGE
Lloh262:
	add	x0, x0, l_.str.108@PAGEOFF
	bl	_printf
	madd	x8, x26, x20, x19
	ldr	x8, [x8, #24]
	cmn	x8, #1
	b.eq	LBB1_106
; %bb.70:                               ;   in Loop: Header=BB1_40 Depth=2
Lloh263:
	adrp	x9, _execute.variables@PAGE
Lloh264:
	add	x9, x9, _execute.variables@PAGEOFF
	str	x23, [x9, x22, lsl #3]
	sub	x23, x8, #1
	b	LBB1_85
LBB1_71:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x26, x22, [sp]
Lloh265:
	adrp	x0, l_.str.110@PAGE
Lloh266:
	add	x0, x0, l_.str.110@PAGEOFF
	bl	_printf
Lloh267:
	adrp	x8, _execute.variables@PAGE
Lloh268:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x8, [x8, x26, lsl #3]
	strb	w9, [x8]
	b	LBB1_85
LBB1_72:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x26, x22, [sp]
Lloh269:
	adrp	x0, l_.str.111@PAGE
Lloh270:
	add	x0, x0, l_.str.111@PAGEOFF
	bl	_printf
Lloh271:
	adrp	x8, _execute.variables@PAGE
Lloh272:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x8, [x8, x26, lsl #3]
	strh	w9, [x8]
	b	LBB1_85
LBB1_73:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x26, x22, [sp]
Lloh273:
	adrp	x0, l_.str.112@PAGE
Lloh274:
	add	x0, x0, l_.str.112@PAGEOFF
	bl	_printf
Lloh275:
	adrp	x8, _execute.variables@PAGE
Lloh276:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x8, [x8, x26, lsl #3]
	str	w9, [x8]
	b	LBB1_85
LBB1_74:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x26, x22, [sp]
Lloh277:
	adrp	x0, l_.str.114@PAGE
Lloh278:
	add	x0, x0, l_.str.114@PAGEOFF
	bl	_printf
Lloh279:
	adrp	x8, _execute.variables@PAGE
Lloh280:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldrb	w9, [x9]
	b	LBB1_84
LBB1_75:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x26, x22, [sp]
Lloh281:
	adrp	x0, l_.str.115@PAGE
Lloh282:
	add	x0, x0, l_.str.115@PAGEOFF
	bl	_printf
Lloh283:
	adrp	x8, _execute.variables@PAGE
Lloh284:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldrh	w9, [x9]
	b	LBB1_84
LBB1_76:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x26, x22, [sp]
Lloh285:
	adrp	x0, l_.str.118@PAGE
Lloh286:
	add	x0, x0, l_.str.118@PAGEOFF
	bl	_printf
Lloh287:
	adrp	x8, _execute.variables@PAGE
Lloh288:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldrsb	x9, [x9]
	b	LBB1_84
LBB1_77:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x26, x22, [sp]
Lloh289:
	adrp	x0, l_.str.119@PAGE
Lloh290:
	add	x0, x0, l_.str.119@PAGEOFF
	bl	_printf
Lloh291:
	adrp	x8, _execute.variables@PAGE
Lloh292:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldrsh	x9, [x9]
	b	LBB1_84
LBB1_78:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x26, x22, [sp]
Lloh293:
	adrp	x0, l_.str.120@PAGE
Lloh294:
	add	x0, x0, l_.str.120@PAGEOFF
	bl	_printf
Lloh295:
	adrp	x8, _execute.variables@PAGE
Lloh296:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldrsw	x9, [x9]
	b	LBB1_84
LBB1_79:                                ;   in Loop: Header=BB1_40 Depth=2
	mov	w8, #40
	madd	x21, x26, x8, x19
	ldr	x8, [x21, #24]
	stp	x22, x20, [sp, #8]
	str	x8, [sp]
Lloh297:
	adrp	x0, l_.str.106@PAGE
Lloh298:
	add	x0, x0, l_.str.106@PAGEOFF
	bl	_printf
	ldr	x8, [x21, #24]
Lloh299:
	adrp	x21, lJTI1_0@PAGE
Lloh300:
	add	x21, x21, lJTI1_0@PAGEOFF
	cmn	x8, #1
	b.eq	LBB1_106
; %bb.80:                               ;   in Loop: Header=BB1_40 Depth=2
Lloh301:
	adrp	x9, _execute.variables@PAGE
Lloh302:
	add	x9, x9, _execute.variables@PAGEOFF
	ldr	x10, [x9, x22, lsl #3]
	ldr	x9, [x9, x20, lsl #3]
	sub	x8, x8, #1
	cmp	x10, x9
	csel	x23, x23, x8, eq
	b	LBB1_85
LBB1_81:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x26, x22, [sp]
Lloh303:
	adrp	x0, l_.str.113@PAGE
Lloh304:
	add	x0, x0, l_.str.113@PAGEOFF
	bl	_printf
Lloh305:
	adrp	x8, _execute.variables@PAGE
Lloh306:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x8, [x8, x26, lsl #3]
	str	x9, [x8]
	b	LBB1_85
LBB1_82:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x26, x22, [sp]
Lloh307:
	adrp	x0, l_.str.116@PAGE
Lloh308:
	add	x0, x0, l_.str.116@PAGEOFF
	bl	_printf
Lloh309:
	adrp	x8, _execute.variables@PAGE
Lloh310:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	w9, [x9]
	b	LBB1_84
LBB1_83:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x26, x22, [sp]
Lloh311:
	adrp	x0, l_.str.117@PAGE
Lloh312:
	add	x0, x0, l_.str.117@PAGEOFF
	bl	_printf
Lloh313:
	adrp	x8, _execute.variables@PAGE
Lloh314:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x9, [x9]
LBB1_84:                                ;   in Loop: Header=BB1_40 Depth=2
	str	x9, [x8, x26, lsl #3]
LBB1_85:                                ;   in Loop: Header=BB1_40 Depth=2
	add	x23, x23, #1
	cmp	x23, x28
	b.lo	LBB1_40
	b	LBB1_107
LBB1_86:                                ;   in Loop: Header=BB1_40 Depth=2
	stp	x26, x22, [sp]
Lloh315:
	adrp	x0, l_.str.121@PAGE
Lloh316:
	add	x0, x0, l_.str.121@PAGEOFF
	bl	_printf
	mov	w8, #40
	madd	x8, x22, x8, x19
	ldr	x9, [x8, #8]
	str	x9, [sp, #176]
	ldr	x0, [x8]
	add	x1, sp, #176
	bl	_string_to_number
	mov	x22, x0
	ldr	x8, [sp, #176]
	stp	x0, x8, [sp]
Lloh317:
	adrp	x0, l_.str.122@PAGE
Lloh318:
	add	x0, x0, l_.str.122@PAGEOFF
	bl	_printf
Lloh319:
	adrp	x8, _execute.variables@PAGE
Lloh320:
	add	x8, x8, _execute.variables@PAGEOFF
	str	x22, [x8, x26, lsl #3]
	b	LBB1_85
LBB1_87:                                ;   in Loop: Header=BB1_40 Depth=2
	mov	w8, #152
	ldr	x9, [sp, #64]                   ; 8-byte Folded Reload
	madd	x8, x23, x8, x9
	ldp	x25, x24, [x8, #48]
	ldp	x27, x21, [x8, #32]
	stp	x25, x24, [sp, #40]
	stp	x27, x21, [sp, #24]
	stp	x22, x20, [sp, #8]
Lloh321:
	adrp	x0, l_.str.123@PAGE
Lloh322:
	add	x0, x0, l_.str.123@PAGEOFF
	str	x26, [sp]
	bl	_printf
Lloh323:
	adrp	x8, _execute.variables@PAGE
Lloh324:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x26, lsl #3]
	ldr	x10, [x8, x22, lsl #3]
	ldr	x11, [x8, x20, lsl #3]
	ldr	x12, [x8, x27, lsl #3]
	mov	x27, x11
	ldr	x22, [x8, x21, lsl #3]
	mov	x21, x9
	ldr	x8, [x8, x25, lsl #3]
	mov	x25, x10
	stp	x8, x8, [sp, #48]               ; 8-byte Folded Spill
	stp	x12, x22, [sp, #32]
	mov	x20, x12
	stp	x10, x11, [sp, #16]
Lloh325:
	adrp	x0, l_.str.130@PAGE
Lloh326:
	add	x0, x0, l_.str.130@PAGEOFF
	stp	x24, x9, [sp]
	bl	_printf
	sub	x8, x24, #1
	cmp	x8, #7
	b.hi	LBB1_93
; %bb.88:                               ;   in Loop: Header=BB1_40 Depth=2
Lloh327:
	adrp	x9, lJTI1_1@PAGE
Lloh328:
	add	x9, x9, lJTI1_1@PAGEOFF
	adr	x10, LBB1_89
	ldrb	w11, [x9, x8]
	add	x10, x10, x11, lsl #2
	br	x10
LBB1_89:                                ;   in Loop: Header=BB1_40 Depth=2
	bl	_fork
	b	LBB1_100
LBB1_90:                                ;   in Loop: Header=BB1_40 Depth=2
	str	x26, [sp]
Lloh329:
	adrp	x0, l_.str.124@PAGE
Lloh330:
	add	x0, x0, l_.str.124@PAGEOFF
	bl	_printf
Lloh331:
	adrp	x8, _execute.variables@PAGE
Lloh332:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x8, [x8, x26, lsl #3]
	str	x8, [sp]
Lloh333:
	adrp	x0, l_.str.125@PAGE
Lloh334:
	add	x0, x0, l_.str.125@PAGEOFF
	b	LBB1_92
LBB1_91:                                ;   in Loop: Header=BB1_40 Depth=2
	str	x26, [sp]
Lloh335:
	adrp	x0, l_.str.126@PAGE
Lloh336:
	add	x0, x0, l_.str.126@PAGEOFF
	bl	_printf
Lloh337:
	adrp	x8, _execute.variables@PAGE
Lloh338:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x8, [x8, x26, lsl #3]
	str	x8, [sp]
Lloh339:
	adrp	x0, l_.str.127@PAGE
Lloh340:
	add	x0, x0, l_.str.127@PAGEOFF
LBB1_92:                                ;   in Loop: Header=BB1_40 Depth=2
	bl	_printf
	b	LBB1_85
LBB1_93:                                ;   in Loop: Header=BB1_40 Depth=2
	add	x24, sp, #176
Lloh341:
	adrp	x25, l_.str.85@PAGE
Lloh342:
	add	x25, x25, l_.str.85@PAGEOFF
	mov	w27, #152
Lloh343:
	adrp	x21, lJTI1_0@PAGE
Lloh344:
	add	x21, x21, lJTI1_0@PAGEOFF
	b	LBB1_85
LBB1_94:                                ;   in Loop: Header=BB1_40 Depth=2
	mov	x0, x21
	mov	x1, x25
	mov	x2, x27
	bl	_read
	b	LBB1_101
LBB1_95:                                ;   in Loop: Header=BB1_40 Depth=2
	mov	x0, x21
	mov	x1, x25
	mov	x2, x27
	bl	_write
	b	LBB1_101
LBB1_96:                                ;   in Loop: Header=BB1_40 Depth=2
	str	x27, [sp]
	mov	x0, x21
	mov	x1, x25
	bl	_open
	b	LBB1_100
LBB1_97:                                ;   in Loop: Header=BB1_40 Depth=2
	mov	x0, x21
	bl	_close
	b	LBB1_100
LBB1_98:                                ;   in Loop: Header=BB1_40 Depth=2
	mov	x0, x21
	mov	x1, x25
	mov	x2, x27
	mov	x3, x20
	mov	x4, x22
	ldr	x5, [sp, #56]                   ; 8-byte Folded Reload
	bl	_mmap
	b	LBB1_101
LBB1_99:                                ;   in Loop: Header=BB1_40 Depth=2
	mov	x0, x21
	mov	x1, x25
	bl	_munmap
LBB1_100:                               ;   in Loop: Header=BB1_40 Depth=2
                                        ; kill: def $w0 killed $w0 def $x0
	sxtw	x0, w0
LBB1_101:                               ;   in Loop: Header=BB1_40 Depth=2
	add	x24, sp, #176
Lloh345:
	adrp	x25, l_.str.85@PAGE
Lloh346:
	add	x25, x25, l_.str.85@PAGEOFF
	mov	w27, #152
Lloh347:
	adrp	x21, lJTI1_0@PAGE
Lloh348:
	add	x21, x21, lJTI1_0@PAGEOFF
Lloh349:
	adrp	x8, _execute.variables@PAGE
Lloh350:
	add	x8, x8, _execute.variables@PAGEOFF
	str	x0, [x8, x26, lsl #3]
	b	LBB1_85
LBB1_102:                               ;   in Loop: Header=BB1_1 Depth=1
Lloh351:
	adrp	x0, l_.str.7@PAGE
Lloh352:
	add	x0, x0, l_.str.7@PAGEOFF
	bl	_printf
	add	x24, sp, #176
	b	LBB1_1
LBB1_103:                               ;   in Loop: Header=BB1_1 Depth=1
	ldp	x0, x1, [sp, #72]
	bl	_print_dictionary
	add	x24, sp, #176
	b	LBB1_1
LBB1_104:                               ;   in Loop: Header=BB1_1 Depth=1
	ldp	x0, x1, [sp, #88]
	ldr	x2, [sp, #72]
	bl	_print_instructions
	b	LBB1_1
LBB1_105:                               ;   in Loop: Header=BB1_1 Depth=1
	str	xzr, [sp, #80]
	str	xzr, [sp, #96]
	b	LBB1_1
LBB1_106:                               ;   in Loop: Header=BB1_1 Depth=1
Lloh353:
	adrp	x0, l_.str.102@PAGE
Lloh354:
	add	x0, x0, l_.str.102@PAGEOFF
	bl	_puts
LBB1_107:                               ;   in Loop: Header=BB1_1 Depth=1
Lloh355:
	adrp	x0, l_.str.129@PAGE
Lloh356:
	add	x0, x0, l_.str.129@PAGEOFF
	bl	_puts
	b	LBB1_1
LBB1_108:
	mov	w0, #0
	bl	_exit
LBB1_109:
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh357:
	adrp	x0, l_.str.95@PAGE
Lloh358:
	add	x0, x0, l_.str.95@PAGEOFF
	b	LBB1_111
LBB1_110:
	stp	x22, x20, [sp, #8]
	str	x26, [sp]
Lloh359:
	adrp	x0, l_.str.96@PAGE
Lloh360:
	add	x0, x0, l_.str.96@PAGEOFF
LBB1_111:
	bl	_printf
Lloh361:
	adrp	x8, _execute.variables@PAGE
Lloh362:
	add	x8, x8, _execute.variables@PAGEOFF
	ldr	x9, [x8, x22, lsl #3]
	ldr	x10, [x8, x20, lsl #3]
	mul	x9, x10, x9
	str	x9, [x8, x26, lsl #3]
	bl	_abort
LBB1_112:
	str	x9, [sp]
Lloh363:
	adrp	x0, l_.str.128@PAGE
Lloh364:
	add	x0, x0, l_.str.128@PAGEOFF
	bl	_printf
	bl	_abort
LBB1_113:
	mov	x0, x21
	bl	_exit
	.loh AdrpAdd	Lloh150, Lloh151
	.loh AdrpAdd	Lloh152, Lloh153
	.loh AdrpAdd	Lloh154, Lloh155
	.loh AdrpAdd	Lloh156, Lloh157
	.loh AdrpLdrGotLdr	Lloh158, Lloh159, Lloh160
	.loh AdrpAdd	Lloh161, Lloh162
	.loh AdrpAdd	Lloh163, Lloh164
	.loh AdrpAdd	Lloh165, Lloh166
	.loh AdrpAdd	Lloh167, Lloh168
	.loh AdrpAdd	Lloh171, Lloh172
	.loh AdrpAdd	Lloh169, Lloh170
	.loh AdrpAdd	Lloh173, Lloh174
	.loh AdrpAdd	Lloh175, Lloh176
	.loh AdrpAdd	Lloh177, Lloh178
	.loh AdrpAdd	Lloh179, Lloh180
	.loh AdrpAdd	Lloh181, Lloh182
	.loh AdrpAdd	Lloh183, Lloh184
	.loh AdrpAdd	Lloh187, Lloh188
	.loh AdrpAdd	Lloh185, Lloh186
	.loh AdrpAdd	Lloh191, Lloh192
	.loh AdrpAdd	Lloh189, Lloh190
	.loh AdrpAdd	Lloh193, Lloh194
	.loh AdrpAdd	Lloh195, Lloh196
	.loh AdrpAdd	Lloh197, Lloh198
	.loh AdrpAdd	Lloh201, Lloh202
	.loh AdrpAdd	Lloh199, Lloh200
	.loh AdrpAdd	Lloh205, Lloh206
	.loh AdrpAdd	Lloh203, Lloh204
	.loh AdrpAdd	Lloh209, Lloh210
	.loh AdrpAdd	Lloh207, Lloh208
	.loh AdrpAdd	Lloh213, Lloh214
	.loh AdrpAdd	Lloh211, Lloh212
	.loh AdrpAdd	Lloh217, Lloh218
	.loh AdrpAdd	Lloh215, Lloh216
	.loh AdrpAdd	Lloh221, Lloh222
	.loh AdrpAdd	Lloh219, Lloh220
	.loh AdrpAdd	Lloh223, Lloh224
	.loh AdrpAdd	Lloh225, Lloh226
	.loh AdrpAdd	Lloh227, Lloh228
	.loh AdrpAdd	Lloh229, Lloh230
	.loh AdrpAdd	Lloh231, Lloh232
	.loh AdrpAdd	Lloh233, Lloh234
	.loh AdrpAdd	Lloh235, Lloh236
	.loh AdrpAdd	Lloh237, Lloh238
	.loh AdrpAdd	Lloh239, Lloh240
	.loh AdrpAdd	Lloh241, Lloh242
	.loh AdrpAdd	Lloh243, Lloh244
	.loh AdrpAdd	Lloh245, Lloh246
	.loh AdrpAdd	Lloh247, Lloh248
	.loh AdrpAdd	Lloh249, Lloh250
	.loh AdrpAdd	Lloh253, Lloh254
	.loh AdrpAdd	Lloh251, Lloh252
	.loh AdrpAdd	Lloh255, Lloh256
	.loh AdrpAdd	Lloh259, Lloh260
	.loh AdrpAdd	Lloh257, Lloh258
	.loh AdrpAdd	Lloh261, Lloh262
	.loh AdrpAdd	Lloh263, Lloh264
	.loh AdrpAdd	Lloh267, Lloh268
	.loh AdrpAdd	Lloh265, Lloh266
	.loh AdrpAdd	Lloh271, Lloh272
	.loh AdrpAdd	Lloh269, Lloh270
	.loh AdrpAdd	Lloh275, Lloh276
	.loh AdrpAdd	Lloh273, Lloh274
	.loh AdrpAdd	Lloh279, Lloh280
	.loh AdrpAdd	Lloh277, Lloh278
	.loh AdrpAdd	Lloh283, Lloh284
	.loh AdrpAdd	Lloh281, Lloh282
	.loh AdrpAdd	Lloh287, Lloh288
	.loh AdrpAdd	Lloh285, Lloh286
	.loh AdrpAdd	Lloh291, Lloh292
	.loh AdrpAdd	Lloh289, Lloh290
	.loh AdrpAdd	Lloh295, Lloh296
	.loh AdrpAdd	Lloh293, Lloh294
	.loh AdrpAdd	Lloh299, Lloh300
	.loh AdrpAdd	Lloh297, Lloh298
	.loh AdrpAdd	Lloh301, Lloh302
	.loh AdrpAdd	Lloh305, Lloh306
	.loh AdrpAdd	Lloh303, Lloh304
	.loh AdrpAdd	Lloh309, Lloh310
	.loh AdrpAdd	Lloh307, Lloh308
	.loh AdrpAdd	Lloh313, Lloh314
	.loh AdrpAdd	Lloh311, Lloh312
	.loh AdrpAdd	Lloh319, Lloh320
	.loh AdrpAdd	Lloh317, Lloh318
	.loh AdrpAdd	Lloh315, Lloh316
	.loh AdrpAdd	Lloh325, Lloh326
	.loh AdrpAdd	Lloh323, Lloh324
	.loh AdrpAdd	Lloh321, Lloh322
	.loh AdrpAdd	Lloh327, Lloh328
	.loh AdrpAdd	Lloh333, Lloh334
	.loh AdrpAdd	Lloh331, Lloh332
	.loh AdrpAdd	Lloh329, Lloh330
	.loh AdrpAdd	Lloh339, Lloh340
	.loh AdrpAdd	Lloh337, Lloh338
	.loh AdrpAdd	Lloh335, Lloh336
	.loh AdrpAdd	Lloh343, Lloh344
	.loh AdrpAdd	Lloh341, Lloh342
	.loh AdrpAdd	Lloh349, Lloh350
	.loh AdrpAdd	Lloh347, Lloh348
	.loh AdrpAdd	Lloh345, Lloh346
	.loh AdrpAdd	Lloh351, Lloh352
	.loh AdrpAdd	Lloh353, Lloh354
	.loh AdrpAdd	Lloh355, Lloh356
	.loh AdrpAdd	Lloh357, Lloh358
	.loh AdrpAdd	Lloh359, Lloh360
	.loh AdrpAdd	Lloh361, Lloh362
	.loh AdrpAdd	Lloh363, Lloh364
	.cfi_endproc
	.section	__TEXT,__const
	.p2align	1
lJTI1_0:
	.short	(LBB1_42-LBB1_42)>>2
	.short	(LBB1_43-LBB1_42)>>2
	.short	(LBB1_44-LBB1_42)>>2
	.short	(LBB1_46-LBB1_42)>>2
	.short	(LBB1_47-LBB1_42)>>2
	.short	(LBB1_48-LBB1_42)>>2
	.short	(LBB1_49-LBB1_42)>>2
	.short	(LBB1_50-LBB1_42)>>2
	.short	(LBB1_51-LBB1_42)>>2
	.short	(LBB1_109-LBB1_42)>>2
	.short	(LBB1_110-LBB1_42)>>2
	.short	(LBB1_52-LBB1_42)>>2
	.short	(LBB1_53-LBB1_42)>>2
	.short	(LBB1_54-LBB1_42)>>2
	.short	(LBB1_56-LBB1_42)>>2
	.short	(LBB1_58-LBB1_42)>>2
	.short	(LBB1_59-LBB1_42)>>2
	.short	(LBB1_60-LBB1_42)>>2
	.short	(LBB1_63-LBB1_42)>>2
	.short	(LBB1_79-LBB1_42)>>2
	.short	(LBB1_66-LBB1_42)>>2
	.short	(LBB1_68-LBB1_42)>>2
	.short	(LBB1_69-LBB1_42)>>2
	.short	(LBB1_71-LBB1_42)>>2
	.short	(LBB1_72-LBB1_42)>>2
	.short	(LBB1_73-LBB1_42)>>2
	.short	(LBB1_81-LBB1_42)>>2
	.short	(LBB1_74-LBB1_42)>>2
	.short	(LBB1_75-LBB1_42)>>2
	.short	(LBB1_82-LBB1_42)>>2
	.short	(LBB1_83-LBB1_42)>>2
	.short	(LBB1_76-LBB1_42)>>2
	.short	(LBB1_77-LBB1_42)>>2
	.short	(LBB1_78-LBB1_42)>>2
	.short	(LBB1_86-LBB1_42)>>2
	.short	(LBB1_87-LBB1_42)>>2
	.short	(LBB1_90-LBB1_42)>>2
	.short	(LBB1_91-LBB1_42)>>2
lJTI1_1:
	.byte	(LBB1_113-LBB1_89)>>2
	.byte	(LBB1_89-LBB1_89)>>2
	.byte	(LBB1_94-LBB1_89)>>2
	.byte	(LBB1_95-LBB1_89)>>2
	.byte	(LBB1_96-LBB1_89)>>2
	.byte	(LBB1_97-LBB1_89)>>2
	.byte	(LBB1_98-LBB1_89)>>2
	.byte	(LBB1_99-LBB1_89)>>2
                                        ; -- End function
	.section	__TEXT,__text,regular,pure_instructions
	.p2align	2                               ; -- Begin function print_dictionary
_print_dictionary:                      ; @print_dictionary
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #112
	.cfi_def_cfa_offset 112
	stp	x22, x21, [sp, #64]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #80]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #96]             ; 16-byte Folded Spill
	add	x29, sp, #96
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	mov	x19, x1
	mov	x20, x0
Lloh365:
	adrp	x0, l_str@PAGE
Lloh366:
	add	x0, x0, l_str@PAGEOFF
	bl	_puts
	cbz	x19, LBB2_3
; %bb.1:
	mov	x22, #0
Lloh367:
	adrp	x21, l_.str.19@PAGE
Lloh368:
	add	x21, x21, l_.str.19@PAGEOFF
LBB2_2:                                 ; =>This Inner Loop Header: Depth=1
	str	x22, [sp]
	mov	x0, x21
	bl	_printf
	ldp	q0, q1, [x20]
	stp	q0, q1, [sp, #16]
	ldr	x8, [x20, #32]
	str	x8, [sp, #48]
	add	x0, sp, #16
	bl	_print_word
	add	x22, x22, #1
	add	x20, x20, #40
	cmp	x19, x22
	b.ne	LBB2_2
LBB2_3:
Lloh369:
	adrp	x0, l_str.194@PAGE
Lloh370:
	add	x0, x0, l_str.194@PAGEOFF
	ldp	x29, x30, [sp, #96]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #80]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #64]             ; 16-byte Folded Reload
	add	sp, sp, #112
	b	_puts
	.loh AdrpAdd	Lloh365, Lloh366
	.loh AdrpAdd	Lloh367, Lloh368
	.loh AdrpAdd	Lloh369, Lloh370
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function print_instructions
_print_instructions:                    ; @print_instructions
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #240
	.cfi_def_cfa_offset 240
	stp	x24, x23, [sp, #176]            ; 16-byte Folded Spill
	stp	x22, x21, [sp, #192]            ; 16-byte Folded Spill
	stp	x20, x19, [sp, #208]            ; 16-byte Folded Spill
	stp	x29, x30, [sp, #224]            ; 16-byte Folded Spill
	add	x29, sp, #224
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	mov	x19, x2
	mov	x20, x1
	mov	x21, x0
Lloh371:
	adrp	x0, l_str.192@PAGE
Lloh372:
	add	x0, x0, l_str.192@PAGEOFF
	bl	_puts
	cbz	x20, LBB3_3
; %bb.1:
	mov	x23, #0
Lloh373:
	adrp	x22, l_.str.29@PAGE
Lloh374:
	add	x22, x22, l_.str.29@PAGEOFF
LBB3_2:                                 ; =>This Inner Loop Header: Depth=1
	str	x23, [sp]
	mov	x0, x22
	bl	_printf
	ldp	q0, q1, [x21, #96]
	stp	q0, q1, [sp, #112]
	ldr	q0, [x21, #128]
	str	q0, [sp, #144]
	ldr	x8, [x21, #144]
	str	x8, [sp, #160]
	ldp	q0, q1, [x21, #32]
	stp	q0, q1, [sp, #48]
	ldp	q0, q1, [x21, #64]
	stp	q0, q1, [sp, #80]
	ldp	q0, q1, [x21]
	stp	q0, q1, [sp, #16]
	add	x0, sp, #16
	mov	x1, x19
	bl	_print_instruction
	add	x23, x23, #1
	add	x21, x21, #152
	cmp	x20, x23
	b.ne	LBB3_2
LBB3_3:
Lloh375:
	adrp	x0, l_str.194@PAGE
Lloh376:
	add	x0, x0, l_str.194@PAGEOFF
	ldp	x29, x30, [sp, #224]            ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #208]            ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #192]            ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #176]            ; 16-byte Folded Reload
	add	sp, sp, #240
	b	_puts
	.loh AdrpAdd	Lloh371, Lloh372
	.loh AdrpAdd	Lloh373, Lloh374
	.loh AdrpAdd	Lloh375, Lloh376
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function parse
_parse:                                 ; @parse
	.cfi_startproc
; %bb.0:
	stp	x28, x27, [sp, #-96]!           ; 16-byte Folded Spill
	.cfi_def_cfa_offset 96
	stp	x26, x25, [sp, #16]             ; 16-byte Folded Spill
	stp	x24, x23, [sp, #32]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #48]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #64]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #80]             ; 16-byte Folded Spill
	add	x29, sp, #80
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	.cfi_offset w25, -72
	.cfi_offset w26, -80
	.cfi_offset w27, -88
	.cfi_offset w28, -96
	sub	sp, sp, #528
	str	x3, [sp, #24]                   ; 8-byte Folded Spill
	str	x2, [sp, #8]                    ; 8-byte Folded Spill
	mov	x27, x1
	stp	xzr, x0, [sp, #80]              ; 16-byte Folded Spill
	str	xzr, [sp, #72]                  ; 8-byte Folded Spill
	mov	x19, #0
	mov	x22, #0
	mov	w24, #0
	mov	x21, #0
Lloh377:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh378:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh379:
	ldr	x8, [x8]
	stur	x8, [x29, #-96]
	str	x4, [sp]                        ; 8-byte Folded Spill
	ldr	x26, [x4]
	str	x5, [sp, #16]                   ; 8-byte Folded Spill
	ldr	x28, [x5]
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [sp, #480]
	stp	q0, q0, [sp, #448]
	stp	q0, q0, [sp, #416]
	stp	q0, q0, [sp, #384]
	add	x8, sp, #96
	add	x10, x8, #8
	add	x9, x8, #72
	stp	x9, x10, [sp, #48]              ; 16-byte Folded Spill
	add	x9, x8, #128
	add	x8, sp, #256
	orr	x8, x8, #0x8
	stp	x8, x9, [sp, #32]               ; 16-byte Folded Spill
	stp	q0, q0, [sp, #352]
Lloh380:
	adrp	x25, _spelling@PAGE
Lloh381:
	add	x25, x25, _spelling@PAGEOFF
	stp	q0, q0, [sp, #320]
	mov	w23, #40
	stp	q0, q0, [sp, #288]
	stp	q0, q0, [sp, #256]
	str	x1, [sp, #64]                   ; 8-byte Folded Spill
LBB4_1:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB4_13 Depth 2
                                        ;       Child Loop BB4_14 Depth 3
                                        ;       Child Loop BB4_21 Depth 3
                                        ;     Child Loop BB4_29 Depth 2
	cmp	x22, x27
	b.hs	LBB4_5
; %bb.2:                                ;   in Loop: Header=BB4_1 Depth=1
	ldr	x8, [sp, #88]                   ; 8-byte Folded Reload
	ldrsb	w0, [x8, x22]
	tbnz	w0, #31, LBB4_10
; %bb.3:                                ;   in Loop: Header=BB4_1 Depth=1
Lloh382:
	adrp	x8, __DefaultRuneLocale@GOTPAGE
Lloh383:
	ldr	x8, [x8, __DefaultRuneLocale@GOTPAGEOFF]
	add	x8, x8, w0, uxtw #2
	ldr	w8, [x8, #60]
	and	w0, w8, #0x4000
	cbz	w0, LBB4_11
LBB4_4:                                 ;   in Loop: Header=BB4_1 Depth=1
	cbnz	x21, LBB4_6
	b	LBB4_34
LBB4_5:                                 ;   in Loop: Header=BB4_1 Depth=1
	cbz	x21, LBB4_39
LBB4_6:                                 ;   in Loop: Header=BB4_1 Depth=1
	ldp	x9, x8, [sp, #80]               ; 16-byte Folded Reload
	add	x20, x8, x9
Lloh384:
	adrp	x0, l_.str.82@PAGE
Lloh385:
	add	x0, x0, l_.str.82@PAGEOFF
	mov	x1, x20
	mov	x2, x21
	bl	_is
	cbz	w0, LBB4_8
; %bb.7:                                ;   in Loop: Header=BB4_1 Depth=1
	mov	x21, #0
	eor	w24, w24, #0x1
	b	LBB4_34
LBB4_8:                                 ;   in Loop: Header=BB4_1 Depth=1
	tbz	w24, #0, LBB4_12
; %bb.9:                                ;   in Loop: Header=BB4_1 Depth=1
	mov	x21, #0
	mov	w24, #1
	b	LBB4_34
LBB4_10:                                ;   in Loop: Header=BB4_1 Depth=1
	mov	w1, #16384
	bl	___maskrune
	cbnz	w0, LBB4_4
LBB4_11:                                ;   in Loop: Header=BB4_1 Depth=1
	cmp	x21, #0
	ldr	x8, [sp, #80]                   ; 8-byte Folded Reload
	csel	x8, x22, x8, eq
	str	x8, [sp, #80]                   ; 8-byte Folded Spill
	add	x21, x21, #1
	b	LBB4_34
LBB4_12:                                ;   in Loop: Header=BB4_1 Depth=1
	mov	w27, #0
	mov	w8, #1
LBB4_13:                                ;   Parent Loop BB4_1 Depth=1
                                        ; =>  This Loop Header: Depth=2
                                        ;       Child Loop BB4_14 Depth 3
                                        ;       Child Loop BB4_21 Depth 3
	mov	x24, x8
LBB4_14:                                ;   Parent Loop BB4_1 Depth=1
                                        ;     Parent Loop BB4_13 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	ldr	x0, [x25, x24, lsl #3]
	mov	x1, x20
	mov	x2, x21
	bl	_is
	cbnz	w0, LBB4_16
; %bb.15:                               ;   in Loop: Header=BB4_14 Depth=3
	add	x24, x24, #1
	cmp	x24, #39
	b.ne	LBB4_14
	b	LBB4_23
LBB4_16:                                ;   in Loop: Header=BB4_13 Depth=2
	and	x8, x24, #0x7ffffffffffffff8
	cmp	x8, #16
	b.ne	LBB4_20
; %bb.17:                               ;   in Loop: Header=BB4_13 Depth=2
	ldr	x8, [sp, #256]
	madd	x9, x8, x23, x26
	ldr	x9, [x9, #16]
	cmp	x9, #2
	b.ne	LBB4_22
; %bb.18:                               ;   in Loop: Header=BB4_13 Depth=2
	madd	x8, x8, x23, x26
	ldr	x9, [x8, #24]!
	cmp	x9, x19
	b.ne	LBB4_22
; %bb.19:                               ;   in Loop: Header=BB4_13 Depth=2
	mov	x9, #-1
	str	x9, [x8]
	b	LBB4_22
LBB4_20:                                ;   in Loop: Header=BB4_13 Depth=2
Lloh386:
	adrp	x8, _arity@PAGE
Lloh387:
	add	x8, x8, _arity@PAGEOFF
	ldr	x8, [x8, x24, lsl #3]
	cmp	x8, #1
	csinc	x8, x8, xzr, hi
	add	x9, sp, #256
	mov	w11, #1
LBB4_21:                                ;   Parent Loop BB4_1 Depth=1
                                        ;     Parent Loop BB4_13 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	ldr	x10, [x9], #8
	madd	x10, x10, x23, x26
	str	x11, [x10, #16]
	subs	x8, x8, #1
	b.ne	LBB4_21
LBB4_22:                                ;   in Loop: Header=BB4_13 Depth=2
	str	x24, [sp, #96]
	mov	x10, #-1
	str	x10, [sp, #160]
	movi.2d	v0, #0000000000000000
	ldp	x9, x8, [sp, #40]               ; 16-byte Folded Reload
	stp	q0, q0, [x8]
	str	q0, [x8, #32]
	str	xzr, [x8, #48]
	cmp	x24, #35
	cset	w8, eq
	stp	x10, x10, [x9]
	str	x8, [sp, #240]
	ldr	x0, [sp, #56]                   ; 8-byte Folded Reload
	movi.2d	v0, #0xffffffffffffffff
	stp	q0, q0, [x0]
	str	q0, [x0, #32]
Lloh388:
	adrp	x8, _arity@PAGE
Lloh389:
	add	x8, x8, _arity@PAGEOFF
	ldr	x8, [x8, x24, lsl #3]
	lsl	x2, x8, #3
	str	x10, [x0, #48]
	add	x1, sp, #256
	mov	w3, #144
	bl	___memcpy_chk
	mov	w8, #152
	mul	x27, x19, x8
	add	x1, x27, #152
	ldr	x0, [sp, #72]                   ; 8-byte Folded Reload
	bl	_realloc
	add	x19, x19, #1
	str	x0, [sp, #72]                   ; 8-byte Folded Spill
	add	x9, x0, x27
	ldp	q0, q1, [sp, #192]
	stp	q0, q1, [x9, #96]
	ldr	q0, [sp, #224]
	str	q0, [x9, #128]
	ldr	x8, [sp, #240]
	str	x8, [x9, #144]
	ldp	q0, q1, [sp, #128]
	stp	q0, q1, [x9, #32]
	ldp	q0, q1, [sp, #160]
	stp	q0, q1, [x9, #64]
	ldp	q0, q1, [sp, #96]
	add	x8, x24, #1
	stp	q0, q1, [x9]
	mov	w27, #1
	cmp	x24, #38
	b.ne	LBB4_13
	b	LBB4_24
LBB4_23:                                ;   in Loop: Header=BB4_1 Depth=1
	tbz	w27, #0, LBB4_27
LBB4_24:                                ;   in Loop: Header=BB4_1 Depth=1
	ldr	x27, [sp, #64]                  ; 8-byte Folded Reload
	cbz	x19, LBB4_26
; %bb.25:                               ;   in Loop: Header=BB4_1 Depth=1
	ldr	x8, [sp, #72]                   ; 8-byte Folded Reload
	mov	w9, #152
	madd	x8, x19, x9, x8
	ldur	q0, [x8, #-152]
	ldur	q1, [x8, #-136]
	stp	q0, q1, [sp, #96]
	ldur	q0, [x8, #-120]
	ldur	q1, [x8, #-104]
	ldur	q2, [x8, #-88]
	ldur	q3, [x8, #-72]
	stp	q2, q3, [sp, #160]
	stp	q0, q1, [sp, #128]
	ldur	q0, [x8, #-56]
	ldur	q1, [x8, #-40]
	ldur	q2, [x8, #-24]
	ldur	x8, [x8, #-8]
	str	x8, [sp, #240]
	stp	q1, q2, [sp, #208]
	str	q0, [sp, #192]
	add	x0, sp, #96
	mov	x1, x26
	bl	_print_instruction
LBB4_26:                                ;   in Loop: Header=BB4_1 Depth=1
	mov	w24, #0
	mov	x21, #0
	b	LBB4_34
LBB4_27:                                ;   in Loop: Header=BB4_1 Depth=1
	cbz	x28, LBB4_32
; %bb.28:                               ;   in Loop: Header=BB4_1 Depth=1
	mov	x24, #0
	add	x27, x26, #24
LBB4_29:                                ;   Parent Loop BB4_1 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldur	x8, [x27, #-16]
	cmp	x8, x21
	b.ne	LBB4_31
; %bb.30:                               ;   in Loop: Header=BB4_29 Depth=2
	ldur	x0, [x27, #-24]
	mov	x1, x20
	mov	x2, x21
	bl	_strncmp
	cbz	w0, LBB4_35
LBB4_31:                                ;   in Loop: Header=BB4_29 Depth=2
	add	x24, x24, #1
	add	x27, x27, #40
	cmp	x28, x24
	b.ne	LBB4_29
LBB4_32:                                ;   in Loop: Header=BB4_1 Depth=1
	add	x1, sp, #256
	ldr	x0, [sp, #32]                   ; 8-byte Folded Reload
	mov	w2, #248
	bl	_memmove
	str	x28, [sp, #256]
	add	x27, x28, #1
	add	x24, x28, x28, lsl #2
	lsl	x8, x24, #3
	add	x1, x8, #40
	mov	x0, x26
	bl	_realloc
	mov	x26, x0
	add	x24, x0, x24, lsl #3
	mov	x0, x20
	mov	x1, x21
	bl	_strndup
	stp	x0, x21, [x24]
	mov	w8, #2
	stp	x8, x19, [x24, #16]
	mov	x8, #-1
	str	x8, [x24, #32]
Lloh390:
	adrp	x0, l_.str.84@PAGE
Lloh391:
	add	x0, x0, l_.str.84@PAGEOFF
	bl	_printf
	ldp	q0, q1, [x24]
	stp	q0, q1, [sp, #96]
	ldr	x8, [x24, #32]
	str	x8, [sp, #128]
	add	x0, sp, #96
	bl	_print_word
	mov	w24, #0
	mov	x21, #0
	mov	x28, x27
LBB4_33:                                ;   in Loop: Header=BB4_1 Depth=1
	ldr	x27, [sp, #64]                  ; 8-byte Folded Reload
LBB4_34:                                ;   in Loop: Header=BB4_1 Depth=1
	add	x22, x22, #1
	b	LBB4_1
LBB4_35:                                ;   in Loop: Header=BB4_1 Depth=1
Lloh392:
	adrp	x0, l_.str.83@PAGE
Lloh393:
	add	x0, x0, l_.str.83@PAGEOFF
	bl	_printf
	ldur	q0, [x27, #-24]
	ldur	q1, [x27, #-8]
	ldr	x8, [x27, #8]
	str	x8, [sp, #128]
	stp	q0, q1, [sp, #96]
	add	x0, sp, #96
	bl	_print_word
	add	x1, sp, #256
	ldr	x0, [sp, #32]                   ; 8-byte Folded Reload
	mov	w2, #248
	bl	_memmove
	str	x24, [sp, #256]
	ldur	x8, [x27, #-8]
	cmp	x8, #2
	b.ne	LBB4_37
; %bb.36:                               ;   in Loop: Header=BB4_1 Depth=1
	ldr	x8, [x27]
	cmn	x8, #1
	b.eq	LBB4_38
LBB4_37:                                ;   in Loop: Header=BB4_1 Depth=1
	mov	w24, #0
	mov	x21, #0
	b	LBB4_33
LBB4_38:                                ;   in Loop: Header=BB4_1 Depth=1
	mov	w24, #0
	mov	x21, #0
	str	x19, [x27]
	b	LBB4_33
LBB4_39:
	ldr	x8, [sp]                        ; 8-byte Folded Reload
	str	x26, [x8]
	ldr	x8, [sp, #16]                   ; 8-byte Folded Reload
	str	x28, [x8]
	ldr	x8, [sp, #8]                    ; 8-byte Folded Reload
	ldr	x9, [sp, #72]                   ; 8-byte Folded Reload
	str	x9, [x8]
	ldr	x8, [sp, #24]                   ; 8-byte Folded Reload
	str	x19, [x8]
	ldur	x8, [x29, #-96]
Lloh394:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh395:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh396:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB4_41
; %bb.40:
	add	sp, sp, #528
	ldp	x29, x30, [sp, #80]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #64]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #48]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #32]             ; 16-byte Folded Reload
	ldp	x26, x25, [sp, #16]             ; 16-byte Folded Reload
	ldp	x28, x27, [sp], #96             ; 16-byte Folded Reload
	ret
LBB4_41:
	bl	___stack_chk_fail
	.loh AdrpAdd	Lloh380, Lloh381
	.loh AdrpLdrGotLdr	Lloh377, Lloh378, Lloh379
	.loh AdrpLdrGot	Lloh382, Lloh383
	.loh AdrpAdd	Lloh384, Lloh385
	.loh AdrpAdd	Lloh386, Lloh387
	.loh AdrpAdd	Lloh388, Lloh389
	.loh AdrpAdd	Lloh390, Lloh391
	.loh AdrpAdd	Lloh392, Lloh393
	.loh AdrpLdrGotLdr	Lloh394, Lloh395, Lloh396
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function print_word
_print_word:                            ; @print_word
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #112
	.cfi_def_cfa_offset 112
	stp	x20, x19, [sp, #80]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #96]             ; 16-byte Folded Spill
	add	x29, sp, #96
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	x19, x0
	ldp	q0, q1, [x0]
	stp	q0, q1, [sp, #32]
	ldr	x8, [x0, #32]
	str	x8, [sp, #64]
	add	x0, sp, #32
	bl	_print_name
	ldp	x8, x9, [x19, #8]
	cmp	x9, #2
	b.hi	LBB5_2
; %bb.1:
Lloh397:
	adrp	x10, l_switch.table.print_word@PAGE
Lloh398:
	add	x10, x10, l_switch.table.print_word@PAGEOFF
	ldr	x9, [x10, x9, lsl #3]
	b	LBB5_3
LBB5_2:
Lloh399:
	adrp	x9, l_.str.27@PAGE
Lloh400:
	add	x9, x9, l_.str.27@PAGEOFF
LBB5_3:
	ldp	x10, x11, [x19, #24]
	stp	x10, x11, [sp, #16]
	stp	x8, x9, [sp]
Lloh401:
	adrp	x0, l_.str.21@PAGE
Lloh402:
	add	x0, x0, l_.str.21@PAGEOFF
	bl	_printf
	ldp	x29, x30, [sp, #96]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #80]             ; 16-byte Folded Reload
	add	sp, sp, #112
	ret
	.loh AdrpAdd	Lloh397, Lloh398
	.loh AdrpAdd	Lloh399, Lloh400
	.loh AdrpAdd	Lloh401, Lloh402
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function print_name
_print_name:                            ; @print_name
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
	mov	w8, #67
	str	x8, [sp]
Lloh403:
	adrp	x0, l_.str.22@PAGE
Lloh404:
	add	x0, x0, l_.str.22@PAGEOFF
	bl	_printf
	ldr	x8, [x19, #8]
	cbz	x8, LBB6_3
; %bb.1:
	mov	x20, #0
LBB6_2:                                 ; =>This Inner Loop Header: Depth=1
	ldr	x8, [x19]
	ldrsb	w0, [x8, x20]
	bl	_putchar
	add	x20, x20, #1
	ldr	x8, [x19, #8]
	cmp	x20, x8
	b.lo	LBB6_2
LBB6_3:
Lloh405:
	adrp	x0, l_.str.23@PAGE
Lloh406:
	add	x0, x0, l_.str.23@PAGEOFF
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #48
	b	_printf
	.loh AdrpAdd	Lloh403, Lloh404
	.loh AdrpAdd	Lloh405, Lloh406
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function print_instruction
_print_instruction:                     ; @print_instruction
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #144
	.cfi_def_cfa_offset 144
	stp	x24, x23, [sp, #80]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #96]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #112]            ; 16-byte Folded Spill
	stp	x29, x30, [sp, #128]            ; 16-byte Folded Spill
	add	x29, sp, #128
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	mov	x20, x1
	mov	x19, x0
	mov	w0, #9
	bl	_putchar
	ldr	x8, [x19]
	cbz	x8, LBB7_2
; %bb.1:
	ldr	x8, [x19, #8]
	mov	w9, #40
	madd	x8, x8, x9, x20
	ldp	q0, q1, [x8]
	stp	q0, q1, [sp, #32]
	ldr	x8, [x8, #32]
	str	x8, [sp, #64]
	add	x0, sp, #32
	bl	_print_name
LBB7_2:
Lloh407:
	adrp	x0, l_.str.30@PAGE
Lloh408:
	add	x0, x0, l_.str.30@PAGEOFF
	bl	_printf
	ldr	x8, [x19]
Lloh409:
	adrp	x9, _ins_color@PAGE
Lloh410:
	add	x9, x9, _ins_color@PAGEOFF
	ldr	x9, [x9, x8, lsl #3]
Lloh411:
	adrp	x10, _spelling@PAGE
Lloh412:
	add	x10, x10, _spelling@PAGEOFF
	ldr	x8, [x10, x8, lsl #3]
Lloh413:
	adrp	x10, l_.str.23@PAGE
Lloh414:
	add	x10, x10, l_.str.23@PAGEOFF
	stp	x8, x10, [sp, #8]
	str	x9, [sp]
Lloh415:
	adrp	x0, l_.str.31@PAGE
Lloh416:
	add	x0, x0, l_.str.31@PAGEOFF
	bl	_printf
	mov	w0, #123
	bl	_putchar
	ldr	x8, [x19]
	sub	x8, x8, #1
Lloh417:
	adrp	x21, _arity@PAGE
Lloh418:
	add	x21, x21, _arity@PAGEOFF
	cmp	x8, #35
	b.hi	LBB7_5
; %bb.3:
	mov	w22, #2
	mov	w23, #40
LBB7_4:                                 ; =>This Inner Loop Header: Depth=1
	mov	w0, #32
	bl	_putchar
	ldr	x8, [x19, x22, lsl #3]
	madd	x8, x8, x23, x20
	ldp	q0, q1, [x8]
	stp	q0, q1, [sp, #32]
	ldr	x8, [x8, #32]
	str	x8, [sp, #64]
	add	x0, sp, #32
	bl	_print_name
	ldr	x8, [x19]
	ldr	x8, [x21, x8, lsl #3]
	add	x9, x22, #1
	cmp	x22, x8
	mov	x22, x9
	b.lo	LBB7_4
LBB7_5:
	ldr	x8, [x19, #64]
	ldp	x9, x10, [x19, #128]
	ldr	x11, [x19, #144]
	stp	x10, x11, [sp, #16]
	stp	x8, x9, [sp]
Lloh419:
	adrp	x0, l_.str.33@PAGE
Lloh420:
	add	x0, x0, l_.str.33@PAGEOFF
	bl	_printf
	ldr	x20, [x19]
Lloh421:
	adrp	x0, l_.str.80@PAGE
Lloh422:
	add	x0, x0, l_.str.80@PAGEOFF
	bl	_printf
	cbz	x20, LBB7_8
; %bb.6:
	ldr	x20, [x21, x20, lsl #3]
	add	x21, x19, #72
Lloh423:
	adrp	x19, l_.str.81@PAGE
Lloh424:
	add	x19, x19, l_.str.81@PAGEOFF
LBB7_7:                                 ; =>This Inner Loop Header: Depth=1
	ldr	x8, [x21], #8
	str	x8, [sp]
	mov	x0, x19
	bl	_printf
	subs	x20, x20, #1
	b.ne	LBB7_7
LBB7_8:
Lloh425:
	adrp	x0, l_str.194@PAGE
Lloh426:
	add	x0, x0, l_str.194@PAGEOFF
	ldp	x29, x30, [sp, #128]            ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #112]            ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #96]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #80]             ; 16-byte Folded Reload
	add	sp, sp, #144
	b	_puts
	.loh AdrpAdd	Lloh417, Lloh418
	.loh AdrpAdd	Lloh415, Lloh416
	.loh AdrpAdd	Lloh413, Lloh414
	.loh AdrpAdd	Lloh411, Lloh412
	.loh AdrpAdd	Lloh409, Lloh410
	.loh AdrpAdd	Lloh407, Lloh408
	.loh AdrpAdd	Lloh421, Lloh422
	.loh AdrpAdd	Lloh419, Lloh420
	.loh AdrpAdd	Lloh423, Lloh424
	.loh AdrpAdd	Lloh425, Lloh426
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function is
_is:                                    ; @is
	.cfi_startproc
; %bb.0:
	stp	x22, x21, [sp, #-48]!           ; 16-byte Folded Spill
	.cfi_def_cfa_offset 48
	stp	x20, x19, [sp, #16]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #32]             ; 16-byte Folded Spill
	add	x29, sp, #32
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	mov	x19, x2
	mov	x21, x1
	mov	x20, x0
	bl	_strlen
	cmp	x0, x19
	b.ne	LBB8_2
; %bb.1:
	mov	x0, x21
	mov	x1, x20
	mov	x2, x19
	bl	_strncmp
	cmp	w0, #0
	cset	w0, eq
	b	LBB8_3
LBB8_2:
	mov	w0, #0
LBB8_3:
	ldp	x29, x30, [sp, #32]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #16]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp], #48             ; 16-byte Folded Reload
	ret
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function string_to_number
_string_to_number:                      ; @string_to_number
	.cfi_startproc
; %bb.0:
	ldr	x10, [x1]
	cbz	x10, LBB9_11
; %bb.1:
	mov	x8, x0
	mov	x9, #0
	mov	x0, #0
	mov	x11, #0
	mov	w12, #1
Lloh427:
	adrp	x13, _digits@PAGE
Lloh428:
	add	x13, x13, _digits@PAGEOFF
LBB9_2:                                 ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB9_3 Depth 2
	mov	x14, #0
	ldrb	w15, [x8, x9]
LBB9_3:                                 ;   Parent Loop BB9_2 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w16, [x13, x14]
	cmp	w16, w15
	b.eq	LBB9_5
; %bb.4:                                ;   in Loop: Header=BB9_3 Depth=2
	add	x14, x14, #1
	cmp	x14, #96
	b.ne	LBB9_3
LBB9_5:                                 ;   in Loop: Header=BB9_2 Depth=1
	cbz	x9, LBB9_8
; %bb.6:                                ;   in Loop: Header=BB9_2 Depth=1
	cmp	x14, x11
	b.hs	LBB9_12
; %bb.7:                                ;   in Loop: Header=BB9_2 Depth=1
	madd	x0, x14, x12, x0
	mul	x12, x12, x11
	b	LBB9_9
LBB9_8:                                 ;   in Loop: Header=BB9_2 Depth=1
	mov	x11, x14
LBB9_9:                                 ;   in Loop: Header=BB9_2 Depth=1
	add	x9, x9, #1
	cmp	x9, x10
	b.ne	LBB9_2
; %bb.10:
	mov	x9, x10
	b	LBB9_12
LBB9_11:
	mov	x0, #0
	mov	x9, #0
LBB9_12:
	str	x9, [x1]
	ret
	.loh AdrpAdd	Lloh427, Lloh428
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function generate_operation
_generate_operation:                    ; @generate_operation
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #240
	.cfi_def_cfa_offset 240
	stp	x24, x23, [sp, #176]            ; 16-byte Folded Spill
	stp	x22, x21, [sp, #192]            ; 16-byte Folded Spill
	stp	x20, x19, [sp, #208]            ; 16-byte Folded Spill
	stp	x29, x30, [sp, #224]            ; 16-byte Folded Spill
	add	x29, sp, #224
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	.cfi_offset w23, -56
	.cfi_offset w24, -64
	mov	x20, x3
	mov	x21, x2
	mov	x19, x1
	mov	x22, x0
	ldp	q0, q1, [x2, #96]
	stp	q0, q1, [sp, #112]
	ldr	q0, [x2, #128]
	str	q0, [sp, #144]
	ldr	x8, [x2, #144]
	str	x8, [sp, #160]
	ldp	q0, q1, [x2, #32]
	stp	q0, q1, [sp, #48]
	ldp	q0, q1, [x2, #64]
	stp	q0, q1, [sp, #80]
	ldp	q0, q1, [x2]
	stp	q0, q1, [sp, #16]
	add	x0, sp, #16
	mov	x1, x4
	bl	_print_instruction
	str	x22, [sp]
Lloh429:
	adrp	x1, l_.str.181@PAGE
Lloh430:
	add	x1, x1, l_.str.181@PAGEOFF
	mov	x0, x19
	bl	_fprintf
	ldr	x8, [x21, #64]
	str	x8, [sp]
Lloh431:
	adrp	x22, l_.str.182@PAGE
Lloh432:
	add	x22, x22, l_.str.182@PAGEOFF
	mov	x0, x19
	mov	x1, x22
	bl	_fprintf
Lloh433:
	adrp	x23, l_.str.183@PAGE
Lloh434:
	add	x23, x23, l_.str.183@PAGEOFF
	mov	x0, x23
	mov	w1, #2
	mov	w2, #1
	mov	x3, x19
	bl	_fwrite
	ldr	x8, [x21, #80]
	mov	w24, #152
	madd	x8, x8, x24, x20
	ldr	x8, [x8, #64]
	str	x8, [sp]
	mov	x0, x19
	mov	x1, x22
	bl	_fprintf
	mov	x0, x23
	mov	w1, #2
	mov	w2, #1
	mov	x3, x19
	bl	_fwrite
	ldr	x8, [x21, #88]
	madd	x8, x8, x24, x20
	ldr	x8, [x8, #64]
	str	x8, [sp]
	mov	x0, x19
	mov	x1, x22
	bl	_fprintf
	mov	w0, #10
	mov	x1, x19
	ldp	x29, x30, [sp, #224]            ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #208]            ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #192]            ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #176]            ; 16-byte Folded Reload
	add	sp, sp, #240
	b	_fputc
	.loh AdrpAdd	Lloh433, Lloh434
	.loh AdrpAdd	Lloh431, Lloh432
	.loh AdrpAdd	Lloh429, Lloh430
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function main.cold.1
_main.cold.1:                           ; @main.cold.1
	.cfi_startproc
; %bb.0:
	stp	x29, x30, [sp, #-16]!           ; 16-byte Folded Spill
	.cfi_def_cfa_offset 16
	mov	x29, sp
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
Lloh435:
	adrp	x0, l_.str.132@PAGE
Lloh436:
	add	x0, x0, l_.str.132@PAGEOFF
	bl	_perror
	mov	w0, #1
	bl	_exit
	.loh AdrpAdd	Lloh435, Lloh436
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function main.cold.2
_main.cold.2:                           ; @main.cold.2
	.cfi_startproc
; %bb.0:
	stp	x29, x30, [sp, #-16]!           ; 16-byte Folded Spill
	.cfi_def_cfa_offset 16
	mov	x29, sp
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
Lloh437:
	adrp	x0, l_str.205@PAGE
Lloh438:
	add	x0, x0, l_str.205@PAGEOFF
	bl	_puts
	bl	_abort
	.loh AdrpAdd	Lloh437, Lloh438
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function main.cold.3
_main.cold.3:                           ; @main.cold.3
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #32
	.cfi_def_cfa_offset 32
	stp	x29, x30, [sp, #16]             ; 16-byte Folded Spill
	add	x29, sp, #16
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
Lloh439:
	adrp	x8, _spelling@PAGE
Lloh440:
	add	x8, x8, _spelling@PAGEOFF
	ldr	x8, [x8, x0, lsl #3]
	stp	x8, x0, [sp]
Lloh441:
	adrp	x0, l_.str.176@PAGE
Lloh442:
	add	x0, x0, l_.str.176@PAGEOFF
	bl	_printf
	bl	_abort
	.loh AdrpAdd	Lloh441, Lloh442
	.loh AdrpAdd	Lloh439, Lloh440
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function main.cold.4
_main.cold.4:                           ; @main.cold.4
	.cfi_startproc
; %bb.0:
	stp	x29, x30, [sp, #-16]!           ; 16-byte Folded Spill
	.cfi_def_cfa_offset 16
	mov	x29, sp
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
Lloh443:
	adrp	x0, l_str.207@PAGE
Lloh444:
	add	x0, x0, l_str.207@PAGEOFF
	bl	_puts
	bl	_abort
	.loh AdrpAdd	Lloh443, Lloh444
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function main.cold.5
_main.cold.5:                           ; @main.cold.5
	.cfi_startproc
; %bb.0:
	stp	x29, x30, [sp, #-16]!           ; 16-byte Folded Spill
	.cfi_def_cfa_offset 16
	mov	x29, sp
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
Lloh445:
	adrp	x0, l_.str.160@PAGE
Lloh446:
	add	x0, x0, l_.str.160@PAGEOFF
	bl	_puts
	bl	_abort
	.loh AdrpAdd	Lloh445, Lloh446
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function main.cold.6
_main.cold.6:                           ; @main.cold.6
	.cfi_startproc
; %bb.0:
	stp	x29, x30, [sp, #-16]!           ; 16-byte Folded Spill
	.cfi_def_cfa_offset 16
	mov	x29, sp
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
Lloh447:
	adrp	x0, l_.str.157@PAGE
Lloh448:
	add	x0, x0, l_.str.157@PAGEOFF
	bl	_puts
	bl	_abort
	.loh AdrpAdd	Lloh447, Lloh448
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function main.cold.7
_main.cold.7:                           ; @main.cold.7
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #192
	.cfi_def_cfa_offset 192
	stp	x20, x19, [sp, #160]            ; 16-byte Folded Spill
	stp	x29, x30, [sp, #176]            ; 16-byte Folded Spill
	add	x29, sp, #176
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	x19, x2
	mov	x20, x0
	ldr	x8, [x1]
	str	x8, [sp]
Lloh449:
	adrp	x0, l_.str.145@PAGE
Lloh450:
	add	x0, x0, l_.str.145@PAGEOFF
	bl	_printf
	bl	_OUTLINED_FUNCTION_0
	bl	_OUTLINED_FUNCTION_1
	bl	_abort
	.loh AdrpAdd	Lloh449, Lloh450
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function main.cold.8
_main.cold.8:                           ; @main.cold.8
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #192
	.cfi_def_cfa_offset 192
	stp	x20, x19, [sp, #160]            ; 16-byte Folded Spill
	stp	x29, x30, [sp, #176]            ; 16-byte Folded Spill
	add	x29, sp, #176
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	mov	x19, x2
	mov	x20, x0
	ldr	x8, [x1]
	str	x8, [sp]
Lloh451:
	adrp	x0, l_.str.145@PAGE
Lloh452:
	add	x0, x0, l_.str.145@PAGEOFF
	bl	_printf
	bl	_OUTLINED_FUNCTION_0
	bl	_OUTLINED_FUNCTION_1
	bl	_abort
	.loh AdrpAdd	Lloh451, Lloh452
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function OUTLINED_FUNCTION_0
_OUTLINED_FUNCTION_0:                   ; @OUTLINED_FUNCTION_0 Thunk
	.cfi_startproc
; %bb.0:
	add	x0, sp, #8
	mov	x1, x20
	mov	w2, #152
	b	_memcpy
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function OUTLINED_FUNCTION_1
_OUTLINED_FUNCTION_1:                   ; @OUTLINED_FUNCTION_1 Thunk
	.cfi_startproc
; %bb.0:
	add	x0, sp, #8
	mov	x1, x19
	b	_print_instruction
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"executable_program.out"

l_.str.1:                               ; @.str.1
	.asciz	"a repl for my programming language."

l_.str.2:                               ; @.str.2
	.asciz	"\n\trecieved input(%llu): \n\n\t\t\"%s\"\n"

l_.str.3:                               ; @.str.3
	.asciz	"q"

l_.str.4:                               ; @.str.4
	.asciz	"quit"

l_.str.5:                               ; @.str.5
	.asciz	"o"

l_.str.6:                               ; @.str.6
	.asciz	"clear"

l_.str.7:                               ; @.str.7
	.asciz	"\033[H\033[2J"

l_.str.8:                               ; @.str.8
	.asciz	"dictionary"

l_.str.9:                               ; @.str.9
	.asciz	"instructions"

l_.str.10:                              ; @.str.10
	.asciz	"reset"

l_.str.11:                              ; @.str.11
	.asciz	"\n"

l_.str.12:                              ; @.str.12
	.asciz	"        "

l_.str.13:                              ; @.str.13
	.asciz	"\033[A"

l_.str.14:                              ; @.str.14
	.asciz	"\033[%huC"

l_.str.15:                              ; @.str.15
	.asciz	"\b\b\b\b\b\b\b\b"

l_.str.16:                              ; @.str.16
	.asciz	"\b"

l_.str.17:                              ; @.str.17
	.asciz	"\b \b"

l_.str.19:                              ; @.str.19
	.asciz	"\t%3llu  :  \033[32m"

l_.str.21:                              ; @.str.21
	.asciz	"   \t: (%llu) { .type = %s, .val = %lld .def = %lld } \n"

l_.str.22:                              ; @.str.22
	.asciz	"\033[38;5;%dm"

l_.str.23:                              ; @.str.23
	.asciz	"\033[0m"

l_.str.24:                              ; @.str.24
	.asciz	"{null_type}"

l_.str.25:                              ; @.str.25
	.asciz	"\033[36mlabel\033[0m"

l_.str.26:                              ; @.str.26
	.asciz	"\033[32mvariable\033[0m"

l_.str.27:                              ; @.str.27
	.asciz	"unknown"

l_.str.29:                              ; @.str.29
	.asciz	"\t%3llu  :  "

	.section	__TEXT,__const
	.p2align	3                               ; @arity
_arity:
	.quad	0                               ; 0x0
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	3                               ; 0x3
	.quad	2                               ; 0x2
	.quad	2                               ; 0x2
	.quad	2                               ; 0x2
	.quad	2                               ; 0x2
	.quad	2                               ; 0x2
	.quad	2                               ; 0x2
	.quad	2                               ; 0x2
	.quad	2                               ; 0x2
	.quad	2                               ; 0x2
	.quad	2                               ; 0x2
	.quad	2                               ; 0x2
	.quad	2                               ; 0x2
	.quad	2                               ; 0x2
	.quad	2                               ; 0x2
	.quad	7                               ; 0x7
	.quad	1                               ; 0x1
	.quad	1                               ; 0x1

	.section	__TEXT,__cstring,cstring_literals
l_.str.30:                              ; @.str.30
	.asciz	" = "

l_.str.31:                              ; @.str.31
	.asciz	"%s%s%s "

	.section	__DATA,__const
	.p2align	3                               ; @ins_color
_ins_color:
	.quad	l_.str.34
	.quad	l_.str.35
	.quad	l_.str.35
	.quad	l_.str.35
	.quad	l_.str.36
	.quad	l_.str.35
	.quad	l_.str.35
	.quad	l_.str.35
	.quad	l_.str.36
	.quad	l_.str.37
	.quad	l_.str.37
	.quad	l_.str.37
	.quad	l_.str.37
	.quad	l_.str.37
	.quad	l_.str.37
	.quad	l_.str.37
	.quad	l_.str.38
	.quad	l_.str.38
	.quad	l_.str.38
	.quad	l_.str.38
	.quad	l_.str.38
	.quad	l_.str.38
	.quad	l_.str.38
	.quad	l_.str.38
	.quad	l_.str.39
	.quad	l_.str.39
	.quad	l_.str.39
	.quad	l_.str.39
	.quad	l_.str.40
	.quad	l_.str.40
	.quad	l_.str.40
	.quad	l_.str.40
	.quad	l_.str.40
	.quad	l_.str.40
	.quad	l_.str.40
	.quad	l_.str.40
	.quad	l_.str.34
	.quad	l_.str.34
	.quad	l_.str.34

	.p2align	3                               ; @spelling
_spelling:
	.quad	l_.str.41
	.quad	l_.str.42
	.quad	l_.str.43
	.quad	l_.str.44
	.quad	l_.str.45
	.quad	l_.str.46
	.quad	l_.str.47
	.quad	l_.str.48
	.quad	l_.str.49
	.quad	l_.str.50
	.quad	l_.str.51
	.quad	l_.str.52
	.quad	l_.str.53
	.quad	l_.str.54
	.quad	l_.str.55
	.quad	l_.str.56
	.quad	l_.str.57
	.quad	l_.str.58
	.quad	l_.str.59
	.quad	l_.str.60
	.quad	l_.str.61
	.quad	l_.str.62
	.quad	l_.str.63
	.quad	l_.str.64
	.quad	l_.str.65
	.quad	l_.str.66
	.quad	l_.str.67
	.quad	l_.str.68
	.quad	l_.str.69
	.quad	l_.str.70
	.quad	l_.str.71
	.quad	l_.str.72
	.quad	l_.str.73
	.quad	l_.str.74
	.quad	l_.str.75
	.quad	l_.str.76
	.quad	l_.str.77
	.quad	l_.str.78
	.quad	l_.str.79

	.section	__TEXT,__cstring,cstring_literals
l_.str.33:                              ; @.str.33
	.asciz	" }\n\t\t\t\t\t\t\t.ph = %lld .life = [%lld,%lld] .ct = %lld .defs="

l_.str.34:                              ; @.str.34
	.space	1

l_.str.35:                              ; @.str.35
	.asciz	"\033[31m"

l_.str.36:                              ; @.str.36
	.asciz	"\033[32m"

l_.str.37:                              ; @.str.37
	.asciz	"\033[34m"

l_.str.38:                              ; @.str.38
	.asciz	"\033[36m"

l_.str.39:                              ; @.str.39
	.asciz	"\033[33m"

l_.str.40:                              ; @.str.40
	.asciz	"\033[35m"

l_.str.41:                              ; @.str.41
	.asciz	"{nulli}"

l_.str.42:                              ; @.str.42
	.asciz	"sll"

l_.str.43:                              ; @.str.43
	.asciz	"srl"

l_.str.44:                              ; @.str.44
	.asciz	"sra"

l_.str.45:                              ; @.str.45
	.asciz	"add"

l_.str.46:                              ; @.str.46
	.asciz	"xor"

l_.str.47:                              ; @.str.47
	.asciz	"and"

l_.str.48:                              ; @.str.48
	.asciz	"or"

l_.str.49:                              ; @.str.49
	.asciz	"sub"

l_.str.50:                              ; @.str.50
	.asciz	"mul"

l_.str.51:                              ; @.str.51
	.asciz	"mhs"

l_.str.52:                              ; @.str.52
	.asciz	"mhsu"

l_.str.53:                              ; @.str.53
	.asciz	"div"

l_.str.54:                              ; @.str.54
	.asciz	"rem"

l_.str.55:                              ; @.str.55
	.asciz	"divs"

l_.str.56:                              ; @.str.56
	.asciz	"rems"

l_.str.57:                              ; @.str.57
	.asciz	"blt"

l_.str.58:                              ; @.str.58
	.asciz	"bge"

l_.str.59:                              ; @.str.59
	.asciz	"blts"

l_.str.60:                              ; @.str.60
	.asciz	"bges"

l_.str.61:                              ; @.str.61
	.asciz	"bne"

l_.str.62:                              ; @.str.62
	.asciz	"beq"

l_.str.63:                              ; @.str.63
	.asciz	"jalr"

l_.str.64:                              ; @.str.64
	.asciz	"jal"

l_.str.65:                              ; @.str.65
	.asciz	"store1"

l_.str.66:                              ; @.str.66
	.asciz	"store2"

l_.str.67:                              ; @.str.67
	.asciz	"store4"

l_.str.68:                              ; @.str.68
	.asciz	"store8"

l_.str.69:                              ; @.str.69
	.asciz	"load1"

l_.str.70:                              ; @.str.70
	.asciz	"load2"

l_.str.71:                              ; @.str.71
	.asciz	"load4"

l_.str.72:                              ; @.str.72
	.asciz	"load8"

l_.str.73:                              ; @.str.73
	.asciz	"load1s"

l_.str.74:                              ; @.str.74
	.asciz	"load2s"

l_.str.75:                              ; @.str.75
	.asciz	"load4s"

l_.str.76:                              ; @.str.76
	.asciz	"loadi"

l_.str.77:                              ; @.str.77
	.asciz	"ecall"

l_.str.78:                              ; @.str.78
	.asciz	"debugprint"

l_.str.79:                              ; @.str.79
	.asciz	"debughex"

l_.str.80:                              ; @.str.80
	.asciz	"{ "

l_.str.81:                              ; @.str.81
	.asciz	"%llu "

l_.str.82:                              ; @.str.82
	.asciz	"note"

l_.str.83:                              ; @.str.83
	.asciz	"[DEFINED]    "

l_.str.84:                              ; @.str.84
	.asciz	"[not defined]  -->  assuming  "

.zerofill __DATA,__bss,_execute.variables,32768,3 ; @execute.variables
l_.str.85:                              ; @.str.85
	.asciz	"executing @%llu : "

l_.str.86:                              ; @.str.86
	.asciz	"executed sll: %llu = %llu %llu\n"

l_.str.87:                              ; @.str.87
	.asciz	"executed srl: %llu = %llu %llu\n"

l_.str.88:                              ; @.str.88
	.asciz	"executed sra: %llu = %llu %llu\n"

l_.str.89:                              ; @.str.89
	.asciz	"executed add: %llu = %llu %llu\n"

l_.str.90:                              ; @.str.90
	.asciz	"executed xor: %llu = %llu %llu\n"

l_.str.91:                              ; @.str.91
	.asciz	"executed and: %llu = %llu %llu\n"

l_.str.92:                              ; @.str.92
	.asciz	"executed or: %llu = %llu %llu\n"

l_.str.93:                              ; @.str.93
	.asciz	"executed sub: %llu = %llu %llu\n"

l_.str.94:                              ; @.str.94
	.asciz	"executed mul: %llu = %llu %llu\n"

l_.str.95:                              ; @.str.95
	.asciz	"internal error: executed mhs: %llu = %llu %llu\n"

l_.str.96:                              ; @.str.96
	.asciz	"internal error: executed mhsu: %llu = %llu %llu\n"

l_.str.97:                              ; @.str.97
	.asciz	"executed div: %llu = %llu %llu\n"

l_.str.98:                              ; @.str.98
	.asciz	"executed rem: %llu = %llu %llu\n"

l_.str.99:                              ; @.str.99
	.asciz	"executed divs: %llu = %llu %llu\n"

l_.str.100:                             ; @.str.100
	.asciz	"executed rems: %llu = %llu %llu\n"

l_.str.101:                             ; @.str.101
	.asciz	"executing blt -> @%llu [%llu %llu]\n"

l_.str.102:                             ; @.str.102
	.asciz	"\033[31minternal error: unspecified label in branch\033[0m"

l_.str.103:                             ; @.str.103
	.asciz	"executing bge -> @%llu [%llu %llu]\n"

l_.str.104:                             ; @.str.104
	.asciz	"executing blts -> @%llu [%llu %llu]\n"

l_.str.105:                             ; @.str.105
	.asciz	"executing bges -> @%llu [%llu %llu]\n"

l_.str.106:                             ; @.str.106
	.asciz	"executing bne -> @%llu [%llu %llu]\n"

l_.str.107:                             ; @.str.107
	.asciz	"executing beq -> @%llu [%llu %llu]\n"

l_.str.108:                             ; @.str.108
	.asciz	"executing jal -> @%llu (%llu) \n"

l_.str.109:                             ; @.str.109
	.asciz	"executing jalr (%llu) -> %llu]\n"

l_.str.110:                             ; @.str.110
	.asciz	"executed store1: *(%llu) = %llu\n"

l_.str.111:                             ; @.str.111
	.asciz	"executed store2: *(%llu) = %llu\n"

l_.str.112:                             ; @.str.112
	.asciz	"executed store4: *(%llu) = %llu\n"

l_.str.113:                             ; @.str.113
	.asciz	"executed store8: *(%llu) = %llu\n"

l_.str.114:                             ; @.str.114
	.asciz	"executed load1: %llu = *%llu\n"

l_.str.115:                             ; @.str.115
	.asciz	"executed load2: %llu = *%llu\n"

l_.str.116:                             ; @.str.116
	.asciz	"executed load4: %llu = *%llu\n"

l_.str.117:                             ; @.str.117
	.asciz	"executed load8: %llu = *%llu\n"

l_.str.118:                             ; @.str.118
	.asciz	"executed load1s: %llu = *%llu\n"

l_.str.119:                             ; @.str.119
	.asciz	"executed load2s: %llu = *%llu\n"

l_.str.120:                             ; @.str.120
	.asciz	"executed load4s: %llu = *%llu\n"

l_.str.121:                             ; @.str.121
	.asciz	"executing loadi %llu %llu]\n"

l_.str.122:                             ; @.str.122
	.asciz	"in[1] constant = %llu (length = %llu)\n"

l_.str.123:                             ; @.str.123
	.asciz	"executed ecall: { [%llu, %llu, %llu, %llu, %llu, %llu, :: #%llu] }\n"

l_.str.124:                             ; @.str.124
	.asciz	"executed debugprint: %llu\n"

l_.str.125:                             ; @.str.125
	.asciz	"\033[32mdebug: %llu\n\033[0m"

l_.str.126:                             ; @.str.126
	.asciz	"executed hex: %llu\n"

l_.str.127:                             ; @.str.127
	.asciz	"\033[32mdebug: %llx\n\033[0m"

l_.str.128:                             ; @.str.128
	.asciz	"internal error: execute: unexpected instruction: %llu\n"

l_.str.129:                             ; @.str.129
	.asciz	"\033[32m[finished execution]\033[0m"

	.section	__TEXT,__const
_digits:                                ; @digits
	.asciz	"0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,.;:-_=+/?!@#$%^&*()<>[]{}|\\~`'\"\000"

	.section	__TEXT,__cstring,cstring_literals
l_.str.130:                             ; @.str.130
	.asciz	"\033[32mSYSCALL (NR=%llu): {%llu %llu %llu %llu %llu %llu}\033[0m\n"

l_.str.131:                             ; @.str.131
	.asciz	"r"

l_.str.132:                             ; @.str.132
	.asciz	"fopen"

l_.str.133:                             ; @.str.133
	.asciz	"%s read %lu\n"

l_.str.134:                             ; @.str.134
	.asciz	"compile: text = \"%s\"\n"

l_.str.135:                             ; @.str.135
	.asciz	"cat asm_output.s"

l_.str.136:                             ; @.str.136
	.asciz	"as -v asm_output.s -o object_output.o"

l_.str.137:                             ; @.str.137
	.asciz	"/Library/Developer/CommandLineTools/usr/bin/ld -v -demangle -lto_library /Library/Developer/CommandLineTools/usr/lib/libLTO.dylib -dynamic -arch arm64 -platform_version macos 13.0.0 13.3 -syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -e %s -o %s -L/usr/local/lib object_output.o -lSystem /Library/Developer/CommandLineTools/usr/lib/clang/14.0.3/lib/darwin/libclang_rt.osx.a"

l_.str.138:                             ; @.str.138
	.asciz	"objdump -D object_output.o"

l_.str.139:                             ; @.str.139
	.asciz	"found label: %s\n"

l_.str.140:                             ; @.str.140
	.asciz	"found %llu labels: {"

l_.str.141:                             ; @.str.141
	.asciz	"#%llu: %s, "

l_.str.144:                             ; @.str.144
	.asciz	"looking at: \n\t#%llu:"

l_.str.145:                             ; @.str.145
	.asciz	"\033[33mwarning: use of uninitialized variable \"%s\" in instruction: \033[0m\n"

l_.str.146:                             ; @.str.146
	.asciz	"encountered use of \033[35m%s\033[0m, \n using definition at ins #%llu: "

l_.str.147:                             ; @.str.147
	.asciz	"found definition! \033[35m%s\033[0m is being defined at ins #%llu...\n"

l_.str.148:                             ; @.str.148
	.asciz	"generated lifetime of: \n\t#%llu:"

l_.str.152:                             ; @.str.152
	.asciz	"\033[33mwarning: result \"%s\" unused in instruction: \033[0m\n"

l_.str.153:                             ; @.str.153
	.asciz	"found ecall: %llu\n"

l_.str.155:                             ; @.str.155
	.asciz	"ecall #%llu   :  "

l_.str.157:                             ; @.str.157
	.asciz	"internal error: conflict in assigning registers! already assigned reg must be moved...\n"

l_.str.160:                             ; @.str.160
	.asciz	"ran out of regs!"

l_.str.164:                             ; @.str.164
	.asciz	"asm_output.s"

l_.str.165:                             ; @.str.165
	.asciz	"w"

l_.str.166:                             ; @.str.166
	.asciz	"\t.section __TEXT,__text,regular,pure_instructions\n\t.build_version macos, 13, 0 sdk_version 13, 3\n\t.globl %s\n\t.p2align 2\n"

l_.str.168:                             ; @.str.168
	.asciz	"generating label for %s at %llu...\n"

l_.str.169:                             ; @.str.169
	.asciz	".%s:\n"

l_.str.170:                             ; @.str.170
	.asciz	"\tsvc 0x80\n"

l_.str.171:                             ; @.str.171
	.asciz	"orr"

l_.str.172:                             ; @.str.172
	.asciz	"eor"

l_.str.173:                             ; @.str.173
	.asciz	"\tcmp x%llu, x%llu\n"

l_.str.174:                             ; @.str.174
	.asciz	"\tb.ne .%s\n"

l_.str.175:                             ; @.str.175
	.asciz	"\tb.eq .%s\n"

l_.str.176:                             ; @.str.176
	.asciz	"internal error: unknown instruction to generate: %s : %llu\n"

l_.str.177:                             ; @.str.177
	.asciz	"\tmov x0, #37\n"

l_.str.178:                             ; @.str.178
	.asciz	"\tmov x16, #%u\n"

l_.str.179:                             ; @.str.179
	.asciz	"\tsvc 0x80 ; temporary auto-generated exit syscall\n"

l_.str.180:                             ; @.str.180
	.asciz	".subsections_via_symbols\n"

l_.str.181:                             ; @.str.181
	.asciz	"\t%s "

l_.str.182:                             ; @.str.182
	.asciz	"x%llu"

l_.str.183:                             ; @.str.183
	.asciz	", "

l_.str.185:                             ; @.str.185
	.asciz	"\tmovz "

l_.str.186:                             ; @.str.186
	.asciz	", 0x%hx\n"

l_.str.187:                             ; @.str.187
	.asciz	"\tmovk "

l_.str.188:                             ; @.str.188
	.asciz	", 0x%hx, lsl 16\n"

l_.str.189:                             ; @.str.189
	.asciz	", 0x%hx, lsl 32\n"

l_.str.190:                             ; @.str.190
	.asciz	", 0x%hx, lsl 48\n"

l_str:                                  ; @str
	.asciz	"dictionary { "

l_str.192:                              ; @str.192
	.asciz	"instructions { "

l_str.194:                              ; @str.194
	.asciz	"}"

l_str.195:                              ; @str.195
	.asciz	"RA: obtaining lifetime information..."

l_str.196:                              ; @str.196
	.asciz	"printing lifetimes..."

l_str.197:                              ; @str.197
	.asciz	"debugging dict: "

l_str.198:                              ; @str.198
	.asciz	"-------------------"

l_str.199:                              ; @str.199
	.asciz	"printing list of ecalls found: "

l_str.200:                              ; @str.200
	.asciz	"[end of ecall list]"

l_str.201:                              ; @str.201
	.asciz	"printing results from ecall reg assignments: "

l_str.202:                              ; @str.202
	.asciz	"RA: performing register allocation..."

l_str.203:                              ; @str.203
	.asciz	"printing assignments..."

l_str.204:                              ; @str.204
	.asciz	"generating asm file..."

l_str.205:                              ; @str.205
	.asciz	"\033[31merror: missing entry point label\033[0m"

l_str.206:                              ; @str.206
	.asciz	"generate_assembly: generating this "

l_str.207:                              ; @str.207
	.asciz	"internal error: generate_loadi: bad destination register"

	.section	__DATA,__const
	.p2align	3                               ; @switch.table.print_word
l_switch.table.print_word:
	.quad	l_.str.24
	.quad	l_.str.26
	.quad	l_.str.25

.subsections_via_symbols
