
1000001 def label
10001 	def callnumber
0101 	def a0 
1101 	def a1
0011 	def a2
a0 	def exitcode
0001 	def exitfail


exitfail 0 exitcode addi
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

1 0 callnumber addi
ecall


1 ct ecall


















00001 	def exitsuccess






















everything from here is comments.

0 0 0 add
0 0 0 sub
0 0 0 addi
