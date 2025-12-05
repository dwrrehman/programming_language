(writing the hello world program on arm64!
just using the language isa, no machine instructions!
written on 1202506253.014448 by dwrr)

file library/foundation.s
ct
st compiler_target arm64_arch nat
st compiler_format macho_executable nat
st compiler_should_overwrite true nat


set system_exit 1
set system_fork 01
set system_read 11
set system_write 001
rt

register sc_arg0 0
register sc_arg1 1
register sc_arg2 01
register sc_number 00001


set sc_arg0 0011
set sc_number system_exit
system halt


at s string "hello world"
