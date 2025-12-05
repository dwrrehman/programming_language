(this is a compiletime program to write a simple 
interactive cli interface using this language!
uses routines defined in useful.s written on 1202507141.143155 by dwrr)

file library/foundation.s
file library/ascii.s
file library/useful.s
ct do interpretted

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

	ne c 'h' s do cthello do loop at s del s
	ne c 'n' s do ctnl do loop at s del s
	ne c 'o' s do ctclearscreen do loop at s del s

	set c0 stdout set c1 buffer set c2 n do ctwrite

	do loop at done
do ctexit











