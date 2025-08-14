file library/core.s
file library/useful.s
file library/ascii.s

riscv_hex

(set c0 1 
set c1 1111_0111_1011_0011__1101_0101_1001_0001 
li

rr r_reg r_add 0 0 0 0 

eoi)

set textlength 1001 (should be called end)
set text 0001 (should be called begin)
(set cursor 111 (cursor indexed into text))

eq 0 0 main

at clear
	ld ra 0
	set c0 clearscreen_string set c1 clearscreen_string.length writestring
	eq 0 0 ra del ra 
	lt 0 0 clear

at setzero
	ld ra 0
	ri r_imm r_add c0 0 0
	eq 0 0 ra del ra
	lt 0 0 setzero

at increment
	ld ra 0
	ri r_imm r_add c0 c0 1
	eq 0 0 ra del ra
	lt 0 0 increment

at decrement
	ld ra 0
	ri r_imm r_add c0 c0 1111_1111_1111
	eq 0 0 ra del ra
	lt 0 0 decrement

at assign
	ld ra 0
	ri r_imm r_add c0 c1 0
	eq 0 0 ra del ra 
	lt 0 0 assign
	

at display
	ld ra 0 

	set c 011
	set b 101
	set pointer 001

	clear
	set c0 b set c1 buffer la
	set c0 pointer set c1 text assign

	at loop	
		rb r_branch r_bgeu pointer textlength done

		(set c0 invert_string set c1 invert_string.length writestring)

		ri r_load r_lbu c pointer 0
		rs r_store r_sb b c 0

		ri r_imm r_add r_number 0 r_write
		ri r_imm r_add r_arg0 0 stdout
		ri r_imm r_add r_arg1 b 0
		ri r_imm r_add r_arg2 0 1
		ri r_ecall 0 0 0 0

		(set c0 reset_string set c1 reset_string.length writestring)
		
		set c0 pointer increment
		rj r_jal 0 loop del loop
at done del done
	del pointer del c del b
	eq 0 0 ra del ra
	lt 0 0 display

at main 


set c0 text set c1 textlabel la
set c0 textlength set c1 text assign
(set c0 cursor set c1 text assign)

set c0 string0 set c1 string0.length writestring

set backspace_char 1111_1110

at loop
	set c 011 set command 101
	
	display
	set c0 c readchar

	set c0 command set c1 'Q' li
	rb r_branch r_beq c command done

	set c0 command set c1 backspace_char li
	rb r_branch r_beq c command delete

	rs r_store r_sb textlength c 0
	(set c0 cursor increment)
	set c0 textlength increment

	del c del command

	rj r_jal 0 loop

at delete del delete
	rb r_branch r_beq text textlength skip
		set c0 textlength decrement
		(set c0 cursor decrement)
	at skip del skip
	rj r_jal 0 loop
	
del loop
at done del done

set c0 101 exit




at string0
str "welcome to the editor!
written in the assembler. 
"
set c0 string0 getstringlength 
set string0.length c0


at clearscreen_string

emit 1 escape str "[H" 
emit 1 escape str "[2J"

set c0 clearscreen_string getstringlength 
set clearscreen_string.length c0



at invert_string
emit 1 escape str "[7m"
set c0 invert_string getstringlength
set invert_string.length c0

at reset_string
emit 1 escape str "[0m"
set c0 reset_string getstringlength
set reset_string.length c0




at buffer
	emit 0001 0
at textlabel
	zero i 
	at l 
		emit 0001 0 
		incr i 
		lt i 01 l 
	del l del i

eoi

making the editor using this assembler! running in the riscv virtual machine! 
by dwrr
1202508107.012112
