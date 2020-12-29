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
	pushq	%rbx
	subq	$1224, %rsp             ## imm = 0x4C8
	.cfi_offset %rbx, -24
	movq	___stack_chk_guard@GOTPCREL(%rip), %rax
	movq	(%rax), %rax
	movq	%rax, -16(%rbp)
	movl	$0, -596(%rbp)
	movl	%edi, -600(%rbp)
	movq	%rsi, -608(%rbp)
	cmpl	$2, -600(%rbp)
	jge	LBB0_2
## %bb.1:
	movl	$1, -596(%rbp)
	jmp	LBB0_84
LBB0_2:
	xorl	%eax, %eax
	movq	-608(%rbp), %rcx
	movq	8(%rcx), %rcx
	movq	%rcx, -616(%rbp)
	leaq	-760(%rbp), %rcx
	movq	%rcx, %rdi
	movl	%eax, %esi
	movl	$144, %edx
	movl	%eax, -1012(%rbp)       ## 4-byte Spill
	callq	_memset
	movq	-616(%rbp), %rdi
	movl	-1012(%rbp), %esi       ## 4-byte Reload
	movb	$0, %al
	callq	_open
	movl	%eax, -764(%rbp)
	cmpl	$0, -764(%rbp)
	jl	LBB0_4
## %bb.3:
	movq	-616(%rbp), %rdi
	leaq	-760(%rbp), %rsi
	callq	_stat$INODE64
	cmpl	$0, %eax
	jge	LBB0_5
LBB0_4:
	movq	___stderrp@GOTPCREL(%rip), %rax
	movq	(%rax), %rdi
	movq	-616(%rbp), %rdx
	leaq	L_.str(%rip), %rsi
	xorl	%ecx, %ecx
                                        ## kill: def $cl killed $cl killed $ecx
	movb	%cl, %al
	callq	_fprintf
	leaq	L_.str.1(%rip), %rdi
	movl	%eax, -1016(%rbp)       ## 4-byte Spill
	callq	_perror
	movl	$3, %edi
	callq	_exit
LBB0_5:
	xorl	%eax, %eax
	movl	%eax, %ecx
	movq	-664(%rbp), %rdx
                                        ## kill: def $edx killed $edx killed $rdx
	movl	%edx, -768(%rbp)
	movslq	-768(%rbp), %rsi
	movl	-764(%rbp), %r8d
	movq	%rcx, %rdi
	movl	$1, %eax
	movl	%eax, %edx
	movq	%rcx, -1024(%rbp)       ## 8-byte Spill
	movl	%eax, %ecx
	movq	-1024(%rbp), %r9        ## 8-byte Reload
	callq	_mmap
	movq	%rax, -776(%rbp)
	movq	$-1, %rax
	cmpq	%rax, -776(%rbp)
	jne	LBB0_7
## %bb.6:
	movq	___stderrp@GOTPCREL(%rip), %rax
	movq	(%rax), %rdi
	movq	-616(%rbp), %rdx
	leaq	L_.str(%rip), %rsi
	xorl	%ecx, %ecx
                                        ## kill: def $cl killed $cl killed $ecx
	movb	%cl, %al
	callq	_fprintf
	leaq	L_.str.2(%rip), %rdi
	movl	%eax, -1028(%rbp)       ## 4-byte Spill
	callq	_perror
	movl	$4, %edi
	callq	_exit
LBB0_7:
	movl	-764(%rbp), %edi
	callq	_close
	movl	$2359296, %edi          ## imm = 0x240000
	movl	%eax, -1032(%rbp)       ## 4-byte Spill
	callq	_malloc
	movq	%rax, -784(%rbp)
	movl	$2097152, %edi          ## imm = 0x200000
	callq	_malloc
	movq	%rax, -792(%rbp)
	movl	$2097152, %edi          ## imm = 0x200000
	callq	_malloc
	leaq	-400(%rbp), %rcx
	leaq	-336(%rbp), %rdx
	leaq	-272(%rbp), %rsi
	leaq	-208(%rbp), %rdi
	leaq	-144(%rbp), %r8
	leaq	-80(%rbp), %r9
	movq	%rax, -800(%rbp)
	movw	$0, -802(%rbp)
	movw	$0, -804(%rbp)
	movw	$0, -806(%rbp)
	movq	-800(%rbp), %rax
	movw	-806(%rbp), %r10w
	movw	%r10w, %r11w
	addw	$1, %r11w
	movw	%r11w, -806(%rbp)
	movswq	%r10w, %rbx
	shlq	$6, %rbx
	addq	%rbx, %rax
	movq	%r9, %rbx
	movq	%rdi, -1040(%rbp)       ## 8-byte Spill
	movq	%rbx, %rdi
	leaq	l_.str.3(%rip), %rbx
	movq	%rsi, -1048(%rbp)       ## 8-byte Spill
	movq	%rbx, %rsi
	movl	$60, %ebx
	movq	%rdx, -1056(%rbp)       ## 8-byte Spill
	movq	%rbx, %rdx
	movq	%rcx, -1064(%rbp)       ## 8-byte Spill
	movq	%r8, -1072(%rbp)        ## 8-byte Spill
	movq	%r9, -1080(%rbp)        ## 8-byte Spill
	movq	%rax, -1088(%rbp)       ## 8-byte Spill
	movq	%rbx, -1096(%rbp)       ## 8-byte Spill
	callq	_memcpy
	movw	$0, -20(%rbp)
	movb	$5, -18(%rbp)
	movb	$1, -17(%rbp)
	movq	-1088(%rbp), %rax       ## 8-byte Reload
	movq	-1080(%rbp), %rcx       ## 8-byte Reload
	movq	%rax, %rdi
	movq	%rcx, %rsi
	movl	$64, %eax
	movq	%rax, %rdx
	movq	%rax, -1104(%rbp)       ## 8-byte Spill
	callq	_memcpy
	movq	-800(%rbp), %rax
	movw	-806(%rbp), %r10w
	movw	%r10w, %r11w
	addw	$1, %r11w
	movw	%r11w, -806(%rbp)
	movswq	%r10w, %rcx
	shlq	$6, %rcx
	addq	%rcx, %rax
	movq	-1072(%rbp), %rcx       ## 8-byte Reload
	movq	%rcx, %rdi
	leaq	l_.str.4(%rip), %rsi
	movq	-1096(%rbp), %rdx       ## 8-byte Reload
	movq	%rax, -1112(%rbp)       ## 8-byte Spill
	callq	_memcpy
	movw	$0, -84(%rbp)
	movb	$4, -82(%rbp)
	movb	$1, -81(%rbp)
	movq	-1112(%rbp), %rax       ## 8-byte Reload
	movq	-1072(%rbp), %rcx       ## 8-byte Reload
	movq	%rax, %rdi
	movq	%rcx, %rsi
	movq	-1104(%rbp), %rdx       ## 8-byte Reload
	callq	_memcpy
	movq	-800(%rbp), %rax
	movw	-806(%rbp), %r10w
	movw	%r10w, %r11w
	addw	$1, %r11w
	movw	%r11w, -806(%rbp)
	movswq	%r10w, %rcx
	shlq	$6, %rcx
	addq	%rcx, %rax
	movq	-1040(%rbp), %rcx       ## 8-byte Reload
	movq	%rcx, %rdi
	leaq	l_.str.5(%rip), %rsi
	movq	-1096(%rbp), %rdx       ## 8-byte Reload
	movq	%rax, -1120(%rbp)       ## 8-byte Spill
	callq	_memcpy
	movw	$0, -148(%rbp)
	movb	$6, -146(%rbp)
	movb	$1, -145(%rbp)
	movq	-1120(%rbp), %rax       ## 8-byte Reload
	movq	-1040(%rbp), %rcx       ## 8-byte Reload
	movq	%rax, %rdi
	movq	%rcx, %rsi
	movq	-1104(%rbp), %rdx       ## 8-byte Reload
	callq	_memcpy
	movq	-800(%rbp), %rax
	movw	-806(%rbp), %r10w
	movw	%r10w, %r11w
	addw	$1, %r11w
	movw	%r11w, -806(%rbp)
	movswq	%r10w, %rcx
	shlq	$6, %rcx
	addq	%rcx, %rax
	movq	-1048(%rbp), %rcx       ## 8-byte Reload
	movq	%rcx, %rdi
	leaq	l_.str.6(%rip), %rsi
	movq	-1096(%rbp), %rdx       ## 8-byte Reload
	movq	%rax, -1128(%rbp)       ## 8-byte Spill
	callq	_memcpy
	movw	$0, -212(%rbp)
	movb	$5, -210(%rbp)
	movb	$1, -209(%rbp)
	movq	-1128(%rbp), %rax       ## 8-byte Reload
	movq	-1048(%rbp), %rcx       ## 8-byte Reload
	movq	%rax, %rdi
	movq	%rcx, %rsi
	movq	-1104(%rbp), %rdx       ## 8-byte Reload
	callq	_memcpy
	movq	-800(%rbp), %rax
	movw	-806(%rbp), %r10w
	movw	%r10w, %r11w
	addw	$1, %r11w
	movw	%r11w, -806(%rbp)
	movswq	%r10w, %rcx
	shlq	$6, %rcx
	addq	%rcx, %rax
	movq	-1056(%rbp), %rcx       ## 8-byte Reload
	movq	%rcx, %rdi
	leaq	l_.str.7(%rip), %rsi
	movq	-1096(%rbp), %rdx       ## 8-byte Reload
	movq	%rax, -1136(%rbp)       ## 8-byte Spill
	callq	_memcpy
	movw	$0, -276(%rbp)
	movb	$7, -274(%rbp)
	movb	$1, -273(%rbp)
	movq	-1136(%rbp), %rax       ## 8-byte Reload
	movq	-1056(%rbp), %rcx       ## 8-byte Reload
	movq	%rax, %rdi
	movq	%rcx, %rsi
	movq	-1104(%rbp), %rdx       ## 8-byte Reload
	callq	_memcpy
	movq	-800(%rbp), %rax
	movw	-806(%rbp), %r10w
	movw	%r10w, %r11w
	addw	$1, %r11w
	movw	%r11w, -806(%rbp)
	movswq	%r10w, %rcx
	shlq	$6, %rcx
	addq	%rcx, %rax
	movq	-1064(%rbp), %rcx       ## 8-byte Reload
	movq	%rcx, %rdi
	leaq	l_.str.8(%rip), %rsi
	movq	-1096(%rbp), %rdx       ## 8-byte Reload
	movq	%rax, -1144(%rbp)       ## 8-byte Spill
	callq	_memcpy
	movw	$0, -340(%rbp)
	movb	$3, -338(%rbp)
	movb	$1, -337(%rbp)
	movq	-1144(%rbp), %rax       ## 8-byte Reload
	movq	-1064(%rbp), %rcx       ## 8-byte Reload
	movq	%rax, %rdi
	movq	%rcx, %rsi
	movq	-1104(%rbp), %rdx       ## 8-byte Reload
	callq	_memcpy
	movl	$0, -812(%rbp)
	movl	$0, -816(%rbp)
	movw	$0, -818(%rbp)
	movw	$0, -820(%rbp)
	movb	$0, -821(%rbp)
	movb	$0, -822(%rbp)
LBB0_8:                                 ## =>This Inner Loop Header: Depth=1
	xorl	%eax, %eax
                                        ## kill: def $al killed $al killed $eax
	movl	-812(%rbp), %ecx
	cmpl	-768(%rbp), %ecx
	movb	%al, -1145(%rbp)        ## 1-byte Spill
	jge	LBB0_10
## %bb.9:                               ##   in Loop: Header=BB0_8 Depth=1
	movq	-776(%rbp), %rax
	movslq	-812(%rbp), %rcx
	movsbl	(%rax,%rcx), %edx
	cmpl	$32, %edx
	setle	%sil
	movb	%sil, -1145(%rbp)       ## 1-byte Spill
LBB0_10:                                ##   in Loop: Header=BB0_8 Depth=1
	movb	-1145(%rbp), %al        ## 1-byte Reload
	testb	$1, %al
	jne	LBB0_11
	jmp	LBB0_12
LBB0_11:                                ##   in Loop: Header=BB0_8 Depth=1
	movl	-812(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -812(%rbp)
	jmp	LBB0_8
LBB0_12:
	movl	-812(%rbp), %eax
	cmpl	-816(%rbp), %eax
	jle	LBB0_14
## %bb.13:
	movl	-812(%rbp), %eax
	movl	%eax, -816(%rbp)
LBB0_14:
	leaq	-896(%rbp), %rax
	xorl	%esi, %esi
	movq	-784(%rbp), %rcx
	movq	%rax, %rdx
	movq	%rdx, %rdi
	movl	$64, %edx
	movq	%rax, -1160(%rbp)       ## 8-byte Spill
	movq	%rcx, -1168(%rbp)       ## 8-byte Spill
	callq	_memset
	movw	-806(%rbp), %r8w
	movw	%r8w, -836(%rbp)
	movw	$0, -834(%rbp)
	movl	-812(%rbp), %esi
	movl	%esi, -832(%rbp)
	movb	$1, -828(%rbp)
	movb	$0, -827(%rbp)
	movw	$0, -826(%rbp)
	movq	-1168(%rbp), %rax       ## 8-byte Reload
	movq	-1160(%rbp), %rcx       ## 8-byte Reload
	movq	%rax, %rdi
	movq	%rcx, %rsi
	movl	$72, %edx
	callq	_memcpy
LBB0_15:                                ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB0_20 Depth 2
                                        ##       Child Loop BB0_23 Depth 3
                                        ##         Child Loop BB0_30 Depth 4
	movq	-784(%rbp), %rax
	movswq	-802(%rbp), %rcx
	imulq	$72, %rcx, %rcx
	addq	%rcx, %rax
	cmpw	$0, 60(%rax)
	jne	LBB0_19
## %bb.16:                              ##   in Loop: Header=BB0_15 Depth=1
	cmpw	$0, -802(%rbp)
	jne	LBB0_18
## %bb.17:
	movb	$1, -822(%rbp)
	jmp	LBB0_43
LBB0_18:                                ##   in Loop: Header=BB0_15 Depth=1
	movw	-802(%rbp), %ax
	addw	$-1, %ax
	movw	%ax, -802(%rbp)
	jmp	LBB0_42
LBB0_19:                                ##   in Loop: Header=BB0_15 Depth=1
	movq	-784(%rbp), %rax
	movswq	-802(%rbp), %rcx
	imulq	$72, %rcx, %rcx
	addq	%rcx, %rax
	movw	60(%rax), %dx
	addw	$-1, %dx
	movw	%dx, 60(%rax)
	movb	$0, -821(%rbp)
	movq	-784(%rbp), %rax
	movswq	-802(%rbp), %rcx
	imulq	$72, %rcx, %rcx
	addq	%rcx, %rax
	movl	64(%rax), %esi
	movl	%esi, -812(%rbp)
LBB0_20:                                ##   Parent Loop BB0_15 Depth=1
                                        ## =>  This Loop Header: Depth=2
                                        ##       Child Loop BB0_23 Depth 3
                                        ##         Child Loop BB0_30 Depth 4
	movq	-784(%rbp), %rax
	movswq	-802(%rbp), %rcx
	imulq	$72, %rcx, %rcx
	addq	%rcx, %rax
	movw	60(%rax), %dx
	movw	%dx, -818(%rbp)
	movq	-800(%rbp), %rax
	movswq	-818(%rbp), %rcx
	shlq	$6, %rcx
	addq	%rcx, %rax
	leaq	-464(%rbp), %rcx
	movq	%rcx, %rdi
	movq	%rax, %rsi
	movl	$64, %edx
	callq	_memcpy
	movq	-784(%rbp), %rax
	movswq	-802(%rbp), %rcx
	imulq	$72, %rcx, %rcx
	addq	%rcx, %rax
	movsbl	68(%rax), %r8d
	movsbl	-401(%rbp), %r9d
	cmpl	%r9d, %r8d
	je	LBB0_22
## %bb.21:                              ##   in Loop: Header=BB0_15 Depth=1
	jmp	LBB0_42
LBB0_22:                                ##   in Loop: Header=BB0_20 Depth=2
	jmp	LBB0_23
LBB0_23:                                ##   Parent Loop BB0_15 Depth=1
                                        ##     Parent Loop BB0_20 Depth=2
                                        ## =>    This Loop Header: Depth=3
                                        ##         Child Loop BB0_30 Depth 4
	movsbl	-821(%rbp), %eax
	movsbl	-402(%rbp), %ecx
	cmpl	%ecx, %eax
	jge	LBB0_37
## %bb.24:                              ##   in Loop: Header=BB0_23 Depth=3
	movb	-821(%rbp), %al
	movb	%al, %cl
	addb	$1, %cl
	movb	%cl, -821(%rbp)
	movsbq	%al, %rdx
	movb	-464(%rbp,%rdx), %al
	movb	%al, -897(%rbp)
	movsbl	-897(%rbp), %esi
	cmpl	$33, %esi
	jge	LBB0_26
## %bb.25:                              ##   in Loop: Header=BB0_15 Depth=1
	movw	-802(%rbp), %ax
	addw	$1, %ax
	movw	%ax, -802(%rbp)
	movw	-806(%rbp), %ax
	movq	-784(%rbp), %rcx
	movswq	-802(%rbp), %rdx
	imulq	$72, %rdx, %rdx
	addq	%rdx, %rcx
	movw	%ax, 60(%rcx)
	movq	-784(%rbp), %rcx
	movswq	-802(%rbp), %rdx
	imulq	$72, %rdx, %rdx
	addq	%rdx, %rcx
	movw	$0, 62(%rcx)
	movb	-897(%rbp), %sil
	movq	-784(%rbp), %rcx
	movswq	-802(%rbp), %rdx
	imulq	$72, %rdx, %rdx
	addq	%rdx, %rcx
	movb	%sil, 68(%rcx)
	movb	-821(%rbp), %sil
	movq	-784(%rbp), %rcx
	movswq	-802(%rbp), %rdx
	imulq	$72, %rdx, %rdx
	addq	%rdx, %rcx
	movb	%sil, 69(%rcx)
	movl	-812(%rbp), %edi
	movq	-784(%rbp), %rcx
	movswq	-802(%rbp), %rdx
	imulq	$72, %rdx, %rdx
	addq	%rdx, %rcx
	movl	%edi, 64(%rcx)
	jmp	LBB0_15
LBB0_26:                                ##   in Loop: Header=BB0_23 Depth=3
	movl	-812(%rbp), %eax
	cmpl	-768(%rbp), %eax
	jge	LBB0_28
## %bb.27:                              ##   in Loop: Header=BB0_23 Depth=3
	movsbl	-897(%rbp), %eax
	movq	-776(%rbp), %rcx
	movslq	-812(%rbp), %rdx
	movsbl	(%rcx,%rdx), %esi
	cmpl	%esi, %eax
	je	LBB0_29
LBB0_28:                                ##   in Loop: Header=BB0_15 Depth=1
	jmp	LBB0_42
LBB0_29:                                ##   in Loop: Header=BB0_23 Depth=3
	jmp	LBB0_30
LBB0_30:                                ##   Parent Loop BB0_15 Depth=1
                                        ##     Parent Loop BB0_20 Depth=2
                                        ##       Parent Loop BB0_23 Depth=3
                                        ## =>      This Inner Loop Header: Depth=4
	movl	-812(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -812(%rbp)
## %bb.31:                              ##   in Loop: Header=BB0_30 Depth=4
	xorl	%eax, %eax
                                        ## kill: def $al killed $al killed $eax
	movl	-812(%rbp), %ecx
	cmpl	-768(%rbp), %ecx
	movb	%al, -1169(%rbp)        ## 1-byte Spill
	jge	LBB0_33
## %bb.32:                              ##   in Loop: Header=BB0_30 Depth=4
	movq	-776(%rbp), %rax
	movslq	-812(%rbp), %rcx
	movsbl	(%rax,%rcx), %edx
	cmpl	$32, %edx
	setle	%sil
	movb	%sil, -1169(%rbp)       ## 1-byte Spill
LBB0_33:                                ##   in Loop: Header=BB0_30 Depth=4
	movb	-1169(%rbp), %al        ## 1-byte Reload
	testb	$1, %al
	jne	LBB0_30
## %bb.34:                              ##   in Loop: Header=BB0_23 Depth=3
	movl	-812(%rbp), %eax
	cmpl	-816(%rbp), %eax
	jle	LBB0_36
## %bb.35:                              ##   in Loop: Header=BB0_23 Depth=3
	movl	-812(%rbp), %eax
	movl	%eax, -816(%rbp)
	movw	-818(%rbp), %cx
	movw	%cx, -820(%rbp)
LBB0_36:                                ##   in Loop: Header=BB0_23 Depth=3
	jmp	LBB0_23
LBB0_37:                                ##   in Loop: Header=BB0_20 Depth=2
	cmpw	$0, -802(%rbp)
	je	LBB0_39
## %bb.38:                              ##   in Loop: Header=BB0_20 Depth=2
	movq	-784(%rbp), %rax
	movw	-802(%rbp), %cx
	movw	%cx, %dx
	addw	$-1, %dx
	movw	%dx, -802(%rbp)
	movswq	%cx, %rsi
	imulq	$72, %rsi, %rsi
	addq	%rsi, %rax
	movb	69(%rax), %dil
	movb	%dil, -821(%rbp)
	movw	-804(%rbp), %cx
	movq	-784(%rbp), %rax
	movswq	-802(%rbp), %rsi
	imulq	$72, %rsi, %rsi
	addq	%rsi, %rax
	movq	-784(%rbp), %rsi
	movswq	-802(%rbp), %r8
	imulq	$72, %r8, %r8
	addq	%r8, %rsi
	movw	62(%rsi), %dx
	movw	%dx, %r9w
	addw	$1, %r9w
	movw	%r9w, 62(%rsi)
	movswq	%dx, %rsi
	movw	%cx, (%rax,%rsi,2)
	movq	-792(%rbp), %rax
	movw	-804(%rbp), %cx
	movw	%cx, %dx
	addw	$1, %dx
	movw	%dx, -804(%rbp)
	movswq	%cx, %rsi
	shlq	$6, %rsi
	addq	%rsi, %rax
	movq	-784(%rbp), %rsi
	movswl	-802(%rbp), %r10d
	addl	$1, %r10d
	movslq	%r10d, %r8
	imulq	$72, %r8, %r8
	addq	%r8, %rsi
	movq	%rax, %rdi
	movl	$64, %edx
	callq	_memcpy
	jmp	LBB0_20
LBB0_39:                                ##   in Loop: Header=BB0_15 Depth=1
	movl	-812(%rbp), %eax
	cmpl	-768(%rbp), %eax
	jne	LBB0_41
## %bb.40:
	jmp	LBB0_43
LBB0_41:                                ##   in Loop: Header=BB0_15 Depth=1
	jmp	LBB0_42
LBB0_42:                                ##   in Loop: Header=BB0_15 Depth=1
	movq	-784(%rbp), %rax
	movswq	-802(%rbp), %rcx
	imulq	$72, %rcx, %rcx
	addq	%rcx, %rax
	movw	$0, 62(%rax)
	jmp	LBB0_15
LBB0_43:
	movq	-792(%rbp), %rax
	movw	-804(%rbp), %cx
	movw	%cx, %dx
	addw	$1, %dx
	movw	%dx, -804(%rbp)
	movswq	%cx, %rsi
	shlq	$6, %rsi
	addq	%rsi, %rax
	movq	-784(%rbp), %rsi
	movswq	-802(%rbp), %rdi
	imulq	$72, %rdi, %rdi
	addq	%rdi, %rsi
	movq	%rax, %rdi
	movl	$64, %edx
	callq	_memcpy
	leaq	L_.str.9(%rip), %rdi
	movb	$0, %al
	callq	_printf
	movl	$0, -904(%rbp)
LBB0_44:                                ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB0_46 Depth 2
	movl	-904(%rbp), %eax
	movswl	-804(%rbp), %ecx
	cmpl	%ecx, %eax
	jge	LBB0_51
## %bb.45:                              ##   in Loop: Header=BB0_44 Depth=1
	movq	-792(%rbp), %rax
	movslq	-904(%rbp), %rcx
	shlq	$6, %rcx
	addq	%rcx, %rax
	leaq	-968(%rbp), %rcx
	movq	%rcx, %rdi
	movq	%rax, %rsi
	movl	$64, %edx
	callq	_memcpy
	movl	-904(%rbp), %esi
	movswl	-908(%rbp), %edx
	movswl	-906(%rbp), %ecx
	leaq	L_.str.10(%rip), %rdi
	movb	$0, %al
	callq	_printf
	movl	$0, -972(%rbp)
LBB0_46:                                ##   Parent Loop BB0_44 Depth=1
                                        ## =>  This Inner Loop Header: Depth=2
	movl	-972(%rbp), %eax
	movswl	-906(%rbp), %ecx
	cmpl	%ecx, %eax
	jge	LBB0_49
## %bb.47:                              ##   in Loop: Header=BB0_46 Depth=2
	movslq	-972(%rbp), %rax
	movswl	-968(%rbp,%rax,2), %esi
	leaq	L_.str.11(%rip), %rdi
	movb	$0, %al
	callq	_printf
## %bb.48:                              ##   in Loop: Header=BB0_46 Depth=2
	movl	-972(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -972(%rbp)
	jmp	LBB0_46
LBB0_49:                                ##   in Loop: Header=BB0_44 Depth=1
	leaq	L_.str.12(%rip), %rdi
	movb	$0, %al
	callq	_printf
## %bb.50:                              ##   in Loop: Header=BB0_44 Depth=1
	movl	-904(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -904(%rbp)
	jmp	LBB0_44
LBB0_51:
	leaq	L_.str.13(%rip), %rdi
	movb	$0, %al
	callq	_printf
	movl	$0, -976(%rbp)
LBB0_52:                                ## =>This Loop Header: Depth=1
                                        ##     Child Loop BB0_54 Depth 2
	movl	-976(%rbp), %eax
	movswl	-806(%rbp), %ecx
	cmpl	%ecx, %eax
	jge	LBB0_62
## %bb.53:                              ##   in Loop: Header=BB0_52 Depth=1
	movq	-800(%rbp), %rax
	movslq	-976(%rbp), %rcx
	shlq	$6, %rcx
	addq	%rcx, %rax
	leaq	-528(%rbp), %rcx
	movq	%rcx, %rdi
	movq	%rax, %rsi
	movl	$64, %edx
	callq	_memcpy
	movl	-976(%rbp), %esi
	movsbl	-465(%rbp), %edx
	movsbl	-466(%rbp), %ecx
	movswl	-468(%rbp), %r8d
	leaq	L_.str.14(%rip), %rdi
	movb	$0, %al
	callq	_printf
	movl	$0, -980(%rbp)
LBB0_54:                                ##   Parent Loop BB0_52 Depth=1
                                        ## =>  This Inner Loop Header: Depth=2
	movl	-980(%rbp), %eax
	movsbl	-466(%rbp), %ecx
	cmpl	%ecx, %eax
	jge	LBB0_60
## %bb.55:                              ##   in Loop: Header=BB0_54 Depth=2
	movslq	-980(%rbp), %rax
	movsbl	-528(%rbp,%rax), %ecx
	cmpl	$33, %ecx
	jl	LBB0_57
## %bb.56:                              ##   in Loop: Header=BB0_54 Depth=2
	movslq	-980(%rbp), %rax
	movsbl	-528(%rbp,%rax), %esi
	leaq	L_.str.15(%rip), %rdi
	movb	$0, %al
	callq	_printf
	jmp	LBB0_58
LBB0_57:                                ##   in Loop: Header=BB0_54 Depth=2
	movslq	-980(%rbp), %rax
	movsbl	-528(%rbp,%rax), %esi
	leaq	L_.str.16(%rip), %rdi
	movb	$0, %al
	callq	_printf
LBB0_58:                                ##   in Loop: Header=BB0_54 Depth=2
	jmp	LBB0_59
LBB0_59:                                ##   in Loop: Header=BB0_54 Depth=2
	movl	-980(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -980(%rbp)
	jmp	LBB0_54
LBB0_60:                                ##   in Loop: Header=BB0_52 Depth=1
	leaq	L_.str.12(%rip), %rdi
	movb	$0, %al
	callq	_printf
## %bb.61:                              ##   in Loop: Header=BB0_52 Depth=1
	movl	-976(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -976(%rbp)
	jmp	LBB0_52
LBB0_62:
	leaq	L_.str.17(%rip), %rdi
	movb	$0, %al
	callq	_printf
	cmpb	$0, -822(%rbp)
	je	LBB0_80
## %bb.63:
	movl	$0, -984(%rbp)
	movl	$1, -988(%rbp)
	movl	$1, -992(%rbp)
LBB0_64:                                ## =>This Inner Loop Header: Depth=1
	movl	-984(%rbp), %eax
	cmpl	-816(%rbp), %eax
	jge	LBB0_69
## %bb.65:                              ##   in Loop: Header=BB0_64 Depth=1
	movq	-776(%rbp), %rax
	movslq	-984(%rbp), %rcx
	movsbl	(%rax,%rcx), %edx
	cmpl	$10, %edx
	jne	LBB0_67
## %bb.66:                              ##   in Loop: Header=BB0_64 Depth=1
	movl	-988(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -988(%rbp)
	movl	$1, -992(%rbp)
	jmp	LBB0_68
LBB0_67:                                ##   in Loop: Header=BB0_64 Depth=1
	movl	-992(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -992(%rbp)
LBB0_68:                                ##   in Loop: Header=BB0_64 Depth=1
	movl	-984(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -984(%rbp)
	jmp	LBB0_64
LBB0_69:
	movq	___stderrp@GOTPCREL(%rip), %rax
	movq	(%rax), %rdi
	movl	-988(%rbp), %ecx
	movl	-992(%rbp), %r8d
	movl	-816(%rbp), %edx
	cmpl	-768(%rbp), %edx
	movq	%rdi, -1184(%rbp)       ## 8-byte Spill
	movl	%ecx, -1188(%rbp)       ## 4-byte Spill
	movl	%r8d, -1192(%rbp)       ## 4-byte Spill
	jne	LBB0_71
## %bb.70:
	movl	$32, %eax
	movl	%eax, -1196(%rbp)       ## 4-byte Spill
	jmp	LBB0_72
LBB0_71:
	movq	-776(%rbp), %rax
	movslq	-816(%rbp), %rcx
	movsbl	(%rax,%rcx), %edx
	movl	%edx, -1196(%rbp)       ## 4-byte Spill
LBB0_72:
	movl	-1196(%rbp), %eax       ## 4-byte Reload
	movq	-1184(%rbp), %rdi       ## 8-byte Reload
	leaq	L_.str.18(%rip), %rsi
	leaq	L_.str.19(%rip), %rdx
	movl	-1188(%rbp), %ecx       ## 4-byte Reload
	movl	-1192(%rbp), %r8d       ## 4-byte Reload
	movl	%eax, %r9d
	movb	$0, %al
	callq	_fprintf
	movq	-800(%rbp), %rdx
	movswq	-820(%rbp), %rsi
	shlq	$6, %rsi
	addq	%rsi, %rdx
	leaq	-592(%rbp), %rsi
	movq	%rsi, %rdi
	movq	%rdx, %rsi
	movl	$64, %edx
	movl	%eax, -1200(%rbp)       ## 4-byte Spill
	callq	_memcpy
	leaq	L_.str.20(%rip), %rdi
	movb	$0, %al
	callq	_printf
	movb	$0, -993(%rbp)
LBB0_73:                                ## =>This Inner Loop Header: Depth=1
	movsbl	-993(%rbp), %eax
	movsbl	-530(%rbp), %ecx
	cmpl	%ecx, %eax
	jge	LBB0_79
## %bb.74:                              ##   in Loop: Header=BB0_73 Depth=1
	movsbq	-993(%rbp), %rax
	movb	-592(%rbp,%rax), %cl
	movb	%cl, -994(%rbp)
	movsbl	-994(%rbp), %edx
	cmpl	$33, %edx
	jge	LBB0_76
## %bb.75:                              ##   in Loop: Header=BB0_73 Depth=1
	movsbl	-994(%rbp), %esi
	leaq	L_.str.16(%rip), %rdi
	movb	$0, %al
	callq	_printf
	jmp	LBB0_77
LBB0_76:                                ##   in Loop: Header=BB0_73 Depth=1
	movsbl	-994(%rbp), %esi
	leaq	L_.str.15(%rip), %rdi
	movb	$0, %al
	callq	_printf
LBB0_77:                                ##   in Loop: Header=BB0_73 Depth=1
	jmp	LBB0_78
LBB0_78:                                ##   in Loop: Header=BB0_73 Depth=1
	movb	-993(%rbp), %al
	addb	$1, %al
	movb	%al, -993(%rbp)
	jmp	LBB0_73
LBB0_79:
	movsbl	-529(%rbp), %esi
	leaq	L_.str.21(%rip), %rdi
	movb	$0, %al
	callq	_printf
	jmp	LBB0_81
LBB0_80:
	leaq	L_.str.22(%rip), %rdi
	movb	$0, %al
	callq	_printf
LBB0_81:
	leaq	L_.str.23(%rip), %rdi
	movl	$1537, %esi             ## imm = 0x601
	movb	$0, %al
	callq	_open
	movl	%eax, -1000(%rbp)
	cmpl	$0, -1000(%rbp)
	jge	LBB0_83
## %bb.82:
	leaq	L_.str.24(%rip), %rdi
	leaq	L_.str.19(%rip), %rsi
	xorl	%eax, %eax
                                        ## kill: def $al killed $al killed $eax
	callq	_printf
	leaq	L_.str.1(%rip), %rdi
	movl	%eax, -1204(%rbp)       ## 4-byte Spill
	callq	_perror
	movl	$1, %edi
	callq	_exit
LBB0_83:
	leaq	L_.str.25(%rip), %rax
	movq	%rax, -1008(%rbp)
	movl	-1000(%rbp), %edi
	movq	-1008(%rbp), %rsi
	movq	-1008(%rbp), %rax
	movl	%edi, -1208(%rbp)       ## 4-byte Spill
	movq	%rax, %rdi
	movq	%rsi, -1216(%rbp)       ## 8-byte Spill
	callq	_strlen
	movl	-1208(%rbp), %edi       ## 4-byte Reload
	movq	-1216(%rbp), %rsi       ## 8-byte Reload
	movq	%rax, %rdx
	callq	_write
	movl	-1000(%rbp), %edi
	movq	%rax, -1224(%rbp)       ## 8-byte Spill
	callq	_close
	movq	-784(%rbp), %rcx
	movq	%rcx, %rdi
	movl	%eax, -1228(%rbp)       ## 4-byte Spill
	callq	_free
	movq	-792(%rbp), %rcx
	movq	%rcx, %rdi
	callq	_free
	movq	-800(%rbp), %rcx
	movq	%rcx, %rdi
	callq	_free
LBB0_84:
	movl	-596(%rbp), %eax
	movq	___stack_chk_guard@GOTPCREL(%rip), %rcx
	movq	(%rcx), %rcx
	movq	-16(%rbp), %rdx
	cmpq	%rdx, %rcx
	movl	%eax, -1232(%rbp)       ## 4-byte Spill
	jne	LBB0_86
## %bb.85:
	movl	-1232(%rbp), %eax       ## 4-byte Reload
	addq	$1224, %rsp             ## imm = 0x4C8
	popq	%rbx
	popq	%rbp
	retq
LBB0_86:
	callq	___stack_chk_fail
	ud2
	.cfi_endproc
                                        ## -- End function
	.section	__TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
	.asciz	"compiler: error: %s: "

L_.str.1:                               ## @.str.1
	.asciz	"open"

L_.str.2:                               ## @.str.2
	.asciz	"mmap"

	.section	__TEXT,__const
l_.str.3:                               ## @.str.3
	.asciz	"undef\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

l_.str.4:                               ## @.str.4
	.asciz	"unit\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

l_.str.5:                               ## @.str.5
	.asciz	"join\001\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

l_.str.6:                               ## @.str.6
	.asciz	"hello\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

l_.str.7:                               ## @.str.7
	.asciz	"define\001\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

l_.str.8:                               ## @.str.8
	.asciz	"wef\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000\000"

	.section	__TEXT,__cstring,cstring_literals
L_.str.9:                               ## @.str.9
	.asciz	"\n--------- program: -------- \n"

L_.str.10:                              ## @.str.10
	.asciz	"%d | index=%d, count=%d, [ "

L_.str.11:                              ## @.str.11
	.asciz	"%d "

L_.str.12:                              ## @.str.12
	.asciz	"]\n"

L_.str.13:                              ## @.str.13
	.asciz	"\n--------- context: -------- \n"

L_.str.14:                              ## @.str.14
	.asciz	"%d | type=%d, length=%d, def=%d, [ "

L_.str.15:                              ## @.str.15
	.asciz	"%c "

L_.str.16:                              ## @.str.16
	.asciz	"(%d) "

L_.str.17:                              ## @.str.17
	.asciz	"-----------------------------\n\n"

L_.str.18:                              ## @.str.18
	.asciz	"compiler: %s: %u:%u: error: unresolved %c\n"

L_.str.19:                              ## @.str.19
	.asciz	"filename"

L_.str.20:                              ## @.str.20
	.asciz	"...did you mean:  "

L_.str.21:                              ## @.str.21
	.asciz	" which has type (%d)\n"

L_.str.22:                              ## @.str.22
	.asciz	"\n\tcompile successful.\n\n"

L_.str.23:                              ## @.str.23
	.asciz	"out.s"

L_.str.24:                              ## @.str.24
	.asciz	"compile: error: %s: "

L_.str.25:                              ## @.str.25
	.asciz	"this is a file.\nits pretty cool.\nyup.\n"

.subsections_via_symbols
