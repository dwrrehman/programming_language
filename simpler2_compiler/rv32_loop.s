(running a loop in the rv32 virtual machine!
written on 1202505165.174040 by dwrr)

file library/foundation.s

compiler ctsc_set_target rv32_arch
compiler ctsc_set_format hex_array_txt_executable
compiler ctsc_overwrite true


register i 1
set i 0

at loop
	add i 1
	lt i 101 loop

set rv_sc_arg0 1001
set rv_sc_number rv_system_exit
system halt























(


set rv_sc_arg0 1
la rv_sc_arg1 string
constant l set l 0
compiler ctsc_get_length l 
set rv_sc_arg2 l del l 
set rv_sc_number rv_system_write
system

set rv_sc_arg0 0011
set rv_sc_number rv_system_exit
system halt

at string
string "hello, world!
"



)
