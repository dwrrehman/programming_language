. 
	a foundational file 
	for the language which defines the ct system, and basic constants. 
	written on 1202502285.010908 dwrr
. 

def0 halt ret
def1 setarch x ret

def1 zero x obs x ret
def1 incr x ret
def1 decr x ret
def1 not x ret

def2 set x y obs x ret
def2 add x y ret
def2 sub x y ret
def2 mul x y ret
def2 div x y ret
def2 rem x y ret

def2 and x y ret
def2 or  x y ret
def2 eor x y ret
def2 si  x y ret
def2 sd  x y ret

def3 lt x y l obs l ret
def3 ge x y l obs l ret
def3 eq x y l obs l ret
def3 ne x y l obs l ret
def1 do l obs l ret
def1 at l obs l ret

def3 ld dest address size ret
def3 st address source size ret

def1 print x ret

zero 0
set 1 0 incr 1
set 2 1 incr 2
set 3 2 incr 3
set 4 3 incr 4
set 5 4 incr 5
set 6 5 incr 6
set 7 6 incr 7
set 8 7 incr 8
set 9 8 incr 9
set 10 9 incr 10
set 11 10 incr 11
set 12 11 incr 12
set 13 12 incr 13
set 14 13 incr 14
set 15 14 incr 15
set 16 15 incr 16

set 20 10 si 20 1
set 25 20 add 25 5
set 50 25 si 50 1
set 75 50 add 75 25

set 32 16 si 32 1 
set 64 32 si 64 1 
set 128 64 si 128 1 
set 256 128 si 256 1 
set 512 256 si 512 1 
set 1024 512 si 1024 1 
set 2048 1024 si 2048 1 
set 4096 2048 si 4096 1 

set 100 10 mul 100 10
set 1000 100 mul 1000 10
set 10000 1000 mul 10000 10
set 100000 10000 mul 100000 10
set 1000000 100000 mul 1000000 10

set 31 32 decr 31
set 30 31 decr 30

set true 1
set false 0

set nat 64
set byte 8

set compiletime_only 0
set arm64_arch 1
set arm32_arch 2
set rv64_arch 3
set rv32_arch 4

eoi

