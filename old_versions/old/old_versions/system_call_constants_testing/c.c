// mmap example from online 

// 1202503167.201907 dwrr
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#define SIZE 4096

int main() {

/*    void *addr;

    addr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    snprintf(addr, SIZE, "Hello, mmap!");
    printf("%s\n", (char *)addr);

    if (munmap(addr, SIZE) == -1) {
        perror("munmap");
        return 1;
    }


*/

	printf("PROT_READ = %u\n", PROT_READ);
	printf("PROT_WRITE = %u\n", PROT_WRITE);
	printf("MAP_PRIVATE = %u\n", MAP_PRIVATE);
	printf("MAP_ANONYMOUS = %u\n", MAP_ANONYMOUS);
	printf("MAP_FAILED = %d\n", MAP_FAILED);
    return 0;
}


