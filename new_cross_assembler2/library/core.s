(the main standard library file for the language, target independent.
written on 1202507277.191648 by dwrr)


zero x set _ra x
incr x set target x
incr x set format x 
incr x set overwrite x
incr x set debug x
incr x set stacksize x
incr x set ctepass x
incr x set ctputc x
del x

(supported targets)

set r32_arch 001
set hex_array 111



(risc-v op codes)

set r_auipc 		1110_100
set r_ecall 		1100_111
set r_imm 		1100_100
set r_add		000

set r_branch		1100_011
set r_equal		000
set r_less		011



(risc-v sc abi)

set r_arg0 	0101
set r_arg1 	1101
set r_arg2 	0011
set r_number 	10001




(system call numbers for my riscv vm website)

set r_exit 1
set r_write 11







