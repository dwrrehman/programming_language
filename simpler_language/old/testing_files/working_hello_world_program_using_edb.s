def zero
zero cte attr 
arc def nonzero
zero dup dup cte eor
nonzero zero over cte addi 
drop over cte slt 
arc def one drop 
zero dup nonzero cte add drop
one dup dup cte addi drop
one dup dup cte addi 
arc def two drop
zero dup one cte addi drop

two zero dup cte addi 
one swap dup cte addi 
arc def three drop
zero dup dup cte eor drop

three zero dup cte addi 
one swap dup cte addi 
arc def four drop
zero dup dup cte eor drop

four zero dup cte addi 
one swap dup cte addi 
arc def five drop
zero dup dup cte eor drop

five zero dup cte addi 
one swap dup cte addi 
arc def six drop
zero dup dup cte eor drop

six zero dup cte addi 
one swap dup cte addi 
arc def seven drop
zero dup dup cte eor drop

seven zero dup cte addi 
one swap dup cte addi 
arc def eight drop
zero dup dup cte eor drop

eight zero dup cte addi 
one swap dup cte addi 
arc def nine drop
zero dup dup cte eor drop

nine zero dup cte addi 
one swap dup cte addi 
arc def ten drop
zero dup dup cte eor drop

ten zero dup cte addi 
one swap dup cte addi 
arc def eleven drop
zero dup dup cte eor drop

eleven zero dup cte addi 
one swap dup cte addi 
arc def twelve drop
zero dup dup cte eor drop

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
ten 	def 10	def arg0   def newline drop
eleven 	def 11	def arg1 drop
twelve 	def 12	def arg2 drop



1	def ct_exit		drop
2	def ct_print def debug  drop
3	def ct_abort		drop
4	def ct_getchar		drop
5	def ct_putchar		drop
6 	def debug_dictionary 	drop
7 	def debug_instructions 	drop
8 	def debug_registers 	drop
9 	def debug_arguments 	drop
10 	def debug_ctmemory 	drop


eight zero one cte addi
dup dup cte slli
dup dup cte addi arc def 17 def arg7 def function drop
zero zero one cte add drop

8 0 5 cte addi 
3 swap dup cte slli
1 swap dup cte addi
arc def 65 def "A" drop
0 dup 5 cte add drop

8 0 5 cte addi 
3 swap dup cte slli
2 swap dup cte addi
arc def 66 def "B" drop
0 dup 5 cte add drop









3 def string_begin drop
4 def string_end drop
5 def byte_count drop
6 def byte_index drop
7 def temp drop
8 def loop drop
9 def my_string drop


string_end string_begin cte
jal Hello there from space!
this is my document, basically, i am going to try to write as many characters as i want into this string, and see if we are able to actually write it out lol.

	interestingly, you can even have tabs and newlines inside of strings, which is quite cool lol. yay.
i think this way of doing strings is actually quite cool too, because you can have any possible characters in here,

		"even quotes!"  lol

so yeah, thats pretty cool lol 

yay.

lets try printing this now, using our hello world program. 



4 cte attr drop

10 1 dup cte addi string_end dup cte sub drop
1 string_begin dup cte addi drop
string_begin string_end byte_count cte sub drop
byte_count string_begin sp cte stl drop
 








1 0 arg0 addi drop
my_string arg1 auipc drop
byte_count arc 0 arg2 addi drop
4 0 function addi drop
ecall

10 0 arg0 addi drop 
1 0 function addi drop
ecall

my_string attr drop 

loop cte attr drop
sp arc byte_index temp cte ldb arc edb
1 byte_index dup cte addi drop
loop dup byte_count byte_index cte blt

ct_exit 0 function cte addi drop
cte ecall


















------------------------------------------



























debug_registers 0 function cte addi drop cte ecall
ct_abort 0 function cte addi drop cte ecall






debug_registers 0 function cte addi drop  cte ecall
debug_arguments 0 function cte addi drop  cte ecall
ct_getchar 0 function cte addi drop cte ecall









debug_registers 0 function cte addi drop 	cte ecall

debug_ctmemory 0 function cte addi drop 	cte ecall

ct_abort 0 function cte addi drop 	cte ecall










one three cte jal hello there from space! one cte attr dup dup cte eor drop





0 arg0 cte add drop
2 0 function cte addi drop
cte ecall
3 0 function cte addi drop
cte ecall




