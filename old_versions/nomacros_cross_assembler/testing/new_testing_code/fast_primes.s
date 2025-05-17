. 


	mov	i, #0
	mov	count, #0
	mov	w9, #2
	b	init

prime:
	add	count, count, #1

composite:
	add	x9, x9, #1
	add	i, i, #1
	cmp	x9, #64, lsl #12
	b.eq	done
init:
	cmp	x9, #3
	b.lo	prime

	mov	j, #0
inner:
	add	x11, j, #2
	udiv	x12, x9, x11
	msub	x11, x12, x11, x9
	cbz	x11, composite

	add	j, j, #1
	cmp	i, j
	b.ne	inner

	b	prime
done:
	...

. 

