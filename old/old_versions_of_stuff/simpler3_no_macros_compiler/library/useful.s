(useful.s : some useful compiletime routine definitions 
that can be used during CTE.
written on 1202507141.125219 by dwrr)

(this file depends on: 

	library/foundation.s
)

ct

set c0 c0 
set c1 c1
set c2 c2
set c3 c3
set c4 c4
set c5 c5
set c6 c6
set c7 c7

set ctsp 0

do skip_routines



at interpretted
	ld ra 0 nat
	st compiler_target no_arch nat
	st compiler_format no_output nat
	do ra del ra


at ctdebug
	ld ra 0 nat
	st compiler_arg0 compiler_system_debug nat
	st compiler_arg1 c0 nat sc
	do ra del ra


at ctabort 
	ld ra 0 nat
	st compiler_arg0 -1 nat sc      (temporary fix for now... lol)
	do ra del ra


at ctexit
	ld ra 0 nat
	st compiler_arg0 compiler_system_exit nat
	st compiler_arg1 c0 nat sc
	do ra del ra


at ctwrite
	ld ra 0 nat
	st compiler_arg0 compiler_system_write nat
	st compiler_arg1 c0 nat
	st compiler_arg2 c1 nat
	st compiler_arg3 c2 nat 
	sc
	ld c0 compiler_arg1 nat
	ld c1 compiler_arg2 nat
	do ra del ra


at ctread
	ld ra 0 nat
	st compiler_arg0 compiler_system_read nat
	st compiler_arg1 c0 nat
	st compiler_arg2 c1 nat
	st compiler_arg3 c2 nat 
	sc
	ld c0 compiler_arg1 nat
	ld c1 compiler_arg2 nat
	do ra del ra


at ctallocatepages
	ld ra 0 nat
	
	set permissions prot_read or permissions prot_write
	set flags map_private or flags map_anonymous
	set allocation_size 0000_0000_0000_1
	mul allocation_size c0

	st compiler_arg0 compiler_system_mmap nat
	st compiler_arg1 0 nat
	st compiler_arg2 allocation_size nat del allocation_size
	st compiler_arg3 permissions nat del permissions
	st compiler_arg4 flags nat del flags
	st compiler_arg5 -1 nat
	st compiler_arg6 0 nat
	sc

	ld error  compiler_arg2 nat
	ld buffer compiler_arg1 nat

	ne buffer -1 s
		set c0 compiler_system_mmap do ctprint
		set c0 error do ctprint
		do ctabort
	at s del s

	set c0 buffer del buffer del error
	do ra del ra


at ctdeallocatepages
	ld ra 0 nat

	set allocation_size 0000_0000_0000_1
	mul allocation_size c1

	st compiler_arg0 compiler_system_munmap nat
	st compiler_arg1 c0 nat
	st compiler_arg2 allocation_size nat del allocation_size
	sc

	ld error compiler_arg2 nat
	ld r compiler_arg1 nat

	ne r -1 s del r
		set c0 compiler_system_munmap do ctprint
		set c0 error do ctprint del error
		do ctabort
	at s del s
	do ra del ra


at init_ctsp
	ld ra 0 nat

	ne ctsp 0 skip
	set c0 1 do ctallocatepages set ctsp c0
	at skip del skip
	
	do ra del ra



at cthello
	ld ra 0 nat 
	do init_ctsp

	set p ctsp
	st p 'h' byte add p 1
	st p 'e' byte add p 1
	st p 'l' byte add p 1
	st p 'l' byte add p 1
	st p 'o' byte add p 1
	st p '!' byte add p 1
	st p newline byte add p 1

	set length p sub length ctsp del p

	set c0 stdout
	set c1 ctsp
	set c2 length del length
	do ctwrite

	do ra del ra




at ctbinary
	ld ra 0 nat 
	set n c0
	do init_ctsp

	set string ctsp
	set p string
	at loop
		set bit n and bit 1 add bit '0'
		st p bit byte add p 1
		sd n 1 ne n 0 loop del loop del n
	set length p sub length string del p

	set c0 stdout
	set c1 string del string 
	set c2 length del length
	do ctwrite

	do ra del ra



at ctnl
	ld ra 0 nat
	do init_ctsp
	st ctsp newline byte

	set c0 stdout
	set c1 ctsp
	set c2 1
	do ctwrite

	do ra del ra


at ctclearscreen
	ld ra 0 nat
	do init_ctsp

	set p ctsp
	st p escape byte add p 1
	st p '[' byte add p 1
	st p 'H' byte add p 1
	st p escape byte add p 1
	st p '[' byte add p 1
	st p '2' byte add p 1
	st p 'J' byte add p 1

	set length p sub length ctsp del p	
	set c0 stdout
	set c1 ctsp
	set c2 length del length
	do ctwrite

	do ra del ra






at  skip_routines
del skip_routines

