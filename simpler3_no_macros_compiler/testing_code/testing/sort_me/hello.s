(rewriting the prime number program in this version of the language! 
written on 1202505106.220939 dwrr)

lf library/ctsc.s


set lr 0
do skip


at macro

	sl "this is my cool string! lol"
	rt ctsc_callnumber ctsc_display
	set ctsc_arg0 0101
	rt ctsc_callnumber ctsc_write

	add lr 1 do lr


at skip ud skip



set i 0 
at loop
	ge i 101 done

	set ctsc_arg0 i
	rt ctsc_callnumber ctsc_print
	at lr do macro
	add i 1 do loop



at done	 ud i ud loop ud done
rt ctsc_callnumber ctsc_exit





