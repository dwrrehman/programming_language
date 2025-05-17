(1202505143.211927 dwrr 
a simple loop which gets user input, and prints out a stirng to the screen, in a loop.
written for the risc-v backend, to run in the website virtual machine.)


(library code:)

	( ------------- risc-v ABI system call registers --------- ) 
	rt system_call_number     10001
	rt system_call_argument_0 0101
	rt system_call_argument_1 1101
	rt system_call_argument_2 0011


	( ------------ system calls for the risc-v virtual achine ----------)
	set system_exit 1
	set system_read 01
	set system_write 11
	




rt loop 0	(these shouldnt be neccessary...)
rt x 0

set buffer stack_pointer



at loop

	set system_call_argument_0 
	set system_call_number system_write
	sc 

	do loop
	



































