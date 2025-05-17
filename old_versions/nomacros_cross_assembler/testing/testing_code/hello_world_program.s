. first working hello world program, written on 1202503097.051605 dwrr .
. (strings arent implemented yet, so i just emitted the characters individually lollll) .

lf foundation.s

set _targetarchitecture arm64_arch
set _outputformat macho_executable
set _shouldoverwrite true

mov syscallarg0 stdout shift_none type_zero width64
df begin adr syscallarg1 begin 0
df end df length set length end sub length begin
mov syscallarg2 length shift_none type_zero width64
mov syscallnumber system_write shift_none type_zero width64
svc

mov syscallnumber system_exit shift_none type_zero width64
mov syscallarg0 42 shift_none type_zero width64
svc

at begin

emit 1 'H'
emit 1 'e'
emit 1 'l'
emit 1 'l'
emit 1 'o'
emit 1 space
emit 1 't'
emit 1 'h'
emit 1 'e'
emit 1 'r'
emit 1 'e'
emit 1 '!'
emit 1 newline
emit 1 'T'
emit 1 'h'
emit 1 'i'
emit 1 's'
emit 1 space
emit 1 'i'
emit 1 's'
emit 1 space
emit 1 'm'
emit 1 'y'
emit 1 space
emit 1 'f'
emit 1 'i'
emit 1 'r'
emit 1 's'
emit 1 't'
emit 1 space

emit 1 'h'
emit 1 'e'
emit 1 'l'
emit 1 'l'
emit 1 'o'
emit 1 space

emit 1 'w'
emit 1 'o'
emit 1 'r'
emit 1 'l'
emit 1 'd'
emit 1 space

emit 1 'p'
emit 1 'r'
emit 1 'o'
emit 1 'g'
emit 1 'r'
emit 1 'a'
emit 1 'm'
emit 1 '.'
emit 1 newline

at end




