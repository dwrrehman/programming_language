. testing unrolling a loop via compiletime execution. .

lf constants.s

set hello 0

ct i zero i 
at loop
	incr i
	add hello i
	lt i 5 loop

mul hello hello

eoi



