(1202505187.021240 dwrr
 a pulsing text effect, 
 to test out the languages usability
)

file library/foundation.s
file library/ascii.s

constant base_color set base_color 01111

constant gray 	set gray 	base_color add gray 	0
constant red 	set red 	base_color add red 	1
constant green 	set green 	base_color add green 	01
constant yellow set yellow 	base_color add yellow 	11
constant blue 	set blue 	base_color add blue 	001
constant cyan 	set cyan 	base_color add cyan 	101
(constant cyan 	set cyan 	base_color add cyan 	101)



(0) string "hello there from space, c = "
(1) string "please input a single character: "
(2) string "[38;5;"


constant arg0 set arg0 0 constant lr
constant skip do skip

constant setcolor at setcolor
	compiler ctsc_putchar char_escape
	compiler ctsc_putchar '['
	compiler ctsc_printdec arg0
	compiler ctsc_putchar ';'
	compiler ctsc_putchar '1'
	compiler ctsc_putchar 'm'
	add lr 1 do lr


constant setcolor256 at setcolor256
	compiler ctsc_putchar char_escape
	compiler ctsc_print 01
	compiler ctsc_printdec arg0
	compiler ctsc_putchar 'm'
	add lr 1 do lr


constant resetcolor at resetcolor
	compiler ctsc_putchar char_escape
	compiler ctsc_putchar '['
	compiler ctsc_putchar '0'
	compiler ctsc_putchar 'm'
	add lr 1 do lr




constant clearscreen at clearscreen
	compiler ctsc_putchar char_escape
	compiler ctsc_putchar '['
	compiler ctsc_putchar 'H'
	compiler ctsc_putchar char_escape
	compiler ctsc_putchar '['
	compiler ctsc_putchar '2'
	compiler ctsc_putchar 'J'
	add lr 1 do lr



constant clearline at clearline
	compiler ctsc_putchar char_escape
	compiler ctsc_putchar '['
	compiler ctsc_putchar '2'
	compiler ctsc_putchar 'K'
	compiler ctsc_putchar 1011
	add lr 1 do lr


at skip del skip




compiler ctsc_print 1
constant c
compiler ctsc_getchar c
compiler ctsc_putchar c
compiler ctsc_putchar char_newline

constant cycle_size set cycle_size 	00011
constant cycle_start set cycle_start 	00010111  (232, color black)

constant i set i 0
constant k set k 0
constant state set state 0


constant loop at loop

	at lr do clearline

	compiler ctsc_print 0

	set arg0 cycle_start
	add arg0 k
	at lr do setcolor256
		compiler ctsc_putchar c
		compiler ctsc_putchar char_space
		compiler ctsc_putchar '('
		compiler ctsc_printdec arg0
		compiler ctsc_putchar ')'
	at lr do resetcolor
	

	constant _ set _ 0 constant _l at _l 	(yes, this is a delay loop... :P  it works, shhh its okay)
	add _ 1 lt _ 0000000000000000001 _l del _ del _l 


	constant else constant endif

	eq state 1 else 

		add k 1
		constant skip 
		ne k cycle_size skip 
			set state 1
		at skip del skip
		do endif		

	at else 
		sub k 1
		constant skip 
		ne k 0 skip 
			set state 0
		at skip del skip

	at endif del endif del else

	del state  del k


	add i 1 lt i 000000000001 loop 


compiler ctsc_putchar char_newline


compiler ctsc_exit 0









































(

(	constant ' ' set ' ' 000001
	constant 'B' set 'B' 100001
	constant 'A' set 'A' 0100001
)

(string table)
(--------------------)

	(0) string "constant " 		constant constant_string set constant_string 0

	(1) string " set " 		constant set_string set set_string 1

	(2) string "'"    		constant tick_string set tick_string 01


constant c set c 000001 (space) 
constant i set i 0
constant loop at loop	
	compiler ctsc_print constant_string
	compiler ctsc_print tick_string	
	compiler ctsc_putchar c
	compiler ctsc_print tick_string
	compiler ctsc_print set_string
	compiler ctsc_print tick_string	
	compiler ctsc_putchar c
	compiler ctsc_print tick_string
	compiler ctsc_putchar 000001
	compiler ctsc_printbin c
	compiler ctsc_putchar 0101

	add c 1 
	constant done eq c 1111_1110 done
	add i 1 do loop 
at done

compiler ctsc_exit 0















(string table)
(--------------------)

	(0) string "hello world! [i is currenly = " 

	(1) string "]" 

	
constant i set i 0
constant loop at loop
	
	compiler ctsc_print 0 
	compiler ctsc_printbin i
	compiler ctsc_print 1
	compiler ctsc_putchar 0101

	constant c
	compiler ctsc_getchar c

	add i 1 lt i 00001 loop del i

compiler ctsc_exit 0






	constant ' ' set ' ' 000001
	constant 'B' set 'B' 100001
	constant 'A' set 'A' 0100001

)


