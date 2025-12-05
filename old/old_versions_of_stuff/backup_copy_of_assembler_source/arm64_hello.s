(a hello world program for arm64)
(written on 1202507303.130646 by dwrr)

file library/core.s
file library/useful.s

st target arm64_arch
st format macho_executable
st overwrite true



(---------------stdlib code for arm64-----------------)

set mov_type_zero 01

set zero_reg 11111
set link_reg 01111

set a6_number 00001
set a6_arg0 0
set a6_arg1 1
set a6_arg2 01
set a6_arg3 11

set a6_exit 1
set a6_fork 01
set a6_read 11
set a6_write 001

(--------------------------------)



mov a6_number a6_write 0 mov_type_zero 1
mov a6_arg0 stdout 0 mov_type_zero 1
adr a6_arg1 string 0
mov a6_arg2 string.length 0 mov_type_zero 1
svc

mov a6_number a6_exit 0 mov_type_zero 1
mov a6_arg0 10001 0 mov_type_zero 1
svc

at string str 
"how are you today? :)
i'm doing pretty well lol. i think this language 
is actually really nice to use honestly lol. 
its just.. really lovely to write most of the time lol
...even in arm64!
yayyy
" 
set c0 string getstringlength
set string.length c0




eoi









1202507222.003453
a simple hello world program for arm64



1202507222.012226 YAYYY got it working yayyyyy
that wasnt that bad at all lollll   this assembler makes things pretty easyyyy

now time for a prime number program!! that will be an interesting testtttt



1202507303.125609
rewritten to use the new syntax and stdlib for the language!





