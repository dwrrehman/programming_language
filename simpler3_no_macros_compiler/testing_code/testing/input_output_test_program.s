(loop around the hello world program, 
running in the risc-v virtual machine!
written on 1202505165.132734 by dwrr )

file library/foundation.s

ct 
set ctsc_number compiler_target 
set ctsc_arg0 rv32_arch 
system
set ctsc_number compiler_format 
set ctsc_arg0 hex_array_output
system	
set ctsc_number compiler_overwrite 
set ctsc_arg0 true
system 
rt

set iterator 0
at loop
	set rv_sc_arg0 stdin
	la rv_sc_arg1 inputarea	
	set rv_sc_arg2 00001
	set rv_sc_number rv_system_read
	system
	set input_length rv_sc_arg0

	set rv_sc_arg0 stdout
	la rv_sc_arg1 string
	ct set ctsc_number compiler_getlength 
	set ctsc_arg0 0 
	system
	rt set rv_sc_arg2 ctsc_arg0
	set rv_sc_number rv_system_write
	system

	set rv_sc_arg0 stdout
	la rv_sc_arg1 inputarea	
	set rv_sc_arg2 input_length
	set rv_sc_number rv_system_write
	system

	set rv_sc_arg0 stdout
	la rv_sc_arg1 newline
	set rv_sc_arg2 1
	set rv_sc_number rv_system_write
	system

	add iterator 1
	lt iterator 000001 loop

set rv_sc_arg0 0011
set rv_sc_number rv_system_exit
system halt

at string
	string "you said: "

at newline 
	emit 1 0101

at inputarea
	ct set boxi 0 
at boxl 
	rt emit 1 0 
	ct add boxi 1 
	lt boxi 00001 boxl




























