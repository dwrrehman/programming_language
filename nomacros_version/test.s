

this is the library file for the langauge.

zero counter
set 0 counter incr counter
set 1 counter incr counter
set 2 counter incr counter
set 3 counter incr counter
set 4 counter incr counter
set 5 counter incr counter
set 6 counter incr counter
set 7 counter incr counter
set 8 counter incr counter
set 9 counter incr counter
set 10 counter incr counter
set 11 counter incr counter
set 12 counter incr counter

set 65 10
mul 65 6
add 65 5

set systemcall.debug 	0 
set systemcall.exit 	1
set systemcall.execve 	2
set systemcall.fork 	3
set systemcall.wait 	4
set systemcall.openat 	5
set systemcall.close 	6
set systemcall.write 	7
set systemcall.read 	8
set systemcall.ioctl 	9
set systemcall.poll 	10
set systemcall.lseek 	11
set systemcall.munmap 	12
set systemcall.protect 	13
set systemcall.mmap 	14

do jumpoverfunctions

at printnumber
	this is a function used for debugging numbers to the screen.

	incr ret do ret
	sc systemcall.debug arg 0 0 0 0 0 
at jumpoverfunctions


--------------------- applcation code ------------------------------

set limit 5    the limit on the number of iterations of the outter loop. 

set arg 12 at ret do printnumber       just testing out the printing system.
set arg 10 at ret do printnumber       (same here)

zero i
at loop
	set arg i at ret do printnumber
	zero a
	at inner
		set arg a at ret do printnumber
		incr a
		lt a 3 inner
	incr i
	lt i limit loop


set pointer _process_stackpointer
set begin pointer

set char 65
zero l
st pointer char 1   incr pointer incr l incr char
st pointer char 1   incr pointer incr l incr char
st pointer char 1   incr pointer incr l incr char
st pointer char 1   incr pointer incr l incr char
st pointer char 1   incr pointer incr l incr char
st pointer 10 1     incr pointer incr l incr char

set fd 1
set buffer begin
set length l
at ret do write 

set fd 0
set length 1
at ret do read

set exitcode 5 at ret do exit


at exit
	sc systemcall.exit exitcode 0 0 0 0 0
	incr ret do ret

at read
	sc systemcall.read fd buffer length 0 0 0
	incr ret do ret

at write
	sc systemcall.write fd buffer length 0 0 0
	incr ret do ret











