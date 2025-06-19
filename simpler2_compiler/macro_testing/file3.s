(testing out the actual macro machinery now!! lets see how it goes lol
1202506194.030902 dwrr)

file library/foundation.s
ct
st compiler_target rv32_arch nat
st compiler_format hex_array_output nat
st compiler_should_overwrite true nat
st compiler_should_debug_cte true nat

file macro_testing/some_macros.s

rt 

register a 1 	set a 101
register b 01 	set b 011
register u 11 	set u 1101
register v 001 	set v 1011

ct set after after
ref return after st compiler_ctsc_number return nat del return
ref local0 a  st compiler_ctsc_arg0   local0 nat del local0
ref local1 b  st compiler_ctsc_arg1   local1 nat del local1
do my_macro at after del after

ct set after2 after2
ref return after2 st compiler_ctsc_number return nat del return
ref local0 u  st compiler_ctsc_arg0   local0 nat del local0
ref local1 v  st compiler_ctsc_arg1   local1 nat del local1
do my_macro at after2 del after2





halt













































(set bubbles 1
deref hello
ref hello bubbles)

