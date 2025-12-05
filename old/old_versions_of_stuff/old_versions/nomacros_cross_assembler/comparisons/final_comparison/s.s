. printing the iterator 
  of a loop in binary 
  1202503274.051234 dwrr
.

lf library/foundation.s

set _debugexecution false
set _targetarchitecture arm64_arch
set _outputformat macho_executable
set _shouldoverwrite true

df skip do skip

df4 remainder cat remainder
	df remainder set remainder arg0
	df quotient set quotient arg1
	df dividend set dividend arg2
	df divisor set divisor arg3	
	df ret set ret lr

	divr quotient dividend divisor divr_unsigned width64
	madd remainder quotient divisor dividend type_madd type_madd subtract width64

	incr ret set lr ret udf ret do lr

cat skip udf skip

df name df length

df loop df prime df composite df inner df done df init

df count set count 10001
df i set i 111
df j set j 0001
df i_value set i_value 1001 
df j_value set j_value 0101

movz count 1
movz i_value 01
movz i 0

at loop
	movz j 0
	at inner
		addi j_value j 01 shift_none noflags positive width64
		remainder 1111 0111 i_value j_value
		cbz 1111 composite is_zero width64
		addi j j 1 shift_none noflags positive width64
		addr zeroregister j i shift_none 0 setflags subtract width64
		bc is_not_equal inner

at prime		
	addi count count 1 shift_none noflags positive width64

at composite

	addi i i 1 shift_none noflags positive width64
	addi i_value i_value 1 shift_none noflags positive width64
	addi zeroregister i_value 0000001 shift_12 setflags subtract width64
	bc is_unsigned_less loop






. at prime
	addi count count 1 shift_none noflags positive width64
at composite
	addi i_value i_value 1 shift_none noflags positive width64
	addi i i 1 shift_none noflags positive width64
	addi zeroregister i_value 0000001 shift_12 setflags subtract width64
	bc is_equal done
at init	
	addi zeroregister i_value 11 shift_none setflags subtract width64
	bc is_unsigned_less prime
	movz j 0
at inner
	addi j_value j 01 shift_none noflags positive width64
	remainder 1111 0111 i_value j_value
	cbz 1111 composite is_zero width64
	addi j j 1 shift_none noflags positive width64
	addr zeroregister j i shift_none 0 setflags subtract width64
	bc is_not_equal inner
	jmp nolink prime
at done
. 


udf loop udf prime udf composite 
udf i udf j 
udf i_value udf j_value
udf done udf init

print name length count 1001 0101 1101 0011 1011
udf count

df greeting_begin df greeting_length
writestring stdout greeting_begin greeting_length
exit 0

at name string  "total number of primes = "
df end at end set length end udf end 
sub length name
udf name udf length

at greeting_begin string  "[done looping!]" emit 1 newline
df end at end set greeting_length end udf end 
sub greeting_length greeting_begin
udf greeting_begin udf greeting_length

eoi








. df limit set limit 011 mov limit 001 shift_16 type_zero width64 . 
