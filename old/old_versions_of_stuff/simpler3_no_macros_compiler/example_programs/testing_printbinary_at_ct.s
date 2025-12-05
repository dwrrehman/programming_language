(this is a test of using the language just as an interpreted language, 
testing out the print binary at compiletime macros! obtained from "useful.s"
written on 1202507141.142557 by dwrr)

ct 

file library/foundation.s
file library/ascii.s
file library/useful.s

st compiler_target no_arch nat
st compiler_format no_output nat

do cthello

set i 0101
at loop
	set c0 i do ctbinary do ctnl
	sub i 1 ne i 0 loop

set c0 0 do ctexit








