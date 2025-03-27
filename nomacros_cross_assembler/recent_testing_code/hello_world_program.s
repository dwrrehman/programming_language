. first working hello world program, written on 1202503097.051605 dwrr .
. (strings arent implemented yet, so i just emitted the characters individually lollll) .

lf foundation.s

set _targetarchitecture arm64_arch
set _outputformat macho_executable
set _shouldoverwrite true

df begin
df length
df begin2
df length2

df i set i 10
mov i 5 shift_none type_zero width64

df loop at loop
	mov syscallarg0 stdout shift_none type_zero width64
	adr syscallarg1 begin type_byteoffset
	mov syscallarg2 length shift_none type_zero width64
	mov syscallnumber system_write shift_none type_zero width64
	svc	

	addi i i 1 shift_none setflags subtract width64
	bc is_nonzero loop udf loop


mov syscallarg0 stdout shift_none type_zero width64
adr syscallarg1 begin2 type_byteoffset
mov syscallarg2 length2 shift_none type_zero width64
mov syscallnumber system_write shift_none type_zero width64
svc



mov syscallnumber system_exit shift_none type_zero width64
mov syscallarg0 42 shift_none type_zero width64
svc




at begin 

	string "hello there from space! 
this is my cool string lol." 

emit 1 newline
df end at end set length end udf end 
sub length begin



at begin2

	string "...hello there! this is my second string lol 
this is even cooler! lol." 

emit 1 newline
df end at end set length2 end udf end
sub length2 begin2
























eoi 


llo there from space! 
this is my cool string lol.


...hello there! this is my second string lol 
this is even cooler! lol.


















