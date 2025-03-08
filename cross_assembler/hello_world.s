. a working hello world program in the langauge, in arm64.
  written on 1202503075.183910 by dwrr
. 

lf arm64.s

. -----------------...this should be in the stdlib...---------------------- . 

set 42 4
mul 42 10
add 42 2

set syscallnumber 16
set arg0 0
set arg1 1
set arg2 2

set systemexit 1
set systemread 2
set systemwrite 3

def2 movz d k

	mov d k mov_shift_none mov_type_zero width64

	ret

def1 exit code

	movz syscallnumber systemexit
	movz arg0 code
	svc  
	halt

	ret

. --------------------------------------- . 

set 255 256
decr 255


exit 42

emit 1 255
emit 1 0
emit 1 1
emit 1 2
emit 1 3
emit 1 4
emit 1 5
emit 1 255


