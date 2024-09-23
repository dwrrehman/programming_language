lf library/foundation
lf library/characters

comment
todo: implement a print_number function!
also add comments
also fix the control flow in macros bug!!!
comment


def getchar ar buffer
	read 0 buffer 1
	ret

def putstring ar string ar length
	write 1 string length
	ret

set buffer process_stackpointer

def lowercase ar c
	set 32 1
	si 32 5
	set l c
	lt l char_A skip
		add l 32
	at skip ret


def append ar p ar c
	set l c
	lowercase l
	st p l 1 
	incr p
	ret

set p buffer
append p char_H
append p char_E
append p char_L 
append p char_L
append p char_O
append p char_space
append p char_T
append p char_H
append p char_E
append p char_R
append p char_E
append p char_space
append p char_F
append p char_R
append p char_O
append p char_M
append p char_space
append p char_S
append p char_P
append p char_A
append p char_C
append p char_E
append p char_newline

set length p 
sub length buffer
putstring buffer length
getchar buffer

