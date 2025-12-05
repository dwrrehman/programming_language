zero false
set true 1

set -1 0 sub -1 1
set -2 -1 add -2 -1

set stdin  0
set stdout 1
set stderr 01

set byte 	1
set nat16 	01
set nat32 	001
set nat 	0001

zero x set compiler_return_address x
incr x set compiler_target x
incr x set compiler_format x 
incr x set compiler_should_overwrite x
incr x set compiler_should_debug x
incr x set compiler_stack_size x
incr x set compiler_length x
incr x set compiler_putc x

zero x set no_arch x
incr x set arm64_arch x
incr x set arm32_arch x
incr x set rv64_arch x
incr x set rv32_arch x
incr x set msp430_arch x

zero x set no_output x
incr x set macho_executable x
incr x set macho_object x
incr x set elf_executable x
incr x set elf_object x
incr x set ti_txt_executable x
incr x set uf2_executable x
incr x set hex_array x

zero x set c_system_debug x
incr x set c_system_exit x
incr x set c_system_read x
incr x set c_system_write x
incr x set c_system_open x
incr x set c_system_close x
incr x set c_system_mmap x
incr x set c_system_munmap x

zero x set c_system_number x
incr x set c_system_arg0 x
incr x set c_system_arg1 x
incr x set c_system_arg2 x
incr x set c_system_arg3 x
incr x set c_system_arg4 x
incr x set c_system_arg5 x
incr x set c_system_arg6 x

set prot_read 1
set prot_write 01
set map_private 01
set map_anonymous 0000_0000_0000_1
set map_failed -1

zero x set pc x
incr x set sp x
incr x set sr x
incr x set cg x

set msp_mov 	001
set msp_add 	101
set msp_addc 	011
set msp_sub 	111
set msp_subc 	0001
set msp_cmp 	1001
set msp_dadd 	0101
set msp_bit 	1101
set msp_bic 	0011
set msp_bis 	1011
set msp_xor 	0111
set msp_and 	1111

zero x set condjnz x
incr x set condjz x
incr x set condjnc x
incr x set condjc x
incr x set condjn x
incr x set condjge x
incr x set condjl x
incr x set condjmp x

set size_byte 1
set size_word 0

zero x set reg_mode x
incr x set index_mode x
incr x set deref_mode x
incr x set incr_mode x

set imm_mode incr_mode
set imm_reg pc
set literal_mode index_mode
set constant_1 cg
set fixed_reg sr
set fixed_mode index_mode

set bit0 10000000
set bit1 01000000
set bit2 00100000
set bit3 00010000
set bit4 00001000
set bit5 00000100
set bit6 00000010
set bit7 00000001




set rv_lui 		1110_110
set rv_auipc 		1110_100

set rv_jal		1111_011

set rv_jalr_op1		1110_011
set rv_jalr_op2		000

set rv_branch		1100_011
set rv_beq		000
set rv_bne		100
set rv_bltu		011
set rv_bgeu		111
set rv_blt		001
set rv_bge		101

set rv_load 		1100_010
set rv_lb 		000
set rv_lh 		100
set rv_lw 		010
set rv_ld 		110
set rv_lbu 		001
set rv_lhu 		101
set rv_lwu 		011

set rv_store 		1100_010
set rv_sb 		000
set rv_sh 		100
set rv_sw 		010

set rv_ecall 		1100_111
set rv_imm 		1100_100
set rv_signed		0000_010
set rv_signedi		0000_0000_0010
set rv_reg		1100_1100

set rv_add		000
set rv_sll		100
set rv_slt		010
set rv_sltu		110
set rv_xor		001
set rv_srl		101
set rv_or		011
set rv_and		111

set rv_remu_op1		1100_110
set rv_remu_op2		111
set rv_remu_op3		1000000

set rv_divu_op1		1100_110
set rv_divu_op2		101
set rv_divu_op3		1000000

set rv_mul_op1		1100_110
set rv_mul_op2		000
set rv_mul_op3		1000000

set rv_mulhu_op1	1100_110
set rv_mulhu_op2	110
set rv_mulhu_op3	1000000




set rv_zr 0
set rv_ra 1
set rv_sp 01

set rv_system_arg0 	0101
set rv_system_arg1 	1101
set rv_system_arg2 	0011
set rv_system_arg3 	1011
set rv_system_arg4 	0111
set rv_system_arg5 	1111
set rv_system_number 	10001


set x 1 set rv_system_exit x
incr x  set rv_system_read x
incr x  set rv_system_write x

del x

eoi


the core standard library for the optimizing assembler!
defines some basic constants and macros used by the 
rest of the language, and the various targets. 

  written by dwrr on 1202507185.024227

























