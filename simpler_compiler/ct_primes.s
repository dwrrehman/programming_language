(rewriting the prime number program in this version of the language! 
written on 1202505106.220939 dwrr)

lf library/ctsc.s

set i 0 
at loop
	ge i 101 done
	set ctsc_arg0 i
	rt ctsc_callnumber ctsc_print
	add i 1 do loop

at done	 ud i ud loop ud done
rt ctsc_callnumber ctsc_read




set limit 0000_0000_0000_00001

set i 0
at loop
	set j 01
at inner
	ge j i prime
	set r i rem r j eq r 0 composite
	add j 1 do inner

at prime 
	set ctsc_arg0 i
	rt ctsc_callnumber ctsc_print
at composite
	add i 1

	lt i limit loop	

rt ctsc_callnumber ctsc_exit


















