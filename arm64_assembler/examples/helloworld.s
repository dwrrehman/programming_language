10001 ctpc 
0 ctbr hello there from space! A 0 ctstop 
0001 01 ctldi 
001 001 ctadd
1 01 ctldi 
ctimm 
0 mov

001 01 ctldi 
0 1 ctadd 
ctat 
ctimm 
1 adr

1 01 ctldi 
ctimm 01 mov
001 01 ctldi 
ctimm 
00001 0 mov 
svc

01 ctzero 
ctimm 
0 mov
1 01 ctldi
ctimm 
00001 mov
svc

0 001 ctldi 
0 1 ctadd 
11 ctld4
1 ctat 
01 ctld4
11 01 11 ctsub

01 01 ctldi 
11 11 ctshl
11 1 ctst4
11 ctzero
10001 001 ctldt

0 0001 ctldi 
001 001 ctshl 00
1 11 11 ctnor
11 ctimm 
data

eof


do /usr/bin/sed
-i
.backup
1s/^/#!\/bin\/zsh\n/
test_program


do /bin/cat
test_program


"#!/bin/zsh
rm object.o 
rm program.out
./run examples/$1 -c object.o -o program.out
./program.out
echo $?
"


sed -i '1s/^/<added text> /' file



