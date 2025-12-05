zero i zero n zero k

at loop
	ge i n out

	lt k i skip
		incr k
	at skip

	eq n k else
		zero k
		do done
	at else
		zero i 
		do done
	at done

	do loop	
at out


lt k k skip2
	incr k
at skip2

