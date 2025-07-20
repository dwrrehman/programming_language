file library/core.s

st compiler_target rv32_arch
st compiler_format hex_array
st compiler_should_overwrite true

set r5_ecall_op1 1100_111

ri r5_addi_op1 r5_addi_op2 rv_system_number 0 rv_system_exit
ri r5_addi_op1 r5_addi_op2 rv_system_arg0 0 0011
ri r5_ecall_op1 0 0 0 0

eoi



current state:
	we are trying to write a rv exit program currently




testing out the risc-v code gen backend  so that we can start theprocess of writing runtime code in this language! also testing out the rt cte generation logic and semantics, to make sure i implemented it right lol

1202507185.034458 by dwrr





