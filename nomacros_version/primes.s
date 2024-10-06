a program to calculate the prime counting 
function, for a given number "limit".
writtten on 1202410034.133810

lf library/foundation

set limit 1000 mul limit 100
set i 2
zero primecount

at loop
	set j 2
	set halfi i sd halfi 1
	at inner
		lt halfi j prime
		set r i rem r j eq r 0 composite
		incr j
		do inner
	at prime
	set arg i at ret do printnumber
	add primecount 1
	at composite
	incr i 
	lt i limit loop


set arg 0 at ret do printnumber
set arg primecount at ret do printnumber
set exitcode 12 at ret do exit

eoi













	do skip	
	set j 2
	at inner
		ge j limit prime
		set r i rem r j eq r 0 composite
		incr j do inner
	at prime
	set arg i at ret do printnumber
	at composite
	at skip





this stuff is ignored. 

at loop
ge i i loop


set n 12 
zero i 
at loop
	set arg 65536 at ret do printnumber
	incr i 
	lt i n loop

