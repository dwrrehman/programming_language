(testing out the copy propagation implmentation, 
1202504152.095140 dwrr)

rt undefined 0

set b undefined
add b 001

set a 11
set c b
set d c

set n a 
add n b
add n c
add n d

sc

halt