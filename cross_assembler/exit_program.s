lf arm64.s

. a simple runtime exit program for arm64  
  written on 1202502285.185510 dwrr 
.

def0 run 
	ri n 16 
	mov n 10 0 0 0
	br n 0 0 
	svc 
	ret 

run
. br n 0 0 . 


at loop
	nop svc
	bc 0 loop

	eoi
































eoi

lf arm64.s

. a simple runtime exit program for arm64  written on 1202502285.030850 dwrr .

rt m nat 
rt n nat

set k 5

addrlsl d m n k
ri d 1

svc

eoi


