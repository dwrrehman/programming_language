#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

int main(void) {

	printf("PROT_READ = %llx\n", PROT_READ);
	printf("PROT_WRITE = %llx\n", PROT_WRITE);
	printf("MAP_PRIVATE = %llx\n", MAP_PRIVATE);
	printf("MAP_ANONYMOUS = %llx\n", MAP_ANONYMOUS);
	printf("MAP_FAILED = %llx\n", MAP_FAILED);

}




