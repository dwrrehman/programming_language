 (testing out the generation of the 
risc-v uf2 file for programming the pico 2 W.
written 1202505272.173200 by dwrr)

file library/foundation.s ct
set ctsc_number compiler_target set ctsc_arg0 rv32_arch system
set ctsc_number compiler_format set ctsc_arg0 uf2_executable system
set ctsc_number compiler_overwrite set ctsc_arg0 true system
set ctsc_number compiler_stacksize set ctsc_arg0 0 system 


(address atomic bitmasks) 

set clear_on_write 0000_0000_0000_11
set set_on_write 0000_0000_0000_01
set toggle_on_write 0000_0000_0000_1




(memory map of rp2350) 

set ram_start 	0000_0000_0000_0000__0000_0000_0000_0100
set reset_base 	0000_0000_0000_0000__0100_0000_0000_0010




(risc-v op codes)

set addi_op1 1100100 

set sw_op1 1100010
set sw_op2 010




(risc-v registers)

set zr 0




(notes on the arguments for riscv machine instructions: )

   (arguments for r5_i   is always:    r5_i  opcode rd funct rs1 imm12  )
   (arguments for r5_s   is always:    r5_s  opcode imm12 funct rs1(address) rs2(data)  )





(--------- actual code ------------)

rt section ram_start

ct set reset_clear reset_base or reset_clear clear_on_write 
rt

register x 1 set x reset_clear
register data 01 set data 0000_0010_01   (clear bit 9 and bit 6 in the reset_base register)

r5_s sw_op1 0 sw_op2 x data
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



















