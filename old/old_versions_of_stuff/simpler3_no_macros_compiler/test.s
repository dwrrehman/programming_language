(this is a program to test out using my 36-key keyboard to 
write compile-time-executed code, using our compiler!
its going well so far i think yayyyy
written on 1202507152.190824 by dwrr)

file library/foundation.s
file library/ascii.s
file library/useful.s
ct do interpretted

set zero 0 set one 1 
set two one add two one
set arg arg

do skipmacros

at print
	ld ra zero nat

	set c0 arg
	do ctbinary
	do ctnl

	do ra del ra

at skipmacros del skipmacros

set limit two si limit two si limit limit


set i zero
set count zero





do ctexit






















(set arg limit do print)
(set c0 limit do ctdebug)

(at outter
	set e two
	set arg i do print
	add i one
	lt i limit outter del outter
	del i del limit)


(

(do cthello set c0 count do ctbinary do ctnl)




things which i need to add to make the keyboard useable for programming, possibly

modifiers
	. control key
	. command key
	. shift key

punctuation
	. slash key
	. exclamation mark and question mark
	. NUMBERSSSS
	. colon key
	. parens on different layer
	. apostrophe!!!
)















































( 1202507152.190532


set max_input_size 0000001
set c0 1 do ctallocatepages set buffer c0

at loop

	set p buffer
	st p ':' byte add p 1
	st p space byte add p 1
	set c0 stdout set c1 buffer set c2 01 do ctwrite
	set c0 stdin set c1 buffer set c2 max_input_size do ctread
	set n c0

	ld c buffer byte 


	eq c 'q' done
	eq c newline loop

	ne c 'h' s do cthello do loop at s del s	ne c 'n' s do ctnl do loop at s del s
	ne c 'o' s do ctclearscreen do loop at s del s

	set c0 stdout set c1 buffer set c2 n do ctwrite

	do loop at done
)





