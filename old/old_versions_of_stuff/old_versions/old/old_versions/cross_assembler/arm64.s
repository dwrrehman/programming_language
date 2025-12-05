. this file defines the machine instructions for the arm 64-bit architecture. .
lf foundation.s

setarch arm64_arch

def0 nop ret
def0 svc ret
def5 mov d k sh op sf obs d ret
def5 bfm d n k op sf obs d ret
def6 adc d n m s sf sb obs d ret
def8 addx d n m opt i3 s sf sb obs d ret
def7 addi d n i12 sh s sf sb obs d ret
def8 addr d n m sh i6 s sf sb obs d ret
def3 adr d a p obs d obs a ret
def5 shv d n m op sf obs d ret
def4 clz d n sf op obs d ret
def4 rev d n sf op obs d ret
def2 jmp l a obs a ret
def2 bc cond a obs a ret
def3 br n l r ret
def4 cbz n a sf iv obs a ret
def4 tbz n b a iv obs a ret
def7 ccmp n m cond nzcv sf iv im ret
def7 csel d n m cond ic iv sf obs d ret
def5 ori d n k sf op obs d ret
def8 orr d n m sh i6 iv sf op obs d ret
def5 extr d n m i7 sf obs d ret
def3 ldrl d a sz obs d ret
def7 memp d t n i7 l e sz ret
def6 memia d n i9 l eo sz ret
def5 memi d n i12 l sz ret
def6 memr d n m opt l sz ret
def8 madd d n m a op u sf sb obs d ret
def5 divr d n m u sf obs d ret


set _target_architecture arm64_arch

set reg.zr 31
set reg.sp 31
set reg.lr 30

set is_equal 0
set is_not_equal 1
set is_unsigned_higher_or_same 2
set is_unsigned_greater_or_equal 2
set is_unsigned_lower 3
set is_unsigned_less 3
set is_minus 4
set is_negative 4
set is_nonnegative 5
set has_overflow_set 6
set has_overflow_clear 7
set is_unsigned_greater 8
set is_unsigned_lower_or_same 9
set is_signed_greater_or_equal 10
set is_signed_less 11
set is_signed_greater 12
set is_signed_less_or_equal 13
set cond_always 14
set cond_never 15

set width64 1
set width32 0

set mov_type_zero 2
set mov_type_neg 0
set mov_type_keep 3
set mov_shift_none 0
set mov_shift_16 1
set mov_shift_32 2
set mov_shift_48 3







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

def2 movz d k
	mov d k mov_shift_none mov_type_zero width64
	ret

def1 exit code
	movz syscallnumber systemexit
	movz arg0 code
	svc halt ret


eoi

