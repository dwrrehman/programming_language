def zero 
zero ct attr 
arc def nonzero
zero dup dup ct eor
nonzero zero over ct addi 
drop over ct slt 
arc def one drop 
zero dup nonzero ct add drop
one dup dup ct addi drop
one dup dup ct addi 
arc def two drop
zero dup one ct addi drop

two zero dup ct addi 
one swap dup ct addi 
arc def three drop
zero dup dup ct eor drop

three zero dup ct addi 
one swap dup ct addi 
arc def four drop
zero dup dup ct eor drop

four zero dup ct addi 
one swap dup ct addi 
arc def five drop
zero dup dup ct eor drop

five zero dup ct addi 
one swap dup ct addi 
arc def six drop
zero dup dup ct eor drop

six zero dup ct addi 
one swap dup ct addi 
arc def seven drop
zero dup dup ct eor drop

seven zero dup ct addi 
one swap dup ct addi 
arc def eight drop
zero dup dup ct eor drop

eight zero dup ct addi 
one swap dup ct addi 
arc def nine drop
zero dup dup ct eor drop

nine zero dup ct addi 
one swap dup ct addi 
arc def ten drop
zero dup dup ct eor drop


zero	def 0	drop
one	def 1	def ra drop
two	def 2	def sp drop
three	def 3	drop
four	def 4	drop
five	def 5	drop
six	def 6	drop
seven	def 7	drop
eight	def 8	drop
nine	def 9	drop
ten 	def 10	def arg0 drop

eight zero one ct addi
dup dup ct slli
dup dup ct addi arc def 17 def arg7 def function drop
zero zero one ct add drop


5 0 arg0 addi 
1 0 function addi 
ecall






