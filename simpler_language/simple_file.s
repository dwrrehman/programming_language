
slt one zero sp
add two one one
add three two one
add four two two
add five four one
add ten five five
sll n ten ten
add arg0 n ecall
add i

att label
	add arg0 i ecall
	add i i one
	blt label i n
