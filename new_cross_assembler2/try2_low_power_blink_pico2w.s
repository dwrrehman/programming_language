file library/core.s
file library/useful.s

st target rv32_arch
st format uf2_executable
st overwrite true
st stacksize 0

(address atomic bitmasks) 
set clear_on_write 	0000_0000_0000_11
set set_on_write 	0000_0000_0000_01
set toggle_on_write 	0000_0000_0000_1

(memory map of rp2350)
set flash_start 	0000_0000_0000_0000__0000_0000_0000_1000
set sram_start 		0000_0000_0000_0000__0000_0000_0000_0100
set powman_base		0000_0000_0000_0000__0000_1000_0000_0010
set clocks_base		0000_0000_0000_0000__1000_0000_0000_0010
set sio_base		0000_0000_0000_0000__0000_0000_0000_1011
set reset_base 		0000_0000_0000_0000__0100_0000_0000_0010
set io_bank0_base 	0000_0000_0000_0001__0100_0000_0000_0010
set pads_bank0_base 	0000_0000_0000_0001__1100_0000_0000_0010
set accessctrl_base	0000_0000_0000_0000__0110_0000_0000_0010

(powman registers:  ones for the alarm and low power mode configuration)
set powman_password 	0000_0000_0000_0000___0111_1111_0101_1010
set last_swcore_pwrup	0000_0101
set powman_state 	0001_1100
set powman_timer 	0001_0001
set alarm_time_15to0  	0010_0001
set alarm_time_31to16	0000_0001
set alarm_time_47to32	0011_1110
set alarm_time_63to48	0001_1110


(useful thingy)
set reset_clear reset_base 
add reset_clear clear_on_write

(pad and gpio registers)
set io_gpio0_ctrl  001
set io_gpio1_ctrl  0011
set io_gpio2_ctrl  0010_1
set io_gpio3_ctrl  0011_1
set io_gpio23_ctrl 0011_1101

set pads_gpio0  001
set pads_gpio1  0001
set pads_gpio2  0011
set pads_gpio3  0000_1
set pads_gpio23 0000_011

set sio_gpio_oe 	0000_11
set sio_gpio_out 	0000_1
set sio_gpio_in 	001




(program register allocations)

set led_state 	1
set data 	01
set ram 	11
set address 	001
set temp	101

eq 0 0 skip_macros

at setif
	ld ra 0 lt 0 0 setif
	set a c0 set b c1 
	set c c2 set d c3
	lt a b l lt b a l st c d
	at l del l del a del b  
	del c del d eq 0 0 ra del ra 

at setup_output
	ld ra 0
	set p 00001 set c2 p

	set c1 0 set c3 io_gpio0_ctrl setif
	set c1 1 set c3 io_gpio1_ctrl setif
	set c1 01 set c3 io_gpio2_ctrl setif
	set c1 11 set c3 io_gpio3_ctrl setif
	set c1 11101 set c3 io_gpio23_ctrl setif
 	ld control p

	set c1 0 set c3 pads_gpio0 setif
	set c1 1 set c3 pads_gpio1 setif
	set c1 01 set c3 pads_gpio2 setif
	set c1 11 set c3 pads_gpio3 setif
	set c1 11101 set c3 pads_gpio23 setif
	ld pads p

	del p

	set c0 address set c1 io_bank0_base li
	set c0 data set c1 101 li
	rs r_store r_sw address data control

	set c0 address set c1 pads_bank0_base li
	set c0 data set c1 0_1_0_0_11_1_0_0 li
	rs r_store r_sw address data pads

	del pads del control 
	eq 0 0 ra del ra
	lt 0 0 setup_output	

at processor_sleep
	ld ra 0 
	rr r_reg r_slt 0 0 0 0
	eq 0 0 ra del ra
	lt 0 0 processor_sleep


at skip_macros del skip_macros








sect flash_start

rj r_jal 0 skip
			(rp2350 image_def marker)
emit  001  1100_1011_0111_1011__1111_1111_1111_1111
emit  001  0100_0010_1000_0000__1000_0000_1000_1000
emit  001  1111_1111_1000_0000__0000_0000_0000_0000
emit  001  0000_0000_0000_0000__0000_0000_0000_0000
emit  001  1001_1110_1010_1100__0100_1000_1101_0101

at skip del skip


set c0 address set c1 reset_clear li
set c0 data set c1 0000_0010_01 li
rs r_store r_sw address data 0



set sleep_en0 0010_1101
set sleep_en1 0001_1101


set c0 address set c1 clocks_base li
set c0 data set c1 0 li
rs r_store r_sw address data sleep_en0

set c0 address set c1 clocks_base li
set c0 data set c1 0000_1111_1111_1100__0000_0000_0000_0000 li
rs r_store r_sw address data sleep_en1




set c0 0 setup_output
set c0 11101 setup_output


set c0 address set c1 sio_base li
set c0 data set c1 1000_0000__0000_0000__0000_0001__0000_0000 li
rs r_store r_sw address data sio_gpio_oe

set c0 data set c1 0 li
rs r_store r_sw address data sio_gpio_out

set c0 ram set c1 sram_start li

set c0 address set c1 powman_base li

ri r_load r_lw data address last_swcore_pwrup

ri r_imm r_add temp 0 1
rb r_branch r_bne data temp skip_boot

	(initialize RAM led variable)
	ri r_imm r_add led_state 0 0
	rs r_store r_sw ram led_state 0

	set c0 address set c1 sio_base li

set j 00001
set c0 j set c1 0001 li

at times
	(led on)
	set c0 data set c1 1 li
	rs r_store r_sw address data sio_gpio_out

	(delay for a little bit)
	set i temp
	set c0 i set c1 0000_0000_0000_0000_0001 li
	at l
		ri r_imm r_add i i 1111_1111_1111
		rb r_branch r_bne i 0 l
	del l del i

	(led off)
	set c0 data set c1 0 li
	rs r_store r_sw address data sio_gpio_out

	(delay for a little bit)
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





eoi






























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



















