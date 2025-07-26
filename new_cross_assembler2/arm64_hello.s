file library/core.s

st compiler_target arm64_arch
st compiler_format macho_executable
st compiler_should_overwrite true

set mov_type_zero 01

set zero_reg 11111
set link_reg 01111

set a6_system_number 00001
set a6_system_arg0 0
set a6_system_arg1 1
set a6_system_arg2 01
set a6_system_arg3 11

set a6_system_exit 1
set a6_system_fork 01
set a6_system_read 11
set a6_system_write 001

mov a6_system_number a6_system_write 0 mov_type_zero 1
mov a6_system_arg0 stdout 0 mov_type_zero 1
adr a6_system_arg1 string 0
st compiler_length string ld length compiler_length
mov a6_system_arg2 length 0 mov_type_zero 1
svc

mov a6_system_number a6_system_exit 0 mov_type_zero 1
mov a6_system_arg0 10001 0 mov_type_zero 1
svc

at string str "hello, world!
"

eoi

1202507222.003453
a simple hello world program for arm64



1202507222.012226 YAYYY got it working yayyyyy
that wasnt that bad at all lollll   this assembler makes things pretty easyyyy

now time for a prime number program!! that will be an interesting testtttt







