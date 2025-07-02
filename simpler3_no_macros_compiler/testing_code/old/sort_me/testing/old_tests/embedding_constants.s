(
	1202504082.212846 dwrr 
	a program to test out embedding constants into the rt instructions. 
)


ct set debug 1

ct set myconstant 1001
set x myconstant
add myconstant 101
set y myconstant
lt x myconstant skip
	halt
at skip

ge myconstant y skip

sc 0 0 0 0  0 0 0 


sc myconstant 0 0 0  0 0 0 

sc x 0 y myconstant  0 0 myconstant 

sc x x y x x  x x 


ld myconstant myconstant myconstant

ld x myconstant myconstant

(ld myconstant x myconstant            invalid)

(ld myconstant myconstant x            invalid)


st myconstant myconstant myconstant

st x myconstant myconstant

st myconstant x myconstant

(st myconstant myconstant x            invalid)



(testing out storing to ct memory, based on a label! 
to edit the .text section at compiletime. )

ct set sizeof_byte 1 

ct set mybytevalue 101

ct st label mybytevalue sizeof_byte

halt

at label

(we should put an instruction here, to allocate like, a set of bytes... hmmm)
(such as like: )

	(allocate 101)     (which allocates 5 bytes, lets say!)



ct halt



















