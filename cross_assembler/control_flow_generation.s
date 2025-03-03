. 1202503031.011856 dwrr a control flow test with macros, . 
. aiming to test out the rt generation of cf via macros. . 
lf arm64.s

set cond_lt 1
set cond_zero 2
set cond_carry 3

bc cond_carry address
	nop
	nop
at address

svc

eoi



