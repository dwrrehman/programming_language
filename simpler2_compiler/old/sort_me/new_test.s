(a test to see if reducing ct control flow works!"
1202504115.150740 dwrr)

set count 11

set width count
mul width 01

rt output 0
set output 0

lt count 001 skip
	add output 1

at skip
	mul count 0
	sub output count

(at loop)
	add output width
	lt output 0101 skip

si output count

halt
