(the standard library core file, 
defines only constants required to use the language
updated on 1202509173.013930)




zero false
zero true incr true
zero allones sub allones 1 set -1 allones

(define and init arguments for stdlib macros)

zero c0 zero c1
zero c2 zero c3
zero c3 zero c5
zero c6 zero c7


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

(init the compiletime stack pointer)

st assembler_stack_pointer assembler_stack_base



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
set r_reg		1100_110

set r_add		000
set r_si		100
set r_slts		010
set r_slt		110
set r_eor		001
set r_sd		101
set r_or		011
set r_and		111

set r_remu		111
set r_divu		101
set r_mul		000
set r_mulhu		110
set r_m			1000000

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


(macos specific, for macho executables)

set min_stack_size_macos 1000_0000_0000_001
set 16mb_stack_size_macos 0000_0000_0000_0000___0000_0000_1













(arm64 conditions for bc branches and ccmp)

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




(arm64 machine instruction opcodes / arguments) 

set subtract 1 
set setflags 1

set mem_store 		0
set mem_load 		1
set mem_load_signed 	01

set 1_byte  0 
set 2_bytes 1 
set 4_bytes 01
set 8_bytes 11

set shiftnone 	0

set movnegate 	0
set movzero 	01
set movkeep 	11

set is_nonzero is_not_equal
set is_zero is_equal

set bitwise_and 	0 
set bitwise_or 		1
set bitwise_eor 	01
set bitwise_and_setflags 11

set shift_incr 0
set shift_decr 1
set shift_decr_signed 01



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





(macos stuff: )

set pagesize 		0000_0000_0000_1
set prot_read 		1
set prot_write 		01
set map_private 	01
set map_anonymous 	0000_0000_0000_1
set map_failed 		allones



(ioctl requests)

set request_window_size  		0001_0110_0010_1110__0001_0000_0000_0010
set request_get_terminal_attributes	1100_1000_0010_1110__0001_0010_0000_0010
set request_set_terminal_attributes	0010_1000_0010_1110__0001_0010_0000_0001


(termios structure)

set terminal_attributes_inputflags	0 
set terminal_attributes_outputflags	1
set terminal_attributes_controlflags	01
set terminal_attributes_localflags	11
set terminal_attributes_controlcharacters 001


(termios member bitfield flags)

set echoinput 	0001
set canonicalmode 0000_0000_1









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


