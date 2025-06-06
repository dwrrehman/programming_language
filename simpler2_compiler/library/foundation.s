(
	the core standard library for the language: foundation.s
	written on 1202505294.221753 by dwrr.
)

ct 

(booleans)
set false 0
set true 1

(unix file descriptors)
set stdin  0
set stdout 1
set stderr 01

(compiletime system call interface : call numbers)
set x 0 set compiler_abort x
add x 1 set compiler_exit x
add x 1 set compiler_getchar x 
add x 1 set compiler_putchar x
add x 1 set compiler_printbin x
add x 1 set compiler_printdec x
add x 1 set compiler_setdebug x
add x 1 set compiler_print x
add x 1 set compiler_target x
add x 1 set compiler_format x
add x 1 set compiler_overwrite x
add x 1 set compiler_getlength x
add x 1 set compiler_gettarget x
add x 1 set compiler_getformat x
add x 1 set compiler_stacksize x
add x 1 set compiler_getstacksize x

(compiletime system call interface : argument registers)
register ctsc_number 0
register ctsc_arg0 1
register ctsc_arg1 01
register ctsc_arg2 11


(valid arguments to ctsc compiler_target)
set x 0 set no_arch x
add x 1 set arm64_arch x
add x 1 set arm32_arch x
add x 1 set rv64_arch x
add x 1 set rv32_arch x
add x 1 set msp430_arch x

(valid arguments to ctsc compiler_format)
set x 0 set debug_output_only x
add x 1 set macho_executable x
add x 1 set macho_object x
add x 1 set elf_executable x
add x 1 set elf_object x
add x 1 set ti_txt_executable x
add x 1 set uf2_executable x
add x 1 set hex_array_output x

(specific to the rv32 virtual machine running in my website)
set x 1 set rv_system_exit x
add x 1 set rv_system_read x
add x 1 set rv_system_write x
del x

rt
(rv32 system call abi)
register rv_sc_arg0 0101
register rv_sc_arg1 1101
register rv_sc_arg2 0011
register rv_sc_number 10001
ct



(--------------------- msp430 -------------------)

rt
(msp430 registers)
register pc 0
register sp 1
register sr 01
register cg 11
register r4 001
register r5 101
register r6 011
register r7 111
register r8 0001
register r9 1001
register r10 0101
register r11 1101
register r12 0011
register r13 1011
register r14 0111
register r15 1111
ct

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
set nat8 1
set nat16 01

(msp430 bit position constants)
set bit0 10000000
set bit1 01000000
set bit2 00100000
set bit3 00010000
set bit4 00001000
set bit5 00000100
set bit6 00000010
set bit7 00000001

(end of standard library code)







