. first working hello world program with string literals .
.     written on 1202503167.193448 by dwrr     .

lf foundation.s

set _targetarchitecture arm64_arch
set _outputformat macho_executable
set _shouldoverwrite true

df begin df length
mov syscallarg0 stdout shift_none type_zero width64
adr syscallarg1 begin type_byteoffset
mov syscallarg2 length shift_none type_zero width64
mov syscallnumber system_write shift_none type_zero width64
svc	

mov syscallnumber system_exit shift_none type_zero width64
mov syscallarg0 42 shift_none type_zero width64
svc halt

at begin 
	string "hello there from space! :) this is my cool string lol."  emit 1 newline 

df end at end set length end udf end sub length begin

eoi
