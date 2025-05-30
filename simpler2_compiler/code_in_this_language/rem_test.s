(testing the remainder instruction 
specifically in riscv code gen.
written on 1202505235.021613 by dwrr)

file library/foundation.s

compiler ctsc_set_target rv32_arch
compiler ctsc_set_format hex_array_txt_executable
compiler ctsc_overwrite true

set rv_sc_arg0 1111
set rv_sc_arg1 001
rem rv_sc_arg0 rv_sc_arg1
set rv_sc_number rv_system_exit
system halt



