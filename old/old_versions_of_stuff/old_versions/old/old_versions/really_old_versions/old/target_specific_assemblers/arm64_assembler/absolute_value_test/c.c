#include <math.h>
#include <stdlib.h>




int main(int argc, const char** argv) {
	int x = atoi(argv[1]);
	if (x < 0) x = -x;
	return x;
}


/*

0000000100003f8c <_main>:
100003f8c: fd 7b bf a9 	stp	x29, x30, [sp, #-16]!
100003f90: fd 03 00 91 	mov	x29, sp
100003f94: 20 04 40 f9 	ldr	x0, [x1, #8]
100003f98: 05 00 00 94 	bl	0x100003fac <_atoi+0x100003fac>
100003f9c: 1f 00 00 71 	cmp	w0, #0
100003fa0: 00 54 80 5a 	cneg	w0, w0, mi
100003fa4: fd 7b c1 a8 	ldp	x29, x30, [sp], #16
100003fa8: c0 03 5f d6 	ret

	

		this is why the absolute value instruction is not neccessary lol. we arent having it lol. 


			cool. 




*/