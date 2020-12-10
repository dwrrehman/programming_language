#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iso646.h>

typedef int32_t nat;

struct expression { // 3 pointers
	LLVMValueRef value;
	struct expression* args;
	nat count; 
	nat index;
};

struct abstraction { // 4 pointers    
	struct expression* definition;
	nat* syntax;    
	nat length;
	nat type;
	nat use;
	nat _padding;
};

struct context { // 6 pointers
	struct expression* types;
	struct abstraction* names;
	nat* frames;
	nat* indicies;
	nat name_count;
	nat type_count;
	nat index_count;
	nat frame_count;
};

struct stack_element { // 2 + expression pointers;
	struct expression data;
	nat type;
	nat begin;
	nat ind;  
	nat done;
};

enum codegen_type {
	llvm_no_codegen
};

enum action { 
	action_none, 
	action_context,
	action_ir, 
	action_assembly,
	action_objectfile,
	action_executable,
	action_execute,  
	action_count
};

enum intrinsics {
	intrin_undef,
	intrin_root,
	intrin_type,
	intrin_unit,

	intrin_join_p0,
	intrin_join_p1,
	intrin_join,

	intrin_char,
	intrin_A_p0,
	intrin_A,
	intrin_end,

	intrin_define_p0,
	intrin_define_p1,
	intrin_define,

	intrin_do_p0,
	intrin_do
};

enum intrinsic_types {
	intrin_undef_type,
	intrin_root_type,
	intrin_type_type,
	intrin_unit_type,
	intrin_char_type,
};

static const char* action_spellings[] = {
	"--do-nothing", 
	"--emit-context",
	"--emit-llvm-ir",  
	"--emit-assembly", 
	"--emit-object",   
	"--emit-executable",
	"--execute"
};

static inline void print_vector(nat* vector, nat length) { 
	printf("{ ");
	for (nat i = 0; i < length; i++)
		printf("%d ", vector[i]);
	printf("}\n");
}

static inline void debug_program(struct expression* tree, size_t d, struct context* context) {
	if (not tree) {
		for (size_t i = 0; i < d; i++) printf(".   ");
		printf("{NULL}\n");
		return;
	}

	for (size_t i = 0; i < d; i++) printf(".   ");
	printf(" %d \n", tree->index);

	for (nat i = 0; i < tree->count; i++) 
	debug_program(tree->args + i, d + 1, context);
}

static inline void debug_context(struct context* context) { // temp
	printf("---------------- context --------------\n");
	printf("-------- names --------\n");
	printf("name_count = %d\n", context->name_count);
	for (nat i = 0; i < context->name_count; i++) {
		printf("----- [%d] ----- \n", i);        
		printf("\t use = %d\n", context->names[i].use);
		printf("\t type = %d\n", context->names[i].type);
		printf("\t length = %d\n", context->names[i].length);
		printf("\t syntax:     ");
		for (nat s = 0; s < context->names[i].length; s++) {
			nat c = context->names[i].syntax[s];
			if (c >= 256) printf("(%d) ", c - 256);
			else printf("%c ", (char) c);
			
		}                
		printf("\n\t definition:\n");
		debug_program(context->names[i].definition, 0, context); 
	}
	printf("-------------------\n\n");

	printf("-------- types --------\n");
	printf("type_count = %d\n", context->type_count);
	for (nat i = 0; i < context->type_count; i++) {
		printf("----- [%d] ----- \n", i);
		debug_program(context->types + i, 0, context);
	}
	printf("-------------------\n\n");

	printf("---------- indicies --------\n");
	printf("index_count = %d\n", context->index_count);
	printf("indicies:     ");
	print_vector(context->indicies, context->index_count);
	printf("\n");
	printf("---------- frames --------\n");
	printf("frame_count = %d\n", context->frame_count);
	printf("frames:     ");
	print_vector(context->frames, context->frame_count);
	printf("\n");

	printf("-------------------\n\n");
}

static inline void construct_context(struct context* c) { 
	c->name_count = 0;
	c->index_count = 0;
	c->frame_count = 0;    

	c->frames = realloc(c->frames, sizeof(nat) * (size_t) (c->frame_count + 1));
	c->frames[c->frame_count++] = c->index_count;

	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count].type = -1;
	c->names[c->name_count].definition = NULL;
	c->names[c->name_count].use = llvm_no_codegen;
	c->names[c->name_count].length = 5;
	c->names[c->name_count].syntax = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
	c->names[c->name_count].syntax[0] = 'u';
	c->names[c->name_count].syntax[1] = 'n';
	c->names[c->name_count].syntax[2] = 'd';
	c->names[c->name_count].syntax[3] = 'e';
	c->names[c->name_count].syntax[4] = 'f';    
	c->name_count++;

	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count].type = intrin_undef_type;
	c->names[c->name_count].definition = NULL;
	c->names[c->name_count].use = llvm_no_codegen;
	c->names[c->name_count].length = 4;
	c->names[c->name_count].syntax = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
	c->names[c->name_count].syntax[0] = 'r';
	c->names[c->name_count].syntax[1] = 'o';
	c->names[c->name_count].syntax[2] = 'o';
	c->names[c->name_count].syntax[3] = 't';
	c->name_count++;

	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count].type = intrin_root_type;
	c->names[c->name_count].definition = NULL;
	c->names[c->name_count].use = llvm_no_codegen;
	c->names[c->name_count].length = 4;
	c->names[c->name_count].syntax = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
	c->names[c->name_count].syntax[0] = 't';
	c->names[c->name_count].syntax[1] = 'y';
	c->names[c->name_count].syntax[2] = 'p';
	c->names[c->name_count].syntax[3] = 'e';
	c->name_count++;

	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count].type = intrin_type_type;
	c->names[c->name_count].definition = NULL;
	c->names[c->name_count].use = llvm_no_codegen;
	c->names[c->name_count].length = 4;
	c->names[c->name_count].syntax = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
	c->names[c->name_count].syntax[0] = 'u';
	c->names[c->name_count].syntax[1] = 'n';
	c->names[c->name_count].syntax[2] = 'i';
	c->names[c->name_count].syntax[3] = 't';
	c->name_count++;

	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count].type = intrin_unit_type;
	c->names[c->name_count].definition = NULL;
	c->names[c->name_count].use = llvm_no_codegen;
	c->names[c->name_count].length = 1;
	c->names[c->name_count].syntax = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
	c->names[c->name_count].syntax[0] = '1';    
	c->name_count++;

	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count].type = intrin_unit_type;
	c->names[c->name_count].definition = NULL;
	c->names[c->name_count].use = llvm_no_codegen;
	c->names[c->name_count].length = 1;
	c->names[c->name_count].syntax = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
	c->names[c->name_count].syntax[0] = '2';    
	c->name_count++;

	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count].type = intrin_unit_type;
	c->names[c->name_count].definition = NULL;
	c->names[c->name_count].use = llvm_no_codegen;
	c->names[c->name_count].length = 6;
	c->names[c->name_count].syntax = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
	c->names[c->name_count].syntax[0] = 'j';
	c->names[c->name_count].syntax[1] = 'o';
	c->names[c->name_count].syntax[2] = 'i';
	c->names[c->name_count].syntax[3] = 'n';
	c->names[c->name_count].syntax[4] = 256 + intrin_join_p0;
	c->names[c->name_count].syntax[5] = 256 + intrin_join_p1;
	c->name_count++;


	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count].type = intrin_type_type;
	c->names[c->name_count].definition = NULL;
	c->names[c->name_count].use = llvm_no_codegen;
	c->names[c->name_count].length = 4;
	c->names[c->name_count].syntax = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
	c->names[c->name_count].syntax[0] = 'c';
	c->names[c->name_count].syntax[1] = 'h';
	c->names[c->name_count].syntax[2] = 'a';
	c->names[c->name_count].syntax[3] = 'r';    
	c->name_count++;


	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count].type = intrin_char_type;
	c->names[c->name_count].definition = NULL;
	c->names[c->name_count].use = llvm_no_codegen;
	c->names[c->name_count].length = 1;
	c->names[c->name_count].syntax = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
	c->names[c->name_count].syntax[0] = '5';
	c->name_count++;

	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count].type = intrin_char_type;
	c->names[c->name_count].definition = NULL;
	c->names[c->name_count].use = llvm_no_codegen;
	c->names[c->name_count].length = 2;
	c->names[c->name_count].syntax = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
	c->names[c->name_count].syntax[0] = 'A';    
	c->names[c->name_count].syntax[1] = 256 + intrin_A_p0;
	c->name_count++;

	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count].type = intrin_char_type;
	c->names[c->name_count].definition = NULL;
	c->names[c->name_count].use = llvm_no_codegen;
	c->names[c->name_count].length = 0;
	c->names[c->name_count].syntax = calloc((size_t) c->names[c->name_count].length, sizeof(nat));    
	c->name_count++;



	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count].type = intrin_char_type;
	c->names[c->name_count].definition = NULL;
	c->names[c->name_count].use = llvm_no_codegen;
	c->names[c->name_count].length = 1;
	c->names[c->name_count].syntax = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
	c->names[c->name_count].syntax[0] = '3';
	c->name_count++;


	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count].type = intrin_type_type;
	c->names[c->name_count].definition = NULL;
	c->names[c->name_count].use = llvm_no_codegen;
	c->names[c->name_count].length = 1;
	c->names[c->name_count].syntax = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
	c->names[c->name_count].syntax[0] = '4';
	c->name_count++;


	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count].type = intrin_unit_type;
	c->names[c->name_count].definition = NULL;
	c->names[c->name_count].use = llvm_no_codegen;
	c->names[c->name_count].length = 8;
	c->names[c->name_count].syntax = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
	c->names[c->name_count].syntax[0] = 'd';
	c->names[c->name_count].syntax[1] = 'e';
	c->names[c->name_count].syntax[2] = 'f';
	c->names[c->name_count].syntax[3] = 'i';
	c->names[c->name_count].syntax[4] = 'n';
	c->names[c->name_count].syntax[5] = 'e';
	c->names[c->name_count].syntax[6] = 256 + intrin_define_p0;
	c->names[c->name_count].syntax[7] = 256 + intrin_define_p1;
	c->name_count++;


	c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
	c->indicies[c->index_count++] = intrin_end;



	c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
	c->indicies[c->index_count++] = intrin_undef;

	c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
	c->indicies[c->index_count++] = intrin_root;

	c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
	c->indicies[c->index_count++] = intrin_type;

	c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
	c->indicies[c->index_count++] = intrin_unit;


	c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
	c->indicies[c->index_count++] = intrin_join_p0;

	c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
	c->indicies[c->index_count++] = intrin_join_p1;

	c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
	c->indicies[c->index_count++] = intrin_join;


	c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
	c->indicies[c->index_count++] = intrin_char;

	c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
	c->indicies[c->index_count++] = intrin_A_p0;

	c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
	c->indicies[c->index_count++] = intrin_A;

	c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
	c->indicies[c->index_count++] = intrin_define_p0;

	c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
	c->indicies[c->index_count++] = intrin_define_p1;

	c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
	c->indicies[c->index_count++] = intrin_define;
}


static inline void destroy_program(struct expression* __attribute__((unused)) program) {
	// unimplemented;
	return;
}

static inline void destroy_context(struct context* __attribute__((unused)) program) {
	// unimplemented;
	return;
}


static inline void compile(const char* filename, int8_t* text, nat length, LLVMModuleRef module, char* llvm_error) {

	LLVMModuleRef new = LLVMModuleCreateWithName(filename);
	LLVMBuilderRef builder = LLVMCreateBuilder();

	struct context context = {0};
	construct_context(&context);

	const nat stack_size = 65536;
	const nat top_level_type = intrin_unit;

	nat begin = 0, best = 0, top = 0, done = 0, line = 1, column = 1;

	while (begin < length && text[begin] <= ' ') {
		if (text[begin] == '\n') { 
			line++; 
			column = 1; 
		} else column++;
		begin++; 
		if (begin > best) best = begin;
	}

	struct stack_element* stack = malloc(sizeof(struct stack_element) * (size_t) stack_size);
	memset(stack, 0, sizeof(struct stack_element));

	stack[0].ind = context.index_count;
	stack[0].type = top_level_type;
	stack[0].begin = begin;    

_0:
	if (not stack[top].ind) {
		if (not top) {
			fprintf(stderr, "compiler: %s: %u:%u: error: unresolved %c\n",
				filename, line, column, best == length ? ' ' : text[best]);       	    
			stack[top].data.index = -1;
			goto _3;
		}
		top--; 
		goto _2;
	}

	stack[top].ind--;
	done = 0;
	begin = stack[top].begin;

_1: 
	stack[top].data.index = context.indicies[stack[top].ind];
	struct abstraction name = context.names[stack[top].data.index];            

	if (stack[top].type != name.type) goto _2;	

	while (done < name.length) {

		nat c = name.syntax[done];
		done++;

		if (c >= 256 and top + 1 < stack_size) {		   
			top++;
			stack[top] = (struct stack_element){0};
			stack[top].ind = context.index_count;
			stack[top].type = context.names[c - 256].type;
			stack[top].done = done;
			stack[top].begin = begin;
			goto _0;
		}

		if (begin >= length or c != text[begin]) goto _2;

		column++;
		begin++;
		if (begin > best) best = begin;
		while (begin < length && text[begin] <= ' ') {
			if (text[begin] == '\n') { 
				line++; 
				column = 1; 
			} else column++;
			begin++; 
			if (begin > best) best = begin;
		}
	}

	if (top) {
		done = stack[top].done;
		top--;
		stack[top].data.args = realloc(stack[top].data.args, sizeof(struct expression) 
						* (size_t) (stack[top].data.count + 1));
		stack[top].data.args[stack[top].data.count++] = stack[top + 1].data;
		goto _1;
	}

	if (begin == length) goto _3;

_2:
	free(stack[top].data.args);
	stack[top].data.args = NULL;
	stack[top].data.count = 0;
	goto _0;
	_3:

	if (LLVMVerifyModule(new, LLVMPrintMessageAction, &llvm_error) or LLVMLinkModules2(module, new)) {
		fprintf(stderr, "llvm: error5: %s\n", llvm_error);
		LLVMDisposeMessage(llvm_error);
		exit(5);
	}

	debug_context(&context);
	debug_program(&stack->data, 0, &context);

	destroy_program(&stack->data);            
	destroy_context(&context);

	free(stack);
	munmap(text, (size_t) length);
	LLVMDisposeBuilder(builder);
}


int main(int argc, const char** argv, const char** envp) {

	LLVMInitializeAllTargetInfos();
	LLVMInitializeAllTargets();
	LLVMInitializeAllTargetMCs();
	LLVMInitializeAllAsmParsers();
	LLVMInitializeAllAsmPrinters();
	LLVMLinkInInterpreter();
	LLVMLinkInMCJIT();
	LLVMInitializeNativeTarget();

	LLVMModuleRef module = LLVMModuleCreateWithName("main_module.n");

	char* llvm_error = NULL;
	const char* output_name = "out";
	nat at = argc;
	enum action action_type = action_none;

	for (nat i = 1; i < argc; i++) {        

		if (argv[i][0] == '-') {

			for (unsigned int a = action_none; a < action_count; a++) {
				if (not strcmp(argv[i], action_spellings[a])) {
					action_type = a; 
					goto arg_success;
				}
			}

			if (not strcmp(argv[i], "--")) { 
				at = i + 1; 
				break;
			}

			if (not strcmp(argv[i], "--name")) {
				if (i + 1 < argc) output_name = argv[++i];                
				else {
					fprintf(stderr, "compiler: error0: argument not supplied for option --name <string>\n");
					exit(1);
				}
				continue;
			}

			fprintf(stderr, "compiler: error1: unknown argument: %s\n", argv[i]);            
			exit(1);
			arg_success: continue;
		}

		const char* ext = strrchr(argv[i], '.');

		if (ext and not strcmp(ext, ".n")) {

			struct stat file_data = {0};
			int file = open(argv[i], O_RDONLY);            
			if (file < 0 or stat(argv[i], &file_data) < 0) {
				fprintf(stderr, "compiler: error3: %s: ", argv[i]);
				perror("open");
				exit(3);
			} 

			nat length = (nat) file_data.st_size;
			int8_t* text = not length ? NULL : mmap(0, (size_t) length, PROT_READ, MAP_SHARED, file, 0);    

			if (text == MAP_FAILED) {
				fprintf(stderr, "compiler: error4: %s: ", argv[i]);
				perror("mmap");
				exit(4);
			} else close(file);

			compile(argv[i], text, length, module, llvm_error);	    

		} else if (ext and not strcmp(ext, ".ll")) {
			LLVMModuleRef new = NULL;
			LLVMMemoryBufferRef buffer;            
			if (	LLVMCreateMemoryBufferWithContentsOfFile(argv[i], &buffer, &llvm_error) or
				LLVMParseIRInContext(LLVMGetGlobalContext(), buffer, &new, &llvm_error) or
				LLVMVerifyModule(new, LLVMPrintMessageAction, &llvm_error) or
				LLVMLinkModules2(module, new)) {

				fprintf(stderr, "llvm: error6: %s\n", llvm_error);
				LLVMDisposeMessage(llvm_error);
				exit(6);
			}
		} else {
		// fprintf(stderr, "compiler: %s: error7: unknown file type: \"%s\"\n", argv[i], ext);
			compile("<inline_string>", (int8_t*) (argv[i]), (nat) strlen(argv[i]), module, llvm_error);
		}
	}

	if (LLVMVerifyModule(module, LLVMPrintMessageAction, &llvm_error)) {
		fprintf(stderr, "llvm: error8: %s\n", llvm_error);
		LLVMDisposeMessage(llvm_error);
		exit(8);
	}

	// ----- {optimize} -------
	// ----- {/optimize} -------

	if (action_type == action_ir) {	
		if (LLVMPrintModuleToFile(module, output_name, &llvm_error)) {
			fprintf(stderr, "llvm: error9: %s\n", llvm_error);
			LLVMDisposeMessage(llvm_error);
			exit(9);
		}

	} else if (action_type == action_execute) {

		LLVMExecutionEngineRef engine = NULL;
		struct LLVMMCJITCompilerOptions options;
		LLVMInitializeMCJITCompilerOptions(&options, sizeof options); 

		if (LLVMCreateMCJITCompilerForModule(&engine, module, &options, sizeof options, &llvm_error)) {
			fprintf(stderr, "llvm: error10: %s\n", llvm_error);
			LLVMDisposeMessage(llvm_error);
			exit(10);
		}

		LLVMValueRef main_function = NULL;	
		if (LLVMFindFunction(engine, "main", &main_function)) {
			fprintf(stderr, "compiler: error11: no entry point for executable\n");
			exit(11);
		}

		int code = LLVMRunFunctionAsMain(engine, main_function, (unsigned) (argc - at), argv + at, envp);
		LLVMDisposeExecutionEngine(engine);
		exit(code);


	} else if (	action_type == action_assembly or 
			action_type == action_objectfile or 
			action_type == action_executable) {

		char* emit_filename = (char*) (intptr_t) output_name; 

		if (action_type == action_executable) {
			size_t emit_filename_size = strlen(output_name) + 2 + 1;
			emit_filename = calloc(emit_filename_size, sizeof(char));
			strncpy(emit_filename, output_name, emit_filename_size);
			strncat(emit_filename, ".o", emit_filename_size);		 
		}

		LLVMCodeGenFileType output_filetype = action_type == action_assembly ? LLVMAssemblyFile : LLVMObjectFile;

		LLVMCodeGenOptLevel optimization_level = LLVMCodeGenLevelDefault; /// make this configurable?...
		LLVMTargetRef target = NULL;        
		const char* triple = LLVMGetDefaultTargetTriple();
		const char* name = LLVMGetHostCPUName();
		const char* features = LLVMGetHostCPUFeatures();

		if (LLVMGetTargetFromTriple(triple, &target, &llvm_error)) {
			fprintf(stderr, "llvm: error12: get target from triple: %s\n", llvm_error);
			LLVMDisposeMessage(llvm_error);
			exit(12);
		}

		LLVMTargetMachineRef target_machine = LLVMCreateTargetMachine
		(target, triple, name, features, optimization_level, 
		LLVMRelocDefault, LLVMCodeModelDefault);

		if (LLVMTargetMachineEmitToFile(target_machine, module, emit_filename, output_filetype, &llvm_error)) {
			fprintf(stderr, "llvm: error13: target machine emit: %s\n", llvm_error);
			LLVMDisposeMessage(llvm_error);
			exit(13);
		}

		if (action_type == action_executable) {
			char linker_string[4096] = {0};
			snprintf(linker_string, sizeof linker_string, 
				"ld64.lld "
				"-demangle -lto_library /usr/local/Cellar/llvm/11.0.0/lib/libLTO.dylib "
				"-syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX11.0.sdk "
				"-macosx_version_min 11.0 -sdk_version 11.0 "
				"-lSystem -lc "
				"-o %s %s", output_name, emit_filename);
			system(linker_string);
			remove(emit_filename);
			free(emit_filename);
		}
	}
	exit(0);
}
