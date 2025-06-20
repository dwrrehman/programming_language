(testing out the c backend of the compiler that i just finished implementing lol,  1202506194.204004 by dwrr)

file library/foundation.s ct

st compiler_target c_arch nat
st compiler_format c_source_output nat
st compiler_should_overwrite true nat
st compiler_should_debug_cte true nat

set c_system_debug 0
set c_system_exit 1

(...this would all be in the foundation file...)

rt register c_sc_call_number 0
register c_sc_arg0 1
register c_sc_arg1 01

ct do skip

operation print 1 at print ct
	ld ra compiler_ctsc_number nat
	ld data compiler_ctsc_arg0 nat rt 
	set c_sc_call_number c_system_debug
	dr data set c_sc_arg0 data 
	system
	ct do ra del ra del data

operation printi 1 at printi ct
	ld ra compiler_ctsc_number nat
	ld data compiler_ctsc_arg0 nat rt 
	set c_sc_call_number c_system_debug
	set c_sc_arg0 data 
	system
	ct do ra del ra del data

operation exit 1 at exit ct
	ld ra compiler_ctsc_number nat
	ld data compiler_ctsc_arg0 nat rt 
	set c_sc_call_number c_system_exit
	set c_sc_arg0 data 
	system halt
	ct do ra del ra del data


at skip del skip

(...all until here...)
rt

(set sum 0 
set i 0
at loop
	add sum i
	add i 1
	print sum
	lt i 0101 loop del loop


printi 0
)



ct set total 0000_0000_1 rt

set i 0 
set count 0
at loop
	set j 0
at inner
	ge j i prime
	set r i rem r j eq r 0 composite
	add j 1
	do inner
at prime
	print i
	add count 1
at composite
	add i 1
	lt i total loop 
del loop

print count

exit 0011




















