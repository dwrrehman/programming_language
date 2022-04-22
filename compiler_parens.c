#include <stdio.h>    // a compiler written in c, for my programming language 
#include <iso646.h>          // made by daniel warren riaz rehman.
#include <stdlib.h>		// phase 2 began on 2104106.112610
#include <string.h>             // syntax changed to "()" on 2112186.1604
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdint.h>


/*

-----------------------------------------------------------------------------------------------------

	todo:

		- figure out how to include the macho header  the right way...


		- understand the following load_commands:

			LC_SEGMENT_64 (__PAGEZERO)

			LC_SEGMENT_64 (__TEXT)     (contains __text, __const,  __cstring, 

			LC_SEGMENT_64 (__DATA)     (contains __data, __bss,  __cstring, 

			LC_SEGMENT_64 (__LINKEDIT)

			LC_DYLD_INFO_ONLY

			LC_SYMTAB

			LC_DYSYMTAB

			LC_LOAD_DYLINKER

		x	LC_UUID

		x	LC_VERSION_MIN_MACOS

			LC_UNIXTHREAD

			
			

		

-----------------------------------------------------------------------------------------------------

	notes:


		- in order to save debug information, (when we get to that) in the AST,  we would kinda have to completely redo the datastructure for the btucsr parser. 


		- i want to be able to generate an executable. thats the goal. 

		- 






		- i will only be using the parts of libc which are used to make syscalls? 

			or actually, why dont i just not use ANY of LibC at all!?


				theres literally no reason to, actually. cool. yay. 



		- also, i think that "link time optimization is just not relevant to be now.. i mean, i literally am going to be making sure that the whole program is there, and able to be parsed and analyzed...so like theres no reason to use LTO. 

		- 








*/





#include "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include/mach-o/loader.h"

// to disassemble:      objdump --disassemble-all --full-contents  object.o

//                      objdump -m --disassemble-all --syms --section-headers --full-contents test_asm/main.o


// for learning:    objdump --disassemble-all out                with release mode.

// using otool on mac is much prefered, way to look at the binary.





typedef uint8_t uc;

static inline void print_vector(int* v, int l) {
	printf("{ ");
	for (int i = 0; i < l; i++) {
		printf("%d ", v[i]);
	}
	printf("}\n");
}

static void print_output(int* output, int top, int index) {
	puts("\n------- output: -------");
	for (int i = 0; i < top + 4; i += 4) {
		printf("%c%c %10d :   %10di %10dp %10db %10dd \n", 
			i != top ? ' ' : '>',
			i != index ? ' ' : '@', i, 
			output[i], output[i + 1], output[i + 2], output[i + 3]);
	}
	puts("---------------------\n");
}

static void print_index(const char* m, const char* string, int length, int index) {
	printf("\n%s\t\t", m);
	for (int i = 0; i < length; i++) {
		char c = string[i];
		if (i == index) printf("\033[1;31m[%c]\033[m", c);
		else printf("%c", c);
	}
	if (index == length) printf("\033[1;31m[T]\033[m"); 
	else printf("T"); 
	printf("\n");
}

static void debug(const char* m, const char* input, int* output, 
		  int length, int begin, int top, int index, int done) {
	return;	
	printf("\n\n\n\n\n-------------%s---------------:\n",m);

	printf("\n<<<variables:>>>\n\t "
		"length = %d\n\t "
		"begin = %d\n\t "
		"top = %d\n\t "
		"index = %d\n\t "
		"done = %d\n\n", 
		length, begin, top, index, done);

	print_output(output, top, index);
	print_index("\n\n<<<begin:>>>\n\n", input, length, begin);
	print_index("\n\n<<<done:>>>\n\n", input, length, done);
	getchar();
}




int main(const int argc, const char** argv) {
	if (argc != 2) return printf("usage: ./compiler <input>\n");
	const int limit = 8192, args_limit = 64;
	int* output = malloc(limit * sizeof(int));
	memset(output, 0x0F, limit * sizeof(int));
	const char* err = NULL;
	int index = 0, top = 0, begin = 0, done = 0, var = 0, length = 0, where = 0, best = 0;
	struct stat file_data = {0};
	int file = open(argv[1], O_RDONLY);
	if (file < 0 or stat(argv[1], &file_data) < 0) { perror("open"); exit(3); }
	length = (int) file_data.st_size;
	char* input = not length ? 0 : mmap(0, (size_t) length, PROT_READ, MAP_SHARED, file, 0);
	if (input == MAP_FAILED) { perror("mmap"); exit(4); }
	close(file);
	if (not length) goto error;
	
iread_type_in_name: 
	debug("iread_type_in_name", input, output, length, begin, top, index, done);
	if (input[begin] == '(') goto _i15;
	begin++;
	if (begin >= length) {err = "expected ("; goto error;}
	goto iread_type_in_name;
_i15:	debug("_i15", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) {err = "eof after ( in sig"; goto error;}
	if ((uc)input[begin] < 33) goto _i15;
iloop_name: 
	debug("iloop_name", input, output, length, begin, top, index, done);
	if (input[begin] == ')') goto iend_name; 
	if (input[begin] != '(') goto iread_reg; 
_i16:	debug("_i16", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) {err = "expected argument in sig"; goto error;}
	if (input[begin] != ')') goto _i16;
_i17:	debug("_i17", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) {err = "expected ) in arg"; goto error;}
	if ((uc)input[begin] < 33) goto _i17;
	goto iloop_name;
iread_reg: 
	debug("iread_reg", input, output, length, begin, top, index, done);
	if (input[begin] != '\\') goto _i19;
_i18: 	debug("_i18", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) {err = "expected )"; goto error;}
	if ((uc)input[begin] < 33) goto _i18;
_i19: 	debug("_i19", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) {err = "expected char in sig"; goto error;}
	if ((uc)input[begin] < 33) goto _i19;
	goto iloop_name;
iend_name:
	debug("iend_name", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto push_initial;
	if ((uc)input[begin] < 33) goto iend_name;

push_initial: 
	if (top + 7 >= limit) goto error;
	output[top] = limit;
	output[top + 2] = 0;
	output[top + 3] = 0;
	output[top + 5] = 0;
	output[top + 6] = begin;
	top += 4;
	best = begin;

begin:	debug("begin", input, output, length, begin, top, index, done);
	var = output[top + 1];
	if (not var) goto type_check; 
	var = output[var + 3];
_5: 	var++;
	if ((uc)input[var] < 33) goto _5;
	if (input[var] == ')') goto read_name;
type_check: 
	debug("type_check", input, output, length, begin, top, index, done);
	if (input[done] != '(') goto compare_types;
	if (input[var] == ')') goto types_match;
	if (input[var] == '(') goto types_match;
compare_types: 
	debug("compare_types", input, output, length, begin, top, index, done);
	if (input[done] != input[var]) goto next;
_6:	var++;
	if (var >= length) goto next;
	if ((uc)input[var] < 33) goto _6;
_7: 	done++; 
	if (done >= length) goto next;
	if ((uc)input[done] < 33) goto _7;
	goto type_check;
types_match: 
	debug("types_match", input, output, length, begin, top, index, done);
	done++; 
	if ((uc)input[done] < 33) goto types_match;
	begin = output[top + 2];
check_character: if (input[done] == ')') goto publish; 
	if (input[done] != '(') goto match; 
	if (top + 7 >= limit) goto error;
	output[top] = index;
	output[top + 3] = done;
	output[top + 5] = top;
	output[top + 6] = begin;
	top += 4;
	index = 0;
	done = 0;
	goto begin;
match:	debug("match", input, output, length, begin, top, index, done);
	if (input[done] != '\\') goto _12;
_11: 	done++;
	if ((uc)input[done] < 33) goto _11;
_12:	if (begin >= length) goto backtrack;
	if (input[done] != input[begin]) goto backtrack;
_13: 	begin++;
	if (begin >= length) goto _14;
	if ((uc)input[begin] < 33) goto _13;
_14: 	done++;
	if ((uc)input[done] < 33) goto _14;
	if (begin <= best) goto skip_update_best; 
	best = begin;
	where = done;
skip_update_best: goto check_character;
read_name: 
	debug("read_name", input, output, length, begin, top, index, done);
	index = limit; 
read_type_in_name: 
	debug("read_type_in_name", input, output, length, begin, top, index, done);
	if (input[begin] == '(') goto _15;
	begin++;
	if (begin >= length) goto next;
	goto read_type_in_name;
_15:	debug("_15", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto next;
	if ((uc)input[begin] < 33) goto _15;
loop_name: 
	debug("loop_name", input, output, length, begin, top, index, done);
	if (input[begin] == ')') goto end_name; 
	if (input[begin] != '(') goto read_reg; 
_16:	debug("_16", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto next;
	if (input[begin] != ')') goto _16;
_17:	debug("_17", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto next;
	if ((uc)input[begin] < 33) goto _17;
	goto loop_name;
read_reg: debug("read_reg", input, output, length, begin, top, index, done);
	if (input[begin] != '\\') goto _19;
_18: 	debug("_18", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto next;
	if ((uc)input[begin] < 33) goto _18;
_19: 	debug("_19", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto next;
	if ((uc)input[begin] < 33) goto _19;
	goto loop_name;
end_name: debug("end_name", input, output, length, begin, top, index, done);
	begin++;
	if (begin >= length) goto check_if_best;
	if ((uc)input[begin] < 33) goto end_name;
check_if_best: if (begin <= best) goto publish; 
	best = begin;
	where = done;
publish: 
	debug("publish", input, output, length, begin, top, index, done);
	output[top] = index;
	output[top + 3] = done;
	var = output[top + 1];
	if (not var) goto check_success;
	if (top + 7 >= limit) goto error;
	top += 4;
	output[top + 1] = output[var + 1];
	output[top + 2] = begin;
	index = output[var];
	done = output[var + 3];
_20: 	done++;
	if ((uc)input[done] < 33) goto _20;
_23:	if (input[done] == ')') goto _26;
_25: 	done++;
	if ((uc)input[done] < 33) goto _25;
	goto _23;
_26:	done++;
	if ((uc)input[done] < 33) goto _26;
	goto check_character;
check_success: if (begin == length) goto success;
backtrack: 
	debug("backtrack", input, output, length, begin, top, index, done);
	if (index == limit) goto pop;
	var = output[index + 2];
skip_type: 
	debug("skip_type", input, output, length, begin, top, index, done);
	if (input[var] == '(') goto find_first_arg;
	var++;
	goto skip_type;
find_first_arg: 
	debug("find_first_arg", input, output, length, begin, top, index, done);
	var++;
	if (input[var] == ')') goto check_if_first;
	if (input[var] == '(') goto check_if_first;
	if (var == done) goto next;
	goto find_first_arg;
check_if_first:
	debug("check_if_first", input, output, length, begin, top, index, done);
	if (var == done) goto next;
pop:	debug("pop", input, output, length, begin, top, index, done);
	if (not top) {err = "unresolved expression"; goto error;}
	top -= 4;
	index = output[top];
	done = output[top + 3];
	goto backtrack;
next:	debug("next", input, output, length, begin, top, index, done);
	index += 4;
	if (index >= top) goto pop;
	if (output[index] != limit) goto next;
	done = output[index + 2];
	goto begin;
success: top += 4;
	puts("success: compile successful."); 
	debug("success", input, output, length, begin, top, index, done);

	int this = 0, next = 0, count = 0;
	int* args = malloc(args_limit * sizeof(int));
	
code:	if (this >= top) goto out;
	if (output[this] == limit) {
		if ((1)) {
		printf("\n\n\n------------------------- %d ---------------------------\n", this);
		printf(" %10d : %10di %10dp %10db %10dd   : UDS :   ", 
			this, output[this + 0], output[this + 1], output[this + 2], output[this + 3]);
		int s = output[this + 2];
		int c = 0;
		do {
			putchar(input[s]);
			if (input[s] == '(') c++;
			if (input[s] == ')') c--;
			if (input[s] == ')' and !c) break;
			s++;
		} while (1);
		printf("\n");
		}
		goto move;
	}
	if (input[output[this + 3]] != ')') goto move;

	if ((1)) {
	printf("\n\n\n------------------------- %d ---------------------------\n", this);
	printf(" %10d : %10di %10dp %10db %10dd   :   ", 
		this, output[this + 0], output[this + 1], output[this + 2], output[this + 3]);
	int s = output[output[this] + 2];
	int c = 0;
	do {
		putchar(input[s]);
		if (input[s] == '(') c++;
		if (input[s] == ')') c--;
		if (input[s] == ')' and !c) break;
		s++;
	} while (1);
	printf("\n");
	}


	next = this;
	count = 0;
next_child:
	index = output[next];
	if (index == limit) goto first;
	done = output[next + 3];
	var = output[index + 2];
rskip_type: if (input[var] == '(') goto rfind_first_arg;
	var++;
	goto rskip_type;
rfind_first_arg: var++;
	if (input[var] == ')') goto rcheck_if_first;
	if (input[var] == '(') goto rcheck_if_first;
	 if (var == done) goto first;
	goto rfind_first_arg;
rcheck_if_first: if (var == done) goto first;
	args[count++] = next - 4;
	next = output[next - 3];
	goto next_child;
first:;
	print_vector(args, count);

move: 	this += 4;
	goto code;

out:	

	// printf("outputting executable...\n");

	// typedef uint32_t u32;

	// struct instruction {
	// 	// int sf; // what is this!?
	// 	int op; // operation
	// 	int rm, rn, rd;	 // source1, source2, and destination. 
	// };

	//struct instruction ins = {0};

	// u32* bytes = malloc(4);
	// size_t size = 4;
	// bytes[0] = 0x91006108;

	// const int number_of_sections = 1;

	// struct mach_header_64 header = {0};	
	// struct segment_command_64 command = {0};
	// struct section_64 section = {0};

	// header.magic = MH_MAGIC_64;
	// header.cputype = (int)CPU_TYPE_ARM | (int)CPU_ARCH_ABI64;
	// header.cpusubtype = (int)CPU_SUBTYPE_ARM_ALL | (int)CPU_SUBTYPE_LIB64;
	// header.filetype = MH_EXECUTE;
	// header.ncmds = 1;
	// header.sizeofcmds = 0;
	// header.flags = MH_NOUNDEFS | MH_SUBSECTIONS_VIA_SYMBOLS;
	



	// command.cmd = LC_SEGMENT_64;
	// command.cmdsize = sizeof(struct segment_command_64) + sizeof(struct section_64) * number_of_sections;

	// header.sizeofcmds += command.cmdsize;

	// strncpy(command.segname, "__TEXT", 16);
	// command.vmsize = sizeof header + sizeof command + sizeof section * number_of_sections + size;
	// command.vmaddr = 0;
	// command.fileoff = 0;
	// command.filesize = sizeof header + sizeof command + sizeof section * number_of_sections + size;
	// command.maxprot = VM_PROT_ALL;
	// command.initprot = VM_PROT_ALL;
	// command.nsects = number_of_sections;
	
	// strncpy(section.sectname, "__text", 16);
	// strncpy(section.segname, "__TEXT", 16);
	// section.addr = 0;
	// section.size = size;
	// section.offset = sizeof header + sizeof command + sizeof section * number_of_sections;
	// section.align = 3;
	// section.reloff = 0;
	// section.nreloc = 0;

	// printf("\ndebugging header bytes:\n------------------------\n");
	// dumphex((void*) &header, sizeof(header));

	// printf("\ndebugging command bytes:\n------------------------\n");
	// dumphex((void*) &command, sizeof(command));

	// printf("\ndebugging section bytes:\n------------------------\n");
	// dumphex((void*) &section, sizeof(section));

	// printf("\ndebugging bytes bytes:\n------------------------\n");
	// dumphex((void*) bytes, size);
	
	// printf("\n\n--> outputting %zd bytes to output file...\n\n", size);

	// int out_file = open("object.o", O_WRONLY | O_CREAT);
	// if (out_file < 0) { perror("open"); exit(4); }
	// write(out_file, &header, sizeof header);
	// write(out_file, &command, sizeof command);
	// write(out_file, &section, sizeof section);
	// write(out_file, bytes, size);
	// close(out_file);

	
	// system("/usr/bin/ld -demangle -lto_library /opt/homebrew/Cellar/llvm/13.0.0_1/lib/libLTO.dylib "
	// 	"-dynamic -arch arm64 -platform_version macos 12.0.0 12.0.0 "
	// 	"-syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX12.sdk "
	// 	"-o a.out object.o "
	// 	"-lSystem /opt/homebrew/Cellar/llvm/13.0.0_1/lib/clang/13.0.0/lib/darwin/libclang_rt.osx.a");



	goto clean_up;
error:; 
	int at = 0, line = 0, column = 0, wat = 0, wline = 0, wcolumn = 0;
	while (at < best and at < length) {
		if (input[at++] != 10) { column++; } 
		else { line++; column = 0; }
	}
	while (wat < where and wat < length) {
		if (input[wat++] != 10) { wcolumn++; } 
		else { wline++; wcolumn = 0; }
	}
	fprintf(stderr, "%u %u %u  %u %u %u  %u %u  %s\n", 
			line, column, at, wline, wcolumn, wat, top, limit, err);
clean_up:
	munmap(input, (size_t) length);
	free(output);
}




// enum {
// 	rax_r = 1 << 0, 
// 	rcx_r = 1 << 1,
// 	rdx_r = 1 << 2,
// 	rbx_r = 1 << 3,

// 	rsp_r = 1 << 4,
// 	rbp_r = 1 << 5,
// 	rsi_r = 1 << 6,
// 	rdi_r = 1 << 7,

// 	r8_r = 1 << 8,
// 	r9_r = 1 << 9,
// 	r10_r = 1 << 10,
// 	r11_r = 1 << 11,

// 	r12_r = 1 << 12,
// 	r13_r = 1 << 13,
// 	r14_r = 1 << 14,
// 	r15_r = 1 << 15,
// };


// typedef uint32_t u32;
// typedef uint64_t u64;



//NOTE : left shift "<<" is a multiply by 2, and a right shift, ">>" is a divide by two.




	// if (register1 != out_register) {
			// 	emit_rex(out_register, register1, 0);
			// 	emit_mov_register();
			// 	emit_direct(out_register, register1);
				
			// 	printf("NOTE: generated intermetiary MOV instruction.\n");
			// 	usleep(1000000);
			// }


// int arg1 = args[count - 1];
		// int arg2 = args[count - 2];
		// int index1 = output[args[count - 1] + 0];
		// int index2 = output[args[count - 2] + 0];


		
		// int arg1 = args[count - 1];
		// int arg2 = args[count - 2];
		// int index1 = output[args[count - 1] + 0];
		// int index2 = output[args[count - 2] + 0];


		// } else {

		// 	output[this + 2] = (int)register1;

		// 	emit_rex(register1, register2, 0);
		// 	emit_add_register();
		// 	emit_direct(register1, register2);

		// }



			// // scratch_free((int)register1, state);
			// uc out_register = register1;// (uc) scratch_alloc(state);

			// printf("\n---> allocated result at: out=%d     "
			// 	"  (inputs were r1(d)=%d, r2(s)=%d)\n\n",
			// 		out_register, register1, register2);






















  /* nop */
//   static const char nop_1[] = { 0x90 };

  /* xchg %ax,%ax */
  // static const char nop_2[] = { 0x66, 0x90 };

  /* nopl (%[re]ax) */
  // static const char nop_3[] = { 0x0f, 0x1f, 0x00 };

  /* nopl 0(%[re]ax) */
  // static const char nop_4[] = { 0x0f, 0x1f, 0x40, 0x00 };

  /* nopl 0(%[re]ax,%[re]ax,1) */
  // static const char nop_5[] = { 0x0f, 0x1f, 0x44, 0x00, 0x00 };

  /* nopw 0(%[re]ax,%[re]ax,1) */
  // static const char nop_6[] = { 0x66, 0x0f, 0x1f, 0x44, 0x00, 0x00 };

  /* nopl 0L(%[re]ax) */
  // static const char nop_7[] = { 0x0f, 0x1f, 0x80, 0x00, 0x00, 0x00, 0x00 };

  /* nopl 0L(%[re]ax,%[re]ax,1) */
  // static const char nop_8[] = { 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00};

  /* nopw 0L(%[re]ax,%[re]ax,1) */
  // static const char nop_9[] = { 0x66, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };

  /* nopw %cs:0L(%[re]ax,%[re]ax,1) */
  // static const char nop_10[] = { 0x66, 0x2e, 0x0f, 0x1f, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00 };








// emit_rex(r11, r12, 0);
	// emit_add_register();
	// emit_direct(r11, r12);

	// emit_rex(r8, r9, 0);
	// emit_add_register();
	// emit_indirect(r8, r9);

	// emit_rex(r8, r9, 0);
	// emit_add_register();
	// emit_indirect_disp8(r8, r9, 0x12);

	// emit_rex(r8, r9, 0);
	// emit_add_register();
	// emit_indirect_disp32(r8, r9, 0x12345678);


	// emit_rex(rax, 0, 0);
	// emit_add_register();
	// emit_indirect_rip_relative(rax, 0x12345678);

	// emit_rex(rax, 0, 0);
	// emit_add_register();
	// emit_indirect_pure_displacement(rax, 0x12345678);	

	// emit_rex(rbx, rcx, rax);
	// emit_add_register();
	// emit_indexed_indirect(rbx, rcx, rax, scale_4);

	// emit_rex(rbx, rcx, rax);
	// emit_add_register();
	// emit_indexed_indirect_disp8(rbx, rcx, rax, scale_4, 0x12);

	// emit_rex(rbx, rcx, rax);
	// emit_add_register();
	// emit_indexed_indirect_disp32(rbx, rcx, rax, scale_4, 0x12345678);

	// emit_nop1();
	// emit_nop2();
	// emit_nop3();
	// emit_nop4();
	// emit_nop5();
	// emit_nop6();
	// emit_nop7();
	// emit_nop8();
	// emit_nop9();
	// emit_nop10();

	// emit_rex(rbx, rcx, rax);
	// emit_add_register();
	// emit_indexed_indirect_disp32(rbx, rcx, rax, scale_4, 0x12345678);







	// align8();

	// const int number_of_sections = 1;

	// struct mach_header_64 header = {0};	
	// struct segment_command_64 command = {0};
	// struct section_64 section = {0};

	// header.magic = MH_MAGIC_64;
	// header.cputype = (int)CPU_TYPE_X86 | (int)CPU_ARCH_ABI64;
	// header.cpusubtype = (int)CPU_SUBTYPE_I386_ALL | (int)CPU_SUBTYPE_LIB64;
	// header.filetype = MH_OBJECT;
	// header.ncmds = 1;
	// header.sizeofcmds = 0;
	// header.flags = MH_NOUNDEFS | MH_SUBSECTIONS_VIA_SYMBOLS;
	
	// command.cmd = LC_SEGMENT_64;
	// command.cmdsize = sizeof(struct segment_command_64) + sizeof(struct section_64) * number_of_sections;

	// header.sizeofcmds += command.cmdsize;

	// strncpy(command.segname, "__TEXT", 16);
	// command.vmsize = sizeof header + sizeof command + sizeof section * number_of_sections + size;
	// command.vmaddr = 0;
	// command.fileoff = 0;
	// command.filesize = sizeof header + sizeof command + sizeof section * number_of_sections + size;
	// command.maxprot = VM_PROT_ALL;
	// command.initprot = VM_PROT_ALL;
	// command.nsects = number_of_sections;
	
	// strncpy(section.sectname, "__text", 16);
	// strncpy(section.segname, "__TEXT", 16);
	// section.addr = 0;
	// section.size = size;
	// section.offset = sizeof header + sizeof command + sizeof section * number_of_sections;
	// section.align = 3;
	// section.reloff = 0;
	// section.nreloc = 0;

	// printf("\ndebugging header bytes:\n------------------------\n");
	// dumphex((void*) &header, sizeof(header));

	// printf("\ndebugging command bytes:\n------------------------\n");
	// dumphex((void*) &command, sizeof(command));

	// printf("\ndebugging section bytes:\n------------------------\n");
	// dumphex((void*) &section, sizeof(section));

	// printf("\ndebugging bytes bytes:\n------------------------\n");
	// dumphex((void*) bytes, size);
	
	// printf("\n\n--> outputting %zd bytes to output file...\n\n", size);

	// int out_file = open("object.o", O_WRONLY | O_CREAT);
	// if (out_file < 0) { perror("open"); exit(4); }

	// write(out_file, &header, sizeof header);
	// write(out_file, &command, sizeof command);
	// write(out_file, &section, sizeof section);
	// write(out_file, bytes, size);

	// close(out_file);

	// printf("DEBUG: ctr:\n{\n");
	// for (int i = 0; i < ctr_limit; i++) {
	// 	if (ctr[i] != 0x0F0F0F0F0F0F0F0F) 
	// 		printf("\tr%d = %zu\n", i, ctr[i]);
	// }
	// printf("}\n");

	// printf("DEBUG: memory:\n{\n");
	// for (int i = 0; i < ctm_limit; i++) {
	// 	if (memory[i] != 0x0F0F0F0F0F0F0F0F) 
	// 		printf("\t[%d] = %zu\n", i, memory[i]);
	// }
	// printf("}\n");



































	// printf("\n    (index=%d) : parsed %d arguments : ", index, count);
	// print_vector(args, count);
	// printf("\n");

	// int start = output[output[this] + 2];

	// if (skip) {
	// 	if (is("unit:at:label::unit:;", input, start) and output[args[count - 1]] == skip) {
	// 		output[output[args[count - 1]] + 3] = args[count - 2];
	// 		skip = 0;
	// 		this = args[count - 2];
	// 		goto code;
	// 	}
	// 	goto move;
	// }

	// if (is("unit:at:label::unit:;", input, start)) {
	// 	output[output[args[count - 1]] + 3] = args[count - 2];

	// } else if (is("unit:if:register:<:register:,:label:;", input, start)) {
	// 	int left = get(args[count - 1], input, output);
	// 	int right = get(args[count - 2], input, output);
	// 	if (ctr[left] < ctr[right]) goto branch;

	// } else if (is("unit:if:register:=:register:,:label:;", input, start)) {
	// 	int left = get(args[count - 1], input, output);
	// 	int right = get(args[count - 2], input, output);
	// 	if (ctr[left] == ctr[right]) {
	// 	branch:	if (output[output[args[count - 3]] + 3]) {
	// 			this = output[output[args[count - 3]] + 3];
	// 			goto code;
	// 		}
	// 		skip = output[args[count - 3]];
	// 	}

	// } else if (is("unit:increment:register:;", input, start)) 
	// 	ctr[get(args[count - 1], input, output)]++;
	// else if (is("unit:decrement:register:;", input, start)) 
	// 	ctr[get(args[count - 1], input, output)]--;
	// else if (is("unit:zero:register:;", input, start)) 
	// 	ctr[get(args[count - 1], input, output)] = 0;
	// else if (is("unit:copy:register:,:register:;", input, start)) 
	// 	ctr[get(args[count - 1], input, output)] = ctr[get(args[count - 2], input, output)];
	// else if (is("unit:add:register:,:register:;", input, start)) 
	// 	ctr[get(args[count - 1], input, output)] += ctr[get(args[count - 2], input, output)];
	// else if (is("unit:subtract:register:,:register:;", input, start)) 
	// 	ctr[get(args[count - 1], input, output)] -= ctr[get(args[count - 2], input, output)];
	// else if (is("unit:multiply:register:,:register:;", input, start)) 
	// 	ctr[get(args[count - 1], input, output)] *= ctr[get(args[count - 2], input, output)];
	// else if (is("unit:divide:register:,:register:;", input, start)) 
	// 	ctr[get(args[count - 1], input, output)] /= ctr[get(args[count - 2], input, output)];
	// else if (is("unit:modulo:register:,:register:;", input, start)) 
	// 	ctr[get(args[count - 1], input, output)] %= ctr[get(args[count - 2], input, output)];
	// else if (is("unit:xor:register:,:register:;", input, start)) 
	// 	ctr[get(args[count - 1], input, output)] ^= ctr[get(args[count - 2], input, output)];
	// else if (is("unit:and:register:,:register:;", input, start)) 
	// 	ctr[get(args[count - 1], input, output)] &= ctr[get(args[count - 2], input, output)];
	// else if (is("unit:or:register:,:register:;", input, start)) 
	// 	ctr[get(args[count - 1], input, output)] |= ctr[get(args[count - 2], input, output)];
	// else if (is("unit:store:register:,:register:;", input, start)) 
	// 	memory[ctr[get(args[count - 1], input, output)]] = ctr[get(args[count - 2], input, output)];
	// else if (is("unit:load:register:,:register:;", input, start)) 
	// 	ctr[get(args[count - 1], input, output)] = memory[ctr[get(args[count - 2], input, output)]];
	
	// else if (is("unit:nop;", input, start)) {
	// 	bytes[size++] = 0x90;

	// } else if (is("reg64:(:reg64:+:reg64:);", input, start)) {

	// 	uc dest = (uc)output[args[count - 1] + 2];
	// 	uc source = (uc)output[args[count - 2] + 2];
		
	// 	printf("BEFORE: state = ");
	// 	print_vector(state, register_count);

	// 	if (source != dest) scratch_free((int)source, state);
	// 	output[this + 2] = (int)dest;

	// 	emit_rex(dest, source, 0);
	// 	emit_add_register();
	// 	emit_direct(dest, source);

	// 	printf("AFTER: state = ");
	// 	print_vector(state, register_count);

	// } else if (is("unit:xor:reg64:,:reg64:;", input, start)) {

	// 	uc dest = (uc)output[args[count - 1] + 2];
	// 	uc source = (uc)output[args[count - 2] + 2];
		
	// 	printf("BEFORE: state = ");
	// 	print_vector(state, register_count);

	// 	if (source != dest) scratch_free((int)source, state);
	// 	output[this + 2] = (int)dest;

	// 	emit_rex(dest, source, 0);
	// 	emit_xor_register();
	// 	emit_direct(dest, source);

	// 	printf("AFTER: state = ");
	// 	print_vector(state, register_count);



	// } else if (is("unit:inc:reg64:;", input, start)) {

	// 	uc dest = (uc)output[args[count - 1] + 2];
		
	// 	printf("BEFORE: state = ");
	// 	print_vector(state, register_count);

	// 	output[this + 2] = (int)dest;

	// 	emit_rex(dest, 0, 0);
	// 	emit_inc_register();
	// 	emit_direct(dest, 0);

	// 	printf("AFTER: state = ");
	// 	print_vector(state, register_count);

	// } else if (is("unit:new::;", input, start)) {

	// 	printf("BEFORE: state = ");
	// 	print_vector(state, register_count);
	
	// 	uc r = (uc) scratch_alloc(state);
	// 	output[args[count - 1] + 3] = (int)r;

	// 	printf("AFTER: state = ");
	// 	print_vector(state, register_count);

	// } else if (is("unit:discard:reg64:;", input, start)) {

	// 	printf("BEFORE: state = ");
	// 	print_vector(state, register_count);

	// 	int r = output[args[count - 1] + 2]; 
	// 	printf("calling: scratch_free(%d)\n", r);
	// 	scratch_free(r, state);
	
	// 	printf("AFTER: state = ");
	// 	print_vector(state, register_count);

	// } else if (is_type("reg64:", input, start)) { // else if, index != limit:
	// 	output[this + 2] = output[index + 3];

	// 	printf("REG USE: state = ");
	// 	print_vector(state, register_count);
	// }





	// printf("\n\n\n------------------------- %d ---------------------------\n", this);
	// printf(" %10d : %10di %10dp %10db %10dd   :   ", 
	// 	this, output[this + 0], output[this + 1], output[this + 2], output[this + 3]);
	// int s = output[output[this] + 2];
	// while (input[s] != ';') {
	// 	putchar(input[s]);
	// 	s++;
	// }
	// printf("\n");







	// printf(" %10d : %10di %10dp %10db %10dd   : UDS :   ", 
		// 	this, output[this + 0], output[this + 1], output[this + 2], output[this + 3]);
		// int s = output[this + 2];
		// while (input[s] != ';') {
		// 	putchar(input[s]);
		// 	s++;
		// }
		// printf("\n");







/*





hi (begin () () (unit) end)

begin
	unit (x)
	unit (() is cool)
	int (hello) is cool
end






hhi (begin () () (unit) end)

begin
	unit ()
	unit (() is cool (unit) and (unit) )
	int (hello) is cool 	unit (conv (int)) is cool and
		
		and conv hello
end





*/





// printf("DEBUG: ctr:\n{\n");
	// for (int i = 0; i < ctr_limit; i++) {
	// 	if (ctr[i] != 0x0F0F0F0F0F0F0F0F) 
	// 		printf("\tr%d = %zu\n", i, ctr[i]);
	// }
	// printf("}\n");

	// printf("DEBUG: memory:\n{\n");
	// for (int i = 0; i < ctm_limit; i++) {
	// 	if (memory[i] != 0x0F0F0F0F0F0F0F0F) 
	// 		printf("\t[%d] = %zu\n", i, memory[i]);
	// }
	// printf("}\n");





// int out_file = open("object.o", O_WRONLY | O_CREAT);
	// if (not out_file) { perror("open"); exit(1); }
	// write(out_file, code, 4);
	// close(out_file);
