(testing out functions in the language (aka compiletime macros lol) 1202505106.141237)

rt sum 0
set sum 0

set lr 0
set a0 0

do skip

at mymacro
	a6_nop
	add sum a0
	sc
	
	add lr 1 do lr

at skip


set a0 101 at lr do mymacro

set a0 11 at lr do mymacro

set a0 01 at lr do mymacro

set a0 0 at lr do mymacro


halt
