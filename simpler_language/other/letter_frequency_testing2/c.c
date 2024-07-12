/*
1202406252.230309
a program to tally up the letter frequencies that are used in the language isa's spellings. 
	useful for knowing how similar it is to the english language, 
	and whether we should respell some instructions to give a more even distribution, 
	basically. 




warning: include location '/usr/local/include' is unsafe for cross-compilation [-Wpoison-system-directories]
c.c:39:73: warning: format specifies type 'unsigned long long' but the argument has type 'unsigned long' [-Wformat]
        printf("showing letter frequency used in this set of %llu words...\n", sizeof words / sizeof *words);
                                                             ~~~~              ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                                             %lu
c.c:41:28: warning: format specifies type 'int' but the argument has type 'unsigned long long' [-Wformat]
                printf(" %c : %3llu : ", 'a' + i, counts[i]);
                         ~~              ^~~~~~~
                         %llu
3 warnings generated.


showing letter frequency used in this set of 38 words...
 a :   6 : ######
 b :  12 : ############
 c :   1 : #
 d :  11 : ###########
 e :  10 : ##########
 f :   0 : 
 g :   2 : ##
 h :   4 : ####
 i :   3 : ###
 j :   0 : 
 k :   0 : 
 l :  15 : ###############
 m :   5 : #####
 n :   4 : ####
 o :   2 : ##
 p :   0 : 
 q :   1 : #
 r :   7 : #######
 s :  19 : ###################
 t :  10 : ##########
 u :   5 : #####
 v :   2 : ##
 w :   2 : ##
 x :   0 : 
 y :   0 : 
 z :   0 : 
done


1202406252.230813
	should we figure out how to sort the output?.. 
		not sure lol. its not tooimprotant i guess lol. 
			i guess instead of using the letter odring, we would just descend in values oft he bar... that would work lol. ill do that real quick. 





okay lets see if that worked lol 




warning: include location '/usr/local/include' is unsafe for cross-compilation [-Wpoison-system-directories]
c.c:99:73: warning: format specifies type 'unsigned long long' but the argument has type 'unsigned long' [-Wformat]
        printf("showing letter frequency used in this set of %llu words...\n", sizeof words / sizeof *words);
                                                             ~~~~              ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
                                                             %lu
c.c:103:30: warning: format specifies type 'int' but the argument has type 'unsigned long long' [-Wformat]
                                printf(" %c : %3llu : ", 'a' + l, counts[l]);
                                         ~~              ^~~~~~~
                                         %llu
3 warnings generated.


showing letter frequency used in this set of 38 words...
 s :  19 : ###################
 l :  15 : ###############
 b :  12 : ############
 d :  11 : ###########
 e :  10 : ##########
 t :  10 : ##########
 r :   7 : #######
 a :   6 : ######
 m :   5 : #####
 u :   5 : #####
 h :   4 : ####
 n :   4 : ####
 i :   3 : ###
 g :   2 : ##
 o :   2 : ##
 v :   2 : ##
 w :   2 : ##
 c :   1 : #
 q :   1 : #
 f :   0 : 
 j :   0 : 
 k :   0 : 
 p :   0 : 
 x :   0 : 
 y :   0 : 
 z :   0 : 
done

				NICEEEE
		cool


		that wasnt too bad lololol 

				so yeah, i think we kinda know what letters are used a lot now lol. 



			s, l,   and b     for some reason 


				i think we should probably try to make b used less... becuase its not really supposed to be used that much lolol 

					i think we can probably rename the branches to just      lt   ge 



			generally speaking, i think i want to try to make most of the symbols in the language not 3 characetrs, but 2.. 
				i think thats going to be better. 

					so yeah.   i kinda have to be careful in doing so, becuase whatever i define here, cannot be defined by the user at all, 

								well,, i mean, you can define those variables, but you can't use them in anyway lolol 

									because the instruction op code names are checked beforeeeee we check the variable name dictionary. so yeah. 





	ill draft something out soon i think, about the new naming of the symbols of the language isa. ill write it out in its own tb probably. so yeah. 



cool beans 



1202406252.231701 


dwrr









using the new isa:         (ONLY 32 INSTRUCTIONS!! wow!!!)


	static const char* words[] = {
		"sign", "ecall", "use", "at",  "rn",  "ar",  "al",  "la", 
		"lt",  "ge",  "ne",  "eq",  "mul", "mh",  "div", "rem", 	
		"add", "sub", "slt", "and", "or",  "eor", "sl",  "sr",
		"lb",  "lh",  "lw",  "ld",  "sb",  "sh",  "sw",  "sd",
	};



we get this:


showing letter frequency used in this set of 32 words...
 l :  12 : ############
 s :  10 : ##########
 a :   7 : #######
 e :   7 : #######
 d :   6 : ######
 r :   6 : ######
 n :   4 : ####
 b :   3 : ###
 h :   3 : ###
 m :   3 : ###
 t :   3 : ###
 u :   3 : ###
 g :   2 : ##
 i :   2 : ##
 o :   2 : ##
 w :   2 : ##
 c :   1 : #
 q :   1 : #
 v :   1 : #
 f :   0 : 
 j :   0 : 
 k :   0 : 
 p :   0 : 
 x :   0 : 
 y :   0 : 
 z :   0 : 
done






				but wait!


		lets add the atomic operations now too!!! thats imorportant....


			

			lr,   sc,        swpa         adda          anda     ora     eora     maxa      mina   




	yeah so thats like       9  extra instructions i think?.. not too bad 



						...eventuallyyyyy we will have to add floating point stuff to this language too, but we'll hold off on that for a while lololol 


							so yeah
 			


			lr, sc, swpa, adda, anda, ora, eora, maxa, mina          thats what we are adding  to the isa 



	
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* words[] = {
//	"ss", "env", "use", "at",  "rn",  "dr",  "do",  "la", 
//	"lt",  "ge",  "ne",  "eq",  "mul", "muh",  "div", "rem",
//	"add", "sub", "slt", "and", "or",  "eor", "sl",  "sr",
//	"lb",  "lh",  "lw",  "ld",  "sb",  "sh",  "sw",  "sd",
//	"alr", "asc", "swap", "aadd", "aand", "aor", "aeor", "amax", "amin"

	"ms", "ec", "use", "at", "rn", "dr", "do", 
	"lb", "lh", "lw", "ld", "sb", "sh", "sw", "sd",
	"add", "sub", "slt", "and", "or", "eor", "sl", "sr",
	"lt", "ge", "ne", "eq", "mul", "muh", "div", "rem",
	"alr", "asc", "aswp", "aadd", "aand", "aor", "aeor", "amax", "amin",


};


/*



some example code:



			at divide
				div ss c b a
				dr ra

			do divide ra
			add arga oololllol
			add argn l 
			env



i think this syntax for 

	dr, do, and the atomic operations and la   is fine    oh and for    set-signed   to     (ss) 

		i feel like this could work. i think.
	





heres the distribution of the letters now:





showing letter frequency used in this set of 41 words...
 a :  16 : ################
 s :  13 : #############
 d :  11 : ###########
 l :  10 : ##########
 r :   9 : #########
 e :   8 : ########
 n :   6 : ######
 m :   5 : #####
 o :   5 : #####
 u :   4 : ####
 b :   3 : ###
 h :   3 : ###
 t :   3 : ###
 w :   3 : ###
 i :   2 : ##
 v :   2 : ##
 c :   1 : #
 g :   1 : #
 p :   1 : #
 q :   1 : #
 x :   1 : #
 f :   0 : 
 j :   0 : 
 k :   0 : 
 y :   0 : 
 z :   0 : 
done







					a is now the number one   lolol   just from adding the atomic set lollll

					thats okay i guess lol 




		so yeah 


	i think this will be the language 



	lets make the compiler use this language now!


			shouldnt be too difficult lol 

										only 41 instructions, including  the M extension, and A extensions!!!

											niceee



	"ss", "env", "use", "at",  "rn",  "dr",  "do",  "la", 
	"lt",  "ge",  "ne",  "eq",  "mul", "muh",  "div", "rem",
	"add", "sub", "slt", "and", "or",  "eor", "sl",  "sr",
	"lb",  "lh",  "lw",  "ld",  "sb",  "sh",  "sw",  "sd",
	"alr", "asc", "swap", "aadd", "aand", "aor", "aeor", "amax", "amin"



					quite cool actually 







202406263.170549:


OH WAITTT


		i just realized



static const char* words[] = {
	"ss", "env", "use", "at",  "rn",  "dr",  "do",  "la", 
	"lt",  "ge",  "ne",  "eq",  "mul", "muh",  "div", "rem",
	"add", "sub", "slt", "and", "or",  "eor", "sl",  "sr",
	"lb",  "lh",  "lw",  "ld",  "sb",  "sh",  "sw",  "sd",
	"alr", "asc", "swap", "aadd", "aand", "aor", "aeor", "amax", "amin"
};



			we should add a new instruction    called       sr? 
					sub-routine       

								ie, defining a function 



		WAITTT



						WAITTTT






									WE CAN ACTUALLY JUST USE     ss      ANDDDD    at 



							OMG





				at ss function 



									ANDDD   if the sign bit is set for an   at    instruction 



										that means that when its executed,   we actually do    CALL ON USE   semantics!!!!





					ie,  




								just saying the function name           CLOSES the previous instruction's argument list, 



										ANDDDD generates     this:        do function ra





										thats how it works 







			OMG
				thats so cool 




	that means that the above code becomes:







			at ss mydivide                               <------- should we make a new word   instead of using      at ss...     maybeeee....
				div ss c b a
				dr ra

			mydivide
			add arga oololllol
			add argn l
			env
	






	this also allows things like   

					macros!!!


			because its just a function call that is inlined lol.   literally how it works.  
					ANDDDD

					its ALWAYS inlined too lol. 



				sooo yeah 




			
wait



			so

								what if you don't want it to be inlined?








			i feel like  the compiler will just check to see if inlining the function would inflate the code size too much 


			yeah probably 




			



	so we don't need to add some  sort of      dont_inline        word or something lol
			

				hopefully not lol 


								so yeah 





		from the programmers perspective, functions will always be inlined, unless the compiler works out that its better to not inline them, 


		so yeah. idk. that kinda makes sense, for performance, i think. hm.. 

			




	i think we will actually use the 


		at ss  label 



				syntax for defining a call on use  label,  


						i do think that does kinda make sense lolol 



				hm


		kinda interesting 

			i'm not totally sure   about that choice,   but it makes sense right now, at least lol. 

				very interesting that it worked out that way lol

					hm

		





	at ss write
		add argn ool
		env
		dr ra


	at ss read
		add argn ll
		env
		dr ra


	
	at main
		
		add arga o
		add argb sp
		add argc olol
		read

		








hmm  


		i think i like this code 

			i think 


						so 


				kinda




						im thinking about it more and more        the argument passing part of it   is pretty rough  and not that ergonomic, ish, 



									although arguably its kinda okay, if the arguments were named something better, actually. 


							yeah, probably haha







	okay 
	but 
		yeah, i think the general idea of using   at ss     is good    i think that is going to stay 


		its    a bit odd 


					but 
									it does the job,   without adding any new instructions, which is whats important lol 





				so yeah, 


									at ss       is going to be how we define a macro, essentially, 



								(well,   an inlined function,  that is just a pair of compiletime executed branch label (do), and branch register (dr) instructions!  

									so yeah! 




							and the do      is generated by saying the name of the thing, 




									if you only defined it as           at functionname     then you would say

													do functionname ra      to get the same behavior lol. 



						so


	yeah



						i think this kinda makes sense then 



	hm

					interesting 






								hmmmm





			lets put this in a tb now 



 
	

*/







/*
8	si
	ec
	use	file
	at 	label
	rn	source	dest
	ar 	source 	dest
	al 	label 	dest
	la	dest 	label

4	lt 	label 	source 	source 
	ge 	label 	source 	source
	ne 	label 	source 	source
	eq 	label 	source 	source

4	mul 	dest 	source 	source
	mh 	dest 	source 	source
	div 	dest 	source 	source
	rem 	dest 	source 	source

8	add 	dest 	source 	source 
	sub 	dest 	source 	source
	slt 	dest 	source 	source
	and 	dest 	source 	source
	or 	dest 	source 	source
	eor 	dest 	source 	source
	sl 	dest 	source 	source
	sr 	dest 	source 	source

8	lb 	dest 	source
	lh 	dest 	source 
	lw 	dest 	source
	ld 	dest 	source
	sb 	source	source
	sh 	source	source
	sw 	source	source
	sd 	source	source
*/









int main(void) {

	typedef unsigned long long nat;

	nat counts[26] = {0};
	for (nat w = 0; w < sizeof words / sizeof *words; w++) {
		for (nat l = 0; l < strlen(words[w]); l++) {
			counts[words[w][l] - 'a']++;
		}
	}
	printf("showing letter frequency used in this set of %lu words...\n", sizeof words / sizeof *words);
	for (nat v = 50; v--;) {
		for (nat l = 0; l < sizeof counts / sizeof *counts; l++) {
			if (counts[l] == v) {
				printf(" %c : %3llu : ", (int)('a' + l), counts[l]);
				for (nat _ = 0; _ < counts[l]; _++) {
					putchar('#');
				}
				puts("");
			}
		}
	}
	puts("done");
}


