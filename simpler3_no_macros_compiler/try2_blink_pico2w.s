(testing out the generation of the 
risc-v uf2 file for programming the pico 2 W.
written 1202505272.173200 by dwrr)

file library/foundation.s ct
st compiler_target rv32_arch nat
st compiler_format uf2_executable nat
st compiler_should_overwrite true nat
st compiler_stack_size 0 nat 

(address atomic bitmasks) 
set clear_on_write 	0000_0000_0000_11
set set_on_write 	0000_0000_0000_01
set toggle_on_write 	0000_0000_0000_1

(memory map of rp2350)

set flash_start 	0000_0000_0000_0000__0000_0000_0000_1000
set ram_start 		0000_0000_0000_0000__0000_0000_0000_0100
set powman_base		0000_0000_0000_0000__0000_1000_0000_0010
set clocks_base		0000_0000_0000_0000__1000_0000_0000_0010
set sio_base		0000_0000_0000_0000__0000_0000_0000_1011
set reset_base 		0000_0000_0000_0000__0100_0000_0000_0010
set io_bank0_base 	0000_0000_0000_0001__0100_0000_0000_0010
set pads_bank0_base 	0000_0000_0000_0001__1100_0000_0000_0010

(risc-v op codes)
set addi_op1 	1100100
set addi_op2	000
set sw_op1 	1100010
set sw_op2 	010

set reset_clear reset_base 
add reset_clear clear_on_write

set io_gpio0_ctrl 001
set io_gpio1_ctrl 0011
set io_gpio2_ctrl 00101
set io_gpio3_ctrl 00111

set pads_gpio0 001
set pads_gpio1 0001
set pads_gpio2 0011
set pads_gpio3 00001

set sio_gpio_oe 	0000_11
set sio_gpio_out 	0000_1
set sio_gpio_in 	001

rt set a0 a0
set a1 a1
set a2 a2
set a3 a3

ct set c0 c0
set c1 c1
set c2 c2
set c3 c3

do skip_macros

at setif
	ld ra 0 nat
	set a c0 set b c1 
	set c c2 set d c3
	ne a b l st c d nat
	at l del l del a del b  
	del c del d do ra del ra 

at setup_output
	ld ra 0 nat
	set p compiler_base set c2 p

	set c1 0 set c3 io_gpio0_ctrl do setif
	set c1 1 set c3 io_gpio1_ctrl do setif
	set c1 01 set c3 io_gpio2_ctrl do setif
	set c1 11 set c3 io_gpio3_ctrl do setif
	ld control p nat

	set c1 0 set c3 pads_gpio0 do setif
	set c1 1 set c3 pads_gpio1 do setif
	set c1 01 set c3 pads_gpio2 do setif
	set c1 11 set c3 pads_gpio3 do setif
	ld pads p nat
	del p	
	rt set address io_bank0_base
	set data 101
	r5_s sw_op1 sw_op2 address data control
	set address pads_bank0_base
	set data 0_1_0_0_11_1_0_0
	r5_s sw_op1 sw_op2 address data pads
	del pads del control 
	ct do ra del ra

at delay
	ld ra 0 nat

	set count 0000_0000_0000_0000_0000_1

	rt 

	set i 0

	at L	
		lt i count done
		add i 1 do L
	at done

	del count 
	del i 
	del L
	del done

	ct do ra 
	del ra



at skip_macros del skip_macros
rt adr flash_start

do skip  
(rp2350 image_def marker)
emit  001  1100_1011_0111_1011__1111_1111_1111_1111
emit  001  0100_0010_1000_0000__1000_0000_1000_1000
emit  001  1111_1111_1000_0000__0000_0000_0000_0000
emit  001  0000_0000_0000_0000__0000_0000_0000_0000
emit  001  1001_1110_1010_1100__0100_1000_1101_0101
at skip del skip

set address 	reset_clear
set data 	0000_0010_01
r5_s sw_op1 sw_op2 address data 0

set c0 0 do setup_output

set address	sio_base
set data 	1
r5_s sw_op1 sw_op2 address data sio_gpio_oe

set data 1
r5_s sw_op1 sw_op2 address data sio_gpio_out

at loop
	set data 1
	r5_s sw_op1 sw_op2 address data sio_gpio_out
	do delay

	set data 0
	r5_s sw_op1 sw_op2 address data sio_gpio_out
	do delay
do loop






































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



















