( testing out ins sel 1202505235.123256 dwrr)

file library/foundation.s
file library/ascii.s

compiler ctsc_set_target rv32_arch
compiler ctsc_set_format hex_array_txt_executable
compiler ctsc_overwrite true

at loop
set rv_sc_arg2 rv_sc_arg0
add rv_sc_arg2 rv_sc_arg1

lt rv_sc_arg2 0 loop

set rv_sc_arg0 0011
set rv_sc_number rv_system_exit
system halt






















