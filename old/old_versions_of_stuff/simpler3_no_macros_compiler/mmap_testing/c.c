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
static const int map_private = 2;
static const int map_anonymous = 0x1000;

int main(void) {

	void* address = mmap(
			NULL, 
			8 * 4096, 
			prot_read | prot_write, 
			map_private | map_anonymous,
			-1, 
			0
		);

	if (address == (void*) -1) {
		perror("mmap"); 
		printf("errno = %u\n", errno); 
		exit(1);
	}

	printf("obtained memory region starting at address: \n\taddress = %p\n", address);

}













