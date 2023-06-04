/*
we are in the middle of implementing:

	- revise the branching and label def system in the language.

	- getting execution of the instructions working. 

	- allowing the repl to have newlines on a line, by using getdelim. or our cool editor function lol!

	- test control flow working in execution 

	- add macros to the language, using compiletime function calls. 

	- add some sort of constant system to the language. yup. 

	- 







cool piece of code:
-------------------------------

z z z xor 
iter add
a001 z limit addi 
loop limit iter done bge
iter print incr
iter iter loop bge done
`

-------------------------------






*/



#define 	debug 		0




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


typedef size_t nat;
enum instruction_type { null, nop, add, addi, rem, incr, _xor, print, bne, blt, beq, bge, };
struct word { char* name; nat length, type, value; };

static const char digits[96] = 
	"0123456789abcdefghijklmnopqrstuvwxyz"
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ,."
	";:-_=+/?!@#$%^&*()<>[]{}|\\~`\'\"";


static nat arguments[32] = {0};
static nat dictionary_count = 0;
static struct word* dictionary = NULL;
static nat ins_count = 0;
static nat* instructions = NULL;

static const char* spell_ins(nat t) { 
	if (t == null) return "{null}";
	if (t == nop) return magenta "nop" reset;
	if (t == add) return red "add" reset;
	if (t == addi) return red "addi" reset;
	if (t == rem) return yellow "rem" reset;
	if (t == incr) return cyan "incr" reset;
	if (t == _xor) return green "xor" reset;
	if (t == bne) return blue "bne" reset;
	if (t == blt) return green "blt" reset;
	if (t == beq) return magenta "beq" reset;
	if (t == bge) return red "bge" reset;
	if (t == print) return yellow "print" reset;
	return "unknown";
}



static void print_name(struct word w) {
	printf("\033[38;5;%dm", 67);
	for (nat _ = 0; _ < w.length; _++) putchar(w.name[_]);
	printf(reset);
}

static void print_word(struct word w) {
	print_name(w);
	printf(reset "  :  {  .type = %s, .value = %lu } \n",  green "any" reset, 
		w.value
	);
}

static void print_instruction(nat index) {

	if (not instructions) { puts("null instructions"); return; } 

	printf("{%lu %lu %lu %lu}\t",
		instructions[4 * index + 0], 
		instructions[4 * index + 1], 
		instructions[4 * index + 2], 
		instructions[4 * index + 3]
	);

	if (instructions[4 * index] != nop) {
		print_name(dictionary[instructions[4 * index + 1]]); 
		printf(" = ");
	}

	printf("%s ", spell_ins(instructions[4 * index]));

	if (instructions[4 * index] != nop) {
		printf("{ ");
		print_name(dictionary[instructions[4 * index + 2]]); 
		printf(", ");
	}

	if (instructions[4 * index] != nop) {
		print_name(dictionary[instructions[4 * index + 3]]); 
		printf(" } ");
	}
	puts("");
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

static nat string_to_number(char* string, nat* length) {
	nat radix = 0, value = 0;
	nat result = 0, index = 0, place = 1;
begin:	if (index >= *length) goto done;
	value = 0;
top:	if (value >= 96) goto found;
	if (digits[value] == string[index]) goto found;
	value++;
	goto top;
found:	if (index) goto check;
	radix = value;
	goto next;
check:	if (value >= radix) goto done;
	result += place * value;
	place *= radix;
next:	index++;
	goto begin;
done:	*length = index;
	return result;
}

static void ins(nat op, bool is_branch) {
	if (is_branch) {
		if (not dictionary) { puts(red "error: empty dictionary in branch" reset); return; }
		struct word* label = dictionary + *arguments;
		if (label->value == ins_count) label->value = (size_t) -1;
	}
	instructions = realloc(instructions, sizeof(nat) * 4 * (ins_count + 1));
	instructions[4 * ins_count + 0] = op;
	instructions[4 * ins_count + 1] = arguments[0];
	instructions[4 * ins_count + 2] = arguments[1];
	instructions[4 * ins_count + 3] = arguments[2];
	ins_count++;
}

static void push_argument(nat argument) {
	for (nat a = 31; a; a--) arguments[a] = arguments[a - 1];
	*arguments = argument;
}

static void interpret(char* string, nat length) {

	nat count = 0, start = 0;

	for (nat index = 0; index < length; index++) {
		if (not isspace(string[index])) { 
			if (not count) start = index;
			count++; continue;
		} else if (not count) continue;

		process_word:;
		char* word = string + start;

		     if (count == 3 and not strncmp(word, "nop", count)) ins(nop,0);
		else if (count == 3 and not strncmp(word, "add", count)) ins(add,0); 
		else if (count == 4 and not strncmp(word, "addi", count)) ins(addi,0); 
		else if (count == 3 and not strncmp(word, "rem", count)) ins(rem,0); 
		else if (count == 3 and not strncmp(word, "xor", count)) ins(_xor,0); 
		else if (count == 4 and not strncmp(word, "incr", count)) ins(incr,0);
		else if (count == 5 and not strncmp(word, "print", count)) ins(print,0);

		else if (count == 3 and not strncmp(word, "bne", count)) ins(bne,1);
		else if (count == 3 and not strncmp(word, "blt", count)) ins(blt,1);
		else if (count == 3 and not strncmp(word, "beq", count)) ins(beq,1);
		else if (count == 3 and not strncmp(word, "bge", count)) ins(bge,1);

		else {
			for (nat d = 0; d < dictionary_count; d++) {
				if (dictionary[d].length != count or strncmp(dictionary[d].name, word, count)) continue;

				if (debug) printf("[DEFINED]    ");
				if (debug) print_word(dictionary[d]);

				push_argument(d);
				if (dictionary[*arguments].value == (size_t) -1) dictionary[*arguments].value = ins_count;
				goto done;
			}

			push_argument(dictionary_count);
			dictionary = realloc(dictionary, sizeof(struct word) * (dictionary_count + 1));

			dictionary[dictionary_count++] = (struct word) {
				.name = strndup(word, count), 
				.length = count, 
				.value = ins_count
			};

			if (debug) printf("[not defined]  -->  assuming  ");
			if (debug) print_word(dictionary[dictionary_count - 1]);
			goto done;
		}

		if (debug) { if (ins_count) print_instruction(ins_count - 1); } 

		done: count = 0;
	}
	if (count) goto process_word;

	if (debug) print_instructions();
	nat registers[4096] = {0};
	memset(registers, 0xF0, sizeof registers);
	
	for (nat ip = 0; ip < ins_count; ip++) {

		if (debug) printf("executing @%lu : ", ip);
		if (debug) print_instruction(ip);
	
		const nat op  = instructions[4 * ip + 0];
		const nat out = instructions[4 * ip + 1];
		const nat in1 = instructions[4 * ip + 2];
		const nat in2 = instructions[4 * ip + 3];

		if (op == nop) { 
			if (debug) printf("executed nop.\n"); 

		} else if (op == _xor) {
			if (debug) printf("executed xor: %lu = %lu %lu\n", out, in1, in2);
			registers[out] = registers[in1] ^ registers[in2];

		} else if (op == add) {
			if (debug) printf("executed add: %lu = %lu %lu\n", out, in1, in2);
			registers[out] = registers[in1] + registers[in2];

		} else if (op == addi) {
			if (debug) printf("executed addi: %lu = %lu %lu\n", out, in1, in2);
			nat m = dictionary[in2].length;
			const nat n = string_to_number(dictionary[in2].name, &m);
			if (debug) {
				printf("in2 constant = %lu (length = %lu)\n", n, m);
			}
			registers[out] = registers[in1] + n;

		} else if (op == rem) {
			if (debug) printf("executed rem: %lu = %lu %lu\n", out, in1, in2);
			registers[out] = registers[in1] % registers[in2];

		} else if (op == rem) {
			if (debug) printf("executed rem: %lu = %lu %lu\n", out, in1, in2);
			registers[out] = registers[in1] % registers[in2];
		
		} else if (op == incr) {
			if (debug) printf("executed incr: %lu\n", out);
			registers[out]++;

		} else if (op == print) {
			if (debug) printf("executed print: %lu\n", out);
			printf("%lu\n", registers[out]);

		} else if (op == bne) {
			if (debug) printf("executing bne...\n");
			if (dictionary[out].value == (size_t) -1) { puts(red "error: unspecified label in branch" reset); goto halt; }
			if (registers[in1] != registers[in2]) ip = dictionary[out].value - 1;

		} else if (op == beq) {
			if (debug) printf("executing bne...\n");
			if (dictionary[out].value == (size_t) -1) { puts(red "error: unspecified label in branch" reset); goto halt; }
			if (registers[in1] == registers[in2]) ip = dictionary[out].value - 1;
		
		} else if (op == blt) {
			if (debug) printf("executing blt...\n");
			if (dictionary[out].value == (size_t) -1) { puts(red "error: unspecified label in branch" reset); goto halt; }
			if (registers[in1] < registers[in2]) ip = dictionary[out].value - 1;

		} else if (op == bge) {
			if (debug) printf("executing bge...\n");
			if (dictionary[out].value == (size_t) -1) { puts(red "error: unspecified label in branch" reset); goto halt; }
			if (registers[in1] >= registers[in2]) ip = dictionary[out].value - 1;
		}
		else abort();
	}
halt: 	if (debug) puts(green "[finished execution]" reset);
}

static void reset_env(void) {
	memset(arguments, 0, sizeof arguments);
	dictionary_count = 0; 
	free(dictionary); dictionary = NULL; 
	ins_count = 0;
	free(instructions); instructions = NULL;
}

int main() {
	puts("a repl for my programming language.");
	size_t capacity = 0;
	char* input = NULL;

loop: 	printf(" : ");
	ssize_t r = getdelim(&input, &capacity, '`', stdin);
	size_t length = (size_t) r; getchar();
	input[--length] = 0;

	if (not strcmp(input, "q") or not strcmp(input, "quit")) goto done;
	else if (not strcmp(input, "o") or not strcmp(input, "clear")) printf("\033[H\033[J");
	else if (not strcmp(input, "arguments")) print_arguments();
	else if (not strcmp(input, "dictionary")) print_dictionary();
	else if (not strcmp(input, "instructions")) print_instructions();
	else if (not strcmp(input, "reset")) reset_env();
	else interpret(input, length);
	goto loop; done:;
}



















/*
printf("(%s) = ", spell_type(dictionary[instructions[4 * index + 1]].type));
printf("(%s), ", spell_type(dictionary[instructions[4 * index + 2]].type));
printf("(%s) }",  spell_type(dictionary[instructions[4 * index + 3]].type));
*/




















/*

	label:
		instructions;
		and;
		stuff;
		if (condition) goto done;
				
		goto label;

	done:
		other stuff;
	




	nop
	nop
	nop
	zero one add
label
	nop
	nop
	one zero label bne
	one zero done bne 
	nop
	nop
	nop
done



*/






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




else if (label->type == var_def) {
		printf(red "error: found variable instead of label in branch: " reset);
		print_instruction(ins_count - 1);
	}




for (nat i = 1; i < 4; i++) {
		struct word* argument = dictionary + instructions[4 * (ins_count - 1) + i];

		if (	argument->type == forward_def or argument->type == backward_def or
			argument->type == label_def or argument->type == label2_def) {

			if (argument->type == label2_def) argument->type = label_def;

			printf(red "error: found label instead of variable at argument %lu in operation: " reset, i);
			print_instruction(ins_count - 1);

		} else argument->type = var_def;
	}



	     if (label->type == generic_def or label->type == label_def)   label->type = forward_def;
	else if (label->type == generic2_def or label->type == label2_def) label->type = backward_def;



if (dictionary[d].type == forward_def)      dictionary[d].type = label_def;
				else if (dictionary[d].type == generic_def) dictionary[d].type = generic2_def;
				else if (dictionary[d].type == label_def)   dictionary[d].type = label2_def;




*/



//	const nat color_count = 6;
//	const char* color[color_count] = { red, green, yellow, blue, magenta, cyan };






//	if (t == generic_def) return yellow "generic_def" reset;
//	if (t == generic2_def) return yellow "generic2_def" reset;

//	if (t == label_def) return magenta "label_def" reset;
//	if (t == label2_def) return magenta "label2_def" reset;

//	if (t == forward_def) return red "forwards_def" reset;
//	if (t == backward_def) return green "backward_def" reset;

//	if (t == var_def) return cyan "var_def" reset;
//	return "unknown";




//  static const char* spell_type(nat t) { return green "any" reset; }


/*


x x f bne     
   nop
   nop
   nop
f  a b c add
   x x x xor
`

*/


// enum word_type { nullw, generic_def, generic2_def, label_def, label2_def, forward_def, backward_def, var_def };

