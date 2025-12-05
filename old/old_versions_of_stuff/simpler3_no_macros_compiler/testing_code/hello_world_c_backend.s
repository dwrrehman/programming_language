(writing the hello world program for the language's c backend, 
to test out the new compile time system call interface 
and string length and macro functionality.)

file library/foundation.s ct
st compiler_target c_arch nat
st compiler_format c_source nat
st compiler_should_overwrite true nat 
st compiler_should_debug true nat 


do skip

operation stringlength 01 1 at stringlength ct
	ld ra compiler_arg0 nat
	ld im compiler_arg1 nat
	ld d compiler_arg2 nat deref d
	ld k compiler_arg3 nat
	st compiler_get_length k nat
	ld d compiler_arg0 nat
	ct do ra del ra 
	del im del d del k


operation writestring 1 1 at writestring ct
	ld ra compiler_arg0 nat
	ld im compiler_arg1 nat
	ld l compiler_arg2 nat
	deref l rt 
	set c_system_number c_system_write
	set c_system_arg0 stdout
	la c_system_arg1 l
	ct stringlength n l
	rt set c_system_arg2 n del n
	system
	ct do ra del ra del im del l

at skip del skip
rt 
writestring string
set c_system_number c_system_exit
set c_system_arg0 0011
system halt
at string string "hello world
"
















