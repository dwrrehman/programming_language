#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

int main(void) {
	const int limit = 0x80000;
	uint64_t count = 0;
	for (uint64_t i = 2; i < limit; i++) {
		for (uint64_t j = 2; j < i; j++)
			if (i % j == 0) goto composite;
		count++; composite:;
	}
	syscall(SYS_exit, count);
}







/*

#define _GNU_SOURCE
int
main(int argc, char *argv[])
{
    pid_t tid;
    tid = syscall(SYS_gettid);
    tid = syscall(SYS_tgkill, getpid(), tid);
}

*/
