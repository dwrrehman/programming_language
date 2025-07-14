// testing out to see if we can pin particular memory  at particular addresses, so that the compiler can acess the user code's memory allocations.
// 1202507141.043119  by dwrr

#include <sys/mman.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

static const int prot_read = 1;
static const int prot_write = 2;

//set map_private 01
//set map_anonymous 0000_0000_0000_1
//set map_failed -1

// set map_fixed ...???

int main(void) {

	void* address = (void*) 0x0000000ffffff000;
	address = mmap(
			address, 
			4096, 
			prot_read | prot_write, 
			MAP_PRIVATE | MAP_FIXED | MAP_ANON,
			-1, 
			0
		);

	if (address == (void*) -1) {
		perror("mmap"); 
		printf("errno = %u\n", errno); 
		exit(1);
	}

	printf("obtained memory 4k region starting at address: \n\taddress = %p\n", address);

}













