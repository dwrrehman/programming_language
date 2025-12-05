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
	ld ra 0 nat set i 0
	at l rt emit 1 0 ct 
	add i 1 lt i c0 l
	del l del i
	do ra del ra


at skip_macros del skip_macros 
rt


set a0 welcome
set a1 011
do writestring

at loop
	do getc set c a0 
	set backspace 1111_111
	ne c backspace skip
		set a0 delete
		set a1 111
		do writestring
		do loop
	at skip del skip
	set 'q' 1000_111
	ne c 'q' skip
		do done
	at skip del skip
	set a0 c  do putc
	do loop
at done
do exit

at buffer set c0 000001 do allocate

at welcome str "hello
"

at delete str "delete
"











































