	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 12, 0
	.section	__TEXT,__literal16,16byte_literals
	.p2align	4                               ; -- Begin function main
lCPI0_0:
	.long	4277009103                      ; 0xfeedfacf
	.long	16777228                        ; 0x100000c
	.long	2147483648                      ; 0x80000000
	.long	2                               ; 0x2
lCPI0_2:
	.long	1                               ; 0x1
	.long	152                             ; 0x98
	.long	8193                            ; 0x2001
	.long	0                               ; 0x0
lCPI0_3:
	.quad	0                               ; 0x0
	.quad	188                             ; 0xbc
lCPI0_4:
	.quad	0                               ; 0x0
	.quad	4                               ; 0x4
lCPI0_5:
	.long	184                             ; 0xb8
	.long	3                               ; 0x3
	.long	0                               ; 0x0
	.long	0                               ; 0x0
	.section	__TEXT,__literal8,8byte_literals
	.p2align	3
lCPI0_1:
	.long	25                              ; 0x19
	.long	152                             ; 0x98
	.section	__TEXT,__text,regular,pure_instructions
	.globl	_main
	.p2align	2
_main:                                  ; @main
	.cfi_startproc
; %bb.0:
	stp	x28, x27, [sp, #-96]!           ; 16-byte Folded Spill
	stp	x26, x25, [sp, #16]             ; 16-byte Folded Spill
	stp	x24, x23, [sp, #32]             ; 16-byte Folded Spill
	stp	x22, x21, [sp, #48]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #64]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #80]             ; 16-byte Folded Spill
	add	x29, sp, #80                    ; =80
	sub	sp, sp, #448                    ; =448
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
Lloh0:
	adrp	x8, ___stack_chk_guard@GOTPAGE
Lloh1:
	ldr	x8, [x8, ___stack_chk_guard@GOTPAGEOFF]
Lloh2:
	ldr	x8, [x8]
	stur	x8, [x29, #-104]
	cmp	w0, #2                          ; =2
	b.ne	LBB0_9
; %bb.1:
	mov	x20, x1
	mov	w0, #32768
	bl	_malloc
	mov	x19, x0
	mov	w1, #15
	mov	w2, #32768
	bl	_memset
	movi.2d	v0, #0000000000000000
	stp	q0, q0, [sp, #240]
	stp	q0, q0, [sp, #208]
	stp	q0, q0, [sp, #176]
	stp	q0, q0, [sp, #144]
	str	q0, [sp, #128]
	ldr	x0, [x20, #8]
	mov	w1, #0
	bl	_open
	tbnz	w0, #31, LBB0_170
; %bb.2:
	mov	x22, x0
	ldr	x0, [x20, #8]
	add	x1, sp, #128                    ; =128
	bl	_stat
	tbnz	w0, #31, LBB0_170
; %bb.3:
	ldr	x23, [sp, #224]
	sxtw	x20, w23
	cbz	w23, LBB0_10
; %bb.4:
	mov	x0, #0
	mov	x1, x20
	mov	w2, #1
	mov	w3, #1
	mov	x4, x22
	mov	x5, #0
	bl	_mmap
	cmn	x0, #1                          ; =1
	b.eq	LBB0_172
; %bb.5:
	mov	x21, x0
	mov	x0, x22
	bl	_close
	mov	x10, #0
	subs	w9, w23, #1                     ; =1
	csinc	w11, w23, wzr, gt
Lloh3:
	adrp	x8, l_.str.4@PAGE
Lloh4:
	add	x8, x8, l_.str.4@PAGEOFF
LBB0_6:                                 ; =>This Inner Loop Header: Depth=1
	ldrb	w12, [x21, x10]
	add	x10, x10, #1                    ; =1
	cmp	w12, #40                        ; =40
	b.eq	LBB0_16
; %bb.7:                                ;   in Loop: Header=BB0_6 Depth=1
	cmp	x11, x10
	b.ne	LBB0_6
LBB0_8:
	mov	w9, #0
	mov	w12, #0
	mov	w13, #0
	mov	w24, #0
	b	LBB0_11
LBB0_9:
Lloh5:
	adrp	x0, l_.str@PAGE
Lloh6:
	add	x0, x0, l_.str@PAGEOFF
	bl	_printf
	ldur	x8, [x29, #-104]
Lloh7:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh8:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh9:
	ldr	x9, [x9]
	cmp	x9, x8
	b.eq	LBB0_15
	b	LBB0_171
LBB0_10:
	mov	x0, x22
	bl	_close
	mov	w9, #0
	mov	w12, #0
	mov	w13, #0
	mov	x21, #0
	mov	w24, #0
	mov	x8, #0
LBB0_11:
	mov	w17, #0
	mov	w10, #0
	mov	w16, #0
LBB0_12:
Lloh10:
	adrp	x11, ___stderrp@GOTPAGE
Lloh11:
	ldr	x11, [x11, ___stderrp@GOTPAGEOFF]
Lloh12:
	ldr	x0, [x11]
	str	x8, [sp, #64]
	mov	w8, #8192
                                        ; kill: def $w24 killed $w24 killed $x24 def $x24
	stp	x24, x8, [sp, #48]
	stp	x16, x17, [sp, #32]
	stp	x13, x10, [sp, #16]
Lloh13:
	adrp	x1, l_.str.52@PAGE
Lloh14:
	add	x1, x1, l_.str.52@PAGEOFF
	stp	x12, x9, [sp]
	bl	_fprintf
LBB0_13:
	mov	x0, x21
	mov	x1, x20
	bl	_munmap
	mov	x0, x19
	bl	_free
	ldur	x8, [x29, #-104]
Lloh15:
	adrp	x9, ___stack_chk_guard@GOTPAGE
Lloh16:
	ldr	x9, [x9, ___stack_chk_guard@GOTPAGEOFF]
Lloh17:
	ldr	x9, [x9]
	cmp	x9, x8
	b.ne	LBB0_171
; %bb.14:
	mov	w0, #0
LBB0_15:
	add	sp, sp, #448                    ; =448
	ldp	x29, x30, [sp, #80]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #64]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #48]             ; 16-byte Folded Reload
	ldp	x24, x23, [sp, #32]             ; 16-byte Folded Reload
	ldp	x26, x25, [sp, #16]             ; 16-byte Folded Reload
	ldp	x28, x27, [sp], #96             ; 16-byte Folded Reload
	ret
LBB0_16:
Lloh18:
	adrp	x8, l_.str.6@PAGE
Lloh19:
	add	x8, x8, l_.str.6@PAGEOFF
LBB0_17:                                ; =>This Inner Loop Header: Depth=1
	cmp	x10, x20
	b.ge	LBB0_8
; %bb.18:                               ;   in Loop: Header=BB0_17 Depth=1
	ldrb	w11, [x21, x10]
	add	x10, x10, #1                    ; =1
	cmp	w11, #33                        ; =33
	b.lo	LBB0_17
; %bb.19:
	sub	x10, x10, #1                    ; =1
Lloh20:
	adrp	x8, l_.str.9@PAGE
Lloh21:
	add	x8, x8, l_.str.9@PAGEOFF
	mov	x12, #4294967296
Lloh22:
	adrp	x11, l_.str.11@PAGE
Lloh23:
	add	x11, x11, l_.str.11@PAGEOFF
Lloh24:
	adrp	x14, l_.str.14@PAGE
Lloh25:
	add	x14, x14, l_.str.14@PAGEOFF
Lloh26:
	adrp	x15, l_.str.16@PAGE
Lloh27:
	add	x15, x15, l_.str.16@PAGEOFF
	sxtw	x10, w10
	ldrb	w13, [x21, x10]
	cmp	w13, #40                        ; =40
	b.eq	LBB0_29
LBB0_20:
	cmp	w13, #92                        ; =92
	b.eq	LBB0_22
; %bb.21:
	cmp	w13, #41                        ; =41
	b.ne	LBB0_26
	b	LBB0_39
LBB0_22:
	lsl	x13, x10, #32
	add	x10, x10, #1                    ; =1
LBB0_23:                                ; =>This Inner Loop Header: Depth=1
	cmp	x10, x20
	b.ge	LBB0_37
; %bb.24:                               ;   in Loop: Header=BB0_23 Depth=1
	ldrb	w16, [x21, x10]
	add	x13, x13, x12
	add	x10, x10, #1                    ; =1
	cmp	w16, #33                        ; =33
	b.lo	LBB0_23
; %bb.25:
	asr	x10, x13, #32
LBB0_26:
	add	x10, x10, #1                    ; =1
LBB0_27:                                ; =>This Inner Loop Header: Depth=1
	cmp	x10, x20
	b.ge	LBB0_36
; %bb.28:                               ;   in Loop: Header=BB0_27 Depth=1
	ldrb	w13, [x21, x10]
	add	x10, x10, #1                    ; =1
	cmp	w13, #33                        ; =33
	b.lo	LBB0_27
	b	LBB0_35
LBB0_29:
	lsl	x13, x10, #32
	add	x16, x10, #1                    ; =1
LBB0_30:                                ; =>This Inner Loop Header: Depth=1
	cmp	x16, x20
	b.ge	LBB0_8
; %bb.31:                               ;   in Loop: Header=BB0_30 Depth=1
	mov	x10, x16
	ldrb	w17, [x21, x16]
	add	x13, x13, x12
	add	x16, x16, #1                    ; =1
	cmp	w17, #41                        ; =41
	b.ne	LBB0_30
; %bb.32:
	sxtw	x10, w10
	add	x10, x10, #1                    ; =1
LBB0_33:                                ; =>This Inner Loop Header: Depth=1
	cmp	x10, x20
	b.ge	LBB0_38
; %bb.34:                               ;   in Loop: Header=BB0_33 Depth=1
	ldrb	w13, [x21, x10]
	add	x10, x10, #1                    ; =1
	cmp	w13, #33                        ; =33
	b.lo	LBB0_33
LBB0_35:
	sub	x10, x10, #1                    ; =1
	sxtw	x10, w10
	ldrb	w13, [x21, x10]
	cmp	w13, #40                        ; =40
	b.ne	LBB0_20
	b	LBB0_29
LBB0_36:
	mov	w9, #0
	mov	w12, #0
	mov	w13, #0
	mov	w24, #0
	mov	w17, #0
	mov	w10, #0
	mov	w16, #0
	mov	x8, x15
	b	LBB0_12
LBB0_37:
	mov	w9, #0
	mov	w12, #0
	mov	w13, #0
	mov	w24, #0
	mov	w17, #0
	mov	w10, #0
	mov	w16, #0
	mov	x8, x14
	b	LBB0_12
LBB0_38:
	mov	w9, #0
	mov	w12, #0
	mov	w13, #0
	mov	w24, #0
	mov	w17, #0
	mov	w10, #0
	mov	w16, #0
	mov	x8, x11
	b	LBB0_12
LBB0_39:
	add	w8, w10, #1                     ; =1
	cmp	w8, w23
	csinc	w8, w23, w10, le
	add	x11, x10, #1                    ; =1
LBB0_40:                                ; =>This Inner Loop Header: Depth=1
	cmp	x11, x20
	b.ge	LBB0_42
; %bb.41:                               ;   in Loop: Header=BB0_40 Depth=1
	ldrb	w12, [x21, x11]
	add	w10, w10, #1                    ; =1
	add	x11, x11, #1                    ; =1
	cmp	w12, #33                        ; =33
	b.lo	LBB0_40
	b	LBB0_43
LBB0_42:
	mov	x10, x8
LBB0_43:
	mov	w11, #0
	mov	w3, #0
	mov	w1, #0
	mov	w8, #8192
	str	w8, [x19]
	sub	x12, x20, #1                    ; =1
	add	x13, x21, #1                    ; =1
	str	xzr, [x19, #8]
	mov	w24, #4
	mov	x14, #4294967296
	mov	w15, #8184
	stp	wzr, w10, [x19, #20]
	mov	x16, #-4294967296
	mov	x17, #8589934592
	mov	x0, #17179869184
	mov	x2, x10
Lloh28:
	adrp	x8, l_.str.39@PAGE
Lloh29:
	add	x8, x8, l_.str.39@PAGEOFF
LBB0_44:                                ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_46 Depth 2
                                        ;     Child Loop BB0_49 Depth 2
                                        ;     Child Loop BB0_65 Depth 2
                                        ;     Child Loop BB0_81 Depth 2
                                        ;       Child Loop BB0_75 Depth 3
                                        ;       Child Loop BB0_79 Depth 3
                                        ;       Child Loop BB0_69 Depth 3
                                        ;       Child Loop BB0_72 Depth 3
                                        ;     Child Loop BB0_85 Depth 2
                                        ;     Child Loop BB0_54 Depth 2
                                        ;       Child Loop BB0_56 Depth 3
                                        ;       Child Loop BB0_59 Depth 3
                                        ;     Child Loop BB0_62 Depth 2
                                        ;     Child Loop BB0_95 Depth 2
                                        ;     Child Loop BB0_99 Depth 2
                                        ;     Child Loop BB0_104 Depth 2
                                        ;     Child Loop BB0_112 Depth 2
                                        ;     Child Loop BB0_115 Depth 2
                                        ;       Child Loop BB0_114 Depth 3
                                        ;     Child Loop BB0_117 Depth 2
                                        ;     Child Loop BB0_123 Depth 2
                                        ;     Child Loop BB0_125 Depth 2
                                        ;     Child Loop BB0_129 Depth 2
	mov	x4, x24
	sxtw	x5, w4
	add	x4, x5, #1                      ; =1
	ldr	w6, [x19, x4, lsl #2]
	cbz	w6, LBB0_51
; %bb.45:                               ;   in Loop: Header=BB0_44 Depth=1
	add	w6, w6, #3                      ; =3
	ldrsw	x7, [x19, w6, sxtw #2]
LBB0_46:                                ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w6, [x13, x7]
	add	x7, x7, #1                      ; =1
	cmp	w6, #33                         ; =33
	b.lo	LBB0_46
; %bb.47:                               ;   in Loop: Header=BB0_44 Depth=1
	cmp	w6, #41                         ; =41
	b.ne	LBB0_52
; %bb.48:                               ;   in Loop: Header=BB0_44 Depth=1
	mov	x5, x2
	sxtw	x1, w5
	add	x2, x1, #1                      ; =1
	cmp	x2, x20
	csinc	x2, x20, x1, le
	sbfiz	x5, x5, #32, #32
LBB0_49:                                ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w6, [x21, x1]
	cmp	w6, #40                         ; =40
	b.eq	LBB0_64
; %bb.50:                               ;   in Loop: Header=BB0_49 Depth=2
	add	x1, x1, #1                      ; =1
	add	x5, x5, x14
	cmp	x1, x20
	b.lt	LBB0_49
	b	LBB0_88
LBB0_51:                                ;   in Loop: Header=BB0_44 Depth=1
	mov	x7, #0
LBB0_52:                                ;   in Loop: Header=BB0_44 Depth=1
                                        ; kill: def $w3 killed $w3 killed $x3 def $x3
	sxtw	x3, w3
	ldrb	w6, [x21, x3]
	sxtw	x4, w7
	ldrb	w7, [x21, x4]
	cmp	w6, #40                         ; =40
	b.ne	LBB0_54
LBB0_53:                                ;   in Loop: Header=BB0_44 Depth=1
	and	w22, w7, #0xfe
	cmp	w22, #40                        ; =40
	b.eq	LBB0_62
LBB0_54:                                ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Loop Header: Depth=2
                                        ;       Child Loop BB0_56 Depth 3
                                        ;       Child Loop BB0_59 Depth 3
	and	w6, w6, #0xff
	cmp	w6, w7, uxtb
	b.ne	LBB0_128
; %bb.55:                               ;   in Loop: Header=BB0_54 Depth=2
	lsl	x7, x4, #32
	add	x4, x4, #1                      ; =1
LBB0_56:                                ;   Parent Loop BB0_44 Depth=1
                                        ;     Parent Loop BB0_54 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	cmp	x4, x20
	b.ge	LBB0_128
; %bb.57:                               ;   in Loop: Header=BB0_56 Depth=3
	ldrb	w6, [x21, x4]
	add	x7, x7, x14
	add	x4, x4, #1                      ; =1
	cmp	w6, #33                         ; =33
	b.lo	LBB0_56
; %bb.58:                               ;   in Loop: Header=BB0_54 Depth=2
	lsl	x4, x3, #32
	add	x3, x3, #1                      ; =1
LBB0_59:                                ;   Parent Loop BB0_44 Depth=1
                                        ;     Parent Loop BB0_54 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	cmp	x3, x20
	b.ge	LBB0_128
; %bb.60:                               ;   in Loop: Header=BB0_59 Depth=3
	ldrb	w6, [x21, x3]
	add	x4, x4, x14
	add	x3, x3, #1                      ; =1
	cmp	w6, #33                         ; =33
	b.lo	LBB0_59
; %bb.61:                               ;   in Loop: Header=BB0_54 Depth=2
	asr	x3, x4, #32
	ldrb	w6, [x21, x3]
	asr	x4, x7, #32
	ldrb	w7, [x21, x4]
	cmp	w6, #40                         ; =40
	b.eq	LBB0_53
	b	LBB0_54
LBB0_62:                                ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w2, [x13, x3]
	add	x3, x3, #1                      ; =1
	cmp	w2, #33                         ; =33
	b.lo	LBB0_62
; %bb.63:                               ;   in Loop: Header=BB0_44 Depth=1
	add	x2, x19, x5, lsl #2
	ldr	w2, [x2, #8]
                                        ; kill: def $w3 killed $w3 killed $x3 def $x3
	sxtw	x5, w3
	ldrb	w4, [x21, x5]
	cmp	w4, #92                         ; =92
	b.ne	LBB0_92
	b	LBB0_94
LBB0_64:                                ;   in Loop: Header=BB0_44 Depth=1
	asr	x1, x5, #32
	add	x2, x1, #1                      ; =1
	cmp	x2, x20
	csinc	x5, x20, x1, le
LBB0_65:                                ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	add	x2, x1, #1                      ; =1
	cmp	x2, x20
	b.ge	LBB0_67
; %bb.66:                               ;   in Loop: Header=BB0_65 Depth=2
	ldrb	w6, [x13, x1]
	mov	x1, x2
	cmp	w6, #33                         ; =33
	b.lo	LBB0_65
	b	LBB0_81
LBB0_67:                                ;   in Loop: Header=BB0_44 Depth=1
	mov	w1, #8192
	mov	x2, x5
	b	LBB0_128
LBB0_68:                                ;   in Loop: Header=BB0_81 Depth=2
	lsl	x2, x1, #32
	add	x5, x1, #1                      ; =1
LBB0_69:                                ;   Parent Loop BB0_44 Depth=1
                                        ;     Parent Loop BB0_81 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	cmp	x5, x20
	b.ge	LBB0_87
; %bb.70:                               ;   in Loop: Header=BB0_69 Depth=3
	ldrb	w6, [x21, x5]
	add	x2, x2, x14
	add	x5, x5, #1                      ; =1
	cmp	w6, #41                         ; =41
	b.ne	LBB0_69
; %bb.71:                               ;   in Loop: Header=BB0_81 Depth=2
	asr	x1, x2, #32
	mov	x5, x1
LBB0_72:                                ;   Parent Loop BB0_44 Depth=1
                                        ;     Parent Loop BB0_81 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	add	x2, x5, #1                      ; =1
	cmp	x2, x20
	b.ge	LBB0_87
; %bb.73:                               ;   in Loop: Header=BB0_72 Depth=3
	ldrb	w6, [x13, x5]
	mov	x5, x2
	cmp	w6, #33                         ; =33
	b.lo	LBB0_72
	b	LBB0_81
LBB0_74:                                ;   in Loop: Header=BB0_81 Depth=2
	lsl	x2, x1, #32
	add	x5, x1, #1                      ; =1
LBB0_75:                                ;   Parent Loop BB0_44 Depth=1
                                        ;     Parent Loop BB0_81 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	cmp	x5, x20
	b.ge	LBB0_87
; %bb.76:                               ;   in Loop: Header=BB0_75 Depth=3
	ldrb	w6, [x21, x5]
	add	x2, x2, x14
	add	x5, x5, #1                      ; =1
	cmp	w6, #33                         ; =33
	b.lo	LBB0_75
; %bb.77:                               ;   in Loop: Header=BB0_81 Depth=2
	asr	x1, x2, #32
LBB0_78:                                ;   in Loop: Header=BB0_81 Depth=2
	mov	x5, x1
LBB0_79:                                ;   Parent Loop BB0_44 Depth=1
                                        ;     Parent Loop BB0_81 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	add	x2, x5, #1                      ; =1
	cmp	x2, x20
	b.ge	LBB0_87
; %bb.80:                               ;   in Loop: Header=BB0_79 Depth=3
	ldrb	w6, [x13, x5]
	mov	x5, x2
	cmp	w6, #33                         ; =33
	b.lo	LBB0_79
LBB0_81:                                ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Loop Header: Depth=2
                                        ;       Child Loop BB0_75 Depth 3
                                        ;       Child Loop BB0_79 Depth 3
                                        ;       Child Loop BB0_69 Depth 3
                                        ;       Child Loop BB0_72 Depth 3
	sxtw	x1, w2
	ldrb	w2, [x21, x1]
	cmp	w2, #40                         ; =40
	b.eq	LBB0_68
; %bb.82:                               ;   in Loop: Header=BB0_81 Depth=2
	cmp	w2, #92                         ; =92
	b.eq	LBB0_74
; %bb.83:                               ;   in Loop: Header=BB0_81 Depth=2
	cmp	w2, #41                         ; =41
	b.ne	LBB0_78
; %bb.84:                               ;   in Loop: Header=BB0_44 Depth=1
	add	w2, w1, #1                      ; =1
	cmp	w2, w23
	csinc	w2, w23, w1, le
	sub	w5, w2, #1                      ; =1
	add	x2, x1, #1                      ; =1
	cmp	x2, x20
	csinc	x7, x20, x1, le
	sub	w6, w1, #1                      ; =1
LBB0_85:                                ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	add	x2, x1, #1                      ; =1
	cmp	x2, x20
	b.ge	LBB0_89
; %bb.86:                               ;   in Loop: Header=BB0_85 Depth=2
	ldrb	w22, [x13, x1]
	add	w6, w6, #1                      ; =1
	mov	x1, x2
	cmp	w22, #33                        ; =33
	b.lo	LBB0_85
	b	LBB0_90
LBB0_87:                                ;   in Loop: Header=BB0_44 Depth=1
	add	x2, x1, #1                      ; =1
	cmp	x2, x20
	csinc	x2, x20, x1, le
LBB0_88:                                ;   in Loop: Header=BB0_44 Depth=1
	mov	w1, #8192
                                        ; kill: def $w2 killed $w2 killed $x2 def $x2
	b	LBB0_128
LBB0_89:                                ;   in Loop: Header=BB0_44 Depth=1
	mov	x6, x5
	mov	x2, x7
LBB0_90:                                ;   in Loop: Header=BB0_44 Depth=1
	cmp	w6, w10
	csel	w11, w11, w3, lt
	csel	w10, w10, w2, lt
	mov	w1, #8192
                                        ; kill: def $w2 killed $w2 killed $x2 def $x2
	b	LBB0_109
LBB0_91:                                ;   in Loop: Header=BB0_44 Depth=1
                                        ; kill: def $w3 killed $w3 killed $x3 def $x3
	sxtw	x5, w3
	ldrb	w4, [x21, x5]
	cmp	w4, #92                         ; =92
	b.eq	LBB0_94
LBB0_92:                                ;   in Loop: Header=BB0_44 Depth=1
	cmp	w4, #40                         ; =40
	b.eq	LBB0_106
; %bb.93:                               ;   in Loop: Header=BB0_44 Depth=1
	cmp	w4, #41                         ; =41
	b.ne	LBB0_96
	b	LBB0_108
LBB0_94:                                ;   in Loop: Header=BB0_44 Depth=1
	add	x4, x13, x5
LBB0_95:                                ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w5, [x4], #1
	add	w3, w3, #1                      ; =1
	cmp	w5, #33                         ; =33
	b.lo	LBB0_95
LBB0_96:                                ;   in Loop: Header=BB0_44 Depth=1
	cmp	w2, w23
	b.ge	LBB0_119
; %bb.97:                               ;   in Loop: Header=BB0_44 Depth=1
	mov	x4, x3
	sxtw	x4, w4
	ldrb	w5, [x21, x4]
	sxtw	x6, w2
	ldrb	w7, [x21, x6]
	cmp	w5, w7
	b.ne	LBB0_119
; %bb.98:                               ;   in Loop: Header=BB0_44 Depth=1
	sub	x5, x12, x6
	add	x6, x13, x6
LBB0_99:                                ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	cbz	x5, LBB0_102
; %bb.100:                              ;   in Loop: Header=BB0_99 Depth=2
	ldrb	w7, [x6], #1
	add	w2, w2, #1                      ; =1
	sub	x5, x5, #1                      ; =1
	cmp	w7, #33                         ; =33
	b.lo	LBB0_99
; %bb.101:                              ;   in Loop: Header=BB0_44 Depth=1
	sub	w5, w2, #1                      ; =1
	b	LBB0_103
LBB0_102:                               ;   in Loop: Header=BB0_44 Depth=1
	mov	x5, x9
	mov	x2, x23
LBB0_103:                               ;   in Loop: Header=BB0_44 Depth=1
	add	x4, x13, x4
LBB0_104:                               ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w6, [x4], #1
	add	w3, w3, #1                      ; =1
	cmp	w6, #33                         ; =33
	b.lo	LBB0_104
; %bb.105:                              ;   in Loop: Header=BB0_44 Depth=1
	cmp	w5, w10
	csel	w11, w11, w3, lt
	csel	w10, w10, w2, lt
	sxtw	x5, w3
	ldrb	w4, [x21, x5]
	cmp	w4, #92                         ; =92
	b.ne	LBB0_92
	b	LBB0_94
LBB0_106:                               ;   in Loop: Header=BB0_44 Depth=1
	cmp	w24, w15
	b.gt	LBB0_132
; %bb.107:                              ;   in Loop: Header=BB0_44 Depth=1
	add	x4, x19, w24, sxtw #2
	str	w1, [x4]
	str	w3, [x4, #12]
	stp	w24, w2, [x4, #20]
	add	w24, w24, #4                    ; =4
	mov	w3, #0
	mov	w1, #0
	b	LBB0_44
LBB0_108:                               ;   in Loop: Header=BB0_44 Depth=1
	add	w4, w24, #1                     ; =1
	sxtw	x4, w4
LBB0_109:                               ;   in Loop: Header=BB0_44 Depth=1
	mov	x5, x24
	sxtw	x26, w5
	add	x5, x19, w24, sxtw #2
	str	w1, [x5]
	str	w3, [x5, #12]
	ldrsw	x4, [x19, x4, lsl #2]
	cbz	w4, LBB0_118
; %bb.110:                              ;   in Loop: Header=BB0_44 Depth=1
	cmp	w24, w15
	b.gt	LBB0_132
; %bb.111:                              ;   in Loop: Header=BB0_44 Depth=1
	add	w1, w4, #1                      ; =1
	ldr	w1, [x19, w1, sxtw #2]
	add	x3, x19, x26, lsl #2
	stp	w1, w2, [x3, #20]
	ldr	w1, [x19, x4, lsl #2]
	add	w3, w4, #3                      ; =3
	ldrsw	x4, [x19, w3, sxtw #2]
	lsl	x3, x4, #32
LBB0_112:                               ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w5, [x13, x4]
	add	x4, x4, #1                      ; =1
	add	x3, x3, x14
	cmp	w5, #33                         ; =33
	b.lo	LBB0_112
	b	LBB0_115
LBB0_113:                               ;   in Loop: Header=BB0_115 Depth=2
	lsl	x3, x4, #32
	sxtw	x4, w4
LBB0_114:                               ;   Parent Loop BB0_44 Depth=1
                                        ;     Parent Loop BB0_115 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	ldrb	w5, [x13, x4]
	add	x4, x4, #1                      ; =1
	add	x3, x3, x14
	cmp	w5, #33                         ; =33
	b.lo	LBB0_114
LBB0_115:                               ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Loop Header: Depth=2
                                        ;       Child Loop BB0_114 Depth 3
	asr	x3, x3, #32
	ldrb	w5, [x21, x3]
	cmp	w5, #41                         ; =41
	b.ne	LBB0_113
; %bb.116:                              ;   in Loop: Header=BB0_44 Depth=1
	add	w24, w24, #4                    ; =4
LBB0_117:                               ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w4, [x13, x3]
	add	x3, x3, #1                      ; =1
	cmp	w4, #33                         ; =33
	b.lo	LBB0_117
	b	LBB0_91
LBB0_118:                               ;   in Loop: Header=BB0_44 Depth=1
	cmp	w2, w23
	b.eq	LBB0_146
LBB0_119:                               ;   in Loop: Header=BB0_44 Depth=1
	cmp	w1, #2, lsl #12                 ; =8192
	b.ne	LBB0_122
LBB0_120:                               ;   in Loop: Header=BB0_44 Depth=1
	cbz	w24, LBB0_133
; %bb.121:                              ;   in Loop: Header=BB0_44 Depth=1
	add	x3, x19, w24, sxtw #2
                                        ; kill: def $w24 killed $w24 killed $x24 def $x24
	sxtw	x1, w24
	sub	x24, x1, #4                     ; =4
	ldr	w1, [x19, x24, lsl #2]
	ldur	w3, [x3, #-4]
                                        ; kill: def $w24 killed $w24 killed $x24 def $x24
	b	LBB0_119
LBB0_122:                               ;   in Loop: Header=BB0_44 Depth=1
	add	w4, w1, #2                      ; =2
	ldrsw	x7, [x19, w4, sxtw #2]
	add	x5, x16, x7, lsl #32
	add	x6, x21, x7
	mvn	w4, w7
	add	w25, w4, w3
LBB0_123:                               ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	mov	x4, x25
	mov	x22, x7
	ldrb	w26, [x6], #1
	add	x5, x5, x14
	sub	w25, w25, #1                    ; =1
	add	w7, w7, #1                      ; =1
	cmp	w26, #40                        ; =40
	b.ne	LBB0_123
; %bb.124:                              ;   in Loop: Header=BB0_44 Depth=1
                                        ; kill: def $w3 killed $w3 killed $x3 def $x3
	sxtw	x3, w3
	sxtw	x5, w22
	sub	x3, x3, x5
	add	x5, x13, x5
LBB0_125:                               ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrb	w6, [x5]
	and	w6, w6, #0xfe
	cmp	w6, #40                         ; =40
	b.eq	LBB0_127
; %bb.126:                              ;   in Loop: Header=BB0_125 Depth=2
	sub	w4, w4, #1                      ; =1
	add	x5, x5, #1                      ; =1
	subs	x3, x3, #1                      ; =1
	b.ne	LBB0_125
	b	LBB0_128
LBB0_127:                               ;   in Loop: Header=BB0_44 Depth=1
	cbnz	w4, LBB0_120
LBB0_128:                               ;   in Loop: Header=BB0_44 Depth=1
	sxtw	x5, w1
	sxtw	x4, w24
	add	x3, x17, x5, lsl #32
	add	x5, x5, #4                      ; =4
LBB0_129:                               ;   Parent Loop BB0_44 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	cmp	x5, x4
	b.ge	LBB0_120
; %bb.130:                              ;   in Loop: Header=BB0_129 Depth=2
	ldr	w6, [x19, x5, lsl #2]
	add	x3, x3, x0
	add	w1, w1, #4                      ; =4
	add	x5, x5, #4                      ; =4
	cmp	w6, #2, lsl #12                 ; =8192
	b.ne	LBB0_129
; %bb.131:                              ;   in Loop: Header=BB0_44 Depth=1
	asr	x3, x3, #30
	ldr	w3, [x19, x3]
	b	LBB0_44
LBB0_132:
	mov	x8, #0
LBB0_133:
	mov	w13, #0
	mov	w12, #0
	mov	w9, #0
	cmp	w10, #1                         ; =1
	b.lt	LBB0_139
; %bb.134:
	cmp	w23, #1                         ; =1
	b.lt	LBB0_139
; %bb.135:
	mov	x13, #0
	mov	w9, #0
	mov	w12, #0
	mov	w10, w10
LBB0_136:                               ; =>This Inner Loop Header: Depth=1
	ldrb	w14, [x21, x13]
	add	x13, x13, #1                    ; =1
	cmp	w14, #10                        ; =10
	cinc	w12, w12, eq
	csinc	w9, wzr, w9, eq
	cmp	x13, x10
	b.hs	LBB0_138
; %bb.137:                              ;   in Loop: Header=BB0_136 Depth=1
	cmp	x13, x20
	b.lt	LBB0_136
LBB0_138:
                                        ; kill: def $w13 killed $w13 killed $x13 def $x13
LBB0_139:
	mov	w17, #0
	cmp	w11, #1                         ; =1
	b.lt	LBB0_145
; %bb.140:
	cmp	w23, #1                         ; =1
	b.lt	LBB0_145
; %bb.141:
	mov	x17, #0
	mov	w16, #0
	mov	w10, #0
	mov	w11, w11
LBB0_142:                               ; =>This Inner Loop Header: Depth=1
	ldrb	w14, [x21, x17]
	add	x17, x17, #1                    ; =1
	cmp	w14, #10                        ; =10
	cinc	w10, w10, eq
	csinc	w16, wzr, w16, eq
	cmp	x17, x11
	b.hs	LBB0_144
; %bb.143:                              ;   in Loop: Header=BB0_142 Depth=1
	cmp	x17, x20
	b.lt	LBB0_142
LBB0_144:
                                        ; kill: def $w17 killed $w17 killed $x17 def $x17
	b	LBB0_12
LBB0_145:
	mov	x10, x17
	mov	x16, x17
	b	LBB0_12
LBB0_146:
Lloh30:
	adrp	x0, l_.str.41@PAGE
Lloh31:
	add	x0, x0, l_.str.41@PAGEOFF
	bl	_puts
	mov	w0, #256
	bl	_malloc
	str	x0, [sp, #88]                   ; 8-byte Folded Spill
	cmn	w24, #3                         ; =3
	b.lt	LBB0_168
; %bb.147:
	mov	x27, #0
	add	x8, x21, #1                     ; =1
	str	x8, [sp, #80]                   ; 8-byte Folded Spill
Lloh32:
	adrp	x23, l_.str.43@PAGE
Lloh33:
	add	x23, x23, l_.str.43@PAGEOFF
Lloh34:
	adrp	x22, l_.str.44@PAGE
Lloh35:
	add	x22, x22, l_.str.44@PAGEOFF
	mov	x25, #4294967296
	b	LBB0_151
LBB0_148:                               ;   in Loop: Header=BB0_151 Depth=1
	mov	w1, #0
	mov	x23, x22
Lloh36:
	adrp	x22, l_.str.44@PAGE
Lloh37:
	add	x22, x22, l_.str.44@PAGEOFF
LBB0_149:                               ;   in Loop: Header=BB0_151 Depth=1
	ldr	x0, [sp, #88]                   ; 8-byte Folded Reload
                                        ; kill: def $w1 killed $w1 killed $x1
	bl	_print_vector
LBB0_150:                               ;   in Loop: Header=BB0_151 Depth=1
	add	x8, x27, #4                     ; =4
	cmp	x27, x26
	mov	x27, x8
	b.ge	LBB0_168
LBB0_151:                               ; =>This Loop Header: Depth=1
                                        ;     Child Loop BB0_157 Depth 2
                                        ;     Child Loop BB0_160 Depth 2
                                        ;       Child Loop BB0_161 Depth 3
                                        ;       Child Loop BB0_163 Depth 3
                                        ;     Child Loop BB0_153 Depth 2
	ldr	w8, [x19, x27, lsl #2]
	cmp	w8, #2, lsl #12                 ; =8192
	b.ne	LBB0_155
; %bb.152:                              ;   in Loop: Header=BB0_151 Depth=1
	str	x27, [sp]
	mov	x0, x23
	bl	_printf
	lsl	x8, x27, #2
	orr	x9, x8, #0x4
	orr	x28, x8, #0x8
	orr	x10, x8, #0xc
	ldr	w8, [x19, x8]
	ldr	w9, [x19, x9]
	ldr	w11, [x19, x28]
	ldr	w10, [x19, x10]
	stp	x11, x10, [sp, #24]
	stp	x8, x9, [sp, #8]
	str	x27, [sp]
	mov	x0, x22
	bl	_printf
	mov	w24, #0
	ldrsw	x8, [x19, x28]
	add	x28, x21, x8
LBB0_153:                               ;   Parent Loop BB0_151 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrsb	w0, [x28]
	bl	_putchar
	ldrb	w8, [x28], #1
	cmp	w8, #40                         ; =40
	cinc	w9, w24, eq
	cmp	w8, #41                         ; =41
	cset	w10, eq
	sub	w24, w9, w10
	cmp	w8, #41                         ; =41
	ccmp	w24, #0, #0, eq
	b.ne	LBB0_153
; %bb.154:                              ;   in Loop: Header=BB0_151 Depth=1
	mov	w0, #10
	bl	_putchar
	b	LBB0_150
LBB0_155:                               ;   in Loop: Header=BB0_151 Depth=1
	orr	x24, x27, #0x3
	ldrsw	x8, [x19, x24, lsl #2]
	ldrb	w8, [x21, x8]
	cmp	w8, #41                         ; =41
	b.ne	LBB0_150
; %bb.156:                              ;   in Loop: Header=BB0_151 Depth=1
	str	x27, [sp]
	mov	x22, x23
	mov	x0, x23
	bl	_printf
	lsl	x23, x27, #2
	orr	x8, x23, #0x4
	orr	x9, x23, #0x8
	ldr	w10, [x19, x23]
	ldr	w8, [x19, x8]
	ldr	w9, [x19, x9]
	ldr	w11, [x19, x24, lsl #2]
	stp	x9, x11, [sp, #24]
	stp	x10, x8, [sp, #8]
	str	x27, [sp]
Lloh38:
	adrp	x0, l_.str.46@PAGE
Lloh39:
	add	x0, x0, l_.str.46@PAGEOFF
	bl	_printf
	mov	w28, #0
	ldrsw	x8, [x19, x23]
	add	x8, x19, x8, lsl #2
	ldrsw	x8, [x8, #8]
	add	x24, x21, x8
LBB0_157:                               ;   Parent Loop BB0_151 Depth=1
                                        ; =>  This Inner Loop Header: Depth=2
	ldrsb	w0, [x24]
	bl	_putchar
	ldrb	w8, [x24], #1
	cmp	w8, #40                         ; =40
	cinc	w9, w28, eq
	cmp	w8, #41                         ; =41
	cset	w10, eq
	sub	w28, w9, w10
	cmp	w8, #41                         ; =41
	ccmp	w28, #0, #0, eq
	b.ne	LBB0_157
; %bb.158:                              ;   in Loop: Header=BB0_151 Depth=1
	mov	w0, #10
	bl	_putchar
	ldr	w10, [x19, x27, lsl #2]
	cmp	w10, #2, lsl #12                ; =8192
	b.eq	LBB0_148
; %bb.159:                              ;   in Loop: Header=BB0_151 Depth=1
	mov	x8, #0
	mov	x11, x27
	mov	x23, x22
Lloh40:
	adrp	x22, l_.str.44@PAGE
Lloh41:
	add	x22, x22, l_.str.44@PAGEOFF
LBB0_160:                               ;   Parent Loop BB0_151 Depth=1
                                        ; =>  This Loop Header: Depth=2
                                        ;       Child Loop BB0_161 Depth 3
                                        ;       Child Loop BB0_163 Depth 3
	sxtw	x9, w11
	add	x11, x19, w11, sxtw #2
	ldrsw	x11, [x11, #12]
	add	w10, w10, #2                    ; =2
	ldrsw	x14, [x19, w10, sxtw #2]
	mov	x10, #-4294967296
	add	x12, x10, x14, lsl #32
	add	x13, x21, x14
	mvn	w10, w14
	add	w15, w10, w11
LBB0_161:                               ;   Parent Loop BB0_151 Depth=1
                                        ;     Parent Loop BB0_160 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	mov	x10, x15
	mov	x16, x14
	ldrb	w17, [x13], #1
	add	x12, x12, x25
	sub	w15, w15, #1                    ; =1
	add	w14, w14, #1                    ; =1
	cmp	w17, #40                        ; =40
	b.ne	LBB0_161
; %bb.162:                              ;   in Loop: Header=BB0_160 Depth=2
	sxtw	x12, w16
	sub	x11, x11, x12
	ldr	x13, [sp, #80]                  ; 8-byte Folded Reload
	add	x12, x13, x12
LBB0_163:                               ;   Parent Loop BB0_151 Depth=1
                                        ;     Parent Loop BB0_160 Depth=2
                                        ; =>    This Inner Loop Header: Depth=3
	ldrb	w13, [x12]
	and	w13, w13, #0xfe
	cmp	w13, #40                        ; =40
	b.eq	LBB0_165
; %bb.164:                              ;   in Loop: Header=BB0_163 Depth=3
	sub	w10, w10, #1                    ; =1
	add	x12, x12, #1                    ; =1
	subs	x11, x11, #1                    ; =1
	b.ne	LBB0_163
	b	LBB0_167
LBB0_165:                               ;   in Loop: Header=BB0_160 Depth=2
	cbz	w10, LBB0_167
; %bb.166:                              ;   in Loop: Header=BB0_160 Depth=2
	sub	w10, w9, #4                     ; =4
	add	x1, x8, #1                      ; =1
	ldr	x11, [sp, #88]                  ; 8-byte Folded Reload
	str	w10, [x11, x8, lsl #2]
	add	x8, x19, x9, lsl #2
	ldursw	x11, [x8, #-12]
	ldr	w10, [x19, x11, lsl #2]
	mov	x8, x1
                                        ; kill: def $w11 killed $w11 killed $x11 def $x11
	cmp	w10, #2, lsl #12                ; =8192
	b.ne	LBB0_160
	b	LBB0_149
LBB0_167:                               ;   in Loop: Header=BB0_151 Depth=1
	mov	x1, x8
	b	LBB0_149
LBB0_168:
Lloh42:
	adrp	x0, l_str@PAGE
Lloh43:
	add	x0, x0, l_str@PAGEOFF
	bl	_puts
	mov	w23, #4
	mov	w0, #4
	bl	_malloc
	mov	x22, x0
	mov	w8, #24840
	movk	w8, #37120, lsl #16
	str	w8, [x0]
	stur	xzr, [x29, #-112]
Lloh44:
	adrp	x8, lCPI0_0@PAGE
Lloh45:
	ldr	q0, [x8, lCPI0_0@PAGEOFF]
Lloh46:
	adrp	x8, lCPI0_1@PAGE
Lloh47:
	ldr	d1, [x8, lCPI0_1@PAGEOFF]
Lloh48:
	adrp	x8, lCPI0_2@PAGE
Lloh49:
	ldr	q2, [x8, lCPI0_2@PAGEOFF]
	stp	q0, q2, [sp, #96]
Lloh50:
	adrp	x8, l_str.58@PAGE
Lloh51:
	add	x8, x8, l_str.58@PAGEOFF
Lloh52:
	ldr	q0, [x8]
	sub	x8, x29, #176                   ; =176
	stur	q0, [x8, #8]
Lloh53:
	adrp	x9, lCPI0_3@PAGE
Lloh54:
	ldr	q2, [x9, lCPI0_3@PAGEOFF]
	stur	q2, [x8, #24]
	stur	q2, [x8, #40]
	movi.2s	v2, #7
	stur	d1, [x29, #-176]
	stur	d2, [x29, #-120]
	mov	w8, #1
Lloh55:
	adrp	x9, l_str.57@PAGE
	add	x9, x9, l_str.57@PAGEOFF
	stur	w8, [x29, #-112]
	ldr	q1, [x9]
	stp	xzr, xzr, [x29, #-192]
	stp	q1, q0, [x29, #-256]
Lloh56:
	adrp	x8, lCPI0_4@PAGE
Lloh57:
	ldr	q0, [x8, lCPI0_4@PAGEOFF]
Lloh58:
	adrp	x8, lCPI0_5@PAGE
Lloh59:
	ldr	q1, [x8, lCPI0_5@PAGEOFF]
	stp	q0, q1, [x29, #-224]
	str	x23, [sp]
Lloh60:
	adrp	x0, l_.str.50@PAGE
Lloh61:
	add	x0, x0, l_.str.50@PAGEOFF
	bl	_printf
Lloh62:
	adrp	x0, l_.str.51@PAGE
Lloh63:
	add	x0, x0, l_.str.51@PAGEOFF
	mov	w1, #513
	bl	_open
	tbnz	w0, #31, LBB0_173
; %bb.169:
	mov	x23, x0
	add	x1, sp, #96                     ; =96
	mov	w2, #32
	bl	_write
	sub	x1, x29, #176                   ; =176
	mov	x0, x23
	mov	w2, #72
	bl	_write
	sub	x1, x29, #256                   ; =256
	mov	x0, x23
	mov	w2, #80
	bl	_write
	mov	x0, x23
	mov	x1, x22
	mov	w2, #4
	bl	_write
	mov	x0, x23
	bl	_close
	b	LBB0_13
LBB0_170:
Lloh64:
	adrp	x0, l_.str.1@PAGE
Lloh65:
	add	x0, x0, l_.str.1@PAGEOFF
	bl	_perror
	mov	w0, #3
	bl	_exit
LBB0_171:
	bl	___stack_chk_fail
LBB0_172:
Lloh66:
	adrp	x0, l_.str.2@PAGE
Lloh67:
	add	x0, x0, l_.str.2@PAGEOFF
	bl	_perror
	mov	w0, #4
	bl	_exit
LBB0_173:
Lloh68:
	adrp	x0, l_.str.1@PAGE
Lloh69:
	add	x0, x0, l_.str.1@PAGEOFF
	bl	_perror
	mov	w0, #4
	bl	_exit
	.loh AdrpLdrGotLdr	Lloh0, Lloh1, Lloh2
	.loh AdrpAdd	Lloh3, Lloh4
	.loh AdrpLdrGotLdr	Lloh7, Lloh8, Lloh9
	.loh AdrpAdd	Lloh5, Lloh6
	.loh AdrpAdd	Lloh13, Lloh14
	.loh AdrpLdrGotLdr	Lloh10, Lloh11, Lloh12
	.loh AdrpLdrGotLdr	Lloh15, Lloh16, Lloh17
	.loh AdrpAdd	Lloh18, Lloh19
	.loh AdrpAdd	Lloh26, Lloh27
	.loh AdrpAdd	Lloh24, Lloh25
	.loh AdrpAdd	Lloh22, Lloh23
	.loh AdrpAdd	Lloh20, Lloh21
	.loh AdrpAdd	Lloh28, Lloh29
	.loh AdrpAdd	Lloh30, Lloh31
	.loh AdrpAdd	Lloh34, Lloh35
	.loh AdrpAdd	Lloh32, Lloh33
	.loh AdrpAdd	Lloh36, Lloh37
	.loh AdrpAdd	Lloh38, Lloh39
	.loh AdrpAdd	Lloh40, Lloh41
	.loh AdrpAdd	Lloh62, Lloh63
	.loh AdrpAdd	Lloh60, Lloh61
	.loh AdrpLdr	Lloh58, Lloh59
	.loh AdrpAdrp	Lloh56, Lloh58
	.loh AdrpLdr	Lloh56, Lloh57
	.loh AdrpAdrp	Lloh53, Lloh55
	.loh AdrpLdr	Lloh53, Lloh54
	.loh AdrpAddLdr	Lloh50, Lloh51, Lloh52
	.loh AdrpAdrp	Lloh48, Lloh50
	.loh AdrpLdr	Lloh48, Lloh49
	.loh AdrpAdrp	Lloh46, Lloh48
	.loh AdrpLdr	Lloh46, Lloh47
	.loh AdrpAdrp	Lloh44, Lloh46
	.loh AdrpLdr	Lloh44, Lloh45
	.loh AdrpAdd	Lloh42, Lloh43
	.loh AdrpAdd	Lloh64, Lloh65
	.loh AdrpAdd	Lloh66, Lloh67
	.loh AdrpAdd	Lloh68, Lloh69
	.cfi_endproc
                                        ; -- End function
	.p2align	2                               ; -- Begin function print_vector
_print_vector:                          ; @print_vector
	.cfi_startproc
; %bb.0:
	sub	sp, sp, #64                     ; =64
	stp	x22, x21, [sp, #16]             ; 16-byte Folded Spill
	stp	x20, x19, [sp, #32]             ; 16-byte Folded Spill
	stp	x29, x30, [sp, #48]             ; 16-byte Folded Spill
	add	x29, sp, #48                    ; =48
	.cfi_def_cfa w29, 16
	.cfi_offset w30, -8
	.cfi_offset w29, -16
	.cfi_offset w19, -24
	.cfi_offset w20, -32
	.cfi_offset w21, -40
	.cfi_offset w22, -48
	mov	x20, x1
	mov	x19, x0
Lloh70:
	adrp	x0, l_.str.53@PAGE
Lloh71:
	add	x0, x0, l_.str.53@PAGEOFF
	bl	_printf
	cmp	w20, #1                         ; =1
	b.lt	LBB1_3
; %bb.1:
	mov	w21, w20
Lloh72:
	adrp	x20, l_.str.54@PAGE
Lloh73:
	add	x20, x20, l_.str.54@PAGEOFF
LBB1_2:                                 ; =>This Inner Loop Header: Depth=1
	ldr	w8, [x19], #4
	str	x8, [sp]
	mov	x0, x20
	bl	_printf
	subs	x21, x21, #1                    ; =1
	b.ne	LBB1_2
LBB1_3:
Lloh74:
	adrp	x0, l_str.59@PAGE
Lloh75:
	add	x0, x0, l_str.59@PAGEOFF
	ldp	x29, x30, [sp, #48]             ; 16-byte Folded Reload
	ldp	x20, x19, [sp, #32]             ; 16-byte Folded Reload
	ldp	x22, x21, [sp, #16]             ; 16-byte Folded Reload
	add	sp, sp, #64                     ; =64
	b	_puts
	.loh AdrpAdd	Lloh70, Lloh71
	.loh AdrpAdd	Lloh72, Lloh73
	.loh AdrpAdd	Lloh74, Lloh75
	.cfi_endproc
                                        ; -- End function
	.section	__TEXT,__cstring,cstring_literals
l_.str:                                 ; @.str
	.asciz	"usage: ./compiler <input>\n"

l_.str.1:                               ; @.str.1
	.asciz	"open"

l_.str.2:                               ; @.str.2
	.asciz	"mmap"

l_.str.4:                               ; @.str.4
	.asciz	"expected ("

l_.str.6:                               ; @.str.6
	.asciz	"eof after ( in sig"

l_.str.9:                               ; @.str.9
	.asciz	"expected argument in sig"

l_.str.11:                              ; @.str.11
	.asciz	"expected ) in arg"

l_.str.14:                              ; @.str.14
	.asciz	"expected )"

l_.str.16:                              ; @.str.16
	.asciz	"expected char in sig"

l_.str.39:                              ; @.str.39
	.asciz	"unresolved expression"

l_.str.41:                              ; @.str.41
	.asciz	"success: compile successful."

l_.str.43:                              ; @.str.43
	.asciz	"\n\n\n------------------------- %d ---------------------------\n"

l_.str.44:                              ; @.str.44
	.asciz	" %10d : %10di %10dp %10db %10dd   : UDS :   "

l_.str.46:                              ; @.str.46
	.asciz	" %10d : %10di %10dp %10db %10dd   :   "

l_.str.50:                              ; @.str.50
	.asciz	"\n\n--> outputting %zd bytes to output file...\n\n"

l_.str.51:                              ; @.str.51
	.asciz	"object.o"

l_.str.52:                              ; @.str.52
	.asciz	"%u %u %u  %u %u %u  %u %u  %s\n"

l_.str.53:                              ; @.str.53
	.asciz	"{ "

l_.str.54:                              ; @.str.54
	.asciz	"%d "

l_str:                                  ; @str
	.asciz	"outputting executable..."

	.section	__TEXT,__const
l_str.57:                               ; @str.57
	.asciz	"__text\000\000\000\000\000\000\000\000\000\000"

l_str.58:                               ; @str.58
	.asciz	"__TEXT\000\000\000\000\000\000\000\000\000\000"

	.section	__TEXT,__cstring,cstring_literals
l_str.59:                               ; @str.59
	.asciz	"}"

.subsections_via_symbols
