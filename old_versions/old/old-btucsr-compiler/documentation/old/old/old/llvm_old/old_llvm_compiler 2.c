//////////////////////////////////////////////////////////////////////////////////////////
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

struct expression {
	struct expression* args;
	void* value;
	nat count; // at most 31.
	nat index;
};

struct abstraction {	
	struct expression type;
	struct expression def;
	nat* syntax;
	nat length;
	nat use;
};

struct context {	
	struct abstraction* names;
	nat* frames;
	nat* indicies;
	nat name_count;	
	nat index_count;
	nat frame_count;
	nat top_level;
};

struct stack_element {
	struct expression data;
	nat param;
	nat begin;
	nat ind;
	nat done;
	nat eval; // bool. rework.
	nat _padding;
};

enum codegen_type {
	codegen_macro,
	codegen_llvm_function,
	codegen_llvm_function_parameter,
	codegen_llvm_structure,
	codegen_llvm_structure_member,
	codegen_llvm_global_constant,
	codegen_llvm_global_variable,
	codegen_llvm_local_variable,
	codegen_llvm_attribute,
};

enum intrinsics {
	intrin_undef,
	intrin_root,
	intrin_type,
	intrin_unit,
	intrin_char,
	intrin_pass,

	intrin_literal_first = 6,  //   6 + 27 = 33 = '!'
	intrin_literal_last = 228, // 228 + 27 = 255 = '\u255'

	intrin_param_p0,
	intrin_param,

	intrin_scope_p0,
	intrin_scope,

	intrin_join_p0,
	intrin_join_p1,
	intrin_join,

	intrin_declare0_p0,

	intrin_declare0 = 237,
	intrin_declare31 = 795,



	// intrin_decltype_p0,
	// intrin_decltype_p1,
	// intrin_decltype,

	// intrin_define_p0,
	// intrin_define_p1,
	// intrin_define_p2,
	// intrin_define,

	// intrin_deftype_p0,
	// intrin_deftype_p1,
	// intrin_deftype_p2,
	// intrin_deftype,
	
	_intrin_count
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

static const char* action_spellings[] = {
	"--do-nothing", 
	"--emit-context",
	"--emit-llvm-ir",  
	"--emit-assembly", 
	"--emit-object",   
	"--emit-executable",
	"--execute"
};

static inline void debug_program(struct expression tree, nat d, struct context* context) {
	
	for (nat i = 0; i < d; i++) printf(".   ");
	printf("[%d] ", tree.index);

	for (nat s = 0; s < context->names[tree.index].length; s++) {
		nat c = context->names[tree.index].syntax[s];
		if (c >= 256) printf(" (%d) ", c - 256);
		else if (c >= 127) printf(" [c=%d] ", c);
		else printf("%c", (char) c);
	}
	printf("\n\n");

	for (nat i = 0; i < tree.count; i++) debug_program(tree.args[i], d + 1, context);
}

static inline void debug_context(struct context* context) {
	printf("---------------- context --------------\n");
	printf("-------- names --------\n");
	printf("name_count = %d\n", context->name_count);
	for (nat i = 0; i < context->name_count; i++) {

		printf("----- [%d] ----- \n", i);
		
		printf("\t syntax (%d):     ", context->names[i].length);
		for (nat s = 0; s < context->names[i].length; s++) {
			nat c = context->names[i].syntax[s];
			if (c >= 256) printf(" (%d) ", c - 256);
			else if (c >= 127) printf(" [c=%d] ", c);
			else printf("%c", (char) c);			
		}
		puts("");
		printf("\t type:\n");
		debug_program(context->names[i].type, 0, context);
			
		printf("\t def:\n");
		debug_program(context->names[i].def, 0, context);
	}
	printf("-------------------\n\n---------- indicies --------\n");	
	printf("index_count = %d\n", context->index_count);
	printf("indicies:     { ");
	for (nat i = 0; i < context->index_count; i++) printf("%d ", context->indicies[i]);
	printf("}\n\n---------- frames --------\n");	
	printf("frame_count = %d\n", context->frame_count);
	printf("frames:     { ");	
	for (nat i = 0; i < context->frame_count; i++) printf("%d ", context->frames[i]);
	printf("}\n\n-------------------\n\n");	
}

static inline void add(nat* signature, nat type, struct context* c) {
	
	struct abstraction new = {0};	

	new.type.index = type;
	
	new.syntax = calloc((size_t) 100, sizeof(nat));
	for (nat i = 0; signature[i]; i++)
		new.syntax[new.length++] = signature[i];	
	
	c->names = realloc(c->names, sizeof(struct abstraction) * (size_t) (c->name_count + 1));
	c->names[c->name_count++] = new;
}

static inline void add_indicies(nat* ind, struct context* c) {
	for (nat i = 0; ind[i] != _intrin_count; i++) {
		nat place = c->index_count;
		while (place > 0 and c->names[ind[i]].length < c->names[c->indicies[place - 1]].length) place--;
		c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
		memmove(c->indicies + place + 1, c->indicies + place, sizeof(nat) * (size_t) (c->index_count - place));
		c->indicies[place] = ind[i];
		c->index_count++;
	}
}

static inline struct context* construct_context() { 

	struct context* c = calloc(1, sizeof(struct context));
	c->top_level = intrin_pass;
	c->frames = realloc(c->frames, sizeof(nat) * (size_t) (c->frame_count + 1));
	c->frames[c->frame_count++] = c->index_count;
	
	add((nat[]){'u','n','d','e','f', 0}, intrin_undef, c);
	add((nat[]){'r','o','o','t', 0}, intrin_undef, c);
	add((nat[]){'t','y','p','e', 0}, intrin_root, c);
	add((nat[]){'u','n','i','t', 0}, intrin_type, c);
	add((nat[]){'c','h','a','r', 0}, intrin_type, c);
	add((nat[]){'p','a','s','s', 0}, intrin_unit, c);

	for (nat literal = 33; literal < 256; literal++) 
		add((nat[]){literal, 0}, intrin_char, c);

	add((nat[]){'0', 0}, intrin_unit, c);
	add((nat[]){'p','a','r','a','m', 
		256 + intrin_param_p0, 
	0}, intrin_char, c);


	add((nat[]){'0', 0}, intrin_unit, c);
	add((nat[]){'s','c','o','p','e',
		256 + intrin_scope_p0, 
	0}, intrin_unit, c);


	add((nat[]){'0', 0}, intrin_unit, c);
	add((nat[]){'1', 0}, intrin_unit, c);
	add((nat[]){'j','o','i','n', 
		256 + intrin_join_p0, 
		256 + intrin_join_p1, 
	0}, intrin_unit, c);


	const nat maximum_count = 32;
	nat last_declare = c->name_count; 

	nat declare_indicies[maximum_count + 1] = {0};
	nat declare_count = 0;

	for (nat count = 0; count < maximum_count; count++) {

		for (nat i = 0; i < count; i++) 
			add((nat[]){'_', 0}, intrin_char, c);

		add((nat[]){'T', 0}, intrin_type, c);

		nat* args = malloc(sizeof(nat) * 100);
		nat arg_count = 0;		

		args[arg_count++] = 'd';
		args[arg_count++] = 'e';
		args[arg_count++] = 'c';
		args[arg_count++] = 'l';
		args[arg_count++] = 'a';
		args[arg_count++] = 'r';
		args[arg_count++] = 'e';

		for (nat i = 0; i <= count; i++) 
			args[arg_count++] = 256 + last_declare + i;
		
		args[arg_count++] = 0;

		declare_indicies[declare_count++] = c->name_count;

		add(args, intrin_unit, c);
		last_declare = c->name_count;
	} 

	declare_indicies[declare_count++] = _intrin_count;

	for (nat literal = intrin_literal_first; literal <= intrin_literal_last; literal++) {
		c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
		c->indicies[c->index_count++] = literal;
	}

	add_indicies(declare_indicies, c);

	add_indicies((nat[]){
		intrin_root, 
		intrin_type,
		intrin_unit,
		intrin_char,
		intrin_undef,
		intrin_pass,
		intrin_join,
		intrin_scope,
		intrin_param,
		_intrin_count,
	}, c);
	
	return c;
}

static inline uint8_t* open_file(const char* filename, nat* out_length) {

	struct stat file_data = {0};
	int file = open(filename, O_RDONLY);
	if (file < 0 or stat(filename, &file_data) < 0) {
		fprintf(stderr, "compiler: error3: %s: ", filename);
		perror("open");
		exit(3);
	}

	size_t length = (size_t) file_data.st_size;
	if (not length) return NULL;
	uint8_t* text = mmap(0, length, PROT_READ, MAP_SHARED, file, 0);
	close(file);
	
	if (text == MAP_FAILED) {
		fprintf(stderr, "compiler: error4: %s: ", filename);
		perror("mmap");
		exit(4);
	}

	*out_length = (nat) length;
	return text;
}

static inline void print_error(nat best, nat best_index, nat length, uint8_t* text, 
				const char* filename, struct context* context) {

	nat at = 0, line = 1, column = 1;
	while (at < best) {
		if (text[at] == '\n') { line++; column = 1; } else column++;
		at++;
	}
	
	fprintf(stderr, "compiler: %s: %u:%u: error: unresolved %c\n",
	filename, line, column, best == length ? ' ' : text[best]);
	
	struct abstraction candidate = context->names[best_index];
	
	printf("...did you mean:  ");
	for (nat s = 0; s < candidate.length; s++) {
		nat c = candidate.syntax[s];
		if (c >= 256) printf(" (%d) ", c - 256);
		else printf("%c", (char) c);
	}
	printf(" which has type: "); debug_program(candidate.type, 0, context);	
}

static inline void destroy_program(struct expression* program) { 
	//TODO: MAKE NON RECURSIVE	
	for (nat i = 0; i < program->count; i++) 
		destroy_program(program->args + i);
	free(program->args);
	program->args = NULL;
	program->count = 0;
}

static inline nat expressions_equal(struct expression a, struct expression b, struct context* c) { 
	//TODO: MAKE NON RECURSIVE
	if (c->names[a.index].def.index) return expressions_equal(c->names[a.index].def, b, c);
	if (a.index != b.index or a.count != b.count) return 0;
	for (nat i = 0; i < a.count; i++)
		if (not expressions_equal(a.args[i], b.args[i], c)) return 0;	
	return 1;
}

static inline void eval_intrinsic(struct expression* this, struct context* context, 
				struct stack_element* stack, nat top) {

	const nat index = this->index;
	
	if (index == intrin_param)  this->value = this->args->value;

	else if (index >= intrin_literal_first and 
		 index <= intrin_literal_last) {
	
		this->value = malloc(sizeof(nat));
		*((nat*)this->value) = this->index + 27;
	
	} else if (index == intrin_scope) {

		context->index_count = context->frames[--context->frame_count];
		this->value = this->args->value;

	} else if (index >= intrin_declare0 and 
		   index <= intrin_declare31) {

		if (context->frame_count < 2) {			
			printf("error: incorrect usage of decl variant: fc < 2, (%d < 2)\n", context->frame_count);
			return;
		}

		this->value = malloc(sizeof(nat));
		*((nat*)this->value) = context->name_count + 256;
		
		nat length = this->count - 1;
		nat* syntax = malloc(sizeof(nat) * (size_t) length);

		for (nat i = 0; i < length; i++) 
			syntax[i] = *((nat*)this->args[i].value);
		
		struct abstraction new_name = {0};
		new_name.syntax = syntax;
		new_name.length = length;
		new_name.type = this->args[length];
		
		context->names = realloc(context->names, sizeof(struct abstraction) * (size_t) (context->name_count + 1));
		context->names[context->name_count++] = new_name;
		
		nat frame = context->frame_count - 1, place = context->frames[frame];
		while (place > context->frames[frame - 1] and 
			length < context->names[context->indicies[place - 1]].length) 
				place--;

		context->indicies = realloc(context->indicies, sizeof(nat) * (size_t) (context->index_count + 1));
		memmove(context->indicies + place + 1, context->indicies + place, 
			sizeof(nat) * (size_t) (context->index_count - place));

		context->indicies[place] = context->name_count - 1;
		context->index_count++;
		context->frames[frame]++;

		for (nat s = 0; s <= top; s++) 
			if (place <= stack[s].ind) stack[s].ind++;
	}
}

static inline struct expression clone(struct expression e, struct context* context, struct stack_element* stack, nat top) { 
	//TODO: MAKE NON RECURSIVE

	struct expression new = e;
	new.args = calloc((size_t) e.count, sizeof(struct expression));

	if (new.index == intrin_scope) {
		context->frames = realloc(context->frames, sizeof(nat) * (size_t) (context->frame_count + 1));
		context->frames[context->frame_count++] = context->index_count;
	}

	for (nat i = 0; i < e.count; i++)
		new.args[i] = clone(e.args[i], context, stack, top);
	eval_intrinsic(&new, context, stack, top);
	
	return new;
}

static inline void copy_replace(struct expression def, struct expression call, struct expression* out,
				struct context* context, struct stack_element* stack, nat top) {
	//TODO: MAKE NON RECURSIVE

	if (def.index >= call.index - call.count and def.index < call.index) { 
		*out = clone(call.args[call.count - (call.index - def.index)], context, stack, top);
		return;
	}
	*out = def;
	out->args = calloc((size_t) def.count, sizeof(struct expression));

	if (out->index == intrin_scope) {
		context->frames = realloc(context->frames, sizeof(nat) * (size_t) (context->frame_count + 1));
		context->frames[context->frame_count++] = context->index_count;
	}

	for (nat i = 0; i < def.count; i++) 
		copy_replace(def.args[i], call, out->args + i, context, stack, top);
	eval_intrinsic(out, context, stack, top);
}

static inline void expand_macro(struct context* context, struct stack_element* stack, nat top) {
	struct expression call = stack[top].data;	
	copy_replace(context->names[call.index].def, call, &stack[top].data, context, stack, top);
}

static inline void compile(const char* filename, uint8_t* text, nat length, LLVMModuleRef main_module) {
	struct context* context = construct_context();
	
	const nat stack_size = 65536;
	nat index = 0, begin = 0, best = 0, candidate = 0, top = 0, done = 0, error = 0;
	while (begin < length and text[begin] <= ' ') begin++;
	if (begin > best) best = begin;
	
	struct stack_element* stack = malloc(sizeof(struct stack_element) * (size_t) stack_size);
	stack->data = (struct expression) {0};
	stack->eval = 1;
	stack->param = context->top_level;
	stack->ind = context->index_count;	
	stack->begin = begin;
_0:
	if (not stack[top].ind) {
		if (not top) {
			print_error(best, candidate, length, text, filename, context);
			error = 1;
			goto _3;
		}
		top--;
		goto _2;
	}
	stack[top].ind--;
	done = 0;
	begin = stack[top].begin;
_1:
	index = context->indicies[stack[top].ind];
	stack[top].data.index = index;
	struct abstraction name = context->names[index];
	
	if (not expressions_equal(context->names[stack[top].param].type, name.type, context)) goto _2;
	
	while (done < name.length) {
		nat c = name.syntax[done];
		done++;
		if (c >= 256 and top + 1 < stack_size) {

			if (stack[top].eval and index == intrin_scope) {  ///TODO: make a function to do this eval.
				context->frames = realloc(context->frames, sizeof(nat) * (size_t) (context->frame_count + 1));
				context->frames[context->frame_count++] = context->index_count;
			}

			top++;
			stack[top] = (struct stack_element){0};
			stack[top].ind = context->index_count;
			stack[top].param = c - 256;
			stack[top].done = done;
			stack[top].begin = begin;
			stack[top].eval = stack[top - 1].eval and (name.use or not name.def.index);			
			goto _0;
		}

		if (begin >= length or c != text[begin]) goto _2;		
		do begin++; while (begin < length and text[begin] <= ' ');
		if (begin > best) { best = begin; candidate = index; }
	}	
	
	if (stack[top].eval) eval_intrinsic(&stack[top].data, context, stack, top);
	if (not name.use and name.def.index) expand_macro(context, stack, top);

	if (top) {
		context->names[stack[top].param].def = stack[top].data;
		done = stack[top].done;
		top--;
		stack[top].data.args = realloc(stack[top].data.args, sizeof(struct expression) * (size_t) (stack[top].data.count + 1));
		stack[top].data.args[stack[top].data.count] = stack[top + 1].data;
		stack[top].data.count++;
		goto _1;
	}

	if (begin == length) goto _3;
_2:
	free(stack[top].data.args);
	stack[top].data.args = NULL;
	stack[top].data.count = 0;
	goto _0;
_3:;

	LLVMModuleRef new = LLVMModuleCreateWithName(filename);
	LLVMBuilderRef builder = LLVMCreateBuilder();
	char* llvm_error = NULL;
	// do nonrecursive walk of program and codegenerate using builder.
	debug_context(context);
	debug_program(stack->data, 0, context);
	LLVMDisposeBuilder(builder);
	destroy_program(&stack->data);

	for (nat i = 0; i < context->name_count; i++) free(context->names[i].syntax);
	free(context->indicies);
	free(context->frames);	
	free(context);
	free(stack);
	
	if (	LLVMVerifyModule(new, LLVMPrintMessageAction, &llvm_error) or 
		LLVMLinkModules2(main_module, new)) {		
		fprintf(stderr, "llvm: error5: %s\n", llvm_error);
		LLVMDisposeMessage(llvm_error);
		exit(5);
	}

	if (error) {
		printf("compiler: there was an error. exiting...\n");
		exit(1);
	} else puts("success");	
}

int main(int argc, const char** argv) {

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
	enum action action_type = action_none;

	for (nat i = 1; i < argc; i++) {

		if (argv[i][0] == '-') {

			for (unsigned int a = action_none; a < action_count; a++) {
				if (not strcmp(argv[i], action_spellings[a])) {
					action_type = a; 
					goto arg_success;
				}
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
			
			const char* filename = argv[i]; 
			nat length = 0;
			uint8_t* text = open_file(filename, &length);
			compile(filename, text, length, module);
			munmap(text, (size_t) length);
			
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
			compile("<inline_string>", (uint8_t*) (intptr_t) (argv[i]), (nat) strlen(argv[i]), module);
		}
	}

	if (LLVMVerifyModule(module, LLVMPrintMessageAction, &llvm_error)) {
		fprintf(stderr, "llvm: error8: %s\n", llvm_error);
		LLVMDisposeMessage(llvm_error);
		exit(8);
	}
// backend:
	if (action_type == action_ir) {	
		if (LLVMPrintModuleToFile(module, output_name, &llvm_error)) {
			fprintf(stderr, "llvm: error9: %s\n", llvm_error);
			LLVMDisposeMessage(llvm_error);
			exit(9);
		}

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

		LLVMCodeGenFileType output_filetype = action_type == action_assembly 
							? LLVMAssemblyFile : LLVMObjectFile;

		LLVMCodeGenOptLevel optimization_level = LLVMCodeGenLevelDefault;
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

		if (LLVMTargetMachineEmitToFile(target_machine, module, emit_filename, 
						output_filetype, &llvm_error)) {

			fprintf(stderr, "llvm: error13: target machine emit: %s\n", llvm_error);
			LLVMDisposeMessage(llvm_error);
			exit(13);
		}

		if (action_type == action_executable) {
			char linker_string[4096] = {0};
			snprintf(linker_string, sizeof linker_string,
				"ld64.lld "
				"-demangle -lto_library /usr/local/Cellar/llvm/11.0.0/lib/libLTO.dylib "
				"-syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk "
				"-macosx_version_min 11.1 -sdk_version 11.1 "
				"-lSystem -lc "
				"-o %s %s", output_name, emit_filename);
			system(linker_string);
			remove(emit_filename);
			free(emit_filename);
		}
	}
	exit(0);
}



/*


join	scope define CAB 
		param scope declare A end name
		param scope decltype B end type
		end unit

		scope declare A B

join CAB CC param scope declare BB end unit end unit
join CC pass

join scope deftype ALIAS FOR UNIT end type unit

join scope declare HELLO THERE FROM SPACE end ALIAS FOR UNIT

join scope declare ABCDEFGHIJKLMNOPQRSTUVWXYZ end ALIAS FOR UNIT

pass




*/



