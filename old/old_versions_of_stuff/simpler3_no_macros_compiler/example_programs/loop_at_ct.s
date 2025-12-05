(this is a test of using the language just as an interpreted language, 
testing out looping first! written on 1202507141.055047 by dwrr)

file library/foundation.s ct
st compiler_target no_arch nat
st compiler_format no_output nat

set i 0 at loop
	st compiler_arg0 compiler_system_debug nat
	st compiler_arg1 i nat sc
	add i 1 lt i 1001 loop del i del loop

st compiler_arg0 compiler_system_exit nat
st compiler_arg1 0 nat 
sc























(
1202507141.042743
	lets try to use mmap ctsc!

	lets write the impl in the compiler now to use statically known memory addresses now 
	instead of the memory[] array that we are using currently in cte!

)













