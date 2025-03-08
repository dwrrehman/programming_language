. testing out the new instructions ive coded up 
	1202503075.145837 dwrr
.

lf arm64.s

mov 15 10 shift_none mov_type_zero width64

set should_link 1
jmp should_link skip_over
nop

set cond cond_always
set should_incr 0
set should_inv 0
csel 3 4 5 is_unsigned_less should_incr should_inv width64

at skip_over
svc

halt

