 (testing out the generation of the 
risc-v uf2 file for programming the pico 2 W.
written 1202505272.173200 by dwrr)

file library/foundation.s ct
set ctsc_number compiler_target set ctsc_arg0 rv32_arch system
set ctsc_number compiler_format set ctsc_arg0 uf2_executable system	
set ctsc_number compiler_overwrite set ctsc_arg0 true system 
set ctsc_number compiler_stacksize set ctsc_arg0 0 system rt

at loop   
	(r5_i 1100100 0 0 0 0   )
	do loop del loop
halt

set i 0
at loop
	add i 1
	lt i 0000_1000_0100_1100_0010_1010_0110_1110 loop
halt


