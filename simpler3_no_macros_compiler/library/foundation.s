(
	the core standard library for the language: foundation.s
	written on 1202505294.221753 by dwrr.
)

ct 

(numbers)
set -1 0 sub -1 1

(booleans)
set false 0
set true 1

(unix file descriptors)
set stdin  0
set stdout 1
set stderr 01

(unsigned integer sizes)
set byte 	1
set nat16 	01
set nat32 	001
set nat 	0001



(memory mapped ctsc address)
set x 0000 set compiler_return_address x
add x 0001 set compiler_target x
add x 0001 set compiler_format x 

add x 0001 set compiler_should_overwrite x
add x 0001 set compiler_should_debug x

add x 0001 set compiler_stack_size x
add x 0001 set compiler_get_length x
add x 0001 set compiler_is_compiletime x

add x 0001 set compiler_arg0 x
add x 0001 set compiler_arg1 x
add x 0001 set compiler_arg2 x
add x 0001 set compiler_arg3 x
add x 0001 set compiler_arg4 x
add x 0001 set compiler_arg5 x
add x 0001 set compiler_arg6 x
add x 0001 set compiler_arg7 x

add x 0001 set compiler_base x



(compiletime system call interface : call numbers)
set x 0 set compiler_system_debug x
add x 1 set compiler_system_exit x
add x 1 set compiler_system_read x 
add x 1 set compiler_system_write x
add x 1 set compiler_system_open x
add x 1 set compiler_system_close x

(valid arguments to ctsc compiler_target)
set x 0 set no_arch x
add x 1 set arm64_arch x
add x 1 set arm32_arch x
add x 1 set rv64_arch x
add x 1 set rv32_arch x
add x 1 set msp430_arch x
add x 1 set c_arch x

(valid arguments to ctsc compiler_format)
set x 0 set no_output x
add x 1 set macho_executable x
add x 1 set macho_object x
add x 1 set elf_executable x
add x 1 set elf_object x
add x 1 set ti_txt_executable x
add x 1 set uf2_executable x
add x 1 set hex_array x
add x 1 set c_source x





(---------------- c backend -------------------)



(system calls suppported by the c backend) 

set x 0 set c_system_debug x
add x 1 set c_system_exit x
add x 1 set c_system_read x
add x 1 set c_system_write x
add x 1 set c_system_open x
add x 1 set c_system_close x
add x 1 set c_system_mmap x
add x 1 set c_system_munmap x


(constants for the mmap system call interface: )

set prot_read 1
set prot_write 01
set map_private 01
set map_anonymous 0000_0000_0000_1
set map_failed -1


rt 
register c_system_number 0
register c_system_arg0 1
register c_system_arg1 01
register c_system_arg2 11
register c_system_arg3 001
register c_system_arg4 101
register c_system_arg5 011 
register c_system_arg6 111 
ct
















(--------------------- msp430 -------------------)

((msp430 registers)
register pc_reg 0
register sp_reg 1
register sr_reg 01
register cg_reg 11
register r4_reg 001
register r5_reg 101
register r6_reg 011
register r7_reg 111
register r8_reg 0001
register r9_reg 1001
register r10_reg 0101
register r11_reg 1101
register r12_reg 0011
register r13_reg 1011
register r14_reg 0111
register r15_reg 1111
)

(msp430 register index constants)
set pc 0
set sp 1
set sr 01
set cg 11
set r4 001
set r5 101
set r6 011
set r7 111
set r8 0001
set r9 1001
set r10 0101
set r11 1101
set r12 0011
set r13 1011
set r14 0111
set r15 1111

(m4_op: op codes)
set msp_mov 001
set msp_add 101
set msp_addc 011
set msp_sub 111
set msp_subc 0001
set msp_cmp 1001
set msp_dadd 0101
set msp_bit 1101
set msp_bic 0011
set msp_bis 1011
set msp_xor 0111
set msp_and 1111

(m4_br: branch conditions)
set condjnz 0
set condjz 1
set condjnc 01
set condjc 11
set condjn 001
set condjge 101
set condjl 011
set condjmp 111

(m4_op: size parameter)
set size_byte 1
set size_word 0

(m4_op: addressing modes)
set reg_mode 0
set index_mode 1
set deref_mode 01
set incr_mode 11

(specific addressing modes)
set imm_mode incr_mode
set imm_reg pc
set literal_mode index_mode
set constant_1 cg
set fixed_reg sr
set fixed_mode index_mode

(msp430 bit position constants)
set bit0 10000000
set bit1 01000000
set bit2 00100000
set bit3 00010000
set bit4 00001000
set bit5 00000100
set bit6 00000010
set bit7 00000001









( ---------------- risc-v -----------------)

(risc-v op codes)
set r5_addi_op1 	1100100
set r5_addi_op2		000
set r5_sw_op1 		1100010
set r5_sw_op2 		010

(risc-v registers)
set r5_zr 0
set r5_ra 1


(rv32 system call abi)
rt register rv_sc_arg0 0101
register rv_sc_arg1 1101
register rv_sc_arg2 0011
register rv_sc_number 10001 ct


(specific to the rv32 virtual machine running in my website)
set x 1 set rv_system_exit x
add x 1 set rv_system_read x
add x 1 set rv_system_write x






del x


(end of standard library code)














