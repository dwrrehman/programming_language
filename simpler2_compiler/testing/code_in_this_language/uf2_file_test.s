(testing out the generation of the uf2 file for programming the pico 2!
written 1202505272.173200 by dwrr)

file library/foundation.s

compiler ctsc_set_target rv32_arch
compiler ctsc_set_format uf2_executable
compiler ctsc_overwrite true

set i 0
at loop
add i 1
lt i 101 loop
halt

