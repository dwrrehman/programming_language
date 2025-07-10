(writing the hello world program on arm64!
just using the language isa, no machine instructions!
written on 1202506253.014448 by dwrr)

file library/foundation.s ct

st compiler_target arm64_arch nat
st compiler_format macho_executable nat
st compiler_should_overwrite true nat


(this code would be in the standard library,)

set arm_system_exit 1
set arm_system_fork 01
set arm_system_read 11
set arm_system_write 001

reg arm_system_arg0 0
reg arm_system_arg1 1
reg arm_system_arg2 01
reg arm_system_number 00001

(...all the way until here.)



rt 
set arm_system_number arm_system_write
set arm_system_arg0 stdout
set arm_system_arg1 s
set arm_system_arg2 011
sc 

set arm_system_number arm_system_exit
set arm_system_arg0 0011
sc halt

at s str "hello
"
