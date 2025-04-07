
(testing runtime code generation via compiltime execution. 1202504067.225729 dwrr)

ct set debug 1



ct set i 0
ct set limit 0001

set count 00000001

at loop
	halt
	add i 1
	lt i limit loop

set x 001
set y 101
add x y
set y 0
set x 0



set i 0
at ct_loop
	set rti 0

	at inner
		add rti 1
		lt rti 001 inner ud

	add i 1 lt i 101 ct_loop

halt



