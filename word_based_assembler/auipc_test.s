10001 def callnumber
0101 def a0
1101 def a1
0011 def a2
0000001 def my_string 
1000001 def my_length

1 0 a0 addi
my_string a1 auipc
my_length ctstrlen 0 a2 addi
001 0 callnumber addi
ecall

0111 0 a0 addi
1 0 callnumber addi
ecall

my_string attr

	makestring

1 ct ecall

