file library/core.s
file library/useful.s

st output_format no_output
(st overwrite_output false)
(str "my_new_output.txt" set_output_name)

zero i
at l
	set j 01
	at m
		lt i j prime eq i j prime
		set r i rem r j eq r 0 composite
		incr j eq 0 0 m
	at prime set c0 i ctprintbinary ctnl ctabort
	set c0 011 decrement
	at composite incr i lt i 0000001 l 
	del l del i

eoi

1202508144.004052

a compiletime-only program that prints out prime numbers in binary!
testing out the compiletime system a bit more!




