(writing a simple shell for my website!
using the risc-v backend
written on 1202507104.191823 by dwrr)

file library/foundation.s ct

st compiler_target rv32_arch nat
st compiler_format hex_array nat
st compiler_should_overwrite true nat 


(st compiler_should_debug true nat   (temporary))


rt
	set a0 a0
	set a1 a1
ct
	set c0 c0
	set c1 c1

do skip_macros


at writestring
	ld ra 0 nat rt
	set rv_system_number rv_system_write
	set rv_system_arg0 stdout
	set rv_system_arg1 a0
	set rv_system_arg2 a1
	sc ct do ra del ra

at getc
	ld ra 0 nat rt
	set rv_system_number rv_system_read
	set rv_system_arg0 stdin
	set rv_system_arg1 buffer
	set rv_system_arg2 1
	sc
	ld a0 buffer byte
	ct do ra del ra

at putc
	ld ra 0 nat rt
	set rv_system_number rv_system_write
	set rv_system_arg0 stdout
	set rv_system_arg1 buffer
	set rv_system_arg2 1
	st rv_system_arg1 a0 byte
	sc ct do ra del ra

at exit
	ld ra 0 nat
	rt 
	set rv_system_number rv_system_exit
	set rv_system_arg0 0
	sc halt
	ct 
	do ra del ra


at allocate
	ld ra 0 nat 
	set i 0 at l rt emit 1 0 ct 
	add i 1 lt i c0 l
	del l del i
	do ra del ra


at printbinary
	ld ra 0 nat

	rt set n a0
	at loop
		set bit n
		and bit 1
		add bit 000011

		set a0 bit do putc  del bit
		sd n 1
		ne n 0 loop
		del loop del n

	set a0 0101 do putc
	ct do ra del ra


at skip_macros del skip_macros 
rt

ct set space 000001 rt




set a0 welcome
set a1 111
do writestring

at loop
	do getc set c a0 

	set backspace 0001
	ne c backspace skip
		set a0 delete
		set a1 111
		do writestring
		do loop
	at skip del skip

	set 'Q' 1000_111 sub 'Q' 000001
	set '~' 0111_111

	ne c 'Q' skip
		do done
	at skip del skip

	lt c space s
	lt '~' c s
		set a0 c do putc
		do loop
	at s del s

	ne c 0101 s
		set a0 c do putc
		do loop
	at s del s

	ne c 1001 s
		set a0 c do putc
		do loop
	at s del s

	set a0 c  do printbinary
	do loop

at done

set a0 0010100111 do printbinary

set a0 closing
set a1 00001
do writestring

do exit

at buffer set c0 000001 do allocate

at welcome str "hello!
"
at closing str "[process exited]"

at delete str "delete
"











































