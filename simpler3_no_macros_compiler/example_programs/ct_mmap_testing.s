(
	this is a test of using the language just as an interpreted language, 
	in this file, i'm testing out the allocation and deallocation routines
	defined in useful.s, which call mmap and munmap system calls at ct. 
	written on 1202507141.042328 --> 1202507141.143013  
	written by dwrr
)

ct 

file library/foundation.s
file library/ascii.s
file library/useful.s

st compiler_target no_arch nat
st compiler_format no_output nat

set page_count 01
set c0 page_count
do ctallocatepages 
set buffer c0
do ctdebug

do cthello
set c0 ctsp do ctdebug 
do ctbinary do ctnl

set c0 buffer 
set c1 page_count
do ctdeallocatepages

set c0 0 do ctexit




















(do cthello

at loop

	set c0 101 do ctbinary do ctnl

	do loop

set c0 0 do ctexit)














(




(set page_count 01
set c0 page_count
do ctallocatepages 
set buffer c0)

do cthello

set c0 ctsp 

do ctprint 
do ctbinary 
do ctnl


(set c0 buffer 
set c1 page_count
do ctdeallocatepages)

set c0 0 do ctexit






set p buffer
st p 'H' byte add p 1
st p 'E' byte add p 1
st p 'L' byte add p 1
st p 'L' byte add p 1
st p 'O' byte add p 1
st p space byte add p 1
st p 'T' byte add p 1
st p 'H' byte add p 1
st p 'E' byte add p 1
st p 'R' byte add p 1
st p 'E' byte add p 1
st p '!' byte add p 1
st p newline byte add p 1

set length p sub length buffer

st compiler_arg0 compiler_system_write nat
st compiler_arg1 1 nat
st compiler_arg2 buffer nat
st compiler_arg3 length nat
sc


set input p
st compiler_arg0 compiler_system_read nat
st compiler_arg1 0 nat
st compiler_arg2 input nat
st compiler_arg3 1 nat
sc

add p 1
st p newline byte add p 1


set i 0
at loop
	st compiler_arg0 compiler_system_write nat
	st compiler_arg1 1 nat
	st compiler_arg2 input nat
	st compiler_arg3 01 nat
	sc
	add i 1 lt i 101 loop


st compiler_arg0 compiler_system_exit nat
st compiler_arg1 0 nat 
sc






)






























(

set i 0 at loop
	st compiler_arg0 compiler_system_debug nat
	st compiler_arg1 i nat sc
	add i 1 lt i 1001 loop del i del loop


)


















(
1202507141.042743
	lets try to use mmap ctsc!

	lets write the impl in the compiler now to use statically known memory addresses now 
	instead of the memory[] array that we are using currently in cte!

)













