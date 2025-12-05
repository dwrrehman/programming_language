file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/ascii.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s

str "run" set_output_name arm64

set c0 hello set c1 hello.length writestring
set c0 0 exit

at hello
str "hello world!" emitnl
set c0 hello getstringlength
set hello.length c0


eoi

