file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/ascii.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s

str "run" set_output_name 
arm64 

set stringtable_start 0000_0000_0000_01


eq 0 0 main



at copy_string_into_table
	ld ra 0
	set length c1 (N)
	set begin c0 (B)
	set c0 ra function_begin






current state as of 1202509173.023507

	XXXXXXX
	...read through the last N bytes in the executable output bytes, (using assembler_data) starting from B, 
	and store/write those to the stringtable_start ctmemory location.

	once that is done, delete the last N bytes from the executable output bytes (using assembler_count).
	del begin del length






	function_end
	eq 0 0 ra del ra
	lt copy_string_into_table




at displaystring
	ld ra 0
	set begin c0
	set c0 ra 
	function_begin



	set c0 begin getstringlength set stringlength c0

	set c0 begin set c1 stringlength writestring

	del begin del stringlength
	function_end	
	eq 0 0 ra del ra
	lt 0 0 dosomething





at main del main

at s str "hello there!" emitnl set c0 s del s displaystring 

set c0 0 exit

at hello
str "printing the rule 110 lifetime..."  emitnl
set c0 hello getstringlength
set hello.length c0

generate_strings





eoi



a program to print out the rule 110 lifetime in our language, using the arm64 backend!

written on 1202509173.020146 by dwrr












(ld r executable_stack_size
set c0 r ctprintbinary ctnl)






















allocate_16mb_stack_memory

addi array a6_sp 0 0 0 0


set execution_limit 0000_0000_1

mov timestep 0 shiftnone movzero
at loop

	(...)

	addi timestep timestep 1 0 0 0 
	addi a6_zero timestep execution_limit 0 setflags subtract 
	bc is_not_equal loop del loop







