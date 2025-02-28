lf foundation.s

. 
	a file to define the machine instructions for the arm 64-bit architecture.
.

settarget arm64_arch
set target_architecture arm64_arch

rt nzvc 4

def0 nop ret
def0 svc ret
def0 return ret

def4 addrlsl d n m k obs d ret



eoi