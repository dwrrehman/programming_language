#include <stdio.h>
#include <iso646.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <mach-o/loader.h> // technically not even neccessary.

static inline void print_program(int* program, int* context, int p, int depth) {
	for (int i = 0; i < depth; i++) printf(".   ");
	int index = program[p], count = program[p + 1];
	printf("p=%d:  [i=%d] : (c=%d) : ", p, index, count);
	fflush(stdout);
	int* n = context + index;
	for (int j = 0; j <= n[1]; j++) {
		int c = n[j + 2];
		if (c < 33) printf(" char{%d} ", c);
		else if (c < 128) printf("%c ", c);
		else if (c < 256) printf(" unicode{%d} ", c);
		else printf(" (%d) ", c);
	}
	puts("\n");
	for (int i = 0; i < count; i++) print_program(program, context, program[p + i + 2], depth + 1);
}

static inline void print_context(int alphabet, int index_count, int context_count, int* context, int* indicies) {

	printf("context indexes: (index_count = %d, context_count = %d, alphabet = %d)\n", 
		index_count, context_count, alphabet);

	for (int i = 0; i < index_count; i++) {
		int index = indicies[i];
		int* n = context + index;
		printf("i=%d index=%d  |  (def=%d)(length=%d) [ ", i, index, n[0], n[1]);
		for (int j = 0; j <= n[1]; j++) {
			int c = n[j + 2];
			if (c < 33) printf(" char{%d} ", c);
			else if (c < 128) printf("%c ", c);
			else if (c < 256) printf(" unicode{%d} ", c);
			else printf(" (%d) ", c);
		}
		printf(" ] \n");
	}
	printf("-----------------------------\n\n");
}

static inline void* open_file(const char* filename, int* length) {

	struct stat file_data = {0};
	const int file = open(filename, O_RDONLY);

	if (file < 0 or stat(filename, &file_data) < 0) {
		fprintf(stderr, "error: %s: ", filename);
		perror("open");
		exit(3);
	}

	*length = (int) file_data.st_size;
	if (not *length) return NULL;
	void* input = mmap(0, (size_t) *length, PROT_READ, MAP_SHARED, file, 0);
	if (input == MAP_FAILED) {
		fprintf(stderr, "error: %s: ", filename);
		perror("mmap");
		exit(4);
	}
	close(file);
	return input;
}

int main(int argc, const char** argv) {

	if (argc < 2) return 1;
	const char* filename = argv[1], * reason = NULL;
	const int 
		program_limit = 4096,
		context_limit = 4096,
		index_limit = 4096,
		argument_limit = 4096,
		stack_limit = 4096;
	
	int* program = malloc(program_limit * sizeof(int));
	int* context = malloc(context_limit * sizeof(int));
	int* indicies = malloc(index_limit * sizeof(int));
	int* arguments = malloc(argument_limit * sizeof(int));
	int* stack = malloc(stack_limit * sizeof(int));

	int program_count = 0, context_count = 0, index_count = 0, alphabet = 0,
	    arg = 0, top = 0, begin = 0, index = 0, count = 0, done = 0, 
	    best = 0, candidate = 0;

	int length = 0;
	unsigned char* input = open_file(filename, &length);

	{
		int base_length = 0;
		int* base = open_file("i.i", &base_length);
		index_count = base[0];
		context_count = base[1];
		alphabet = base[2];
		memcpy(indicies, base + 3, sizeof(int) * (size_t) index_count);
		memcpy(context, base + index_count + 3, sizeof(int) * (size_t) context_count);
		munmap(base, (size_t) base_length);
	}

	while (begin < length and input[begin] < 33) begin++;
	if (begin > best) best = begin;
	stack[top] = index_count;
	stack[top + 2] = alphabet;
	stack[top + 3] = begin;
	stack[top + 7] = program_count;
try:
	if (not stack[top]) { 
		if (not top) { reason = "unresolved expression"; goto error; }
		top -= 8; 
		goto try; 
	}
	stack[top]--;
	index = indicies[stack[top]];
	
	int actual_type = context[index + context[index + 1] + 2],  expected_type = stack[top + 2];
	if (not actual_type) goto try;
	if (expected_type != alphabet and expected_type != actual_type) goto try; 

	done = 0;
	count = 0;
	begin = stack[top + 3];
	program_count = stack[top + 7];
parent:
	while (done < context[index + 1]) {
		int element = context[index + done + 2];
		done++;

		if (element >= alphabet) {
			top += 8;
			if (top + 7 >= stack_limit) { reason = "stack limit exceeded"; goto error; } 
			stack[top] = index_count;
			stack[top + 1] = done;
			stack[top + 2] = element;
			stack[top + 3] = begin;
			stack[top + 4] = index;
			stack[top + 5] = count;
			stack[top + 6] = arg;
			stack[top + 7] = program_count;
			arg += count;
			goto try;
		}
		if (begin >= length or element != input[begin]) goto try;
		do begin++; while (begin < length and input[begin] < 33);
		if (begin > best) { best = begin; candidate = index; } 
	}

	if (index == 899) context[program[arguments[arg]] + 2 + context[program[arguments[arg]] + 1]] = 0;
	if (index == 891) {
		if (index_count >= index_limit) { reason = "index limit exceeded"; goto error; }
		int name_length = program[arguments[arg] + 1] - 1, place = index_count;
		if (context_count + name_length + 3 >= context_limit) { reason = "context limit exceeded"; goto error; }
		while (place and name_length < context[indicies[place - 1] + 1]) place--;
		memmove(indicies + place + 1, indicies + place, sizeof(int) * (size_t) (index_count - place));
		indicies[place] = context_count; index_count++;
		for (int i = 0; i <= top; i += 8) 
			if (place <= stack[i]) stack[i]++;
		context[context_count++] = arguments[arg + 1];
		context[context_count++] = name_length;
		for (int i = 0; i <= name_length; i++) {
			int ind = program[program[arguments[arg] + i + 2]];
			context[context_count++] = ind < alphabet ? context[ind + 2] : ind;
		}
	}

	if (program[context[index]] >= alphabet) {
		index = program[context[index]];
		count = program[context[index] + 1];
		memcpy(arguments + arg, program + context[index] + 2, sizeof(int) * (size_t) count);
	}

	if (program_count + 2 + count > program_limit) { reason = "program limit exceeded"; goto error; } 
	program[program_count] = index;
	program[program_count + 1] = count;
	memcpy(program + program_count + 2, arguments + arg, sizeof(int) * (size_t) count);

	if (top) {
		int save = count;
		done = stack[top + 1];
		index = stack[top + 4];
		count = stack[top + 5];
		arg = stack[top + 6];
		if (arg + count >= argument_limit) { reason = "argument limit exceeded"; goto error; } 
		arguments[arg + count] = program_count;
		count++;
		top -= 8;
		program_count += 2 + save;
		goto parent;
	}
	if (begin != length) goto try;
	printf("\n\tcompile successful.\n\n");

	printf("generating code...\n");

	
	goto final;
error:;
	int at = 0, line = 1, column = 1;
	while (at < best) {
		if (input[at++] == '\n') { line++; column = 1; } else column++;
	}
	fprintf(stderr, "\033[1m%s:%u:%u: \033[1;31merror:\033[m \033[1m%s\033[m\n", 
			filename, line, column, reason);

	int b = line > 2 ? line - 2 : 0, e = line + 2;
	for (int i = 0, l = 1, c = 1; i < length + 1; i++) {
		if (c == 1 and l >= b and l <= e) 
			printf("\n\033[90m%5d\033[0m\033[32m │ \033[0m", l);
		if ((i == length or input[i] != '\n') and l >= b and l <= e) {
			if (l == line and c == column) printf("\033[1;31m");
			if (i < length) printf("%c", input[i]);
			else if (l == line and c == column) printf("<EOF>");
			if (l == line and c == column) printf("\033[m");
		}
		if (i < length and input[i] == '\n') { l++; c = 1; } else c++;
	}
	printf("\n\n  did you mean:   ");
	int* n = context + candidate;
	for (int j = 0; j <= n[1]; j++) {
		int c = n[j + 2];
		if (c < 33) printf(" char{%d} ", c);
		else if (c < 128) printf("%c ", c);
		else if (c < 256) printf(" unicode{%d} ", c);
		else printf(" (%d) ", c);
	}
	puts("\n");
	
final:
	print_context(alphabet, index_count, context_count, context, indicies);
	print_program(program, context, program_count, 0);
	munmap(input, (size_t) length);
	free(context);
	free(program);
	free(arguments);
	free(stack);
	free(indicies);
}


/*
	todo list:
	
	x	0. get ucsr working with densely packed arrays. 


	x	1. make the context!! its just an array of ints.   
  			   only put in [248 + intrinsic] signatures.

	x	2. make context printer?

	x	3. make context loader!!
	x		3.1. extract out a open file function? we just need it. use void pointers. 

	x	x 4. make the declare intrinsic!     (rename to def). even though its takes one arg.
	x	5.  test it
	

	x	8. get macros with arguments working!!	
	x	9. test it

	x	6. get the other two intrinsic wworking:    attach,  and undef.
	x	7. test those 

		10. get code generation working for simple intructions!!

		11. improve the code generator over time. 


		      -->      i think thats it, then we have a compiler!... lol












// const char* file_head = 
	// "	.section	__TEXT,__text,regular,pure_instructions\n"
	// "	.build_version macos, 11, 0	sdk_version 11, 1\n"
	// "	.globl	_main\n"
	// "	.p2align	4, 0x90\n"
	// "_main:\n";
	// const char* file_tail = 
	// "	mov $5, %rax\n"
	// "	retq\n"
	// "\n";

	// int fd = open("out.s", O_WRONLY | O_CREAT | O_TRUNC);
	// if (fd < 0) {
	// 	printf("error: %s: ", "filename");
	// 	perror("open");
	// 	exit(1);
	// }

	// write(fd, file_head, strlen(file_head));
	// i16 stack_count = 0;
	// stack[stack_count++].ind = (i16) program_count - 1;
	// while (stack_count) {
	// 	i16 e = stack[--stack_count].ind;
	// 	index = program[S * e];
	// 	// printf("stack_count=%d | (expr=%d) : looking at %d (%.*s) (count=%d)\n", 
	// 	// 	stack_count, e, index, context[S * index], context + S * index + 1, program[S * e + 1]);
	// 	for (i16 i = program[S * e + 1]; i--;) stack[stack_count++].ind = program[S * e + 2 + i];
	// }

	// write(fd, file_tail, strlen(file_tail));
	// close(fd);
























		if (not stack_count) abort();
		
		int TOS = stack[--stack_count + top];
		int definition_index = program[TOS];
		int definition_count = program[TOS + 1];

		for (int i = 0; i < definition_count; i++) 
			stack[top + stack_count++] = program[TOS + 2 + i];
			stack[top + stack_count++] = program[TOS + 2 + i];
			goto try;
		}

		printf("MACRO: DEBUG: TOS = %d, def index = %d, def count = %d\n", TOS, definition_index, definition_count);
		printf("MACRO: DEBUG: LOOKING AT: ");
		print(program + TOS + 2, definition_count);

		if (stack_count) {
			stack_count--;
			goto parent;
		}

		abort();
		
		top -= 8;




// 	top += 8;
	// 	int call_index = index;
	// 	int call_argument_count = count;
	// 	int* call_arguments = arguments + arg;
	// 	int stack_count = 0;
	// 	stack[top + stack_count++] = context[index];

	// 	macro_loop:;
	// 	int TOS = stack[--stack_count + top];
	// 	int definition_index = program[TOS];
	// 	int definition_count = program[TOS + 1];
	// 	printf("MACRO: DEBUG: TOS = %d, def index = %d, def count = %d\n", TOS, definition_index, definition_count);
	// 	printf("MACRO: DEBUG: LOOKING AT: ");
	// 	print(program + TOS + 2, definition_count);

	// 	if (program[context[definition_index]] == 'P') {
	// 		printf(" ----> THIS IS TRUE!!!\n");
	// 	}

	// 	for (int i = 0; i < definition_count; i++) stack[top + stack_count++] = program[TOS + 2 + i];
	// 	if (stack_count) goto macro_loop;	
	// 	top -= 8;
	// }






*/


// static inline void print(int* vector, int length) {
// 	printf("(%d){ ", length);
// 	for (int i = 0; i < length; i++) 
// 		printf("%d%c ", vector[i], vector[i] > 32 and vector[i] < 128 ? vector[i] : 0);
// 	printf("}\n");
// }



  // ___pagezerostart:
  //       dd 0x19         ; LC_SEGMENT_64
  //       dd ___pagezeroend - ___pagezerostart    ; command size
  //       db '__PAGEZERO',0,0,0,0,0,0 ; segment name (pad to 16 bytes)
  //       dq 0            ; VM address
  //       dq 0x100000000  ; VM size
  //       dq 0            ; file offset
  //       dq 0            ; file size
  //       dd 0x0          ; VM_PROT_NONE (maximum protection)
  //       dd 0x0          ; VM_PROT_NONE (inital protection)
  //       dd 0            ; number of sections
  //       dd 0x0          ; flags
  //       align 8, db 0   ; pad with zero to 8-byte boundary
  //   ___pagezeroend:


// __mh_execute_header:
//         dd 0xfeedfacf   ; MH_MAGIC_64
//         dd 16777223     ; CPU_TYPE_X86 | CPU_ARCH_ABI64
//         dd 0x80000003   ; CPU_SUBTYPE_I386_ALL | CPU_SUBTYPE_LIB64
//         dd 2            ; MH_EXECUTE
//         dd 16           ; number of load commands
//         dd ___loadcmdsend - ___loadcmdsstart    ; size of load commands
//         dd 0x00200085   ; MH_NOUNDEFS | MH_DYLDLINK | MH_TWOLEVEL | MH_PIE
//         dd 0            ; reserved
//     ___loadcmdsstart:%      









// void dumphex(uint8_t* bytes, size_t byte_count) {
// 	for (int i = 0; i < byte_count; i++) {
// 		if (!(i % 8)) printf("\n");
// 		printf("%02x ", bytes[i]);
// 	}
// 	printf("\n");
// }

// int main() {

// 	const int number_of_commands = 1; // temp.
// 	const int size_of_commands = 0xffff; // temp.
// 	const int number_of_sections = 0; // 1 ? 

// 	struct mach_header_64 h = {0};	
// 	h.magic = MH_MAGIC_64;
// 	h.cputype = CPU_TYPE_X86 | CPU_ARCH_ABI64;
// 	h.cpusubtype = CPU_SUBTYPE_I386_ALL | CPU_SUBTYPE_LIB64;
// 	h.filetype = MH_OBJECT;
// 	h.ncmds = number_of_commands;
// 	h.sizeofcmds = size_of_commands;
// 	h.flags = MH_NOUNDEFS | MH_SUBSECTIONS_VIA_SYMBOLS;
	
// 	struct segment_command_64 c = {0};
// 	c.cmd = LC_SEGMENT_64;

// 	c.cmdsize = sizeof(struct segment_command_64) + sizeof(struct section_64) * number_of_sections;
// 	strncpy(c.segname, "__TEXT", 16);
// 	c.vmsize = 0x100000000;
// 	c.vmaddr = 0;
// 	c.fileoff = 0;
// 	c.nsects = number_of_sections;

// 	struct section_64 s = {0};

// 	strncpy(s.sectname, "__text", 16);
// 	strncpy(s.segname, "__TEXT", 16);
	
// 	dumphex((void*) &h, sizeof(h));
// 	dumphex((void*) &c, sizeof(c));
	
// }


