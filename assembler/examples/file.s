

r1 r2 ctldi ctimm r0 r0 movzx
r0 r4 ctldi r0 r1 ctadd ctat ctimm r1 adr
r3 r2 ctldi ctimm r0 r2 movzx

r4 r2 ctldi ctimm r0 r16 movzw
svc


r2 ctzero ctimm r0 r0 movzx
r32 r9 ctldi ctimm r31 r31 subix
r2 ctzero ctimm r31 r1 addix
r9 ctimm r0 r2 movzx
r3 r2 ctldi ctimm r0 r16 movzw
svc


r2 ctzero ctimm r0 r6 addix


r1 r2 ctldi ctimm r0 r0 movzx
r2 ctzero ctimm r31 r1 addix
r2 ctzero ctimm r6 r2 addix
r4 r2 ctldi ctimm r0 r16 movzw
svc


r1 r2 ctldi ctimm r0 r0 movzx
r4 r4 ctldi r0 r1 ctadd ctat ctimm r1 adr
r6 r2 ctldi ctimm r0 r2 movzx
r4 r2 ctldi ctimm r0 r16 movzw
svc

r2 ctzero ctimm r6 r0 addix
r1 r2 ctldi ctimm r0 r16 movzw 
svc

r0 r4 ctldi r0 r1 ctadd 
r3 ctld4
r1 ctat r2 ctld4
r3 r2 r3 ctsub
r2 r2 ctldi r3 r3 ctshl
r3 r1 ctst4

r3 ctzero
r104 r4 ctldi 
r0 r8 ctldi r4 r4 ctshl r4 r3 r3 ctor
r105 r4 ctldi 
r8 r8 ctldi r4 r4 ctshl r4 r3 r3 ctor
r10 r4 ctldi 
r16 r8 ctldi r4 r4 ctshl r4 r3 r3 ctor
r0 r4 ctldi 
r24 r8 ctldi r4 r4 ctshl r4 r3 r3 ctor
r3 ctimm dw

r4 r4 ctldi r0 r1 ctadd 
r3 ctld4
r1 ctat r2 ctld4
r3 r2 r3 ctsub
r2 r2 ctldi r3 r3 ctshl
r3 r1 ctst4

r3 ctzero
r58 r4 ctldi 
r0 r8 ctldi r4 r4 ctshl r4 r3 r3 ctor
r100 r4 ctldi 
r8 r8 ctldi r4 r4 ctshl r4 r3 r3 ctor
r111 r4 ctldi 
r16 r8 ctldi r4 r4 ctshl r4 r3 r3 ctor
r110 r4 ctldi 
r24 r8 ctldi r4 r4 ctshl r4 r3 r3 ctor
r3 ctimm dw

r3 ctzero
r101 r4 ctldi 
r0 r8 ctldi r4 r4 ctshl r4 r3 r3 ctor
r10 r4 ctldi 
r8 r8 ctldi r4 r4 ctshl r4 r3 r3 ctor
r0 r4 ctldi 
r16 r8 ctldi r4 r4 ctshl r4 r3 r3 ctor
r0 r4 ctldi 
r24 r8 ctldi r4 r4 ctshl r4 r3 r3 ctor
r3 ctimm dw




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


