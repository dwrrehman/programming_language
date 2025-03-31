.
	a simple file which tests compiletime looping. 1202502285.011202 dwrr
.

lf foundation.s

print 4

set i 0
set count 14

at loop
	print i 
	incr i 
	lt i count loop

print 0
print 0
print 0

eoi
