(1202505143.211927 dwrr 
a simple loop which gets user input, and prints out a stirng to the screen, in a loop.
written for the risc-v backend, to run in the website virtual machine.)


(this would be in the standard library) 


lf library/ctsc.s

(hardware registers:)

	set hr 1 
	si hr 111_111
	set x0 hr or x0 0  
	set zr x0
	set x1 hr or x1 1
	set x2 hr or x2 01
	set x3 hr or x3 11
	set x4 hr or x4 001 
	set x5 hr or x5 101 
	set x10 hr or x10 0101
	set x11 hr or x11 1101
	set x12 hr or x12 0011
	set x17 hr or x17 10001

(risc-v ABI system call registers) 

	rt zero_register 	x0
	rt link_register 	x1
	rt stack_pointer 	x2

	rt rv_sc_call     	x17
	rt rv_sc_arg0 		x10
	rt rv_sc_arg1 		x11
	rt rv_sc_arg2 		x12



(system calls implemented in the risc-v virtual machine)

	set rv_system_exit 1
	set rv_system_read 01
	set rv_system_write 11
	


rt string 0

(rt buffer 0 set buffer stack_pointer)

rt loop 0 at loop

set ctsc_arg0 0   (string #0!!! in the string list.)
rt ctsc_call ctsc_strlen
set length ctsc_arg0   (ctsc_arg0 will have the populated string length!)

la 	rv_sc_arg0 	string
set 	rv_sc_arg1 	length
set 	rv_sc_call 	rv_system_write
sc

set 	rv_sc_call 	rv_system_exit
sc

at string 
sl "hello there"








	





