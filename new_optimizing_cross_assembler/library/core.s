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
incr x set compiler_putc x

zero x set no_arch x
incr x set arm64_arch x
incr x set arm32_arch x
incr x set rv64_arch x
incr x set rv32_arch x
incr x set msp430_arch x
incr x set c_arch x

zero x set no_output x
incr x set macho_executable x
incr x set macho_object x
incr x set elf_executable x
incr x set elf_object x
incr x set ti_txt_executable x
incr x set uf2_executable x
incr x set hex_array x
incr x set c_source x

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

set r5_addi_op1 	1100100
set r5_addi_op2		000

set r5_sw_op1 		1100010
set r5_sw_op2 		010

set r5_zr 0
set r5_ra 1

set rv_system_number 	10001
set rv_system_arg0 	0101
set rv_system_arg1 	1101
set rv_system_arg2 	0011

set x 1 set rv_system_exit x
incr x  set rv_system_read x
incr x  set rv_system_write x

del x

eoi


the core standard library for the optimizing assembler!
defines some basic constants and macros used by the 
rest of the language, and the various targets. 

  written by dwrr on 1202507185.024227


 