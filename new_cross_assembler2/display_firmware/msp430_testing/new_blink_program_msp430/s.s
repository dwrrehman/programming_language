(
	a simple blink program for the msp430fr5994 
	microcontroller, that will be used in the display!

	written on 1202509232.000625 by dwrr
)


file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/ascii.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s

(this is all just stdlib code)

zero x set pc x
incr x set sp x
incr x set sr x
incr x set cg x

set m_mov 	001
set m_add 	101
set m_addc 	011
set m_sub 	111
set m_subc 	0001
set m_cmp 	1001
set m_dadd 	0101
set m_bit 	1101
set m_bic 	0011
set m_bis 	1011
set m_xor 	0111
set m_and 	1111

zero x set condjnz x
incr x set condjz x
incr x set condjnc x
incr x set condjc x
incr x set condjn x
incr x set condjge x
incr x set condjl x
incr x set condjmp x

set size_byte 1
set size_word 0

zero x set reg_mode x
incr x set index_mode x
incr x set deref_mode x
incr x set incr_mode x

set imm_mode incr_mode
set imm_reg pc
set literal_mode index_mode
set constant_1 cg
set fixed_reg sr
set fixed_mode index_mode

set bit0 10000000
set bit1 01000000
set bit2 00100000
set bit3 00010000
set bit4 00001000
set bit5 00000100
set bit6 00000010
set bit7 00000001

del x





eq 0 0 skipmacros

at reti
	ld ra 0
	set c0 ra function_begin

	emit 01   0000_0000_1100_1000       (todo: make this built in to the assembler...)

	function_end 
	eq 0 0 ra del ra
	lt 0 0 reti 

at m_nop
	ld ra 0
	set c0 ra function_begin

	mo m_mov reg_mode 001 0  reg_mode 001 0  size_word

	function_end 
	eq 0 0 ra del ra
	lt 0 0 m_nop

at skipmacros del skipmacros


(
	constants for the msp430g2553 specifically:
	1202509022.220354
)


set start_of_fram 		0000_0000_0000_0010

set reset_vector    		0111_1111_1111_1111

set start_of_sram 		0000_0000_0011_1000
set end_of_sram 		1111_1111_1101_1100

set start_of_stack end_of_sram
sub start_of_stack 0000_001

set pm5ctl0     0000_1100_1000_0000
set wdtctl 	0011_1010_1000_0000
set port1_base  0000_0000_0100_0000

set p1in 	0000   add p1in port1_base
set p1out 	0100   add p1out port1_base
set p1dir 	0010   add p1dir port1_base

set csctl0      0000_0110_1000_0000
set csctl1      0100_0110_1000_0000
set csctl2      0010_0110_1000_0000
set csctl3      0110_0110_1000_0000
set csctl4      0001_0110_1000_0000
set csctl5      0101_0110_1000_0000
set csctl6      0011_0110_1000_0000

(user code starts here)

str "firmware.txt" set_output_name

set target msp430_arch
st output_format ti_txt_executable
st overwrite_output true

sect start_of_sram

	mo m_mov  reg_mode sp 0   imm_mode imm_reg start_of_stack   size_word
	mo m_mov  fixed_mode fixed_reg wdtctl   imm_mode imm_reg 1111_1011_0101_1010   size_word

	mo m_mov  fixed_mode fixed_reg p1dir    imm_mode imm_reg 1111_1111 size_byte
	mo m_mov  fixed_mode fixed_reg p1out    imm_mode imm_reg 0000_0000 size_byte

	mo m_bic  fixed_mode fixed_reg pm5ctl0  literal_mode constant_1 0 size_byte



	at mainloop 
		mo m_bis  fixed_mode fixed_reg p1out    imm_mode imm_reg 1000_0000 size_byte

		mo m_mov reg_mode 001 0  imm_mode imm_reg 1 size_word
	at outter
		mo m_mov reg_mode 101 0  imm_mode imm_reg 1111_1111_1111_1111  size_word
		at inner
			mo m_sub reg_mode 101 0 literal_mode constant_1 0  size_word
			mb condjnz inner del inner
		mo m_sub reg_mode 001 0 literal_mode constant_1 0 size_word
		mb condjnz outter del outter
		
		mo m_bic  fixed_mode fixed_reg p1out    imm_mode imm_reg 1000_0000 size_byte

		mo m_mov reg_mode 001 0  imm_mode imm_reg 1 size_word
	at outter
		mo m_mov reg_mode 101 0  imm_mode imm_reg 1111_1111_1111_1111  size_word
		at inner
			mo m_sub reg_mode 101 0 literal_mode constant_1 0  size_word
			mb condjnz inner del inner
		mo m_sub reg_mode 001 0 literal_mode constant_1 0 size_word
		mb condjnz outter del outter
		
		mb condjmp mainloop
		del mainloop
	m_nop

sect reset_vector
	emit 01 start_of_sram


eoi

updated this program to work with the msp430fr5994, on 1202509232.011804

just a simple blink program, to test to see if things are workinggg

so far they arent lol 








just a simple blink program to test if everythings working! 

we are going to copy the program into RAM on boot, and then run from SRAM first

1202509022.221445


























(set p1_interrupt_vector    	0111_1011_1111_1111)
(set wdt_interrupt_vector 	0100_1111_1111_1111)



(

set ie1		0
set ie2		1
set ifg1	01
set ifg2	11


set p1sel 	0110_0100
set p1sel2 	1000_0010
set p1ren 	1110_0100
set p1ie  	1010_0100
set p1ies 	0010_0100
set p1ifg 	1100_0100

set p2dir 	0101_0100
set p2in  	0001_0100
set p2out 	1001_0100
set p2sel 	0111_0100
set p2ren 	1111_0100

set ucb0ctl0	0001_0110
set ucb0ctl1	1001_0110

set ucb0br0	0101_0110
set ucb0br1	1101_0110
set ucb0stat	1011_0110
set ucb0rxbuf	0111_0110
set ucb0txbuf	1111_0110

)



(
eq 0 0 skiproutines


at delay
	ld ra 0
	set a c0
	set b c1
	set c0 ra function_begin

	mo m_mov reg_mode a 0  imm_mode imm_reg 1 size_word
	at outter
		mo m_mov reg_mode b 0  imm_mode imm_reg 1111_1111  size_word
		at inner
			mo m_sub reg_mode b 0 literal_mode constant_1 0  size_word
			mb condjnz inner del inner
		mo m_sub reg_mode a 0 literal_mode constant_1 0 size_word
		mb condjnz outter del outter

	del a del b 
	function_end
	eq 0 0 ra del ra
	lt 0 0 delay


at skiproutines del skiproutines
)














(
	(use dco at 16mhz)	
	(mo m_mov  fixed_mode fixed_reg csctl0   imm_mode imm_reg 0000_0000_1010_0101   size_word)
	(mo m_mov  fixed_mode fixed_reg csctl1   imm_mode imm_reg 0000_0000_1010_0101   size_word)

	




	(use dco at 16mhz)
	(mo m_mov  fixed_mode fixed_reg dcoctl   imm_mode imm_reg 0000_0111   size_byte
	mo m_mov  fixed_mode fixed_reg bcsctl1  imm_mode imm_reg 1111_0001   size_byte
	mo m_mov  fixed_mode fixed_reg bcsctl2  imm_mode imm_reg 0000_0000   size_byte
	mo m_mov  fixed_mode fixed_reg bcsctl3  imm_mode imm_reg 0010_0100   size_byte)




	(mo m_mov  fixed_mode fixed_reg p2dir    imm_mode imm_reg 1111_1111 size_byte
	mo m_mov  fixed_mode fixed_reg p2out    imm_mode imm_reg 0000_0000 size_byte
	mo m_mov  fixed_mode fixed_reg p2sel    imm_mode imm_reg 0 size_byte)

	(mo m_mov  fixed_mode fixed_reg p1sel    imm_mode imm_reg 0000_0101 size_byte
	mo m_mov  fixed_mode fixed_reg p1sel2   imm_mode imm_reg 0000_0101 size_byte)

	(mo m_mov  fixed_mode fixed_reg p1ren    imm_mode imm_reg 1000_0000 size_byte)



	(mo m_bis  fixed_mode fixed_reg p1ie     literal_mode constant_1 0 size_byte
	mo m_bis  fixed_mode fixed_reg p1ies    literal_mode constant_1 0 size_byte)

	(mo m_mov  fixed_mode fixed_reg ucb0br0  imm_mode imm_reg 01 size_byte
	mo m_mov  fixed_mode fixed_reg ucb0ctl0 imm_mode imm_reg 1001_0001 size_byte
	mo m_mov  fixed_mode fixed_reg ucb0ctl1 imm_mode imm_reg 0_00000_11 size_byte

	mo m_bic  fixed_mode fixed_reg p1out    imm_mode imm_reg 0000_1000 size_byte
	mo m_mov  fixed_mode fixed_reg ucb0txbuf   imm_mode imm_reg 0100_0000 size_byte
	at wait  
		mo m_bit  fixed_mode fixed_reg ucb0stat   literal_mode constant_1 0  size_byte
		mb condjnz wait 
		del wait
	mo m_mov  fixed_mode fixed_reg ucb0txbuf   imm_mode imm_reg 0000_0000 size_byte
	at wait  
		mo m_bit  fixed_mode fixed_reg ucb0stat   literal_mode constant_1 0  size_byte
		mb condjnz wait
		del wait

	mo m_mov  fixed_mode fixed_reg ucb0txbuf   imm_mode imm_reg 0000_0000 size_byte
	at wait  
		mo m_bit  fixed_mode fixed_reg ucb0stat   literal_mode constant_1 0  size_byte
		mb condjnz wait 
		del wait
	mo m_mov  fixed_mode fixed_reg ucb0txbuf   imm_mode imm_reg 0000_0000 size_byte
	at wait  
		mo m_bit  fixed_mode fixed_reg ucb0stat   literal_mode constant_1 0  size_byte
		mb condjnz wait 
		del wait
	mo m_bis  fixed_mode fixed_reg p1out    imm_mode imm_reg 0000_1000 size_byte	
	mo m_bis  fixed_mode fixed_reg ucb0ctl1 imm_mode imm_reg 1 size_byte

	set data 001
	mo m_mov  reg_mode data 0    imm_mode imm_reg 0 size_byte
	
	mo m_mov  fixed_mode fixed_reg wdtctl   imm_mode imm_reg 0011_1000__0101_1010 size_word
	mo m_bis  fixed_mode fixed_reg ie1     literal_mode constant_1 0 size_byte
	mo m_bic  fixed_mode fixed_reg ifg1    literal_mode constant_1 0 size_byte

	mo m_bis  reg_mode sr 0  imm_mode imm_reg 0001_1011_0000_0000  size_word   (...sleep in lpm3...)
	m_nop)





(at wdt_interrupt_routine
	
	mo m_xor  reg_mode data 0    imm_mode imm_reg 1111_1111 size_byte

	mo m_bic  fixed_mode fixed_reg ucb0ctl1 imm_mode imm_reg 1 size_byte
	mo m_bic  fixed_mode fixed_reg p1out    imm_mode imm_reg 0000_1000 size_byte
	mo m_mov  fixed_mode fixed_reg ucb0txbuf   imm_mode imm_reg 0100_0000 size_byte
	at wait 
		mo m_bit  fixed_mode fixed_reg ucb0stat   literal_mode constant_1 0 size_byte
		mb condjnz wait 
		del wait
	mo m_mov  fixed_mode fixed_reg ucb0txbuf   imm_mode imm_reg 0001_0100 size_byte
	at wait  
		mo m_bit  fixed_mode fixed_reg ucb0stat   literal_mode constant_1 0  size_byte
		mb condjnz wait 
		del wait
	mo m_mov  fixed_mode fixed_reg ucb0txbuf   reg_mode data 0 size_byte
	at wait  
		mo m_bit  fixed_mode fixed_reg ucb0stat   literal_mode constant_1 0 size_byte
		mb condjnz wait 
		del wait
	mo m_mov  fixed_mode fixed_reg ucb0txbuf   reg_mode data 0 size_byte
	at wait 
		mo m_bit  fixed_mode fixed_reg ucb0stat   literal_mode constant_1 0  size_byte
		mb condjnz wait 
		del wait
	mo m_bis  fixed_mode fixed_reg p1out    imm_mode imm_reg 0000_1000 size_byte
	mo m_bis  fixed_mode fixed_reg ucb0ctl1 imm_mode imm_reg 1 size_byte
	mo m_bic  fixed_mode fixed_reg ifg1     literal_mode constant_1 0 size_byte
	reti m_nop


at p1_interrupt_routine
	mo m_xor  fixed_mode fixed_reg p2out    imm_mode imm_reg 01 size_byte
	mo m_bic  fixed_mode fixed_reg p1ifg    imm_mode imm_reg 1 size_byte
	reti m_nop


sect p1_interrupt_vector
	set address start_of_flash
	add address p1_interrupt_routine
	emit 01 address del address


sect wdt_interrupt_vector
	set address start_of_flash 
	add address wdt_interrupt_routine
	emit 01 address del address

)








	(mo m_xor  fixed_mode fixed_reg p2out    literal_mode constant_1 0 size_byte)






(1202509055.034045 VLO: useful!!!)


(	(use vlo/8 for clock sources)
	mo m_mov  fixed_mode fixed_reg dcoctl   imm_mode imm_reg 0000_0111   size_byte
	mo m_mov  fixed_mode fixed_reg bcsctl1  imm_mode imm_reg 0000_1101   size_byte
	mo m_mov  fixed_mode fixed_reg bcsctl2  imm_mode imm_reg 0111_1111   size_byte
	mo m_mov  fixed_mode fixed_reg bcsctl3  imm_mode imm_reg 0010_0100   size_byte

)













	(set a 001 set b 101
	mo m_bis  fixed_mode fixed_reg p2out    imm_mode imm_reg 11 size_byte
	set c0 a set c1 b delay
	mo m_bic  fixed_mode fixed_reg p2out    imm_mode imm_reg 11 size_byte
	set c0 a set c1 b delay)









		(
		set a 001 set b 101
		mo m_bis  fixed_mode fixed_reg p2out    imm_mode imm_reg 1 size_byte
		set c0 a set c1 b delay
		mo m_bic  fixed_mode fixed_reg p2out    imm_mode imm_reg 1 size_byte
		set c0 a set c1 b delay
		)




































































	mo msp_mov reg_mode 101 0  imm_mode imm_reg 1111_1111_1111_1111  size_word
	at loop2

		(mo msp_mov reg_mode 001 0  imm_mode imm_reg 1111_1111_1111_1111  size_word
		at loop
			mo msp_sub reg_mode 001 0  literal_mode constant_1 0   size_word	
			mb condjnz loop del loop)

		mo msp_sub reg_mode 101 0  literal_mode constant_1 0   size_word
		mb condjnz loop2 

		del loop2




















old reference code:




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





