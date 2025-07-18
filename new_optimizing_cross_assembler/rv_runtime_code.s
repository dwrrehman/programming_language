file library/core.s

st compiler_target rv32_arch
st compiler_format hex_array

set myvar 0


ri r5_addi_op1 r5_addi_op2 myvar myvar 00000
ri r5_addi_op1 r5_addi_op2 myvar myvar 00000
ri r5_addi_op1 r5_addi_op2 0 0 00000


eoi

current state:
	we are trying to write a rv exit program currently




testing out the risc-v code gen backend  so that we can start theprocess of writing runtime code in this language! also testing out the rt cte generation logic and semantics, to make sure i implemented it right lol

1202507185.034458 by dwrr





