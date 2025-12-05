(testing out printing binary numbers using the c backend! 
written on 1202507034.203838 by dwrr)

file library/foundation.s

(...this would all be in the standard library...)

rt 	set a0 a0  
	set a1 a1	
ct 	set c0 c0 
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
	set c_system_arg0 0
	sc halt ct do ra del ra

at print
	ld ra compiler_return_address nat
	rt set c_system_number c_system_write
	set c_system_arg0 stdout
	set c_system_arg1 a0
	set c_system_arg2 c0
	sc ct do ra del ra

at skip del skip

set newline 0101

(...until here...)







(my code starts here!)

do skip

at print0 ct 
	ld ra compiler_return_address nat
	rt set a0 digitzero
	set c0 1
	do print
	ct do ra del ra

at print1 ct
	ld ra compiler_return_address nat
	rt set a0 digitone
	set c0 1
	do print
	ct do ra del ra

at print_newline ct
	ld ra compiler_return_address nat
	rt set a0 newline_char
	set c0 1
	do print
	ct do ra del ra

at printbinary
	ld ra compiler_return_address nat	
	rt set data a0 
	at loopb set bit data and bit 1
	eq bit 0 else do print1 do done 
	at else do print0 at done
	sd data 1 ne data 0 loopb
	do print_newline
	ct do ra del ra 
	del loopb
	del bit del data 
	del done del else 
	del print0 
	del print1 
	del print_newline


at footer
	ld ra compiler_return_address nat
	rt at digitzero str "0"
	at digitone str "1"
	at newline_char emit 1 newline
	ct do ra del ra



at skip del skip














do c_backend rt

(set i 0
at loop
	set a0 i do printbinary
	add i 1
	lt i 0101 loop 
del loop del i
)


ct set count 0000_0000_0000_0000__01 rt
set i 0
at loop set j 01
at inner ge j i prime
set r i rem r j eq r 0 composite
add j 1 do inner
at prime set a0 i do printbinary
at composite add i 1 lt i count loop
del i del loop del count
do exit do footer























(at s str "hello" 
emit 1 newline)




