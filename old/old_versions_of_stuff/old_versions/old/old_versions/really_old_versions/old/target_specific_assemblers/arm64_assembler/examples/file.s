0 ctbr
	this is a comment lol
	
	202401011.003520:  i switched to having the language use little-endian binary numbers for its identifiers, 
				and i don't think i am going to be going back.. i like it alot so far actually. 
0 ctstop

1000 use

0 setsf 

0 0 rbit
0 0 revh
0 0 rev
0 0 clz
0 0 cls


1 setst 1 setsb 
1001 1 ctldi ctimm 0 11111 addi
0 11111 0 0010 1 1 csel 			0 ctbr 		.0010 is mi condition for csel.    0 ctstop


0 ctbr 

	we should make a hello world program now! that uses the string      hello world from space      in the source,  to populate the .text section!!

	lets work on that next. 

	and then we start the prime number program, i think. yay. 

	also, we are going to print out prime numbers in binary!!! fun stuff. 

	yay

0 ctstop


01 setoc
1001 1 ctldi ctimm 00000 mov
1000 1 ctldi ctimm 00001 mov 
svc





eof




