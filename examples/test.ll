









joy ()
joy h












join decl ((type) (i)) 11 0

join decl ((lazy
    ((lazy-t) (type) (i))
) (type) (i)) 13 0

join decl (
(define
    ((define-s) (name) (i))
    ((define-t) (type) (i))
    ((define-d) (lazy define-t) (type) (i))
    ((define-extern) (nat) (i))
) (i)) 18 0

join decl (
(load
    ((load-filename) (name) (i))
    ((load-type))
) (load-type)) 21 0

(load (file) i)





















join decl ((type) (i)) 6 0

join decl ((lazy
    ((lazy-t) (type) (i))
) (type) (i)) 8 0

join decl (
(define
    ((define-s) (name) (i))
    ((define-t) (type) (i))
    ((define-d) (lazy define-t) (type) (i))
    ((define-extern) (nat) (i))
) (i)) 13 0

join decl (
(load
    ((load-filename) (name) (i))
    ((load-type) (type) (i))
) (load-type) (type) (i)) 16 0


join decl ((unit) (type) (i)) 55 0
join decl ((i1) (type) (i)) 17 0
join decl ((i8) (type) (i)) 18 0
join decl ((i16) (type) (i)) 19 0
join decl ((i32) (type) (i)) 20 0
join decl ((i64) (type) (i)) 21 0
join decl ((i128) (type) (i)) 22 0
join decl ((x86mmx) (type) (i)) 23 0
join decl ((f16) (type) (i)) 24 0
join decl ((f32) (type) (i)) 25 0
join decl ((f64) (type) (i)) 26 0
join decl ((f128) (type) (i)) 27 0
join decl ((label) (type) (i)) 52 0
join decl ((metadata) (type) (i)) 53 0
join decl ((token) (type) (i)) 54 0
join decl ((string) (pointer 0 i8) (type) (i)) 56 0

join decl ((pointer
        ((pointer-addrspace) (nat) (i))
        ((pointer-type) (type) (i))
    ) (type) (i)) 30 0
    
join decl ((vector
          ((vector-width) (nat) (i))
          ((vector-type) (type) (i))
      ) (type) (i)) 33 0
    
join decl ((scalable
        ((scalable-width) (nat) (i))
        ((scalable-type) (type) (i))
    ) (type) (i)) 36 0
    
join decl ((array
          ((array-size) (nat) (i))
          ((array-type) (type) (i))
      ) (type) (i)) 39 0

join decl ((opaque
      ((opaque-s) (name) (i))
  ) (type) (i)) 41 0


join decl ((struct
    ((struct-s) (name) (i))
    ((struct-d) (name) (i))
    ((struct-extern) (nat) (i))
) (type) (i)) 45 0

join decl ((packed
    ((packed-s) (name) (i))
    ((packed-d) (name) (i))
    ((packed-extern) (nat) (i))
) (type) (i)) 49 0

join decl ((function
    ((function-type) (name) (i))
) (type) (i)) 51 0

join decl ((unreachable) (unit) (type) (i)) 57 0
join decl ((ret void) (unit) (type) (i)) 58 0

join decl ((ret
    ((ret-t) (type) (i))
    ((ret-v) (ret-t) (type) (i))
) (unit) (type) (i)) 61 0

join decl ((create label
    ((label-name) (name) (i))
) (unit) (type) (i)) 63 0

join decl ((jump
    ((jump-label) (name) (i))
) (unit) (type) (i)) 65 0

join decl ((br
    ((br-cond) (i1) (type) (i))
    ((br-1) (name) (i))
    ((br-2) (name) (i))
) (unit) (type) (i)) 69 0

type










