(a macro that adds numbers using the rt add instruction in the lang isa, 
just for testing lol. written on 1202506194.032354 by dwrr)

ct do skip_macros 

at my_macro
	ct ld ra compiler_ctsc_number nat
	ld x compiler_ctsc_arg0 nat
	ld y compiler_ctsc_arg1 nat
	rt rep x rep y add x y del x del y 
	ct rep ra do ra del ra
	
at skip_macros 
del skip_macros


