(a shell-like program for arm64
written on 1202508041.175448 by dwrr)

file library/core.s
file library/useful.s

set target arm64_arch
st output_format macho_executable
st executable_stack_size min_stack_size_macos
st overwrite_output true


addi a6_sp a6_sp 0000_01 0 0 1 1     (allocate 32 bytes of stack memory)


mov a6_number a6_write 0 mov_type_zero 1
mov a6_arg0 stdout 0 mov_type_zero 1
adr a6_arg1 welcome 0
mov a6_arg2 welcome.length 0 mov_type_zero 1
svc

set input_length 1001
mov input_length 0 0 mov_type_zero 1


at loop

addi a6_zero input_length 10000    0 1 1 1
bc is_equal terminateprogram

mov a6_number a6_write 0 mov_type_zero 1
mov a6_arg0 stdout 0 mov_type_zero 1
adr a6_arg1 prompt 0
mov a6_arg2 prompt.length 0 mov_type_zero 1
svc

mov a6_number a6_read 0 mov_type_zero 1
mov a6_arg0 stdin 0 mov_type_zero 1
addi a6_arg1 a6_sp 0 0 0 0 1
mov a6_arg2 0001 0 mov_type_zero 1
svc

addi input_length a6_arg0 0 0 0 0 1

mov a6_number a6_write 0 mov_type_zero 1
mov a6_arg0 stdout 0 mov_type_zero 1
addi a6_arg1 a6_sp 0 0 0 0 1
addi a6_arg2 input_length 0 0 0 0 1         (register move)
svc


(addi a6_arg1 a6_sp 0 0 0 0 1)   (here, we would load a byte from the stack pointer, and then compare with 'q'....we'll need the memiu instruction probably? not sure...)


jmp 0 loop





at terminateprogram

addi a6_sp a6_sp 0000_01 0 0 0 1    (deallocate 32 bytes of stack memory)

mov a6_number a6_exit 0 mov_type_zero 1
mov a6_arg0 10001 0 mov_type_zero 1
svc




at welcome str 
"[shell version 0.1]
"

set c0 welcome getstringlength
set welcome.length c0

at prompt str 
":ready: "

set c0 prompt getstringlength
set prompt.length c0









eoi









1202507222.003453
a simple hello world program for arm64



1202507222.012226 YAYYY got it working yayyyyy
that wasnt that bad at all lollll   this assembler makes things pretty easyyyy

now time for a prime number program!! that will be an interesting testtttt



1202507303.125609
rewritten to use the new syntax and stdlib for the language!






























zero false
zero true incr true
zero allones sub allones 1 set -1 allones

set stdin 0
set stdout 1
set stderr 01

zero target

zero x set no_arch x
incr x set riscv_arch x
incr x set msp430_arch x
incr x set arm64_arch x 

zero x set no_output x
incr x set macho_executable x 
incr x set macho_object x 
incr x set elf_executable x 
incr x set elf_object x 
incr x set ti_txt_executable x 
incr x set uf2_executable x 
incr x set hex_array x 
incr x set bin_output x

zero x set return_address x
incr x set set_output_format x
incr x set executable_stack_size x 
incr x set uf2_family_id x
incr x set overwrite_output x
incr x set assembler_pass x 
incr x set assembler_putc x 


del x


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




(risc-v system call abi)

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











(arm64 machine instruction opcodes) 

set mov_type_zero 01



(arm64 hardware registers)

set a6_zero_reg 11111
set a6_link_reg 01111



(system call abi for macos)

set a6_number 00001
set a6_arg0 0
set a6_arg1 1
set a6_arg2 01
set a6_arg3 11


(system call numbers for macos)

set a6_exit 1
set a6_fork 01
set a6_read 11
set a6_write 001






eoi 
------------------------------------------------------


the core standard library for the assembler

   written on 1202508037.200136 by dwrr








(some useful routines for the standard library)


set '0' 000011
set newline 0101

zero c0 zero c1
zero c2 zero c3
zero c3 zero c5
zero c6 zero c7

eq 0 0 skip_all_routines

at riscv_uf2
	ld ra 0

	set target riscv_arch
	st set_output_format uf2_executable
	st overwrite 1

	eq 0 0 ra del ra lt 0 0 riscv_uf2


at riscv_hex
	ld ra 0

	set target riscv_arch
	st set_output_format hex_array
	st overwrite 1

	eq 0 0 ra del ra lt 0 0 riscv_hex


at rem
	ld ra 0
	set n c0 set m c1
	set d n div d m
	set r d mul r m set k n sub k r
	set c0 k
	del k del r del n del m del d
	eq 0 0 ra del ra
	lt 0 0 rem

at ctprint
	ld ra 0
	set n c0
	at loop
		set bit n 
		set c0 bit set c1 01 rem set bit c0 
		add bit '0'
		st assembler_putc bit del bit
		div n 01
		lt 0 n loop  
	del loop del n
	eq 0 0 ra del ra lt 0 0 ctprintbinary

at ctnl 
	ld ra 0
	st assembler_putc newline 
	eq 0 0 ra del ra lt 0 0 ctnl

at not 
	ld ra 0
	set r -1 sub r c0
	set c0 r del r
	eq 0 0 ra del ra 
	lt 0 0 not

at si
	ld ra 0
	set value c0
	set shift_amount c1

	lt shift_amount 0000_001 valid
		eq 0 0 -1 str "error: invalid shift amount to si" 
	at valid del valid

	set i 0
	at loop
		eq i shift_amount done 
		mul value 01
		incr i 
		eq 0 0 loop 
		del loop del i del shift_amount
	at done del done

	set c0 value del value 
	eq 0 0 ra del ra
	lt 0 0 si

at sd
	ld ra 0
	set value c0	
	set shift_amount c1

	lt shift_amount 0000_001 valid
		eq 0 0 -1 str "error: invalid shift amount to si" 
	at valid del valid

	set i 0
	at loop
		eq i shift_amount done 
		div value 01
		incr i 
		eq 0 0 loop 
		del loop del i del shift_amount
	at done del done

	set c0 value del value 
	eq 0 0 ra del ra
	lt 0 0 sd

at and     (a = a & b  in 64 bits)
	ld ra 0 set a c0 set b c1
	set c0 1 set c1 111_111 si set msb c0
	zero i zero c
	at loop
		mul c 01
		lt a msb s 
		lt b msb s 
			incr c 
		at s del s
		mul a 01 mul b 01 
		incr i lt i 0000_001 loop del loop del i
	del a del b del msb
	set c0 c del c
	eq 0 0 ra del ra 
	lt 0 0 and



at or    (a = a | b    in 64 bits)
	ld ra 0 set a c0 set b c1
	set c0 1 set c1 111_111 si set msb c0
	zero i zero c
	at loop
		add c c
		lt a msb try_b
			incr c 
			eq 0 0 advance
		at try_b 
		lt b msb advance
			incr c
		at advance 
		add a a 
		add b b
		incr i lt i 0000_001 loop 

	del a del b del msb 
	del advance del try_b 
	del loop del i
	set c0 c del c
	eq 0 0 ra del ra 
	lt 0 0 or




at la
	ld ra 0

	eq target riscv_arch valid
		eq 0 0 -1 str "error: la only supports riscv currently" 
	at valid del valid
	
	at pc 
	set offset c1 
	sub offset pc 
	del pc
	ru r_auipc c0 0

(todo: do the bitwise operations to extract 
the top 20 bits of the immediate too!!)

	ri r_imm r_add c0 c0 offset 
	del offset
	eq 0 0 ra del ra
	lt 0 0 la



at li
	ld ra 0	

	eq target riscv_arch valid
		eq 0 0 -1 str "error: la only supports riscv currently" 
	at valid del valid

	set destination c0
	set immediate c1
	lt immediate 0000_0000_0000_0000_0000_0000_0000_0000_1 skip_abort
		eq 0 0 -1 str "error: argument to li did not fit in a 32-bit number."
	at skip_abort del skip_abort

	set c0 immediate set c1 0000_0000_0001 and set bit c0
	set c0 bit set c1 1101 sd set bit c0
	set c0 immediate set c1 0011 sd set a c0 add a bit 
	set c0 a set c1 1111_1111_1111_1111_1111 and set a c0
	set c0 immediate set c1 1111_1111_1111 and set b c0

	set source 0
	eq 0 a s
		ru r_lui destination a
		set source destination
	at s del s

	eq 0 b s
		ri r_imm r_add destination source b
	at s del s

	lt 0 immediate s
		ri r_imm r_add destination source b
	at s del s

	del source del a del b del bit
	del destination del immediate
	eq 0 0 ra del ra
	lt 0 0 li


at exit
	ld ra 0

	eq target riscv_arch riscv_exit
	eq target arm64_arch arm64_exit
	eq 0 0 -1 str "error: exit system only supports riscv and arm64" 

	at arm64_exit 
	
		mov a6_number a6_exit 0 mov_type_zero 1
		mov a6_arg0 c0 0 mov_type_zero 1
		svc

	eq 0 0 return
	at riscv_exit

		ri r_imm r_add r_number 0 r_exit
		ri r_imm r_add r_arg0 0 c0
		ri r_ecall 0 0 0 0

	at return
	del riscv_exit del arm64_exit
	eq 0 0 ra del ra 
	lt 0 0 exit



at writestring
	ld ra 0

	set string c0
	set length c1

	eq target riscv_arch riscv_writestring
	eq target arm64_arch arm64_writestring

	eq 0 0 -1 str "error: writestring only supports riscv and arm64"

	at arm64_writestring

		mov a6_number a6_write 0 mov_type_zero 1
		mov a6_arg0 stdout 0 mov_type_zero 1
		adr a6_arg1 string 0
		mov a6_arg2 string.length 0 mov_type_zero 1
		svc
	
	eq 0 0 return
	at riscv_writestring

		ri r_imm r_add r_number 0 r_write
		ri r_imm r_add r_arg0 0 stdout
		set c0 r_arg1 set c1 string la
		ri r_imm r_add r_arg2 0 length
		ri r_ecall 0 0 0 0
	
	at return

	del riscv_writestring 
	del arm64_writestring 
	del string del length

	eq 0 0 ra del ra  lt 0 0 writestring



at readchar
	ld ra 0

	eq target riscv_arch valid
		eq 0 0 -1 str "error: readchar only supports riscv currrently"
	at valid del valid

	set out c0
	set copy r_arg3
	ri r_imm r_add r_number 0 r_read
	ri r_imm r_add r_arg0 0 stdin
	set c0 r_arg1 set c1 buffer la
	ri r_imm r_add copy r_arg1 0 
	ri r_imm r_add r_arg2 0 1
	ri r_ecall 0 0 0 0
	ri r_load r_lbu out copy 0
	eq 0 0 ra del ra del out del copy
	lt 0 0 readchar


at writechar
	ld ra 0
	set in c0
	ri r_imm r_add r_number 0 r_write
	ri r_imm r_add r_arg0 0 stdout
	set c0 r_arg1 set c1 buffer la
	rs r_store r_sb r_arg1 in 0
	ri r_imm r_add r_arg2 0 1
	ri r_ecall 0 0 0 0
	eq 0 0 ra del ra del in
	lt 0 0 writechar




at writebuffer 
	ld ra 0

	eq target riscv_arch valid
		eq 0 0 -1 str "error: writebuffer only supports riscv currrently"
	at valid del valid

	set pointer_register c0
	set length_register c1
	ri r_imm r_add r_number 0 r_write
	ri r_imm r_add r_arg0 0 stdout
	ri r_imm r_add r_arg1 pointer_register 0
	ri r_imm r_add r_arg2 length_register 0
	ri r_ecall 0 0 0 0
	eq 0 0 ra del ra 
	del pointer_register del length_register
	lt 0 0 writebuffer

at getstringlength
	ld ra 0
	set begin c0
	at here set c0 here sub c0 begin
	del here del begin
	eq 0 0 ra del ra
	lt 0 0 getstringlength




at printbinary   (note: "buffer" must be present in the executable.)
	ld ra 0


	eq target riscv_arch valid
		eq 0 0 -1 str "error: writebuffer only supports riscv currrently"
	at valid del valid


	set input c0
	set bit 	r_number
	set data 	r_arg0
	set begin 	r_arg1
	set p 		r_arg2
	
	ri r_imm r_add data input 0
	set c0 begin set c1 buffer la
	ri r_imm r_add p begin 0
	at loop
		ri r_imm r_and bit data 1
		ri r_imm r_add bit bit '0'
		rs r_store r_sb p bit 0
		ri r_imm r_add p p 1
		ri r_imm r_srl data data 1
		rb r_branch r_bne data 0 loop

	ri r_imm r_add bit 0 newline
	rs r_store r_sb p bit 0
	ri r_imm r_add p p 1

	ri r_imm r_add r_number 0 r_write
	ri r_imm r_add r_arg0 0 stdout
	rr r_reg r_add r_arg2 p begin r_signed
	ri r_ecall 0 0 0 0

	del loop 
	del p 
	del bit 
	del data 
	del begin
	del input

	eq 0 0 ra del ra
	lt 0 0 printbinary



at readstring
	ld ra 0

	eq target riscv_arch valid
		eq 0 0 -1 str "error: writebuffer only supports riscv currrently"
	at valid del valid

	set string c0
	set length c1
	ri r_imm r_add r_number 0 r_read
	ri r_imm r_add r_arg0 0 stdin
	set c0 r_arg1 set c1 string la
	ri r_imm r_add r_arg2 0 length
	ri r_ecall 0 0 0 0
	eq 0 0 ra del ra 
	del string del length
	lt 0 0 readstring


at stringsequal 
	ld ra 0

	eq 0 0 allones
	str 'unimplemented'

	eq 0 0 ra del ra


at skip_all_routines del skip_all_routines



eoi


a set of utility routines which are useful
for risc-v programs, and compiletime programs.
originally written on 1202507292.174513 by dwrr 

rewritten and made more portable on 1202508037.203119 

















