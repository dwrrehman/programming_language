

	r0 ctzero 
	r2 ctzero ctincr ctincr
	r5 ctzero ctincr ctincr ctincr ctincr ctincr
	r5 r5 r1 ctmul
	r1 r1 r1 ctadd
	r1 r1 r1 ctadd
	r2 r1 r1 ctsub
	r5 r1 r1 ctsub

	r0 r0 r0 movzx
	r1 r0 r8 movzw
	svc




	r0 r0 r0 movzx
	r1 r0 r8 movzw
	svc

	r0 r0 r0 movkx
	r1 r0 r8 movkw
	svc


	r0 r0 r0 movnx
	r1 r0 r8 movnw
	svc


	r5 r2 r1 addix
	r5 r2 r1 addiw
	r5 r2 r1 addhx
	r5 r2 r1 addhw


	nop
	nop
	nop
	nop
	nop
	nop



