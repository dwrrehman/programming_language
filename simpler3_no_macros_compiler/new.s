(trying out not using macro syntax in the language, 
but instead just using the compiletime execution semantics as is. 
lets see how it goes! written on 1202507023.185715 by dwrr)

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

do c_backend rt

set i 0
at loop
	la a0 mystring 
	set c0 011
	do print
	add i 1
	lt i 0101 loop
do exit

at mystring str "hello" 
emit 1 newline








