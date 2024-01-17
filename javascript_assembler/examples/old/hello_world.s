. examples/foundation . include

enable 
comment.0192840918234




202310146.180557:

	this is a comment! 

	this file is for testing out the system calls and the compiletime system in the language. 


	kinda messy right now because we are debugging strings at the moment. 



	ctnop

	r5 r6 r1 ctadd           [ 5, 6, 1  _ ]                             registers[1] = registers[6] + registers[5];
				            ^      acount = 4
	
	r4 r3 r2 acountincr ctsub              [4, 3, 2, 1 ]
				              ^              acount = 0


	r4 r1 r6 ctmul

	




	stuff r0 r0 r1 ctadd stuff

	stuff






comment.0192840918234 
disable



macros

	65 
		15 r1 r2 +
		2 r2 r2 *
		5 r2 r2 +
		r1 ctzero
		r1 r2 r1 +
		r1
	65 

	address r0 address

	gotoaddress r0 ctat r0 gotoaddress

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

	then
		string emit remove
	then

	newline 10 db newline

	\n then newline \n

	mystring2
		enable
			" Hello there from space! this is my cool string. yay. "  \n
			" i still can't beleive this works lol... " \n
		disable
	mystring2

	noshift r0 noshift


	macroname otherfile.s macroname

lives
	r11 ctset 
	4 r11 r11 ctadd
	r11 ctat
	address r21 ctld4
	r11 r22 ctld4
	r21 r22 r23 ctsub
	4 r23 r23 ctmul
	r23 address ctst4
lives 

endmacros




1 noshift r0 movzx   	enable ;41234 	1 = stdout 						;41234 disable

gotoaddress r1 adr  	enable ;41235 	string address  points to after __text section. 	;41235 disable

100 noshift r2 movzx   	enable ;41236 	100 characters in string. 				;41236 disable

4 noshift r16 movzw  	enable ;41237 	4 = write() system call. 				;41237 disable

svc

45 noshift r0 movzx
1 noshift r16 movzw
svc

address lives 

	mystring2





