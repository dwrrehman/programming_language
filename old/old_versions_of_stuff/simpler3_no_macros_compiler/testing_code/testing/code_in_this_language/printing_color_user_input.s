(
	a 2048 game written in this language, 
	made to test out the usability of the compiletime system for larger programs. 
)

file library/foundation.s
file library/ascii.s

(0) string "hello there from space, c = "
(1) string "please input a single character: "


( making macros for outputting:  "\033[32m"   "\033[0m"  in order to make colored text! ) 


constant arg0 set arg0 0 constant lr
constant skip do skip

constant green at green
	compiler ctsc_putchar 11011
	compiler ctsc_putchar '['
	compiler ctsc_putchar '3'
	compiler ctsc_putchar '2'
	compiler ctsc_putchar ';'
	compiler ctsc_putchar '1'
	compiler ctsc_putchar 'm'
	add lr 1 do lr


constant resetcolor at resetcolor
	compiler ctsc_putchar 11011
	compiler ctsc_putchar '['
	compiler ctsc_putchar '0'
	compiler ctsc_putchar 'm'
	add lr 1 do lr

at skip del skip




compiler ctsc_print 1
constant c
compiler ctsc_getchar c
compiler ctsc_putchar c
compiler ctsc_putchar char_newline

constant i set i 0
constant loop at loop

	compiler ctsc_print 0

	at lr do green 
		compiler ctsc_putchar c
	at lr do resetcolor
	
	compiler ctsc_putchar char_newline

	add i 1 lt i 0101 loop 

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


