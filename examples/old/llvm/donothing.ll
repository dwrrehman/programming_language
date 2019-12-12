; ModuleID = '/Users/deniylreimn/Documents/projects/n3zqx2l/examples/test.n'
source_filename = "/Users/deniylreimn/Documents/projects/n3zqx2l/examples/test.n"
target triple = "x86_64-apple-darwin19.0.0"

define i32 @main(i32, i8**) {
entry:
  call void @llvm.donothing()
  ret i32 0
}

; Function Attrs: nounwind readnone
declare void @llvm.donothing() #0

attributes #0 = { nounwind readnone }