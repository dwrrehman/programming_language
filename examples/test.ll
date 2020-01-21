define fastcc i32 @foo() {
entry:
    %t = call ccc i32 @foo()
    ret i32 %t
}
