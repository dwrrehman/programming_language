
comment foundation include comment 


----- construct constants -----

	4 ctzero ctincr ctincr ctincr ctincr


c -1 c
	1 ctzero
	0 1 2 ctadd

c 0 c
	5 ctzero
	5 2 ctst4 
	4 2 2 ctadd

c 1 c
	5 ctzero ctincr
	5 2 ctst4 
	4 2 2 ctadd 

c 2 c
	5 ctzero ctincr ctincr ctincr ctincr
	5 2 ctst4 
	4 2 2 ctadd 

c 3 c
	5 ctzero 

				ctincr ctincr ctincr ctincr
				ctincr ctincr ctincr ctincr 
				ctincr ctincr ctincr ctincr 
				ctincr ctincr ctincr ctincr 

				ctincr ctincr ctincr ctincr 
				ctincr ctincr ctincr ctincr 
				ctincr ctincr ctincr ctincr ctincr 
				
	5 2 ctst4 
	4 2 2 ctadd 

c 4 c
	5 ctzero ctincr ctincr ctincr ctincr ctincr ctincr ctincr ctincr ctincr ctincr
	5 2 ctst4 
	4 2 2 ctadd 

c 5 c
	5 ctzero ctincr ctincr ctincr 
	5 5 5 ctshl 
	6 ctzero ctincr 
	6 5 5 ctshl 

	10 ctzero ctincr ctincr ctincr ctincr ctincr ctincr ctincr ctincr ctincr ctincr
	8 ctzero ctincr ctincr ctincr ctincr ctincr ctincr ctincr ctincr 
	8 10 9 ctshl 
	9 5 5 ctor

	5 ctincr ctincr ctincr

	5 2 ctst4 
	4 2 2 ctadd 

v 6 v
	5 ctzero ctincr ctincr 
	5 2 ctst4
	4 2 2 ctadd


---- call the write system call ----

	1 0 0 movzx
	  3 1 adr
	6 0 2 movzx
	2 0 16 movzw
	svc

---- call the exit system call ----

	0 0 0 movzx
	1 0 16 movzw
	svc


3 ctat
	5 dw






comment

	so uhhhh this code is kind of not that great lol

	its pretty messy 


		i think macros will make it better though, 

		and i think alot of it is just    we need to use a principled way of manipulating memory at compiletime. 

		like, i think its good that all compiletime constants are stored in memory, i think thats good... 

			its just then... we need to associate those ctmemory addresses with label names,   when we get macros. 

			very critical step.   asoc them with label names. 



		anyways, i also think that having the first 256 numbers baked in by having a loop at the beginning which uses the first 256 memory locations for the constants that they reperesent, is good. that will kinda solve the constant problem roughly. yup. i think so. cool. 


			because once you have a byte, you can just or them together for other constants. yay. 


			also writing them in hex is trivial, by simply using macros.   so yeah. this is solvable.  via macros and ct loops. 



		i think the only other outstanding issuses i see with this is     we need some way of getting the textual input from the file, 

				to be turned into file. thats critical. 

					or rather, it would be nice if we could just copy a range of the file into ctmemory, basically. 

					thats what we need. if we can do that, then we are fine, i think. 


									...once we implement macros, of course lol.  but yeah.  we're fine. 






			oh but strings stuff actaully comes after we implement macros,   beacuse we need a delimiter


					so yeah, thats important


				we'll do that   in that sequence. 



	yay


cool













comment



			


































asht

3 ctat
3 ctzero ctincr ctincr ctincr
1 ctzero ctincr ctincr
1 3 3 ctshl
0 3 12 ctadd
12 11 ctld4
4 11 11 ctmul
11 12 ctst4


asht



