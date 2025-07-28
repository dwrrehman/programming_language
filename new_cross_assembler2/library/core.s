(the main standard library file for the language, target independent.
written on 1202507277.191648 by dwrr)


set stdin 0
set stdout 1


zero x set _return_address x
incr x set target x
incr x set format x 
incr x set overwrite x
incr x set ctedebug x
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
set r_arg3 	1011
set r_arg4 	0111
set r_arg5 	1111

set r_number 	10001




(system call numbers for my riscv vm website)

set r_exit 1
set r_read 01
set r_write 11











set r_lui 		1110_110
set r_auipc 		1110_100

set r_jal		1111_011

set r_jalr_op1		1110_011
set r_jalr_op2		000

set r_branch		1100_011
set r_beq		000
set r_bne		100
set r_bltu		011
set r_bgeu		111
set r_blt		001
set r_bge		101

set r_load 		1100_010
set r_lb 		000
set r_lh 		100
set r_lw 		010
set r_ld 		110
set r_lbu 		001
set r_lhu 		101
set r_lwu 		011

set r_store 		1100_010
set r_sb 		000
set r_sh 		100
set r_sw 		010

set r_ecall 		1100_111
set r_imm 		1100_100
set r_signed		0000_010
set r_signedi		0000_0000_0010
set r_reg		1100_1100

set r_add		000
set r_sll		100
set r_slt		010
set r_sltu		110
set r_xor		001
set r_srl		101
set r_or		011
set r_and		111

set r_remu_op1		1100_110
set r_remu_op2		111
set r_remu_op3		1000000

set r_divu_op1		1100_110
set r_divu_op2		101
set r_divu_op3		1000000

set r_mul_op1		1100_110
set r_mul_op2		000
set r_mul_op3		1000000

set r_mulhu_op1		1100_110
set r_mulhu_op2		110
set r_mulhu_op3		1000000

eoi




