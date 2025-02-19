lf constants.s

rt x 3
rt y 3
rt z 3


set z 5
set y 2
set x y
si x 9
add x z

eoi











drafting some arm64 instructions for ins sel:
branching is hard... hmmm


set s m
si_imm s k
set d n
sub d s

	-->	subsrlsl





/*
set s m
si_imm s k
lt n s label

	-->	subs_srlsl     wronggggggg
*/




set d Y
set s m
si_imm s k
lt n s label
set d X
at label


	-->	subs_srlsl ZXR, N, M << K    
		csel_ge X Y





set s m
si_imm s k
lt n s label

	-->	subs_srlsl ZXR, N, M << K
		bc.lt LABEL







so what i am seeing is that 




















