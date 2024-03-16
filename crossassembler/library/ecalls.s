0 ctf	
	this file defines how particular registers 
	are used as per the risc-v abi, as well as
	some commonly used system call numbers.
0 ctstop

00000 cc.zr define
10000 cc.lr define
01000 cc.sp define
11000 cc.gp define

00100 cc.tp define
10100 cc.t0 define
01100 cc.t1 define
11100 cc.t2 define

00010 cc.fp define
10010 cc.s1 define
01010 cc.a0 define
11010 cc.a1 define

00110 cc.a2 define
10110 cc.a3 define
01110 cc.a4 define
11110 cc.a5 define

00001 cc.a6 define
10001 cc.a7 define
01001 cc.s2 define
11001 cc.s3 define

00101 cc.s4 define
10101 cc.s5 define
01101 cc.s6 define
11101 cc.s7 define

00011 cc.s8 define
10011 cc.s9 define
01011 cc.s10 define
11011 cc.s11 define

00111 cc.t3 define
10111 cc.t4 define
01111 cc.t5 define
11111 cc.t6 define





0 ctf 		macos system call numbers:		0 ctstop


1 		systemcall.exit 	define
01		systemcall.fork 	define
11 		systemcall.read 	define
001 		systemcall.write 	define
101 		systemcall.open 	define 
011		systemcall.close	define

0011		systemcall.chdir	define

100001		systemcall.access	define
110111		systemcall.execve	define




0 ctf 
others to add soon:

	197	systemcall.mmap 	define

	199	systemcall.lseek	define

	189	systemcall.fstat	define

	201	systemcall.ftruncate	define

	136	systemcall.mkdir 	define

	137	systemcall.rmdir	define

	128	systemcall.rename	define

	73	systemcall.munmap	define

	54	systemcall.ioctl	define


0 ctstop



