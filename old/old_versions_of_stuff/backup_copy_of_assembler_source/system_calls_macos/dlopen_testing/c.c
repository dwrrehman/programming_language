// figuring out how to call an external function...
// written on 1202507303.232801 by dwrr
// to learn from this executable file / object file:
//
//  first do this:
//
//     clang -c c.c -Os
// 
//  then do this:
//
//     otool -htvxVlLrITRMHGCjd -dyld_info -dyld_opcodes c.o
//     objdump -D c.o
//     nm -ap c.o 
// 
//

#include <dlfcn.h>

int main(void) {
	void* p = dlsym((void*) -2, "puts");
	((void (*)(const char*))p)("hello");
}


	//void* handle = dlopen("/usr/lib/libSystem.B.dylib", RTLD_NOW);
	//printf("%p\n", RTLD_DEFAULT); // prints out 0xfffffffffffffffe    in other words, -2 
	//return 0;




