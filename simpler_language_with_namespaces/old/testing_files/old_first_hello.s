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




0 def -------------------user.code.starts.here!------------------------ drop


3 def string_begin drop
4 def string_end drop
5 def pointer drop
6 def my_string drop
7 def byte_count drop
8 def my_string2 drop
9 def byte_count2 drop

0 sp pointer cte add drop

string_end string_begin cte jal Hello there from space!
this is my document, basically, i am going to try to write as many characters as i want into this string, and see if we are able to actually write it out lol.

	interestingly, you can even have tabs and newlines inside of strings, which is quite cool lol. yay.
i think this way of doing strings is actually quite cool too, because you can have any possible characters in here,

		"even quotes!"  lol

so yeah, thats pretty cool lol 

yay.

lets try printing this now, using our hello world program. 
4 cte attr drop

10 0 1 cte addi 
string_end dup cte sub drop
1 string_begin dup cte addi drop
string_begin string_end byte_count cte sub
string_begin pointer cte stl drop
byte_count pointer dup cte add drop

string_end dup dup cte eor drop
string_end string_begin cte jal ......(this is an empty string)....
4 cte attr drop

10 0 1 cte addi string_end dup cte sub drop
1 string_begin dup cte addi drop
string_begin string_end byte_count2 cte sub
string_begin pointer cte stl drop
byte_count2 pointer dup cte add drop

1 0 arg0 addi drop
my_string arg1 auipc drop
byte_count arc 0 arg2 addi drop
4 0 function addi drop
ecall

1 0 arg0 addi drop
my_string2 arg1 auipc drop
byte_count2 arc 0 arg2 addi drop
4 0 function addi drop
ecall

0 0 arg0 addi drop 
1 0 function addi drop
ecall



0 sp pointer cte add
my_string attr drop
byte_count swap ecm
byte_count pointer dup cte add
my_string2 attr drop 
byte_count2 swap ecm
byte_count2 pointer dup cte add





ct_exit 0 function cte addi drop cte ecall



------------------------------------------












	.0101 .1100 .


	s = 1;   r = 0;

		
	r += 10 * s; s <<= 4;

	r += 3 * s; s <<= 4;










0 0 arg0 addi drop
1 0 function addi drop
ecall





ct_exit 0 function cte addi drop cte ecall




	"dup", "over", "third", "drop", "swap", "rot",
	"def", "arc", "cte", "attr", 

	"add", "addi", "sub", "slt", "slti", "slts", "sltis", 
	"and", "andi", "ior", "iori", 
	"eor", "eori",  "sll", "slli",  "srl", "srli","sra", "srai", 
	"blt", "blts", "bge", "bges", "bne", "beq", 
	"ldb", "ldh", "ldw", "ldd", "stb", "sth", "stw", "std", 
	"mul", "mulh", "mulhs", "div", "divs", "rem", "rems", 
	"jalr", "jal", "auipc", "ecall", "ecm", "stl",





loop cte attr drop
sp arc byte_index temp cte ldb arc edb
1 byte_index dup cte addi drop
loop dup byte_count byte_index cte blt












debug_ctmemory 0 function cte addi cte ecall
debug_registers 0 function cte addi cte ecall
ct_abort 0 function cte addi cte ecall






















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




