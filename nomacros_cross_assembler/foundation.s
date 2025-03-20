.
  the foundation file for the language
  written on 1202503086.033529 by dwrr
.

df _targetarchitecture zero _targetarchitecture
df _outputformat zero _outputformat
df _shouldoverwrite zero _shouldoverwrite

df lr 
df  arg0 df  arg1 df  arg2 df  arg3
df  arg4 df  arg5 df  arg6 df  arg7
df  arg8 df  arg9 df arg10 df arg11
df arg12 df arg13 df arg14 df arg15



df 0 bn 0 0
df 1 bn 1 1
df 01 bn 01 01
df 11 bn 11 11
df 001 bn 001 001
df 101 bn 101 101
df 011 bn 011 011
df 111 bn 111 111
df 0001 bn 0001 0001
df 1001 bn 1001 1001
df 0101 bn 0101 0101
df 1101 bn 1101 1101
df 0011 bn 0011 0011
df 1011 bn 1011 1011
df 0111 bn 0111 0111
df 1111 bn 1111 1111
df 00001 bn 00001 00001
df 10001 bn 10001 10001
df 01001 bn 01001 01001
df 11001 bn 11001 11001
df 00101 bn 00101 00101
df 10101 bn 10101 10101
df 01101 bn 01101 01101
df 11101 bn 11101 11101
df 00011 bn 00011 00011
df 10011 bn 10011 10011
df 01011 bn 01011 01011
df 11011 bn 11011 11011
df 00111 bn 00111 00111
df 10111 bn 10111 10111
df 01111 bn 01111 01111
df 11111 bn 11111 11111
df 000001 bn 000001 000001
df 0000001 bn 0000001 0000001
df 00000001 bn 00000001 00000001




df false bn false 0
df true bn true 1

df space 	bn space 000001
df newline 	bn newline 0101
df tab 		bn tab 1001

. editor generator for ascii: 
	auncauwpiiiiseiiiiiiiiseiiiiseiiiiiiiiisep 
.

df '!' set '!' space incr '!'
df '"' set '"' '!' incr '"'
df '#' set '#' '"' incr '#'
df '$' set '$' '#' incr '$'
df '%' set '%' '$' incr '%'
df '&' set '&' '%' incr '&'
df ''' set ''' '&' incr '''
df '(' set '(' ''' incr '('
df ')' set ')' '(' incr ')'
df '*' set '*' ')' incr '*'
df '+' set '+' '*' incr '+'
df ',' set ',' '+' incr ','
df '-' set '-' ',' incr '-'
df '.' set '.' '-' incr '.'
df '/' set '/' '.' incr '/'
df '0' set '0' '/' incr '0'
df '1' set '1' '0' incr '1'
df '2' set '2' '1' incr '2'
df '3' set '3' '2' incr '3'
df '4' set '4' '3' incr '4'
df '5' set '5' '4' incr '5'
df '6' set '6' '5' incr '6'
df '7' set '7' '6' incr '7'
df '8' set '8' '7' incr '8'
df '9' set '9' '8' incr '9'
df ':' set ':' '9' incr ':'
df ';' set ';' ':' incr ';'
df '<' set '<' ';' incr '<'
df '=' set '=' '<' incr '='
df '>' set '>' '=' incr '>'
df '?' set '?' '>' incr '?'
df '@' set '@' '?' incr '@'
df 'A' set 'A' '@' incr 'A'
df 'B' set 'B' 'A' incr 'B'
df 'C' set 'C' 'B' incr 'C'
df 'D' set 'D' 'C' incr 'D'
df 'E' set 'E' 'D' incr 'E'
df 'F' set 'F' 'E' incr 'F'
df 'G' set 'G' 'F' incr 'G'
df 'H' set 'H' 'G' incr 'H'
df 'I' set 'I' 'H' incr 'I'
df 'J' set 'J' 'I' incr 'J'
df 'K' set 'K' 'J' incr 'K'
df 'L' set 'L' 'K' incr 'L'
df 'M' set 'M' 'L' incr 'M'
df 'N' set 'N' 'M' incr 'N'
df 'O' set 'O' 'N' incr 'O'
df 'P' set 'P' 'O' incr 'P'
df 'Q' set 'Q' 'P' incr 'Q'
df 'R' set 'R' 'Q' incr 'R'
df 'S' set 'S' 'R' incr 'S'
df 'T' set 'T' 'S' incr 'T'
df 'U' set 'U' 'T' incr 'U'
df 'V' set 'V' 'U' incr 'V'
df 'W' set 'W' 'V' incr 'W'
df 'X' set 'X' 'W' incr 'X'
df 'Y' set 'Y' 'X' incr 'Y'
df 'Z' set 'Z' 'Y' incr 'Z'
df '[' set '[' 'Z' incr '['
df '\' set '\' '[' incr '\'
df ']' set ']' '\' incr ']'
df '^' set '^' ']' incr '^'
df '_' set '_' '^' incr '_'
df '`' set '`' '_' incr '`'
df 'a' set 'a' '`' incr 'a'
df 'b' set 'b' 'a' incr 'b'
df 'c' set 'c' 'b' incr 'c'
df 'd' set 'd' 'c' incr 'd'
df 'e' set 'e' 'd' incr 'e'
df 'f' set 'f' 'e' incr 'f'
df 'g' set 'g' 'f' incr 'g'
df 'h' set 'h' 'g' incr 'h'
df 'i' set 'i' 'h' incr 'i'
df 'j' set 'j' 'i' incr 'j'
df 'k' set 'k' 'j' incr 'k'
df 'l' set 'l' 'k' incr 'l'
df 'm' set 'm' 'l' incr 'm'
df 'n' set 'n' 'm' incr 'n'
df 'o' set 'o' 'n' incr 'o'
df 'p' set 'p' 'o' incr 'p'
df 'q' set 'q' 'p' incr 'q'
df 'r' set 'r' 'q' incr 'r'
df 's' set 's' 'r' incr 's'
df 't' set 't' 's' incr 't'
df 'u' set 'u' 't' incr 'u'
df 'v' set 'v' 'u' incr 'v'
df 'w' set 'w' 'v' incr 'w'
df 'x' set 'x' 'w' incr 'x'
df 'y' set 'y' 'x' incr 'y'
df 'z' set 'z' 'y' incr 'z'
df '{' set '{' 'z' incr '{'
df '|' set '|' '{' incr '|'
df '}' set '}' '|' incr '}'
df '~' set '~' '}' incr '~'


. _targetarchitecture: supported architectures . 
df no_arch 	bn  no_arch 	0
df arm64_arch 	bn  arm64_arch  1
df arm32_arch 	bn  arm32_arch  01
df rv64_arch 	bn  rv64_arch   11
df rv32_arch 	bn  rv32_arch   001
df msp430_arch 	bn  msp430_arch 101

. _outputformat: supported output formats . 
df debug_output_only 	bn  debug_output_only  0
df macho_executable 	bn  macho_executable   1
df elf_executable 	bn  elf_executable     01
df ti_txt_executable 	bn  ti_txt_executable  11

. file descriptors used in system calls .
df stdin bn stdin   0
df stdout bn stdout 1
df stderr bn stderr 01

. ...........arm64............. . 

df width64 bn width64 1
df width32 bn width32 0

df type_zero bn type_zero 01
df type_neg  bn type_neg  0
df type_keep bn type_keep 11

df shift_none bn shift_none 0
df shift_16   bn shift_16 1
df shift_32   bn shift_32 01
df shift_48   bn shift_48 11

. system calls on macos .
df syscallnumber bn syscallnumber 00001
df syscallarg0 	 bn syscallarg0 0
df syscallarg1   bn syscallarg1 1
df syscallarg2   bn syscallarg2 01
df syscallarg3   bn syscallarg3 11
df syscallarg4   bn syscallarg4 001

df system_exit 	bn system_exit  1
df system_fork 	bn system_fork  01
df system_read 	bn system_read  11
df system_write bn system_write 001
df system_open 	bn system_open  101
df system_close bn system_close 101

. register indexes .
df linkregister bn linkregister 01111
df stackpointer bn stackpointer 11111
df zeroregister bn zeroregister 11111

. branch conditions .
df is_equal 
bn is_equal 0
df is_not_equal 
bn is_not_equal 1
df is_unsigned_higher_or_same 
bn is_unsigned_higher_or_same 01
df is_unsigned_greater_or_equal 
bn is_unsigned_greater_or_equal 01
df is_unsigned_lower 
bn is_unsigned_lower 11
df is_unsigned_less 
bn is_unsigned_less 11
df is_minus 
bn is_minus 001
df is_negative 
bn is_negative 001
df is_nonnegative 
bn is_nonnegative 101
df has_overflow_set 
bn has_overflow_set 011
df has_overflow_clear 
bn has_overflow_clear 111
df is_unsigned_greater 
bn is_unsigned_greater 0001
df is_unsigned_lower_or_same 
bn is_unsigned_lower_or_same 1001
df is_signed_greater_or_equal 
bn is_signed_greater_or_equal 0101
df is_signed_less 
bn is_signed_less 1101
df is_signed_greater 
bn is_signed_greater 0011
df is_signed_less_or_equal 
bn is_signed_less_or_equal 1011
df cond_always 
bn cond_always 0111
df cond_never 
bn cond_never 1111

df with_register bn with_register 0
df with_immediate bn with_immediate 1

df is_zero bn is_zero 0           . valid with cond arguments .
df is_nonzero bn is_nonzero 1     . valid with cond arguments .

df type_byteoffset bn type_byteoffset 0
df type_pageoffset bn type_pageoffset 1

df isreturn bn isreturn 1
df isnotreturn bn isnotreturn 0
df nolink bn nolink 0
df jumplink bn jumplink 1

df shift_increase bn shift_increase 0
df shift_decrease bn shift_decrease 1
df shift_signed_decrease bn shift_signed_decrease 01
df rotate_decrease bn rotate_decrease 11

df positive bn positive 0
df positive bn positive 0
df subtract bn subtract 1

df noflags  bn noflags  0
df setflags bn setflags 1

df shift_12 bn shift_12 1

df cmp_negative bn cmp_negative 0
df cmp_positive bn cmp_positive 1

df extend_uxtb bn extend_uxtb 0
df extend_uxth bn extend_uxth 1
df extend_uxtw bn extend_uxtw 01
df extend_uxtx bn extend_uxtx 11 df addx_sp_lsl bn addx_sp_lsl 11
df extend_sxtb bn extend_sxtb 001
df extend_sxth bn extend_sxth 101
df extend_sxtw bn extend_sxtw 011
df extend_sxtx bn extend_sxtx 111

df csel_incr bn csel_incr 1
df csel_not bn csel_not 1
df csel_set bn csel_set 0

df maddl_unsigned bn maddl_unsigned 1
df maddl_signed bn maddl_signed 0
df type_maddl bn type_maddl 1
df type_madd bn type_madd 0

df bfm_signed_zero bn bfm_signed_zero 0
df bfm_keep bn bfm_keep 1
df bfm_unsigned_zero bn bfm_unsigned_zero 01

df regular_second bn regular_second 0
df invert_second bn invert_second 1

df bitwise_and bn bitwise_and 0
df bitwise_or bn bitwise_or 1
df bitwise_eor bn bitwise_eor 01
df bitwise_and_setflags bn bitwise_and_setflags 11

df store bn store 0
df load_width32 bn load_width32 001
df load_width64 bn load_width64 101
df load_signed_width32 bn load_signed_width32 011
df load_signed_width64 bn load_signed_width64 111

df 1_byte bn 1_byte 0
df 2_bytes bn 2_bytes 1
df 4_bytes bn 4_bytes 01
df 8_bytes bn 8_bytes 11

df zero_extend_second32 
bn zero_extend_second32 0
df use_second64 
bn use_second64 1
df sign_extend_second32 
bn sign_extend_second32 01
df sign_extend_second64 
bn sign_extend_second64 11
df scale_then_zero_extend_second32 
bn scale_then_zero_extend_second32 001
df scale_second64 
bn scale_second64 101
df scale_then_sign_extend_second32 
bn scale_then_sign_extend_second32 011
df scale_then_sign_extend_second64 
bn scale_then_sign_extend_second64 111

df store_pair bn store_pair 0
df load_pair bn load_pair 1
df pair_width32 bn pair_width32 0
df pair_width64 bn pair_width64 01
df nontemporal_pair bn nontemporal_pair 0
df post_advance_pair bn post_advance_pair 1
df offset_pair bn offset_pair 01
df pre_advance_pair bn pre_advance_pair 11

df pre_advance bn pre_advance 1
df post_advance bn post_advance 0





. useful macros! : arm64 . 

df skip do skip

df2 movz cat movz
	df ret set ret lr
	mov arg0 arg1 shift_none type_zero width64
	incr ret set lr ret udf ret do lr

df1 exit cat exit
	df ret set ret lr
	df exitcode 
	set exitcode arg0

	movz syscallnumber system_exit
	movz syscallarg0 exitcode
	svc halt

	udf exitcode
	incr ret set lr ret udf ret do lr

df3 writestring cat writestring

	df ret set ret lr
	df fd set fd arg0
	df string_address set string_address arg1
	df string_length set string_length arg2

	movz syscallarg0 fd
	adr syscallarg1 string_address type_byteoffset
	movz syscallarg2 string_length
	movz syscallnumber system_write
	svc

	udf fd 
	udf string_address
	udf string_length
	incr ret set lr ret udf ret do lr

cat skip udf skip

eoi



