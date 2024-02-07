0 ctf 
this is a prime number program written
in the compiletime programming language of 
my assembler. its not very fast, but it does
print prime numbers! 

modify the count variable to
set the largest prime to print. 
0 ctstop

0000000000001 0001 count define ctli 

1001  1001 tab   define ctli
01 nextnumber define 
001 nextdivisor define
111 composite define 
0101 prime define 
1101 i/2 define
11 j define
1 i define 

ctzero
nextnumber ctpc
	j ctzero ctincr ctincr
	nextdivisor ctpc
		ctclear
		1 101 ctli i i/2 ctsr
		i/2 j ctbge prime ctpc ctf
		101 ctzero j i 011 ctrem 101 ctbeq composite ctpc ctf
		j ctincr i/2 j ctblt nextdivisor ctb
	prime ctstop i ctprint 
	composite ctstop
	i ctincr count i ctblt nextnumber ctb

eof



















for (int i = 0; i < 25; i++) {

	for (int j = 2; j < i; j++) {
		if (i % j == 0) goto composite;
	}
prime:
	print(i);
composite: 
	continue;
}




i ctzero 
loop ctpc
	i ctprint 

	j ctzero
	loop2 ctpc

		tab ctput j ctprint

		j ctincr a j ctblt loop2 ctb

	i ctincr a i ctblt loop ctb



ascii('C') a ctli ctprint 


