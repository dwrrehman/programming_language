	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 11, 0	sdk_version 11, 1
	.globl	_main                   ## -- Begin function main
	.p2align	4, 0x90
_main:                                  ## @main
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	subq	$344, %rsp              ## imm = 0x158
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	movl	$1, %eax
	cmpl	$2, %edi
	jl	LBB0_143
## %bb.1:
	movq	%rsi, -192(%rbp)        ## 8-byte Spill
	movq	8(%rsi), %r14
	xorps	%xmm0, %xmm0
	movaps	%xmm0, -240(%rbp)
	movaps	%xmm0, -256(%rbp)
	movaps	%xmm0, -272(%rbp)
	movaps	%xmm0, -288(%rbp)
	movaps	%xmm0, -304(%rbp)
	movaps	%xmm0, -320(%rbp)
	movaps	%xmm0, -336(%rbp)
	movaps	%xmm0, -352(%rbp)
	movaps	%xmm0, -368(%rbp)
	movq	%r14, %rdi
	xorl	%esi, %esi
	xorl	%eax, %eax
	callq	_open
	testl	%eax, %eax
	js	LBB0_157
## %bb.2:
	movl	%eax, %ebx
	leaq	-368(%rbp), %rsi
	movq	%r14, %rdi
	callq	_stat$INODE64
	testl	%eax, %eax
	js	LBB0_157
## %bb.3:
	movq	-272(%rbp), %r12
	xorl	%r15d, %r15d
	testl	%r12d, %r12d
	je	LBB0_4
## %bb.5:
	movslq	%r12d, %rsi
	xorl	%edi, %edi
	movl	$1, %edx
	movl	$1, %ecx
	movl	%ebx, %r8d
	xorl	%r9d, %r9d
	callq	_mmap
	movq	%rax, %rcx
	movq	%rax, -80(%rbp)         ## 8-byte Spill
	cmpq	$-1, %rax
	jne	LBB0_6
## %bb.158:
	movq	%r14, %rdi
	callq	_main.cold.3
LBB0_4:
	xorl	%eax, %eax
	movq	%rax, -80(%rbp)         ## 8-byte Spill
LBB0_6:
	movl	%ebx, %edi
	callq	_close
	movl	$4194304, %edi          ## imm = 0x400000
	callq	_malloc
	movq	%rax, %rbx
	movl	$4194304, %edi          ## imm = 0x400000
	callq	_malloc
	movq	%rax, -128(%rbp)        ## 8-byte Spill
	movl	$4194304, %edi          ## imm = 0x400000
	callq	_malloc
	movq	%rax, -160(%rbp)        ## 8-byte Spill
	movl	$262144, %edi           ## imm = 0x40000
	callq	_malloc
	movq	%rax, -104(%rbp)        ## 8-byte Spill
	movl	$65536, %edi            ## imm = 0x10000
	callq	_malloc
	movq	%rax, %r14
	movl	$65536, %edi            ## imm = 0x10000
	callq	_malloc
	movw	$11777, (%rbx)          ## imm = 0x2E01
	movb	$1, 2(%rbx)
	movw	$0, (%r14)
	movw	$357, 132(%rbx)         ## imm = 0x165
	movl	$1835101700, 128(%rbx)  ## imm = 0x6D616E04
	movw	$0, 2(%r14)
	movl	$16867074, 256(%rbx)    ## imm = 0x1015F02
	movw	$0, 4(%r14)
	movl	$16867586, 384(%rbx)    ## imm = 0x1016102
	movw	$0, 6(%r14)
	movl	$16867842, 512(%rbx)    ## imm = 0x1016202
	movw	$0, 8(%r14)
	movl	$16868098, 640(%rbx)    ## imm = 0x1016302
	movw	$0, 10(%r14)
	movl	$16868354, 768(%rbx)    ## imm = 0x1016402
	movw	$0, 12(%r14)
	movb	$2, 900(%rbx)
	movl	$1886350851, 896(%rbx)  ## imm = 0x706F6E03
	movw	$0, 14(%r14)
	movw	$513, 1028(%rbx)        ## imm = 0x201
	movl	$1818584068, 1024(%rbx) ## imm = 0x6C656404
	movw	$0, 16(%r14)
	movl	$33554790, 1155(%rbx)   ## imm = 0x2000166
	movl	$1717920773, 1152(%rbx) ## imm = 0x66656405
	movw	$0, 18(%r14)
	movabsq	$144680811267844614, %rcx ## imm = 0x202026E696F6A06
	movq	%rcx, 1280(%rbx)
	movq	%r14, -176(%rbp)        ## 8-byte Spill
	movw	$0, 20(%r14)
	movq	l___const.main.intrinsics+14(%rip), %rcx
	movq	%rcx, 14(%rax)
	movq	l___const.main.intrinsics+8(%rip), %rcx
	movq	%rcx, 8(%rax)
	movq	l___const.main.intrinsics(%rip), %rcx
	movq	%rcx, (%rax)
	testl	%r12d, %r12d
	movq	-80(%rbp), %r9          ## 8-byte Reload
	jle	LBB0_11
## %bb.7:
	movl	%r12d, %ecx
	xorl	%r15d, %r15d
	.p2align	4, 0x90
LBB0_8:                                 ## =>This Inner Loop Header: Depth=1
	cmpb	$32, (%r9,%r15)
	jg	LBB0_11
## %bb.9:                               ##   in Loop: Header=BB0_8 Depth=1
	incq	%r15
	cmpq	%r15, %rcx
	jne	LBB0_8
## %bb.10:
	movl	%r12d, %r15d
LBB0_11:
	movq	-104(%rbp), %r8         ## 8-byte Reload
	movl	%r15d, (%r8)
	leaq	4(%r8), %rcx
	movq	%rcx, -208(%rbp)        ## 8-byte Spill
	movl	$131083, 4(%r8)         ## imm = 0x2000B
	movslq	%r12d, %rcx
	leal	-1(%r12), %edx
	movl	%edx, -168(%rbp)        ## 4-byte Spill
	movl	$1, %edx
	movq	%rcx, -184(%rbp)        ## 8-byte Spill
	subq	%rcx, %rdx
	movq	%rdx, -88(%rbp)         ## 8-byte Spill
	leaq	1(%r9), %rcx
	movq	%rcx, -120(%rbp)        ## 8-byte Spill
	movl	$11, %ecx
	movq	%rcx, -136(%rbp)        ## 8-byte Spill
	xorl	%r11d, %r11d
	xorl	%esi, %esi
	xorl	%r14d, %r14d
	movq	%rbx, -56(%rbp)         ## 8-byte Spill
	movq	-128(%rbp), %r10        ## 8-byte Reload
	movq	%rax, -112(%rbp)        ## 8-byte Spill
	movq	%r12, -72(%rbp)         ## 8-byte Spill
	movq	%r9, -80(%rbp)          ## 8-byte Spill
	movslq	%r14d, %rdi
	movzwl	4(%r8,%rdi,8), %edx
	testw	%dx, %dx
	je	LBB0_14
LBB0_13:
	movl	%r11d, -44(%rbp)        ## 4-byte Spill
	leaq	4(%r8,%rdi,8), %rcx
	jmp	LBB0_19
	.p2align	4, 0x90
LBB0_14:
	movl	%esi, %r12d
	movq	-208(%rbp), %rcx        ## 8-byte Reload
	leaq	(%rcx,%rdi,8), %rcx
	xorl	%esi, %esi
	.p2align	4, 0x90
LBB0_15:                                ## =>This Inner Loop Header: Depth=1
	cmpq	%rsi, %rdi
	je	LBB0_16
## %bb.17:                              ##   in Loop: Header=BB0_15 Depth=1
	movzwl	-8(%rcx), %edx
	addq	$-8, %rcx
	incq	%rsi
	testw	%dx, %dx
	je	LBB0_15
## %bb.18:
	movl	%r11d, -44(%rbp)        ## 4-byte Spill
	movl	%edi, %r14d
	subl	%esi, %r14d
	subq	%rsi, %rdi
	movl	%r12d, %esi
LBB0_19:
	decl	%edx
	movw	%dx, (%rcx)
	movl	%r14d, %ecx
	shll	$6, %ecx
	orl	$1, %ecx
	movslq	%ecx, %rcx
	movq	-160(%rbp), %r13        ## 8-byte Reload
	movw	$0, (%r13,%rcx,2)
	movl	(%r8,%rdi,8), %r12d
	movl	%esi, -148(%rbp)        ## 4-byte Spill
	movslq	%esi, %rcx
	movq	%rcx, -64(%rbp)         ## 8-byte Spill
	xorl	%r11d, %r11d
LBB0_20:                                ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB0_26 Depth 2
                                        ##       Child Loop BB0_33 Depth 3
                                        ##     Child Loop BB0_44 Depth 2
                                        ##     Child Loop BB0_50 Depth 2
                                        ##     Child Loop BB0_55 Depth 2
	movl	%r14d, -92(%rbp)        ## 4-byte Spill
	movslq	%r14d, %rdx
	movswq	4(%r8,%rdx,8), %rcx
	movswq	(%rax,%rcx,2), %rax
	movl	%edx, %ecx
	shll	$6, %ecx
	movl	%ecx, -144(%rbp)        ## 4-byte Spill
	movslq	%ecx, %rcx
	movq	%rcx, -216(%rbp)        ## 8-byte Spill
	movw	%ax, (%r13,%rcx,2)
	movq	%rax, %rsi
	shlq	$7, %rsi
	movq	%r8, %rdi
	leaq	(%rbx,%rsi), %r8
	movq	%rdx, -200(%rbp)        ## 8-byte Spill
	movb	6(%rdi,%rdx,8), %cl
	movsbq	(%rbx,%rsi), %r14
	testb	%cl, %cl
	je	LBB0_23
## %bb.21:                              ##   in Loop: Header=BB0_20 Depth=1
	cmpb	1(%r14,%r8), %cl
	jne	LBB0_22
LBB0_23:                                ##   in Loop: Header=BB0_20 Depth=1
	cmpb	%r14b, %r11b
	jge	LBB0_24
## %bb.25:                              ##   in Loop: Header=BB0_20 Depth=1
	movsbq	%r11b, %rdx
	movq	-72(%rbp), %rsi         ## 8-byte Reload
	movl	-44(%rbp), %r11d        ## 4-byte Reload
	jmp	LBB0_26
	.p2align	4, 0x90
LBB0_34:                                ##   in Loop: Header=BB0_26 Depth=2
	movl	-168(%rbp), %ecx        ## 4-byte Reload
	movl	%esi, %r12d
LBB0_37:                                ##   in Loop: Header=BB0_26 Depth=2
	cmpl	%r15d, %ecx
	cmovgel	%eax, %r11d
	cmovgel	%r12d, %r15d
	cmpq	%r14, %rdx
	jge	LBB0_38
LBB0_26:                                ##   Parent Loop BB0_20 Depth=1
                                        ## =>  This Loop Header: Depth=2
                                        ##       Child Loop BB0_33 Depth 3
	movb	1(%r8,%rdx), %bl
	incq	%rdx
	cmpb	$32, %bl
	jle	LBB0_27
## %bb.29:                              ##   in Loop: Header=BB0_26 Depth=2
	cmpl	%esi, %r12d
	jge	LBB0_30
## %bb.31:                              ##   in Loop: Header=BB0_26 Depth=2
	movslq	%r12d, %rdi
	cmpb	(%r9,%rdi), %bl
	jne	LBB0_30
## %bb.32:                              ##   in Loop: Header=BB0_26 Depth=2
	movq	-88(%rbp), %rcx         ## 8-byte Reload
	leaq	(%rcx,%rdi), %rbx
	addq	-120(%rbp), %rdi        ## 8-byte Folded Reload
	xorl	%ecx, %ecx
	.p2align	4, 0x90
LBB0_33:                                ##   Parent Loop BB0_20 Depth=1
                                        ##     Parent Loop BB0_26 Depth=2
                                        ## =>    This Inner Loop Header: Depth=3
	cmpq	%rcx, %rbx
	je	LBB0_34
## %bb.35:                              ##   in Loop: Header=BB0_33 Depth=3
	decq	%rcx
	cmpb	$33, (%rdi)
	leaq	1(%rdi), %rdi
	jl	LBB0_33
## %bb.36:                              ##   in Loop: Header=BB0_26 Depth=2
	subl	%ecx, %r12d
	leal	-1(%r12), %ecx
	jmp	LBB0_37
	.p2align	4, 0x90
LBB0_24:                                ##   in Loop: Header=BB0_20 Depth=1
	movl	-44(%rbp), %r11d        ## 4-byte Reload
LBB0_38:                                ##   in Loop: Header=BB0_20 Depth=1
	cmpw	$9, %ax
	movq	-160(%rbp), %rdx        ## 8-byte Reload
	movl	%r11d, -44(%rbp)        ## 4-byte Spill
	je	LBB0_41
## %bb.39:                              ##   in Loop: Header=BB0_20 Depth=1
	cmpw	$8, %ax
	movq	-56(%rbp), %rbx         ## 8-byte Reload
	jne	LBB0_59
## %bb.40:                              ##   in Loop: Header=BB0_20 Depth=1
	movl	-144(%rbp), %eax        ## 4-byte Reload
	orl	$2, %eax
	cltq
	movswq	(%rdx,%rax,2), %rax
	shlq	$7, %rax
	movswq	(%r10,%rax), %rax
	shlq	$7, %rax
	movb	1(%rbx,%rax), %al
	movb	%al, 1(%rbx)
LBB0_59:                                ##   in Loop: Header=BB0_20 Depth=1
	movq	-64(%rbp), %r14         ## 8-byte Reload
	cmpq	$32767, %r14            ## imm = 0x7FFF
	je	LBB0_60
LBB0_61:                                ##   in Loop: Header=BB0_20 Depth=1
	movq	-216(%rbp), %rax        ## 8-byte Reload
	leaq	(%rdx,%rax,2), %rsi
	movl	%r14d, %eax
	shll	$6, %eax
	cltq
	leaq	(%r10,%rax,2), %rdi
	movl	-144(%rbp), %eax        ## 4-byte Reload
	orl	$1, %eax
	cltq
	movswq	(%rdx,%rax,2), %rax
	movq	%rdx, %r13
	leaq	4(%rax,%rax), %rdx
	callq	_memcpy
	incq	%r14
	movl	-92(%rbp), %eax         ## 4-byte Reload
	testl	%eax, %eax
	movq	%r14, -64(%rbp)         ## 8-byte Spill
	je	LBB0_65
## %bb.62:                              ##   in Loop: Header=BB0_20 Depth=1
	incl	-148(%rbp)              ## 4-byte Folded Spill
	decl	%eax
	movl	%eax, %edi
                                        ## kill: def $eax killed $eax def $rax
	shll	$6, %eax
	leal	1(%rax), %ecx
	movslq	%ecx, %rcx
	movswl	(%r13,%rcx,2), %esi
	cmpl	$61, %esi
	movq	-80(%rbp), %r9          ## 8-byte Reload
	movq	-104(%rbp), %r8         ## 8-byte Reload
	jg	LBB0_63
## %bb.64:                              ##   in Loop: Header=BB0_20 Depth=1
	movq	-200(%rbp), %rdx        ## 8-byte Reload
	movb	7(%r8,%rdx,8), %r11b
	leal	-1(%r14), %edx
	orl	$2, %eax
	addl	%esi, %eax
	cltq
	movw	%dx, (%r13,%rax,2)
	incw	(%r13,%rcx,2)
	movq	-128(%rbp), %r10        ## 8-byte Reload
	movq	-112(%rbp), %rax        ## 8-byte Reload
	movl	%edi, %r14d
	jmp	LBB0_20
	.p2align	4, 0x90
LBB0_41:                                ##   in Loop: Header=BB0_20 Depth=1
	movq	-136(%rbp), %rax        ## 8-byte Reload
	cmpl	$32767, %eax            ## imm = 0x7FFF
	movq	-56(%rbp), %rbx         ## 8-byte Reload
	je	LBB0_84
## %bb.42:                              ##   in Loop: Header=BB0_20 Depth=1
	movslq	%eax, %r13
	movq	%r13, %rax
	shlq	$7, %rax
	movb	$0, (%rbx,%rax)
	movl	-144(%rbp), %ecx        ## 4-byte Reload
	orl	$2, %ecx
	movslq	%ecx, %rcx
	movswq	(%rdx,%rcx,2), %rdx
	movq	%rdx, %rcx
	shlq	$7, %rcx
	cmpw	$0, 2(%r10,%rcx)
	je	LBB0_86
## %bb.43:                              ##   in Loop: Header=BB0_20 Depth=1
	addq	%rbx, %rax
	shlq	$6, %rdx
	xorl	%ecx, %ecx
	jmp	LBB0_44
	.p2align	4, 0x90
LBB0_47:                                ##   in Loop: Header=BB0_44 Depth=2
	movsbq	%cl, %rcx
	leaq	1(%rcx), %rdi
	movb	%dil, (%rax)
	movb	%sil, 1(%rax,%rcx)
	orl	$2, %edx
	movslq	%edx, %rcx
	movswq	(%r10,%rcx,2), %rcx
	movq	%rcx, %rdx
	shlq	$6, %rdx
	shlq	$7, %rcx
	cmpw	$0, 2(%r10,%rcx)
	movzbl	(%rax), %ecx
	je	LBB0_48
LBB0_44:                                ##   Parent Loop BB0_20 Depth=1
                                        ## =>  This Inner Loop Header: Depth=2
	cmpb	$127, %cl
	je	LBB0_83
## %bb.45:                              ##   in Loop: Header=BB0_44 Depth=2
	movslq	%edx, %rsi
	movzwl	(%r10,%rsi,2), %esi
	cmpw	$2, %si
	jle	LBB0_47
## %bb.46:                              ##   in Loop: Header=BB0_44 Depth=2
	shlq	$7, %rsi
	movzbl	1(%rbx,%rsi), %esi
	jmp	LBB0_47
LBB0_48:                                ##   in Loop: Header=BB0_20 Depth=1
	testb	%cl, %cl
	movq	-64(%rbp), %rdx         ## 8-byte Reload
	je	LBB0_85
## %bb.49:                              ##   in Loop: Header=BB0_20 Depth=1
	decb	%cl
	movb	%cl, (%rax)
	movq	-136(%rbp), %r8         ## 8-byte Reload
	leal	1(%r8), %ebx
	movq	%r13, %rax
	.p2align	4, 0x90
LBB0_50:                                ##   Parent Loop BB0_20 Depth=1
                                        ## =>  This Inner Loop Header: Depth=2
	testq	%rax, %rax
	je	LBB0_51
## %bb.52:                              ##   in Loop: Header=BB0_50 Depth=2
	movq	-112(%rbp), %rdx        ## 8-byte Reload
	movswq	-2(%rdx,%rax,2), %rdx
	decq	%rax
	shlq	$7, %rdx
	decl	%ebx
	movq	-56(%rbp), %rsi         ## 8-byte Reload
	cmpb	(%rsi,%rdx), %cl
	jl	LBB0_50
	jmp	LBB0_53
LBB0_51:                                ##   in Loop: Header=BB0_20 Depth=1
	xorl	%ebx, %ebx
LBB0_53:                                ##   in Loop: Header=BB0_20 Depth=1
	movslq	%ebx, %r14
	movq	-112(%rbp), %rax        ## 8-byte Reload
	leaq	(%rax,%r14,2), %rsi
	leaq	2(%rax,%r14,2), %rdi
	movl	%r8d, %eax
	subl	%r14d, %eax
	movslq	%eax, %rdx
	addq	%rdx, %rdx
	callq	_memmove
	movq	-136(%rbp), %rsi        ## 8-byte Reload
	movq	-112(%rbp), %rax        ## 8-byte Reload
	movw	%si, (%rax,%r14,2)
	movl	-92(%rbp), %edi         ## 4-byte Reload
	testl	%edi, %edi
	movq	-128(%rbp), %r10        ## 8-byte Reload
	movq	-160(%rbp), %r8         ## 8-byte Reload
	movq	-104(%rbp), %rdx        ## 8-byte Reload
	movl	-44(%rbp), %r11d        ## 4-byte Reload
	js	LBB0_58
## %bb.54:                              ##   in Loop: Header=BB0_20 Depth=1
	xorl	%eax, %eax
	jmp	LBB0_55
	.p2align	4, 0x90
LBB0_57:                                ##   in Loop: Header=BB0_55 Depth=2
	incl	%eax
	movswl	%ax, %ecx
	cmpl	%ecx, %edi
	jl	LBB0_58
LBB0_55:                                ##   Parent Loop BB0_20 Depth=1
                                        ## =>  This Inner Loop Header: Depth=2
	movswq	%ax, %rax
	movswl	4(%rdx,%rax,8), %ecx
	cmpl	%ecx, %ebx
	jg	LBB0_57
## %bb.56:                              ##   in Loop: Header=BB0_55 Depth=2
	incl	%ecx
	movw	%cx, 4(%rdx,%rax,8)
	jmp	LBB0_57
LBB0_58:                                ##   in Loop: Header=BB0_20 Depth=1
	movl	-144(%rbp), %eax        ## 4-byte Reload
	orl	$3, %eax
	cltq
	movq	%r8, %rdx
	movswq	(%r8,%rax,2), %rax
	movq	%rax, %rcx
	shlq	$7, %rcx
	cmpw	$0, (%r10,%rcx)
	movl	$0, %ecx
	cmovel	%ecx, %eax
	movq	-176(%rbp), %rcx        ## 8-byte Reload
	movw	%ax, (%rcx,%r13,2)
	incl	%esi
	movq	%rsi, -136(%rbp)        ## 8-byte Spill
	movq	-56(%rbp), %rbx         ## 8-byte Reload
	movq	-64(%rbp), %r14         ## 8-byte Reload
	cmpq	$32767, %r14            ## imm = 0x7FFF
	jne	LBB0_61
	jmp	LBB0_60
	.p2align	4, 0x90
LBB0_30:
	movq	-56(%rbp), %rbx         ## 8-byte Reload
	movq	-104(%rbp), %r8         ## 8-byte Reload
	movq	-112(%rbp), %rax        ## 8-byte Reload
	movl	-92(%rbp), %r14d        ## 4-byte Reload
	jmp	LBB0_67
	.p2align	4, 0x90
LBB0_27:
	movl	-92(%rbp), %r14d        ## 4-byte Reload
	cmpl	$32767, %r14d           ## imm = 0x7FFF
	movq	-112(%rbp), %rax        ## 8-byte Reload
	je	LBB0_82
## %bb.28:
	incl	%r14d
	movq	-104(%rbp), %r8         ## 8-byte Reload
	movq	-200(%rbp), %rsi        ## 8-byte Reload
	movl	%r12d, 8(%r8,%rsi,8)
	movq	-136(%rbp), %rcx        ## 8-byte Reload
	movw	%cx, 12(%r8,%rsi,8)
	movb	%bl, 14(%r8,%rsi,8)
	movb	%dl, 15(%r8,%rsi,8)
	movq	-56(%rbp), %rbx         ## 8-byte Reload
LBB0_67:
	movq	-64(%rbp), %rcx         ## 8-byte Reload
	movl	%ecx, %esi
	movslq	%r14d, %rdi
	movzwl	4(%r8,%rdi,8), %edx
	testw	%dx, %dx
	jne	LBB0_13
	jmp	LBB0_14
LBB0_22:
	movq	-104(%rbp), %r8         ## 8-byte Reload
	movq	-112(%rbp), %rax        ## 8-byte Reload
	movl	-44(%rbp), %r11d        ## 4-byte Reload
	movl	-92(%rbp), %r14d        ## 4-byte Reload
	jmp	LBB0_67
LBB0_65:
	xorl	%r14d, %r14d
	cmpl	-72(%rbp), %r12d        ## 4-byte Folded Reload
	movq	-80(%rbp), %r9          ## 8-byte Reload
	movq	-104(%rbp), %r8         ## 8-byte Reload
	movl	-44(%rbp), %r11d        ## 4-byte Reload
	je	LBB0_68
## %bb.66:
	movq	-128(%rbp), %r10        ## 8-byte Reload
	movq	-112(%rbp), %rax        ## 8-byte Reload
	jmp	LBB0_67
LBB0_16:
	leaq	L_.str.14(%rip), %r9
                                        ## kill: def $r12d killed $r12d def $r12
	jmp	LBB0_88
LBB0_83:
	leaq	L_.str.17(%rip), %r9
LBB0_87:
	movq	-64(%rbp), %r12         ## 8-byte Reload
LBB0_88:
	testl	%r15d, %r15d
	movq	%r12, -64(%rbp)         ## 8-byte Spill
	jle	LBB0_89
## %bb.90:
	movl	%r15d, %edx
	movl	%edx, %r10d
	andl	$1, %r10d
	cmpl	$1, %r15d
	jne	LBB0_92
## %bb.91:
	movl	$1, %r13d
	movl	$2, %edx
	xorl	%ecx, %ecx
                                        ## implicit-def: $r8d
	movl	%r11d, %r14d
	testq	%r10, %r10
	movq	-192(%rbp), %rsi        ## 8-byte Reload
	jne	LBB0_96
	jmp	LBB0_97
LBB0_89:
	movl	%r11d, %r14d
	movl	$1, %r13d
	movl	$1, %r8d
	movq	-192(%rbp), %rsi        ## 8-byte Reload
	jmp	LBB0_97
LBB0_92:
	subq	%r10, %rdx
	movl	$1, %esi
	xorl	%ecx, %ecx
	movl	$2, %edi
	movl	$1, %r8d
	movl	$1, %r13d
	movq	-80(%rbp), %rax         ## 8-byte Reload
	.p2align	4, 0x90
LBB0_93:                                ## =>This Inner Loop Header: Depth=1
	addl	$2, %r8d
	xorl	%ebx, %ebx
	cmpb	$10, (%rax,%rcx)
	sete	%bl
	cmovel	%edi, %r8d
	addl	%r13d, %ebx
	xorl	%r13d, %r13d
	cmpb	$10, 1(%rax,%rcx)
	leaq	2(%rcx), %rcx
	sete	%r13b
	cmovel	%esi, %r8d
	addl	%ebx, %r13d
	cmpq	%rcx, %rdx
	jne	LBB0_93
## %bb.94:
	leal	1(%r8), %edx
	movq	-56(%rbp), %rbx         ## 8-byte Reload
	movl	%r11d, %r14d
	testq	%r10, %r10
	movq	-192(%rbp), %rsi        ## 8-byte Reload
	je	LBB0_97
LBB0_96:
	xorl	%eax, %eax
	movq	-80(%rbp), %rdi         ## 8-byte Reload
	cmpb	$10, (%rdi,%rcx)
	sete	%al
	movl	$1, %ecx
	cmovel	%ecx, %edx
	addl	%eax, %r13d
	movl	%edx, %r8d
LBB0_97:
	movq	___stderrp@GOTPCREL(%rip), %rax
	movq	(%rax), %rdi
	movq	8(%rsi), %rdx
	leaq	L_.str.31(%rip), %rsi
	xorl	%r12d, %r12d
	movl	%r13d, %ecx
	movq	%r8, -144(%rbp)         ## 8-byte Spill
                                        ## kill: def $r8d killed $r8d killed $r8
	xorl	%eax, %eax
	callq	_fprintf
	testw	%r14w, %r14w
	je	LBB0_106
## %bb.98:
	movq	%rbx, %r15
	movswq	%r14w, %rbx
	shlq	$7, %rbx
	leaq	L_.str.32(%rip), %rdi
	xorl	%eax, %eax
	callq	_printf
	movb	(%r15,%rbx), %al
	testb	%al, %al
	js	LBB0_105
## %bb.99:
	addq	-56(%rbp), %rbx         ## 8-byte Folded Reload
	movb	$1, %r15b
	leaq	L_.str.33(%rip), %r14
	movsbl	%al, %esi
	cmpb	$32, %sil
	jg	LBB0_102
	.p2align	4, 0x90
LBB0_101:
	movq	%r14, %rdi
	xorl	%eax, %eax
	callq	_printf
	cmpb	%r15b, (%rbx)
	jge	LBB0_104
	jmp	LBB0_105
	.p2align	4, 0x90
LBB0_102:
	movl	%esi, %edi
	callq	_putchar
	cmpb	%r15b, (%rbx)
	jl	LBB0_105
LBB0_104:
	movsbq	%r15b, %r15
	movb	(%rbx,%r15), %al
	incb	%r15b
	movsbl	%al, %esi
	cmpb	$32, %sil
	jg	LBB0_102
	jmp	LBB0_101
LBB0_105:
	movl	$10, %edi
	callq	_putchar
	movq	-56(%rbp), %rbx         ## 8-byte Reload
LBB0_106:
	movl	%r13d, %eax
	subl	$2, %eax
	cmovbl	%r12d, %eax
	movl	%eax, -120(%rbp)        ## 4-byte Spill
	movq	-72(%rbp), %rsi         ## 8-byte Reload
	testl	%esi, %esi
	movq	-80(%rbp), %rdx         ## 8-byte Reload
	js	LBB0_130
## %bb.107:
	leal	2(%r13), %eax
	movl	%eax, -88(%rbp)         ## 4-byte Spill
	movl	%esi, %eax
	movq	%rax, -168(%rbp)        ## 8-byte Spill
	incl	%esi
	movl	$1, %ecx
	xorl	%r15d, %r15d
	movl	$1, %r14d
	movq	%rsi, -72(%rbp)         ## 8-byte Spill
	jmp	LBB0_108
	.p2align	4, 0x90
LBB0_128:                               ##   in Loop: Header=BB0_108 Depth=1
	incl	%ecx
	incq	%r15
	cmpq	%r15, %rsi
	je	LBB0_130
LBB0_108:                               ## =>This Inner Loop Header: Depth=1
	cmpl	-88(%rbp), %r14d        ## 4-byte Folded Reload
	jg	LBB0_112
## %bb.109:                             ##   in Loop: Header=BB0_108 Depth=1
	cmpl	-120(%rbp), %r14d       ## 4-byte Folded Reload
	jl	LBB0_112
## %bb.110:                             ##   in Loop: Header=BB0_108 Depth=1
	cmpl	$1, %ecx
	jne	LBB0_112
## %bb.111:                             ##   in Loop: Header=BB0_108 Depth=1
	leaq	L_.str.35(%rip), %rdi
	movl	%r14d, %esi
	xorl	%eax, %eax
	movq	%rbx, %r12
	movq	%r13, %rbx
	movl	%ecx, %r13d
	callq	_printf
	movl	%r13d, %ecx
	movq	%rbx, %r13
	movq	%r12, %rbx
	movq	-72(%rbp), %rsi         ## 8-byte Reload
	movq	-80(%rbp), %rdx         ## 8-byte Reload
LBB0_112:                               ##   in Loop: Header=BB0_108 Depth=1
	cmpq	%r15, -168(%rbp)        ## 8-byte Folded Reload
	jne	LBB0_113
## %bb.116:                             ##   in Loop: Header=BB0_108 Depth=1
	cmpl	-120(%rbp), %r14d       ## 4-byte Folded Reload
	jl	LBB0_125
## %bb.117:                             ##   in Loop: Header=BB0_108 Depth=1
	cmpl	-88(%rbp), %r14d        ## 4-byte Folded Reload
	jle	LBB0_118
	jmp	LBB0_125
	.p2align	4, 0x90
LBB0_113:                               ##   in Loop: Header=BB0_108 Depth=1
	cmpl	-88(%rbp), %r14d        ## 4-byte Folded Reload
	jg	LBB0_125
## %bb.114:                             ##   in Loop: Header=BB0_108 Depth=1
	cmpl	-120(%rbp), %r14d       ## 4-byte Folded Reload
	jl	LBB0_125
## %bb.115:                             ##   in Loop: Header=BB0_108 Depth=1
	cmpb	$10, (%rdx,%r15)
	je	LBB0_125
LBB0_118:                               ##   in Loop: Header=BB0_108 Depth=1
	movl	%r14d, %eax
	xorl	%r13d, %eax
	movl	%ecx, -44(%rbp)         ## 4-byte Spill
	xorl	-144(%rbp), %ecx        ## 4-byte Folded Reload
	orl	%eax, %ecx
	sete	%bl
	jne	LBB0_120
## %bb.119:                             ##   in Loop: Header=BB0_108 Depth=1
	leaq	L_.str.36(%rip), %rdi
	xorl	%eax, %eax
	callq	_printf
	movq	-72(%rbp), %rsi         ## 8-byte Reload
	movq	-80(%rbp), %rdx         ## 8-byte Reload
LBB0_120:                               ##   in Loop: Header=BB0_108 Depth=1
	cmpq	-184(%rbp), %r15        ## 8-byte Folded Reload
	jge	LBB0_121
## %bb.123:                             ##   in Loop: Header=BB0_108 Depth=1
	movsbl	(%rdx,%r15), %edi
	callq	_putchar
	movq	-72(%rbp), %rsi         ## 8-byte Reload
	movq	-80(%rbp), %rdx         ## 8-byte Reload
	testb	%bl, %bl
	movq	-56(%rbp), %rbx         ## 8-byte Reload
	movl	-44(%rbp), %ecx         ## 4-byte Reload
	jne	LBB0_124
	jmp	LBB0_125
	.p2align	4, 0x90
LBB0_121:                               ##   in Loop: Header=BB0_108 Depth=1
	testb	%bl, %bl
	movq	-56(%rbp), %rbx         ## 8-byte Reload
	movl	-44(%rbp), %ecx         ## 4-byte Reload
	je	LBB0_125
## %bb.122:                             ##   in Loop: Header=BB0_108 Depth=1
	leaq	L_.str.38(%rip), %rdi
	xorl	%eax, %eax
	callq	_printf
LBB0_124:                               ##   in Loop: Header=BB0_108 Depth=1
	leaq	L_.str.39(%rip), %rdi
	xorl	%eax, %eax
	callq	_printf
	movl	-44(%rbp), %ecx         ## 4-byte Reload
	movq	-72(%rbp), %rsi         ## 8-byte Reload
	movq	-80(%rbp), %rdx         ## 8-byte Reload
LBB0_125:                               ##   in Loop: Header=BB0_108 Depth=1
	cmpq	-184(%rbp), %r15        ## 8-byte Folded Reload
	jge	LBB0_128
## %bb.126:                             ##   in Loop: Header=BB0_108 Depth=1
	cmpb	$10, (%rdx,%r15)
	jne	LBB0_128
## %bb.127:                             ##   in Loop: Header=BB0_108 Depth=1
	incl	%r14d
	movl	$1, %ecx
	incq	%r15
	cmpq	%r15, %rsi
	jne	LBB0_108
LBB0_130:
	leaq	L_str(%rip), %rdi
	callq	_puts
	leaq	L_str.55(%rip), %rdi
	callq	_puts
	cmpl	$0, -64(%rbp)           ## 4-byte Folded Reload
	jle	LBB0_138
LBB0_133:
	movl	-64(%rbp), %eax         ## 4-byte Reload
	movq	%rax, -120(%rbp)        ## 8-byte Spill
	movq	-128(%rbp), %rax        ## 8-byte Reload
	addq	$4, %rax
	movq	%rax, -72(%rbp)         ## 8-byte Spill
	leaq	L_.str.43(%rip), %r15
	xorl	%esi, %esi
	movq	-128(%rbp), %r12        ## 8-byte Reload
	jmp	LBB0_134
	.p2align	4, 0x90
LBB0_137:                               ##   in Loop: Header=BB0_134 Depth=1
	leaq	L_str.62(%rip), %rdi
	callq	_puts
	movq	-88(%rbp), %rsi         ## 8-byte Reload
	incq	%rsi
	cmpq	-120(%rbp), %rsi        ## 8-byte Folded Reload
	movq	-56(%rbp), %rbx         ## 8-byte Reload
	je	LBB0_138
LBB0_134:                               ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB0_136 Depth 2
	movl	%esi, %r13d
	shll	$6, %r13d
	movswl	(%r12,%r13,2), %eax
	movslq	%eax, %rdx
	movq	%rdx, %rax
	shlq	$7, %rax
	movsbl	(%rbx,%rax), %ecx
	leaq	1(%rbx,%rax), %r8
	movswl	2(%r12,%r13,2), %r14d
	leaq	L_.str.42(%rip), %rdi
	movq	%rsi, -88(%rbp)         ## 8-byte Spill
                                        ## kill: def $esi killed $esi killed $rsi
                                        ## kill: def $edx killed $edx killed $rdx
	movl	%r14d, %r9d
	xorl	%eax, %eax
	callq	_printf
	testl	%r14d, %r14d
	jle	LBB0_137
## %bb.135:                             ##   in Loop: Header=BB0_134 Depth=1
	movq	-88(%rbp), %r14         ## 8-byte Reload
                                        ## kill: def $r14d killed $r14d killed $r14 def $r14
	andl	$67108863, %r14d        ## imm = 0x3FFFFFF
	shlq	$7, %r14
	addq	-72(%rbp), %r14         ## 8-byte Folded Reload
	xorl	%ebx, %ebx
	.p2align	4, 0x90
LBB0_136:                               ##   Parent Loop BB0_134 Depth=1
                                        ## =>  This Inner Loop Header: Depth=2
	movswl	(%r14,%rbx,2), %esi
	movq	%r15, %rdi
	xorl	%eax, %eax
	callq	_printf
	incq	%rbx
	movswq	2(%r12,%r13,2), %rax
	cmpq	%rax, %rbx
	jl	LBB0_136
	jmp	LBB0_137
LBB0_60:
	movl	$32767, %r12d           ## imm = 0x7FFF
	leaq	L_.str.19(%rip), %r9
	jmp	LBB0_88
LBB0_63:
	leaq	L_.str.20(%rip), %r9
	movl	-148(%rbp), %eax        ## 4-byte Reload
	movl	%eax, %r12d
	movl	-44(%rbp), %r11d        ## 4-byte Reload
	jmp	LBB0_88
LBB0_84:
	movl	$32767, %eax            ## imm = 0x7FFF
	movq	%rax, -136(%rbp)        ## 8-byte Spill
	leaq	L_.str.16(%rip), %r9
	jmp	LBB0_87
LBB0_86:
	leaq	L_.str.18(%rip), %r9
	jmp	LBB0_87
LBB0_82:
	leaq	L_.str.15(%rip), %r9
	movq	-56(%rbp), %rbx         ## 8-byte Reload
	jmp	LBB0_87
LBB0_85:
	leaq	L_.str.18(%rip), %r9
	movq	%rdx, %r12
	jmp	LBB0_88
LBB0_68:
	leaq	L_str.63(%rip), %rdi
	callq	_puts
	leaq	L_.str.24(%rip), %rdi
	movl	$1537, %esi             ## imm = 0x601
	xorl	%eax, %eax
	callq	_open
	testl	%eax, %eax
	js	LBB0_159
## %bb.69:
	leaq	L_.str.22(%rip), %rsi
	movl	$136, %edx
	movl	%eax, -72(%rbp)         ## 4-byte Spill
	movl	%eax, %edi
	callq	_write
	movq	-64(%rbp), %rax         ## 8-byte Reload
	decl	%eax
	movq	-104(%rbp), %rdi        ## 8-byte Reload
	movw	%ax, 4(%rdi)
	movw	$1, %r13w
	movq	-128(%rbp), %r8         ## 8-byte Reload
	jmp	LBB0_71
LBB0_75:                                ##   in Loop: Header=BB0_71 Depth=1
	movq	-88(%rbp), %r13         ## 8-byte Reload
                                        ## kill: def $r13w killed $r13w killed $r13 def $r13
	movq	-104(%rbp), %rdi        ## 8-byte Reload
LBB0_70:                                ##   in Loop: Header=BB0_71 Depth=1
	testw	%r13w, %r13w
	je	LBB0_131
LBB0_71:                                ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB0_81 Depth 2
	leal	-1(%r13), %r15d
	movswq	%r15w, %rax
	movswl	4(%rdi,%rax,8), %ecx
	movslq	%ecx, %rax
	movq	%rax, %r12
	shlq	$6, %r12
	shlq	$7, %rax
	movswl	(%r8,%rax), %r14d
	cmpl	$7, %r14d
	movq	%r15, -88(%rbp)         ## 8-byte Spill
	jne	LBB0_73
## %bb.72:                              ##   in Loop: Header=BB0_71 Depth=1
	leaq	L_str.65(%rip), %rdi
	callq	_puts
	movl	$5, %edx
	movl	-72(%rbp), %edi         ## 4-byte Reload
	leaq	L_.str.28(%rip), %rsi
	callq	_write
	movl	%r12d, %eax
	orl	$1, %eax
	movslq	%eax, %r15
	jmp	LBB0_74
LBB0_73:                                ##   in Loop: Header=BB0_71 Depth=1
	movslq	%r14d, %rbx
	leaq	L_str.64(%rip), %rdi
	movl	%ecx, -120(%rbp)        ## 4-byte Spill
	callq	_puts
	movswl	%r15w, %esi
	shlq	$7, %rbx
	movq	-56(%rbp), %rax         ## 8-byte Reload
	movsbl	(%rax,%rbx), %r8d
	movq	-56(%rbp), %rax         ## 8-byte Reload
	leaq	1(%rax,%rbx), %r9
	movq	-56(%rbp), %rbx         ## 8-byte Reload
	movl	%r12d, %eax
	orl	$1, %eax
	movslq	%eax, %r15
	movq	-128(%rbp), %rax        ## 8-byte Reload
	movswl	(%rax,%r15,2), %eax
	movl	%eax, (%rsp)
	leaq	L_.str.30(%rip), %rdi
	movl	-120(%rbp), %edx        ## 4-byte Reload
	movl	%r14d, %ecx
	xorl	%eax, %eax
	callq	_printf
LBB0_74:                                ##   in Loop: Header=BB0_71 Depth=1
	movq	-128(%rbp), %rax        ## 8-byte Reload
	movzwl	(%rax,%r15,2), %ecx
	testw	%cx, %cx
	movq	%rax, %r8
	je	LBB0_75
## %bb.76:                              ##   in Loop: Header=BB0_71 Depth=1
	orl	$2, %r12d
	testb	$1, %cl
	movq	-104(%rbp), %rdi        ## 8-byte Reload
	movq	-88(%rbp), %rsi         ## 8-byte Reload
	jne	LBB0_78
## %bb.77:                              ##   in Loop: Header=BB0_71 Depth=1
	movl	%ecx, %eax
	jmp	LBB0_79
LBB0_78:                                ##   in Loop: Header=BB0_71 Depth=1
	leal	-1(%rcx), %eax
	movswl	%ax, %edx
	addl	%r12d, %edx
	movslq	%edx, %rdx
	movzwl	(%r8,%rdx,2), %edx
	movswq	%si, %rsi
	movw	%dx, 4(%rdi,%rsi,8)
	movl	%r13d, %esi
LBB0_79:                                ##   in Loop: Header=BB0_71 Depth=1
	cmpw	$1, %cx
	je	LBB0_70
## %bb.80:                              ##   in Loop: Header=BB0_71 Depth=1
	decl	%eax
	movl	%esi, %r13d
	.p2align	4, 0x90
LBB0_81:                                ##   Parent Loop BB0_71 Depth=1
                                        ## =>  This Inner Loop Header: Depth=2
	movswl	%ax, %ecx
	addl	%r12d, %ecx
	movslq	%ecx, %rcx
	movzwl	(%r8,%rcx,2), %ecx
	leal	1(%r13), %edx
	movswq	%r13w, %rsi
	movw	%cx, 4(%rdi,%rsi,8)
	leal	-1(%rax), %ecx
	movswl	%cx, %ecx
	addl	%r12d, %ecx
	movslq	%ecx, %rcx
	movzwl	(%r8,%rcx,2), %ecx
	addl	$2, %r13d
	movswq	%dx, %rdx
	movw	%cx, 4(%rdi,%rdx,8)
	addl	$-2, %eax
	cmpw	$-1, %ax
	jne	LBB0_81
	jmp	LBB0_70
LBB0_131:
	leaq	L_.str.23(%rip), %rsi
	movl	$21, %edx
	movl	-72(%rbp), %r14d        ## 4-byte Reload
	movl	%r14d, %edi
	callq	_write
	movl	%r14d, %edi
	callq	_close
	leaq	L_str.55(%rip), %rdi
	callq	_puts
	cmpl	$0, -64(%rbp)           ## 4-byte Folded Reload
	jg	LBB0_133
LBB0_138:
	leaq	L_str.56(%rip), %rdi
	callq	_puts
	leaq	L_.str.46(%rip), %rdi
	movq	-136(%rbp), %r14        ## 8-byte Reload
	movl	%r14d, %esi
	xorl	%eax, %eax
	callq	_printf
	testl	%r14d, %r14d
	jle	LBB0_139
## %bb.144:
	movl	%r14d, %r12d
	leaq	L_.str.43(%rip), %r14
	xorl	%ebx, %ebx
	movq	-112(%rbp), %r15        ## 8-byte Reload
	.p2align	4, 0x90
LBB0_145:                               ## =>This Inner Loop Header: Depth=1
	movswl	(%r15,%rbx,2), %esi
	movq	%r14, %rdi
	xorl	%eax, %eax
	callq	_printf
	incq	%rbx
	cmpq	%rbx, %r12
	jne	LBB0_145
## %bb.146:
	leaq	L_str.57(%rip), %rdi
	callq	_puts
	movq	-56(%rbp), %rbx         ## 8-byte Reload
	movq	%rbx, %rax
	incq	%rax
	movq	%rax, -120(%rbp)        ## 8-byte Spill
	xorl	%r15d, %r15d
	movq	%r12, -88(%rbp)         ## 8-byte Spill
	jmp	LBB0_147
	.p2align	4, 0x90
LBB0_156:                               ##   in Loop: Header=BB0_147 Depth=1
	incq	%r15
	movq	-88(%rbp), %r12         ## 8-byte Reload
	cmpq	%r12, %r15
	movq	-56(%rbp), %rbx         ## 8-byte Reload
	je	LBB0_140
LBB0_147:                               ## =>This Inner Loop Header: Depth=1
	movl	%r15d, %r13d
	shll	$7, %r13d
	movsbl	(%rbx,%r13), %edx
	leaq	L_.str.48(%rip), %rdi
	movl	%r15d, %esi
	xorl	%eax, %eax
	callq	_printf
	movzbl	(%rbx,%r13), %eax
	cmpb	$-1, %al
	jl	LBB0_154
## %bb.148:                             ##   in Loop: Header=BB0_147 Depth=1
	movl	%r15d, %r12d
	andl	$33554431, %r12d        ## imm = 0x1FFFFFF
	shlq	$7, %r12
	addq	-120(%rbp), %r12        ## 8-byte Folded Reload
	xorl	%r14d, %r14d
	movsbl	%al, %esi
	cmpb	$32, %sil
	jg	LBB0_151
	.p2align	4, 0x90
LBB0_150:                               ##   in Loop: Header=BB0_147 Depth=1
	leaq	L_.str.33(%rip), %rdi
	xorl	%eax, %eax
	callq	_printf
	movsbq	(%rbx,%r13), %rax
	cmpq	%rax, %r14
	jle	LBB0_153
	jmp	LBB0_154
	.p2align	4, 0x90
LBB0_151:                               ##   in Loop: Header=BB0_147 Depth=1
	movl	%esi, %edi
	callq	_putchar
	movsbq	(%rbx,%r13), %rax
	cmpq	%rax, %r14
	jg	LBB0_154
LBB0_153:                               ##   in Loop: Header=BB0_147 Depth=1
	movzbl	(%r12,%r14), %eax
	incq	%r14
	movsbl	%al, %esi
	cmpb	$32, %sil
	jg	LBB0_151
	jmp	LBB0_150
	.p2align	4, 0x90
LBB0_154:                               ##   in Loop: Header=BB0_147 Depth=1
	leaq	L_str.59(%rip), %rdi
	callq	_puts
	movq	-176(%rbp), %rax        ## 8-byte Reload
	movzwl	(%rax,%r15,2), %ebx
	testw	%bx, %bx
	je	LBB0_156
## %bb.155:                             ##   in Loop: Header=BB0_147 Depth=1
	leaq	L_str.60(%rip), %rdi
	callq	_puts
	movswl	%bx, %esi
	movq	-128(%rbp), %rdi        ## 8-byte Reload
	xorl	%edx, %edx
	movq	-56(%rbp), %rcx         ## 8-byte Reload
	callq	_print_program
	leaq	L_str.61(%rip), %rdi
	callq	_puts
	jmp	LBB0_156
LBB0_139:
	leaq	L_str.57(%rip), %rdi
	callq	_puts
LBB0_140:
	leaq	L_str.58(%rip), %rdi
	callq	_puts
	movq	-64(%rbp), %rax         ## 8-byte Reload
	testl	%eax, %eax
	movq	-128(%rbp), %r14        ## 8-byte Reload
	je	LBB0_142
## %bb.141:
	decl	%eax
	movswl	%ax, %esi
	movq	%r14, %rdi
	xorl	%edx, %edx
	movq	%rbx, %rcx
	callq	_print_program
LBB0_142:
	movq	-80(%rbp), %rdi         ## 8-byte Reload
	movq	-184(%rbp), %rsi        ## 8-byte Reload
	callq	_munmap
	movq	%rbx, %rdi
	callq	_free
	movq	%r14, %rdi
	callq	_free
	movq	-160(%rbp), %rdi        ## 8-byte Reload
	callq	_free
	movq	-104(%rbp), %rdi        ## 8-byte Reload
	callq	_free
	movq	-176(%rbp), %rdi        ## 8-byte Reload
	callq	_free
	movq	-112(%rbp), %rdi        ## 8-byte Reload
	callq	_free
	xorl	%eax, %eax
LBB0_143:
	addq	$344, %rsp              ## imm = 0x158
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
LBB0_157:
	movq	%r14, %rdi
	callq	_main.cold.1
LBB0_159:
	callq	_main.cold.2
	.cfi_endproc
                                        ## -- End function
	.p2align	4, 0x90         ## -- Begin function print_program
_print_program:                         ## @print_program
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	pushq	%r15
	pushq	%r14
	pushq	%r13
	pushq	%r12
	pushq	%rbx
	pushq	%rax
	.cfi_offset %rbx, -56
	.cfi_offset %r12, -48
	.cfi_offset %r13, -40
	.cfi_offset %r14, -32
	.cfi_offset %r15, -24
	movq	%rcx, %r15
	movl	%esi, %r12d
	movq	%rdi, -48(%rbp)         ## 8-byte Spill
	movl	%edx, %r13d
	testl	%edx, %edx
	jle	LBB1_3
## %bb.1:
	leaq	L_.str.53(%rip), %r14
	movl	%r13d, %ebx
	.p2align	4, 0x90
LBB1_2:                                 ## =>This Inner Loop Header: Depth=1
	movq	%r14, %rdi
	xorl	%eax, %eax
	callq	_printf
	decl	%ebx
	jne	LBB1_2
LBB1_3:
	movswl	%r12w, %ebx
	shll	$6, %ebx
	movslq	%ebx, %rax
	movq	-48(%rbp), %r14         ## 8-byte Reload
	movswl	(%r14,%rax,2), %ecx
	movslq	%ecx, %rsi
	orl	$1, %eax
	cltq
	movswl	(%r14,%rax,2), %r12d
	movq	%rsi, %rax
	shlq	$7, %rax
	movsbl	(%r15,%rax), %ecx
	leaq	1(%r15,%rax), %r8
	leaq	L_.str.54(%rip), %rdi
                                        ## kill: def $esi killed $esi killed $rsi
	movl	%r12d, %edx
	xorl	%eax, %eax
	callq	_printf
	testl	%r12d, %r12d
	jle	LBB1_6
## %bb.4:
	movq	%r14, %rcx
	movzwl	%r12w, %r12d
	orl	$2, %ebx
	incl	%r13d
	movslq	%ebx, %rax
	leaq	(%r14,%rax,2), %rbx
	xorl	%r14d, %r14d
	.p2align	4, 0x90
LBB1_5:                                 ## =>This Inner Loop Header: Depth=1
	movswl	(%rbx,%r14,2), %esi
	movq	-48(%rbp), %rdi         ## 8-byte Reload
	movl	%r13d, %edx
	movq	%r15, %rcx
	callq	_print_program
	incq	%r14
	cmpq	%r14, %r12
	jne	LBB1_5
LBB1_6:
	addq	$8, %rsp
	popq	%rbx
	popq	%r12
	popq	%r13
	popq	%r14
	popq	%r15
	popq	%rbp
	retq
	.cfi_endproc
                                        ## -- End function
	.p2align	4, 0x90         ## -- Begin function main.cold.1
_main.cold.1:                           ## @main.cold.1
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movq	%rdi, %rdx
	movq	___stderrp@GOTPCREL(%rip), %rax
	movq	(%rax), %rdi
	leaq	L_.str(%rip), %rsi
	xorl	%eax, %eax
	callq	_fprintf
	leaq	L_.str.1(%rip), %rdi
	callq	_perror
	pushq	$3
	popq	%rdi
	callq	_exit
	.cfi_endproc
                                        ## -- End function
	.p2align	4, 0x90         ## -- Begin function main.cold.2
_main.cold.2:                           ## @main.cold.2
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	leaq	L_.str.25(%rip), %rdi
	leaq	L_.str.26(%rip), %rsi
	xorl	%eax, %eax
	callq	_printf
	leaq	L_.str.1(%rip), %rdi
	callq	_perror
	pushq	$1
	popq	%rdi
	callq	_exit
	.cfi_endproc
                                        ## -- End function
	.p2align	4, 0x90         ## -- Begin function main.cold.3
_main.cold.3:                           ## @main.cold.3
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movq	%rdi, %rdx
	movq	___stderrp@GOTPCREL(%rip), %rax
	movq	(%rax), %rdi
	leaq	L_.str(%rip), %rsi
	xorl	%eax, %eax
	callq	_fprintf
	leaq	L_.str.2(%rip), %rdi
	callq	_perror
	pushq	$4
	popq	%rdi
	callq	_exit
	.cfi_endproc
                                        ## -- End function
	.section	__TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
	.asciz	"compiler: error: %s: "

L_.str.1:                               ## @.str.1
	.asciz	"open"

L_.str.2:                               ## @.str.2
	.asciz	"mmap"

L_.str.3:                               ## @.str.3
	.asciz	"\001.\001"

L_.str.4:                               ## @.str.4
	.asciz	"\004name\001\001"

L_.str.10:                              ## @.str.10
	.asciz	"\003nop\002"

L_.str.11:                              ## @.str.11
	.asciz	"\004del\001\002"

	.section	__TEXT,__literal8,8byte_literals
L_.str.12:                              ## @.str.12
	.asciz	"\005def\001\000\002"

	.section	__TEXT,__const
	.p2align	4               ## @__const.main.intrinsics
l___const.main.intrinsics:
	.short	0                       ## 0x0
	.short	3                       ## 0x3
	.short	4                       ## 0x4
	.short	5                       ## 0x5
	.short	6                       ## 0x6
	.short	2                       ## 0x2
	.short	7                       ## 0x7
	.short	1                       ## 0x1
	.short	8                       ## 0x8
	.short	9                       ## 0x9
	.short	10                      ## 0xa

	.section	__TEXT,__cstring,cstring_literals
L_.str.14:                              ## @.str.14
	.asciz	"unresolved expression"

L_.str.15:                              ## @.str.15
	.asciz	"depth limit exceeded (32767)"

L_.str.16:                              ## @.str.16
	.asciz	"context limit exceeded (32767)"

L_.str.17:                              ## @.str.17
	.asciz	"signature limit exceeded (127)"

L_.str.18:                              ## @.str.18
	.asciz	"defining zero-length signature"

L_.str.19:                              ## @.str.19
	.asciz	"expression limit exceeded (32767)"

L_.str.20:                              ## @.str.20
	.asciz	"argument limit exceeded (62)"

L_.str.22:                              ## @.str.22
	.asciz	"\t.section\t__TEXT,__text,regular,pure_instructions\n\t.build_version macos, 11, 0\tsdk_version 11, 1\n\t.globl\t_main\n\t.p2align\t4, 0x90\n_main:\n"

L_.str.23:                              ## @.str.23
	.asciz	"\tmov $5, %rax\n\tretq\n\n"

L_.str.24:                              ## @.str.24
	.asciz	"out.s"

L_.str.25:                              ## @.str.25
	.asciz	"compile: error: %s: "

L_.str.26:                              ## @.str.26
	.asciz	"filename"

L_.str.28:                              ## @.str.28
	.asciz	"\tnop\n"

L_.str.30:                              ## @.str.30
	.asciz	"stack_count=%d | (expr=%d) : looking at %d (%.*s) (count=%d)\n"

L_.str.31:                              ## @.str.31
	.asciz	"\033[1m%s:%u:%u: \033[1;31merror:\033[m \033[1m%s\033[m\n"

L_.str.32:                              ## @.str.32
	.asciz	"candidate: "

L_.str.33:                              ## @.str.33
	.asciz	" (%d) "

L_.str.35:                              ## @.str.35
	.asciz	"\n\033[90m%5d\033[0m\033[32m \342\224\202 \033[0m"

L_.str.36:                              ## @.str.36
	.asciz	"\033[1;31m"

L_.str.38:                              ## @.str.38
	.asciz	"<EOF>"

L_.str.39:                              ## @.str.39
	.asciz	"\033[m"

L_.str.42:                              ## @.str.42
	.asciz	"%d | index=%d : \"%.*s\", count=%d, [ "

L_.str.43:                              ## @.str.43
	.asciz	"%d "

L_.str.46:                              ## @.str.46
	.asciz	"indicies = (%d){ "

L_.str.48:                              ## @.str.48
	.asciz	"%d | (length=%d) [ "

L_.str.53:                              ## @.str.53
	.asciz	".   "

L_.str.54:                              ## @.str.54
	.asciz	"[%d] : (%d) : %.*s\n\n"

L_str:                                  ## @str
	.asciz	"\n"

L_str.55:                               ## @str.55
	.asciz	"\n--------- program: -------- "

L_str.56:                               ## @str.56
	.asciz	"\n--------- context: -------- "

L_str.57:                               ## @str.57
	.asciz	"}"

L_str.58:                               ## @str.58
	.asciz	"-----------------------------\n"

L_str.59:                               ## @str.59
	.asciz	" ] "

L_str.60:                               ## @str.60
	.asciz	"MACRO DEF: "

L_str.61:                               ## @str.61
	.asciz	"END MACRO"

L_str.62:                               ## @str.62
	.asciz	"]"

L_str.63:                               ## @str.63
	.asciz	"\n\tcompile successful.\n"

L_str.64:                               ## @str.64
	.asciz	"found an unknown instruction..."

L_str.65:                               ## @str.65
	.asciz	"found a nop instruction..."

.subsections_via_symbols
