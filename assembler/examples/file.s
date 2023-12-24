5 4 ctldi
14 7 ctldi

1 ctzero

128 ctpc
ctdel 0 ctsta 2 ctzero 1 0 ctbeq 

	7 ctprint
	4 ctprint
	4 ctprint

	2 ctzero 0 0 movzx

0 ctlda ctbr 0 ctstop


129 ctpc 
ctdel 0 ctsta 2 ctzero 1 0 ctbeq 

	77 2 ctldi ctprint
	77 2 ctldi ctprint
	77 2 ctldi ctprint

	nop
	nop

0 ctlda ctbr 0 ctstop


1 ctincr

0 ctprint

64 ctpc 128 ctgoto
64 ctstop

65 ctpc 128 ctgoto
65 ctstop

66 ctpc 129 ctgoto
66 ctstop


41 2 ctldi ctput
20 2 ctldi ctput
42 2 ctldi ctput
a 2 ctldi ctput

2 ctget


43 2 ctldi ctput
42 2 ctldi ctput
41 2 ctldi ctput
a 2 ctldi ctput







1 2 ctldi ctimm 0 0 movzx
0 4 ctldi 0 1 ctadd ctat ctimm 1 adr
3 2 ctldi ctimm 0 2 movzx
4 2 ctldi ctimm 0 10 movzw
svc


2 ctzero ctimm 0 0 movzx
20 9 ctldi ctimm 1f 1f subix
2 ctzero ctimm 1f 1 addix
9 ctimm 0 2 movzx
3 2 ctldi ctimm 0 10 movzw
svc

2 ctzero ctimm 0 6 addix

1 2 ctldi ctimm 0 0 movzx
2 ctzero ctimm 1f 1 addix
2 ctzero ctimm 6 2 addix
4 2 ctldi ctimm 0 10 movzw
svc

1 2 ctldi ctimm 0 0 movzx
4 4 ctldi 0 1 ctadd ctat ctimm 1 adr
6 2 ctldi ctimm 0 2 movzx
4 2 ctldi ctimm 0 10 movzw
svc

2 ctzero ctimm 6 0 addix
1 2 ctldi ctimm 0 10 movzw 
svc

0 4 ctldi 0 1 ctadd 
3 ctld4
1 ctat 2 ctld4
3 2 3 ctsub
2 2 ctldi 3 3 ctshl
3 1 ctst4

3 ctzero

68 4 ctldi 
0 8 ctldi 4 4 ctshl 4 3 3 ctor

69 4 ctldi
8 8 ctldi 4 4 ctshl 4 3 3 ctor

a 4 ctldi
10 8 ctldi 4 4 ctshl 4 3 3 ctor

0 4 ctldi 
18 8 ctldi 4 4 ctshl 4 3 3 ctor

3 ctimm dw



4 4 ctldi 0 1 ctadd 
3 ctld4
1 ctat 2 ctld4
3 2 3 ctsub
2 2 ctldi 3 3 ctshl
3 1 ctst4



3 ctzero

3a 4 ctldi 
0 8 ctldi 4 4 ctshl 4 3 3 ctor

64 4 ctldi 
8 8 ctldi 4 4 ctshl 4 3 3 ctor

6f 4 ctldi 
10 8 ctldi 4 4 ctshl 4 3 3 ctor

6e 4 ctldi 
18 8 ctldi 4 4 ctshl 4 3 3 ctor

3 ctimm dw




3 ctzero

65 4 ctldi 
0 8 ctldi 4 4 ctshl 4 3 3 ctor

a 4 ctldi 
8 8 ctldi 4 4 ctshl 4 3 3 ctor

0 4 ctldi 
10 8 ctldi 4 4 ctshl 4 3 3 ctor

0 4 ctldi 
18 8 ctldi 4 4 ctshl 4 3 3 ctor

3 ctimm dw


eof
























write: w16=4, x0 fd    x1 buf    x2 len
1 for stdout
using .text section to store string literal. constructing the string literal using 
db or dw? not sure... hm....

































































r10 r1 ctldi 
r12 r2 ctldi 
r1 ctprint r2 ctprint r3 ctadd ctprint 
r1 r1 ctldi ctprint r3 ctprint r3 ctshl ctprint

r10 r10 ctldi
r5 r1 ctldi 
r2 ctzero
r9 ctpc

r1 r2 r10 ctbeq

	r2 ctprint

	r11 r11 ctldi
	r3 r4 ctldi 
	r3 ctzero
	r8 ctpc

	r4 r3 r11 ctbeq

	r3 ctprint

	r3 ctincr
	r8 ctgoto
	r11 ctstop

r2 ctincr
r9 ctgoto
r10 ctstop







r42 r254 ctldi ctimm r0 r0 movzx 
r1 r5 ctldi ctimm r0 r16 movzw 
svc

eof




	idea:           we should allow for       alias   "macros" only        ie,        the ability to set      bubbles         as a synonym for r10 

													or set       pasta        as a synonym for  r4


															etc



			that simple functionality would go a longggggggg way. but yeah. i mean, we don't needdddddd it. its just nice i guess. hm. 
					ill think about it. ill see how far we can really get with just numbers, though. because obviosly thats preferable lol. 

						mk 



		














































r0 ctzero ctincr r0 r0 movzx
r0 ctzero r0 r1 adr
r0 ctat
r1 ctzero ctincr ctincr
r1 r1 r1 ctadd
r1 r0 r0 ctmul

















r0 r0 r128 ctpc ctbeq



ctstop


r128 ctgoto 
ctstop 




