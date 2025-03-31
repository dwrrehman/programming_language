. a simple exit program to test if the rt generation is working as it should. . 

lf arm64.s

def0 gen
	at loop
	mov q 5  0 0 0 
	svc
	bc 1 loop
	ret
gen
gen
ri q 3
set i 0 at inner 
print i incr i 
bc 1 skip mov q i 0 0 0 at skip 
lt i 4 inner
halt eoi




