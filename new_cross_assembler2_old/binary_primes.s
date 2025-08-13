file library/core.s

st compiler_target arm64_arch
st compiler_format macho_executable
st compiler_should_overwrite true
st compiler_should_debug 0

eq 0 0 s str "all of this would be in the standard library..." at s del s

set mov_type_zero 01

set zero_reg 11111
set stack_pointer 11111
set link_reg 01111

set a6_system_number 00001
set a6_system_arg0 0
set a6_system_arg1 1
set a6_system_arg2 01
set a6_system_arg3 11

set a6_system_exit 1
set a6_system_fork 01
set a6_system_read 11
set a6_system_write 001




set cond_always 0111
set cond_never 1111

set is_equal 0
set is_not_equal 1
set is_negative 001
set is_nonnegative 101
set has_overflow_set 011
set has_overflow_clear 111
		
set is_signed_less 1101
set is_signed_greater 0011
set is_signed_less_or_equal 1011
set is_signed_greater_or_equal 0101
			
set is_unsigned_less 11
set is_unsigned_greater 0001
set is_unsigned_less_or_equal 1001
set is_unsigned_greater_or_equal 01







set c0 0
set c1 0
set c2 0

eq 0 0 skip_macros

at puts
	ld ra 0 lt 0 0 puts
	set string c0
	mov a6_system_number a6_system_write 0 mov_type_zero 1
	mov a6_system_arg0 stdout 0 mov_type_zero 1
	adr a6_system_arg1 string 0
	st compiler_length string ld length compiler_length
	mov a6_system_arg2 length 0 mov_type_zero 1 
	svc
	del length del string
	eq 0 0 ra del ra 

at exit
	ld ra 0 lt 0 0 exit
	mov a6_system_number a6_system_exit 0 mov_type_zero 1
	mov a6_system_arg0 0 0 mov_type_zero 1
	svc
	eq 0 0 ra del ra

at skip_macros del skip_macros

eq 0 0 s str "heres our actual program!" at s del s









eq 0 0 comment str "

this is a forwards loop!

set myconstant 0
set i 00011
mov i myconstant 0 mov_type_zero 1
at skip
	set c0 hello puts
	addi i i 1  0 1 0 1
	addi zero_reg i 101 true     1 1 1
	bc is_not_equal skip del skip







this is a backwards loop!


set c0 hello puts

set myconstant 1
set i 00011
mov i myconstant 01 mov_type_zero 1
at skip
	addi i i 1  0 1 1 1
	bc is_not_equal skip del skip


" at comment del comment









next: we need to figure out how to use the stack! 

	you first subtract from it

		sub sp 4

		then store to  sp   


	then deallocate, revert the stack:

		add sp 4












set c0 goodbye puts

exit

at hello str "hello world!
"

at goodbye str "terminating program! bye!!!
"






















eoi







1202507222.210255 a prime number program that 
prints the primes in little endian binary! 


































