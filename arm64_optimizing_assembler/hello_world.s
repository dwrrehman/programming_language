include library/foundation

stringliteral 
	an arm64 assembly hello world program!
	written on 202410314.132919 by dwrr.
stringliteral ignore

mov r0 stdout
adr r1 string_begin
adr r2 string_end
add inv r2 r2 r1
mov r16 system_write
svc

mov r0 101
mov r16 system_exit
svc

at string_begin stringliteral
hello there from space! 
this is my hello world program lol.
what do you think of it?
 stringliteral at string_end

eoi 

















emit size1  char_H char_I char_space char_A
emit size1  char_B char_C char_D char_E
emit size1  char_F char_G char_H char_newline