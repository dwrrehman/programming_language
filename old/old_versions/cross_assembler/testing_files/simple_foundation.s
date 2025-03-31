def1 settarget x ret
def2 ri x n obs x ret
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

