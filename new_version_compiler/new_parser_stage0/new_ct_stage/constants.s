eor 0 0
at _unused
set 1 _unused
div 1 1

set 2 1 add 2 1
set 3 2 add 3 1
set 4 3 add 4 1
set 5 4 add 5 1
set 6 5 add 6 1
set 7 6 add 7 1
set 8 7 add 8 1
set 9 8 add 9 1
set 10 9 add 10 1

set 16 8 si 16 1
set 32 16 si 32 1
set 64 32 si 64 1
set 128 64 si 128 1
set 256 128 si 256 1
set 512 256 si 512 1
set 1024 512 si 1024 1

set _ 0

eoi

this file is just a file to hold basic constants
this will be in the standard library probably. 


written on 1202502123.130918 
by dwrr

this is a test of the language ct system which doesnt  have incr or zero as those were replaced by computational immediates used with add and set. 
the computational immediates  1 and 0 are obtained using:

	eor x x
	at l
	set y l 
	div y y

x will be 0 and y will be 1. 

