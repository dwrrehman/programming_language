
. 
	this is a test of the language 
	to idendify system call numbers
	at compiletime, and thus detect
	when the control flow graph
	terminates at certain points.
.

zero 0 
set 1 0 incr 1
set 2 1 incr 2
set 3 2 incr 3
set 4 3 incr 4
set 5 4 incr 5

set system_exit 0
set system_read 1
set system_write 2
set system_open 3
set system_close 4


set n system_exit
sc n  0 0 0  0 0 0