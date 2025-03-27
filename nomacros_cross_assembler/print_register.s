. 
printing a register value in arm64 
1202503167.225302 dwrr 
finished on 1202503171.000025 

example output:
	debugging binary value for register = 0000000000000000000000000000000101101101100101111011010000110000
yay
. 

lf library/foundation.s

set _targetarchitecture arm64_arch
set _outputformat macho_executable
set _shouldoverwrite true

df target set target 10001

movz target 11011
mov  target 10111 shift_16 type_keep width64
mov  target 10101 shift_32 type_keep width64
mov  target 11101 shift_48 type_keep width64

df string_begin df string_length

print string_begin string_length target 0001 1001 0101 1101 0011
print string_begin string_length linkregister 0001 1001 0101 1101 0011
addi target stackpointer 0 shift_none noflags positive width64
print string_begin string_length target 0001 1001 0101 1101 0011
exit 0

at string_begin   string  "debugging binary value for register = "
df end at end set string_length end udf end 
sub string_length string_begin

udf string_begin 
udf string_length

eoi







































. print linkregister  0001 1001 0101 1101 0011  . 

. choose your register you want to debug: . 
. addi target stackpointer 0 shift_none noflags positive width64 . 
. movr target linkregister . 







































. choose your register you want to debug: . 
addi target stackpointer 0 shift_none noflags positive width64
. orr bitwise_or target linkregister zeroregister shift_increase shift_none regular_second width64 . 


df mask set mask 21  mov mask 1 shift_none type_zero width64
df digit set digit 6  mov digit '0' shift_none type_zero width64

df loop at loop
	addi i i 1 shift_none setflags subtract width64
	orr bitwise_and_setflags zeroregister target mask shift_increase shift_none regular_second width64
	csel 17 digit digit is_zero csel_incr csel_set width64
	memr store 17 register_data i use_second64 1_byte 
	addr mask zeroregister mask shift_increase 1 noflags positive width64
	cbz i loop is_nonzero width64
udf loop  udf i  
udf digit udf mask udf target

df final set final 6 
mov final newline shift_none type_zero width64	
memi store final register_data 64 1_byte 
udf final

df total_string_length set total_string_length string_length  add total_string_length 65

mov syscallarg0 stdout shift_none type_zero width64
orr bitwise_or syscallarg1 address zeroregister shift_increase shift_none regular_second width64
mov syscallarg2 total_string_length shift_none type_zero width64
mov syscallnumber system_write shift_none type_zero width64
svc













lf foundation.s

set _targetarchitecture arm64_arch
set _outputformat macho_executable
set _shouldoverwrite true


df buffer_size set buffer_size 16
df allocation set allocation 16 mul allocation buffer_size

addi stackpointer stackpointer allocation shift_none noflags subtract width64    udf allocation  udf buffer_size

df address set address 10
addi address stackpointer 0 shift_none noflags positive width64

df string_begin df string_length

df string_literal set string_literal 12
adr string_literal string_begin type_byteoffset

df i set i 5
mov i string_length shift_none type_zero width64
df loop at loop
	addi i i 1 shift_none setflags subtract width64
	df temp set temp 6 
	memr load_width64 temp string_literal i use_second64 1_byte
	memr store temp address i use_second64 1_byte udf temp
	bc is_nonzero loop udf loop udf i

df register_data set register_data 13
addi register_data address string_length shift_none noflags positive width64

df i set i 5
mov i 64 shift_none type_zero width64

df target set target 20
. mov target 4096 shift_none type_zero width64
mov target 4096 shift_16 type_keep width64
mov target 4096 shift_32 type_keep width64
mov target 4096 shift_48 type_keep width64
.


. choose your register you want to debug: . 
addi target stackpointer 0 shift_none noflags positive width64
. orr bitwise_or target linkregister zeroregister shift_increase shift_none regular_second width64 . 


df mask set mask 21  mov mask 1 shift_none type_zero width64
df digit set digit 6  mov digit '0' shift_none type_zero width64

df loop at loop
	addi i i 1 shift_none setflags subtract width64
	orr bitwise_and_setflags zeroregister target mask shift_increase shift_none regular_second width64
	csel 17 digit digit is_zero csel_incr csel_set width64
	memr store 17 register_data i use_second64 1_byte 
	addr mask zeroregister mask shift_increase 1 noflags positive width64
	cbz i loop is_nonzero width64
udf loop  udf i  
udf digit udf mask udf target

df final set final 6 
mov final newline shift_none type_zero width64	
memi store final register_data 64 1_byte 
udf final

df total_string_length set total_string_length string_length  add total_string_length 65

mov syscallarg0 stdout shift_none type_zero width64
orr bitwise_or syscallarg1 address zeroregister shift_increase shift_none regular_second width64
mov syscallarg2 total_string_length shift_none type_zero width64
mov syscallnumber system_write shift_none type_zero width64
svc

exit 1001


at string_begin   string  "debugging binary value for register = "
df end at end set string_length end udf end 
sub string_length string_begin

eoi




































. df 65 set 65 64 add 65 1
df 66 set 66 64 add 66 2
df 67 set 67 64 add 67 3
df x set x 66 si x 8 add x 65
df {66-65} set {66-65} x udf x
df x set x 10 si x 8 add x 67
df {10-67} set {10-67} x udf x . 

. mov data {66-65} shift_none type_zero width64
mov data {10-67} shift_16 type_keep width64 . 

. memi store data address 0 4_bytes .





. df x set x 64 incr x si x 8
or x 64 add x 2 si x 8
or x 64 incr x si x 8 add x 10
df {0x65-66-65-10} set {0x65-66-65-10} x udf x . 




df str1_begin
df str1_length



. addi 4 5 42 should_shift12 noflags positive width32 . 
. addx syscallarg1 stackpointer zeroregister addx_sp_lsl shift_none noflags positive width64 .     . use this if you want to put the stack pointer into arg1. . 
. orr bitwise_or syscallarg1 address zeroregister shift_increase shift_none regular_second width64 .     . this is effectively a mov (reg -> reg). . 




mov syscallarg0 stdout shift_none type_zero width64
adr syscallarg1 begin type_byteoffset
mov syscallarg2 length shift_none type_zero width64
mov syscallnumber system_write shift_none type_zero width64
svc	

df i set i 64
mov i 5 shift_none type_zero width64

df loop at loop
	addi i i 1 shift_none setflags subtract width64
	bc is_nonzero loop udf loop



mov syscallarg0 stdout shift_none type_zero width64

adr syscallarg1 begin type_byteoffset
mov syscallarg2 length shift_none type_zero width64
mov syscallnumber system_write shift_none type_zero width64
svc	


mov syscallarg0 stdout shift_none type_zero width64

addx syscallarg1 stackpointer zeroregister addx_sp_lsl shift_none noflags positive width64

mov syscallarg2 length shift_none type_zero width64
mov syscallnumber system_write shift_none type_zero width64
svc	

mov syscallnumber system_exit shift_none type_zero width64
mov syscallarg0 42 shift_none type_zero width64
svc

at begin string "register SP = " 
df end at end set length end udf end 
sub length begin


eoi































------------------------------------------------------------------------------------------

addi stackpointer stackpointer 16 shift_none noflags subtract width64

address = stackpointer;
address2 = stackpointer + 8;

------------------------------------------------------------------------------------------





sp usage from clang:
	
	100006f48: ff 83 01 d1 	sub	sp, sp, #96
	100006f4c: fa 67 01 a9 	stp	x26, x25, [sp, #16]
	100006f50: f8 5f 02 a9 	stp	x24, x23, [sp, #32]
	100006f54: f6 57 03 a9 	stp	x22, x21, [sp, #48]
	100006f58: f4 4f 04 a9 	stp	x20, x19, [sp, #64]
	100006f5c: fd 7b 05 a9 	stp	x29, x30, [sp, #80]


[sp = 0x100]
sp -= 64   [sp = 192
store r r -->  (sp + 8), (sp + 16)



------------------------------------------------------------------------------------------

allocating some stack memory:

	sp -= BYTE_COUNT;
	set addr = sp + BYTE_COUNT;

	... use addr ...
	
	sp += BYTE_COUNT

------------------------------------------------------------------------------------------

example mmap usage:
    addr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);


------------------------------------------------------------------------------------------
PROT_READ = 1
PROT_WRITE = 2
MAP_PRIVATE = 2
MAP_ANONYMOUS = 4096


------------------------------------------------------------------------------------------
{
	size = 4096
	permissions = PROT_READ | PROT_WRITE
	type = MAP_PRIVATE | MAP_ANONYMOUS
	
	addr = mmap(0, size, permissions, type, -1, 0);
	if (addr == -1) error();

	.. use addr ...

	r = munmap(addr, size);
	if (r == -1) error();
}

------------------------------------------------------------------------------------------









































