; my cool module here.
; this is code here.

%t = type { i32, i16 }

define i32 @sum(i32 %a, i32 %b) {  	
	%x = add i32 %a, %b
	ret i32 %x
}

@.str = private unnamed_addr constant [13 x i8] c"hello world\0A\00"

declare i32 @puts(i8* nocapture) nounwind

define i32 @main(i32 %a, i32 %b) { 
    %c = getelementptr [13 x i8],[13 x i8]* @.str, i64 0, i64 0
    call i32 @puts(i8* %c)
    ret i32 0
}

