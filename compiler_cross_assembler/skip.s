(testing out skipping in the cfg traversal 
1202504071.013841 dwrr)


ct set debug 1



do skip

	(a lot of code here)

	set x 101
	set y 001
	add x y 
	mul y x
	si x 1
	sd y 01

at skip

add y 0001 

add x 1

sc 0 x   0 0 0 0 0 

halt


