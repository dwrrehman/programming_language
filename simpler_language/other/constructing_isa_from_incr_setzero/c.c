/*
	1202407151.201352

		dwrr    


a program to test how many other primitives can be easily synthesized
out of only the operations  incr and setzero.

so far, we have synthesized:


	C syntax	isa name / function
--------------------------------------------

	d += r          modifying add

	d = r * s       mul

	d = r + s       add

	d = 1           slt, basically

	d = r           mov

	d <<= 1         sll by 1

	d = r << s      sll

	d = 64          binary constants


still to go:
-------------------

	nor

	nand

	or

	and

	not

	eor

	sr

	sub

	div

	rem

		
lets try to get     and/or  working now!

	i actually don't know if   "not" is possible... we might need to do that one ourselves lol... like ie, bake it into the languagee.... hmmm idk... we'll seeeee



202407151.202430:		
OMG! 
i just realized the way this is going to work!! the way its going to work is taht:


		basically, we look at the most low level primitives which are still syethesized and not baked in,  we check those patterns first, reduce those, and then start looking for more patterns again, 

			the idea here is that we graduallyyyy build up a more and more complex vocabulary to have when talking about the code that the user wrote,   ie the FIRST stage in the compiler, which actually executes before the optimization phase, righttttt after validation/parsing happens.    basically, instruction selection happens first, so that the optimization phase is dealing with things which are a bitttt more high level lol. basically so that from that point on, we are pretty much guaranteed to select the right instructions  during MACHINE ins sel.  


			wow!!! niceeeee






*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


typedef unsigned long long nat;

static void print(nat x) {
	printf("%llu\n", x);
}

static void add(nat* d, nat r) {
	for (int i = 0; i < r; i++) 
		(*d)++;
}

static void mul(nat* d, nat r, nat s) {
	*d = 0;
	for (nat i = 0; i < r; i++) 
		for (nat j = 0; j < s; j++) 
			(*d)++;
}

static void set(nat* d, nat r) {
	*d = 0;
	add(d, r);
}

static void sum(nat* d, nat r, nat s) {
	set(d, r);
	add(d, s);
}

static void sll_1(nat* d, nat r) {
	sum(d, r, r);
}

static void sll_var(nat* d, nat r, nat s) {
	set(d, r);
	for (nat i = 0; i < s; i++) 
		sll_1(d, *d);
}

static void set_1(nat* d) {
	*d = 0;
	(*d)++;
}

static void set_64(nat* d) {
	set_1(d);
	sll_1(d, *d);
	sll_1(d, *d);
	sll_1(d, *d);
	sll_1(d, *d);
	sll_1(d, *d);
	sll_1(d, *d);
}

int main(void) {
	nat r;
	sum(&r, 3, 5);
	print(r);
	mul(&r, 5, 100);
	print(r);

}



/*

do /usr/bin/clang
./c.c
-ferror-limit=1

do ./a.out

*/

/*static void mul(nat* d, nat r, nat s) {
	nat bits; set_64(&bits);
	*d = 0;
	for (nat i = 0; i < bits; i++) {

		nat c = 0;
		// how do we get the i'th bit of s?.... 
		
		if (c) {
			nat sh;
			sll_var(&sh, r, i);
			add(d, sh);
		}
	}
}*/












