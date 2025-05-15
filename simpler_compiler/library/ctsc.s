(the standard library compiletime system call interface!
this file defines constants relating to it. 
written on 1202505106.223908 by dwrr.)

set 	ctsc_arg0 0        (this variable must be defined first in the program: it is the first argument register to CT SC's.)
rt 	ctsc_call 0        (this variable must be UNUSED in the program, EXCEPT as the target of "rt" instructions.)

set 1024 0000_0000_001

set ctsc_exit 		1024 add ctsc_exit 		1 	 ("exit(0);")
set ctsc_read 		1024 add ctsc_read 		01       ("a0 = getchar();")
set ctsc_write 		1024 add ctsc_write 		11       ("putchar(a0);")
set ctsc_hex 		1024 add ctsc_hex 		001      (prints hex value of first argument register.)
set ctsc_print 		1024 add ctsc_print 		101      (prints decimal value of first argument register.)
set ctsc_abort 		1024 add ctsc_abort 		011      ("abort();")
set ctsc_debug 		1024 add ctsc_debug 		111      (toggles if compiletime execution debug output is enabled.)
set ctsc_display 	1024 add ctsc_display 		0001     (prints the most recently defined string to the screen.)
set ctsc_target 	1024 add ctsc_target 		1001     (set the target architecture to compile for.)
set ctsc_format 	1024 add ctsc_format 		0101     (set the ouput format the compiler should use.)
set ctsc_overwrite 	1024 add ctsc_overwrite 	1101     (toggle if the output is allowed to be overwritten.)
set ctsc_strlen 	1024 add ctsc_strlen 		0011     (compute the string length for a given string index.)


(end of document)
