(
	core standard library for the language: foundation.s
	written on 1202505165.132635 by dwrr.
)

(rv32 system call abi)

register rv_sc_arg0 0101
register rv_sc_arg1 1101
register rv_sc_arg2 0011
register rv_sc_number 10001



(specific to the rv32 virtual machine running in my website: )

constant rv_system_exit   set rv_system_exit   1
constant rv_system_read   set rv_system_read   01
constant rv_system_write  set rv_system_write  11 



(compiler interface)

constant ctsc_abort 		set ctsc_abort 0
constant ctsc_exit 		set ctsc_exit 1
constant ctsc_getchar 		set ctsc_getchar 01
constant ctsc_putchar 		set ctsc_putchar 11
constant ctsc_printbin 		set ctsc_printbin 001
constant ctsc_printdec 		set ctsc_printdec 101
constant ctsc_set_debug 	set ctsc_set_debug 011
constant ctsc_print 		set ctsc_print 111
constant ctsc_set_target 	set ctsc_set_target 0001
constant ctsc_set_format 	set ctsc_set_format 1001
constant ctsc_overwrite 	set ctsc_overwrite 0101
constant ctsc_get_length 	set ctsc_get_length 1101

constant no_arch 	set no_arch 0
constant arm64_arch 	set arm64_arch 1
constant arm32_arch 	set arm32_arch 01
constant rv64_arch 	set rv64_arch 11
constant rv32_arch 	set rv32_arch 001
constant msp430_arch 	set msp430_arch 101

constant debug_output_only 	set debug_output_only 0
constant macho_executable 	set macho_executable 1
constant macho_object 		set macho_object 01
constant elf_executable 	set elf_executable 11
constant elf_object 		set elf_object 001
constant ti_txt_executable 	set ti_txt_executable 101
constant hex_array_txt_executable  set hex_array_txt_executable 011

constant true set true 1
constant false set false 0



(end of standard library code) 




