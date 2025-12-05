. 1202502285.033844 dwrr a loop rt program . 
lf arm64.s

def0 dosomecoolstuff 
	svc svc 
	ret

set i 0
at loop
	print i 
	dosomecoolstuff
	incr i
	lt i 5 loop
eoi



