define fastcc i32 @foo() {
entry:
    %t = call ccc i32 @foo()
    ret i32 %t
}

join (declare ((unit) (_)))
join ( declare ((( (x) (unit) (_) )) (_)) )
join ( declare ((( (x) (_) )) (unit) (_)) )
join ( def ((((first) (unit) (_)) ((second) (unit) (_))) (unit) (_)) unit second)
name name
