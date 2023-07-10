#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <iso646.h>
#include <stdbool.h>
#include <ctype.h>

typedef unsigned long long nat;


static inline void print_words(char* dict, nat* words, nat word_count) {


	printf("\nprinting the words found(%llu): { \n", word_count);
		
	for (nat i = 0; i < word_count; i++) {
		printf("#%llu: found: \"", i);
		for (nat w = words[i]; dict[w] != ' '; w++) {
			printf("%c", dict[w]);
		}
		printf("\"\n");
	}

	printf("}\ndone.\n");
}



int main() {
	const char* raw_text = "  hello                                  there\n     file    beans";
	const char* raw_text2 = "          hi   my      name is daniel    and i like           bubbles   :)";
	puts(raw_text);
	
	
	char* file = strdup(raw_text);
	char* dict = calloc(4096, sizeof(char));
	nat* words = calloc(4096, sizeof(nat));

	nat file_length = (nat) strlen(file);
	nat dict_length = 0;
	nat word_count = 0;

	nat F_index = 0;
	nat F_begin = 0;
	nat D_begin = 0;

	goto s;

	while (F_index < file_length) {

		if (not isspace(file[F_index])) {
			dict[dict_length++] = file[F_index++];
			continue;
		} 

	push:	dict[dict_length++] = ' ';
		words[word_count++] = D_begin;

	s:	while (F_index < file_length and isspace(file[F_index])) F_index++; 
		F_begin = F_index;
		D_begin = dict_length;
	}
	if (F_begin != F_index) goto push;


	free(file);
	file = strdup(raw_text2);
	file_length = (nat) strlen(file);

	F_index = 0;
	F_begin = 0;

	goto s2;

	while (F_index < file_length) {

		if (not isspace(file[F_index])) {
			dict[dict_length++] = file[F_index++];
			continue;
		} 

	push2:	dict[dict_length++] = ' ';
		words[word_count++] = D_begin;

	s2:	while (F_index < file_length and isspace(file[F_index])) F_index++; 
		F_begin = F_index;
		D_begin = dict_length;
	}
	if (F_begin != F_index) goto push2;





	puts(dict);
	printf("(%llu)[ ", word_count);
	for (nat i = 0; i < word_count; i++) {
		printf("%llu, ", words[i]);
	}
	printf("]\n");





	print_words(dict, words, word_count);



}







































/*



//print_word(words[count - 1]);
		//if (not strcmp(words[count - 1], "file")) abort();


static bool equal(char* text, nat begin, nat end, const char* this) {

	const nat length = strlen(this);             // this should be compiletime known.
	if (length != end - begin) return false;
	nat o = 0;
	for (nat i = begin; i < end; i++, o++) 
		if (text[i] != this[o]) return false;
	return true;
}

static void print_word(char* word) {
	printf("printing word: \"");
	printf("%s", word);
	printf("\"\n");
}

static void print_string_as_ints(char* string, nat length) {
	printf("printing %llu characters: \n", length);
	for (nat i = 0; i < length; i++) 
		printf("%d, ", string[i]);
	printf("[done]\n");
}















	 // counts the number of nats, and two nats is one word.
			// one to give the file index, and one to give the begin for the word.
				// and it is terminated by writing into the file itself, a null terminator. i think...

	hm... i feel like... we need to combine the text that we find in the included file, into a 

			

			so basically...          here ill write a tb about it 
*/


		






/*
nat word_len = 0, i = 0;
	char word[4096] = {0};
begin:	if (i >= text_length) goto done;
	if (isspace(text[i])) goto skip;
	goto use;
skip:	i++;
	if (i >= text_length) goto done;
	if (isspace(text[i])) goto skip;
add:	if (not word_len) goto begin;
	word[word_len] = 0; 
	word_len = 0;
	code[code_count++] = strdup(word);
	// interpret this word right here! i think.
	if (i >= text_length) return;
use: 	word[word_len++] = text[i++];
	goto begin;
done:	if (word_len) goto add;

*/


