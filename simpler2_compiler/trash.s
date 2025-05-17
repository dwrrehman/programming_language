


(




( ---------------------------- foundation.s file stdlib code --------------------)

(macos arm64 sc abi)
register system_call_arg0 0
register system_call_arg1 1
register system_call_arg2 01 
register system_call_number 00001
constant system_write  set system_write  001
constant system_exit   set system_exit   1

(compiler interface)

constant ctsc_abort set ctsc_abort 0
constant ctsc_exit set ctsc_exit 1
constant ctsc_getchar set ctsc_getchar 01
constant ctsc_putchar set ctsc_putchar 11
constant ctsc_printhex set ctsc_printhex 001
constant ctsc_printdec set ctsc_printdec 101
constant ctsc_set_debug set ctsc_set_debug 011
constant ctsc_print set ctsc_print 111
constant ctsc_set_target set ctsc_length 0001
constant ctsc_set_outputformat set ctsc_set_outputformat 1001
constant ctsc_should_overwrite set ctsc_should_overwrite 0101
constant ctsc_get_length set ctsc_get_length 1101

constant no_arch set no_arch 0
constant arm64_arch set arm64_arch 1
constant arm32_arch set arm32_arch 01
constant rv64_arch set rv64_arch 11
constant rv32_arch set rv32_arch 001
constant msp430_arch set msp430_arch 101

constant debug_output_only set debug_output_only 0
constant macho_executable set macho_executable 1
constant macho_object set macho_object 01
constant elf_executable set elf_executable 11
constant elf_object set elf_object 001
constant ti_txt_executable set ti_txt_executable 101
constant hex_txt_executable set hex_txt_executable 011

constant true set true 1
constant false set false 0

(------------------------------- end of stdlib code ------------------------------)
constant rv32_arch 	set rv32_arch 001
constant msp430_arch 	set msp430_arch 101

constant debug_output_only set debug_output_only 0
constant macho_executable set macho_executable 1
constant macho_object set macho_object 01
constant elf_executable set elf_executable 11
constant elf_object set elf_object 001
constant ti_txt_executable set ti_txt_executable 101
constant hex_txt_executable set hex_txt_executable 011

constant true set true 1
constant false set false 0

(------------------------------- end of stdlib code ------------------------------)


)

