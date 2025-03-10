.
  the foundation file for the language
  written on 1202503086.033529 by dwrr
.

df _targetarchitecture zero _targetarchitecture
df _outputformat zero _outputformat
df _shouldoverwrite zero _shouldoverwrite

df 0 zero 0
df 1 set 1 0 incr 1
df 2 set 2 1 incr 2
df 3 set 3 2 incr 3
df 4 set 4 3 incr 4
df 5 set 5 4 incr 5
df 6 set 6 5 incr 6
df 7 set 7 6 incr 7
df 8 set 8 7 incr 8
df 9 set 9 8 incr 9
df 10 set 10 9 incr 10
df 11 set 11 10 incr 11
df 12 set 12 11 incr 12
df 13 set 13 12 incr 13
df 14 set 14 13 incr 14
df 15 set 15 14 incr 15

df 16 set 16 1 si 16 4
df 32 set 32 1 si 32 5
df 64 set 64 1 si 64 6
df 128 set 128 1 si 128 7
df 256 set 256 1 si 256 8
df 512 set 512 1 si 512 9
df 1024 set 1024 1 si 1024 10
df 2048 set 2048 1 si 2048 11
df 4096 set 4096 1 si 4096 12

df 63 set 63 64 decr 63
df 30 set 30 3 mul 30 10
df 31 set 31 30 incr 31
df 42 set 42 10 si 42 2 add 42 2
df 65 set 65 64 incr 65
df 33 set 33 32 incr 33
df 48 set 48 12 si 48 2
df 97 set 97 32 add 97 65

df space set space 32
df newline set newline 10
df tab set tab 9
df exclaimation set exclaimation 33

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





df no_arch set no_arch 0
df arm64_arch set arm64_arch 1
df arm32_arch set arm32_arch 2
df rv64_arch set rv64_arch 3
df rv32_arch set rv32_arch 4


df stderr set stderr 2
df stdout set stdout 1
df stdin set stdin 0



. ...........arm64............. . 

df width64 set width64 1
df width32 set width32 0


. arm64 mov instruction .

df type_zero set type_zero 2
df type_neg set type_neg 0
df type_keep set type_keep 3

df shift_none set shift_none 0
df shift_16 set shift_16 1
df shift_32 set shift_32 2
df shift_48 set shift_48 3




. svc : system calls on macos .

df syscallnumber set syscallnumber 16
df syscallarg0 set syscallarg0 0
df syscallarg1 set syscallarg1 1
df syscallarg2 set syscallarg2 2
df syscallarg3 set syscallarg3 3

df system_exit 	set system_exit 1
df system_fork 	set system_fork 2
df system_read 	set system_read 3
df system_write set system_write 4
df system_open 	set system_open 5
df system_close set system_close 6


. useful register indexes .
df linkregister set linkregister 30
df stackpointer set stackpointer 31
df zeroregister set zeroregister 31


. branch conditions .
df is_equal set is_equal 0
df is_not_equal set is_not_equal 1
df is_unsigned_higher_or_same set is_unsigned_higher_or_same 2
df is_unsigned_greater_or_equal set is_unsigned_greater_or_equal 2
df is_unsigned_lower set is_unsigned_lower 3
df is_unsigned_less set is_unsigned_less 3
df is_minus set is_minus 4
df is_negative set is_negative 4
df is_nonnegative set is_nonnegative 5
df has_overflow_set set has_overflow_set 6
df has_overflow_clear set has_overflow_clear 7
df is_unsigned_greater set is_unsigned_greater 8
df is_unsigned_lower_or_same set is_unsigned_lower_or_same 9
df is_signed_greater_or_equal set is_signed_greater_or_equal 10
df is_signed_less set is_signed_less 11
df is_signed_greater set is_signed_greater 12
df is_signed_less_or_equal set is_signed_less_or_equal 13
df cond_always set cond_always 14
df cond_never set cond_never 15


df with_register set with_register 0
df with_immediate set with_immediate 1

df is_zero set is_zero 0
df is_nonzero set is_nonzero 1

df type_byteoffset set type_byteoffset 0
df type_pageoffset set type_pageoffset 1

df isreturn set isreturn 1
df isnotreturn set isnotreturn 0
df nolink set nolink 0
df jumplink set jumplink 1

df shift_increase set shift_increase 0
df shift_decrease set shift_decrease 1
df shift_signed_decrease set shift_signed_decrease 2
df rotate_decrease set rotate_decrease 3

df positive set positive 0
df positive set positive 0
df subtract set subtract 1

df noflags  set noflags  0
df setflags set setflags 1

df shift_12 set shift_12 1

df cmp_negative set cmp_negative 0
df cmp_positive set cmp_positive 1


df extend_uxtb set extend_uxtb 0
df extend_uxth set extend_uxth 1
df extend_uxtw set extend_uxtw 2
df extend_uxtx set extend_uxtx 3 df addx_sp_lsl set addx_sp_lsl 3
df extend_sxtb set extend_sxtb 4
df extend_sxth set extend_sxth 5
df extend_sxtw set extend_sxtw 6
df extend_sxtx set extend_sxtx 7

df csel_incr set csel_incr 1
df csel_not set csel_not 1
df csel_set set csel_set 0

df maddl_unsigned set maddl_unsigned 1
df maddl_signed set maddl_signed 0
df type_maddl set type_maddl 1
df type_madd set type_madd 0







