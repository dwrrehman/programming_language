file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/ascii.s

str "run" set_output_name arm64


(stdlib stuff
----------------------------------------------)

(arm64 stuff: )

set subtract 1 
set setflags 1

set mem_store 		0
set mem_load 		1
set mem_load_signed 	01

set 1_byte  0 
set 2_bytes 1 
set 4_bytes 01
set 8_bytes 11

set shiftnone 	0

set movzero 	01   (mov_type_zero, rename this in the stdlib...)
set movnegate 	0
set movkeep 	11

set is_nonzero is_not_equal
set is_zero is_equal

set bitwise_and 	0 
set bitwise_or 		1
set bitwise_eor 	01
set bitwise_and_setflags 11

set shift_incr 0
set shift_decr 1
set shift_decr_signed 01


(macos stuff: )

set pagesize 		0000_0000_0000_1
set prot_read 		1
set prot_write 		01
set map_private 	01
set map_anonymous 	0000_0000_0000_1
set map_failed 		allones




(ioctl requests)

set request_window_size  		0001_0110_0010_1110__0001_0000_0000_0010
set request_get_terminal_attributes	1100_1000_0010_1110__0001_0010_0000_0010
set request_set_terminal_attributes	0010_1000_0010_1110__0001_0010_0000_0001


(termios structure)

set terminal_attributes_inputflags	0 
set terminal_attributes_outputflags	1
set terminal_attributes_controlflags	01
set terminal_attributes_localflags	11
set terminal_attributes_controlcharacters 001


(termios member bitfield flags)

set echoinput 	0001
set canonicalmode 0000_0000_1




(----------------------------------------------)




eq allones allones s str q 

	this is a comment basically that only uses letters yayyy 

q at s del s



set begin 	0001
set end   	1001
set cursor   	0101
set inputchar   1101
set size 	0011
set temp 	1011

zero function_call_history
set function_labels_base 0000_0000_001

eq 0 0 main

at printnumber           (destroys data c0 register)
	ld ra 0
	set data c0
	set c0 ra function_begin

	set function_base function_labels_base
	add function_base function_call_history

	set off_save function_base
	set writedigit_save function_base incr writedigit_save

	add function_call_history 01 ( label count ) 

	at loop
		ld off off_save
		tbz data 0 off true
		adr a6_arg1 zerodigit 0
		ld writedigit writedigit_save
		jmp 0 writedigit

	at off
		st off_save off
		adr a6_arg1 onedigit 0

	at writedigit
		st writedigit_save writedigit
		mov a6_number a6_write shiftnone movzero 
		mov a6_arg0 stdout shiftnone movzero 
		mov a6_arg2 1 shiftnone movzero
		svc
		addr 0 setflags data a6_zero data shift_decr 1
		bc is_nonzero loop 
	
	del data 
	del loop 
	del off 
	del writedigit

	del off_save
	del writedigit_save
	del function_base
	
	mov a6_number a6_write shiftnone movzero 
	mov a6_arg0 stdout shiftnone movzero 
	adr a6_arg1 newlinestring 0
	mov a6_arg2 1 shiftnone movzero
	svc

	function_end
	eq 0 0 ra del ra
	lt 0 0 printnumber


at display
	ld ra 0
	set c0 ra function_begin
	
	set c0 clearscreen set c1 clearscreen.length writestring

	(...)

	function_end
	eq 0 0 ra del ra
	lt 0 0 display





at armwritechar
	ld ra 0
	set char c0
	set c0 ra function_begin

	memi mem_store 1_byte char a6_sp 0

	mov a6_number a6_write shiftnone movzero 
	mov a6_arg0 stdout shiftnone movzero 
	addi a6_arg1 a6_sp 0 0 0 0
	mov a6_arg2 1 shiftnone movzero 
	svc

	del char
	function_end
	eq 0 0 ra del ra
	lt 0 0 armwritechar
	




at armreadchar
	ld ra 0
	set char c0
	set c0 ra function_begin

	mov a6_number a6_read shiftnone movzero 
	mov a6_arg0 stdin shiftnone movzero 
	addi a6_arg1 a6_sp 0 0 0 0
	mov a6_arg2 1 shiftnone movzero 
	svc

	memi mem_load 1_byte char a6_sp 0

	del char 
	function_end
	eq 0 0 ra del ra
	lt 0 0 armreadchar
	

at emitnl
	ld ra 0
	set c0 ra function_begin
	emit 1 newline
	function_begin
	eq 0 0 ra del ra
	lt 0 0 emitnl


at allocatepages
	ld ra 0
	set count c0
	set c0 ra function_begin

	set prot prot_read 
	or prot prot_write
	set flags map_private 
	or flags map_anonymous	
	set allocationsize pagesize
	mul allocationsize count
	
	mov a6_number a6_mmap shiftnone movzero 
	mov a6_arg0 0 shiftnone movzero
	mov a6_arg1 allocationsize shiftnone movzero
	mov a6_arg2 prot shiftnone movzero
	mov a6_arg3 flags shiftnone movzero
	mov a6_arg4 0 shiftnone movnegate
	mov a6_arg5 0 shiftnone movzero
	svc

	del count del prot del flags
	del allocationsize
	function_end
	eq 0 0 ra del ra
	lt 0 0 allocatepages
	


at a6li 
	ld ra 0
	set destination c0
	set immediate c1
	set c0 ra function_begin

	set i0 immediate sd i0 0000_00 and i0 1111_1111_1111_1111
	set i1 immediate sd i1 0000_10 and i1 1111_1111_1111_1111
	set i2 immediate sd i2 0000_01 and i2 1111_1111_1111_1111
	set i3 immediate sd i3 0000_11 and i3 1111_1111_1111_1111
	
	mov destination i0 00 movzero
	mov destination i1 10 movkeep
	mov destination i2 01 movkeep
	mov destination i3 11 movkeep

	del i0
	del i1
	del i2
	del i3
	del destination
	del immediate

	function_end
	eq 0 0 ra del ra
	lt 0 0 a6li




at main

addi a6_sp a6_sp 1 true 0 subtract

mov a6_number a6_ioctl shiftnone movzero 
mov a6_arg0 stdin shiftnone movzero
set c0 a6_arg1 set c1 request_get_terminal_attributes a6li 
addi a6_arg2 a6_sp 0 0 0 0
svc

memi mem_load 8_bytes temp a6_sp terminal_attributes_localflags

set t size
set flags echoinput
or  flags canonicalmode
mov size t shiftnone movzero
orr bitwise_and temp temp true t 0 0 

memi mem_store 8_bytes temp a6_sp terminal_attributes_localflags

mov a6_number a6_ioctl shiftnone movzero 
mov a6_arg0 stdin shiftnone movzero
set c0 a6_arg1 set c1 request_set_terminal_attributes a6li 
addi a6_arg2 a6_sp 0 0 0 0
svc





mov a6_number a6_ioctl shiftnone movzero 
mov a6_arg0 stdin shiftnone movzero
set c0 a6_arg1 set c1 request_window_size a6li 
addi a6_arg2 a6_sp 0 0 0 0
svc

set c0 windowsizestring set c1 windowsizestring.length writestring

memi mem_load 2_bytes size a6_sp 0
set c0 size printnumber

set c0 windowsizestring set c1 windowsizestring.length writestring

memi mem_load 2_bytes size a6_sp 1
set c0 size printnumber



set c0 1 allocatepages

addi begin a6_arg0 0 0 0 0
addi end begin 0 0 0 0
addi cursor begin 0 0 0 0



at loop
	(display)
	(set c0 prompt set c1 prompt.length writestring)
	set c0 inputchar armreadchar
	
	addi 11111 inputchar 'q' 0 1 1
	bc is_equal done

	addi 11111 inputchar 1111_111 0 1 1
	bc is_equal delete

	set c0 debugstring set c1 debugstring.length writestring
	set c0 inputchar printnumber

	(set c0 inputchar armwritechar)

	jmp 0 loop

at delete
	set c0 deletestring set c1 deletestring.length writestring	
	jmp 0 loop

at done 
set c0 endingstring set c1 endingstring.length writestring
del done del loop

addi a6_sp a6_sp 1 true 0 0

set c0 0 exit

at zerodigit str "0"
at onedigit  str "1"
at newlinestring emit 1 newline

at clearscreen
emit 1 escape str "[H"
emit 1 escape str "[2J"
set c0 clearscreen getstringlength
set clearscreen.length c0

at debugstring str "user input the character: "
set c0 debugstring getstringlength 
set debugstring.length c0 

at deletestring str "delete!
" set c0 deletestring getstringlength 
set deletestring.length c0 

at prompt str ":ready: "
set c0 prompt getstringlength 
set prompt.length c0 

at windowsizestring str "window size: (rows, columns)" emitnl
set c0 windowsizestring getstringlength
set windowsizestring.length c0 

at endingstring str "terminating program!" emitnl
set c0 endingstring getstringlength
set endingstring.length c0 


at terminalsettingsstring str "terminal settings: local flags: "
set c0 terminalsettingsstring getstringlength
set terminalsettingsstring.length c0 



eoi





a runtime version of the screen based editor,
i'm going to try to use the arm64 backend to remake the editor nowwww
hopefully this goes welllll

written on 1202508203.051022 by dwrr

got mmap working and the display function started on 1202508214.043954



got multiple macro call instance with forward branches in macro body   working on 1202508225.230651












( 1202508225.234146

	NOTE: its TOTALLYYYY possible to dynamically form the string table,  ie, put the strings along side where they are printed, 

		via the fact that we can put strings on the outptubytes    and then remove them from there 


			and then store the data to compiletime,

			keeping track of the location of the string, relative to the string table,


			and then we just have a function that is called at the end to generate the string table and set the values of all labels, etc i think 

		so yeah! nice lol


)










(set c0 terminalsettingsstring 
set c1 terminalsettingsstring.length 
writestring 
set c0 temp printnumber)

















(

TIOCGETA = 0x40487413

TIOCSETA = 0x80487414

TIOCGWINSZ = 0x40087468


sizeof(struct termios) = 72

sizeof(tcflag_t) = 8


NCCS = 20


offsetof(struct termios, c_iflag) = 0
offsetof(struct termios, c_oflag) = 8
offsetof(struct termios, c_cflag) = 16
offsetof(struct termios, c_lflag) = 24
offsetof(struct termios, c_cc) = 32


sizeof(cc_t) = 1

	BRKINT = 0x2
	ICRNL = 0x100
	INPCK = 0x10
	ISTRIP = 0x20
	IXON = 0x200

	OPOST = 0x1

	ECHO = 0x8
	ICANON = 0x100
	IEXTEN = 0x400
	ISIG = 0x80

	VMIN = 0x10
	VTIME = 0x11



)














(
get 

mov a6_number a6_ioctl shiftnone movzero 
mov a6_arg0 stdin shiftnone movzero 
mov a6_arg1 command_tcsets shiftnone movzero 
mov a6_arg2 STRUCT_POINTER_HERE shiftnone movzero
svc


modify


set 

mov a6_number a6_ioctl shiftnone movzero 
mov a6_arg0 stdin shiftnone movzero 
mov a6_arg1 command_tcgets shiftnone movzero 
mov a6_arg2 STRUCT_POINTER_HERE shiftnone movzero 
svc


)




















(dealing with the stack:)
addi a6_sp a6_sp 1 true 0 subtract
memi mem_store 8_bytes end a6_sp 0
memi mem_load 8_bytes end a6_sp 0
addi a6_sp a6_sp 1 true 0 0




memi mem_store 8_bytes end a6_sp 0
memi mem_load 8_bytes end a6_sp 0










	( calling mmap(0, page_size, prot_read | prot_write, map_private | map_anonymous, -1, 0) )








(
set command_tcgets allones

set command_tcsets allones
)





(mov a6_number a6_exit shiftnone movzero
svc)









(addi a6_zero data 0 1 1 0 )
(bc is_nonzero loop del loop)
(cbz data loop 1)


(
mov i 0 shiftnone movzero
ori bitwise_and_setflags a6_zero copy 1 1 0
bc is_zero off
)





























1202508214.060539
running this program, we get:

ioctl_testing: ./run

TIOCGETA = 0x40487413
TIOCSETA = 0x80487414
TIOCGWINSZ = 0x40087468

sizeof(struct termios) = 72
sizeof(tcflag_t) = 8

NCCS = 20

offsetof(struct termios, c_iflag) = 0
offsetof(struct termios, c_oflag) = 8
offsetof(struct termios, c_cflag) = 16
offsetof(struct termios, c_lflag) = 24
offsetof(struct termios, c_cc) = 32

sizeof(cc_t) = 1

	BRKINT = 0x2
	ICRNL = 0x100
	INPCK = 0x10
	ISTRIP = 0x20
	IXON = 0x200

	OPOST = 0x1

	ECHO = 0x8
	ICANON = 0x100
	IEXTEN = 0x400
	ISIG = 0x80

	VMIN = 0x10
	VTIME = 0x11

