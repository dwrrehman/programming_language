(a new hello world with the new cross assembler label arch.
1202507255.170159 by dwrr)
st 001 1

zero x set _ra x
incr x set target x
incr x set format x 
incr x set overwrite x
incr x set debug x
incr x set stacksize x
incr x set strlen x del x

set r32_arch 001
set hex_array 111

set r_auipc 		1110_100
set r_ecall 		1100_111
set r_imm 		1100_100
set r_add		000

set r_branch		1100_011
set r_equal		000
set r_less		011

set r_arg0 	0101
set r_arg1 	1101
set r_arg2 	0011
set r_number 	10001

set r_exit 1
set r_write 11

st target r32_arch
st format hex_array
st overwrite 1

set limit 1
set i 01

ri r_imm r_add i 0 0
ri r_imm r_add limit 0 101

at loop

set done -1  (only required if in a macro-like cte context lol)

rb r_branch r_equal i limit done

ri r_imm r_add r_number 0 r_write
ri r_imm r_add r_arg0 0 1

ru r_auipc r_arg1 0
ri r_imm r_add r_arg1 r_arg1 001001

ri r_imm r_add r_arg2 0 011
ri r_ecall 0 0 0 0

ri r_imm r_add i i 1
rb r_branch r_equal 0 0 loop

at done

ri r_imm r_add r_number 0 r_exit
ri r_imm r_add r_arg0 0 0011
ri r_ecall 0 0 0 0

at string
str "hello
"





