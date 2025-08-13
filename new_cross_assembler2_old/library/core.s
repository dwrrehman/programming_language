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

zero x set return_address x
incr x set output_format x 
incr x set executable_stack_size x 
incr x set uf2_family_id x 
incr x set overwrite_output x 
incr x set assembler_pass x 
incr x set assembler_putc x 

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


set a6_link 01111

set a6_sp 11111
set a6_zero 11111



(system call abi for macos)

set a6_number 00001
set a6_arg0 0
set a6_arg1 1
set a6_arg2 01
set a6_arg3 11


(system call numbers for macos)

set a6_exit 1
set a6_fork 01
set a6_read 11
set a6_write 001




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








