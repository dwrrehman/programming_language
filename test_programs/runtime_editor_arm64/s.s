file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/ascii.s

str "run" set_output_name arm64


(stdlib stuff
----------------------------------------------)

(arm64 stuff: )

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

(----------------------------------------------)

eq allones allones s str q 

	this is a comment basically that only uses letters yayyy 

q at s del s


set begin 	0001
set end   	1001
set cursor   	0101
set inputchar   1101


eq 0 0 main

at printnumber           (destroys data c0 register)
	ld ra 0
	set data c0
	set c0 ra function_begin
	at loop
		tbz data 0 off true
		adr a6_arg1 zerodigit 0
		jmp 0 writedigit
	at off 
		adr a6_arg1 onedigit 0
	at writedigit
		mov a6_number a6_write shiftnone movzero 
		mov a6_arg0 stdout shiftnone movzero 
		mov a6_arg2 1 shiftnone movzero
		svc
		set subtract 1 set setflags 1
		addr 0 setflags data a6_zero data shift_decr 1
		bc is_nonzero loop 
	
	del data del loop del off del writedigit

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



at armreadchar
	ld ra 0
	set outputchar c0
	set c0 ra function_begin

	mov a6_number a6_read shiftnone movzero 
	mov a6_arg0 stdin shiftnone movzero 
	addi a6_arg1 a6_sp 0 0 0 0
	mov a6_arg2 1 shiftnone movzero 
	svc

	(memi mem_store 8_bytes end a6_sp 0)
	memi mem_load 1_byte outputchar a6_sp 0

	del outputchar 
	function_end
	eq 0 0 ra del ra
	lt 0 0 armreadchar
	

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
	
at main

addi a6_sp a6_sp 1 true 0 subtract

set c0 1 allocatepages

addi begin a6_arg0 0 0 0 0
addi end begin 0 0 0 0
addi cursor begin 0 0 0 0













set command_tcgets allones

set command_tcsets allones







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
















at loop
	(display)
	set c0 prompt set c1 prompt.length writestring
	set c0 inputchar armreadchar	
	
	addi 11111 inputchar 'q' 0 1 1
	bc is_equal done

	addi 11111 inputchar 1111_111 0 1 1
	bc is_equal delete

	set c0 debugstring set c1 debugstring.length writestring
	set c0 inputchar printnumber
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

at endingstring str "editor: terminating program, goodbye!
" set c0 endingstring getstringlength 
set endingstring.length c0 


eoi





a runtime version of the screen based editor,
i'm going to try to use the arm64 backend to remake the editor nowwww
hopefully this goes welllll

written on 1202508203.051022 by dwrr

got mmap working and the display function started on 1202508214.043954



(dealing with the stack:)
addi a6_sp a6_sp 1 true 0 subtract
memi mem_store 8_bytes end a6_sp 0
memi mem_load 8_bytes end a6_sp 0
addi a6_sp a6_sp 1 true 0 0




memi mem_store 8_bytes end a6_sp 0
memi mem_load 8_bytes end a6_sp 0










	( calling mmap(0, page_size, prot_read | prot_write, map_private | map_anonymous, -1, 0) )








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























