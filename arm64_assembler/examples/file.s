0 ctbr
	this is a comment lol
	
	202401011.003520:  i switched to having the language use little-endian binary numbers for its identifiers, 
				and i don't think i am going to be going back.. i like it alot so far actually. 
				


1000001 01 ctldi ctput
000001 01 ctldi ctput
0100001 01 ctldi ctput
0101 01 ctldi ctput

01 ctget

1100001 01 ctldi ctput
0100001 01 ctldi ctput
1000001 01 ctldi ctput
0101 01 ctldi ctput

0 ctstop

0 0 0 0 rbit
0 0 0 0 revh
0 0 0 0 rev
0 0 0 0 clz
0 0 0 0 cls

1001 01 ctldi ctimm 00000 0 01 1 mov
1 01 ctldi ctimm 00001 0 01 0 mov 
svc