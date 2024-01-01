0 ctbr
	this is a comment lol
	
	202401011.003520:  i switched to having the language use little-endian binary numbers for its identifiers, 
				and i don't think i am going to be going back.. i like it alot so far actually. 
0 ctstop




1 sf 

0 0 rbit
0 0 revh
0 0 rev
0 0 clz
0 0 cls

1 st 1 sb 
1001 1 ctldi ctimm 0 11111 0 addi
0 11111 0 0010 1 1 csel 			0 ctbr 		.0010 is mi condition for csel.    0 ctstop

1001 1 ctldi ctimm 00000 0 01 mov
1000 1 ctldi ctimm 00001 0 01 mov 
svc

