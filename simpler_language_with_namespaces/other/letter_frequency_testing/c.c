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









*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char* words[] = {
	"ecl", "att", "use", "nsb", "nse",
	"ldb", "ldh", "ldw", "ldd", "stb", "sth", "stw", "std", "balr", "bal",
	"add", "sub", "slt", "slts", "and", "ior", "eor", "sll", "srl", "sra", "mul", "muh", 
	"muhs", "div", "divs", "rem", "rems", "blt", "blts", "bge", "bges", "bne", "beq", 
};


int main(void) {

	typedef unsigned long long nat;

	nat counts[26] = {0};
	for (nat w = 0; w < sizeof words / sizeof *words; w++) {
		for (nat l = 0; l < strlen(words[w]); l++) {
			counts[words[w][l] - 'a']++;
		}
	}
	printf("showing letter frequency used in this set of %llu words...\n", sizeof words / sizeof *words);
	for (nat v = 50; v--;) {
		for (nat l = 0; l < sizeof counts / sizeof *counts; l++) {
			if (counts[l] == v) {
				printf(" %c : %3llu : ", 'a' + l, counts[l]);
				for (nat _ = 0; _ < counts[l]; _++) {
					putchar('#');
				}
				puts("");
			}
		}
	}
	puts("done");
}


