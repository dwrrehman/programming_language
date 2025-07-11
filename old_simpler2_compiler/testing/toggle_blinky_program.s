(a shift register program 
written in our own language! 
written on 1202506032.232224 by dwrr)

file library/foundation.s ct
set ctsc_number compiler_target set ctsc_arg0 msp430_arch system
set ctsc_number compiler_format set ctsc_arg0 ti_txt_executable system	
set ctsc_number compiler_overwrite set ctsc_arg0 true system 
set ctsc_number compiler_stacksize set ctsc_arg0 0 system 

set reset_vector    	0111_1111_1111_1111
set start_of_flash 	0000_0000_0000_0011
set start_of_memory 	0000_0000_0100
set stack_size 000001
set start_of_stack start_of_memory 
add start_of_stack stack_size
set wdtctl 0000_0100_1000
set p1dir 0100_0100
set p1out 1000_0100
set p1in  0000_0100
set p1sel 0110_0100
set p2dir 0101_0100
set p2out 1001_0100
set p2in  0001_0100
set p2sel 0111_0100

rt m4_sect start_of_flash
	m4_op msp_mov  reg_mode sp 0   imm_mode imm_reg start_of_stack   size_word
	m4_op msp_mov  fixed_mode fixed_reg wdtctl   imm_mode imm_reg 0000_0001_0101_1010   size_word
ct 	set led1 bit6 set led3 bit5 set led2 bit7 set bits led1 or bits led2 or bits led3
rt	m4_op msp_mov  fixed_mode fixed_reg p2dir    imm_mode imm_reg bits   size_byte
	m4_op msp_mov  fixed_mode fixed_reg p2out    imm_mode imm_reg bits   size_byte
	m4_op msp_mov  fixed_mode fixed_reg p2sel    imm_mode imm_reg 0      size_byte
at loop
	m4_op msp_bis  fixed_mode fixed_reg p2out    imm_mode imm_reg led1   size_byte
	m4_op msp_bic  fixed_mode fixed_reg p2out    imm_mode imm_reg led2   size_byte	
	m4_op msp_bic  fixed_mode fixed_reg p2out    imm_mode imm_reg led3   size_byte	

	m4_op msp_mov  reg_mode r4 0   imm_mode imm_reg 1111_1111_1111_1110   size_word
at delay_loop
	m4_op msp_sub reg_mode r4 0  literal_mode constant_1 0 size_word
	m4_br condjnz delay_loop del delay_loop

	m4_op msp_bic  fixed_mode fixed_reg p2out    imm_mode imm_reg led1   size_byte
	m4_op msp_bis  fixed_mode fixed_reg p2out    imm_mode imm_reg led2   size_byte	
	m4_op msp_bic  fixed_mode fixed_reg p2out    imm_mode imm_reg led3   size_byte	

	m4_op msp_mov  reg_mode r4 0   imm_mode imm_reg 1111_1111_1111_1110   size_word
at delay_loop
	m4_op msp_sub reg_mode r4 0  literal_mode constant_1 0 size_word
	m4_br condjnz delay_loop del delay_loop

	m4_op msp_bic  fixed_mode fixed_reg p2out    imm_mode imm_reg led1   size_byte
	m4_op msp_bic  fixed_mode fixed_reg p2out    imm_mode imm_reg led2   size_byte	
	m4_op msp_bis  fixed_mode fixed_reg p2out    imm_mode imm_reg led3   size_byte

	m4_op msp_mov  reg_mode r4 0   imm_mode imm_reg 1111_1111_1111_1110   size_word
at delay_loop
	m4_op msp_sub reg_mode r4 0  literal_mode constant_1 0 size_word
	m4_br condjnz delay_loop del delay_loop

	m4_br condjmp loop
	m4_op msp_mov reg_mode r4 0  reg_mode r4 0  size_word
m4_sect reset_vector
	emit nat16 start_of_flash






























