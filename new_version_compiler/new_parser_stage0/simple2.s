ct 5 zero 5 incr 5 incr 5

zero i 
zero sum

at loop
	ge i 5 done
	add sum i
	incr i 
	ge sum sum loop

at done
incr sum




