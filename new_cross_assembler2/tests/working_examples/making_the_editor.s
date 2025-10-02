file library/core.s
file library/useful.s
file library/ascii.s

str "executable_editor_output" set_output_name riscv_hex

set begin 	1
set end 	01
set cursor 	11 

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

at assign
	ld ra 0
	ri r_imm r_add c0 c1 0
	eq 0 0 ra del ra 
	lt 0 0 assign

	
at display
	ld ra 0 
	set c0 ra function_begin

	set screen 	11111
	set text 	01111
	set col_count 	10111
	set row_count 	00111
	set r		11011
	set c		01011
	set imm		10011
	set nl_imm 	00011
	set tab_imm 	11101
	set space_imm 	01101
	
	set c0 screen set c1 screenbuffer la
	set c0 text set c1 begin assign

	set c0 nl_imm set c1 newline li
	set c0 tab_imm set c1 tab li
	set c0 space_imm set c1 space li

	rb r_branch r_bgeu text end done at loop

		ri r_load r_lbu c text 0 set c0 text increment		
		rb r_branch r_beq c nl_imm printnewline
		rb r_branch r_beq c tab_imm printtab

		rs r_store r_sb screen c 0 set c0 screen increment
		set c0 col_count increment
		rb r_branch r_bltu text end loop
		rj r_jal 0 done

	at printnewline
		set c0 row_count increment
		set c0 col_count setzero
		rs r_store r_sb screen nl_imm 0 set c0 screen increment
		rb r_branch r_bltu text end loop
		rj r_jal 0 done

	at printtab
		set c0 imm set c1 0001 li
		rr r_remu_op1 r_remu_op2 r col_count imm r_remu_op3
		rr r_reg r_add imm imm r r_signed
		set c0 r setzero
		at l 
			rs r_store r_sb screen space_imm 0
			set c0 screen increment
			set c0 r increment
		rb r_branch r_bltu r imm l del l
		rb r_branch r_bltu text end loop del loop

at done del done

	set c0 r_number set c1 r_write li
	set c0 r_arg0 set c1 stdout li
	set c0 r_arg1 set c1 clearscreen_string la

	rr r_reg r_add r_arg2 screen r_arg1 r_signed
	ri r_ecall 0 0 0 0

	del screen del text
	del col_count del row_count 
	del r del c del imm
	del nl_imm del tab_imm del space_imm

	function_end
	eq 0 0 ra del ra
	lt 0 0 display

at main 

set c0 begin set c1 textbuffer la
set c0 end set c1 welcomestring_end la
set c0 cursor set c1 begin assign

set delete_char 	1111_1110
set backspace_char 	0001

at loop
	set c 011 set command 101
	
	display
	set c0 c readchar

	set c0 command set c1 'Q' li
	rb r_branch r_beq c command done

	set c0 command set c1 backspace_char li
	rb r_branch r_beq c command delete

	set c0 command set c1 delete_char li
	rb r_branch r_beq c command delete

	rs r_store r_sb end c 0 set c0 end increment

	del c del command

	rj r_jal 0 loop

at delete del delete
	rb r_branch r_beq begin end skip
		set c0 end decrement
	at skip del skip
	rj r_jal 0 loop
	
del loop
at done del done

set c0 exitstring set c1 exitstring.length writestring
set c0 101 exit

at buffer emit 0001 0

at exitstring
str "terminating the editor! (written in the assembler)
" set c0 exitstring getstringlength 
set exitstring.length c0

at clearscreen_string
emit 1 escape str "[H" emit 1 escape str "[2J"
set c0 clearscreen_string getstringlength 
set clearscreen_string.length c0



at screenbuffer
	set max_screen_size 0000_0000_0000_1
	zero i 
	at l 
		emit 1 0 
		incr i 
		lt i max_screen_size l del max_screen_size
	del l del i


at textbuffer
str 
"welcome to Daniel Rehman's website!
----------------------------------------
the interface to this website is my shell and text editor, 
which has been assembled using my cross-assembler and 
which is running inside of a RISC-V virtual machine 
running in the browser.

this document explains how to use the editor, and how to 
issue commands to explore the website further.

to begin, we'll start by navigating around the document:

   n u p i   left/down/up/right navigation (workman)
   j i o ;   left/down/up/right navigation (qwerty)

once you are able to move the cursor around, you can try 
to go into insert mode via:

   t   insert mode via (workman)
   f   insert mode via (qwerty)

you can also perform page up and down movements using:

   h m   page-up/page-down navigation (workman)
   d c   page-up/page-down navigation (qwerty)

more of the tutorial coming soon! WIP.
"
at welcomestring_end
	set max_text_size 0000_1
	zero i 
	at l 
		emit 1 0 
		incr i 
		lt i max_text_size l del max_text_size
	del l del i



eoi

making the editor using this assembler! running in the riscv virtual machine! 
by dwrr
1202508107.012112















(set c0 1 
set c1 1111_0111_1011_0011__1101_0101_1001_0001 
li

rr r_reg r_add 0 0 0 0 

eoi)


