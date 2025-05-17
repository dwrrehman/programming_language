(1202504082.223747 dwrr)
ct set debug 1

ct set is_a_label 0
ct set byte 1
ct set exit 1
ct set write 01
ct set stdout 0
ct set _ 0




bc string_begin is_a_label

ct st string_begin 101 byte

ct set string_length 1



sc write stdout string_begin string_length  _ _ _

sc exit 101  _ _ _ _ _


halt




at string_begin

	(allocate 10 bytes...)

ct halt






okay so it seems we have a major problem in the language:

	basically, i want to allow the compiler to do math on labels, basically to allow you to compute a compiletime known pc-relative offset 

		and then like, either embed that into another instruction    (via an  auipc, or etc) 


			orrrr for you to instead  like   at compiletime, store data to that location in the .text section, 


		


butttt yeah this entire system is actually adding an INSANEEEE amount of complexity to the system as a whole... 


		soooo honestly   yeah i'm thinking of just ripping it out lol 



				basically, to acheive what we want to do here completely differently... hmmm





	basically, it kinda feels like the onlyyyyy real way that we could actually do this is by exposing the "la" instruction, 


		basically, to force the user to actaully first load a label's address into a regsiter, and then we could in theory do logic on it, to transform this into a byte offset? maybee???


	hmm




	like, i guess we could do 


		ct la pcrel   my_label 


	

	but like 


	then it still begs the question:  like, what is the ACTUAL VALUE   THAT IS GIVEN


		there isnt any 
lol 


		like,
			theres no sensical value that we could possibly give for  the ct variable  "pcrel" 


			because like, we would need to know the size of all instructions, leading up to that point, 

						(which we definitely don't have access to at this stage lol )


				because liek, we would need to know the actual concrete CTK   executable byte offset


					that represents   that labelllll's value 






		sooooooo yeah it basicallyyyyyyy seems like a genuinely impossible task lollll 


	its actually kind of a serious problem honestly lol 

	hm


				yeah 
						




		but like, the thing is, this is actually critical, because like, the ability for a programmer to locally put data next to the  RT instructions

			is like, super duperrrr useful, 


	and generally speaking, we should be able to manipulate the executable bytes in any way we want lol... hmmm





but yeah, i mean, i should say, it feels like we need to have some instruction who's job it is   to   like, put zero's or like uninitialized bytes 

		ie, we could have an instruction called "emitbytes k"

		or something like that lol 


	maybe just "emit"



and its always rt



but 

	yeah 


	


			and then basically, we would put this instruction after a label, 


			

				at label

					emit 0101           (emits 10 zeros into the executable at this point!) 




		



ohhhh wowwwwww



i see why this is such a hard problemmm wowwww





its becuase 
	now, 
			theres not just   compiletime known    and runtime known 





			THERES SOMETHING IN THE MIDDLEEEEE


							theres liek 



							"LATE_COMPILETIME KNOWN"





		like, 
			AND THATS DIFFERENT      THAN COMPILETIME KNOWN



	namely, 


	its transisitvely like the following:



		all compiletime known things are also known  at runtime,  and late compiletime, 


		and late compiletime things     are known at runtime 


		and runtime things        ARE ONLY known    at runtime. 






so yeah! 

	thats how it works 




gosh i hate this lol 


				LOLLLL



							i kinda just wish there was only two things... uhhh   hmmm



	like, 


	theres really no need to like... do all of this just to.. hmm

idkkkk


i feel like this is just a bit much lol.... hmmmm







but yeah, technically speaking, this would solve the problem lol. 


basically, we would need to delay   parts of compiletime computation/instructions      untillll  AFTERRRR all the lengths of instructions are known. 

	which is 




						need i say?


									stupid 



						and completely worthless lol    and we shouldnt even be going to a situation like that lol 

								"deferring CT instructions till AFTERRR ins sel and ins sched"




									NO
											NEVER






						bad









	not happening 






so yeah. i feel like we should just.. idk. like, find some other solution.  that doesnt involve adding a new  type of compiletime data variable, that involved defering computations till later lol. hmm



idk. i mean. 



i feel like theres definitely a solution probably. hmm




maybe we could just have a 



OH WAIT!





yeah, 
	okay if you think about it,   we can simplify this, 


		we can just make it so that 



			we allow the       la             runtime instruction 

		to take      a ct argument! 




	ie, 


		la myruntimevar mylabelvar mycompiletimevar





and the idea is 



	we compute  (label + ct_var)



and then THATSSSS the value we add to the program counter, via the rt la instruction!





coollllll 


	i kinda like that lol 



because like, realistically, what will we want to do with a label, 

				....

										we'll want to add a constant to it lol. 





									.... thats it. 




			thats all the possible computation we could possibly want to do loll.... 



	and then, like,  


the cool part 
	is 

		this now decouples the logic for determining this ct constant (which can be done using simple ct ins, simple ct execution,)


			from the actual        label   late-ct-executed    "pcrel offset"  part of it lol 



			ie, the final offset we embed in the executable, is the executable's offset that represents the label, 

										plus the ct constant!



		which, yeah 

	i think thats genuinely everything we could possibly want then! niceeee


	i love thisss



nice 

okay cool 

thats an easy fix lol 


it is currently 1202504082.233052







 


		

		


	

yayyy i'm happy we solved this problemmmm


