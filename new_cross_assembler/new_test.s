file library/core.s
file library/ascii.s

st compiler_target rv32_arch
st compiler_format hex_array
st compiler_should_overwrite true
st compiler_should_debug true

set c0 c0
set c1 c1
set c2 c2
set c3 c3
set c4 c4

eq 0 0 macroskip

at exit
	ld ra 0 lt 0 0 exit
	ri rv_imm rv_add rv_system_number 0 rv_system_exit
	ri rv_imm rv_add rv_system_arg0 0 0
	ri rv_ecall 0 0 0 0
	eq 0 0 ra del ra 

at write
	ld ra 0 lt 0 0 write
	ri rv_imm rv_add rv_system_number 0 rv_system_write
	ri rv_imm rv_add rv_system_arg0 0 stdout
	ru rv_auipc rv_system_arg1 c0
	ri rv_imm rv_add rv_system_arg1 rv_system_arg1 c0
	ri rv_imm rv_add rv_system_arg2 0 c1
	ri rv_ecall 0 0 0 0
	eq 0 0 ra del ra 

at macroskip del macroskip
st compiler_length string ld length compiler_length
set c0 string
set c1 length
write exit
at string str

"hello! this is my cool string!
i'm trying out this cool thing
where i try to write a really 
large string lol. lets see how it
goes! :) i also have to get the 
length of this string lol. 
that will be fun probably.

wish me luck on that! lol.


oh, and also i'm going to try to 
fix the other stuff in the assembler!
	i feel like there are seriously 
	some things we can improve lol

but yeah anyways, thats my whole string :)
hope this goes well...
"
eoi


1202507211.085008 by dwrr
a test using the new parser system, 
which unified ct and rt labels and ct values, 
allowing us to distinguish between rt labels and ct values via just the value itself.

















