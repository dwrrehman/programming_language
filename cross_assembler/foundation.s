def1 settarget x ret

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


.

	settarget = isa_count,

	zero, incr, decr, not_, 

	set, add, sub, mul, div_, rem, 

	and_, or_, eor, si, sd, 

	lt, eq, ge, ne, do_, 

	at, ld, st,

.


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


