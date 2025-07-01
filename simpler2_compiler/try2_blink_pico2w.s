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

set powman_base		0000_0000_0000_0000__0000_0000_0000_0000

set sio_base		0000_0000_0000_0000__0000_0000_0000_1011
set reset_base 		0000_0000_0000_0000__0100_0000_0000_0010
set io_bank0_base 	0000_0000_0000_0001__0100_0000_0000_0010
set pads_bank0_base 	0000_0000_0000_0001__1100_0000_0000_0010

(risc-v op codes)
set addi_op1 	1100100
set addi_op2	000

set sw_op1 	1100010
set sw_op2 	010


(risc-v registers)
set zr 0
set ra 1


(notes on the arguments for riscv machine instructions: )

   (arguments for r5_i   is always:    r5_i  opcode rd funct rs1 imm12  )
   (arguments for r5_s   is always:    r5_s  opcode imm12 funct rs1(address) rs2(data)  )



(other useful hardware peripherial register constants)

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


(--------- runtime program ------------)

rt section flash_start

register address  1 
register data 	01 

do skip  ( this is the valid IMAGE_DEF marker for the rp2350 binary. )
emit  001  1100_1011_0111_1011__1111_1111_1111_1111
emit  001  0100_0010_1000_0000__1000_0000_1000_1000
emit  001  1111_1111_1000_0000__0000_0000_0000_0000
emit  001  0000_0000_0000_0000__0000_0000_0000_0000
emit  001  1001_1110_1010_1100__0100_1000_1101_0101
at skip del skip


(deassert subsystem resets on the io_bank and pads_bank subsystems.)

set address 	reset_clear
set data 	0000_0010_01
r5_s sw_op1 0 sw_op2 address data




(gpio 0)

(select the SIO gpio function for gpio0.)

set address 	io_bank0_base
set data 	101
r5_s sw_op1 io_gpio0_ctrl sw_op2 address data

(configure the gpio0's pad options as: output, drive 12mA)

set address 	pads_bank0_base
set data 	0_1_0_0_11_1_0_0
r5_s sw_op1 pads_gpio0 sw_op2 address data



(gpio 1)

(select the SIO gpio function for gpio1.)

set address 	io_bank0_base
set data 	101
r5_s sw_op1 io_gpio1_ctrl sw_op2 address data

(configure the gpio1's pad options as: output, drive 12mA)

set address 	pads_bank0_base
set data 	0_1_0_0_11_1_0_0
r5_s sw_op1 pads_gpio1 sw_op2 address data




(gpio 2)

(select the SIO gpio function for gpio2.)

set address 	io_bank0_base
set data 	101
r5_s sw_op1 io_gpio2_ctrl sw_op2 address data

(configure the gpio2's pad options as: output, drive 12mA)

set address 	pads_bank0_base
set data 	0_1_0_0_11_1_0_0
r5_s sw_op1 pads_gpio2 sw_op2 address data



(gpio 3)

(select the SIO gpio function for gpio3.)

set address 	io_bank0_base
set data 	101
r5_s sw_op1 io_gpio3_ctrl sw_op2 address data

(configure the gpio3's pad options as: output, drive 12mA)

set address 	pads_bank0_base
set data 	0_1_0_0_11_1_0_0
r5_s sw_op1 pads_gpio3 sw_op2 address data







(gpio 23)

(select the SIO gpio function for gpio23.)

set address 	io_bank0_base
set data 	101
r5_s sw_op1 io_gpio23_ctrl sw_op2 address data

(configure the gpio23's pad options as: output, drive 12mA)

set address 	pads_bank0_base
set data 	0_1_0_0_11_1_0_0
r5_s sw_op1 pads_gpio23 sw_op2 address data





(enable output via SIO  for gpio0 through gpio2.)

set address	sio_base
set data 	1111_0000__0000_0000___0000_0001__0000_0000
r5_s sw_op1 sio_gpio_oe  sw_op2 address data



(set gpio23 to 0, to disable the wifi chip's regulator,  and set all other gpios to 1.)

set data 1111_0000__0000_0000___0000_0000__0000_0000
r5_s sw_op1 sio_gpio_out sw_op2 address data



register i 11
register count 001

at loop
	set data 1000
	r5_s sw_op1 sio_gpio_out sw_op2 address data

	set count 0000_0000_0000_0000_0000_01
	set i 0 at delayloop add i 1 lt i count delayloop del delayloop

	set data 0100
	r5_s sw_op1 sio_gpio_out sw_op2 address data

	set count 0000_0000_0000_0000_0000_01
	set i 0 at delayloop add i 1 lt i count delayloop del delayloop

	set data 0010
	r5_s sw_op1 sio_gpio_out sw_op2 address data

	set count 0000_0000_0000_0000_0000_01
	set i 0 at delayloop add i 1 lt i count delayloop del delayloop

	set data 0001
	r5_s sw_op1 sio_gpio_out sw_op2 address data

	set count 0000_0000_0000_0000_0000_01
	set i 0 at delayloop add i 1 lt i count delayloop del delayloop

	do loop
halt



















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



















