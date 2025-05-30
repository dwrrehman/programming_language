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

(compiletime system call interface)
set x 0 set compiler_abort x
add x 1 set compiler_exit x
add x 1 set compiler_getchar x 
add x 1 set compiler_putchar x
add x 1 set compiler_printbin x
add x 1 set compiler_printdec x
add x 1 set compiler_set_debug x
add x 1 set compiler_print x
add x 1 set compiler_target x
add x 1 set compiler_format x
add x 1 set compiler_overwrite x
add x 1 set compiler_get_length x
add x 1 set compiler_get_target x

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
set x 0 set rv_system_exit x
add x 1 set rv_system_read x
add x 1 set rv_system_write x

del x
rt

(rv32 system call abi)
register rv_sc_arg0 0101
register rv_sc_arg1 1101
register rv_sc_arg2 0011
register rv_sc_number 10001

(end of standard library code)





