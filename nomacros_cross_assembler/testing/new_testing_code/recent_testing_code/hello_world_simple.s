. first working hello world program with string literals .
.     written on 1202503167.193448 by dwrr     .
. updated on 1202503204.043015 to use macros, and binary literals. . 

lf foundation.s
set _targetarchitecture arm64_arch
set _outputformat macho_executable
set _shouldoverwrite true

df begin df length 
writestring stdout begin length
exit 0011

at begin string 

"hello there from space! :) 
...this is my cool string lol.
its kind of interesting because
it can easily be mulitple lines!
and can contain tabs and unicode too lol!
	such as ∑, 	for example lol. 
so yeah. thats string!"  

emit 1 newline 
df end at end set length end udf end sub length begin

eoi
