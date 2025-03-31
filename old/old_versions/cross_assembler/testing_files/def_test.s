lf arm64.s
. a test of the def system and some simple rt instructions . 

set x 5
def myvar x
mul myvar 2
print x

nop
nop 
nop
svc 

eoi

