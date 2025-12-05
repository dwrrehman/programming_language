file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/core.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/useful.s
file /Users/dwrr/root/projects/programming_language/new_cross_assembler2/library/ascii.s


(the start of a screen editor  written using the compiletime language of our assembler!
written on 1202508203.051100 by dwrr)


set begin 0000_0000_1
set end begin
(set cursor end)

ld pass assembler_pass	
eq pass 0 terminate del pass

eq 0 0 main

at ctgetchar
	ld ra 0
	set c0 ra function_begin

	ld pass assembler_pass	
	eq pass 0 s
		ld c assembler_read
	at s del s
	function_end
	set c0 c
	eq 0 0 ra del ra
	lt 0 0 ctgetchar

at display
	ld ra 0
	set c0 ra function_begin

	emit 1 escape str "[H" 
	emit 1 escape str "[J" ctprintstring

	set p begin
	at loop
		eq p end done		
		ld c p incr p
		set c0 c ctputchar
		eq 0 0 loop del loop
	at done del done

	function_end
	eq 0 0 ra del ra
	lt 0 0 display

at main

at l
	display
	ld c assembler_read

	eq c 'q' done
	eq c '.' paste
	eq c 1111_111 backspace

	st end c incr end
	eq 0 0 l

at backspace
	sub end 1
	eq 0 0 l

at paste
	st end 'h' incr end
	st end 'e' incr end
	st end 'l' incr end
	st end 'l' incr end
	st end 'o' incr end
	eq 0 0 l

at done

str "terminating..." ctprintstring ctnl

at terminate

eoi

a program to try to recreate the editor during compiletime! 

