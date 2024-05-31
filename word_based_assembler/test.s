
100001 	def label
10001 	def callnumber
0 	def zero
1 	def ra
01 	def sp
0101 	def a0 
1101	def a1
a0 	def exitcode
a1 	def was_less

ra zero a1 slt
0011 was_less exitcode addi
1 zero callnumber addi
ecall

1 ct ecall

zero zero zero sll
zero zero zero srl
zero zero zero sra
0 exitcode exitcode addi

exitfail 0 011 addi
label 011 exitcode beq

	1 exitcode exitcode addi
	1 exitcode exitcode addi
	1 exitcode exitcode addi
	1 exitcode exitcode addi
label attr

000001 exitcode exitcode addi
000001 exitcode exitcode addi

exitcode def z

z z z eor 
z z z and

z z z slt
z z z slts

1101 	def a1
0011 	def a2
0001 	def exitfail


00001 	def exitsuccess


everything from here is comments.

0 0 0 add
0 0 0 sub
0 0 0 addi
