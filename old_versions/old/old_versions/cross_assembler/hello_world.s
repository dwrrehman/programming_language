. a working hello world program in the langauge, in arm64.
  written on 1202503075.183910 by dwrr
. 

lf arm64.s

exit 32

eoi


def2 writeout given_string given_length
	movz syscallnumber systemwrite
	movz 0 stdout
	adr 1 given_string 0
	movz 2 given_length
	svc ret



. initialize string table . 
set ctsp 0   storenat ctsp 0  add ctsp 8


at [ . hello world! . at ]


. taking inputs "[" and "]", and generating an entry in the string table. . 
la textdata [
la enddata ]
set length enddata
sub length textdata
set start textdata

storenat ctsp start  add ctsp 8
storenat ctsp length  add ctsp 8

def .stringcount  loadnat .stringcount 0 incr .stringcount storenat 0 x


writeout string_table length
exit 42




at string_table

def stringcount 
loadnat stringcount 0

set a 8 set i 0 at loop

	def this_start  loadnat this_start a   add a 8
	def this_length loadnat this_length a  add a 8

	ctprint this_length
	ctprint this_start
	
	set j 0 at inner
		set at this_start add at j
		lc r at emit 1 r
		incr j lt j this_length inner

	emit 1 0

	incr i lt i stringcount loop


eoi

























. -----------------...this should be in the stdlib...---------------------- . 

set 42 4
mul 42 10
add 42 2

set 255 256  decr 255

set syscallnumber 16
set arg0 0
set arg1 1
set arg2 2


set systemexit 1
set systemfork 2

set systemread 3
set systemwrite 4

set systemopen 5
set systemclose 6



set stdout 1
set stdin 0


def2 movz d k
	mov d k mov_shift_none mov_type_zero width64
	ret

def1 exit code
	movz syscallnumber systemexit
	movz arg0 code
	svc halt ret

def1 ctdebug x   ctprint x ctabort ret


def2 storenat address data
	def x set x data  def i set i 0  def a set a address
	def loop at loop
		set k i  mul k 8  sd x k
		set b x  and b 255
		st a b  incr a		
		incr i  lt i 8 loop
	ret

def2 loadnat data address
	def x set x 0  def i set i 0  def a set a address
	def loop at loop
		ld b a  incr a 
		set k i  mul k 8  
		si b k  or x b
		incr i  lt i 8 loop
	set data x
	ret

set 24 25 decr 24

. set i 0 def loop at loop

storenat a i add a 8
storenat 12 512
storenat 24 1024

def x def y def z

loadnat x 0   ctprint x 
loadnat y 12  ctprint y
loadnat z 24  ctprint z

ctabort

eoi
.

. --------------------------------------- . 

