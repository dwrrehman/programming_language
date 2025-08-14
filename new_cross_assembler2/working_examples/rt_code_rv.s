(this is a riscv test exit program! rewritten on 1202508133.054042)
file library/core.s

st output_format hex_array
st overwrite_output true

ri r_imm r_add r_number 0 r_exit
ri r_imm r_add r_arg0 0 10001
ri r_ecall 0 0 0 0

str "hi lol"

eoi

current state:
	we are trying to write a rv exit program currently

testing out the risc-v code gen backend  so that we can start theprocess of writing runtime code in this language! also testing out the rt cte generation logic and semantics, to make sure i implemented it right lol
1202507185.034458 by dwrr





