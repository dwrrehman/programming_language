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

do skip_routines

at ctprint 
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
	st compiler_arg3 c2 nat sc
	do ra del ra

at ctread
	ld ra 0 nat
	st compiler_arg0 compiler_system_read nat
	st compiler_arg1 c0 nat
	st compiler_arg2 c1 nat
	st compiler_arg3 c2 nat sc
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





at  skip_routines
del skip_routines

