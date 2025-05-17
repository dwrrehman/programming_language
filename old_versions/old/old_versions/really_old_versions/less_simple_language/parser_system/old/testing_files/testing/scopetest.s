lf library/foundation

def print ar x
	debug_number x
	ret

def l
	set x 5
	set y x
	mul y x
	print x
	print y
ret

def l
	set a 1
	set a 2
	print a
ret

def l
	set a 1
	set a 2
	print a
ret



def . 
	def define-as-zero obs ar variable obs
		zero variable
		ret
ret


def print ar x debug_number x ret


define-as-zero bubbles
incr bubbles
debug_number bubbles

set 8 4
mul 8 2

print stacksize
print stackpointer

st stackpointer 5 8
ld r stackpointer 8
print r













