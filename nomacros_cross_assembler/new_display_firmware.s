. 
	a program to blink an LED on the 
	msp430fr2433 msp430 microcontroller, 
	written on 1202503134.230344 by dwrr. 
	rewritten to use macros and binary 
	constants on 1202503204.214715 
. 

lf foundation.s 
lf msp430fr2433_hardware.s

set _targetarchitecture 	msp430_arch 
set _outputformat 		ti_txt_executable
set _shouldoverwrite 		true

. port 2 pins : controlling a parellel-output 
  serial-input shift register. 
. 
df srclk	set srclk	bit0
df rclk		set rclk	bit1
df srclr	set srclr	bit2
df oe		set oe		bit3
df ser		set ser		bit4
df latch	set latch	bit5

df stack_start set stack_start ram_start add stack_start 000000001

section fram_start
	gen4 msp_mov  reg_mode sp 0   imm_mode imm_reg  stack_start  size_word

	df x bn x 0000_0001_0101_1010 
	gen4 msp_mov  fixed_mode fixed_reg wdt_wdtctl   imm_mode imm_reg x size_word
	udf x

	gen4 msp_bis  fixed_mode fixed_reg port2_p2dir  imm_mode imm_reg  1111_111     size_byte
	gen4 msp_bis  fixed_mode fixed_reg port2_p2out  imm_mode imm_reg  1111_111     size_byte
	gen4 msp_bic  fixed_mode fixed_reg pmm_pm5ctl0  literal_mode constant_1 0  size_word

	df x bn x 0000_0000_1010_0101
	gen4 msp_mov  fixed_mode fixed_reg pmm_pmmctl0  imm_mode imm_reg  x  size_word
	udf x

	. set_p2_pin   latch . 
	set_p2_pin   srclr
	reset_p2_pin oe
	reset_p2_pin ser
	reset_p2_pin rclk
	reset_p2_pin srclk

	reset_p2_pin srclr  delay
	set_p2_pin srclr

	df value set value 011
	df addend set addend 111
	gen4 msp_xor reg_mode value 0   reg_mode value 0  size_word
	gen4 msp_mov reg_mode addend 0  imm_mode imm_reg 1  size_word

df main at main		
	reset_p2_pin ser
	gen4 msp_add reg_mode value 0 reg_mode addend 0 size_word
	gen4 msp_bit reg_mode value 0 imm_mode imm_reg 0001 size_word
	df l br4 condjz l
		set_p2_pin ser
	at l udf l

	gen4 msp_bit reg_mode value 0 imm_mode imm_reg 0000_0000_0001 size_word
	df l br4 condjz l
		reset_p2_pin latch
	at l udf l

	set_p2_pin srclk
	set_p2_pin rclk    delay
	reset_p2_pin srclk
	reset_p2_pin rclk  delay

	br4 condjmp main
	msp_nop

section reset_vector
	emit nat16 fram_start

eoi







