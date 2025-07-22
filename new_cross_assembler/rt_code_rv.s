file library/core.s

st compiler_target rv32_arch
st compiler_format hex_array
st compiler_should_overwrite true

ri rv_imm rv_add rv_system_number 0 rv_system_exit
ri rv_imm rv_add rv_system_arg0 0 10001
ri rv_ecall 0 0 0 0

eoi


current state:
	we are trying to write a rv exit program currently

testing out the risc-v code gen backend  so that we can start theprocess of writing runtime code in this language! also testing out the rt cte generation logic and semantics, to make sure i implemented it right lol

1202507185.034458 by dwrr





