define fastcc i32 @foo() {
entry:
    %t = call fastcc i32 @foo()
    ret i32 %t
}

declare i32 @putchar(i32 %char)
