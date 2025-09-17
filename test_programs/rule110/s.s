file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s

str "run" set_output_name arm64
st executable_stack_size 0000_0000_0000_0000___0000_0000_1

set x 11001
set array x incr x 
set timestep x incr x 
set pointer x incr x 
del x

eq 0 0 main

at display_binary_array
	ld ra 0
	set c0 ra 
	function_begin

	addi pointer array 0 0 0 0

	set c0 hello set c1 hello.length writestring

at loop

	memi .... ERROR HEREEE    XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX



	bc is_equal loop del loop

	function_end	
	eq 0 0 ra del ra
	lt 0 0 display_binary_array

at main del main

addi a6_sp a6_sp 0000_0000_0001 1 0 1
addi a6_sp a6_sp 0000_0000_0001 1 0 1

addi array a6_sp 0 0 0 0




at loop

display_binary_array

bc is_equal loop del loop


set c0 0 exit

at hello
str "printing the rule 110 lifetime..." 
emit 1 newline
set c0 hello getstringlength
set hello.length c0

eoi

