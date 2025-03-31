// a complete redesign of the language to not use btucsr.
// also its interpreted, just to be able to bootstrap itself, at first.
// made by daniel warren riaz rehman.
// started redesign on 2206105.120844

// #include <stdint.h>
#include <stdbool.h>
#include <iso646.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>

struct instruction {
	char** list;
	int count;
	int padding;
};

struct operation {
	char* name;
	int arity;
	int built_in; 			// if built in, leave the following two members uninitialized.
	struct instruction parameters;
	struct instruction* body;
	int body_count;
	int padding;
};

static char* read_file(const char* filename, int* out_length) {
	
	int file = open(filename, O_RDONLY);
	if (file < 0) {
		perror("open"); 
		exit(1); 
	}

	struct stat file_data = {0};
	if (stat(filename, &file_data) < 0) { 
		perror("stat"); 
		exit(1); 
	}

	int length = (int) file_data.st_size;
	char* buffer = not length ? NULL : mmap(0, (size_t) length, PROT_READ, MAP_SHARED, file, 0);
	if (buffer == MAP_FAILED) { 
		perror("mmap"); 
		exit(1); 
	}

	close(file);
	*out_length = length;
	return buffer;
}

static void print_instruction(struct instruction s) {
	printf(" ( ");
	for (int i = 0; i < s.count; i++) {
		printf("*%d: \"%s\"", i, s.list[i]);
		if (i < s.count - 1) printf(", ");
	}
	printf(" ) \n");
}

static void print_instructions(struct instruction* ins, int count) {
	printf("ins={: (%d total)\n", count);
	for (int i = 0; i < count; i++) {
		printf("\t#%d: ", i);
		print_instruction(ins[i]);
	}
	printf("}[end ins]\n");
}


// static int find(char* this, char** names, int name_count) {
// 	for (int n = 0; n < name_count; n++) {
// 		if (not strcmp(names[n], this)) return n;
// 	}
// 	return name_count;
// }

static int lookup(char* this, char** names, int name_count) {
	for (int n = 0; n < name_count; n++) {
		if (not strcmp(names[n], this)) return n;
	}
	printf("error: unknown reference: \"%s\" aborting...\n", this); 
	abort();
}

static void print_registers(char** names, int64_t* values, int name_count) {
	printf("registers: {\n");
	for (int i = 0; i < name_count; i++) {
		printf("\tnames=\"%s\":%llu\n", names[i], values[i]);
	}
	printf("}:end of registers.\n");
}

static void print_locations(char** labels, int* locations, int label_count) {
	printf("locations: {\n");
	for (int i = 0; i < label_count; i++) {
		printf("\tlabel=\"%s\":%d\n", labels[i], locations[i]);
	}
	printf("}:end of labels.\n");
}

static void print_dictionary(char** op_names, struct operation* op_values, int op_count) {
	printf("printing current dictionary of operation names/values:\n\n");
	for (int i = 0; i < op_count; i++) {
		printf("---- operation #%d: \"%s\" ----\n", i, op_names[i]);
		printf("\t. name = %s\n", op_values[i].name);
		printf("\t. arity = %d\n", op_values[i].arity);
		printf("\t. built_in = %d\n", op_values[i].built_in);
		printf("\t. parameters := ");
		print_instruction(op_values[i].parameters);
		printf("\t. body := (count=%d)\n", op_values[i].body_count);
		print_instructions(op_values[i].body, op_values[i].body_count);
		puts("");
	}
	printf("[end of operations]\n");
}

static void load_dictionary(char** op_names, struct operation* op_values, int* op_count) {

	// printf("loading dictionary with isa primitives...\n");

	int count = 7;
	struct operation isa[] = {

		(struct operation) {.name = strdup("halt"),  .arity = 0, .built_in = true},
		(struct operation) {.name = strdup("init"),  .arity = 1, .built_in = true},
		(struct operation) {.name = strdup("zero"),  .arity = 1, .built_in = true},
		(struct operation) {.name = strdup("incr"),  .arity = 1, .built_in = true},

		(struct operation) {.name = strdup("decr"),  .arity = 1, .built_in = true},
		(struct operation) {.name = strdup("print"), .arity = 1, .built_in = true},
		(struct operation) {.name = strdup("blt"),   .arity = 3, .built_in = true},

		// (struct operation) {
		// 	.name = strdup("macro1"),      // single arity macro definition construct. 
		// 	.arity = 2, // two args:  the operation name, and the parameter name.    
		// 					// (macros are typeless)
		// 	.built_in = true,
		// },

		// (struct operation) {
		// 	.name = strdup("endmacro1"),
		// 	.arity = 0,
		// 	.built_in = true,
		// }
		
	};
	
	for (int i = 0; i < count; i++) {
		op_names[i] = isa[i].name;
		op_values[i] = isa[i];
	}	

	*op_count = count;
	
}


int main(const int argc, const char** argv) {
	if (argc < 2) return printf("usage: ./compiler -i string\n./compiler -f <txtfile>\n");
	
	int length = 0;
	char* input = read_file(argv[1], &length);

	// printf("\n\n\n------------- printing input file -------------------\n");
	// printf("input(%d) = <<<%.*s>>>\n\n\n",length, length, input);
	
	struct instruction* ins = calloc(1, sizeof(struct instruction));
	int ins_count = 0;

	char buffer[1024] = {0};
	int buffer_count = 0;

	for (int i = 0; i < length; i++) {
		if ((unsigned char) input[i] < 33) continue;

		if (input[i] == '.') {

			buffer[buffer_count++] = 0;	
			ins[ins_count].list = realloc(ins[ins_count].list, (size_t)(ins[ins_count].count + 1) * sizeof(char*));
			ins[ins_count].list[ins[ins_count].count++] = strdup(buffer);
			buffer_count = 0;

			++ins_count;
			ins = realloc(ins, (size_t)(ins_count + 1) * sizeof(struct instruction));
			ins[ins_count] = (struct instruction){0};
		
		} else if (input[i] == ',') {

			buffer[buffer_count++] = 0;	
			ins[ins_count].list = realloc(ins[ins_count].list, (size_t)(ins[ins_count].count + 1) * sizeof(char*));
			ins[ins_count].list[ins[ins_count].count++] = strdup(buffer);
			buffer_count = 0;

		} else buffer[buffer_count++] = input[i];
	}


	munmap(input, (size_t) length);

	// printf("\n\n\n------------- printing instructions... -------------------\n");

	// print_instructions(ins, ins_count);

	
	// printf("\n\n\n------------- loading ISA dictionary... -------------------\n");

	char** op_names = calloc(512, sizeof(char*));
	struct operation* op_values = calloc(512, sizeof(struct operation));
	int op_count = 0;

	load_dictionary(op_names, op_values, &op_count);
	// print_dictionary(op_names, op_values, op_count);

	// do macro stuff here

	// printf("\n\n\n------------- finding labels for all instructions... -------------------\n");

	int* locations = calloc(512, sizeof(int));
	char** labels = calloc(512, sizeof(char*));
	int label_count = 0;
	
	for (int i = 0; i < ins_count; i++) {
		// printf("checking arity of instruction #%d  ::  ", i);
		// print_instruction(ins[i]);
		char* op = ins[i].list[0];

		for (int item = 0; item < op_count; item++) {
			if (not strcmp(op, op_names[item])) {
				int c = op_values[item].arity + 1;
				if (ins[i].count > c) {	
					labels[label_count] = ins[i].list[c];
					// printf("found label: \"%s\"\n", labels[label_count]);
					locations[label_count++] = i;
				}
				goto next;
			} 
		}
		printf("error: unknown arity for unknown command \"%s\", aborting...\n", op);
		abort();
		next:;
	}



	// printf("\n\n\n------------- printing labels for all instructions... -------------------\n");


	// print_locations(labels, locations, label_count);

	// printf("\n\n\n------------- interpreting instructions... -------------------\n");

	int64_t* values = calloc(512, sizeof(int64_t));
	char** names = calloc(512, sizeof(char*));
	int name_count = 0;

	for (int i = 0; i < ins_count; i++) {

		// this loop only needs to deal with the isa instructions, 
		// because the macros are already expanded at this point. 

		// printf("executing instruction #%d :: ", i);
		// print_instruction(ins[i]);

		struct instruction this = ins[i];
		char* operation = this.list[0];

		if (not strcmp(operation, "halt")) {
			printf("\033[1;32m HALTING PROGRAM... \033[0m \n");
			exit(0); // todo make this a syscall instruction. 


		// add the   write(), read(), open(), close()    syscall instructions, 
		// which just call the C ones. 

		// } else if (not strcmp(operation, "debug") ) {
		// 	printf("\033[1;32m DEBUG: <<<%s>>> \033[0m \n", this.list[1]);

		} else if (not strcmp(operation, "init")) {
			names[name_count++] = this.list[1];

		} else if (not strcmp(operation, "zero")) {
			values[lookup(this.list[1], names, name_count)] = 0;

		} else if (not strcmp(operation, "incr")) {
			values[lookup(this.list[1], names, name_count)]++;

		} else if (not strcmp(operation, "decr")) {
			values[lookup(this.list[1], names, name_count)]--;

		} else if (not strcmp(operation, "print")) {
			printf("\033[1;32m PROGRAM OUTPUT: %llu \033[0m \n", 
				values[lookup(this.list[1], names, name_count)]);
		
		} else if (not strcmp(operation, "blt")) {
			if (values[lookup(this.list[1], names, name_count)] < values[lookup(this.list[2], names, name_count)]) 
				i = locations[lookup(this.list[3], labels, label_count)] - 1;

		} else {
			printf("error: executing unknown command, aborting...\n");
			abort();
		}
	}

	print_registers(names, values, name_count);
	printf("[no halt found]\n");
}

















/*
test code:


init,. 
zero,.

incr,.

init,a.
zero,a.

blt,   a , , done.


incr,.


print,,done.

halt.













init, my cool register, start of the code.
zero, my cool register.
halt, halt.















init, starting register for lols, begin of code.
init, another starting register.



macro1,     set to unity,  r-input.

	zero, r-input.
	incr, r - input.

endmacro1.

macro1,     add 4 to, A.

	incr, A.
	incr, A.
	incr, A.
	incr, A.

endmacro1.

init, bubbles.
set to unity, bubbles.

add 4 to, bubbles.
add 4 to, bubbles.
add 4 to, bubbles.


print, bubbles.

init, this cool register.
set to unity, this cool register.
set to unity, this cool register.
set to unity, this cool register.
set to unity, this cool register.

add 4 to, this cool register.
add 4 to, this cool register.

print, this cool register.


halt.









// printf("\n\n\n------------- expanding macro definitions... -------------------\n");

// i dont really like my implementation of this. it really is quite special casey, and not very well done. 





for (int i = 0; i < ins_count; i++) {
		printf("checking for macro instruction #%d  ::  ", i);
		print_instruction(ins[i]);
		char* op = ins[i].list[0];
		printf("\n     >> looking at operation: %s...\n\n", op);

		if (not strcmp(op, "macro1")) {
			printf("\t found a macro definition!\n");
			printf("\t the user is defining: \"%s\", "
					"with parameter name: \"%s\" \n",
					ins[i].list[1], ins[i].list[2]);

			int n = i + 1, save = n;
			printf("the following instructions are part of the body of this macro: n=%d\n\n", n);

			for (; n < ins_count; n++) {
				printf("\t ins#%d, body@+%d  ::  ", n, n - i - 1);
				print_instruction(ins[n]);
				
				if (not strcmp(ins[n].list[0],"endmacro1")) {
					break;
				} 
			}
			
			printf("\n[end of instruction list: n=%d]\n", n);
			
			printf("---> note: body comprises of %d instructions.\n", n - save);
			struct instruction* body = calloc((size_t) (n - save), sizeof(struct instruction));
			if (n - save) memcpy(body, ins + save, (size_t) (n - save) * sizeof(struct instruction));

			printf("adding new operation to dictionary...\n");

			struct operation new_op = {
				.name = strdup(ins[i].list[1]),
				.arity = 1, // because macro1. (hardcoded arity of 1.
				.built_in = false,
				.parameters = ins[i],
				.body = body, 
				.body_count = n - save,
			};
			
			op_names[op_count] = ins[i].list[1];
			op_values[op_count++] = new_op;

			printf("done. dictionary is now: \n");
			print_dictionary(op_names, op_values, op_count);

			printf("deleting the body in the instruction sequence now...\n");
			// note: this causes a memory leak now. 


			puts("\033[1;32m before: \033[0m");
			print_instructions(ins, ins_count);
			
			printf("deleting from #%d to #%d...\n", save, n);
			printf("moving starting from #%d to the end #%d... (%d instructions)\n", 
				n, ins_count, ins_count - n);
			
			memmove(ins + (save - 1), ins + n + 1, (size_t) (ins_count - (n + 1)) * sizeof(struct instruction));
			printf("move done. now resizing...\n");

			ins_count -= (n - save) + 2;
			ins = realloc(ins, (size_t) (ins_count) * sizeof(struct instruction));
			i--;

			puts("\033[1;32m after: \033[0m ");
			print_instructions(ins, ins_count);

			continue;
		}

		for (int item = 0; item < op_count; item++) {
			if (not strcmp(op, op_names[item]) and not op_values[item].built_in) {
				printf("\033[1;31m FOUND A MACRO CALL!!! \033[0m \n");

				char* param = op_values[item].parameters.list[2];
				char* arg = ins[i].list[1];
				printf("\n   parameter: %s\n",  param);
				printf("   argument:  %s\n\n",  arg);


				// struct instruction call = ins[i];


				//todo: 
				//     1. insert body instructions from    op_values[item].body
				//     2. replace the parameter-register name with the argument-register name.
				//    
				//          parameter:     op_values[item].parameters[2]   (first arg)
				//          argument:      ins[i].list[1]

				print_instructions(op_values[item].body, op_values[item].body_count);


				printf("inserting body into instruction sequence...\n");

				puts("\033[1;32m before: \033[0m");
				print_instructions(ins, ins_count);

				struct instruction* body = op_values[item].body;
				int body_count = op_values[item].body_count;

				printf("DEBUG: body_count = %d\n", body_count);

				
				

				ins = realloc(ins, (size_t) (ins_count + (body_count - 1)) 
									* sizeof(struct instruction));


				puts("currently looking at: \n");
				print_instruction(ins[i]);

				// int L = i + 1;
				// int K = (body_count) + L;

				// printf("DEBUG: L = %d, K = %d\n", L,K);

				memmove(ins + i + 1 + (body_count - 1), ins + i + 1, (size_t) 
		(ins_count - (i + 1)) * sizeof(struct instruction));
				ins_count += body_count - 1;

				// // set zero the memory that we got.
				// for (int e = i; e < i + body_count; e++) {
				// 	ins[e] = (struct instruction){0};
				// }

				// we need to do a deep copy:

				puts("\033[1;32m before deep copy: \033[0m");
				print_instructions(ins, ins_count);

				for (int e = 0; e < body_count; e++) {

					puts("destroying:");
					print_instruction(ins[i + e]);
					puts("replacing with:");
					print_instruction(body[e]);
					ins[i + e].list = malloc((size_t)(body[e].count) * sizeof(char*));
					memcpy(ins[i + e].list, body[e].list, (size_t) body[e].count * sizeof(char*));
					ins[i + e].count = body[e].count;
				}


				// memcpy(ins + i, body, (size_t) body_count * sizeof(struct instruction));

				
				
				puts("\033[1;32m after: \033[0m");
				print_instructions(ins, ins_count);

				printf("replacing all occurences in the body of the param  with the arg...\n");

				for (int c = i; c < i + body_count; c++) {
					char* _op = ins[c].list[0];
					
					for (int item = 0; item < op_count; item++) {
						if (not strcmp(_op, op_names[item])) {
							int arity = op_values[item].arity;

							for (int a = 1; a < 1 + arity; a++) {
								if (not strcmp(ins[c].list[a], param))
									ins[c].list[a] = arg;
							}
						}
					}
				}

				puts("\033[1;32m after param/argument subsitution: \033[0m");
				print_instructions(ins, ins_count);

				i--;
				break;
			}
		}
	}

	*/








	// if (find(operation, op_names, op_count) != op_count) {

	// i think we need to find the label locations AFTER we find and expand the macros. i think. 




