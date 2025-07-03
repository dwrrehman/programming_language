(computing the number of primes less than a given number
 at runtime, using the c backend! also testing out the macro system further.
written on 1202507023.214455 by dwrr)

file library/foundation.s

(...this would all be in the standard library...)
rt 
	set a0 a0  
	set a1 a1	
ct 
	set c0 c0 
	set c1 c1 
do skip

at c_backend
	ld ra compiler_return_address nat
	st compiler_target c_arch nat
	st compiler_format c_source nat
	st compiler_should_overwrite true nat
	do ra del ra

at exit
	ld ra compiler_return_address nat
	rt set c_system_number c_system_exit
	set c_system_arg0 a0
	sc halt ct do ra del ra

at skip del skip
set newline 0101
(...until here...)


(my code starts here!)
do c_backend 
set limit 0101
rt set i 0 set count 0
at loop
	set j 01
at inner 
	ge j i prime
	set r i rem r j 
	eq r 0 composite
	add j 1 do inner
at prime
	add count 1
at composite
	add i 1 lt i limit loop

set a0 count 
do exit 





















