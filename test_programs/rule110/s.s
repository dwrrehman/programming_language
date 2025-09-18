file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/ascii.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s

str "run" set_output_name arm64

set x 11001
set array x incr x 
set timestep x incr x 
set pointer x incr x 
del x

eq 0 0 main



at local_arm64_printbinary
	ld ra 0
	set given_register c0 
	set c0 ra 
	function_begin


	at loop

		bc loop


	function_end	
	eq 0 0 ra del ra 
	lt 0 0 local_arm64_printbinary








at display_binary_array
	ld ra 0
	set c0 ra 
	function_begin

	addi pointer array 0 0 0 0

	set c0 hello set c1 hello.length writestring



	function_end	
	eq 0 0 ra del ra
	lt 0 0 display_binary_array




at main del main

allocate_16mb_stack_memory

addi array a6_sp 0 0 0 0


set execution_limit 0000_0000_1




mov timestep 0 shiftnone movzero
at loop

	display_binary_array

	addi timestep timestep 1 0 0 0 
	addi a6_zero timestep execution_limit 0 setflags subtract 
	bc is_not_equal loop del loop


set c0 0 exit

at hello
str "printing the rule 110 lifetime..."  emitnl
set c0 hello getstringlength
set hello.length c0

eoi



a program to print out the rule 110 lifetime in our language, using the arm64 backend!

written on 1202509173.020146 by dwrr






(ld r executable_stack_size
set c0 r ctprintbinary ctnl)
