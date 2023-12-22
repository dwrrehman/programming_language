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




	todo:           we should allow for       alias   "macros" only        ie,        the ability to set      bubbles         as a synonym for r10 

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


