#include <stdio.h>
#include <stdbool.h>
#include <iso646.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>        

#define red   "\x1B[31m"
#define green   "\x1B[32m"
#define yellow   "\x1B[33m"
#define blue   "\x1B[34m"
#define magenta   "\x1B[35m"
#define cyan   "\x1B[36m"
#define reset "\x1B[0m"
#define debug 1

typedef size_t nat;
enum word_type { nullw, generic_def, generic2_def, label_def, label2_def, forward_def, backward_def, var_def };
enum instruction_type { nulli, nop, add, _xor, bne };
struct word { char* name; nat length, type, value; };

static nat arguments[32] = {0};
static nat dictionary_count = 0;
static struct word* dictionary = NULL;
static nat ins_count = 0;
static nat* instructions = NULL;

//////////////////////////////////////////////////////////////////////////////////////////

static const char* spell_type(nat t) { 
	if (t == nullw) return "{null}";

	if (t == generic_def) return yellow "generic_def" reset;
	if (t == generic2_def) return yellow "generic2_def" reset;

	if (t == label_def) return magenta "label_def" reset;
	if (t == label2_def) return magenta "label2_def" reset;

	if (t == forward_def) return red "forwards_def" reset;
	if (t == backward_def) return green "backward_def" reset;

	if (t == var_def) return cyan "var_def" reset;
	return "unknown";
}

static const char* spell_ins(nat t) { 
	if (t == nulli) return "{null}";
	if (t == nop) return magenta "nop" reset;
	if (t == add) return red "add" reset;
	if (t == _xor) return green "xor" reset;
	if (t == bne) return blue "bne" reset;
	return "unknown";
}

static void print_name(struct word w) {
	for (nat i = 0; i < w.length; i++) putchar(w.name[i]);
}

static void print_word(struct word w) {
	print_name(w);
	printf(reset "  :  {  .type = %s, .value = %lu } \n", 
		spell_type(w.type), 
		w.value
	);
}

static void print_instruction(nat index) {

	printf("[ ");
	print_name(dictionary[instructions[4 * index + 1]]); printf("(%s) = ", spell_type(dictionary[instructions[4 * index + 1]].type));
	printf("%s { ", spell_ins(instructions[4 * index + 0]));
	print_name(dictionary[instructions[4 * index + 2]]); printf("(%s), ", spell_type(dictionary[instructions[4 * index + 2]].type));
	print_name(dictionary[instructions[4 * index + 3]]); printf("(%s) }",  spell_type(dictionary[instructions[4 * index + 3]].type));

 	printf(" ]\t\t{%lu %lu %lu %lu}\n",
		instructions[4 * index + 0], 
		instructions[4 * index + 1], 
		instructions[4 * index + 2], 
		instructions[4 * index + 3]
	);
}

static void print_arguments() {
	printf("arguments { \n");
	for (nat i = 0; i < 32; i++) {
		printf("\t%3lu : %lu \n", i, arguments[i]);
	}
	printf("}\n");
}

static void print_dictionary() {
	printf("dictionary { \n");
	for (nat i = 0; i < dictionary_count; i++) {
		printf("\t%3lu  :  " green, i);
		print_word(dictionary[i]);
	}
	printf("}\n");
}

static void print_instructions() {
	printf("instructions { \n");
	for (nat i = 0; i < ins_count; i++) {
		printf("\t%3lu  :  " , i);
		print_instruction(i);
	}
	printf("}\n");
}

//////////////////////////////////////////////////////////////////////////////////////////


static void ins(nat op) {
	instructions = realloc(instructions, sizeof(nat) * 4 * (ins_count + 1));
	instructions[4 * ins_count + 0] = op;
	instructions[4 * ins_count + 1] = arguments[0];
	instructions[4 * ins_count + 2] = arguments[1];
	instructions[4 * ins_count++ + 3] = arguments[2];
}

static void branch_check() {
	struct word* label = dictionary + instructions[4 * (ins_count - 1) + 1];

	     if (label->type == generic_def or label->type == label_def)   label->type = forward_def;
	else if (label->type == generic2_def or label->type == label2_def) label->type = backward_def;

	else if (label->type == var_def) {
		printf(red "error: found variable instead of label in branch: " reset);
		print_instruction(ins_count - 1);
	}

	if (debug) printf(cyan "info: generating branch with: " reset);
	if (debug) print_instruction(ins_count - 1);
}

static void operation_check() {
	for (nat i = 1; i < 4; i++) {
		struct word* argument = dictionary + instructions[4 * (ins_count - 1) + i];

		if (	argument->type == forward_def or argument->type == backward_def or
			argument->type == label_def or argument->type == label2_def) {

			if (argument->type == label2_def) argument->type = label_def;

			printf(red "error: found label instead of variable at argument %lu in operation: " reset, i);
			print_instruction(ins_count - 1);

		} else argument->type = var_def;
	}
	if (debug) printf(cyan "info: generating operation with: " reset);
	if (debug) print_instruction(ins_count - 1);
}

static void push_argument(nat argument) {
	for (nat a = 31; a; a--) arguments[a] = arguments[a - 1]; 
	arguments[0] = argument;
}

static void interpret(char* string, nat length) {

	nat count = 0, start = 0;
	for (nat i = 0; i < length; i++) {

		if (not isspace(string[i])) { 
			if (not count) start = i; 
			count++; continue; 
		} else if (not count) continue;

		process_word:; 
		char* word = string + start;

		     if (not strncmp(word, "nop", count)) ins(nop);
		else if (not strncmp(word, "add", count)) { ins(add); operation_check(); } 
		else if (not strncmp(word, "xor", count)) { ins(_xor); operation_check(); } 
		else if (not strncmp(word, "bne", count)) { ins(bne); branch_check(); }

		else {
			for (nat d = 0; d < dictionary_count; d++) {
				if (strncmp(dictionary[d].name, word, count)) continue;

				if (dictionary[d].type == forward_def)      dictionary[d].type = label_def;
				else if (dictionary[d].type == generic_def) dictionary[d].type = generic2_def;
				else if (dictionary[d].type == label_def)   dictionary[d].type = label2_def;

				if (debug) printf("[DEFINED]    ");
				if (debug) print_word(dictionary[d]);

				push_argument(d);
				goto finish_word;
			}

			push_argument(dictionary_count);
			dictionary = realloc(dictionary, sizeof(struct word) * (dictionary_count + 1));
			dictionary[dictionary_count++] = (struct word) {.name = strndup(word, count), .length = count, .type = generic_def, .value = 0};

			if (debug) printf("[not defined]  -->  assuming  ");
			if (debug) print_word(dictionary[dictionary_count - 1]);

			finish_word:
			if (dictionary[*arguments].type == generic_def or dictionary[*arguments].type == label_def) dictionary[*arguments].value = ins_count;
		}
		count = 0;
	}
	if (count) goto process_word;
}

int main() {
	puts("a repl for my programming language.");
	char line[4096] = {0};
loop: 	printf(" : ");
	fgets(line, sizeof line, stdin);
	nat length = strlen(line);
	line[--length] = 0;
	if (not strcmp(line, "q")) goto done;
	else if (not strcmp(line, "o")) printf("\033[H\033[J");
	else if (not strcmp(line, "arguments")) print_arguments();
	else if (not strcmp(line, "dictionary")) print_dictionary();
	else if (not strcmp(line, "instructions")) print_instructions();
	else interpret(line, length);
	goto loop; done:;
}








































/*

	zero zero zero xor

	zero one label bne

	label

	zero label pasta add

	nop

	nop

	one one label bne

*/



















/*

label 
zero one label bne


zero one label bne











else if (not strcmp(line, "i")) interpret(strdup(test_string), strlen(test_string));










printf("%c [%lu] ", string[i], count);
puts("");





printf("unknown word found: [@start=%lu, count=%lu]\n", start, count);
				printf("ie, ---> ");
				print_word(string, start, count);
				puts("");


printf("---> ");
				print_word(string, start, count);
				puts("");










static char* read_file(const char* filename, size_t* out_length) {
	const int file = open(filename, O_RDONLY);
	if (file < 0) {
		perror("open"); 
		return NULL;
	}
	struct stat file_data = {0};
	if (stat(filename, &file_data) < 0) { 
		perror("stat"); 
		return NULL;
	}
	const size_t length = (size_t) file_data.st_size;
	char* buffer = not length ? NULL : mmap(0, (size_t) length, PROT_READ, MAP_SHARED, file, 0);
	if (buffer == MAP_FAILED) {
		perror("mmap");
		return NULL;
	}
	close(file);
	*out_length = length;
	return buffer;
}





	else if (not strcmp(string, "file")) {

		char filename[4096] = {0};
		printf("filename: ");
		fgets(filename, sizeof filename, stdin);
		filename[strlen(filename) - 1] = 0;
		size_t length = 0;
		char* contents = read_file(filename, &length);
		if (contents) interpret(contents, length);
	}



static nat data_node_count = 0;
static struct data_node data_nodes = NULL;




//struct data_node { nat type, value; };


data_node = realloc(data_node, sizeof(struct data_node) * (data_node_count + 1));
			data_node[data_node_count++] = (struct data_node) {...};



// if we see that the label argument's data node   has a value which is 0, then we know it was not defined yet. overwrite its type with 

			// if (not data_node[*arguments].value) data_node[*arguments].type = forward_label;



if (data_node[dictionary[entry].index].type == forward_label) {
					
				}



  // then define the name as a variable, with the data_node of the expression!


// define the name as a label. 





if (data_nodes[*arguments].type == expression) {  
				
			} else {  
				
			}




enum data_node_type { nulld, variable, label, forward_label };



if (dictionary[instructions[4 * (ins_count - 1) + 1]].use_count == 1) {

			if (debug) printf(red "ERROR: forward declaring label \"");
			if (debug) print_word(dictionary[instructions[4 * (ins_count - 1) + 1]]);

			type = 
			
		} else {
			if (debug) printf(cyan "info: found already declared label.\n" reset);
			if (debug) printf("[.use_count = %lu]\n", dictionary[instructions[4 * (ins_count - 1) + 1]].use_count);
		}


if (debug) printf(red "ERROR: type mismatch. expected label variable for branch arg0. ...aborting...\n" reset);
		if (debug) printf("[.type = %lu]\n", dictionary[instructions[4 * (ins_count - 1) + 1]].type);
		if (debug) abort();



/// if (dictionary[d].type == label_def) dictionary[d].type = backward_def;




if (dictionary[instructions[4 * (ins_count - 1) + 1]].type == forward_def) {
		printf("forward def\n");
		
	} else {
		printf("label def\n");
	}



 puts(red "FORWARDS BRANCH" reset);


 puts(green "BACKWARDS BRANCH" reset);

  puts(red "DOUBLE FORWARDS BRANCH" reset);  

puts(red "IMM FORWARDS BRANCH" reset);




	//else if (label->type == forward_def) {}  
	//else if (label->type == backward_def) {} 
	//else 

*/




