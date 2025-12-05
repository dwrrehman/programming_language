// a simple c program to see what the value of certain ioctl related constants are 
// for the purposes of recreating it in our languages standard library 
// for use in making our text editor in our own language! 
//
//  written on 1202508214.060639 by dwrr
//
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include <termios.h>
#include <sys/ioctl.h>
#include <stddef.h>

typedef uint64_t nat;

int main(void) {
	printf("TIOCGETA = 0x%llx\n", (nat) TIOCGETA);
	printf("TIOCSETA = 0x%llx\n", (nat) TIOCSETA);
	printf("TIOCGWINSZ = 0x%llx\n", (nat) TIOCGWINSZ);

	printf("sizeof(struct termios) = %llu\n", 
		(nat) sizeof(struct termios)
	);

	printf("sizeof(tcflag_t) = %llu\n", (nat) sizeof(tcflag_t));
	printf("NCCS = %llu\n", (nat) NCCS);

	//printf("NCCS + 4 * sizeof(nat) = %llu\n", (nat) NCCS + (nat) 4 * (nat) sizeof(tcflag_t));

	printf("offsetof(struct termios, c_iflag) = %llu\n", 
		(nat) offsetof(struct termios, c_iflag)
	);

	printf("offsetof(struct termios, c_oflag) = %llu\n", 
		(nat) offsetof(struct termios, c_oflag)
	);

	printf("offsetof(struct termios, c_cflag) = %llu\n", 
		(nat) offsetof(struct termios, c_cflag)
	);

	printf("offsetof(struct termios, c_lflag) = %llu\n", 
		(nat) offsetof(struct termios, c_lflag)
	);

	printf("offsetof(struct termios, c_cc) = %llu\n", 
		(nat) offsetof(struct termios, c_cc)
	);

	printf("sizeof(cc_t) = %llu\n", (nat) sizeof(cc_t));



	printf("	BRKINT = 0x%llx\n", (nat) BRKINT);
	printf("	ICRNL = 0x%llx\n", (nat) ICRNL);
	printf("	INPCK = 0x%llx\n", (nat) INPCK);
	printf("	ISTRIP = 0x%llx\n", (nat) ISTRIP);
	printf("	IXON = 0x%llx\n", (nat) IXON);

	printf("	OPOST = 0x%llx\n", (nat) OPOST);

	printf("	ECHO = 0x%llx\n", (nat) ECHO);
	printf("	ICANON = 0x%llx\n", (nat) ICANON);
	printf("	IEXTEN = 0x%llx\n", (nat) IEXTEN);
	printf("	ISIG = 0x%llx\n", (nat) ISIG);

	printf("	VMIN = 0x%llx\n", (nat) VMIN);
	printf("	VTIME = 0x%llx\n", (nat) VTIME);


/*
       struct winsize {
           unsigned short ws_row;
           unsigned short ws_col;
           unsigned short ws_xpixel;  /* unused */
           unsigned short ws_ypixel;  /* unused */
       };

*/
}





/*







1202508214.060539
running this program, we get:

ioctl_testing: ./run

TIOCGETA = 0x40487413
TIOCSETA = 0x80487414
TIOCGWINSZ = 0x40087468

sizeof(struct termios) = 72
sizeof(tcflag_t) = 8

NCCS = 20

offsetof(struct termios, c_iflag) = 0
offsetof(struct termios, c_oflag) = 8
offsetof(struct termios, c_cflag) = 16
offsetof(struct termios, c_lflag) = 24
offsetof(struct termios, c_cc) = 32

sizeof(cc_t) = 1

	BRKINT = 0x2
	ICRNL = 0x100
	INPCK = 0x10
	ISTRIP = 0x20
	IXON = 0x200

	OPOST = 0x1

	ECHO = 0x8
	ICANON = 0x100
	IEXTEN = 0x400
	ISIG = 0x80

	VMIN = 0x10
	VTIME = 0x11


*/








