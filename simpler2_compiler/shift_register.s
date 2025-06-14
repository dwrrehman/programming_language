(a shift register program 
written in our own language! 
written on 1202506032.232224 by dwrr)

( 
shift register interface: 
---------------------------
	
	1.0 : output enable  (1 = enable)
	1.1 : serial clock   (falling edge, 1->0 triggers data latch read)
	1.2 : serial data  (no interrupt capability)

86 / 512 bytes consumed so far...
)

file library/foundation.s ct
set ctsc_number compiler_target set ctsc_arg0 msp430_arch system
set ctsc_number compiler_format set ctsc_arg0 ti_txt_executable system	
set ctsc_number compiler_overwrite set ctsc_arg0 true system 
set ctsc_number compiler_stacksize set ctsc_arg0 0 system 

set reset_vector    		0111_1111_1111_1111
set p1_interrupt_vector    	0010_0111_1111_1111

set start_of_flash 		0000_0000_0000_0011
set flash_interrupt_vector 	0000_0000_0000_0111

set start_of_memory 	0000_0000_0100
set end_of_memory 	1111_1111_1100
set start_of_stack end_of_memory 
sub start_of_stack 0000_001

set wdtctl 0000_0100_1000
set p1dir 0100_0100
set p1out 1000_0100
set p1in  0000_0100
set p1sel 0110_0100

set p1ie  1010_0100
set p1ies 0010_0100
set p1ifg 1100_0100

set p2dir 0101_0100
set p2out 1001_0100
set p2in  0001_0100
set p2sel 0111_0100

rt m4_sect start_of_flash

	m4_op msp_mov  reg_mode sp 0   imm_mode imm_reg start_of_stack   size_word
	m4_op msp_mov  fixed_mode fixed_reg wdtctl   imm_mode imm_reg 0000_0001_0101_1010   size_word

	m4_op msp_mov  fixed_mode fixed_reg p2dir    imm_mode imm_reg 1111_1111   size_byte
	m4_op msp_mov  fixed_mode fixed_reg p2out    imm_mode imm_reg 0 size_byte
	m4_op msp_mov  fixed_mode fixed_reg p2sel    imm_mode imm_reg 0 size_byte

	m4_op msp_mov  fixed_mode fixed_reg p1dir    imm_mode imm_reg 0001_1111 size_byte
	m4_op msp_mov  fixed_mode fixed_reg p1out    imm_mode imm_reg 0 size_byte
	m4_op msp_mov  fixed_mode fixed_reg p1sel    imm_mode imm_reg 0 size_byte

	m4_op msp_bis  fixed_mode fixed_reg p1ie     imm_mode imm_reg 11 size_byte
	m4_op msp_bis  fixed_mode fixed_reg p1ies    imm_mode imm_reg 11 size_byte
	
at loop
	m4_op msp_bis reg_mode sr 0  imm_mode imm_reg 0001_1111_0000_0000  size_word   (go to sleep in LPM4)
	m4_br condjmp loop
	m4_op msp_mov reg_mode r4 0  reg_mode r4 0  size_word


m4_sect flash_interrupt_vector

	m4_op msp_xor  fixed_mode fixed_reg p2out    imm_mode imm_reg 1111_1111   size_byte
	m4_op msp_xor  fixed_mode fixed_reg p1out    imm_mode imm_reg 0001_1111   size_byte

	m4_op msp_bic  fixed_mode fixed_reg p1ifg    imm_mode imm_reg 11 size_byte
	
	emit nat16 0000_0000_1100_1000   (the "reti" instruction, 0x1300)
	m4_op msp_mov reg_mode r4 0  reg_mode r4 0  size_word


m4_sect p1_interrupt_vector
	emit nat16 flash_interrupt_vector

m4_sect reset_vector
	emit nat16 start_of_flash













(	m4_op msp_mov  reg_mode r6 0   imm_mode imm_reg 1   size_word

at inner

	m4_op msp_xor  fixed_mode fixed_reg p2out    imm_mode imm_reg 1111_1111   size_byte
	m4_op msp_xor  fixed_mode fixed_reg p1out    imm_mode imm_reg 0001_1111   size_byte


	m4_op msp_mov  reg_mode r4 0   imm_mode imm_reg 1111_1111_1111_1111   size_word
	at delay_loop m4_op msp_sub reg_mode r4 0  literal_mode constant_1 0 size_word
	m4_br condjnz delay_loop del delay_loop


	m4_op msp_xor  fixed_mode fixed_reg p2out    imm_mode imm_reg 1111_1111   size_byte
	m4_op msp_xor  fixed_mode fixed_reg p1out    imm_mode imm_reg 0001_1111   size_byte


	m4_op msp_mov  reg_mode r4 0   imm_mode imm_reg 1111_1111_1111_1111   size_word
	at delay_loop m4_op msp_sub reg_mode r4 0  literal_mode constant_1 0 size_word
	m4_br condjnz delay_loop del delay_loop


	m4_op msp_sub reg_mode r6 0  literal_mode constant_1 0 size_word
	m4_br condjnz inner del inner
)




	(register disable_output 0   (required because of a bug in the compiler...)

	m4_op msp_bit fixed_mode fixed_reg p1in imm_mode imm_reg bit0 size_byte
	m4_br condjz disable_output

		(m4_op msp_bis  fixed_mode fixed_reg p2out    imm_mode imm_reg 1111_1111   size_byte
		m4_op msp_bis  fixed_mode fixed_reg p1out    imm_mode imm_reg 0001_1111   size_byte)

		m4_op msp_bic  fixed_mode fixed_reg p2out    imm_mode imm_reg bit7   size_byte
		m4_op msp_bis  fixed_mode fixed_reg p2out    imm_mode imm_reg bit6   size_byte
		m4_op msp_bic  fixed_mode fixed_reg p1out    imm_mode imm_reg bit7   size_byte
		m4_op msp_bis  fixed_mode fixed_reg p1out    imm_mode imm_reg bit6   size_byte

		m4_br condjmp loop

	at disable_output	

		(m4_op msp_bic  fixed_mode fixed_reg p2out    imm_mode imm_reg 1111_1111   size_byte
		m4_op msp_bic  fixed_mode fixed_reg p1out    imm_mode imm_reg 0001_1111   size_byte)

		m4_op msp_bis  fixed_mode fixed_reg p2out    imm_mode imm_reg bit7   size_byte
		m4_op msp_bic  fixed_mode fixed_reg p2out    imm_mode imm_reg bit6   size_byte
		m4_op msp_bis  fixed_mode fixed_reg p1out    imm_mode imm_reg bit7   size_byte
		m4_op msp_bic  fixed_mode fixed_reg p1out    imm_mode imm_reg bit6   size_byte


		m4_br condjmp loop

	)
















(
	m4_op msp_mov  reg_mode r6 0   imm_mode imm_reg 11   size_word

at inner
	m4_op msp_bis  fixed_mode fixed_reg p2out    imm_mode imm_reg bit7   size_byte
	m4_op msp_bic  fixed_mode fixed_reg p2out    imm_mode imm_reg bit6   size_byte
	m4_op msp_bis  fixed_mode fixed_reg p1out    imm_mode imm_reg bit7   size_byte
	m4_op msp_bic  fixed_mode fixed_reg p1out    imm_mode imm_reg bit6   size_byte

	m4_op msp_mov  reg_mode r4 0   imm_mode imm_reg 1111_1111_1111_1111   size_word
at delay_loop
	m4_op msp_sub reg_mode r4 0  literal_mode constant_1 0 size_word
	m4_br condjnz delay_loop del delay_loop

	m4_op msp_bic  fixed_mode fixed_reg p2out    imm_mode imm_reg bit7   size_byte
	m4_op msp_bis  fixed_mode fixed_reg p2out    imm_mode imm_reg bit6   size_byte
	m4_op msp_bic  fixed_mode fixed_reg p1out    imm_mode imm_reg bit7   size_byte
	m4_op msp_bis  fixed_mode fixed_reg p1out    imm_mode imm_reg bit6   size_byte

	m4_op msp_mov  reg_mode r4 0   imm_mode imm_reg 1111_1111_1111_1111   size_word
at delay_loop
	m4_op msp_sub reg_mode r4 0  literal_mode constant_1 0 size_word
	m4_br condjnz delay_loop del delay_loop

	m4_op msp_sub reg_mode r6 0  literal_mode constant_1 0 size_word
	m4_br condjnz inner del inner
)


































