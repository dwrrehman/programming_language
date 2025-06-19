(testing out the actual macro machinery now!! lets see how it goes lol
1202506194.030902 dwrr)

file library/foundation.s ct

st compiler_target rv32_arch nat
st compiler_format hex_array_output nat
st compiler_should_overwrite true nat
st compiler_should_debug_cte true nat

do macros

operation add_numbers 11 at add_numbers ct
	ld ra compiler_ctsc_number nat
	ld x compiler_ctsc_arg0 nat
	ld y compiler_ctsc_arg1 nat
	ld c compiler_ctsc_arg2 nat rt
	ar x ar y add x y 
	ar x add x c
	ar ra ct do ra 
	del x del y del c del ra
	
at macros
del macros

set my_constant 1001

rt 
register a 1 set a 101
register b 01 set b 011

add_numbers a b my_constant

halt













































(set bubbles 1
deref hello
ref hello bubbles)

