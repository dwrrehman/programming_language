#include <unistd.h>
#include <sys/syscall.h>

int main (void) {
	syscall(1, 5);
}