(loop around the hello world program, 
running in the risc-v virtual machine!
written on 1202505165.132734 by dwrr )

file library/foundation.s

compiler ctsc_set_target rv32_arch
compiler ctsc_set_format hex_array_txt_executable
compiler ctsc_overwrite true

set iterator 0
at loop
	set rv_sc_arg0 stdin
	la rv_sc_arg1 inputarea	
	set rv_sc_arg2 00001
	set rv_sc_number rv_system_read
	system

	set rv_sc_arg0 stdout
	la rv_sc_arg1 string
	ct l set l 0 compiler ctsc_get_length l
	set rv_sc_arg2 l del l
	set rv_sc_number rv_system_write
	system

	set input_length rv_sc_arg0

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
	ct boxi set boxi 0 
	ct boxl at boxl emit 1 0
	add boxi 1 lt boxi 00001 boxl 
	del boxl del boxi





























