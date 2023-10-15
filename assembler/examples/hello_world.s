. examples/foundation . include

enable comment.0192840918234

202310146.180557:

	this is a comment! 

	this file is for testing out the system calls and the compiletime system in the language. 


	kinda messy right now because we are debugging strings at the moment. 



comment.0192840918234 disable



macros

	65 
		15 r1 r2 +
		2 r2 r2 *
		5 r2 r2 +
		r1 ctzero
		r1 r2 r1 +
		r1
	65 

	address
		28
	address

	last    r3 r3 +  r3 dw  	last
	next    r3 r3 +  8 r3 r3 ctshl  next

	mystring
		r3 ctzero

		'L' next
		'L'  next
		'E'  next
		'H'  last

		r3 ctzero

		'I' next
		'H'  next
		'\n'  next
		'O'  last
		
	mystring

	then string emit remove then
	newline 10 db newline

	mystring2
		enable 
			" Hello there from space! this is my cool string. yay. " 

				then newline

			" i still can't beleive this works lol... " 

				then newline
		disable
	mystring2

endmacros



1 r0 r0 movzx   	enable ;41234 	1 = stdout 						;41234 disable

address r1 adr  	enable ;41235 	string address  points to after __text section. 	;41235 disable

100 r0 r2 movzx   	enable ;41236 	4 characters in string. 				;41236 disable

4 r0 r16 movzw  	enable ;41237 	4 = write() system call. 				;41237 disable

svc

2 r0 r0 movzx
1 r0 r16 movzw
svc


address

	mystring2





