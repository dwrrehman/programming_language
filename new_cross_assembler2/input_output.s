file library/core.s
file library/ascii.s

st compiler_target rv32_arch
st compiler_format hex_array
st compiler_should_overwrite true

set c0 c0
set c1 c1
set c2 c2
set c3 c3
set c4 c4

set printbinary_singleline 0

eq 0 0 macroskip

at printbinary
	ld ra 0

	set data c0 
	set n c1 
	set bit c2 
	set pointer c3

	set s rv_system_arg1
	ru rv_auipc s buffer
	ri rv_imm rv_add s s buffer
	ri rv_imm rv_add n data 0  del data
	ri rv_imm rv_add pointer s 0

	at loop
		ri rv_imm rv_and bit n 1
		ri rv_imm rv_add bit bit '0'
		rs rv_store rv_sb pointer bit 0
		ri rv_imm rv_add pointer pointer 1
		ri rv_imm rv_srl n n 1
		rb rv_branch rv_bne n 0 loop 

	del loop 
	del n 

	lt 0 printbinary_singleline skip_newline
		ri rv_imm rv_add bit 0 newline
		rs rv_store rv_sb pointer bit 0
		ri rv_imm rv_add pointer pointer 1
	at skip_newline del skip_newline
	del bit

	ri rv_imm rv_add rv_system_number 0 rv_system_write
	ri rv_imm rv_add rv_system_arg0 0 stdout
	rr rv_reg rv_add rv_system_arg2 pointer s rv_signed
	ri rv_ecall 0 0 0 0 

	del pointer 
	del s
	eq 0 0 ra del ra

at nl
	ld ra 0
	ri rv_imm rv_add rv_system_number 0 rv_system_write
	ri rv_imm rv_add rv_system_arg0 0 stdout
	ru rv_auipc rv_system_arg1 newline_string
	ri rv_imm rv_add rv_system_arg1 rv_system_arg1 newline_string
	ri rv_imm rv_add rv_system_arg2 0 1
	ri rv_ecall 0 0 0 0 

	eq 0 0 ra del ra

at exit
	ld ra 0
	ri rv_imm rv_add rv_system_number 0 rv_system_exit
	ri rv_imm rv_add rv_system_arg0 0 0
	ri rv_ecall 0 0 0 0
	eq 0 0 ra del ra









at writestring
	ld ra 0
	ri rv_imm rv_add rv_system_number 0 rv_system_write
	ri rv_imm rv_add rv_system_arg0 0 stdout
	ru rv_auipc rv_system_arg1 c0
	ri rv_imm rv_add rv_system_arg1 rv_system_arg1 c0
	ri rv_imm rv_add rv_system_arg0 0 c1
	ri rv_ecall 0 0 0 0
	eq 0 0 ra del ra


at putc
	ld ra 0
	ri rv_imm rv_add rv_system_number 0 rv_system_write
	ri rv_imm rv_add rv_system_arg0 0 stdout
	ru rv_auipc rv_system_arg1 c0
	ri rv_imm rv_add rv_system_arg1 rv_system_arg1 c0
	ri rv_imm rv_add rv_system_arg0 0 1
	rs rv_store rv_sb rv_system_arg1 c1 0
	ri rv_ecall 0 0 0 0
	eq 0 0 ra del ra

at getc
	ld ra 0
	ri rv_imm rv_add rv_system_number 0 rv_system_read
	ri rv_imm rv_add rv_system_arg0 0 stdin
	ru rv_auipc rv_system_arg1 c0
	ri rv_imm rv_add rv_system_arg1 rv_system_arg1 c0
	ri rv_imm rv_add rv_system_arg2 0 1
	ri rv_ecall 0 0 0 0
	ri rv_load rv_lbu c1 rv_system_arg1 0
	eq 0 0 ra del ra


at nl
	ld ra 0
	ri rv_imm rv_add rv_system_number 0 rv_system_write
	ri rv_imm rv_add rv_system_arg0 0 stdout
	ru rv_auipc rv_system_arg1 newline_string
	ri rv_imm rv_add rv_system_arg1 rv_system_arg1 newline_string
	ri rv_imm rv_add rv_system_arg2 0 1
	ri rv_ecall 0 0 0 0 
	do ra del ra

at li
	ld ra 0
	lt c1 0000_0000_0000_0000_0000_0000_0000_0000_1 skip_abort
		do -1 str "argument to li did not fit in a 32-bit number."
	at skip_abort del skip_abort
	set immediate c1
	set bit immediate nand bit 0000_0000_0001 
	nor bit 0 sd bit 1101 
	set a immediate sd a 0011 add a bit del bit 
	nand a 1111_1111_1111_1111_1111 nor a 0
	set b immediate del immediate 
	nand b 1111_1111_1111 nor b 0
	set source 0
	eq 0 a s  ru rv_lui c0 a set source c0
	at s del s
	eq 0 b s  ri rv_imm rv_add c0 source b
	at s del s
	del source
	del a del b
	do ra del ra






at macroskip del macroskip











do s str "
	this is how you would comment out a piece of code, or put a comment in the middle of the source code.
" at s del s





set myvar 1

set c0 myvar 
set c1 1100_1111
do li
set c1 01
set c2 11
set c3 001
do printbinary


set c0 myvar 
set c1 0000_1111_0001_0111_1111_1
do li
set c1 01
set c2 11
set c3 001
do printbinary



set c 1

at main

	set c0 buffer
	set c1 c
	do getc

	ri rv_imm rv_add 01 0 'q'
	rb rv_branch rv_beq c 01 done
	
	ri rv_imm rv_add 01 0 0001
	rb rv_branch rv_bne c 01 skip
		set c0 delete
		set c1 011
		do writestring
		rj rv_jal 0 mainloop
	at skip del skip

	set c0 buffer
	set c1 c
	do putc	
	rj rv_jal 0 mainloop
at done

do exit

at delete
	str "delete"

at newline_string 
	emit 1 newline

at buffer 
	set i 0 at loop
	emit 1 0 incr i
	lt i 0000001 loop 
	del loop del i







eoi




1202507185.170003 a  print-binary   prime number program   to test out the risc-v backend more fully
hopefully it goes well lolll

1202507185.182228
just got the print binary at runtime   functionality working i think!! its quite involved lollll










	(1202507196.165657   current state:  we just realized that    we need to unify   rt labels    and ct values    beucaes we are unable to passs a reference to a runtime label into a ct argument, for a macro call.... big issue i think....)




