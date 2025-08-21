file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s

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

set begin 	0001
set end   	1001
set cursor   	0101
set temp   	1101

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


at main

set c0 welcome set c1 welcome.length writestring

set prot prot_read 
or prot prot_write
set flags map_private 
or flags map_anonymous

set allocationsize pagesize

mov a6_number a6_mmap shiftnone movzero 
mov a6_arg0 0 shiftnone movzero
mov a6_arg1 allocationsize shiftnone movzero
mov a6_arg2 prot shiftnone movzero
mov a6_arg3 flags shiftnone movzero
mov a6_arg4 0 shiftnone movnegate
mov a6_arg5 0 shiftnone movzero
svc

addi begin a6_arg0 0 0 0 0

set c0 debugstring set c1 debugstring.length writestring

addi a6_arg3 begin 0 0 0 0
set c0 a6_arg3 printnumber

addi a6_sp a6_sp 1 true 0 subtract
memi mem_store 8_bytes end a6_sp 0
memi mem_load 8_bytes end a6_sp 0
addi a6_sp a6_sp 1 true 0 0

set c0 0 exit

at welcome
	str "going to allocate a page of memory! 
" set c0 welcome getstringlength
set welcome.length c0

at debugstring
	str "printing --->  x0 : "
set c0 debugstring getstringlength 
set debugstring.length c0 


at zerodigit str "0"
at onedigit  str "1"
at newlinestring emit 1 newline



eoi

a runtime version of the screen based editor,
i'm going to try to use the arm64 backend to remake the editor nowwww
hopefully this goes welllll

written on 1202508203.051022 by dwrr















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























