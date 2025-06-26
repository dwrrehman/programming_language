( testing out ins sel 1202505235.123256 dwrr)

file library/foundation.s
file library/ascii.s

compiler ctsc_set_target rv32_arch
compiler ctsc_set_format hex_array_txt_executable
compiler ctsc_overwrite true

set rv_sc_arg0 stdout
la rv_sc_arg1 string
ct l set l 0 compiler ctsc_get_length l add l 1
set rv_sc_arg2 l del l
set rv_sc_number rv_system_write
system

set rv_sc_arg0 0011
set rv_sc_number rv_system_exit
system halt

at string
string "this is a test" 
emit 1 char_newline






















