(
	core standard library for the language: foundation.s
	written on 1202505165.132635 by dwrr.
)

(rv32 system call abi)

register rv_sc_arg0 0101
register rv_sc_arg1 1101
register rv_sc_arg2 0011
register rv_sc_number 10001


(unix i/o file descriptors)
ct stdin   set stdin   0
ct stdout  set stdout  1
ct stderr  set stderr  01


(specific to the rv32 virtual machine running in my website: )

ct rv_system_exit   set rv_system_exit   1
ct rv_system_read   set rv_system_read   01
ct rv_system_write  set rv_system_write  11 



(compiler interface)

ct ctsc_abort 		set ctsc_abort 0
ct ctsc_exit 		set ctsc_exit 1
ct ctsc_getchar 		set ctsc_getchar 01
ct ctsc_putchar 		set ctsc_putchar 11
ct ctsc_printbin 		set ctsc_printbin 001
ct ctsc_printdec 		set ctsc_printdec 101
ct ctsc_set_debug 	set ctsc_set_debug 011
ct ctsc_print 		set ctsc_print 111
ct ctsc_set_target 	set ctsc_set_target 0001
ct ctsc_set_format 	set ctsc_set_format 1001
ct ctsc_overwrite 	set ctsc_overwrite 0101
ct ctsc_get_length 	set ctsc_get_length 1101

ct no_arch 	set no_arch 0
ct arm64_arch 	set arm64_arch 1
ct arm32_arch 	set arm32_arch 01
ct rv64_arch 	set rv64_arch 11
ct rv32_arch 	set rv32_arch 001
ct msp430_arch 	set msp430_arch 101

ct debug_output_only 	set debug_output_only 0

ct macho_executable 	set macho_executable 1
ct macho_object 	set macho_object 01

ct elf_executable 	set elf_executable 11
ct elf_object 		set elf_object 001

ct ti_txt_executable 	set ti_txt_executable 101
ct uf2_executable 	set uf2_executable 011

ct hex_array_txt_executable  set hex_array_txt_executable 111

ct true set true 1
ct false set false 0


(end of standard library code) 

























