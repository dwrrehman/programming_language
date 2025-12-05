. a program to print out all ascii characters in numerical sequence to the screen. .
. written on 1202503097.051705 dwrr . 

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
df numberoftimes set numberoftimes 5
df e set e 0
df outerloop cat outerloop 
	df i set i space
	df charlimit set charlimit '~'
	df loop cat loop
		emit 1 i
		incr i
		ge charlimit i loop
	emit 1 newline
	incr e
	lt e numberoftimes outerloop
at end
eoi

