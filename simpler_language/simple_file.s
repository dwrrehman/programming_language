add 	this.is.prime.number.program!.written.on.1202406237.203040.

slt one zero sp
add two one one
add three two one
add four two two
add five four one
add ten five five
sll n ten ten
sll twenty ten one
sll eighty twenty four
sll n n five

add counter

add i
att loop
	add j two
	att inner
		bge prime j i
		rem r i j
		beq composite r
		add j j one
		blt inner j i
att prime
	blt skip counter eighty
	add counter
	add arga i ecall
att skip	
	add counter counter one
att composite
	add i i one
	blt loop i n


