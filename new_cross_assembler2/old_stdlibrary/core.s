(the main standard library file for the language, target independent.
written on 1202507277.191648 by dwrr)

set -1 0 sub -1 1

set stdin 0
set stdout 1
set stderr 01

set true 1
set false 0

(supported targets)

zero x set no_arch x
incr x set rv64_arch x
incr x set rv32_arch x
incr x set arm64_arch x
incr x set arm32_arch x
incr x set msp430_arch x


(supported formats)

zero x set no_output x
incr x set macho_executable x
incr x set macho_object x
incr x set elf_executable x
incr x set elf_object x
incr x set ti_txt_executable x
incr x set uf2_executable x
incr x set hex_array x


(cte memory locations)

zero x set _returnaddress x
incr x set target x
incr x set format x 
incr x set overwrite x
incr x set ctedebug x
incr x set stacksize x
incr x set ctepass x
incr x set ctputc x



(risc-v op codes)

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

set r_load 		1100_000
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







del x
eoi




