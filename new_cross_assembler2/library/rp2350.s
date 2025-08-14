(
a file contains constants and useful things for the pico 2w microcontroller

written on 1202508133.235133 by dwrr

depends on library/useful.s, 
	which depends on library/core.s
)




(rp2350-riscv family ID)

set rp2350_riscv 0101_1010_1111_1111__1101_0001_0010_0111 


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
set spi0_base		0000_0000_0000_0000__0001_0000_0000_0010


(clock registers)

set clock_ref_control 	0000_1100
set clock_ref_div 	0010_1100
set rosc_div 		0010_1000


(spi peripheral registers)

set spi0_control0 	0
set spi0_control1 	001
set spi0_data 		0001
set spi0_status 	0011
set spi0_prescale 	0000_1



(powman registers:  ones for the alarm and low power mode configuration)

set powman_password 	0000_0000_0000_0000___0111_1111_0101_1010
set last_swcore_pwrup	0000_0101
set powman_state 	0001_1100
set powman_timer 	0001_0001
set alarm_time_15to0  	0010_0001
set alarm_time_31to16	0000_0001
set alarm_time_47to32	0011_1110
set alarm_time_63to48	0001_1110


(reset registers)

set reset_clear reset_base 
add reset_clear clear_on_write



(pad and gpio registers)

set io_gpio0_ctrl  001
set io_gpio1_ctrl  001_1
set io_gpio2_ctrl  001_01
set io_gpio3_ctrl  001_11
set io_gpio4_ctrl  001_001
set io_gpio5_ctrl  001_101
set io_gpio6_ctrl  001_011
set io_gpio7_ctrl  001_111
set io_gpio8_ctrl  001_0001
set io_gpio9_ctrl  001_1001
set io_gpio10_ctrl  001_0101
set io_gpio11_ctrl  001_1101
set io_gpio12_ctrl  001_0011
set io_gpio13_ctrl  001_1011
set io_gpio14_ctrl  001_0111
set io_gpio15_ctrl  001_1111
set io_gpio16_ctrl  001_0000_1
set io_gpio17_ctrl  001_1000_1
set io_gpio18_ctrl  001_0100_1
set io_gpio19_ctrl  001_1100_1
set io_gpio20_ctrl  001_0010_1
set io_gpio21_ctrl  001_1010_1
set io_gpio22_ctrl  001_0110_1
set io_gpio23_ctrl  001_1110_1
set io_gpio24_ctrl  001_0001_1
set io_gpio25_ctrl  001_1001_1
set io_gpio26_ctrl  001_0101_1
set io_gpio27_ctrl  001_1101_1
set io_gpio28_ctrl  001_0011_1
set io_gpio29_ctrl  001_1011_1
set io_gpio30_ctrl  001_0111_1

set pads_gpio0  00_1
set pads_gpio1  00_01
set pads_gpio2  00_11
set pads_gpio3  00_001
set pads_gpio4  00_101
set pads_gpio5  00_011
set pads_gpio6  00_111
set pads_gpio7  00_0001
set pads_gpio8  00_1001
set pads_gpio9  00_0101
set pads_gpio10  00_1101
set pads_gpio11  00_0011
set pads_gpio12  00_1011
set pads_gpio13  00_0111
set pads_gpio14  00_1111
set pads_gpio15  00_0000_1
set pads_gpio16  00_1000_1
set pads_gpio17  00_0100_1
set pads_gpio18  00_1100_1
set pads_gpio19  00_0010_1
set pads_gpio20  00_1010_1
set pads_gpio21  00_0110_1
set pads_gpio21  00_1110_1
set pads_gpio23  00_0001_1
set pads_gpio24  00_1001_1
set pads_gpio25  00_0101_1
set pads_gpio26  00_1101_1
set pads_gpio27  00_0011_1
set pads_gpio28  00_1011_1
set pads_gpio29  00_0111_1
set pads_gpio30  00_1111_1


(sio gpio registers)

set sio_gpio_oe 	0000_11
set sio_gpio_out 	0000_1
set sio_gpio_in 	001




eq 0 0 skip_routines

at rp2350
	ld ra 0
	set c0 ra function_begin

	riscv_uf2
	st uf2_family_id rp2350_riscv
	
	function_end
	eq 0 0 ra del ra
	lt 0 0 rp2350


(1202508133.235719 this function is deprecated, as theres a more efficient way to do it...)

at setup_output

	ld ra 0
	set n c0
	set c0 ra function_begin

	set control n
	mul control 0001
	add control 001

	set pads n 
	add pads 1
	mul pads 001

	set c0 address set c1 io_bank0_base li
	set c0 data set c1 101 li
	rs r_store r_sw address data control

	set c0 address set c1 pads_bank0_base li
	set c0 data set c1 0_1_0_0_11_1_0_0 li
	rs r_store r_sw address data pads

	del pads del control 
	function_end
	eq 0 0 ra del ra
	lt 0 0 setup_output	


at processor_sleep
	ld ra 0 set c0 ra function_begin

	rr r_reg r_slt 0 0 0 0

	function_end eq 0 0 ra del ra
	lt 0 0 processor_sleep




at start_rp2350_binary
	ld ra 0
	set c0 ra function_begin

	rj r_jal 0 skip     (rp2350 image_def marker)
	emit  001  1100_1011_0111_1011__1111_1111_1111_1111
	emit  001  0100_0010_1000_0000__1000_0000_1000_1000
	emit  001  1111_1111_1000_0000__0000_0000_0000_0000
	emit  001  0000_0000_0000_0000__0000_0000_0000_0000
	emit  001  1001_1110_1010_1100__0100_1000_1101_0101
	at skip del skip

	function_end eq 0 0 ra del ra
	lt 0 0 start_rp2350_binary


at skip_routines del skip_routines





eoi

a pico 2w standard library
written on 1202508133.235434 by dwrr









