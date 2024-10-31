include library/constants


comment 
	an arm64 assembly hello world program!
	written on 202410314.132919 by dwrr.

comment 

def length 0011

mov r0 stdout
adr r1 string
mov r2 length
mov r16 system_write
svc

mov r0 101
mov r16 system_exit
svc

at string  

emit size1  char_H char_I char_space char_A
emit size1  char_B char_C char_D char_E
emit size1  char_F char_G char_H char_newline
