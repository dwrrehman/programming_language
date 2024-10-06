a file to test out printing numbers in binary, using the "write" system call, anddd functions sythesized out of the base isa. pretty cool actually. showed a bug though. written on 1202410067.015941 by dwrr. 


this line includes the (very small) standard library so far lol. 
oh also this is a comment btw. you can just type it, no special syntax.


lf library/foundation




...just realized that we need a define and. undefine  syntax. 
		theres no other way around this. we need it. 



set limit 1000
mul limit 1000


set in 0   <------ for instance, this variable cannot be called "i" 
			otherwise bugs happen. nottt good. 
			we should be able to call it "i"...



at loopn
	set n in at lr do printbinary 
	at lr do newline
	incr in
	lt in limit loopn

at lr do newline at lr do newline
set exitcode 5 at lr do exit

at putchar
	set putchar_lr lr
	set pointer _process_stackpointer st pointer c 1
	set fd 1 
	set buffer pointer 
	set length 1
	at lr do write
	incr putchar_lr do putchar_lr

at putdigit
	set putdigit_lr lr
	set c 48 add c n at lr do putchar
	incr putdigit_lr do putdigit_lr

at newline
	set newline_lr lr
	set c 10 at lr do putchar
	incr newline_lr do newline_lr

at printbinary
	set printbinary_lr lr
	set i n
	at loop
		set a i and a 1
		set n a at lr do putdigit
		sd i 1
		lt 0 i loop

	incr printbinary_lr do printbinary_lr




eoi









set n 0 at lr do putdigit at lr do newline





















set n _process_stackpointer at ret do function
set n _process_stacksize at ret do function
