

this is the library file for the langauge.
written on 1202410023.182419 by dwrr.

def counter 
zero counter
def 0 set 0 counter incr counter
def 1 set 1 counter incr counter
def 2 set 2 counter incr counter
def 3 set 3 counter incr counter
def 4 set 4 counter incr counter
def 5 set 5 counter incr counter
def 6 set 6 counter incr counter
def 7 set 7 counter incr counter
def 8 set 8 counter incr counter
def 9 set 9 counter incr counter
def 10 set 10 counter incr counter
def 11 set 11 counter incr counter
def 12 set 12 counter incr counter
def 13 set 13 counter incr counter
def 14 set 14 counter incr counter

udf counter

def systemcall.debug 
set systemcall.debug 	0 

def systemcall.exit 
set systemcall.exit 	1

def systemcall.execve 
set systemcall.execve 	2

def systemcall.fork
set systemcall.fork 	3

def systemcall.wait
set systemcall.wait 	4

def systemcall.openat
set systemcall.openat 	5

def systemcall.close
set systemcall.close 	6

def systemcall.write
set systemcall.write 	7

def systemcall.read
set systemcall.read 	8

def systemcall.ioctl
set systemcall.ioctl 	9

def systemcall.poll
set systemcall.poll 	10

def systemcall.lseek
set systemcall.lseek 	11

def systemcall.munmap
set systemcall.munmap 	12

def systemcall.mprotect
set systemcall.mprotect  13

def systemcall.mmap
set systemcall.mmap 	14


def skip
do skip

def debug
at debug
	sc systemcall.debug arg 0 0 0 0 0 
	incr lr do lr

def exit
at exit
	sc systemcall.exit exitcode 0 0 0 0 0
	incr lr do lr

def read
at read
	sc systemcall.read fd buffer length 0 0 0
	incr lr do lr

def write
at write
	sc systemcall.write fd buffer length 0 0 0
	incr lr do lr

at skip
udf skip



def i zero i incr i incr i
udf i zero i 