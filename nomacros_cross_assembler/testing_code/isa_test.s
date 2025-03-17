. 
  testing out the machine code generation 
  for all the instructions we have implemented 
  so far, for arm64. 
  
  written on 1202503097.054017 dwrr 
. 

lf foundation.s

set _targetarchitecture arm64_arch
set _outputformat macho_executable
set _shouldoverwrite true

nop svc

mov 4 42 shift_none type_zero width64 
mov 4 42 shift_16 type_zero width64 
mov 4 42 shift_32 type_zero width64 
mov 4 42 shift_48 type_zero width64 

mov 1 42 shift_none type_zero width64 
mov 2 42 shift_none type_keep width64 
mov 3 42 shift_none type_neg width64 

mov 1 42 shift_none type_zero width32
mov 2 42 shift_none type_keep width32
mov 3 42 shift_none type_neg width32

nop


df label 
jmp jumplink label
jmp nolink label

nop

bc cond_always label
bc is_equal label
bc is_unsigned_less label
bc is_unsigned_greater_or_equal label

nop


. br 4 jumplink isreturn    jumplink is ignored if isreturn. .
br 4 nolink isreturn
br 4 jumplink isnotreturn
br 4 nolink isnotreturn

nop

df should_shift12 set should_shift12 1

addi 4 5 42 should_shift12 setflags subtract width64
addi 4 5 42 should_shift12 noflags subtract width64
addi 4 5 42 should_shift12 setflags positive width64
addi 4 5 42 should_shift12 noflags positive width64
nop
addi 4 5 42 should_shift12 setflags subtract width32
addi 4 5 42 should_shift12 noflags subtract width32
addi 4 5 42 should_shift12 setflags positive width32
addi 4 5 42 should_shift12 noflags positive width32
nop

zero should_shift12

addi 4 5 42 should_shift12 setflags subtract width64
addi 4 5 42 should_shift12 noflags subtract width64
addi 4 5 42 should_shift12 setflags positive width64
addi 4 5 42 should_shift12 noflags positive width64
nop
addi 4 5 42 should_shift12 setflags subtract width32
addi 4 5 42 should_shift12 noflags subtract width32
addi 4 5 42 should_shift12 setflags positive width32
addi 4 5 42 should_shift12 noflags positive width32
nop

df d set d 1
df n set n 2
df m set m 3

adc d n m   noflags positive width64
adc d n m   noflags subtract width64
adc d n m   setflags positive width64
adc d n m   setflags subtract width64
nop

adc d n m   noflags positive width32
adc d n m   noflags subtract width32
adc d n m   setflags positive width32
adc d n m   setflags subtract width32
nop

shv d n m shift_increase width64
shv d n m shift_decrease width64
shv d n m shift_signed_decrease width64
shv d n m rotate_decrease width64
nop

shv d n m shift_increase width32
shv d n m shift_decrease width32
shv d n m shift_signed_decrease width32
shv d n m rotate_decrease width32
nop

adr d label type_byteoffset
adr d label type_pageoffset
nop

cbz n label is_zero width32
cbz n label is_nonzero width32
cbz n label is_zero width64
cbz n label is_nonzero width64
nop

df k set k 5 . check bit 5 .

tbz n 1 label is_zero 
tbz n 2 label is_zero 
tbz n 3 label is_nonzero 
tbz n k label is_nonzero 
nop


set n 4  set m 5
df nvzc set nvzc 15

ccmp is_signed_greater n with_register m cmp_positive nvzc width64 
ccmp is_signed_greater n with_immediate m cmp_positive nvzc width64 
ccmp is_signed_greater n with_register m cmp_negative nvzc width64 
ccmp is_signed_greater n with_immediate m cmp_negative nvzc width64 
nop

ccmp is_signed_greater n with_register m cmp_positive nvzc width32
ccmp is_signed_greater n with_immediate m cmp_positive nvzc width32
ccmp is_signed_greater n with_register m cmp_negative nvzc width32
ccmp is_signed_greater n with_immediate m cmp_negative nvzc width32 
nop

set d 1
set n 2
set m 3

addr d n m     0 0 setflags subtract width64
addr d n m     0 0 setflags positive width64
addr d n m     0 0 noflags subtract width64
addr d n m     0 0 noflags positive width64
nop
df shift_amount set shift_amount 63   . 6 bits max .
addr d n m shift_increase shift_amount noflags positive width64
addr d n m shift_decrease shift_amount noflags positive width64
addr d n m shift_signed_decrease shift_amount noflags positive width64
nop

df option set option 0
df addx_lsl_amount set addx_lsl_amount 4    . must be 0 through 4 . 

addx d n m option addx_lsl_amount setflags subtract width64
addx d n m option addx_lsl_amount setflags positive width64
addx d n m option addx_lsl_amount noflags subtract width64
addx d n m option addx_lsl_amount noflags positive width64
nop

addx d n m extend_uxtb addx_lsl_amount noflags positive width64
addx d n m extend_uxth addx_lsl_amount noflags positive width64
addx d n m extend_uxtw addx_lsl_amount noflags positive width64
addx d n m extend_uxtx addx_lsl_amount noflags positive width64
nop

addx d n m extend_sxtb addx_lsl_amount noflags positive width64
addx d n m extend_sxth addx_lsl_amount noflags positive width64
addx d n m extend_sxtw addx_lsl_amount noflags positive width64
addx d n m extend_sxtx addx_lsl_amount noflags positive width64
nop

csel d n m is_signed_greater csel_set csel_set width64
csel d n m is_signed_greater csel_incr csel_set width64
csel d n m is_signed_greater csel_set csel_not width64
csel d n m is_signed_greater csel_incr csel_not width64
nop

df a
set d 4
set n 5
set m 6
set a 7


    . note: if type_maddl, then width64 must be set. .

madd d n m a type_madd type_madd positive width32
madd d n m a type_madd type_madd subtract width32
madd d n m a type_madd type_madd positive width64
madd d n m a type_madd type_madd subtract width64
nop
madd d n m a type_maddl maddl_signed positive width64
madd d n m a type_maddl maddl_unsigned positive width64
madd d n m a type_maddl maddl_signed subtract width64
madd d n m a type_maddl maddl_unsigned subtract width64
nop








. df shift_increase set shift_increase 0
df shift_decrease set shift_decrease 1
df shift_signed_decrease set shift_signed_decrease 2
df rotate_decrease set rotate_decrease 3
.

nop
orr  bitwise_or d n m   0 0 regular_second width64 
orr  bitwise_and d n m   0 0 regular_second width64
orr  bitwise_eor d n m   0 0 regular_second width64
orr  bitwise_and_setflags d n m   0 0 regular_second width64
nop
orr  bitwise_or d n m   0 0 invert_second width64 
orr  bitwise_and d n m   0 0 invert_second width64
orr  bitwise_eor d n m   0 0 invert_second width64
orr  bitwise_and_setflags d n m   0 0 invert_second width64
nop
df shift_amount set shift_amount 3
df 62 set 62 63 decr 62
orr  bitwise_or  d n m   shift_increase shift_amount invert_second width64 
orr  bitwise_and d n m   shift_decrease shift_amount invert_second width64
orr  bitwise_eor d n m   shift_signed_decrease shift_amount invert_second width64
orr  bitwise_eor d n m   rotate_decrease shift_amount invert_second width64
nop
orr  bitwise_or d n m   shift_increase 0 regular_second width64
orr  bitwise_or d n m   shift_increase 1 regular_second width64
orr  bitwise_or d n m   shift_increase 2 regular_second width64
orr  bitwise_or d n m   shift_increase 3 regular_second width64
orr  bitwise_or d n m   shift_increase 4 regular_second width64
orr  bitwise_or d n m   shift_increase 5 regular_second width64
orr  bitwise_or d n m   shift_increase 62 regular_second width64
orr  bitwise_or d n m   shift_increase 63 regular_second width64
nop

. memi type d n imm12 size . 
nop

memi  store         d n 128   1_byte
memi  load_width32  d n 128   1_byte
. memi  load_width64  d n 128   1_byte .        . same as the line above. . 
memi  load_signed_width32  d n 128   1_byte
memi  load_signed_width64  d n 128   1_byte

nop

memi  store         d n 128   2_bytes
memi  load_width32  d n 128   2_bytes
. memi  load_width64  d n 128   2_bytes .       . same as the line above. . 
memi  load_signed_width32  d n 128   2_bytes
memi  load_signed_width64  d n 128   2_bytes

nop

memi  store         d n 128   4_bytes
memi  load_width32  d n 128   4_bytes
. memi  load_width64  d n 128   4_bytes .         . same as the line above. . 
. memi  load_signed_width32  d n 128   4_bytes .     . invalid combination. . 
memi  load_signed_width64  d n 128   4_bytes

nop

memi  store         d n 128   8_bytes
. memi  load_width32  d n 128   8_bytes .      . invalid combination . 
memi  load_width64  d n 128   8_bytes
. memi  load_signed_width32  d n 128   8_bytes .       . invalid combination . 
. memi  load_signed_width64  d n 128   8_bytes .       . invalid combination . 

nop


. memr type d n m opt size . 
nop
memr   load_width64 d n m   zero_extend_second32   8_bytes
memr   load_width64 d n m   use_second64   8_bytes
memr   load_width64 d n m   sign_extend_second32   8_bytes
memr   load_width64 d n m   sign_extend_second64   8_bytes
memr   load_width64 d n m   scale_then_zero_extend_second32   8_bytes
memr   load_width64 d n m   scale_second64   8_bytes
memr   load_width64 d n m   scale_then_sign_extend_second32   8_bytes
memr   load_width64 d n m   scale_then_sign_extend_second64   8_bytes
nop

memr   load_signed_width32 d n m   zero_extend_second32   2_bytes
memr   load_signed_width32 d n m   use_second64   2_bytes
memr   load_signed_width32 d n m   sign_extend_second32   2_bytes
memr   load_signed_width32 d n m   sign_extend_second64   2_bytes
memr   load_signed_width32 d n m   scale_then_zero_extend_second32  2_bytes
memr   load_signed_width32 d n m   scale_second64   2_bytes
memr   load_signed_width32 d n m   scale_then_sign_extend_second32   2_bytes
memr   load_signed_width32 d n m   scale_then_sign_extend_second64   2_bytes
nop


. memia type d n imm9 size mode .
memia   load_width64 d n 128  8_bytes  pre_advance
memia   load_width64 d n 128  8_bytes  post_advance

memia   load_width64 d n 128  4_bytes  pre_advance
memia   load_width64 d n 128  4_bytes  post_advance

memia   load_width64 d n 128  2_bytes  pre_advance
memia   load_width64 d n 128  2_bytes  post_advance

memia   load_width64 d n 128  1_byte  pre_advance
memia   load_width64 d n 128  1_byte  post_advance

memia   load_signed_width64 d n 128  1_byte  pre_advance
memia   load_signed_width64 d n 128  1_byte  post_advance

nop


. memp   L type_size   d t n imm7  mode .

df k set k 10

nop

memp   store_pair 0 d n m  k 0
memp   load_pair 0 d n m  k 0
. memp load_pair 1 d n m  k 0 .     . invalid . 
memp   load_pair 2 d n m  k 0
. memp   load_pair 3 d n m  k 0 .    . invalid . 

nop

memp   store_pair pair_width32 d n m  k 0
memp   store_pair pair_width64 d n m  k 0
memp   load_pair pair_width32 d n m  k 0
memp   load_pair pair_width64 d n m  k 0

nop

memp   load_pair pair_width64 d n m  k  nontemporal_pair
memp   load_pair pair_width64 d n m  k  post_advance_pair
memp   load_pair pair_width64 d n m  k  offset_pair
memp   load_pair pair_width64 d n m  k  pre_advance_pair

nop

nop


at label
nop

halt

eoi











		memi type d n imm12 size
		memr type d n m opt size
		memia type d n imm9 size mode
		memp L type_size d t n imm7		








STP X0, X1, [SP, #-16]!               <---------- PRE-ADVANCE
----------------------	

	- stores at address (SP - 16), and then 

	- SP = SP - 16

in other words, 

	- SP = SP - 16, and then after this completes,

	- it stores at address (SP).


	



LDP X0, X1, [SP], #16                     <---------- POST-ADVANCE
----------------------	
	- loads from address (SP), and then after the transfer is performed, 

	- SP = SP + 16.








































































































.
cond a0
n a1
ri a2
m a3
neg a4
nvzc a5
sf a6
.

.
n a0
m a1
cond a2
nvzc a3
sf a4
neg a5
ri a6

ccmp n m is_signed_greater   nvzc width64 compare_negative with_register 
ccmp n m is_signed_greater   nvzc width64 compare_positive with_register 
ccmp n m is_signed_greater   nvzc width64 compare_negative with_immediate 
ccmp n m is_signed_greater   nvzc width64 compare_positive with_immediate 
nop

ccmp n m is_signed_greater   nvzc width32 compare_negative with_register 
ccmp n m is_signed_greater   nvzc width32 compare_positive with_register 
ccmp n m is_signed_greater   nvzc width32 compare_negative with_immediate 
ccmp n m is_signed_greater   nvzc width32 compare_positive with_immediate 
nop

.


. desired form:
	ccmp is_signed_greater n with_register cmp_positive m nvzc 0
. 



























