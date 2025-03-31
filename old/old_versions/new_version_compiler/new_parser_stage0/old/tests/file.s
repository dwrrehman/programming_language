

ct limit
zero limit
incr limit
incr limit
incr limit
incr limit
incr limit
incr limit
incr limit

ct i zero i
ct sum zero sum
rt variable limit zero variable

at loop
	ge i limit done
	add sum i
	add variable sum
	incr i do loop
at done 
incr variable

