	.text
	.attribute	4, 16
	.attribute	5, "rv64i2p0_m2p0_a2p0_f2p0_d2p0_c2p0"
	.file	"c.c"
	.globl	main                            # -- Begin function main
	.p2align	1
	.type	main,@function
main:                                   # @main
# %bb.0:
	addi	sp, sp, -2032
	sd	ra, 2024(sp)                    # 8-byte Folded Spill
	sd	s0, 2016(sp)                    # 8-byte Folded Spill
	sd	s1, 2008(sp)                    # 8-byte Folded Spill
	sd	s2, 2000(sp)                    # 8-byte Folded Spill
	sd	s3, 1992(sp)                    # 8-byte Folded Spill
	sd	s4, 1984(sp)                    # 8-byte Folded Spill
	sd	s5, 1976(sp)                    # 8-byte Folded Spill
	sd	s6, 1968(sp)                    # 8-byte Folded Spill
	sd	s7, 1960(sp)                    # 8-byte Folded Spill
	sd	s8, 1952(sp)                    # 8-byte Folded Spill
	sd	s9, 1944(sp)                    # 8-byte Folded Spill
	sd	s10, 1936(sp)                   # 8-byte Folded Spill
	sd	s11, 1928(sp)                   # 8-byte Folded Spill
	addi	sp, sp, -2048
	addi	sp, sp, -288
	li	a2, 2
	sd	a1, 136(sp)                     # 8-byte Folded Spill
	bne	a0, a2, .LBB0_132
# %bb.1:
	lui	a0, %hi(arg_count)
	ld	a1, %lo(arg_count)(a0)
	li	s9, 0
	lui	a0, %hi(name_count)
	ld	s8, %lo(name_count)(a0)
	sd	a1, 160(sp)                     # 8-byte Folded Spill
	slli	a0, a1, 3
	lui	a1, %hi(arguments)
	addi	a1, a1, %lo(arguments)
	add	a0, a0, a1
	addi	a0, a0, -8
	sd	a0, 144(sp)                     # 8-byte Folded Spill
	lui	a0, %hi(spelling)
	addi	a0, a0, %lo(spelling)
	sd	a0, 152(sp)                     # 8-byte Folded Spill
	lui	a0, %hi(lengths)
	addi	s2, a0, %lo(lengths)
	lui	a0, %hi(values)
	addi	s10, a0, %lo(values)
	lui	a0, %hi(names)
	addi	s7, a0, %lo(names)
	li	s11, 33
	j	.LBB0_3
.LBB0_2:                                #   in Loop: Header=BB0_3 Depth=1
	sd	a0, 0(s3)
	addi	s9, s9, 1
	addi	s8, s8, 1
	li	a0, 61
	beq	s9, a0, .LBB0_14
.LBB0_3:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_5 Depth 2
                                        #     Child Loop BB0_12 Depth 2
	slli	a0, s9, 3
	ld	a1, 152(sp)                     # 8-byte Folded Reload
	add	a0, a0, a1
	ld	s1, 0(a0)
	mv	a0, s1
	call	strlen
	mv	s0, a0
	li	a1, 0
	beqz	s8, .LBB0_8
# %bb.4:                                #   in Loop: Header=BB0_3 Depth=1
	mv	a0, s2
.LBB0_5:                                #   Parent Loop BB0_3 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	ld	a2, 0(a0)
	bgeu	s0, a2, .LBB0_8
# %bb.6:                                #   in Loop: Header=BB0_5 Depth=2
	addi	a1, a1, 1
	addi	a0, a0, 8
	bne	s8, a1, .LBB0_5
# %bb.7:                                #   in Loop: Header=BB0_3 Depth=1
	mv	a1, s8
.LBB0_8:                                #   in Loop: Header=BB0_3 Depth=1
	slli	s5, a1, 3
	add	s4, s2, s5
	addi	a0, s4, 8
	sub	s6, s8, a1
	slli	s6, s6, 3
	mv	a1, s4
	mv	a2, s6
	call	memmove@plt
	add	s3, s10, s5
	addi	a0, s3, 8
	mv	a1, s3
	mv	a2, s6
	call	memmove@plt
	add	s5, s5, s7
	addi	a0, s5, 8
	mv	a1, s5
	mv	a2, s6
	call	memmove@plt
	addi	a0, s0, 1
	li	a1, 1
	call	calloc
	li	a1, 0
	bnez	s0, .LBB0_12
.LBB0_9:                                #   in Loop: Header=BB0_3 Depth=1
	sd	a1, 0(s4)
	sd	a0, 0(s5)
	ld	a0, 160(sp)                     # 8-byte Folded Reload
	beqz	a0, .LBB0_2
# %bb.10:                               #   in Loop: Header=BB0_3 Depth=1
	ld	a0, 144(sp)                     # 8-byte Folded Reload
	ld	a0, 0(a0)
	j	.LBB0_2
.LBB0_11:                               #   in Loop: Header=BB0_12 Depth=2
	addi	s0, s0, -1
	addi	s1, s1, 1
	beqz	s0, .LBB0_9
.LBB0_12:                               #   Parent Loop BB0_3 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	lbu	a2, 0(s1)
	bltu	a2, s11, .LBB0_11
# %bb.13:                               #   in Loop: Header=BB0_12 Depth=2
	add	a3, a0, a1
	addi	a1, a1, 1
	sb	a2, 0(a3)
	j	.LBB0_11
.LBB0_14:
	lui	a0, %hi(name_count)
	sd	s8, %lo(name_count)(a0)
	ld	a0, 136(sp)                     # 8-byte Folded Reload
	ld	s3, 8(a0)
	lui	a0, %hi(text_length)
	sd	zero, %lo(text_length)(a0)
	lui	a1, 16
	mv	a0, s3
	call	open
	bgez	a0, .LBB0_133
# %bb.15:
	mv	a0, s3
	li	a1, 0
	li	a2, 0
	call	open
	bltz	a0, .LBB0_134
# %bb.16:
	mv	s1, a0
	li	a2, 2
	li	a1, 0
	call	lseek
	mv	s0, a0
	call	malloc
	mv	s2, a0
	mv	a0, s1
	li	a1, 0
	li	a2, 0
	call	lseek
	mv	a0, s1
	mv	a1, s2
	mv	a2, s0
	call	read
	mv	a0, s1
	call	close
	lui	s1, %hi(text_length)
	sd	s0, %lo(text_length)(s1)
	lui	s4, %hi(text)
	sd	s2, %lo(text)(s4)
	lui	a0, %hi(.L.str.1)
	addi	a0, a0, %lo(.L.str.1)
	mv	a1, s0
	call	printf
	ld	a0, %lo(text)(s4)
	ld	a2, %lo(text_length)(s1)
	lui	a1, %hi(stdout)
	ld	a3, %lo(stdout)(a1)
	li	a1, 1
	call	fwrite
	lui	a0, %hi(.L.str.2)
	addi	a0, a0, %lo(.L.str.2)
	call	puts
	lui	a0, 16
	li	a1, 8
	call	calloc
	mv	s0, a0
	lui	a0, %hi(array)
	sd	s0, %lo(array)(a0)
	lui	a0, 16
	li	a1, 8
	call	calloc
	lui	a1, %hi(file_count)
	ld	a2, %lo(file_count)(a1)
	sd	a0, 16(s0)
	li	a0, 24
	mul	a0, a2, a0
	lui	a3, %hi(files)
	ld	a4, %lo(text_length)(s1)
	addi	a3, a3, %lo(files)
	add	a0, a0, a3
	sd	s3, 16(a0)
	sd	a4, 8(a0)
	lui	a4, %hi(arg_count)
	ld	a3, %lo(arg_count)(a4)
	addi	a2, a2, 1
	sd	a2, %lo(file_count)(a1)
	sd	zero, 0(a0)
	addi	a0, a3, 1
	sd	a0, %lo(arg_count)(a4)
	lui	a1, %hi(arguments)
	lui	a0, %hi(name_count)
	ld	a7, %lo(name_count)(a0)
	addi	a0, a1, %lo(arguments)
	slli	a3, a3, 3
	sd	a0, 160(sp)                     # 8-byte Folded Spill
	add	a3, a3, a0
	sd	zero, 0(a3)
	beqz	a7, .LBB0_136
# %bb.17:
	li	a1, 0
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	li	s2, 0
	lui	a2, %hi(lengths)
	addi	s11, a2, %lo(lengths)
	lui	a2, %hi(names)
	addi	s8, a2, %lo(names)
	lui	a2, %hi(.L.str.4)
	addi	a0, a2, %lo(.L.str.4)
	sd	a0, 152(sp)                     # 8-byte Folded Spill
	lui	a2, %hi(spelling)
	addi	a0, a2, %lo(spelling)
	sd	a0, 144(sp)                     # 8-byte Folded Spill
	lui	a2, %hi(.L.str.7)
	addi	a0, a2, %lo(.L.str.7)
	sd	a0, 136(sp)                     # 8-byte Folded Spill
	lui	a2, %hi(.L.str.14)
	addi	a0, a2, %lo(.L.str.14)
	sd	a0, 24(sp)                      # 8-byte Folded Spill
	lui	a2, %hi(.L.str.15)
	addi	a0, a2, %lo(.L.str.15)
	sd	a0, 16(sp)                      # 8-byte Folded Spill
	lui	a2, %hi(.L.str.16)
	addi	a0, a2, %lo(.L.str.16)
	sd	a0, 40(sp)                      # 8-byte Folded Spill
	lui	a2, %hi(.LJTI0_1)
	addi	a0, a2, %lo(.LJTI0_1)
	sd	a0, 32(sp)                      # 8-byte Folded Spill
	lui	a2, %hi(.LJTI0_0)
	addi	a0, a2, %lo(.LJTI0_0)
	sd	a0, 104(sp)                     # 8-byte Folded Spill
	lui	a2, %hi(.L.str.12)
	addi	a0, a2, %lo(.L.str.12)
	sd	a0, 88(sp)                      # 8-byte Folded Spill
	lui	a2, %hi(.L.str.13)
	addi	a0, a2, %lo(.L.str.13)
	sd	a0, 80(sp)                      # 8-byte Folded Spill
	lui	a2, %hi(.L.str.11)
	addi	a0, a2, %lo(.L.str.11)
	sd	a0, 72(sp)                      # 8-byte Folded Spill
	lui	a2, %hi(.L.str.9)
	addi	a0, a2, %lo(.L.str.9)
	sd	a0, 64(sp)                      # 8-byte Folded Spill
	lui	a2, %hi(.L.str.10)
	addi	a0, a2, %lo(.L.str.10)
	sd	a0, 56(sp)                      # 8-byte Folded Spill
	lui	a2, %hi(.L.str.8)
	addi	a0, a2, %lo(.L.str.8)
	sd	a0, 48(sp)                      # 8-byte Folded Spill
	lui	a2, %hi(.L.str.5)
	addi	a0, a2, %lo(.L.str.5)
	sd	a0, 128(sp)                     # 8-byte Folded Spill
	lui	a2, %hi(values)
	addi	a0, a2, %lo(values)
	sd	a0, 120(sp)                     # 8-byte Folded Spill
	lui	a2, %hi(.L.str.6)
	addi	a0, a2, %lo(.L.str.6)
	sd	a0, 112(sp)                     # 8-byte Folded Spill
	li	s6, 61
	li	s3, 33
	li	s5, 0
.LBB0_18:                               # =>This Loop Header: Depth=1
                                        #     Child Loop BB0_20 Depth 2
                                        #       Child Loop BB0_29 Depth 3
                                        #     Child Loop BB0_38 Depth 2
	ld	a6, %lo(text)(s4)
	ld	a4, %lo(text_length)(s1)
	li	s1, 0
	li	s9, 0
	add	a5, a6, a1
	j	.LBB0_20
.LBB0_19:                               #   in Loop: Header=BB0_20 Depth=2
	mv	s2, s0
	bgeu	s9, a7, .LBB0_130
.LBB0_20:                               #   Parent Loop BB0_18 Depth=1
                                        # =>  This Loop Header: Depth=2
                                        #       Child Loop BB0_29 Depth 3
	slli	a2, s9, 3
	add	s0, s11, a2
	ld	a3, 0(s0)
	beq	s1, a3, .LBB0_37
# %bb.21:                               #   in Loop: Header=BB0_20 Depth=2
	bgeu	s5, a4, .LBB0_114
# %bb.22:                               #   in Loop: Header=BB0_20 Depth=2
	add	a3, a6, s5
	lbu	a3, 0(a3)
	bltu	a3, s3, .LBB0_25
# %bb.23:                               #   in Loop: Header=BB0_20 Depth=2
	add	a2, a2, s8
	ld	a0, 0(a2)
	add	a0, a0, s1
	lbu	a0, 0(a0)
	bne	a0, a3, .LBB0_27
.LBB0_24:                               #   in Loop: Header=BB0_20 Depth=2
	addi	s1, s1, 1
.LBB0_25:                               #   in Loop: Header=BB0_20 Depth=2
	addi	s5, s5, 1
	mv	s0, s5
	bltu	s2, s5, .LBB0_19
.LBB0_26:                               #   in Loop: Header=BB0_20 Depth=2
	mv	s0, s2
	j	.LBB0_19
.LBB0_27:                               #   in Loop: Header=BB0_20 Depth=2
	addi	s9, s9, 1
	bgeu	s9, a7, .LBB0_129
# %bb.28:                               #   in Loop: Header=BB0_20 Depth=2
	addi	s1, a2, 8
	addi	s0, s0, 8
.LBB0_29:                               #   Parent Loop BB0_18 Depth=1
                                        #     Parent Loop BB0_20 Depth=2
                                        # =>    This Inner Loop Header: Depth=3
	ld	a2, 0(s0)
	beqz	a2, .LBB0_36
# %bb.30:                               #   in Loop: Header=BB0_29 Depth=3
	bgeu	a1, a4, .LBB0_113
# %bb.31:                               #   in Loop: Header=BB0_29 Depth=3
	lbu	a2, 0(a5)
	bltu	a2, s3, .LBB0_34
# %bb.32:                               #   in Loop: Header=BB0_29 Depth=3
	ld	a3, 0(s1)
	lbu	a3, 0(a3)
	beq	a3, a2, .LBB0_35
# %bb.33:                               #   in Loop: Header=BB0_29 Depth=3
	addi	s9, s9, 1
	addi	s1, s1, 8
	addi	s0, s0, 8
	bltu	s9, a7, .LBB0_29
	j	.LBB0_129
.LBB0_34:                               #   in Loop: Header=BB0_20 Depth=2
	li	s1, 0
	mv	s5, a1
	addi	s5, s5, 1
	mv	s0, s5
	bgeu	s2, s5, .LBB0_26
	j	.LBB0_19
.LBB0_35:                               #   in Loop: Header=BB0_20 Depth=2
	li	s1, 0
	mv	s5, a1
	j	.LBB0_24
.LBB0_36:                               #   in Loop: Header=BB0_18 Depth=1
	mv	s5, a1
.LBB0_37:                               #   in Loop: Header=BB0_18 Depth=1
	slli	s9, s9, 3
	add	s0, s8, s9
	ld	a1, 0(s0)
	ld	a0, 152(sp)                     # 8-byte Folded Reload
	call	printf
	ld	s1, 0(s0)
	li	s7, 0
	li	s4, 0
	ld	s0, 144(sp)                     # 8-byte Folded Reload
.LBB0_38:                               #   Parent Loop BB0_18 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	ld	s10, 0(s0)
	mv	a0, s10
	mv	a1, s1
	call	strcmp
	beqz	a0, .LBB0_43
# %bb.39:                               #   in Loop: Header=BB0_38 Depth=2
	addi	s4, s4, 1
	addiw	s7, s7, 1
	addi	s0, s0, 8
	bne	s4, s6, .LBB0_38
# %bb.40:                               #   in Loop: Header=BB0_18 Depth=1
	ld	a0, 128(sp)                     # 8-byte Folded Reload
	mv	a1, s1
	call	printf
	lui	a3, %hi(arg_count)
	ld	a0, %lo(arg_count)(a3)
	ld	a1, 120(sp)                     # 8-byte Folded Reload
	add	s9, s9, a1
	ld	a1, 0(s9)
	addi	a2, a0, 1
	sd	a2, %lo(arg_count)(a3)
	slli	a0, a0, 3
	ld	a2, 160(sp)                     # 8-byte Folded Reload
	add	a0, a0, a2
	sd	a1, 0(a0)
	ld	a0, 112(sp)                     # 8-byte Folded Reload
	call	printf
.LBB0_41:                               #   in Loop: Header=BB0_18 Depth=1
	lui	s1, %hi(text_length)
	lui	s4, %hi(text)
.LBB0_42:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(name_count)
	ld	a7, %lo(name_count)(a0)
	mv	a1, s5
	bnez	a7, .LBB0_18
	j	.LBB0_131
.LBB0_43:                               #   in Loop: Header=BB0_18 Depth=1
	ld	a0, 136(sp)                     # 8-byte Folded Reload
	mv	a1, s10
	call	printf
	lui	a0, %hi(arg_count)
	ld	a0, %lo(arg_count)(a0)
	beqz	a0, .LBB0_46
# %bb.44:                               #   in Loop: Header=BB0_18 Depth=1
	slli	a1, a0, 3
	ld	a2, 160(sp)                     # 8-byte Folded Reload
	add	a1, a1, a2
	ld	s9, -8(a1)
	li	a2, 1
	bne	a0, a2, .LBB0_47
# %bb.45:                               #   in Loop: Header=BB0_18 Depth=1
	li	s1, 0
	j	.LBB0_49
.LBB0_46:                               #   in Loop: Header=BB0_18 Depth=1
	li	s1, 0
	li	s9, 0
	j	.LBB0_49
.LBB0_47:                               #   in Loop: Header=BB0_18 Depth=1
	ld	s1, -16(a1)
	li	a2, 3
	bltu	a0, a2, .LBB0_49
# %bb.48:                               #   in Loop: Header=BB0_18 Depth=1
	ld	s0, -24(a1)
	j	.LBB0_50
.LBB0_49:                               #   in Loop: Header=BB0_18 Depth=1
	li	s0, 0
.LBB0_50:                               #   in Loop: Header=BB0_18 Depth=1
	addi	a0, s4, -1
	li	a1, 10
	bltu	a1, a0, .LBB0_58
# %bb.51:                               #   in Loop: Header=BB0_18 Depth=1
	slli	a0, a0, 2
	ld	a1, 104(sp)                     # 8-byte Folded Reload
	add	a0, a0, a1
	lw	a0, 0(a0)
	jr	a0
.LBB0_52:                               #   in Loop: Header=BB0_18 Depth=1
	ld	a0, 88(sp)                      # 8-byte Folded Reload
	call	puts
	lui	a3, %hi(arg_count)
	ld	a0, %lo(arg_count)(a3)
	slli	a1, a0, 3
	ld	a2, 160(sp)                     # 8-byte Folded Reload
	add	a1, a1, a2
	ld	a2, -8(a1)
	addi	a0, a0, 1
	sd	a0, %lo(arg_count)(a3)
	sd	a2, 0(a1)
	j	.LBB0_41
.LBB0_53:                               #   in Loop: Header=BB0_18 Depth=1
	ld	a0, 48(sp)                      # 8-byte Folded Reload
	call	puts
	li	a0, 1
	sd	a0, 96(sp)                      # 8-byte Folded Spill
	j	.LBB0_41
.LBB0_54:                               #   in Loop: Header=BB0_18 Depth=1
	ld	a0, 80(sp)                      # 8-byte Folded Reload
	call	puts
	lui	a0, %hi(arg_count)
	ld	a0, %lo(arg_count)(a0)
	slli	a0, a0, 3
	ld	a1, 160(sp)                     # 8-byte Folded Reload
	add	a0, a0, a1
	ld	a1, -16(a0)
	ld	a2, -8(a0)
	sd	a1, -8(a0)
	sd	a2, -16(a0)
	j	.LBB0_41
.LBB0_55:                               #   in Loop: Header=BB0_18 Depth=1
	ld	a0, 72(sp)                      # 8-byte Folded Reload
	call	puts
	lui	a1, %hi(arg_count)
	ld	a0, %lo(arg_count)(a1)
	addi	a0, a0, -1
	sd	a0, %lo(arg_count)(a1)
	j	.LBB0_41
.LBB0_56:                               #   in Loop: Header=BB0_18 Depth=1
	ld	a0, 64(sp)                      # 8-byte Folded Reload
	call	puts
	lui	a0, %hi(arg_count)
	ld	a0, %lo(arg_count)(a0)
	slli	a0, a0, 3
	ld	a1, 160(sp)                     # 8-byte Folded Reload
	add	a0, a0, a1
	ld	a1, -8(a0)
	addi	a1, a1, 1
	sd	a1, -8(a0)
	j	.LBB0_41
.LBB0_57:                               #   in Loop: Header=BB0_18 Depth=1
	ld	a0, 56(sp)                      # 8-byte Folded Reload
	call	puts
	lui	a0, %hi(arg_count)
	ld	a0, %lo(arg_count)(a0)
	slli	a0, a0, 3
	ld	a1, 160(sp)                     # 8-byte Folded Reload
	add	a0, a0, a1
	sd	zero, -8(a0)
	j	.LBB0_41
.LBB0_58:                               #   in Loop: Header=BB0_18 Depth=1
	ld	a0, 96(sp)                      # 8-byte Folded Reload
	beqz	a0, .LBB0_62
# %bb.59:                               #   in Loop: Header=BB0_18 Depth=1
	ld	a0, 40(sp)                      # 8-byte Folded Reload
	mv	a1, s4
	mv	a2, s10
	mv	a3, s9
	mv	a4, s1
	mv	a5, s0
	call	printf
	addi	a0, s4, -13
	li	a1, 46
	bltu	a1, a0, .LBB0_138
# %bb.60:                               #   in Loop: Header=BB0_18 Depth=1
	slli	a0, a0, 2
	ld	a1, 32(sp)                      # 8-byte Folded Reload
	add	a0, a0, a1
	lw	a0, 0(a0)
	jr	a0
.LBB0_61:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	slli	a2, s0, 3
	add	a2, a2, a0
	ld	a2, 0(a2)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	add	a1, a1, a2
	j	.LBB0_104
.LBB0_62:                               #   in Loop: Header=BB0_18 Depth=1
	ld	a0, 24(sp)                      # 8-byte Folded Reload
	mv	a1, s4
	mv	a2, s10
	call	printf
	lui	a0, %hi(arg_count)
	ld	a0, %lo(arg_count)(a0)
	beqz	a0, .LBB0_64
# %bb.63:                               #   in Loop: Header=BB0_18 Depth=1
	slli	a0, a0, 3
	ld	a1, 160(sp)                     # 8-byte Folded Reload
	add	a0, a0, a1
	lw	a2, -8(a0)
	j	.LBB0_65
.LBB0_64:                               #   in Loop: Header=BB0_18 Depth=1
	li	a2, 0
.LBB0_65:                               #   in Loop: Header=BB0_18 Depth=1
	ld	s0, 16(sp)                      # 8-byte Folded Reload
	li	a1, 1
	mv	a0, s0
	sd	a2, 8(sp)                       # 8-byte Folded Spill
	call	printf
	lui	a0, %hi(arg_count)
	ld	a0, %lo(arg_count)(a0)
	li	a1, 2
	bltu	a0, a1, .LBB0_67
# %bb.66:                               #   in Loop: Header=BB0_18 Depth=1
	slli	a0, a0, 3
	ld	a1, 160(sp)                     # 8-byte Folded Reload
	add	a0, a0, a1
	lw	s10, -16(a0)
	j	.LBB0_68
.LBB0_67:                               #   in Loop: Header=BB0_18 Depth=1
	li	s10, 0
.LBB0_68:                               #   in Loop: Header=BB0_18 Depth=1
	li	a1, 2
	mv	a0, s0
	mv	a2, s10
	call	printf
	lui	a0, %hi(arg_count)
	ld	a0, %lo(arg_count)(a0)
	li	a1, 3
	bltu	a0, a1, .LBB0_70
# %bb.69:                               #   in Loop: Header=BB0_18 Depth=1
	slli	a0, a0, 3
	ld	a1, 160(sp)                     # 8-byte Folded Reload
	add	a0, a0, a1
	lw	s9, -24(a0)
	j	.LBB0_71
.LBB0_70:                               #   in Loop: Header=BB0_18 Depth=1
	li	s9, 0
.LBB0_71:                               #   in Loop: Header=BB0_18 Depth=1
	li	a1, 3
	mv	a0, s0
	mv	a2, s9
	call	printf
	lui	a0, %hi(ins_count)
	ld	s0, %lo(ins_count)(a0)
	lui	s4, %hi(ins)
	ld	a0, %lo(ins)(s4)
	li	a1, 40
	mul	s1, s0, a1
	addi	a1, s1, 40
	call	realloc
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	addi	s0, s0, 1
	lui	a1, %hi(ins_count)
	sd	s0, %lo(ins_count)(a1)
	sd	a0, %lo(ins)(s4)
	add	a0, a0, s1
	sw	s7, 0(a0)
	ld	a1, 8(sp)                       # 8-byte Folded Reload
	sw	a1, 4(a0)
	sw	s10, 8(a0)
	sw	s9, 12(a0)
	sd	s5, 16(a0)
	sd	zero, 24(a0)
	li	a1, 4
	sd	a1, 32(a0)
	j	.LBB0_41
.LBB0_72:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	slli	a2, s0, 3
	add	a2, a2, a0
	ld	a2, 0(a2)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	sub	a1, a1, a2
	j	.LBB0_104
.LBB0_73:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	add	a1, a1, s0
	j	.LBB0_104
.LBB0_74:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	slli	a2, s0, 3
	add	a2, a2, a0
	ld	a2, 0(a2)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	sltu	a1, a1, a2
	j	.LBB0_104
.LBB0_75:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	sltu	a1, a1, s0
	j	.LBB0_104
.LBB0_76:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	slli	a2, s0, 3
	add	a2, a2, a0
	ld	a2, 0(a2)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	and	a1, a1, a2
	j	.LBB0_104
.LBB0_77:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	and	a1, a1, s0
	j	.LBB0_104
.LBB0_78:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	slli	a2, s0, 3
	add	a2, a2, a0
	ld	a2, 0(a2)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	or	a1, a1, a2
	j	.LBB0_104
.LBB0_79:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	or	a1, a1, s0
	j	.LBB0_104
.LBB0_80:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	slli	a2, s0, 3
	add	a2, a2, a0
	ld	a2, 0(a2)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	xor	a1, a1, a2
	j	.LBB0_104
.LBB0_81:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	xor	a1, a1, s0
	j	.LBB0_104
.LBB0_82:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	slli	a2, s0, 3
	add	a2, a2, a0
	ld	a2, 0(a2)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	sll	a1, a1, a2
	j	.LBB0_104
.LBB0_83:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	sll	a1, a1, s0
	j	.LBB0_104
.LBB0_84:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	slli	a2, s0, 3
	add	a2, a2, a0
	ld	a2, 0(a2)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	srl	a1, a1, a2
	j	.LBB0_104
.LBB0_85:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	srl	a1, a1, s0
	j	.LBB0_104
.LBB0_86:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a1, %hi(arg_count)
	ld	a2, %lo(arg_count)(a1)
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	addi	a2, a2, -3
	sd	a2, %lo(arg_count)(a1)
	slli	a1, s9, 3
	add	a1, a1, a0
	ld	a1, 0(a1)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a2, 0(s1)
	bgeu	a1, a2, .LBB0_95
	j	.LBB0_90
.LBB0_87:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a1, %hi(arg_count)
	ld	a2, %lo(arg_count)(a1)
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	addi	a2, a2, -3
	sd	a2, %lo(arg_count)(a1)
	slli	a1, s9, 3
	add	a1, a1, a0
	ld	a1, 0(a1)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a2, 0(s1)
	bgeu	a1, a2, .LBB0_90
	j	.LBB0_95
.LBB0_88:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a1, %hi(arg_count)
	ld	a2, %lo(arg_count)(a1)
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	addi	a2, a2, -3
	sd	a2, %lo(arg_count)(a1)
	slli	a1, s9, 3
	add	a1, a1, a0
	ld	a1, 0(a1)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a2, 0(s1)
	bne	a1, a2, .LBB0_90
	j	.LBB0_95
.LBB0_89:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a1, %hi(arg_count)
	ld	a2, %lo(arg_count)(a1)
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	addi	a2, a2, -3
	sd	a2, %lo(arg_count)(a1)
	slli	a1, s9, 3
	add	a1, a1, a0
	ld	a1, 0(a1)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a2, 0(s1)
	bne	a1, a2, .LBB0_95
.LBB0_90:                               #   in Loop: Header=BB0_18 Depth=1
	slli	a1, s0, 3
	add	a0, a0, a1
	ld	a0, 0(a0)
	lui	s1, %hi(text_length)
	lui	s4, %hi(text)
	beqz	a0, .LBB0_92
# %bb.91:                               #   in Loop: Header=BB0_18 Depth=1
	mv	s5, a0
.LBB0_92:                               #   in Loop: Header=BB0_18 Depth=1
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	j	.LBB0_42
.LBB0_93:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(arg_count)
	ld	a1, %lo(arg_count)(a0)
	lui	a2, %hi(array)
	ld	a2, %lo(array)(a2)
	addi	a1, a1, -2
	sd	a1, %lo(arg_count)(a0)
	slli	a0, s9, 3
	add	a0, a0, a2
	sd	s5, 0(a0)
	slli	s1, s1, 3
	add	a2, a2, s1
	ld	a0, 0(a2)
	beqz	a0, .LBB0_95
# %bb.94:                               #   in Loop: Header=BB0_18 Depth=1
	mv	s5, a0
.LBB0_95:                               #   in Loop: Header=BB0_18 Depth=1
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	j	.LBB0_41
.LBB0_96:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(arg_count)
	ld	a1, %lo(arg_count)(a0)
	lui	a2, %hi(array)
	ld	a2, %lo(array)(a2)
	addi	a1, a1, -3
	sd	a1, %lo(arg_count)(a0)
	slli	a0, s9, 3
	add	a0, a0, a2
	sd	s5, 0(a0)
	slli	s1, s1, 3
	add	a2, a2, s1
	ld	s5, 0(a2)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	add	s5, s5, s0
	j	.LBB0_41
.LBB0_97:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(.L.str.17)
	addi	a0, a0, %lo(.L.str.17)
	call	puts
	li	a0, 2
	bne	s9, a0, .LBB0_127
# %bb.98:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	a0, a0, s1
	ld	a1, 0(a0)
	lui	a0, %hi(.L.str.18)
	addi	a0, a0, %lo(.L.str.18)
	mv	a2, a1
	call	printf
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	j	.LBB0_41
.LBB0_99:                               #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	add	a1, a1, s0
	lbu	a1, 0(a1)
	j	.LBB0_103
.LBB0_100:                              #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	add	a1, a1, s0
	lhu	a1, 0(a1)
	j	.LBB0_103
.LBB0_101:                              #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	add	a1, a1, s0
	lwu	a1, 0(a1)
	j	.LBB0_103
.LBB0_102:                              #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	add	a1, a1, s0
	ld	a1, 0(a1)
.LBB0_103:                              #   in Loop: Header=BB0_18 Depth=1
	sd	zero, 96(sp)                    # 8-byte Folded Spill
.LBB0_104:                              #   in Loop: Header=BB0_18 Depth=1
	slli	a2, s9, 3
	add	a0, a0, a2
.LBB0_105:                              #   in Loop: Header=BB0_18 Depth=1
	sd	a1, 0(a0)
	j	.LBB0_41
.LBB0_106:                              #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	a1, s0, 3
	add	a1, a1, a0
	slli	a2, s9, 3
	add	a0, a0, a2
	ld	a0, 0(a0)
	ld	a1, 0(a1)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	add	a0, a0, s1
	sb	a1, 0(a0)
	j	.LBB0_41
.LBB0_107:                              #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	a1, s0, 3
	add	a1, a1, a0
	slli	a2, s9, 3
	add	a0, a0, a2
	ld	a0, 0(a0)
	ld	a1, 0(a1)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	add	a0, a0, s1
	sh	a1, 0(a0)
	j	.LBB0_41
.LBB0_108:                              #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	a1, s0, 3
	add	a1, a1, a0
	slli	a2, s9, 3
	add	a0, a0, a2
	ld	a0, 0(a0)
	ld	a1, 0(a1)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	add	a0, a0, s1
	sw	a1, 0(a0)
	j	.LBB0_41
.LBB0_109:                              #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	a1, s0, 3
	add	a1, a1, a0
	slli	a2, s9, 3
	add	a0, a0, a2
	ld	a0, 0(a0)
	ld	a1, 0(a1)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	add	a0, a0, s1
	j	.LBB0_105
.LBB0_110:                              #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	slli	a2, s0, 3
	add	a2, a2, a0
	ld	a2, 0(a2)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	mul	a1, a2, a1
	j	.LBB0_104
.LBB0_111:                              #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	slli	a2, s0, 3
	add	a2, a2, a0
	ld	a2, 0(a2)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	divu	a1, a1, a2
	j	.LBB0_104
.LBB0_112:                              #   in Loop: Header=BB0_18 Depth=1
	lui	a0, %hi(array)
	ld	a0, %lo(array)(a0)
	slli	s1, s1, 3
	add	s1, s1, a0
	ld	a1, 0(s1)
	slli	a2, s0, 3
	add	a2, a2, a0
	ld	a2, 0(a2)
	sd	zero, 96(sp)                    # 8-byte Folded Spill
	remu	a1, a1, a2
	j	.LBB0_104
.LBB0_113:
	mv	s5, a1
.LBB0_114:
	lui	a0, %hi(.L.str.23)
	addi	a0, a0, %lo(.L.str.23)
	call	puts
	lui	a0, %hi(text_length)
	ld	a0, %lo(text_length)(a0)
	beq	s5, a0, .LBB0_116
# %bb.115:
	lui	a0, %hi(.Lstr)
	addi	a0, a0, %lo(.Lstr)
	j	.LBB0_117
.LBB0_116:
	lui	a0, %hi(.Lstr.111)
	addi	a0, a0, %lo(.Lstr.111)
.LBB0_117:
	call	puts
	lui	a0, %hi(.Lstr.112)
	addi	a0, a0, %lo(.Lstr.112)
	call	puts
	lui	s2, %hi(arg_count)
	ld	a0, %lo(arg_count)(s2)
	beqz	a0, .LBB0_120
# %bb.118:
	li	s0, 0
	lui	a0, %hi(arguments)
	addi	s1, a0, %lo(arguments)
	lui	a0, %hi(.L.str.102)
	addi	s3, a0, %lo(.L.str.102)
.LBB0_119:                              # =>This Inner Loop Header: Depth=1
	ld	a2, 0(s1)
	mv	a0, s3
	mv	a1, s0
	li	a3, 0
	li	a4, 0
	call	printf
	ld	a0, %lo(arg_count)(s2)
	addi	s0, s0, 1
	addi	s1, s1, 8
	bltu	s0, a0, .LBB0_119
.LBB0_120:
	lui	a0, %hi(.L.str.103)
	addi	a0, a0, %lo(.L.str.103)
	call	puts
	lui	a0, %hi(.L.str.104)
	addi	a0, a0, %lo(.L.str.104)
	call	puts
	lui	s2, %hi(name_count)
	ld	a0, %lo(name_count)(s2)
	beqz	a0, .LBB0_123
# %bb.121:
	li	s0, 0
	lui	a0, %hi(values)
	addi	s4, a0, %lo(values)
	lui	a0, %hi(lengths)
	addi	s5, a0, %lo(lengths)
	lui	a0, %hi(names)
	addi	s1, a0, %lo(names)
	lui	a0, %hi(.L.str.105)
	addi	s3, a0, %lo(.L.str.105)
.LBB0_122:                              # =>This Inner Loop Header: Depth=1
	ld	a2, 0(s1)
	ld	a3, 0(s5)
	ld	a4, 0(s4)
	mv	a0, s3
	mv	a1, s0
	call	printf
	ld	a0, %lo(name_count)(s2)
	addi	s0, s0, 1
	addi	s4, s4, 8
	addi	s5, s5, 8
	addi	s1, s1, 8
	bltu	s0, a0, .LBB0_122
.LBB0_123:
	lui	a0, %hi(.L.str.106)
	addi	a0, a0, %lo(.L.str.106)
	call	puts
	lui	a0, %hi(.Lstr.113)
	addi	a0, a0, %lo(.Lstr.113)
	call	puts
	lui	s4, %hi(ins_count)
	ld	a0, %lo(ins_count)(s4)
	beqz	a0, .LBB0_126
# %bb.124:
	li	s1, 0
	li	s0, 0
	lui	s6, %hi(ins)
	lui	a0, %hi(spelling)
	addi	s7, a0, %lo(spelling)
	lui	a0, %hi(.L.str.108)
	addi	s2, a0, %lo(.L.str.108)
	lui	a0, %hi(.L.str.109)
	addi	s5, a0, %lo(.L.str.109)
	lui	a0, %hi(.L.str.110)
	addi	s3, a0, %lo(.L.str.110)
.LBB0_125:                              # =>This Inner Loop Header: Depth=1
	ld	a0, %lo(ins)(s6)
	add	a0, a0, s1
	lw	a2, 0(a0)
	slli	a1, a2, 32
	srli	a1, a1, 29
	add	a1, a1, s7
	ld	a3, 0(a1)
	ld	a4, 32(a0)
	mv	a0, s2
	mv	a1, s0
	call	printf
	ld	a0, %lo(ins)(s6)
	add	a0, a0, s1
	lwu	a1, 4(a0)
	mv	a0, s5
	call	printf
	ld	a0, %lo(ins)(s6)
	add	a0, a0, s1
	lwu	a1, 8(a0)
	mv	a0, s5
	call	printf
	ld	a0, %lo(ins)(s6)
	add	a0, a0, s1
	lwu	a1, 12(a0)
	mv	a0, s5
	call	printf
	ld	a0, %lo(ins)(s6)
	add	a0, a0, s1
	ld	a1, 16(a0)
	mv	a0, s3
	call	printf
	ld	a0, %lo(ins_count)(s4)
	addi	s0, s0, 1
	addi	s1, s1, 40
	bltu	s0, a0, .LBB0_125
.LBB0_126:
	lui	a0, %hi(.L.str.98)
	addi	a0, a0, %lo(.L.str.98)
	call	puts
	li	a0, 0
	addi	sp, sp, 2032
	addi	sp, sp, 304
	ld	ra, 2024(sp)                    # 8-byte Folded Reload
	ld	s0, 2016(sp)                    # 8-byte Folded Reload
	ld	s1, 2008(sp)                    # 8-byte Folded Reload
	ld	s2, 2000(sp)                    # 8-byte Folded Reload
	ld	s3, 1992(sp)                    # 8-byte Folded Reload
	ld	s4, 1984(sp)                    # 8-byte Folded Reload
	ld	s5, 1976(sp)                    # 8-byte Folded Reload
	ld	s6, 1968(sp)                    # 8-byte Folded Reload
	ld	s7, 1960(sp)                    # 8-byte Folded Reload
	ld	s8, 1952(sp)                    # 8-byte Folded Reload
	ld	s9, 1944(sp)                    # 8-byte Folded Reload
	ld	s10, 1936(sp)                   # 8-byte Folded Reload
	ld	s11, 1928(sp)                   # 8-byte Folded Reload
	addi	sp, sp, 2032
	ret
.LBB0_127:
	li	a0, 1
	beq	s9, a0, .LBB0_114
# %bb.128:
	lui	a0, %hi(.L.str.19)
	addi	a0, a0, %lo(.L.str.19)
	mv	a1, s9
	call	printf
	call	abort
.LBB0_129:
	mv	s5, a1
	j	.LBB0_131
.LBB0_130:
	mv	s5, a1
	mv	s2, s0
.LBB0_131:
	lui	a0, %hi(.L.str.3)
	addi	a0, a0, %lo(.L.str.3)
	mv	a1, s5
	mv	a2, s2
	call	print_error
	li	a0, 1
	call	exit
.LBB0_132:
	lui	a0, %hi(.L.str)
	addi	a0, a0, %lo(.L.str)
	call	puts
	call	exit
.LBB0_133:
	call	close
	call	__errno_location
	mv	s2, a0
	li	a0, 21
	sw	a0, 0(s2)
	j	.LBB0_135
.LBB0_134:
	call	__errno_location
	mv	s2, a0
.LBB0_135:
	addi	a0, sp, 168
	lui	a2, 1
	li	a1, 0
	call	memset@plt
	lw	a0, 0(s2)
	call	strerror
	mv	a3, a0
	lui	a0, %hi(.L.str.87)
	addi	a2, a0, %lo(.L.str.87)
	addi	a0, sp, 168
	lui	a1, 1
	mv	a4, s3
	call	snprintf
	addi	a0, sp, 168
	li	a1, 0
	li	a2, 0
	call	print_error
	lui	a0, %hi(stdout)
	ld	s1, %lo(stdout)(a0)
	lw	a0, 0(s2)
	call	strerror
	mv	a2, a0
	lui	a0, %hi(.L.str.88)
	addi	a1, a0, %lo(.L.str.88)
	mv	a0, s1
	mv	a3, s3
	call	fprintf
	li	a0, 1
	call	exit
.LBB0_136:
	li	s5, 0
	li	s2, 0
	j	.LBB0_131
.LBB0_137:
	call	abort
.LBB0_138:
	lui	a0, %hi(.L.str.20)
	addi	a0, a0, %lo(.L.str.20)
	mv	a1, s4
	mv	a2, s10
	call	printf
	lui	a0, %hi(.L.str.21)
	addi	a0, a0, %lo(.L.str.21)
	mv	a1, s4
	call	printf
	lui	a0, %hi(.L.str.22)
	addi	a0, a0, %lo(.L.str.22)
	mv	a1, s5
	li	a2, 0
	call	print_error
	call	abort
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.section	.rodata,"a",@progbits
	.p2align	2, 0x0
.LJTI0_0:
	.word	.LBB0_52
	.word	.LBB0_54
	.word	.LBB0_55
	.word	.LBB0_137
	.word	.LBB0_58
	.word	.LBB0_137
	.word	.LBB0_58
	.word	.LBB0_56
	.word	.LBB0_57
	.word	.LBB0_58
	.word	.LBB0_53
.LJTI0_1:
	.word	.LBB0_61
	.word	.LBB0_72
	.word	.LBB0_73
	.word	.LBB0_74
	.word	.LBB0_75
	.word	.LBB0_138
	.word	.LBB0_138
	.word	.LBB0_76
	.word	.LBB0_77
	.word	.LBB0_78
	.word	.LBB0_79
	.word	.LBB0_80
	.word	.LBB0_81
	.word	.LBB0_82
	.word	.LBB0_83
	.word	.LBB0_84
	.word	.LBB0_85
	.word	.LBB0_138
	.word	.LBB0_138
	.word	.LBB0_86
	.word	.LBB0_138
	.word	.LBB0_87
	.word	.LBB0_138
	.word	.LBB0_88
	.word	.LBB0_89
	.word	.LBB0_93
	.word	.LBB0_96
	.word	.LBB0_138
	.word	.LBB0_97
	.word	.LBB0_99
	.word	.LBB0_138
	.word	.LBB0_100
	.word	.LBB0_138
	.word	.LBB0_101
	.word	.LBB0_138
	.word	.LBB0_102
	.word	.LBB0_106
	.word	.LBB0_107
	.word	.LBB0_108
	.word	.LBB0_109
	.word	.LBB0_110
	.word	.LBB0_138
	.word	.LBB0_138
	.word	.LBB0_138
	.word	.LBB0_111
	.word	.LBB0_138
	.word	.LBB0_112
                                        # -- End function
	.text
	.p2align	1                               # -- Begin function print_error
	.type	print_error,@function
print_error:                            # @print_error
# %bb.0:
	addi	sp, sp, -2032
	sd	ra, 2024(sp)                    # 8-byte Folded Spill
	sd	s0, 2016(sp)                    # 8-byte Folded Spill
	sd	s1, 2008(sp)                    # 8-byte Folded Spill
	sd	s2, 2000(sp)                    # 8-byte Folded Spill
	sd	s3, 1992(sp)                    # 8-byte Folded Spill
	sd	s4, 1984(sp)                    # 8-byte Folded Spill
	sd	s5, 1976(sp)                    # 8-byte Folded Spill
	sd	s6, 1968(sp)                    # 8-byte Folded Spill
	sd	s7, 1960(sp)                    # 8-byte Folded Spill
	sd	s8, 1952(sp)                    # 8-byte Folded Spill
	sd	s9, 1944(sp)                    # 8-byte Folded Spill
	sd	s10, 1936(sp)                   # 8-byte Folded Spill
	sd	s11, 1928(sp)                   # 8-byte Folded Spill
	lui	a3, 24
	addiw	a3, a3, -1856
	sub	sp, sp, a3
	mv	s2, a2
	mv	s7, a1
	mv	s0, a0
	lui	a0, %hi(.L.str.89)
	addi	a0, a0, %lo(.L.str.89)
	li	a1, 32
	call	printf
	lui	a0, 16
	addiw	a0, a0, 72
	add	a0, a0, sp
	lui	a2, 8
	li	a1, 0
	call	memset@plt
	lui	a0, 8
	addiw	a0, a0, 72
	add	a0, a0, sp
	lui	a2, 8
	lui	a1, 8
	addiw	a1, a1, 72
	add	s3, sp, a1
	li	a1, 0
	call	memset@plt
	addi	a0, sp, 72
	lui	a2, 8
	addi	s1, sp, 72
	li	a1, 0
	call	memset@plt
	lui	s5, %hi(text_length)
	ld	a0, %lo(text_length)(s5)
	sd	s0, 24(sp)                      # 8-byte Folded Spill
	sd	s2, 16(sp)                      # 8-byte Folded Spill
	beqz	a0, .LBB1_15
# %bb.1:
	li	s4, 0
	li	s0, 0
	lui	s9, %hi(file_count)
	lui	a0, %hi(files)
	addi	s6, a0, %lo(files)
	addi	a0, s6, 8
	sd	a0, 64(sp)                      # 8-byte Folded Spill
	li	s11, 24
	lui	a0, %hi(.L.str.96)
	addi	a0, a0, %lo(.L.str.96)
	sd	a0, 48(sp)                      # 8-byte Folded Spill
	lui	a0, %hi(.L.str.98)
	addi	a0, a0, %lo(.L.str.98)
	sd	a0, 40(sp)                      # 8-byte Folded Spill
	lui	a0, %hi(.L.str.90)
	addi	a0, a0, %lo(.L.str.90)
	sd	a0, 32(sp)                      # 8-byte Folded Spill
	lui	a0, %hi(.L.str.97)
	addi	s10, a0, %lo(.L.str.97)
.LBB1_2:                                # =>This Loop Header: Depth=1
                                        #     Child Loop BB1_4 Depth 2
                                        #     Child Loop BB1_11 Depth 2
	ld	a0, %lo(file_count)(s9)
	beqz	a0, .LBB1_7
# %bb.3:                                #   in Loop: Header=BB1_2 Depth=1
	li	a1, 0
	addi	s8, s0, -1
	slli	a3, s8, 3
	add	a2, s1, a3
	add	a3, a3, s3
	ld	s2, 64(sp)                      # 8-byte Folded Reload
.LBB1_4:                                #   Parent Loop BB1_2 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	ld	a4, -8(s2)
	beq	s4, a4, .LBB1_8
# %bb.5:                                #   in Loop: Header=BB1_4 Depth=2
	ld	a4, 0(a3)
	ld	a5, 0(a2)
	mul	a4, a4, s11
	add	a4, a4, s6
	ld	a4, 8(a4)
	beq	a5, a4, .LBB1_9
# %bb.6:                                #   in Loop: Header=BB1_4 Depth=2
	addi	a1, a1, 1
	addi	s2, s2, 24
	bne	a0, a1, .LBB1_4
.LBB1_7:                                #   in Loop: Header=BB1_2 Depth=1
	mv	s8, s0
	j	.LBB1_13
.LBB1_8:                                #   in Loop: Header=BB1_2 Depth=1
	slli	a0, s0, 3
	lui	a2, 16
	addiw	a2, a2, 72
	add	a2, a2, sp
	add	a2, a2, a0
	sd	s4, 0(a2)
	add	a2, s3, a0
	sd	a1, 0(a2)
	addi	s8, s0, 1
	add	a0, a0, s1
	sd	zero, 0(a0)
	j	.LBB1_13
.LBB1_9:                                #   in Loop: Header=BB1_2 Depth=1
	ld	a0, 0(s2)
	sd	a0, 56(sp)                      # 8-byte Folded Spill
	ld	a0, 48(sp)                      # 8-byte Folded Reload
	mv	a1, s0
	call	printf
	beqz	s0, .LBB1_12
# %bb.10:                               #   in Loop: Header=BB1_2 Depth=1
	li	s1, 0
	addi	s5, sp, 72
	lui	a0, 16
	addiw	a0, a0, 72
	add	s3, sp, a0
	lui	a0, 8
	addiw	a0, a0, 72
	add	s9, sp, a0
.LBB1_11:                               #   Parent Loop BB1_2 Depth=1
                                        # =>  This Inner Loop Header: Depth=2
	ld	a4, 0(s9)
	mul	a0, a4, s11
	add	a0, a0, s6
	ld	a2, 16(a0)
	ld	a3, 0(s3)
	ld	a5, 0(s5)
	ld	a6, 8(a0)
	mv	a0, s10
	mv	a1, s1
	call	printf
	addi	s1, s1, 1
	addi	s5, s5, 8
	addi	s3, s3, 8
	addi	s9, s9, 8
	bne	s0, s1, .LBB1_11
.LBB1_12:                               #   in Loop: Header=BB1_2 Depth=1
	ld	a0, 40(sp)                      # 8-byte Folded Reload
	call	puts
	ld	a1, 8(s2)
	ld	a0, 32(sp)                      # 8-byte Folded Reload
	mv	a2, s8
	ld	a3, 56(sp)                      # 8-byte Folded Reload
	call	printf
	lui	a0, 8
	addiw	a0, a0, 72
	add	s3, sp, a0
	addi	s1, sp, 72
	lui	s5, %hi(text_length)
	lui	s9, %hi(file_count)
	beqz	s8, .LBB1_17
.LBB1_13:                               #   in Loop: Header=BB1_2 Depth=1
	slli	s0, s8, 3
	beq	s4, s7, .LBB1_18
# %bb.14:                               #   in Loop: Header=BB1_2 Depth=1
	lui	a0, %hi(text)
	ld	a0, %lo(text)(a0)
	add	a0, a0, s4
	lbu	a0, 0(a0)
	lui	a1, %hi(stdout)
	ld	a1, %lo(stdout)(a1)
	call	putc
	add	s0, s0, s1
	ld	a0, -8(s0)
	ld	a1, %lo(text_length)(s5)
	addi	a0, a0, 1
	addi	s4, s4, 1
	sd	a0, -8(s0)
	mv	s0, s8
	bltu	s4, a1, .LBB1_2
	j	.LBB1_16
.LBB1_15:
	li	s8, 0
.LBB1_16:
	lui	a0, %hi(.L.str.92)
	addi	a0, a0, %lo(.L.str.92)
	call	printf
.LBB1_17:
	li	s4, 0
	li	s5, 0
	j	.LBB1_19
.LBB1_18:
	addi	s0, s0, -8
	lui	a0, 8
	addiw	a0, a0, 72
	add	a0, a0, sp
	add	a0, a0, s0
	ld	a0, 0(a0)
	li	a1, 24
	mul	a0, a0, a1
	lui	a1, %hi(files)
	addi	a1, a1, %lo(files)
	add	s1, a1, a0
	ld	a1, 16(s1)
	addi	a0, sp, 72
	add	a0, a0, s0
	ld	s4, 0(a0)
	lui	a0, %hi(.L.str.91)
	addi	a0, a0, %lo(.L.str.91)
	mv	a2, s4
	call	printf
	ld	s5, 16(s1)
.LBB1_19:
	lui	s2, %hi(file_count)
	ld	a1, %lo(file_count)(s2)
	lui	a0, %hi(.L.str.99)
	addi	a0, a0, %lo(.L.str.99)
	call	printf
	ld	a0, %lo(file_count)(s2)
	beqz	a0, .LBB1_22
# %bb.20:
	li	s0, 0
	lui	a0, %hi(files+8)
	addi	s1, a0, %lo(files+8)
	lui	a0, %hi(.L.str.100)
	addi	s3, a0, %lo(.L.str.100)
.LBB1_21:                               # =>This Inner Loop Header: Depth=1
	ld	a2, 8(s1)
	ld	a3, -8(s1)
	ld	a4, 0(s1)
	mv	a0, s3
	mv	a1, s0
	call	printf
	ld	a0, %lo(file_count)(s2)
	addi	s0, s0, 1
	addi	s1, s1, 24
	bltu	s0, a0, .LBB1_21
.LBB1_22:
	lui	a0, %hi(.L.str.98)
	addi	s6, a0, %lo(.L.str.98)
	mv	a0, s6
	call	puts
	lui	a0, %hi(.L.str.96)
	addi	a0, a0, %lo(.L.str.96)
	mv	a1, s8
	call	printf
	beqz	s8, .LBB1_25
# %bb.23:
	li	s1, 0
	addi	s3, sp, 72
	lui	a0, 16
	addiw	a0, a0, 72
	add	s7, sp, a0
	lui	a0, 8
	addiw	a0, a0, 72
	add	s0, sp, a0
	li	s9, 24
	lui	a0, %hi(files)
	addi	s10, a0, %lo(files)
	lui	a0, %hi(.L.str.97)
	addi	s2, a0, %lo(.L.str.97)
.LBB1_24:                               # =>This Inner Loop Header: Depth=1
	ld	a4, 0(s0)
	mul	a0, a4, s9
	add	a0, a0, s10
	ld	a2, 16(a0)
	ld	a3, 0(s7)
	ld	a5, 0(s3)
	ld	a6, 8(a0)
	mv	a0, s2
	mv	a1, s1
	call	printf
	addi	s1, s1, 1
	addi	s3, s3, 8
	addi	s7, s7, 8
	addi	s0, s0, 8
	bne	s8, s1, .LBB1_24
.LBB1_25:
	mv	a0, s6
	call	puts
	lui	s0, %hi(stdout)
	ld	a0, %lo(stdout)(s0)
	bnez	s5, .LBB1_27
# %bb.26:
	lui	a1, %hi(.L.str.94)
	addi	s5, a1, %lo(.L.str.94)
.LBB1_27:
	ld	s1, 24(sp)                      # 8-byte Folded Reload
	ld	a4, 16(sp)                      # 8-byte Folded Reload
	lui	a1, %hi(.L.str.93)
	addi	a1, a1, %lo(.L.str.93)
	mv	a2, s5
	mv	a3, s4
	call	fprintf
	ld	a0, %lo(stdout)(s0)
	lui	a1, %hi(.L.str.95)
	addi	a1, a1, %lo(.L.str.95)
	mv	a2, s1
	lui	a3, 24
	addiw	a3, a3, -1856
	add	sp, sp, a3
	ld	ra, 2024(sp)                    # 8-byte Folded Reload
	ld	s0, 2016(sp)                    # 8-byte Folded Reload
	ld	s1, 2008(sp)                    # 8-byte Folded Reload
	ld	s2, 2000(sp)                    # 8-byte Folded Reload
	ld	s3, 1992(sp)                    # 8-byte Folded Reload
	ld	s4, 1984(sp)                    # 8-byte Folded Reload
	ld	s5, 1976(sp)                    # 8-byte Folded Reload
	ld	s6, 1968(sp)                    # 8-byte Folded Reload
	ld	s7, 1960(sp)                    # 8-byte Folded Reload
	ld	s8, 1952(sp)                    # 8-byte Folded Reload
	ld	s9, 1944(sp)                    # 8-byte Folded Reload
	ld	s10, 1936(sp)                   # 8-byte Folded Reload
	ld	s11, 1928(sp)                   # 8-byte Folded Reload
	addi	sp, sp, 2032
	tail	fprintf
.Lfunc_end1:
	.size	print_error, .Lfunc_end1-print_error
                                        # -- End function
	.type	.L.str,@object                  # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"asm: \033[31;1merror:\033[0m usage: ./asm <source.s>"
	.size	.L.str, 47

	.type	spelling,@object                # @spelling
	.section	.rodata,"a",@progbits
	.p2align	3, 0x0
spelling:
	.quad	.L.str.26
	.quad	.L.str.27
	.quad	.L.str.28
	.quad	.L.str.29
	.quad	.L.str.30
	.quad	.L.str.31
	.quad	.L.str.32
	.quad	.L.str.33
	.quad	.L.str.34
	.quad	.L.str.35
	.quad	.L.str.36
	.quad	.L.str.37
	.quad	.L.str.38
	.quad	.L.str.39
	.quad	.L.str.40
	.quad	.L.str.41
	.quad	.L.str.42
	.quad	.L.str.43
	.quad	.L.str.44
	.quad	.L.str.45
	.quad	.L.str.46
	.quad	.L.str.47
	.quad	.L.str.48
	.quad	.L.str.49
	.quad	.L.str.50
	.quad	.L.str.51
	.quad	.L.str.52
	.quad	.L.str.53
	.quad	.L.str.54
	.quad	.L.str.55
	.quad	.L.str.56
	.quad	.L.str.57
	.quad	.L.str.58
	.quad	.L.str.59
	.quad	.L.str.60
	.quad	.L.str.61
	.quad	.L.str.62
	.quad	.L.str.63
	.quad	.L.str.64
	.quad	.L.str.65
	.quad	.L.str.66
	.quad	.L.str.67
	.quad	.L.str.68
	.quad	.L.str.69
	.quad	.L.str.70
	.quad	.L.str.71
	.quad	.L.str.72
	.quad	.L.str.73
	.quad	.L.str.74
	.quad	.L.str.75
	.quad	.L.str.76
	.quad	.L.str.77
	.quad	.L.str.78
	.quad	.L.str.79
	.quad	.L.str.80
	.quad	.L.str.81
	.quad	.L.str.82
	.quad	.L.str.83
	.quad	.L.str.84
	.quad	.L.str.85
	.quad	.L.str.86
	.size	spelling, 488

	.type	text_length,@object             # @text_length
	.section	.sbss,"aw",@nobits
	.p2align	3
text_length:
	.quad	0                               # 0x0
	.size	text_length, 8

	.type	text,@object                    # @text
	.p2align	3
text:
	.quad	0
	.size	text, 8

	.type	.L.str.1,@object                # @.str.1
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.1:
	.asciz	"read file: (length = %llu): \n<<<"
	.size	.L.str.1, 33

	.type	.L.str.2,@object                # @.str.2
.L.str.2:
	.asciz	">>>"
	.size	.L.str.2, 4

	.type	array,@object                   # @array
	.section	.sbss,"aw",@nobits
	.p2align	3
array:
	.quad	0
	.size	array, 8

	.type	files,@object                   # @files
	.local	files
	.comm	files,98304,8
	.type	file_count,@object              # @file_count
	.p2align	3
file_count:
	.quad	0                               # 0x0
	.size	file_count, 8

	.type	arguments,@object               # @arguments
	.local	arguments
	.comm	arguments,32768,8
	.type	arg_count,@object               # @arg_count
	.p2align	3
arg_count:
	.quad	0                               # 0x0
	.size	arg_count, 8

	.type	name_count,@object              # @name_count
	.p2align	3
name_count:
	.quad	0                               # 0x0
	.size	name_count, 8

	.type	lengths,@object                 # @lengths
	.local	lengths
	.comm	lengths,32768,8
	.type	names,@object                   # @names
	.local	names
	.comm	names,32768,8
	.type	.L.str.3,@object                # @.str.3
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.3:
	.asciz	"unresolved symbol"
	.size	.L.str.3, 18

	.type	.L.str.4,@object                # @.str.4
.L.str.4:
	.asciz	"debug: found name: %s\n"
	.size	.L.str.4, 23

	.type	.L.str.5,@object                # @.str.5
.L.str.5:
	.asciz	"info: found user-defined name:     calling \"%s\"!! \n"
	.size	.L.str.5, 52

	.type	values,@object                  # @values
	.local	values
	.comm	values,32768,8
	.type	.L.str.6,@object                # @.str.6
.L.str.6:
	.asciz	"----> pushed: user-defined value %llu onto stack...\n"
	.size	.L.str.6, 53

	.type	.L.str.7,@object                # @.str.7
.L.str.7:
	.asciz	"debug: found builtin name: %s\n"
	.size	.L.str.7, 31

	.type	.L.str.8,@object                # @.str.8
.L.str.8:
	.asciz	"executing ct..."
	.size	.L.str.8, 16

	.type	.L.str.9,@object                # @.str.9
.L.str.9:
	.asciz	"executing mi..."
	.size	.L.str.9, 16

	.type	.L.str.10,@object               # @.str.10
.L.str.10:
	.asciz	"executing mz..."
	.size	.L.str.10, 16

	.type	.L.str.11,@object               # @.str.11
.L.str.11:
	.asciz	"executing drop..."
	.size	.L.str.11, 18

	.type	.L.str.12,@object               # @.str.12
.L.str.12:
	.asciz	"executing dup..."
	.size	.L.str.12, 17

	.type	.L.str.13,@object               # @.str.13
.L.str.13:
	.asciz	"executing swap..."
	.size	.L.str.13, 18

	.type	.L.str.14,@object               # @.str.14
.L.str.14:
	.asciz	"info: pushing rt ins %llu(\"%s\")...\n"
	.size	.L.str.14, 36

	.type	.L.str.15,@object               # @.str.15
.L.str.15:
	.asciz	" ... argument #%llu : u32 = %u\n"
	.size	.L.str.15, 32

	.type	ins,@object                     # @ins
	.section	.sbss,"aw",@nobits
	.p2align	3
ins:
	.quad	0
	.size	ins, 8

	.type	ins_count,@object               # @ins_count
	.p2align	3
ins_count:
	.quad	0                               # 0x0
	.size	ins_count, 8

	.type	.L.str.16,@object               # @.str.16
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str.16:
	.asciz	"------> info: CT: EXECUTING: op = %llu (\"%s\"), args={a0:%llu, a1:%llu, a2:%llu}\n"
	.size	.L.str.16, 81

	.type	.L.str.17,@object               # @.str.17
.L.str.17:
	.asciz	"executing ecall at ct..."
	.size	.L.str.17, 25

	.type	.L.str.18,@object               # @.str.18
.L.str.18:
	.asciz	"debug: %lld (hex 0x%016llx)\n"
	.size	.L.str.18, 29

	.type	.L.str.19,@object               # @.str.19
.L.str.19:
	.asciz	"error: ct: unknown ecall number %llu... aborting...\n"
	.size	.L.str.19, 53

	.type	.L.str.20,@object               # @.str.20
.L.str.20:
	.asciz	"internal error: found unimplemented operation: %llu (\"%s\")...\n"
	.size	.L.str.20, 63

	.type	.L.str.21,@object               # @.str.21
.L.str.21:
	.asciz	"error: unknown ct ins = %llu\n"
	.size	.L.str.21, 30

	.type	.L.str.22,@object               # @.str.22
.L.str.22:
	.asciz	"unknown ct ins"
	.size	.L.str.22, 15

	.type	.L.str.23,@object               # @.str.23
.L.str.23:
	.asciz	"DONE: finished assembling program."
	.size	.L.str.23, 35

	.type	.L.str.26,@object               # @.str.26
.L.str.26:
	.asciz	"null_instruction"
	.size	.L.str.26, 17

	.type	.L.str.27,@object               # @.str.27
.L.str.27:
	.asciz	"dup"
	.size	.L.str.27, 4

	.type	.L.str.28,@object               # @.str.28
.L.str.28:
	.asciz	"swap"
	.size	.L.str.28, 5

	.type	.L.str.29,@object               # @.str.29
.L.str.29:
	.asciz	"drop"
	.size	.L.str.29, 5

	.type	.L.str.30,@object               # @.str.30
.L.str.30:
	.asciz	"over"
	.size	.L.str.30, 5

	.type	.L.str.31,@object               # @.str.31
.L.str.31:
	.asciz	"rot"
	.size	.L.str.31, 4

	.type	.L.str.32,@object               # @.str.32
.L.str.32:
	.asciz	"tuck"
	.size	.L.str.32, 5

	.type	.L.str.33,@object               # @.str.33
.L.str.33:
	.asciz	"def"
	.size	.L.str.33, 4

	.type	.L.str.34,@object               # @.str.34
.L.str.34:
	.asciz	"mi"
	.size	.L.str.34, 3

	.type	.L.str.35,@object               # @.str.35
.L.str.35:
	.asciz	"mz"
	.size	.L.str.35, 3

	.type	.L.str.36,@object               # @.str.36
.L.str.36:
	.asciz	"mar"
	.size	.L.str.36, 4

	.type	.L.str.37,@object               # @.str.37
.L.str.37:
	.asciz	"ct"
	.size	.L.str.37, 3

	.type	.L.str.38,@object               # @.str.38
.L.str.38:
	.asciz	"atr"
	.size	.L.str.38, 4

	.type	.L.str.39,@object               # @.str.39
.L.str.39:
	.asciz	"add"
	.size	.L.str.39, 4

	.type	.L.str.40,@object               # @.str.40
.L.str.40:
	.asciz	"sub"
	.size	.L.str.40, 4

	.type	.L.str.41,@object               # @.str.41
.L.str.41:
	.asciz	"addi"
	.size	.L.str.41, 5

	.type	.L.str.42,@object               # @.str.42
.L.str.42:
	.asciz	"slt"
	.size	.L.str.42, 4

	.type	.L.str.43,@object               # @.str.43
.L.str.43:
	.asciz	"slti"
	.size	.L.str.43, 5

	.type	.L.str.44,@object               # @.str.44
.L.str.44:
	.asciz	"slts"
	.size	.L.str.44, 5

	.type	.L.str.45,@object               # @.str.45
.L.str.45:
	.asciz	"sltis"
	.size	.L.str.45, 6

	.type	.L.str.46,@object               # @.str.46
.L.str.46:
	.asciz	"and"
	.size	.L.str.46, 4

	.type	.L.str.47,@object               # @.str.47
.L.str.47:
	.asciz	"andi"
	.size	.L.str.47, 5

	.type	.L.str.48,@object               # @.str.48
.L.str.48:
	.asciz	"or"
	.size	.L.str.48, 3

	.type	.L.str.49,@object               # @.str.49
.L.str.49:
	.asciz	"ori"
	.size	.L.str.49, 4

	.type	.L.str.50,@object               # @.str.50
.L.str.50:
	.asciz	"xor"
	.size	.L.str.50, 4

	.type	.L.str.51,@object               # @.str.51
.L.str.51:
	.asciz	"xori"
	.size	.L.str.51, 5

	.type	.L.str.52,@object               # @.str.52
.L.str.52:
	.asciz	"sll"
	.size	.L.str.52, 4

	.type	.L.str.53,@object               # @.str.53
.L.str.53:
	.asciz	"slli"
	.size	.L.str.53, 5

	.type	.L.str.54,@object               # @.str.54
.L.str.54:
	.asciz	"srl"
	.size	.L.str.54, 4

	.type	.L.str.55,@object               # @.str.55
.L.str.55:
	.asciz	"srli"
	.size	.L.str.55, 5

	.type	.L.str.56,@object               # @.str.56
.L.str.56:
	.asciz	"sra"
	.size	.L.str.56, 4

	.type	.L.str.57,@object               # @.str.57
.L.str.57:
	.asciz	"srai"
	.size	.L.str.57, 5

	.type	.L.str.58,@object               # @.str.58
.L.str.58:
	.asciz	"blt"
	.size	.L.str.58, 4

	.type	.L.str.59,@object               # @.str.59
.L.str.59:
	.asciz	"blts"
	.size	.L.str.59, 5

	.type	.L.str.60,@object               # @.str.60
.L.str.60:
	.asciz	"bge"
	.size	.L.str.60, 4

	.type	.L.str.61,@object               # @.str.61
.L.str.61:
	.asciz	"bges"
	.size	.L.str.61, 5

	.type	.L.str.62,@object               # @.str.62
.L.str.62:
	.asciz	"bne"
	.size	.L.str.62, 4

	.type	.L.str.63,@object               # @.str.63
.L.str.63:
	.asciz	"beq"
	.size	.L.str.63, 4

	.type	.L.str.64,@object               # @.str.64
.L.str.64:
	.asciz	"jal"
	.size	.L.str.64, 4

	.type	.L.str.65,@object               # @.str.65
.L.str.65:
	.asciz	"jalr"
	.size	.L.str.65, 5

	.type	.L.str.66,@object               # @.str.66
.L.str.66:
	.asciz	"auipc"
	.size	.L.str.66, 6

	.type	.L.str.67,@object               # @.str.67
.L.str.67:
	.asciz	"ecall"
	.size	.L.str.67, 6

	.type	.L.str.68,@object               # @.str.68
.L.str.68:
	.asciz	"lb"
	.size	.L.str.68, 3

	.type	.L.str.69,@object               # @.str.69
.L.str.69:
	.asciz	"lbs"
	.size	.L.str.69, 4

	.type	.L.str.70,@object               # @.str.70
.L.str.70:
	.asciz	"lh"
	.size	.L.str.70, 3

	.type	.L.str.71,@object               # @.str.71
.L.str.71:
	.asciz	"lhs"
	.size	.L.str.71, 4

	.type	.L.str.72,@object               # @.str.72
.L.str.72:
	.asciz	"lw"
	.size	.L.str.72, 3

	.type	.L.str.73,@object               # @.str.73
.L.str.73:
	.asciz	"lws"
	.size	.L.str.73, 4

	.type	.L.str.74,@object               # @.str.74
.L.str.74:
	.asciz	"ld"
	.size	.L.str.74, 3

	.type	.L.str.75,@object               # @.str.75
.L.str.75:
	.asciz	"sb"
	.size	.L.str.75, 3

	.type	.L.str.76,@object               # @.str.76
.L.str.76:
	.asciz	"sh"
	.size	.L.str.76, 3

	.type	.L.str.77,@object               # @.str.77
.L.str.77:
	.asciz	"sw"
	.size	.L.str.77, 3

	.type	.L.str.78,@object               # @.str.78
.L.str.78:
	.asciz	"sd"
	.size	.L.str.78, 3

	.type	.L.str.79,@object               # @.str.79
.L.str.79:
	.asciz	"mul"
	.size	.L.str.79, 4

	.type	.L.str.80,@object               # @.str.80
.L.str.80:
	.asciz	"mh"
	.size	.L.str.80, 3

	.type	.L.str.81,@object               # @.str.81
.L.str.81:
	.asciz	"mhs"
	.size	.L.str.81, 4

	.type	.L.str.82,@object               # @.str.82
.L.str.82:
	.asciz	"mhsu"
	.size	.L.str.82, 5

	.type	.L.str.83,@object               # @.str.83
.L.str.83:
	.asciz	"div_"
	.size	.L.str.83, 5

	.type	.L.str.84,@object               # @.str.84
.L.str.84:
	.asciz	"divs"
	.size	.L.str.84, 5

	.type	.L.str.85,@object               # @.str.85
.L.str.85:
	.asciz	"rem"
	.size	.L.str.85, 4

	.type	.L.str.86,@object               # @.str.86
.L.str.86:
	.asciz	"rems"
	.size	.L.str.86, 5

	.type	.L.str.87,@object               # @.str.87
.L.str.87:
	.asciz	"%s: \"%s\""
	.size	.L.str.87, 9

	.type	.L.str.88,@object               # @.str.88
.L.str.88:
	.asciz	"asm: \033[31;1merror:\033[0m %s: \"%s\"\n"
	.size	.L.str.88, 33

	.type	.L.str.89,@object               # @.str.89
.L.str.89:
	.asciz	"\033[%dm"
	.size	.L.str.89, 6

	.type	.L.str.90,@object               # @.str.90
.L.str.90:
	.asciz	"file %s reached the end the file! (stack_o[%llu] == count == %llu)\n"
	.size	.L.str.90, 68

	.type	.L.str.91,@object               # @.str.91
.L.str.91:
	.asciz	"\033[38;5;255m(ERROR_HERE:%s:%llu)\033[0m"
	.size	.L.str.91, 36

	.type	.L.str.92,@object               # @.str.92
.L.str.92:
	.asciz	"\033[0m"
	.size	.L.str.92, 5

	.type	.L.str.93,@object               # @.str.93
.L.str.93:
	.asciz	"\033[1masm: %s:%lld:%lld:"
	.size	.L.str.93, 23

	.type	.L.str.94,@object               # @.str.94
.L.str.94:
	.asciz	"(top-level)"
	.size	.L.str.94, 12

	.type	.L.str.95,@object               # @.str.95
.L.str.95:
	.asciz	" \033[1;31merror:\033[m \033[1m%s\033[m\n"
	.size	.L.str.95, 29

	.type	.L.str.96,@object               # @.str.96
.L.str.96:
	.asciz	"current stack: (%lld entries) { \n"
	.size	.L.str.96, 34

	.type	.L.str.97,@object               # @.str.97
.L.str.97:
	.asciz	"\t entry #%-8lld :   name = \"%-20s\", i = %-8lld, f = %-8lld, o = %-8lld / %lld\n"
	.size	.L.str.97, 79

	.type	.L.str.98,@object               # @.str.98
.L.str.98:
	.asciz	"}"
	.size	.L.str.98, 2

	.type	.L.str.99,@object               # @.str.99
.L.str.99:
	.asciz	"here are the current files used in the program: (%lld files) { \n"
	.size	.L.str.99, 65

	.type	.L.str.100,@object              # @.str.100
.L.str.100:
	.asciz	"\t file #%-8lld :   name = \"%-20s\", .start = %-8lld, .size = %-8lld\n"
	.size	.L.str.100, 68

	.type	.L.str.102,@object              # @.str.102
.L.str.102:
	.asciz	"\targuments[%llu] = { %llu  :  (.start=%llu,.count=%llu)} \n"
	.size	.L.str.102, 59

	.type	.L.str.103,@object              # @.str.103
.L.str.103:
	.asciz	"} \n"
	.size	.L.str.103, 4

	.type	.L.str.104,@object              # @.str.104
.L.str.104:
	.asciz	"printing dictionary..."
	.size	.L.str.104, 23

	.type	.L.str.105,@object              # @.str.105
.L.str.105:
	.asciz	"\t#%llu: name %s  length %llu  value %llu\n"
	.size	.L.str.105, 42

	.type	.L.str.106,@object              # @.str.106
.L.str.106:
	.asciz	"done."
	.size	.L.str.106, 6

	.type	.L.str.108,@object              # @.str.108
.L.str.108:
	.asciz	"\t%llu\tins(.op=%u (\"%s\"), .size=%llu, args:{ "
	.size	.L.str.108, 45

	.type	.L.str.109,@object              # @.str.109
.L.str.109:
	.asciz	"%llu "
	.size	.L.str.109, 6

	.type	.L.str.110,@object              # @.str.110
.L.str.110:
	.asciz	"} -- [found @ %llu]\n"
	.size	.L.str.110, 21

	.type	.Lstr,@object                   # @str
.Lstr:
	.asciz	"\n\n\t\t\t\033[31mFAILURE\033[0m\n"
	.size	.Lstr, 23

	.type	.Lstr.111,@object               # @str.111
.Lstr.111:
	.asciz	"\n\n\t\t\t\033[32mSUCCESS\033[0m\n"
	.size	.Lstr.111, 23

	.type	.Lstr.112,@object               # @str.112
.Lstr.112:
	.asciz	"\narguments[]: { "
	.size	.Lstr.112, 17

	.type	.Lstr.113,@object               # @str.113
.Lstr.113:
	.asciz	"instructions: {"
	.size	.Lstr.113, 16

	.ident	"clang version 16.0.6 (Fedora 16.0.6-3.rv64.fc38)"
	.section	".note.GNU-stack","",@progbits
	.addrsig
