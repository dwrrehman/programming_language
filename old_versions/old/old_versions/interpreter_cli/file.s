zero  ___1202501245.211208__written_by_dwrr____
zero  a_prime_number_program_which_counts_the_number_
zero  of_primes_less_than_a_given_number_"maxnumber".
zero  also..._yes,_i_havent_implemented_comments_yet,
zero  so_this_method_of_commenting_will_suffice_LOL



zero 0       zero these_definitions_would_be_in_an_included_library_file,_"lf_foundation"...
set 1 0 incr 1
set 2 1 incr 2
set 3 2 incr 3
set 4 3 incr 4
set 5 4 incr 5
set 6 5 incr 6
set 7 6 incr 7
set 8 7 incr 8
set 9 8 incr 9
set 10 9 incr 10
zero _
set 100 10 mul 100 10
set 1000 100 mul 1000 100



do main

at remainder                      zero ___this_function_computes_x_%_d___
	set x arg0  set d arg1

	set r x 
	div r d
	mul r d 
	sub r x

	set result r
do return


at main

set maxnumber 1000
mul maxnumber 5

zero i zero count
at loop
	ge i maxnumber done
	set j 2
	at inner
		ge j i prime
		set arg0 i  set arg1 j  do remainder at return
		eq result 0 composite
		incr j do inner

	at prime  incr count  sc 9 i _ _ _ _ _ 
	at composite 
	incr i do loop at done

sc 9 count _ _ _ _ _
sc 0 count _ _ _ _ _ 
eoi























