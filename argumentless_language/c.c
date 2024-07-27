// trying to make a language that has no explicit argument passing,
// similar to the ua isa. not neccessarily intended to be the mosttt 
// useable language lol. currently only interpreted. 
// 202407265.205102: by dwrr

/*

language isa:
------------------------------------------

	meaning		spelling
.........................................
	i++		is
	i=0		iz
	i+=*i		im
	i+=*n		in

	*n++		ns
	*n=0		nz
	*n~		nn
	*n+=i		ni
	*n+=*i		nm

	*i++		ms
	*i=0		mz
	*i+=i		mi
	*i+=*n		mn
	
	*n < *i		lm
	*n == *i	em
	*n < i		li
	*n == i		ei

	setflag		sf
	clearflag	cf

	jump0		b0
	jump1		b1
	jump2		b2
	jump3		b3
	jump4		b4
	
	pc0		a0
	pc1		a1
	pc2		a2
	pc3		a3
	pc4		a4



*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <

int main() {

}