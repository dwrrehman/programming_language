// calling bsdthread_create   in order to do multithreading in assembly eventually, without pthreads. 
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/syscall.h>
#include <pthread.h>


/*


		user_addr_t 
			bsdthread_create(
				user_addr_t func, 
				user_addr_t func_arg, 
				user_addr_t stack, 
				user_addr_t pthread, 
				uint32_t flags
			) NO_SYSCALL_STUB; } 


(360 syscall number)


		bsdthread_create(myfunction, NULL, malloc(1024), &pthread_t, 0);

lets try coding it up in C first!!!

*/



static void* myfunction(void* unused) {

	while (1) {
		printf("hello world!\n");
		sleep(1);
	}

	return unused;
}




int main(void) {


 //       syscall(SYS_write, 1, "Hello via syscall!\n", 19);

//        printf("SYS_write number: %ld\n", (long)SYS_write);

	void* stack = malloc(4096 * 4);
	void* thread_info = malloc(1024);
	
        int r = syscall(360, &myfunction, 0, stack, thread_info, 0x7000080DB000);
	if (r < 0) { perror("syscall"); exit(1); }

        printf("SYS_write number: %ld\n", (long)360);


	//pthread_t pthread;
	//pthread_create(&pthread, 0, &myfunction, 0);

	while (1) { } ;

	exit(0);

}
