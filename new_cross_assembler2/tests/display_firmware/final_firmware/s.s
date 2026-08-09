(written on 1202510094.031158 by dwrr
 firmware file for the led display!)

file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/ascii.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/rp2350.s

str "firmware.uf2" set_output_name

set 1_second 1111

eq 0 0 skiproutines
at delay
	ld ra 0
	set iterator c0 
	set constant c1
	set c0 ra
	function_begin
	
	set i iterator
	set c0 i set c1 constant li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i

	function_end
	eq 0 0 ra del ra
	lt 0 0 delay

at skiproutines del skiproutines

rp2350

(sect sram_start)
sect flash_start

start_rp2350_binary

set next 	01
set data 	next incr next
set address 	next incr next

set c0 data set c1 1111_1100_1001_1111___1101_1111_1111_1111 li
set c0 address set c1 reset_base li
rs r_store r_sw address data 0

set c0 address set c1 clocks_base li
ri r_imm r_add data 0 0000_0000_0001
rs r_store r_sw address data clock_peri_control

set c0 address set c1 io_bank0_base li
set c0 data set c1 101 li
rs r_store r_sw address data io_gpio0_ctrl
rs r_store r_sw address data io_gpio1_ctrl
rs r_store r_sw address data io_gpio2_ctrl
rs r_store r_sw address data io_gpio3_ctrl
rs r_store r_sw address data io_gpio4_ctrl
rs r_store r_sw address data io_gpio5_ctrl

(rs r_store r_sw address data io_gpio14_ctrl)

rs r_store r_sw address data io_gpio15_ctrl
rs r_store r_sw address data io_gpio25_ctrl

set c0 data set c1 1 li
rs r_store r_sw address data io_gpio16_ctrl
rs r_store r_sw address data io_gpio18_ctrl
rs r_store r_sw address data io_gpio19_ctrl

set c0 address set c1 pads_bank0_base li

rs r_store r_sw address 0 pads_gpio0
rs r_store r_sw address 0 pads_gpio1
rs r_store r_sw address 0 pads_gpio2
rs r_store r_sw address 0 pads_gpio3
rs r_store r_sw address 0 pads_gpio4
rs r_store r_sw address 0 pads_gpio5


rs r_store r_sw address 0 pads_gpio16
rs r_store r_sw address 0 pads_gpio18
rs r_store r_sw address 0 pads_gpio19
rs r_store r_sw address 0 pads_gpio25

set c0 data set c1 0100_0010_0 li
rs r_store r_sw address data pads_gpio15

(set c0 data set c1 0000_1100_0 li
rs r_store r_sw address data pads_gpio14)

set sio 	next incr next
set c0 sio set c1 sio_base li

set c0 data set c1 1111_1100_0000_0000___0000_0000_01 li
rs r_store r_sw sio data sio_gpio_oe
set c0 data set c1 1111_1100_0000_0000 li
rs r_store r_sw sio data sio_gpio_out
(set c0 next set c1 1_second delay)
set c0 data set c1 1111_1000_0000_0000___0000_0000_01 li
rs r_store r_sw sio data sio_gpio_out
(set c0 next set c1 1_second delay)
set c0 data set c1 1111_1100_0000_0000 li
rs r_store r_sw sio data sio_gpio_out
(set c0 next set c1 1_second delay)

set temp	next incr next
set other_temp  next incr next
set copy	next incr next
set led_state   next incr next
set ram   	next incr next

set c0 ram set c1 sram_start li

set c0 address set c1 powman_base li
ri r_load r_lw data address last_swcore_pwrup
ri r_imm r_add temp 0 1

rb r_branch r_bne data temp skip_boot

	ri r_imm r_add led_state 0 0
	rs r_store r_sw ram led_state 0
	set c0 address set c1 sio_base li

	set i   next incr next 

	set c0 i set c1 0001 li	
	at loop	

		set c0 address set c1 sio_base li
		set c0 data set c1 1111_1100_0000_0000___0000_0000_01 li
		rs r_store r_sw address data sio_gpio_out
	
		set c0 next set c1 1111_1111_1111_1111_1111_1 delay
	
		set c0 address set c1 sio_base li
		set c0 data set c1 1111_1100_0000_0000___0000_0000_00 li
		rs r_store r_sw address data sio_gpio_out
	
		set c0 next set c1 1111_1111_1111_1111_1111_1 delay
		
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 loop 
		del loop del i sub next 1

at skip_boot del skip_boot



(set sio_gpio_out_xor  0001_0100

set c0 address set c1 sio_base li

set c0 data set c1 0000_0000_0000_0010 li
rs r_store r_sw address data sio_gpio_out_xor

set c0 next set c1 1111_1111_1111_1111_1111_1 delay
	
set c0 data set c1 0000_0000_0000_0010 li
rs r_store r_sw address data sio_gpio_out_xor

set c0 next set c1 1111_1111_1111_1111_1111_1 delay
)



set c0 address set c1 sio_base li

ri r_load r_lw led_state ram 0
ri r_imm r_eor led_state led_state 1
ri r_imm r_and led_state led_state 1
rs r_store r_sw ram led_state 0

set c0 data set c1 0111_1100_0000_0000 li
rr r_reg r_or led_state led_state data 0
rs r_store r_sw address led_state sio_gpio_out



set c0 address set c1 powman_base li

set n 1000_0110_1000_0000 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data powman_timer

set n 1111_1111_1111_111 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data alarm_time_15to0

set n 1110_1110_1000_0000 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data powman_timer



set n 1111_00___01_11 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data powman_pwrup0

set n 1111_00___11_11 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data powman_pwrup0


set c0 address set c1 powman_base li
set n 0000_1010_0000_0101 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data vreg_control
set n 0000_1011_0000_0000 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data powman_state

processor_sleep



eoi



( nothing is executed below here... )

set spi 	next incr next
set sio 	next incr next

set c0 spi set c1 spi0_base li
set c0 sio set c1 sio_base li

set c0 data set c1 1111_0000_0 li
rs r_store r_sw spi data spi_control0
set c0 data set c1 01 li
rs r_store r_sw spi data spi_prescale
set c0 data set c1 01 li
rs r_store r_sw spi data spi_control1

set c0 data set c1 1111_1100_0000_0000___0000_0000_01 li
rs r_store r_sw sio data sio_gpio_oe
set c0 data set c1 1111_11 li
rs r_store r_sw sio data sio_gpio_out
set c0 next set c1 1_second delay
set c0 data set c1 1111_1000_0000_0000___0000_0000_01 li
rs r_store r_sw sio data sio_gpio_out
set c0 next set c1 1_second delay
set c0 data set c1 1111_11 li
rs r_store r_sw sio data sio_gpio_out
set c0 next set c1 1_second delay

set c0 data set c1 0000_01 li
rs r_store r_sw sio data sio_gpio_out
set c0 data set c1 0000_0000_0111_0010 li
rs r_store r_sw spi data spi_data
rs r_store r_sw spi 0 spi_data
at wait
	ri r_load r_lw data spi spi_status
	ri r_imm r_and data data 0000_1
	rb r_branch r_bne data 0 wait del wait
set c0 data set c1 1111_11 li
rs r_store r_sw sio data sio_gpio_out

set linkregister  1
set param0 	next incr next
set param1 	next incr next
set param2 	next incr next

set chip0_columns chip0_columns
set chip1_columns chip1_columns
set chip2_columns chip2_columns
set chip3_columns chip3_columns
set chip4_columns chip4_columns
set chip5_columns chip5_columns

at displayloop

	set c0 param0 set c1 chip4_columns la
	set c0 param1 set c1 chip5_columns la
	set c0 param2 set c1 001 li
	rj r_jal linkregister chipfunction

	set c0 param0 set c1 chip3_columns la
 	set c0 param1 set c1 chip4_columns la
	set c0 param2 set c1 11 li
	rj r_jal linkregister chipfunction

	set c0 param0 set c1 chip2_columns la
	set c0 param1 set c1 chip3_columns la
	set c0 param2 set c1 01 li
	rj r_jal linkregister chipfunction

	set c0 param0 set c1 chip1_columns la
	set c0 param1 set c1 chip2_columns la
	set c0 param2 set c1 1 li
	rj r_jal linkregister chipfunction
	
	set c0 param0 set c1 chip0_columns la
	set c0 param1 set c1 chip1_columns la
	set c0 param2 set c1 0 li
	rj r_jal linkregister chipfunction

	rj r_jal 0 displayloop


at chip0_columns
	emit 1 01   (0)
	emit 1 11   (1)
	emit 1 1   (2)
	emit 1 0   (3)

	emit 1 001   (4)
	emit 1 011   (5)
	emit 1 111   (6)
	emit 1 101   (7)

	emit 1 0101   (8)
	emit 1 1001   (9)
	emit 1 1011   (10)
	emit 1 0001   (11)

	emit 1 1101   (12)
	emit 1 0000_1

at chip1_columns
	emit 1 0111   (0)
	emit 1 1111   (1)

	emit 1 01   (2)
	emit 1 11   (3)
	emit 1 0101   (4)
	emit 1 0001  (5)

	emit 1 001  (6)
	emit 1 1001   (7)
	emit 1 0000_1


at chip2_columns
	emit 1 1111  (0)

	emit 1 11    (1)
	emit 1 0     (2)
	emit 1 011   (3)
	emit 1 101   (4)

	emit 1 0001  (5)
	emit 1 0101  (6)
	emit 1 0011  (7)
	emit 1 0000_1


at chip3_columns
	emit 1 0111
	emit 1 01
	emit 1 11
	emit 1 001

	emit 1 011
	emit 1 0101
	emit 1 0001
	emit 1 1001

	emit 1 0000_1



at chip4_columns
	emit 1 0011
	emit 1 0111
	emit 1 01

	emit 1 1
	emit 1 101
	emit 1 011
	emit 1 0001

	emit 1 1001
	emit 1 0000_1


at chip5_columns
	emit 1 0
	emit 1 0

at chip_row_masks
	emit 01   0000_0000_0000_0000
	emit 01   0100_0101_0001_0100    (first bit changed!)

	emit 01   0110_1001_0101_0110
	emit 01   1100_0101_0001_1100     (last bit changed!)

	emit 01   1001_1001_0010_0101        (bit 11 changed to a 0, was a 1)
	emit 01   0


(all ones:)
(at chip_row_masks
	emit 01   0000_0000_0000_0000
	emit 01   0000_0000_0000_0000

	emit 01   0000_0000_0000_0000
	emit 01   0000_0000_0000_0000

	emit 01   0000_0000_0000_0000
	emit 01   0
)


(all zeros

at chip_row_masks
	emit 01   0000_0000_0000_0000
	emit 01   1100_0101_0001_0100 

	emit 01   0110_1001_0101_0110
	emit 01   1100_0101_0001_1101

	emit 01   1001_1001_0011_0101
	emit 01   0
)

set pindata 	next incr next
set chipselect 	next incr next
set columnpin 	next incr next
set chip 	next incr next
set linkregister2  next incr next

at latch_transfer
	rs r_store r_sw sio chipselect sio_gpio_out
	set c0 data set c1 0010_1000_0111_0010 li
	rs r_store r_sw spi data spi_data
	rs r_store r_sw spi pindata spi_data
	at wait
		ri r_load r_lw data spi spi_status
		ri r_imm r_and data data 0000_1
		rb r_branch r_bne data 0 wait del wait
	set c0 data set c1 1111_11 li
	rs r_store r_sw sio data sio_gpio_out
	ri r_jalr_op1 r_jalr_op2 0 linkregister2 0

at chipfunction	
	at columnloop
		ri r_load r_lbu columnpin param0 0
		ri r_imm r_add chip 0 1
		rr r_reg r_si columnpin chip columnpin 0
		set c0 chip set c1 1111_1111_1111_1111 li
		rr r_reg r_and columnpin columnpin chip 0
		ri r_imm r_add chip 0 001
		at chiploop
			set c0 pindata set c1 chip_row_masks la
			rr r_reg r_add pindata pindata chip 0
			rr r_reg r_add pindata pindata chip 0
			ri r_load r_lhu pindata pindata 0

			rb r_branch r_bne chip param2 skip
				rr r_reg r_or pindata pindata columnpin 0
			at skip del skip

			ri r_imm r_add chipselect 0 1
			rr r_reg r_si chipselect chipselect chip 0
			ri r_imm r_eor chipselect chipselect 1111_11
			rj r_jal linkregister2 latch_transfer
			ri r_imm r_add chip chip 1111_1111_1111
			rb r_branch r_bne chip 0 chiploop

		rb r_branch r_bne param2 0 skip
			ri r_imm r_add pindata columnpin 0
			ri r_imm r_add chipselect 0 0111_11
			rj r_jal linkregister2 latch_transfer
		at skip del skip

		rb r_branch r_beq columnpin 0 skip
			set c0 pindata set c1 1_second delay
		at skip del skip

		ri r_imm r_add param0 param0 1
		rb r_branch r_bne param0 param1 columnloop 

	ri r_jalr_op1 r_jalr_op2 0 linkregister 0



eoi









1202510035.144019

now i'm starting to work on getting the display function compiling!
its going well so farrrr







1202510024.195920
YAYYYYY i got the spi gpio expander program working!!! YAYY
now we just need to code up the display function!!











(


at loop	
	set c0 address set c1 sio_base li
	set c0 data set c1 1111_1100_0000_0000___0000_0000_01 li
	rs r_store r_sw address data sio_gpio_out

	set c0 next set c1 1111_1111_1111_1111_1111_1111 delay

	set c0 address set c1 sio_base li
	set c0 data set c1 0111_1100_0000_0000___0000_0000_00 li
	rs r_store r_sw address data sio_gpio_out

	set c0 next set c1 1111_1111_1111_1111_1111_1111 delay

rj r_jal 0 loop del loop


)






(


  500uA  at 3.3v  1k   <----- this one is the one we'll do!

x: 70uA  at 3.3v  10k     (too dim...) ;





we can't run at 5v reliabily...

x: 2.1mA  at 5v   1k ;

x: 260uA  at 5v  10k;




)








(set c0 data set c1 0000_0000_0000_0000___1 li
rs r_store r_sw address data clock_ref_div

set c0 data set c1 0000_0000_0000_0000___1 li
rs r_store r_sw address data clock_sys_div)







(current state as of 1202510094.031251

	we need to get gpio-triggered-wakeup working!

	
)








the low power program:



file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/ascii.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/rp2350.s

str "firmware.uf2" set_output_name

set next        01
set data 	next incr next
set address 	next incr next
set temp	next incr next
set other_temp  next incr next
set copy	next incr next
set led_state   next incr next
set ram   	next incr next

eq 0 0 skiproutines
at delay
	ld ra 0
	set c0 ra
	function_begin	
	set i other_temp
	set c0 i set c1 1111_1111_1111_1111_1111 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i
	function_end
	eq 0 0 ra del ra
	lt 0 0 delay
at skiproutines del skiproutines

rp2350
(sect sram_start)
sect flash_start
start_rp2350_binary

set c0 data set c1 1111_1100_1001_1111___1111_1111_1111_1111 li
set c0 address set c1 reset_base li
rs r_store r_sw address data 0
set c0 address set c1 clocks_base li
ri r_imm r_add data 0 0000_0000_0001
rs r_store r_sw address data clock_peri_control

set c0 address set c1 io_bank0_base li
set c0 data set c1 101 li
rs r_store r_sw address data io_gpio0_ctrl
set c0 address set c1 pads_bank0_base li
set c0 data set c1 1 li
rs r_store r_sw address 0 pads_gpio0
set c0 data set c1 01 li
set c0 address set c1 sio_base li
set c0 data set c1 1 li
rs r_store r_sw address data sio_gpio_oe

set c0 ram set c1 sram_start li
set c0 address set c1 powman_base li
ri r_load r_lw data address last_swcore_pwrup
ri r_imm r_add temp 0 1
rb r_branch r_bne data temp skip_boot
	ri r_imm r_add led_state 0 0
	rs r_store r_sw ram led_state 0
	set c0 address set c1 sio_base li	
	set i 0001
	set c0 i set c1 0001 li
	set c0 data set c1 1 li
	at loop		
		rs r_store r_sw address data sio_gpio_out
		delay
		rs r_store r_sw address 0 sio_gpio_out
		delay
	
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 loop 
		del loop

at skip_boot del skip_boot

set c0 address set c1 sio_base li
ri r_load r_lw led_state ram 0
ri r_imm r_eor led_state led_state 1
ri r_imm r_and led_state led_state 1
rs r_store r_sw address led_state sio_gpio_out
rs r_store r_sw ram led_state 0

set c0 address set c1 powman_base li
set n 1000_0110_1000_0000 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data powman_timer
set n 0000_0000_0000_1 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data alarm_time_15to0
set n 1110_1110_1000_0000 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data powman_timer

set c0 address set c1 powman_base li
set n 0000_1010_0000_0101 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data vreg_control
set n 0000_1011_0000_0000 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data powman_state

processor_sleep

eoi







blink program, finally working the way i want!!
updated on 1202510013.174435

1202510083.033816
updated to use the alarm system to wake up every second or so! consumes 240uA  on the pico 2, and 40 microamps less on the pico 2w!
which is pretty much according to the datasheet lol 
































































	(rr r_reg r_eor ledstate ledstate complement 0)







	(at l
	set c0 data set c1 1111_1000_0000_0000___0000_0000_01 li
	rs r_store r_sw sio data sio_gpio_out
	set c0 columnpin set c1 1_second delay	
	set c0 data set c1 1111_1000_0000_0000___0000_0000_00 li
	rs r_store r_sw sio data sio_gpio_out
	set c0 columnpin set c1 1_second delay	
	rj r_jal 0 l)









(




####unexecuted block####

at chip0_columns
	emit 1 0
	emit 1 1
	emit 1 2
	emit 1 3
	emit 1 5
	emit 1 8
	emit 1 9
	emit 1 10
	emit 1 11
	emit 1 12
	emit 1 13
	emit 1 14
	emit 1 15
	emit 1 16 (was 255)

at chip1_columns
	emit 1 0
	emit 1 1
	emit 1 6
	emit 1 7	
	emit 1 10
	emit 1 11
	emit 1 12
	emit 1 16

at chip2_columns
	emit 1 0	
	emit 1 2	
	emit 1 4	
	emit 1 7
	emit 1 8
	emit 1 11	
	emit 1 13
	emit 1 14
	emit 1 16

at chip3_columns
	emit 1 0
	emit 1 1
	emit 1 2
	emit 1 6
	emit 1 10
	emit 1 11
	emit 1 12	
	emit 1 14
	emit 1 16

at chip4_columns
	emit 1 0
	emit 1 1
	emit 1 4
	emit 1 6	
	emit 1 9
	emit 1 10	
	emit 1 13
	emit 1 14
	emit 1 16

at chip5_columns     (there is no chip 5! this marks the end of chip4.)



at chip_row_masks
	emit 01   0000_0000_0000_0000
	emit 01   0001_0100_1100_0101
	emit 01   0101_0110_0110_1001
	emit 01   0001_1101_1100_0101

(	emit 01   0011_0101_1001_1001     (lets edit this so that one bit is on!)      )
	emit 01   0011_0101_1001_1000     (note that, this value is wrong! i flipped the last bit.)    


########################






(	(we are given:
		param0	. pointer to beginning of the array of columns/pin-indexes 	
		param1	. pointer to the end of the columns/pins array.	
		param2	. chipselect sio_out value when selected. 

			

	(0.1. OR the given chipselect-on bitarray  with    

		0000_0000_0000_0000__10_00
		the bottom 5 bits will be filled in by the caller.

	(which keeps the RESET line still 1, so that they are not reset!)


	(1. loop over column pin array that was given. this is the top of the loop) 


	(2. construct a 16-bit    one-hot-encoded   bit-array      using  SLL
		such as    0001_0000_0000_0000
		)
		however, if the column pin index   value is actaully  255, 
			then simply give all zeros instead. no active one bit.



	(3. 	do a BITWISE-AND MASK of the row data for the chip, against the chip's row-mask
			which will look like something like:   0101_1101_0011_1001   idk

		...and then OR in this masked version row data for 
			this chip into the one hot encoding., 

			(specifically, i meant the row data for this chip and 
				this global_column_index, rather)


		(...we can skip step 3 for now though...   but we need to at least set those  one bits in the bitmask  to 1 in the gpio data.)



ENABLE	
	(4. we then send sio_out the data given to us in param2. the chipselect-on data.)



SEND	
	(5. send this 16-bit bit-array data   to the gpio-expander chip currently selected!)
			(via spi_data register, into the expander's latch register)



DISABLE	
	(6. we then send sio_out the "all chips unselected" value of the pin states.)


	(7. we then increment the global_column_index)







DELAY
	(8. then, if we see that the column index is not 255, 
		we then delay for a particular amount of time, based on:

		. the screen brightness we are aiming for    (less delay, more off time, less bright)
		. the refresh-rate that we are aiming for    (less delay, faster refresh rate)


	  if it is 255, then we delay for a much smaller amount. (minimal viable delay.)

		)




	(9. we loop over steps 1 through 9, until we have processed 
		all the given columns in the columns/pins array.)



	(10. return)
)




1202510035.040016


	CRAPPPP

						ALL OF THIS IS VOIDD





					THIS IS SO MUCH MORE COMPLICATED THAN I THOUGHT IT WAS




				WE NEED TO UPDATE  ALLLLLL THE ROW PINS      

								PER COLUMN PIN CHANGINGGG


		crappo lol 





resolution to this..
----------------


	1. we need to pass in the chip index    to the chip_function call. as an argumnt.

	2. we need to derive the  chipselect bitfield  ourselves, based on the chip index arg (arg2). 

	3. we need to have a "chip loop", looping over the chip-indexes, 

		when we see that we are on an iteration of this "chip loop" (for i = 1, i < 5)
		then    when i == arg2    then we must update the column pin  via the one-hot encoding bitarray
					as i depicted above. 

			and then we must OR in   the row data  for this chip,   (and this glbl col idx)


							its only HERE that we modify the columns simultanteously with the row data. 


		in all other cases, (i != arg2), we must simply keep all columns 0, (inactive)
		and then do the   bitwise-and   MASKING  over the row_data, and sending that row data to the gpio chip!


	note, notice how "i" is never 0. ie, chip0 never has any row data that we need to set. 
		this is by design. as per the wiring of the display. 
	


	4. where does this rowdata chip loop  occur???

			it occurs inside the chip_function. 

			specifically, in replace of steps   4, 5, and 6. ie, when we are sending the one-hot encoding to the chip. 

			here, we still do that, but only when we see that     i == arg2

			on the other iterations of this loop, we are talking with chips  1,2,3,4.			
			obviously, if arg2 is 0, then we'll be talking with all chips!

				which means, we need to have a seperate if statement after the chip loop, (which replaces steps 4,5,6)  which is an if statement to DO steps 4, 5, 6!!!


					and we only do this if-body    if arg2 == 0   !!!!


			that way we still send the one-hot-encoding    to chip0,   AND we update all rowdata on all other gpio expander chips!




	to help things a bit, we'll have a send function, (as sending spi data comes up twice in this algorithm, i think)

	ie, a spi_transfer   function, where you give it the  chipselecte-active-on   state of the sio_out register, and it handles the chipselect lines, as well as writing to the spi data registers, and also the wait delay loop to wait for the transmission to stop!

	we will call this function when we need to transfer data to the chips. 




















set linkregister 	1
set param0 	next incr next
set param1 	next incr next
set param2 	next incr next
set constant1 	next incr next
set bit16 	next incr next
set pin 	next incr next
set chip 	next incr next
set pindata 	next incr next
set chipselect 	next incr next

	(takes in pindata and chipselect)
at latch_transfer
	rs r_store r_sw sio chipselect sio_gpio_out
	set c0 data set c1 0010_1000_0111_0010 li
	rs r_store r_sw spi data spi_data
	rs r_store r_sw spi pindata spi_data

	at wait
		ri r_load r_lw data spi spi_status
		ri r_imm r_and data data 0000_1
		rb r_branch r_bne data 0 wait del wait

	set c0 data set c1 1111_1000_0000_0000__1 li
	rs r_store r_sw sio data sio_gpio_out


	ri r_jalr_op1 r_jalr_op2 0 linkregister 0


at chipfunction
	ri r_imm r_add constant1 0 1
	ri r_imm r_si bit16 constant1 0000_1
	
	
	at columnloop
		ri r_load r_lbu columnpin param0 0
		rr r_reg r_si columnpin constant1 columnpin 0

		rb r_branch r_bne pin bit16 skip
			ri r_imm r_add columnpin 0 0
		at skip del skip

		ri r_imm r_add chip 0 001
		at chiploop
			set c0 pindata set c1 chip_row_masks la
			rr r_reg r_add pindata pindata chip 0
			rr r_reg r_add pindata pindata chip 0
			ri r_load r_lhu pindata pindata 0
			(here, we'd put our row data into pindata!)

			rb r_branch r_bne chip param2 skip
				rr r_reg r_or pindata pindata columnpin 0
			at skip del skip

			rr r_reg r_si chipselect constant1 chip 0
			ri r_imm r_eor chipselect chipselect 1111_1
			rr r_reg r_or chipselect chipselect bit16 0

			rj r_jal linkregister latch_transfer

			ri r_imm r_add chip chip 1111_1111_1111
			rb r_branch r_bne chip 0 chiploop

		rb r_branch r_bne param2 0 skip
			ri r_imm r_add pindata columnpin 0
			ri r_imm r_add chipselect 0 0111_1
			rr r_reg r_or chipselect chipselect bit16 0
			rj r_jal linkregister latch_transfer
		at skip del skip

		set c0 columnpin set c1 1_second delay

		ri r_imm r_add param0 param0 1
		rb r_branch r_bne param0 param1 columnloop 
	
	ri r_jalr_op1 r_jalr_op2 0 linkregister 0




	(...)

at displayloop
	set c0 param0 set c1 chip0_columns la
	set c0 param1 set c1 chip1_columns la
	set c0 param2 set c1 0 li
	rj r_jal linkregister chipfunction

	set c0 param0 set c1 chip1_columns la
	set c0 param1 set c1 chip2_columns la
	set c0 param2 set c1 1 li
	rj r_jal linkregister chipfunction

	set c0 param0 set c1 chip2_columns la
	set c0 param1 set c1 chip3_columns la
	set c0 param2 set c1 01 li
	rj r_jal linkregister chipfunction

	set c0 param0 set c1 chip3_columns la
	set c0 param1 set c1 chip4_columns la
	set c0 param2 set c1 11 li
	rj r_jal linkregister chipfunction

	set c0 param0 set c1 chip4_columns la
	set c0 param1 set c1 chip5_columns la
	set c0 param2 set c1 001 li
	rj r_jal linkregister chipfunction
	
	rj r_jal 0 displayloop





































)


































(

at sendloop

	set c0 data set c1 0000_0000_0000_0000__10 li
	rs r_store r_sw sio data sio_gpio_out

	set c0 data set c1 0010_1000_0111_0010 li
	rs r_store r_sw spi data spi_data
	rs r_store r_sw spi ledstate spi_data

	at wait
		ri r_load r_lw data spi spi_status
		ri r_imm r_and data data 0000_1
		rb r_branch r_bne data 0 wait del wait

	set c0 data set c1 0000_0000_0000_0000__11 li
	rs r_store r_sw sio data sio_gpio_out

	rr r_reg r_eor ledstate ledstate complement 0

	set c0 next set c1 1_second delay

	rj r_jal 0 sendloop del sendloop

)


at loop

set c0 address set c1 powman_base li
set n 0000_1010_0000_0101 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data vreg_control
set n 0000_1111_0000_0000 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data powman_state

processor_sleep
rj r_jal 0 loop del loop































(at sendloop

	set c0 data set c1 0000_0000_0000_0000__10 li
	rs r_store r_sw sio data sio_gpio_out

	set c0 data set c1 0010_1000_0111_0010 li
	rs r_store r_sw spi data spi_data
	rs r_store r_sw spi ledstate spi_data

	at wait
		ri r_load r_lw data spi spi_status
		ri r_imm r_and data data 0000_1
		rb r_branch r_bne data 0 wait del wait

	set c0 data set c1 0000_0000_0000_0000__11 li
	rs r_store r_sw sio data sio_gpio_out

	set c0 next set c1 1_second delay

	rj r_jal 0 sendloop del sendloop)





























(

(
	use the low power modes to sleep at 280uA with no SRAM, 330uA with SRAM retention.. 
	trying to get this even lower.. 

	1202509077.141019   
	YAYY GOT THIS EVEN LOWER   down to  130uA now!!! in P1.7,    and then 210 in P1.4!


(set c0 data set c1 11 li
rs r_store r_sw address data clock_ref_control)



)

file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/ascii.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/rp2350.s

str "firmware.uf2" set_output_name

(program register allocations)


set data 	001
set address 	101
set temp	011
set other_temp  111
set copy	0001


eq 0 0 skiproutines

at delay
	ld ra 0
	set c0 ra
	function_begin
	
	set i other_temp
	set c0 i set c1 11 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i

	function_end
	eq 0 0 ra del ra
	lt 0 0 delay





at delay_long
	ld ra 0
	set c0 ra
	function_begin
	
	set i other_temp
	set c0 i set c1 1111_1111__1111_1111_1111 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i

	function_end
	eq 0 0 ra del ra
	lt 0 0 delay_long




at display_value
	ld ra 0
	set given_value c0
	set k c1
	set c0 ra
	function_begin

	rr r_reg r_add copy given_value 0 0 
	set value copy

	mul k 0001
	eq k 0 skip 
		ri r_imm r_sd value value k 
	at skip del skip

	ri r_imm r_and value value 1111_1111
	set c0 address set c1 sio_base li
	rs r_store r_sw address value sio_gpio_out


	delay



	del value
	del given_value del k
	function_end
	eq 0 0 ra del ra
	lt 0 0 display_value

at skiproutines del skiproutines


rp2350

sect flash_start

start_rp2350_binary

set c0 address set c1 reset_base li
rs r_store r_sw address 0 0


(1202509147.180328
YAYYYY I GOT IT WORKINGGGG THE RESETS WERE THE PROBLEM YAYYYYYY    I GOT SPI WORKING YAYYYYYY
)

set c0 address set c1 clocks_base li

ri r_imm r_add data 0 0000_0000_0001
rs r_store r_sw address data clock_peri_control

set c0 data set c1 0000_0000_0000_0000___1111_1111 li
rs r_store r_sw address data clock_ref_div

set c0 data set c1 0000_0000_0000_0000___1111_1111_11 li
rs r_store r_sw address data clock_sys_div

set c0 address set c1 io_bank0_base li

set c0 data set c1 101 li

rs r_store r_sw address data io_gpio0_ctrl
rs r_store r_sw address data io_gpio1_ctrl
rs r_store r_sw address data io_gpio2_ctrl
rs r_store r_sw address data io_gpio3_ctrl
rs r_store r_sw address data io_gpio4_ctrl
rs r_store r_sw address data io_gpio5_ctrl
rs r_store r_sw address data io_gpio6_ctrl
rs r_store r_sw address data io_gpio7_ctrl

rs r_store r_sw address data io_gpio23_ctrl

set c0 data set c1 1 li
rs r_store r_sw address data io_gpio16_ctrl
rs r_store r_sw address data io_gpio17_ctrl
rs r_store r_sw address data io_gpio18_ctrl
rs r_store r_sw address data io_gpio19_ctrl

set c0 address set c1 pads_bank0_base li

(set c0 data set c1 00_00_00_000 li)

rs r_store r_sw address 0 pads_gpio0
rs r_store r_sw address 0 pads_gpio1
rs r_store r_sw address 0 pads_gpio2
rs r_store r_sw address 0 pads_gpio3
rs r_store r_sw address 0 pads_gpio4
rs r_store r_sw address 0 pads_gpio5
rs r_store r_sw address 0 pads_gpio6
rs r_store r_sw address 0 pads_gpio7

rs r_store r_sw address 0 pads_gpio23

set c0 data set c1 01 li
rs r_store r_sw address data pads_gpio16
rs r_store r_sw address data pads_gpio17
rs r_store r_sw address data pads_gpio18
rs r_store r_sw address data pads_gpio19

set c0 address set c1 sio_base li
set c0 data set c1 1111_1111_0000_0000___0000_0001_0000_0000 li
rs r_store r_sw address data sio_gpio_oe
rs r_store r_sw address 0 sio_gpio_out



(

set c0 address set c1 reset_base
set c0 data set c1 1111_1101_1011_1111___1111_1111_1111_1111 li
rs r_store r_sw address data 0

set c0 address set c1 reset_base
set c0 data set c1 1111_1101_1011_1111___1100_1111_1111_1111 li
rs r_store r_sw address data 0

resets		   1111_1100_1001_1111___1011_1110_1011_0000

)







set c0 address set c1 spi0_base li

set c0 data set c1 1110_0000_0000_0000 li
rs r_store r_sw address data spi_control0

set c0 data set c1 0000_001 li
rs r_store r_sw address data spi_prescale




set c0 data set c1 1010_1101 li
rs r_store r_sw address data spi_data
set c0 data set c1 1010_1101 li
rs r_store r_sw address data spi_data
	
set c0 data set c1 01 li
rs r_store r_sw address data spi_control1

set c0 data set c1 1010_1101 li
rs r_store r_sw address data spi_data
set c0 data set c1 1010_1101 li
rs r_store r_sw address data spi_data




at loop

set random 00011
set c0 random set c1 1010_1010 li

set c0 address set c1 reset_base li
ri r_load r_lw temp address 0

set c0 random set c1 0 display_value
set c0 temp set c1 00 display_value set c0 random set c1 0 display_value
set c0 temp set c1 10 display_value set c0 random set c1 0 display_value
set c0 temp set c1 01 display_value set c0 random set c1 0 display_value
set c0 temp set c1 11 display_value set c0 random set c1 0 display_value
set c0 random set c1 0 display_value

rj r_jal 0 loop del loop






eoi




















-----------------------------------





)




































set spi 	next incr next
set sio 	next incr next
set ledstate 	next incr next
set complement  next incr next

set c0 spi set c1 spi0_base li
set c0 sio set c1 sio_base li
set c0 complement set c1 1111_1111_1111_1111 li
set c0 ledstate set c1 0 li

set c0 data set c1 1111_0000_0000_0000 li
rs r_store r_sw spi data spi_control0
set c0 data set c1 01 li
rs r_store r_sw spi data spi_prescale
rs r_store r_sw spi data spi_control1



set c0 data set c1 0000_0000_0000_0000__10 li
rs r_store r_sw sio data sio_gpio_out

set c0 data set c1 0000_0000_1110_0010 li
rs r_store r_sw spi data spi_data
rs r_store r_sw spi 0 spi_data

at wait
	ri r_load r_lw data spi spi_status
	ri r_imm r_and data data 0000_1
	rb r_branch r_bne data 0 wait del wait

set c0 data set c1 0000_0000_0000_0000__11 li
rs r_store r_sw sio data sio_gpio_out

at loop

	set c0 data set c1 0000_0000_0000_0000__10 li
	rs r_store r_sw sio data sio_gpio_out

	set c0 data set c1 0010_1000_1110_0010 li
	rs r_store r_sw spi data spi_data
	rs r_store r_sw spi ledstate spi_data

	at wait
		ri r_load r_lw data spi spi_status
		ri r_imm r_and data data 0000_1
		rb r_branch r_bne data 0 wait del wait

	set c0 data set c1 0000_0000_0000_0000__11 li
	rs r_store r_sw sio data sio_gpio_out

	rr r_reg r_eor ledstate ledstate complement 0
	set c0 next set c1 1_second delay

	rj r_jal 0 loop del loop





eoi

1202510024.175152

spi program, to test out the gpio expander chips working with the pico 2!!!

lets see if this workssss






(set i 00001
set c0 i set c1 000_1 li
at loop

	set c0 address set c1 sio_base li
	set c0 data set c1 0000_0000_0000_0000___1100_0000_01 li
	rs r_store r_sw address data sio_gpio_out

	set c0 next set c1 1_second delay

	set c0 address set c1 sio_base li
	set c0 data set c1 0000_0000_0000_0000___1100_0000_00 li
	rs r_store r_sw address 0 sio_gpio_out

	set c0 next set c1 1_second delay

	ri r_imm r_add i i 1111_1111_1111
	rb r_branch r_bne i 0 loop
)

































set i 0001
set c0 i set c1 0000_1 li
at loop

	set c0 address set c1 sio_base li
	set c0 data set c1 0000_0000_0000_0000___1100_0000_01 li
	rs r_store r_sw address data sio_gpio_out

	delay

	set c0 address set c1 sio_base li
	set c0 data set c1 0000_0000_0000_0000___1100_0000_00 li
	rs r_store r_sw address 0 sio_gpio_out

	delay

	ri r_imm r_add i i 1111_1111_1111
	rb r_branch r_bne i 0 loop


set c0 address set c1 powman_base li
set n 0000_1010_0000_0101 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data vreg_control
set n 0000_1111_0000_0000 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data powman_state

processor_sleep

rj r_jal 0 loop del loop




eoi





a blink program, 
finally working the way i want! 
we should next try to use the alarm to wake up, and use low power modes instead!

then, we are going to adapt this to use the spi gpio chips to blink it! instead of sio.



written on 1202510013.174459 by dwrr





























file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/ascii.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/rp2350.s

str "firmware.uf2" set_output_name

set data 	001
set address 	101
set temp	011
set other_temp  111
set copy	0001

eq 0 0 skiproutines

at delay
	ld ra 0
	set c0 ra
	function_begin
	
	set i other_temp
	set c0 i set c1 1111_1111_1111_1111_1111 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i

	function_end
	eq 0 0 ra del ra
	lt 0 0 delay

at skiproutines del skiproutines

rp2350

(sect sram_start)
sect flash_start

start_rp2350_binary

set c0 data set c1 1111_1100_1001_1111___1111_1111_1111_1111 li
set c0 address set c1 reset_base li
rs r_store r_sw address data 0

ri r_imm r_add data 0 0000_0000_0001
rs r_store r_sw address data clock_peri_control

set c0 address set c1 clocks_base li

set c0 address set c1 io_bank0_base li
set c0 data set c1 101 li
rs r_store r_sw address data io_gpio0_ctrl

set c0 address set c1 pads_bank0_base li
set c0 data set c1 1 li
rs r_store r_sw address 0 pads_gpio0
set c0 data set c1 01 li

set c0 address set c1 sio_base li
set c0 data set c1 1 li
rs r_store r_sw address data sio_gpio_oe
rs r_store r_sw address 0 sio_gpio_out

set i 0001
set c0 i set c1 0000_1 li
at loop
	set c0 data set c1 1 li
	rs r_store r_sw address data sio_gpio_out
	delay
	rs r_store r_sw address 0 sio_gpio_out
	delay

	ri r_imm r_add i i 1111_1111_1111
	rb r_branch r_bne i 0 loop


set c0 address set c1 powman_base li
set n 0000_1010_0000_0101 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data vreg_control
set n 0000_1111_0000_0000 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data powman_state

processor_sleep

rj r_jal 0 loop del loop




eoi





a blink program, 
finally working the way i want! 
we should next try to use the alarm to wake up, and use low power modes instead!

then, we are going to adapt this to use the spi gpio chips to blink it! instead of sio.



written on 1202510013.174459 by dwrr



























































(at delay_long
	ld ra 0
	set c0 ra
	function_begin
	
	set i other_temp
	set c0 i set c1 1111_1111__1111_1111_1111 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i

	function_end
	eq 0 0 ra del ra
	lt 0 0 delay_long)

(at display_value
	ld ra 0
	set given_value c0
	set k c1
	set c0 ra
	function_begin

	rr r_reg r_add copy given_value 0 0 
	set value copy

	mul k 0001
	eq k 0 skip 
		ri r_imm r_sd value value k 
	at skip del skip

	ri r_imm r_and value value 1111_1111
	set c0 address set c1 sio_base li
	rs r_store r_sw address value sio_gpio_out

	delay

	del value
	del given_value del k
	function_end
	eq 0 0 ra del ra
	lt 0 0 display_value)







(set c0 data set c1 00_00_00_000 li)










(ri r_imm r_add data 0 0000_0000_0001
rs r_store r_sw address data clock_peri_control)

(set c0 data set c1 0000_0000_0000_0000___1 li
rs r_store r_sw address data clock_ref_div

set c0 data set c1 0000_0000_0000_0000___1 li
rs r_store r_sw address data clock_sys_div)


















file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/ascii.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/rp2350.s

str "firmware.uf2" set_output_name

set data 	001
set address 	101
set temp	011
set other_temp  111
set copy	0001

eq 0 0 skiproutines

at delay
	ld ra 0
	set c0 ra
	function_begin
	
	set i other_temp
	set c0 i set c1 11 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i

	function_end
	eq 0 0 ra del ra
	lt 0 0 delay





at delay_long
	ld ra 0
	set c0 ra
	function_begin
	
	set i other_temp
	set c0 i set c1 1111_1111__1111_1111_1111 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i

	function_end
	eq 0 0 ra del ra
	lt 0 0 delay_long




at display_value
	ld ra 0
	set given_value c0
	set k c1
	set c0 ra
	function_begin

	rr r_reg r_add copy given_value 0 0 
	set value copy

	mul k 0001
	eq k 0 skip 
		ri r_imm r_sd value value k 
	at skip del skip

	ri r_imm r_and value value 1111_1111
	set c0 address set c1 sio_base li
	rs r_store r_sw address value sio_gpio_out


	delay



	del value
	del given_value del k
	function_end
	eq 0 0 ra del ra
	lt 0 0 display_value

at skiproutines del skiproutines


rp2350
sect flash_start
start_rp2350_binary

set c0 address set c1 reset_base li
set c0 data set c1 1111_1101_1011_1111___1111_1111_1111_1111 li
rs r_store r_sw address data 0


set c0 address set c1 clocks_base li

ri r_imm r_add data 0 0000_0000_0001
rs r_store r_sw address data clock_peri_control

set c0 data set c1 0000_0000_0000_0000___1111_1111 li
rs r_store r_sw address data clock_ref_div

set c0 data set c1 0000_0000_0000_0000___1111_1111_11 li
rs r_store r_sw address data clock_sys_div

set c0 address set c1 io_bank0_base li
set c0 data set c1 101 li
rs r_store r_sw address data io_gpio0_ctrl

set c0 address set c1 pads_bank0_base li

rs r_store r_sw address 0 pads_gpio0

set c0 address set c1 sio_base li
set c0 data set c1 1 li
rs r_store r_sw address data sio_gpio_oe
rs r_store r_sw address 0 sio_gpio_out


at loop

set random 00011
set c0 random set c1 1010_1010 li

set c0 address set c1 reset_base li
ri r_load r_lw temp address 0

set c0 random set c1 0 display_value
set c0 temp set c1 00 display_value set c0 random set c1 0 display_value
set c0 temp set c1 10 display_value set c0 random set c1 0 display_value
set c0 temp set c1 01 display_value set c0 random set c1 0 display_value
set c0 temp set c1 11 display_value set c0 random set c1 0 display_value
set c0 random set c1 0 display_value

rj r_jal 0 loop del loop











eoi

















1202509147.164824

LEDS:  RESET_DONE REGISTER VALUE

0000_0011   0110_0000      0100_0011   0100_0000


LEDS:  RESETS   REGISTER VALUE

1111_1100    1001_1111    1011_1110    1011_0000

(
	use the low power modes to sleep at 280uA with no SRAM, 330uA with SRAM retention.. 
	trying to get this even lower.. 

	1202509077.141019   
	YAYY GOT THIS EVEN LOWER   down to  130uA now!!! in P1.7,    and then 210 in P1.4!


(set c0 data set c1 11 li
rs r_store r_sw address data clock_ref_control)



)























OLD PROGRAM
















file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/ascii.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/rp2350.s

str "firmware.uf2" set_output_name

set data 	01
set address 	001
set temp	101

rp2350
sect flash_start
start_rp2350_binary

set c0 address set c1 reset_base li
set c0 data set c1 1111_1101_1011_1111___1111_1111_1111_1111 li
rs r_store r_sw address data 0

set c0 address set c1 io_bank0_base li
set c0 data set c1 101 li
rs r_store r_sw address data io_gpio0_ctrl
set c0 address set c1 pads_bank0_base li
rs r_store r_sw address 0 pads_gpio0

set c0 address set c1 sio_base li
set c0 data set c1 1 li
rs r_store r_sw address data sio_gpio_oe
set c0 data set c1 1 li
rs r_store r_sw address data sio_gpio_out

(set j 0001
set c0 j set c1 0000_1 li
at loop)

	set c0 address set c1 sio_base li
	set c0 data set c1 1 li
	rs r_store r_sw address data sio_gpio_out
	
	(set i temp
	set c0 i set c1 0000_0000_0000_0000___0000_0001 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i)


	(set c0 address set c1 sio_base li
	rs r_store r_sw address 0 sio_gpio_out

	set i temp
	set c0 i set c1 0000_0000_0000_0000___0000_0001 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i)

	(ri r_imm r_add j j 1111_1111_1111
	rb r_branch r_bne j 0 loop
	del loop del j)

set c0 address set c1 powman_base li
set n 0000_1010_0000_0101 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data vreg_control
set n 0000_1111_0000_0000 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data powman_state

processor_sleep


eoi



1202509302.043240

a simple blink program, modified to hopefully be more low power! (soon)























(
	use the low power modes to sleep at 280uA with no SRAM, 330uA with SRAM retention.. 
	trying to get this even lower.. 

	1202509077.141019   
	YAYY GOT THIS EVEN LOWER   down to  130uA now!!! in P1.7,    and then 210 in P1.4!

)










(set c0 data set c1 1 li
rs r_store r_sw address data io_gpio16_ctrl
rs r_store r_sw address data io_gpio17_ctrl
rs r_store r_sw address data io_gpio18_ctrl
rs r_store r_sw address data io_gpio19_ctrl)



(set c0 data set c1 01 li
rs r_store r_sw address data pads_gpio16
rs r_store r_sw address data pads_gpio17
rs r_store r_sw address data pads_gpio18
rs r_store r_sw address data pads_gpio19)



(rs r_store r_sw address data io_gpio23_ctrl)
(set c0 data set c1 00_00_00_000 li)
(rs r_store r_sw address 0 pads_gpio23)



(set c0 address set c1 spi0_base li
	
set c0 data set c1 1010_1101_1011_0011 li
rs r_store r_sw address data spi0_data
	
set c0 data set c1 1111_0000_1111_1111 li
rs r_store r_sw address data spi0_control0
	
set c0 data set c1 01 li
rs r_store r_sw address data spi0_prescale
	
set c0 data set c1 01 li
rs r_store r_sw address data spi0_control1)








set c0 address set c1 clocks_base li
(set c0 data set c1 11 li
rs r_store r_sw address data clock_ref_control)

(set c0 data set c1 0000_0000_0000_0000___1111_1110 li
rs r_store r_sw address data clock_ref_div)

(set c0 data set c1 0000_0000_0001 li
rs r_store r_sw address data clock_peri_control)



















}

...

ri r_load r_lw led_state ram 0
ri r_imm r_xor led_state led_state 0000_1
ri r_imm r_and led_state led_state 0000_1

set c0 address set c1 sio_base li
rs r_store r_sw address led_state sio_gpio_out
rs r_store r_sw ram led_state 0

set c0 address set c1 powman_base li
set n 1000_0110_1000_0000 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data powman_timer
set n 0000_0000_001 add n powman_password 
set c0 data set c1 n li
rs r_store r_sw address data alarm_time_15to0
set n 1110_1110_1000_0000 add n powman_password 
set c0 data set c1 n li
rs r_store r_sw address data powman_timer




















(set c0 ram set c1 sram_start li
set c0 address set c1 powman_base li
i r_load r_lw data address last_swcore_pwrup
ri r_imm r_add temp 0 1
rb r_branch r_bne data temp skip_boot {

	ri r_imm r_add led_state 0 0
	rs r_store r_sw ram led_state 0

	set c0 address set c1 sio_base li



	.... 

}









(
(turn off the usb phy controller and its pins completely.)
set c0 address set c1 usb_base li
set c0 data set c1 0010_0010_0000_11 li
rs r_store r_sw address data usbphy_direct
set c0 data set c1 1111_1111_1111_1001_111 li
rs r_store r_sw address data usbphy_direct_override
)









(set c0 address set c1 reset_base
set c0 data set c1 1111_1101_1011_1111____1111_1111_1111_1111 li
rs r_store r_sw address data 0

set c0 address set c1 io_bank0_base li
set c0 data set c1 101 li
rs r_store r_sw address data io_gpio16_ctrl
rs r_store r_sw address data io_gpio23_ctrl

set c0 address set c1 pads_bank0_base li
set c0 data set c1 0100_111 li
rs r_store r_sw address data pads_gpio16
rs r_store r_sw address data pads_gpio23

set c0 address set c1 sio_base li
set c0 data set c1 0000_0000_0000_0000___1000_0001_0000_0000 li
rs r_store r_sw address data sio_gpio_oe
rs r_store r_sw address 0 sio_gpio_out)










set c0 address set c1 sio_base li
at times

set c0 data set c1 0000_0000_0000_0000_1 li
rs r_store r_sw address data sio_gpio_out

set i temp
set c0 i set c1 0000_0000_0000_0000_0001 li
at l
	ri r_imm r_add i i 1111_1111_1111
	rb r_branch r_bne i 0 l
del l del i

rs r_store r_sw address 0 sio_gpio_out

set i temp
set c0 i set c1 0000_0000_0000_0000_0001 li
at l
	ri r_imm r_add i i 1111_1111_1111
	rb r_branch r_bne i 0 l
del l del i
	
rj r_jal 0 times


eoi

































1202509022.014214   rewritten to use the modern way of doing rp2350 programs

also, i'm trying to figure out why its not as low power as it should be...













do blink sleep sequence:



(

set c0 ram set c1 sram_start li
ri r_load r_lw led_state ram 0
ri r_imm r_xor led_state led_state 1
ri r_imm r_and led_state led_state 1

set c0 address set c1 sio_base li
ri r_imm r_sll data led_state 0000_1
rs r_store r_sw address data sio_gpio_out
rs r_store r_sw ram led_state 0

set c0 address set c1 powman_base li
set n 1000_0110_1000_0000 add n powman_password 
set c0 data set c1 n li
rs r_store r_sw address data powman_timer

set n 0000_0000_001 add n powman_password 
set c0 data set c1 n li
rs r_store r_sw address data alarm_time_15to0

set n 1110_1110_1000_0000 add n powman_password 
set c0 data set c1 n li
rs r_store r_sw address data powman_timer

)






check boot:



(set c0 address set c1 powman_base li
ri r_load r_lw data address last_swcore_pwrup
ri r_imm r_add temp 0 1
rb r_branch r_bne data temp skip_boot

	(ri r_imm r_add led_state 0 0
	rs r_store r_sw ram led_state 0

	set c0 address set c1 sio_base li
	set j 00001
	set c0 j set c1 0001 li

	at times

	set c0 data set c1 0000_0000_0000_0000_1 li
	rs r_store r_sw address data sio_gpio_out

	set i temp
	set c0 i set c1 0000_0000_0000_0000_0001 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i

	rs r_store r_sw address 0 sio_gpio_out

	set i temp
	set c0 i set c1 0000_0000_0000_0000_0001 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i
	
	ri r_imm r_add j j 1111_1111_1111
	rb r_branch r_bne j 0 times)
	
at skip_boot del skip_boot)













change clocks:


(set c0 address set c1 clocks_base li
set c0 data set c1 11 li
rs r_store r_sw address data clock_ref_control

set c0 address set c1 clocks_base li
set c0 data set c1 0000_0000_0000_0000___0000_0110 li
rs r_store r_sw address data clock_ref_div)





















set c0 address set c1 reset_clear li
set c0 data set c1 0000_0010_01 li
rs r_store r_sw address data 0




set c0 address set c1 sio_base li
set c0 data set c1 1 (1111_1111__1111_1111__1111_1111__1111_1100) li
rs r_store r_sw address data sio_gpio_oe

(set c0 data set c1 0 li)
rs r_store r_sw address 0 sio_gpio_out

set c0 ram set c1 sram_start li
set c0 address set c1 powman_base li
ri r_load r_lw data address last_swcore_pwrup
ri r_imm r_add temp 0 1
rb r_branch r_bne data temp skip_boot

	ri r_imm r_add led_state 0 0
	rs r_store r_sw ram led_state 0

	set c0 address set c1 sio_base li
	set j 00001
	set c0 j set c1 0001 li
	at times
	set c0 data set c1 1 li
	rs r_store r_sw address data sio_gpio_out

	set i temp
	set c0 i set c1 0000_0000_0000_0000_0001 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i

	(set c0 data set c1 0 li)
	rs r_store r_sw address 0 sio_gpio_out

	set i temp
	set c0 i set c1 0000_0000_0000_0000_0001 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i
	
	ri r_imm r_add j j 1111_1111_1111
	rb r_branch r_bne j 0 times
	
at skip_boot del skip_boot

ri r_load r_lw led_state ram 0
ri r_imm r_xor led_state led_state 1
ri r_imm r_and led_state led_state 1

set c0 address set c1 sio_base li
rs r_store r_sw address led_state sio_gpio_out
rs r_store r_sw ram led_state 0

set c0 address set c1 powman_base li

	(set initial the timer config, allow nonsecure writes)
set n 1000_0110_1000_0000 add n powman_password 
set c0 data set c1 n li
rs r_store r_sw address data powman_timer


	(set the alarm duration of time, after which we will wake up  :   1000 milliseconds, == 1 second)
	(on reset, all the other alarm value registers are 0)
set n 0000_0000_001 add n powman_password 
set c0 data set c1 n li
rs r_store r_sw address data alarm_time_15to0

	(start the alarm timer to wake up in that amount of time)
set n 1110_1110_1000_0000 add n powman_password 

set c0 data set c1 n li
rs r_store r_sw address data powman_timer

	(request the new low power state P1.4, write password protected state register)
set n 0000_0011_0000_0000  add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data powman_state
processor_sleep



eoi




















1202507292.234007  got this working!!! YAYYYYY
it still consumes 0.68mA  ie  680 microamps, so we still have some additional work to do, 

	but we are getting there :)

















(


(set sleep_en0 0010_1101
set sleep_en1 0001_1101

set c0 address set c1 clocks_base li
(set c0 data set c1 0 li)
rs r_store r_sw address 0 sleep_en0

set c0 address set c1 clocks_base li
set c0 data set c1 0000_1111_1111_1100__0000_0000_0000_0000 li
rs r_store r_sw address data sleep_en1)
	current state:

		1202508041.202313

			apparently setting all pins to outputs wasnt the current draw problem, 
			which means we have to look through the datasheet to see if we missed anything that is causing there to be 400uA of excess current draw lol... apparently the shmid triggers and pulldowns are enabled for all inputs, on reset, so thats not the problem... hmmm


)




(set i 0
at loop
	set c0 i setup_output
	add i 1
	lt i 01111 loop del loop del i)













(


set c0 address set c1 sio_base li
at ledloop
	set c0 data set c1 1 li
	rs r_store r_sw address data sio_gpio_out

	set i ram
	set c0 i set c1 0000_0000_0000_0000_0001 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i
	
	set c0 data set c1 0 li
	rs r_store r_sw address data sio_gpio_out

	set i ram
	set c0 i set c1 0000_0000_0000_0000_0001 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i
rj r_jal 0 ledloop

)



































(    -------------- first attempt at low power sequence    sleep seq ----------------



	(request the new low power state P1.4, write password protected state register)
set n 0000_0011_0000_0000  add n powman_password 
set c0 data set c1 n li
rs r_store r_sw address data powman_state


	(set initial the timer config, allow nonsecure writes)

set n 1000_0100_1000_0000 add n powman_password 
set c0 data set c1 n li
rs r_store r_sw address data powman_timer


	(set the alarm duration of time, after which we will wake up  :   1024 milliseconds, about 1 second)
	(on reset, all the other alarm value registers are 0)

set n 0000_0000_001 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data alarm_time_15to0


	(start the alarm timer to wake up in that amount of time)

set n 1110_1110_1000_0000 add n powman_password 

set c0 data set c1 n li
rs r_store r_sw address data powman_timer



processor_sleep    (the processor's execution won't return after executing this)

)






















(
////////////////////////////////////////////
1202507292.201314

		bug: currently we are unable to 
		access the powman registers at all.

		figure out why this is happening!




	(request the new low power state P1.4, write password protected state register)
set n 0000_0011_0000_0000  add n powman_password 
set c0 data set c1 n li
rs r_store r_sw address data powman_state

	

		the above code causes us to processor-reset  
		and thus we are obviously doing something wrong lol...



////////////////////////////////////////////

)













(this is where we would check the   lastpwrupcore   register to see what caused us to be here.
   in the case of blinking an led, we don't need to though, luckily.)





(




set c0 ram set c1 sram_start li

(load led state from sram)
ri r_load r_lw led_state ram 0

ri r_imm r_xor led_state led_state 1
ri r_imm r_and led_state led_state 1

(set the led to this state!)
rs r_store r_sw address led_state sio_gpio_out

(storing the led state to memory)
rs r_store r_sw ram led_state 0

set c0 address set c1 powman_base li

	(request the new low power state P1.4, write password protected state register)

set n 0000_0011_0000_0000  add n powman_password 
set c0 data set c1 n li
rs r_store r_sw address data powman_state

	(set initial the timer config, allow nonsecure writes)

set n 1000_0100_1000_0000 add n powman_password 
set c0 data set c1 n li
rs r_store r_sw address data powman_timer

	(set the alarm duration of time, after which we will wake up  :   1024 milliseconds, about 1 second)
	(on reset, all the other alarm value registers are 0)

set n 0000_0000_001 add n powman_password
set c0 data set c1 n li
rs r_store r_sw address data alarm_time_15to0

	(start the alarm timer to wake up in that amount of time)

set n 1110_1110_1000_0000 add n powman_password 

set c0 data set c1 n li
rs r_store r_sw address data powman_timer

processor_sleep    (the processor's execution won't return after executing this)




)




















































(





new iter:   low-power program:
------------------------------------------------------


x
------------------------------------------------------
0. store any data you care about, (and don't wish to reconstruct from scratch on boot)
   in somewhere in SRAM, prior
------------------------------------------------------





------------------------------------------------------
1. we need to first write to the STATE register,    

		0011  bits 7 through 4    to go into  P1.4
------------------------------------------------------



------------------------------------------------------
2. then we need to configure the alarm:

	 the ALARM_TIME_15TO0-ALARM_TIME_63TO48 registers : { 

		ALARM_TIME_15TO0  = 0x84     (16 bit value)
			
		ALARM_TIME_31TO16 = 0x80
		
		ALARM_TIME_47TO32 = 0x7c
		
		ALARM_TIME_63TO48 = 0x78	
	}

	which can we found at:   POWMAN_BASE + (offset)
------------------------------------------------------




------------------------------------------------------
3. then we need to set the POWMAN_BASE + TIMER(0x88) register's bits    .PWRUP_ON_ALARM  and .ALARM_ENAB   bits  simultaneouslyyy
	upon doing this, the alarm will be live, and running
------------------------------------------------------



------------------------------------------------------
4. then we need to issue the   wfi instruction to go to sleep 
------------------------------------------------------



			....

		we will be sleeping now. 

			....




------------------------------------------------------
5. when we wake up , we will be at the beginning of our program, 
	and thus we need to set up the pins/gpio output registers again, 
	and once everything is setup consistent with both program cases:  
					(the lpm wakeup, and chip boot-up cases)



	we will then check:    The POWMAN_BASE + LAST_SWCORE_PWRUP ( 0xa0 )  register


		on value 6,
			this means it was caused from a timer alarm triggering, 

		on value 0, 
			it means chip reset. 

	we can now branch to seperate parts of the program, depending on this value,
	and thus we can load the data we stored to SRAM, back into registers, 
	and then we can continue where we left off. 



------------------------------------------------------





)


( a low power program to blink an LED, 
 and then use the low power sleep modes for delays
written on 1202507163.175855 by dwrr )












(	(set the led to this state!)
	set data 1
	r5_s sw_op1 sw_op2 address data sio_gpio_out
	
	set i 0000_0000_0000_0000_001 at d sub i 1 ne i 0 d del d del i
	
	(set the led to this state!)
	set data 0
	r5_s sw_op1 sw_op2 address data sio_gpio_out
	
	set i 0000_0000_0000_0000_0001 at d sub i 1 ne i 0 d del d del i

)







(ne led_state 0 setzero
	set led_state 1
do done at setzero del setzero
	set led_state 0
at done del done)


(at loop

	(set the led to this state!)
	set data 1
	r5_s sw_op1 sw_op2 address data sio_gpio_out
	
	set i 0000_0000_0000_0000_0000_01 at d sub i 1 ne i 0 d del d del i
	
	(set the led to this state!)
	set data 0
	r5_s sw_op1 sw_op2 address data sio_gpio_out
	
	set i 0000_0000_0000_0000_0000_01 at d sub i 1 ne i 0 d del d del i

eq 0 0 loop)

























(set data 0000_0000__0000_0000__0000_0000__0000_0000
r5_s sw_op1 sw_op2 address data sio_gpio_out
do wfi)








(



at delay
	ld ra 0 nat
	rt set i 0
	at L ge i c0 done
	add i 1 do L at done
	del i del L del done
	ct do ra del ra


at delayr
	ld ra 0 nat
	rt set ii 0
	at LL ge ii a0 donee
	add ii 1 do LL at donee
	del ii del LL del donee
	ct do ra del ra
)









	(ct 
		set millisecond 		0000_0000_0000_1
		set half_millisecond 		0000_000_1

		set 10_milliseconds millisecond 
		mul 10_milliseconds 0101

		set 5_milliseconds millisecond 
		mul 5_milliseconds 101

		set 3_milliseconds millisecond 
		mul 3_milliseconds 11
	rt

	set increment half_millisecond
	set iterator_limit 10_milliseconds
	set iterator_limit2 3_milliseconds

	set iterator increment
	at inner
		set data 1
		r5_s sw_op1 sw_op2 address data sio_gpio_out
		set i iterator at d sub i 1 ne i 0 d del d del i
		set data 0
		r5_s sw_op1 sw_op2 address data sio_gpio_out
		set i iterator_limit sub i iterator at d sub i 1 ne i 0 d del d del i
		add iterator increment
		lt iterator iterator_limit2 inner del inner

	set iterator iterator_limit2
	at inner
		sub iterator increment
		set data 1
		r5_s sw_op1 sw_op2 address data sio_gpio_out
		set i iterator at d sub i 1 ne i 0 d del d del i
		set data 0
		r5_s sw_op1 sw_op2 address data sio_gpio_out
		set i iterator_limit sub i iterator at d sub i 1 ne i 0 d del d del i
		lt increment iterator inner del inner

	set i 0000_0000_0000_0000_0000_01 at d sub i 1 ne i 0 d del d del i)



	(set c0 0000_0000_0000_0000_0000_01
	do delay)


(lt iterator increment else sub iterator increment do if
		at else set iterator 0 at if del if del else)










(


a program to pwm an LED on GPIO 0 using a 
risc-v uf2 file outputted by the compiler,
running on the pico 2 W. 
written on 1202507104.030034 by dwrr

previously:
 testing out the generation of the 
risc-v uf2 file for programming the pico 2 W.
written 1202505272.173200 by dwrr
)








		(set a0 iterator do delayr)
		(set c0 millisecond do delay)
		(set c0 _milliseconds do delay)









		(set a0 i do delayr)


		(set remaining width sub remaining i
		set a0 remaining do delayr)

					(currrent state 1202507082.215905   

					the above code doesnt quite work yet, its supposed to be pwm'ing an LED lol

					)









	(set c0 0000_0000_0000_0000_0000_01 do delay)




















	( ^-----   1202507082.212023 we coulddd generate a cte executed   "del" instruction, as i think this might be the only valid solution to this problem without adding an instructin. del must be executed at compiletime. hmmmmm  crappp no we can't do thissss becuase  we often do    "do ra del ra"
	and thus, we need some way of not requiring the del to be executed, but still knowing that the labels   L and done    are local... hmmmmmmm crapppp

		i mean, hypothetically, we could just emit a del in cte space anyways, 

			and that signals to us that   when we see a label, it means that its local?... hmmm that could work..... 
	
				the idea would be that  the first time we see the label, we
ll use the original label value,   and then the second time we execute this macro, if we see we are trying to use a deleted thingy, 

				and it was deleted as a result of previous execution, 
				then thatttt triggers us to rewrite the variable reference to refer to a new variable entirely. hmmm 

				yikes. this is very complicated.    but it might work though. interesting. 


	)









(risc-v registers)
(set zr 0
set ra 1)

(notes on the arguments for riscv machine instructions: )

   (arguments for r5_i   is always:    r5_i  opcode rd funct rs1 imm12  )
   (arguments for r5_s   is always:    r5_s  opcode imm12 funct rs1(address) rs2(data)  )

(other useful hardware peripherial register constants)







(	set data 0010
	r5_s sw_op1 sio_gpio_out sw_op2 address data

	set count 0000_0000_0000_0000_0000_01
	set i 0 at delayloop add i 1 lt i count delayloop del delayloop

	set data 0001
	r5_s sw_op1 sio_gpio_out sw_op2 address data

	set count 0000_0000_0000_0000_0000_01
	set i 0 at delayloop add i 1 lt i count delayloop del delayloop
)







(
set address 	io_bank0_base
set data 	101
r5_s sw_op1 io_gpio1_ctrl sw_op2 address data
set address 	pads_bank0_base
set data 	0_1_0_0_11_1_0_0
r5_s sw_op1 pads_gpio1 sw_op2 address data

set address 	io_bank0_base
set data 	101
r5_s sw_op1 io_gpio2_ctrl sw_op2 address data
set address 	pads_bank0_base
set data 	0_1_0_0_11_1_0_0
r5_s sw_op1 pads_gpio2 sw_op2 address data

set address 	io_bank0_base
set data 	101
r5_s sw_op1 io_gpio3_ctrl sw_op2 address data
set address 	pads_bank0_base
set data 	0_1_0_0_11_1_0_0
r5_s sw_op1 pads_gpio3 sw_op2 address data

set address 	io_bank0_base
set data 	101
r5_s sw_op1 io_gpio23_ctrl sw_op2 address data
set address 	pads_bank0_base
set data 	0_1_0_0_11_1_0_0
r5_s sw_op1 pads_gpio23 sw_op2 address data

)










	(register x 1 set x ram_start
	r5_i addi_op1 zr 0 zr 0
	r5_s sw_op1 0111 sw_op2 1 01)



(

testing out lui with sets and branches: 

set i 0
at loop
	add i 1
	lt i 0000_1000_0100_1100_0010_1010_0110_1110 loop
halt
)




















at chip0_columns

	emit 1 0    8
	emit 1 1    9
	emit 1 2    10
	emit 1 3    11
	emit 1 5    13

	emit 1 8    0
	emit 1 9    1
	emit 1 10   2
	emit 1 11   3

	emit 1 12   4
	emit 1 13   5
	emit 1 14   6
	emit 1 15   7

	emit 1 16 (was 255)

at chip1_columns

	emit 1 0    8
	emit 1 1    9
	emit 1 6    14
	emit 1 7    15

	emit 1 10   2
	emit 1 11   3
	emit 1 12   4

	emit 1 16

at chip2_columns

	emit 1 0   8 
	emit 1 2   10
	emit 1 4   12
	emit 1 7   15

	emit 1 8   0
	emit 1 11  3
	emit 1 13  5
	emit 1 14  6

	emit 1 16

at chip3_columns

	emit 1 0   8
	emit 1 1   9
	emit 1 2   10
	emit 1 6   14

	emit 1 10  2
	emit 1 11  3
	emit 1 12  4
	emit 1 14  6

	emit 1 16

at chip4_columns

	emit 1 0    8
	emit 1 1    9
	emit 1 4    12
	emit 1 6    14

	emit 1 9    1
	emit 1 10   2
	emit 1 13   5
	emit 1 14   6

	emit 1 16




at chip5_columns     (there is no chip 5! this marks the end of chip4.)


(
at chip_row_masks
	emit 01   0000_0000_0000_0000
	emit 01   0001_0100_1100_0101
	emit 01   0101_0110_0110_1001
	emit 01   0001_1101_1100_0101

(	emit 01   0011_0101_1001_1001     (lets edit this so that one bit is on!)      )
	emit 01   0011_0101_1001_1000     (note that, this value is wrong! i flipped the last bit.)    
)



at chip_row_masks
	emit 01   0000_0000_0000_0000
	emit 01   1100_0101_0001_0100
	emit 01   0110_1001_0101_0110
	emit 01   1100_0101_0001_1101
	emit 01   1001_1001_0011_0100     (note that, this value is wrong! i flipped bit 15.)    


