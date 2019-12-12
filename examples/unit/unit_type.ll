; ModuleID = 'unit_type.n'
source_filename = "unit_type.n"
target datalayout = "e-m:o-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-apple-darwin19.0.0"

%"(_1) (_)" = type {}

define i32 @main(i32, i8**) {
entry:
  call void @llvm.donothing()
  ret i32 0
}

; Function Attrs: nounwind readnone
declare void @llvm.donothing() #0

define %"(_1) (_)" @"() (_1) (_)"() {
  ret %"(_1) (_)" zeroinitializer
}

attributes #0 = { nounwind readnone }
