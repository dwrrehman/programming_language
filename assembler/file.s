

r0 ctzero 
r1 ctzero ctincr 
r4 ctzero ctincr ctincr ctincr ctincr
r10 ctzero ctincr ctincr ctincr ctincr ctincr ctincr ctincr ctincr ctincr ctincr

r4 r1 r5 ctshl
r4 r5 r5 ctshl

r4 r5 r5 ctsub
r4 r5 r5 ctsub
r4 r5 r5 ctsub


	r0 r0 r0 ctadd ctprint
	r0 r0 r0 ctadd ctprint
	r0 r0 r0 ctadd ctprint
	r0 r0 r0 ctadd ctprint

	r27 ctpc

	r26 r10 r1 ctblt

	r4 r4 r11 ctadd ctprint
	r0 r0 r0 ctadd ctprint

	r26 ctincr
	r27 ctgoto
	r1 ctstop

	r5 r5 r7 ctadd ctprint
	r1 r1 r7 ctadd ctprint
	
	

	r5 ctprint
	r5 ctprint
	r5 ctprint






r5 r0 r0 movzx
r1 r0 r16 movzw
svc


