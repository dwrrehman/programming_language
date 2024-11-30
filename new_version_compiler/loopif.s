zero 0
zero 1 incr 1
set r 0 
at loop
	lt r 1 if
	set r 0 do done
	at if set r 1
	at done do loop

