zero 0 
zero 1 incr 1
zero 2 incr 2 incr 2

set a 2   set b 2

eq a b skip
	sc 0   a 0 0  0 0 0 
	do done
at skip
	sc 1   a a a  0 0 0
	do done
at done



