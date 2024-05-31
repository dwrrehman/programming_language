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

	makestring     <-------- this should take a pointer and length     into ct mem.

1 ct ecall

the basic scheme is that we are going to construct the string table in ct memory first, 

	and then we will have all the indexes into it that we need, along with a variable that stores the length of every single string too. 


	that should be enough to use the strings in the program no problem. they don't even need to be null terminated!



yay



