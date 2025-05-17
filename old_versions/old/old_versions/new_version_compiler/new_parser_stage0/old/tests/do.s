ct 0 zero 0
ct 1 zero 1 incr 1
ct 2 zero 2 incr 2 incr 2
ct 3 zero 3 incr 3 incr 3 incr 3

ct limit set limit 3
zero sum
zero bubbles
at loop
	ge bubbles limit done
	add sum bubbles
	incr bubbles do loop
at done


