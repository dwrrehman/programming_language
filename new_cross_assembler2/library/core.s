zero false
zero true incr true
zero allones sub allones 1 set -1 allones

set stdin 0
set stdout 1
set stderr 01

zero target

zero x set no_arch x
incr x set riscv_arch x 
incr x set msp430_arch x 
incr x set arm64_arch x 

zero x set no_output x
incr x set bin_output x
incr x set hex_array x
incr x set macho_executable x
incr x set macho_object x
incr x set elf_executable x
incr x set elf_object x
incr x set ti_txt_executable x
incr x set uf2_executable x


zero x set assembler_return_address x
incr x set assembler_stack_pointer x
incr x set assembler_pass x
incr x set assembler_count x
incr x set assembler_data x
incr x set assembler_read x
incr x set assembler_write x
incr x set assembler_open x
incr x set output_format x 
incr x set output_name x 
incr x set overwrite_output x 
incr x set executable_stack_size x 
incr x set uf2_family_id x 
incr x set assembler_stack_base x

del x








(risc-v op codes)

set r_lui 		1110_110
set r_auipc 		1110_100

set r_jal		1111_011

set r_jalr_op1		1110_011
set r_jalr_op2		000

set r_branch		1100_011
set r_beq		000
set r_bne		100
set r_bltu		011
set r_bgeu		111
set r_blt		001
set r_bge		101

set r_load 		1100_000
set r_lb 		000
set r_lh 		100
set r_lw 		010
set r_ld 		110
set r_lbu 		001
set r_lhu 		101
set r_lwu 		011

set r_store 		1100_010
set r_sb 		000
set r_sh 		100
set r_sw 		010

set r_ecall 		1100_111
set r_imm 		1100_100
set r_signed		0000_010
set r_signedi		0000_0000_0010
set r_reg		1100_1100

set r_add		000
set r_sll		100
set r_slt		010
set r_sltu		110
set r_xor		001
set r_srl		101
set r_or		011
set r_and		111

set r_remu_op1		1100_110
set r_remu_op2		111
set r_remu_op3		1000000

set r_divu_op1		1100_110
set r_divu_op2		101
set r_divu_op3		1000000

set r_mul_op1		1100_110
set r_mul_op2		000
set r_mul_op3		1000000

set r_mulhu_op1		1100_110
set r_mulhu_op2		110
set r_mulhu_op3		1000000




(risc-v system call abi)

set r_arg0 	0101
set r_arg1 	1101
set r_arg2 	0011
set r_arg3 	1011
set r_arg4 	0111
set r_arg5 	1111

set r_number 	10001



(system call numbers for my riscv vm website)

set r_exit 1
set r_read 01
set r_write 11



















(macos specific, for macho files lol)

set min_stack_size_macos 1000_0000_0000_001



(arm64 machine instruction opcodes) 

set mov_type_zero 01


(arm64 hardware registers)


set a6_lr 01111
set a6_sp 11111
set a6_zero 11111


(system call numbers for macos arm64)

set a6_exit 		1
set a6_fork 		01
set a6_read 		11
set a6_write 		001
set a6_open 		101
set a6_close 		101
set a6_wait4		111
set a6_link 		1001
set a6_unlink 		0101
set a6_chdir 		0011
set a6_fchdir 		1011
set a6_chmod		1111
set a6_getpid		00101
set a6_recvmsg		11011
set a6_sendmsg		00111
set a6_recvfrom		10111
set a6_accept		01111
set a6_getsockname	000001
set a6_access		100001
set a6_sync		001001
set a6_kill		101001
set a6_dup		100101
set a6_pipe		010101
set a6_sigaction	011101
set a6_ioctl		011011
set a6_execve		110111
set a6_munmap		1001001
set a6_dup2		0101101
set a6_fcntl		0011101
set a6_select		1011101
set a6_fsync		1111101
set a6_socket		1000011
set a6_connect		0100011
set a6_bind		0001011
set a6_setsockopt	1001011
set a6_listen		0101011
set a6_gettimeofday	0010111
set a6_getsockopt	0110111
set a6_settimeofday	0101111
set a6_fchmod		0011111
set a6_rename		00000001
set a6_sendto		10100001
set a6_shutdown		01100001
set a6_mkdir		00010001
set a6_rmdir		10010001
set a6_mount		11100101
set a6_fdatasync	11011101
set a6_stat		00111101
set a6_fstat		10111111
set a6_lstat		01111111
set a6_getdirentries	00100011
set a6_mmap 		10100011
set a6_lseek		11100011
set a6_truncate		00010011
set a6_ftruncate	10010011
set a6_copyfile		11000111
set a6_poll		01100111
set a6_posix_spawn	00101111
set a6_openat 		111100111

(macos undocumented system calls: )

set a6_bsdthread_create			000101101
set a6_bsdthread_terminate 		100101101


(system call abi for macos)

set a6_number 00001

set a6_arg0 0
set a6_arg1 1
set a6_arg2 01
set a6_arg3 11
set a6_arg4 001
set a6_arg5 101
set a6_arg6 011
set a6_arg7 111




(conditions for bc branches and ccmp)

set cond_always 0111
set cond_never 1111

set is_equal 0
set is_not_equal 1
set is_negative 001
set is_nonnegative 101
set has_overflow_set 011
set has_overflow_clear 111
		
set is_signed_less 1101
set is_signed_greater 0011
set is_signed_less_or_equal 1011
set is_signed_greater_or_equal 0101
			
set is_unsigned_less 11
set is_unsigned_greater 0001
set is_unsigned_less_or_equal 1001
set is_unsigned_greater_or_equal 01





eoi 
------------------------------------------------------


the core standard library for the assembler

   written on 1202508037.200136 by dwrr






















(
9 link
10 unlink

12 chdir 
13 fchdir
15 chmod 
20 getpid


27 recvmsg
28 sendmsg
29 recvfrom
30 accept
32 getsockname

33 access
36 sync

37 kill


41 dup
42 pipe
46 sigaction

54 ioctl
59 execve

73 munmap


90 dup2

92 fcntl
93 select

95 fsync

97 socket
98 connect

104 bind
105 setsockopt

106 listen

116 gettimeofday

118 getsockopt


122 settimeofday

124 fchmod

128 rename

133 sendto
134 shutdown

136 mkdir 
137 rmdir

167 mount

187 fdatasync

188 stat
189 fstat
190 lstat

196 getdirentries

197 mmap

199 lseek

200 truncate
201 ftruncate

227 copyfile

230 poll

244 posix_spawn



360 bsdthread_create
361 bsdthread_terminate


463 openat



)


