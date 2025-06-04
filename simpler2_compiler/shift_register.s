(a shift register program 
written in our own language! 
written on 1202506032.232224 by dwrr)

file library/foundation.s ct
set ctsc_number compiler_target set ctsc_arg0 msp430_arch system
set ctsc_number compiler_format set ctsc_arg0 ti_txt_executable system	
set ctsc_number compiler_overwrite set ctsc_arg0 true system 
set ctsc_number compiler_stacksize set ctsc_arg0 0 system 

set start_of_flash  00000110011
set reset_vector    11111011001





rt m4_sect 0000011001


	


rt m4_sect 0000011001

	emit 01 start_of_flash

