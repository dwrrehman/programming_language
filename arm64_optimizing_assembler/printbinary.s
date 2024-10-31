stringliteral 
	a program to test out printing the contents of a register, in binary, 
	starting from the hello world program.
	wrote on 202410314.144754 by dwrr

lr
 |  | 0001 | 0010 | 1111 | 1111 | 1001 | 0111 | 0000 | 0000 | 1000 | 0000 | 0000 | 0000 | 0000 | 0000 | 0000 | 0000

sp
 |  | 0000 | 1010 | 1010 | 1101 | 0010 | 0110 | 1111 | 0110 | 1000 | 0000 | 0000 | 0000 | 0000 | 0000 | 0000 | 0000



stringliteral ignore


include library/foundation


mov keep r5 0100.1110.0010.1101


do link printsomething 			ignore
addi r5 r31 0		ignore

def i r3  mov i
at loop
	tbz not i 0 skip2  tbz not i 1 skip2
		do link printseperator
	at skip2

	tbz not r5 0 skip  do link print0  do done
	at skip  do link print1  at done

	addi i i 1
	add r5 zr r5 down 1
	addi inv flags zr i 0000001
	if not carry loop
do link printnl

mov r0
mov r16 system_exit
svc


at printseperator
	mov r0 stdout
	adr r1 string2_begin
	adr r2 string2_end
	add inv r2 r2 r1
	mov r16 system_write
	svc
	br return lr


at print0
	mov r0 stdout
	adr r1 string0_begin
	adr r2 string0_end
	add inv r2 r2 r1
	mov r16 system_write
	svc
	br return lr

at print1
	mov r0 stdout
	adr r1 string1_begin
	adr r2 string1_end
	add inv r2 r2 r1
	mov r16 system_write
	svc
	br return lr

at printnl
	mov r0 stdout
	adr r1 stringnl_begin
	adr r2 stringnl_end
	add inv r2 r2 r1
	mov r16 system_write
	svc
	br return lr


at string0_begin 
stringliteral 0 stringliteral 
at string0_end 

at string1_begin 
stringliteral 1 stringliteral 
at string1_end 

at string2_begin 
stringliteral  |  stringliteral 
at string2_end 

at stringnl_begin 
stringliteral 
 stringliteral 
at stringnl_end 

eoi


