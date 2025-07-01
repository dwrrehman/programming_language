(a shell like interface for my website!
written on 1202506301.222553 by dwrr)

file library/foundation.s ct

st compiler_target rv32_arch nat
st compiler_format hex_array_output nat
st compiler_should_overwrite true nat

ct do skip

operation write_string 01 at write_string ct
	ld ra compiler_ctsc_number nat
	ld label compiler_ctsc_arg0 nat
	ld index compiler_ctsc_arg1 nat
	rt set rv_sc_number rv_system_write
	set rv_sc_arg0 stdout
	dr label la rv_sc_arg1 label del label
	ct st compiler_ctsc_number compiler_getlength nat 
	st compiler_ctsc_arg0 index nat del index
	system ld length compiler_ctsc_arg0 nat
	rt set rv_sc_arg2 length del length
	system ct do ra del ra

operation exit 1 at exit ct
	ld ra compiler_ctsc_number nat
	ld code compiler_ctsc_number nat
	rt set rv_sc_number rv_system_exit
	set rv_sc_arg0 code
	system halt ct do ra del ra


operation read 11 at read ct
	ld ra compiler_ctsc_number nat
	ld count compiler_ctsc_arg0 nat
	ld label compiler_ctsc_arg1 nat
	ld size compiler_ctsc_arg2 nat
	rt
	set rv_sc_number rv_system_read
	set rv_sc_arg0 stdin
	dr label la rv_sc_arg1 label
	set rv_sc_arg2 size
	system
	dr count set count rv_sc_arg0
	ct do ra 
	del ra del count del label del size 	


operation write 01 at write ct
	ld ra compiler_ctsc_number nat
	ld label compiler_ctsc_arg0 nat
	ld size compiler_ctsc_arg1 nat
	rt set rv_sc_number rv_system_write
	set rv_sc_arg0 stdout
	dr label la rv_sc_arg1 label
	dr size set rv_sc_arg2 size
	system ct do ra 
	del ra del label del size














at skip del skip


set newline 0101

rt 

set s s  (these sohuldnt be required... we need to implement obs args in the macro system.. lol)
set t t
set n n 
set input input

ct set input_size 00001 rt

set i 0 
at loop
	write_string s 0
	read n input input_size
	write_string t 1
	la p input add p n st p newline 1 add n 1	
	write input n

	(set rv_sc_number rv_system_write
	set rv_sc_arg0 1
	la rv_sc_arg1 input
	set rv_sc_arg2 n
	system)


	add i 1 
	lt i 00011 loop del loop del i

exit 011

at s string "say something: "
at t string "nice, you said: "
at input ct set i 0 at l rt emit 1 0 ct 
add i 1 lt i input_size l del l del i




















