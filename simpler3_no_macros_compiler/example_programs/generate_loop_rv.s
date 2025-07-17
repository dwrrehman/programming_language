(testing out the label generation using the del machinery in CTE stage 1
1202507137.032023 by dwrr
)

file library/foundation.s ct 


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


at macro    ( c0 tripcount       a0 string   c1 length    do macro ) 

	ld ra compiler_return_address nat

	set tripcount c0
	set length c1

	rt set i 0
	at loop
		set c0 length do print
		add i 1
		lt i tripcount loop
	del loop del i 
	del length
	del tripcount

	ct do ra del ra
	
at skip del skip

do c_backend rt 

set c0 11
set a0 hello
set c1 011
do macro

set c0 101
set a0 other
set c1 011
do macro

set c0 01
set a0 hello
set c1 011
do macro


do exit

at hello str "hello
"

at other str "other
"





































