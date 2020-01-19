
define fastcc i32 @foo() {
entry:
    %t = call fastcc i32 @foo()
    ret i32 %t
}

