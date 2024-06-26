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

ten zero dup ct addi 
one swap dup ct addi 
arc def eleven drop
zero dup dup ct eor drop

eleven zero dup ct addi 
one swap dup ct addi 
arc def twelve drop
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
eleven 	def 11	def arg1 drop
twelve 	def 12	def arg2 drop

eight zero one ct addi
dup dup ct slli
dup dup ct addi arc def 17 def arg7 def function drop
zero zero one ct add drop









		0 def now.lets.try.to.define.a.macro!!!!... drop

			0 def also.we.need.to.figure.out.comments.lollllll drop

			0 def next_task:======>...strings. drop


one three ct jal

				<---- note: we are putting the address of RIGHT AFTER the ctjal into   three 

					so that in theoryyyyy we could     use   this entire method of 

								ct branching / block comment-like thingies 


							in order to implement strings too!!!!   YAY  so yeah, trying to think ahead to that a little bit 





	ohhh turns out we already implemented comments via ct branches! lol. its actually super simple lol. 

		you can even nest comments if you get the labels done right lol. so yeah. this works lol. yay.





		so yeah, the next step is to figure out how to do string-table generation, i think.

			that will be quite fun.



			oh and defining a macro! lets do that now maybe. yeah. lets do that now. 












first lets get multiple files working i think..

			andd... lets do it right lol.. 


so yeah


				basically we'll figure out how to do macros, without doing textual inclusion, basically. so yeah. 

				lets try that. hm. 

				


one ct attr dup dup ct eor drop








2 0 arg0 addi  drop
7 def my_string arg1 auipc drop
5 ctstrlen arc 0 arg2 addi drop
4 0 function addi  drop
ecall

10 0 arg0 addi drop 
1 0 function addi drop
ecall

my_string attr drop 
makestring

1 0 function ct addi drop
ct ecall







paste this after a variable to see its value at compiletime:




0 arg0 ct addi drop
2 0 function ct addi drop
ct ecall
3 0 function ct addi drop
ct ecall




