file library/core.s
file library/useful.s

rv32_hex

set var 1
set c0 var set c1 0000_0000_0000_0000_0001  li
set c0 var printbinary
set limit var del var
set i 011 set sum 101
set c0 i set c1 0 li
set c0 sum set c1 0 li

at loop
	rr r_reg r_add sum sum i 0
	set c0 sum printbinary
	(set c0 001 readchar)
	ri r_imm r_add i i 1
	rb r_branch r_bltu i limit loop 
set c0 0 exit

at buffer
	emit 0001 0
	emit 0001 0
	emit 0001 0
	emit 0001 0
	emit 0001 0
	emit 0001 0
	emit 0001 0
	emit 0001 0
eoi

adding some numbers together, and printing it out in binary!
basically for testing out using the new useful.s stdlib file, 
containing both li and la! 

(going to start writing the pico 2w firmware code pretty soon!)
written on 1202507292.175105 by dwrr
