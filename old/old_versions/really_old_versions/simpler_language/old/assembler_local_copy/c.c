// a cross assembler written by dwrr on 1202405186.174952
// used my modal editor to write this whole document! yay. 

// old header: 
// started 202405094.232703: by dwrr
// simple risc-v like language, with minimal terse syntax.


/*
2405201.222347:

	currently the bug is that we are not processing the entire file, we are missing the last instruction that is said, which is a problem. 


		print out where we are in the file inside of our parsing algorithm!
*/

			
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <iso646.h>
#include <ctype.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdnoreturn.h>
#include <mach-o/nlist.h>
#include <mach-o/loader.h>

typedef uint64_t nat;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;

static bool enable_debug_output = false;

static const nat ct_array_count = 1 << 16;
static const nat ct_memory_count = 1 << 16;


drp, dup, ovr, thd, swp, rot,
def, ari, arz, arc, cte, sgn, 
imm, atr, add, sub, slt, and_, 
ior, eor, sll, srl, blt, bge, 
bne, beq, ldb, ldh, ldw, ldd, 
stb, sth, stw, std, mul, muh, 
div, rem, jlr, apc, ecl,



enum language_isa {
	null_instruction, 
	drop, dup_, over, third, swap, rot,
	def, mi, mz, carg, ct, atr, 
	add, sub, addi, slt, slti, slts, 
	sltis, and_, andi, or_, ori, xor_, 
	xori, sll, slli, srl, srli, sra,
	srai, blt, blts, bge, bges, bne,
	beq, jal, jalr, auipc, ecall, lb,
	lbs, lh, lhs, lw, lws, ld, 
	sb, sh, sw, sd, mul, mh, 
	mhs, mhsu, div_, divs, rem, rems,
	isa_count
};

static const char* spelling[isa_count] = {
	"null_instruction", 
	"drp", "dup", "ovr", "thr", "swp", "rot",
	"def", "ari", "arz", "crg", "ctb", "atr", 
	"adr", "sbr", "adi", "slr", "sli", "slts", 
	"sltis", "and", "andi", "or", "ori", "xor", 
	"xori", "sll", "slli", "srl", "srli", "sra",
	"srai", "blt", "blts", "bge", "bges", "bne",
	"beq", "jal", "jalr", "auipc", "ecall", "lb",
	"lbs", "lh", "lhs", "lw", "lws", "ld", 
	"sb", "sh", "sw", "sd", "mul", "mhu", 
	"mhs", "mhsu", "divu", "divs", "remu", "rems",
};

struct instruction { 
	u32 a[4];
	nat start;
	nat is_signed;
	nat size;
}; 
struct file {
	nat start;
	nat count;
	const char* name;
};

static nat byte_count = 0;
static u8* bytes = NULL;

static nat ins_count = 0;
static struct instruction* ins = NULL;

static nat text_length = 0;
static char* text = NULL;

static nat file_count = 0;
static struct file files[4096] = {0};

static nat arg_count = 0;
static nat arguments[4096] = {0};

static nat name_count = 0;
static const char* names[4096] = {0};
static nat values[4096] = {0};
static nat lengths[4096] = {0};

static nat* array = NULL;

static void print_files(void) {
	printf("here are the current files used "
		"in the program: (%lld files) { \n", 
		file_count
	);
	for (nat i = 0; i < file_count; i++) {
		printf("\t file #%-8lld :   "
			"name = \"%-20s\", "
			".start = %-8lld, "
			".size = %-8lld\n",
			i, files[i].name, 
			files[i].start, 
			files[i].count
		);
	}
	puts("}");
}

static void print_stack(nat* stack_i, 
	nat* stack_f, 
	nat* stack_o, 
	nat stack_count
) {
	printf("current stack: (%lld entries) { \n", stack_count);
	for (nat i = 0; i < stack_count; i++) {
		printf("\t entry #%-8lld :   "
			"name = \"%-20s\", "
			"i = %-8lld, "
			"f = %-8lld, "
			"o = %-8lld / %lld\n", 
			i, files[stack_f[i]].name, 
			stack_i[i], stack_f[i], 
			stack_o[i], files[stack_f[i]].count
		);
	}
	puts("}");
}

static void print_arguments(void) {
	printf("\narguments[]: { \n");
	for (nat i = 0; i < arg_count; i++) {
		printf("\targuments[%llu] = { %llu  :"
			"  (.start=%llu,.count=%llu)} \n", 
			i, (nat) arguments[i], 
			0LLU, 0LLU
		);
	}
	puts("} \n");
}

static void print_dictionary(void) {
	puts("printing dictionary...");
	for (nat i = 0; i < name_count; i++) {
		printf("\t#%llu: name %s  length %llu  value %llu\n", i, 
			names[i], lengths[i], values[i]
		);
	}
	puts("done.");
}

static void print_instructions(void) {
	printf("instructions: {\n");
	for (nat i = 0; i < ins_count; i++) {
		printf("\t%llu\tins(.op=%u (\"%s\"), .size=%llu, args:{ ", 
			i, ins[i].a[0], spelling[ins[i].a[0]], ins[i].size
		);
		for (nat a = 0; a < 3; a++) printf("%llu ", (nat) ins[i].a[a + 1]);
		
		printf("} -- [found @ %llu]\n", ins[i].start);

	}
	puts("}");
}

static void print_registers(void) {
	printf("registers: {\n");
	for (nat i = 0; i < 32; i++) {
		if (i % 2 == 0) puts("");
		printf("\t%llu:\t%016llx\t", i, array[i]);
	}
	puts("\n}\n");
}

static void print_error(const char* reason, nat spot, nat spot2) {

	const int colors[] = {31, 32, 33, 34, 35};

	printf("\033[%dm", colors[1]);

	nat location = 0;
	const char* filename = NULL;
	nat stack_i[4096] = {0}, stack_f[4096] = {0}, stack_o[4096] = {0};
	nat stack_count = 0;
	for (nat index = 0; index < text_length; index++) {
		for (nat f = 0; f < file_count; f++) {
			const nat start = files[f].start;
			const nat count = files[f].count;
			if (index == start) {
				//printf("file %s begins at %llu!\n", 
				//files[f].name, index);
				stack_i[stack_count] = index;
				stack_f[stack_count] = f;
				stack_o[stack_count++] = 0;
				break;
			} 
			if (stack_o[stack_count - 1] == files[stack_f[stack_count - 1]].count)  {
				print_stack(stack_i, stack_f, stack_o, stack_count);
				printf("file %s reached the end the file! (stack_o[%llu] == count == %llu)\n", files[f].name, stack_count - 1, count);
				stack_count--;
				if (not stack_count) goto done; else break;
			}
		}
		if (index == spot) {
			printf("\033[38;5;255m(ERROR_HERE:%s:%llu)\033[0m", files[stack_f[stack_count - 1]].name, stack_o[stack_count - 1]);
			filename = files[stack_f[stack_count - 1]].name;
			location = stack_o[stack_count - 1];
			goto done;
		}
		//printf("\033[%dm", colors[stack_count - 1]);
		putchar(text[index]);
		//printf("\033[0m");
		stack_o[stack_count - 1]++;
		//printf("[%s]: incremented stack_o[top=%llu] to be now %llu...\n", files[stack_f[stack_count - 1]].name, stack_count - 1, stack_o[stack_count - 1]);
	}
	printf("\033[0m");
	
done:	
	print_files();
	print_stack(stack_i, stack_f, stack_o, stack_count);
	fprintf(stdout, "\033[1masm: %s:%lld:%lld:", 
		filename ? filename : "(top-level)", 
		location, spot2
	);
	fprintf(stdout, " \033[1;31merror:\033[m \033[1m%s\033[m\n", reason);
}

static char* read_file(const char* name, nat* out_length, nat here) {
	int d = open(name, O_RDONLY | O_DIRECTORY);
	if (d >= 0) { close(d); errno = EISDIR; goto read_error; }
	const int file = open(name, O_RDONLY, 0);
	if (file < 0) { 
		read_error:;
		char reason[4096] = {0};
		snprintf(reason, sizeof reason, "%s: \"%s\"", 
			strerror(errno), name);
		print_error(reason, here, 0);
		fprintf(stdout, "asm: \033[31;1merror:\033[0m %s: "
				"\"%s\"\n", strerror(errno), name); 
		exit(1); 
	}
	size_t length = (size_t) lseek(file, 0, SEEK_END);
	char* string = malloc(length);
	lseek(file, 0, SEEK_SET);
	read(file, string, length);
	close(file); 
	*out_length = length;
	return string;
}

static void push_name(const char* raw_string, const nat raw_length) {
	
	nat length = 0;
	char* string = calloc(raw_length + 1, 1);

	for (nat i = 0; i < raw_length; i++) {
		if ((unsigned char) raw_string[i] < 33) continue;
		string[length++] = raw_string[i];
	}

	nat spot = 0;
	for (; spot < name_count; spot++) 
		if (length >= lengths[spot]) break;

	memmove(lengths + spot + 1, lengths + spot, 
		sizeof(nat) * (name_count - spot));

	memmove(values + spot + 1, values + spot, 
		sizeof(nat) * (name_count - spot));

	memmove(names+ spot + 1, names + spot, 
		sizeof(const char*) * (name_count - spot));
	
	lengths[spot] = length;
	names[spot] = string;
	values[spot] = arg_count ? arguments[arg_count - 1] : 0;
	name_count++;
}

static void print_index_pair(nat start, nat end) {
	for (nat i = 0; i < text_length; i++) {
		if (i == start or i == end) printf("\033[32;1m");
		putchar(text[i]);
		if (i == start or i == end) printf("\033[0m");
	}
	puts("");
}



#define dd if (enable_debug_output)


int main(int argc, const char** argv) {

	if (argc != 2) exit(puts("asm: \033[31;1merror:\033[0m "
				"usage: ./asm <source.s>"));

	for (nat i = 0; i < isa_count; i++) 
		push_name(spelling[i], strlen(spelling[i]));
	
	const char* filename = argv[1];
	text_length = 0;
	text = read_file(filename, &text_length, 0);

	dd printf("read file: (length = %llu): \n<<<", text_length);
	dd fwrite(text, 1, text_length, stdout);
	dd puts(">>>");

	array = calloc(ct_array_count, sizeof(nat));
	array[2] = (nat) (void*) calloc(ct_memory_count, sizeof(nat));

	files[file_count].name = filename;
	files[file_count].count = text_length;
	files[file_count++].start = 0;

	arguments[arg_count++] = 0;

	nat	index = 0, at = 0, save = 0, name = 0,
		defining = 0, op = 0, max = 0, is_compiletime = 0, 
		skip = 0, start = 0, delimiter = 0;

begin:	if (name >= name_count) goto error;
	if (at and at == lengths[name]) goto found;
	if (index >= text_length) goto done;
	if ((unsigned char) text[index] < 33) goto nextc;
	if (names[name][at] != text[index]) goto fail;
	at++; goto nextc;
advance: save = index; name = 0; at = 0; goto begin;
fail: 	name++; index = save; at = 0; goto begin;
nextc:	index++; if (index > max) max = index; goto begin;
error:	
	if (defining and start) { 
		dd puts("skipping over undefined char becuase in defining state..."); 
		index++; goto advance; 
	}
	print_error("unresolved symbol", save, max);
	exit(1);
found:
	dd printf("debug: found name: %s\n", names[name]);
	if (defining) {
		dd puts("[in defining state]");
		if (not start) {
			start = index;
			delimiter = name;
			dd puts("AT OPENING DELIMITER");
		} else if (delimiter == name) {
			dd printf("note: name is between: %llu and %llu... (used delimiter %s)\n", start, save, names[delimiter]);
			dd print_index_pair(start, save);
			push_name(text + start, save - start);
			dd puts("AT CLOSING DELIMITER");
			defining = 0; start = 0;
		}
		goto advance;
	}

	for (nat i = 0; i < isa_count; i++) {
		if (not strcmp(spelling[i], names[name])) { 
			op = i; goto builtin;
		}
	}

	dd printf("info: found user-defined name:     "
		"calling \"%s\"!! \n", names[name]
	);

	arguments[arg_count++] = values[name];
	dd printf("----> pushed: user-defined value %llu onto stack...\n", 
		arguments[arg_count - 1]);

	goto advance;

builtin:;

	dd printf("debug: found builtin name: %s\n", spelling[op]);

	nat a0 = arg_count > 0 ? arguments[arg_count - 1] : 0;
	nat a1 = arg_count > 1 ? arguments[arg_count - 2] : 0;
	nat a2 = arg_count > 2 ? arguments[arg_count - 3] : 0;

	if ((false)) {}
	else if (op == def) { dd puts("executing def..."); defining = true; }
	else if (op == ct) { dd puts("executing ct..."); is_compiletime = true; }
	else if (op == mi) { dd puts("executing mi..."); arguments[arg_count - 1]++; }
	else if (op == mz) { dd puts("executing mz..."); arguments[arg_count - 1] = 0; }
	else if (op == drop) { dd puts("executing drop..."); arg_count--; }
	else if (op == carg) { dd puts("executing carg..."); arguments[arg_count - 1] = array[a0]; }
	else if (op == atr) {
		dd puts("executing atr...");
		array[a0] = is_compiletime ? index : ins_count;
		if (skip == a0) skip = 0;
		is_compiletime = false;

	} else if (op == dup_) {
		dd puts("executing dup...");
		arguments[arg_count++] = a0;

	} else if (op == over) {
		dd puts("executing over...");
		arguments[arg_count++] = a1;

	} else if (op == third) {
		dd puts("executing third...");
		arguments[arg_count++] = a2;

	} else if (op == swap) {
		dd puts("executing swap..."); 
		arguments[arg_count - 1] = a1;
		arguments[arg_count - 2] = a0;

	} else if (op == rot) {
		dd puts("executing rot...");
		// 1 2 3 -> 2 3 1
		arguments[arg_count - 1] = a2;
		arguments[arg_count - 2] = a0;
		arguments[arg_count - 3] = a1;

	} else if (not is_compiletime) {

		// TODO: we still need to 
		// edit the argstack after 
		// pushing the rt ins!!!!

		dd printf("info: pushing rt ins %llu(\"%s\")...\n", 
			op, spelling[op]
		);
		struct instruction new = {0};
		new.a[0] = (u32) op;
		new.start = index;
		for (nat i = 1; i < 4; i++) {
			new.a[i] = (u32) (arg_count > i - 1 
				? arguments[arg_count - 1 - (i - 1)] 
				: 0
				);
			dd printf(" ... argument #%llu : u32 = %u\n", 
				i, new.a[i]);
		}
		new.size = 4;
		ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
		ins[ins_count++] = new;

	} else {
		is_compiletime = false;
		nat e = op;
		
		dd printf("------> info: CT: EXECUTING: op = %llu (\"%s\"), "
			"args={a0:%llu, a1:%llu, a2:%llu}\n", 
			op, spelling[op], a0, a1, a2
		);

		     if (e == add)	{ array[a0] = array[a1] + array[a2]; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == addi)	{ array[a0] = array[a1] + a2; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == sub)	{ array[a0] = array[a1] - array[a2]; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == mul)	{ array[a0] = array[a1] * array[a2]; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == div_)	{ array[a0] = array[a1] / array[a2]; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == rem)	{ array[a0] = array[a1] % array[a2]; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == and_)	{ array[a0] = array[a1] & array[a2]; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == andi)	{ array[a0] = array[a1] & a2; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == or_)	{ array[a0] = array[a1] | array[a2]; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == ori)	{ array[a0] = array[a1] | a2; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == xor_)	{ array[a0] = array[a1] ^ array[a2]; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == xori)	{ array[a0] = array[a1] ^ a2; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == slt)	{ array[a0] = array[a1] < array[a2]; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == slti)	{ array[a0] = array[a1] < a2;arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == sll)	{ array[a0] = array[a1] << array[a2]; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == slli)	{ array[a0] = array[a1] << a2;arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == srl)	{ array[a0] = array[a1] >> array[a2]; arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == srli)	{ array[a0] = array[a1] >> a2;arg_count -= 2; arguments[arg_count - 1] = a0; }

		else if (e == lb)   { array[a0] = *( u8*)(array[a1] + a2);arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == lh)   { array[a0] = *(u16*)(array[a1] + a2);arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == lw)   { array[a0] = *(u32*)(array[a1] + a2);arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == ld)   { array[a0] = *(nat*)(array[a1] + a2);arg_count -= 2; arguments[arg_count - 1] = a0; }

		else if (e == sb)  { *( u8*)(array[a0] + a1) = ( u8)array[a2];arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == sh)  { *(u16*)(array[a0] + a1) = (u16)array[a2];arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == sw)  { *(u32*)(array[a0] + a1) = (u32)array[a2];arg_count -= 2; arguments[arg_count - 1] = a0; }
		else if (e == sd)  { *(nat*)(array[a0] + a1) = (nat)array[a2];arg_count -= 2; arguments[arg_count - 1] = a0; }

		else if (e == blt) { 
			arg_count -= 3; 
			if (array[a0]  < array[a1]) { 
				if (array[a2]) index = array[a2]; 
				else skip = a2; 
			} 
		} else if (e == bge)  { 
			arg_count -= 3; 
			if (array[a0] >= array[a1]) { 
				if (array[a2]) index = array[a2]; 
				else skip = a2; 
			} 
		} else if (e == beq)  { 
			arg_count -= 3; 
			if (array[a0] == array[a1]) { 
				if (array[a2]) index = array[a2]; 
				else skip = a2; 
			}
		} else if (e == bne)  { 
			arg_count -= 3; 
			if (array[a0] != array[a1]) { 
				if (array[a2]) index = array[a2]; 
				else skip = a2; 
			} 

		} else if (e == jal) {
			arg_count -= 2; 
			array[a0] = index;
			if (array[a1]) index = array[a1]; else skip = a1;

		} else if (e == jalr) {
			arg_count -= 3; 
			array[a0] = index;
			index = array[a1] + a2;
			is_compiletime = false;

		} else if (e == ecall) {
			dd puts("executing ecall at ct...");

			if (a0 == 1) { at = 0; goto done; }
			else if (a0 == 2) printf("debug: %lld (hex 0x%016llx)\n", array[a1], array[a1]);
			else if (a0 == 3) array[a1] = getchar();
			else if (a0 == 4) putchar(array[a1]);
			else if (a0 == 5) print_dictionary();
			else if (a0 == 6) print_instructions();
			else if (a0 == 7) print_registers();
			else if (a0 == 8) print_arguments();
			else {
				printf("error: ct: unknown ecall number %llu... aborting...\n", a0);
				abort();
			}

		} else { 
			printf("internal error: found unimplemented "
				"operation: %llu (\"%s\")...\n", 
				op, spelling[op]
			);
			printf("error: unknown ct ins = %llu\n", op); 
			print_error("unknown ct ins", index, 0); 
			abort(); 
		}
	}
	goto advance;

done:;  dd puts("DONE: finished assembling program.");

	if (defining) {
		printf("error: expected closing delimiter for name...\n");
		print_error("missing name closing delimiter", index, max);
		exit(1);
	}

	if (at) {
		printf("error: unexpected end of input\n");
		print_error("unexpected end of input\n", index, max);
		exit(1);
	}

	printf("SUCCESSFUL ASSEMBLING\n");
}








	/*if (index == text_length) {
		printf("\n\n\t\t\t\033[32mSUCCESS\033[0m\n\n");
	} else {
		printf("\n\n\t\t\t\033[31mFAILURE\033[0m\n\n");
	}*/

	// print_arguments();
	// print_dictionary();
	// print_instructions();




















/*
	if (defining) {
		puts("info: found a name while in the defining mode, treating as a delimiter for a new name...");

		defining = false;
		at = 0;
		nat end = 0;
		nat over_end = 0;
		for (nat i = index; i < text_length; i++) {
			if ((unsigned char) text[i] < 33) continue;
			if (not at) end = i;
			if (text[i] == names[name][at]) at++; else at = 0;
			if (at and at == lengths[name]) { over_end = i + 1; goto found_delimiter; }
		}
		puts("asm: error: no closing delimiter found for name...");
		abort();


	found_delimiter:
		
		puts("found a well formed name!!");
		printf("extends from text @ %llu to %llu...\n", index, end);
		
		char* string = NULL;
		nat length = 0;
		
		for (nat i = index; i < end; i++) {
			if ((unsigned char) text[i] < 33) continue;
			string = realloc(string, length + 1);
			string[length++] = text[i];
		}
		string = realloc(string, length + 1);
		string[length] = 0;
		
		printf("final name string: ");
		puts(string);
		printf("(which was length = %llu)\n", length);
		
		names[name_count] = string;
		lengths[name_count] = length;
		values[name_count++] = arguments[arg_count - 1];
		printf("assigning index to be %llu (was %llu)\n", over_end, index);
		index = over_end;				
		goto advance;
	}
	*/



































	/*
	else if (op == def_ins) {
		const nat nstart = index;
		const nat l = (nat) strlen(ins_spelling[end_ins]);
		for (; index < text_length; index++) {
			const nat length = text_length - index < l 
				? text_length - index : l;
			if (not strncmp(text + index, ins_spelling[end_ins], 
				length)) goto found_end;
		}
		printf("asm: fatal error: %llu: "
			"could not find closing name delimiter\n", 
			save
		);
		abort();
	found_end:;
		const nat nend = index;
		printf("user is defining the name \"");
		fwrite(text + nstart, 1, nend - nstart, stdout);
		printf("\"... will have the value %llu...\n", 
			arguments[arg_count - 1]);
		push_name(text + nstart, nend - nstart);
	}
*/




/*

insert ./build

c.c:331:12: error: use of undeclared identifier 'ct_ins'
        if (op == ct_ins) is_compiletime = true;
                  ^
c.c:332:17: error: use of undeclared identifier 'def_ins'
        else if (op == def_ins) { 
                       ^
fatal error: too many errors emitted, stopping now [-ferror-limit=]
3 errors generated.





c.c:331:12: error: use of undeclared identifier 'eof_ins'
        if (op == eof_ins) goto done;
                  ^
c.c:332:17: error: use of undeclared identifier 'ct_ins'
        else if (op == ct_ins) is_compiletime = true;
                       ^
fatal error: too many errors emitted, stopping now [-ferror-limit=]
3 errors generated.


















insert ./build
 
c.c:283:33: error: use of undeclared identifier 'ins_spelling'; did you mean 'spelling'?
                push_name(spelling[i], strlen(ins_spelling[i]));
                                              ^~~~~~~~~~~~
                                              spelling
c.c:57:20: note: 'spelling' declared here
static const char* spelling[isa_count] = {
                   ^
c.c:323:18: error: use of undeclared identifier 'ins_spelling'; did you mean 'spelling'?
                if (not strcmp(ins_spelling[i], names[op])) { op = i; goto builtin; }
                               ^~~~~~~~~~~~
                               spelling
c.c:57:20: note: 'spelling' declared here
static const char* spelling[isa_count] = {
                   ^
fatal error: too many errors emitted, stopping now [-ferror-limit=]
3 errors generated.






















*/






















/*
	nat i = find_name_in__ins_spelling();
	if (i == -1) {
		process "names[op]" as a regular call to a user-defined name!;
		goto advance;
	}








202405094.233253:
==========================
final ISA, and spelling: 
==========================

size_ins(…) sign_ins(±) rt_ins(•)   ct_ins(°)   attr_ins(:) dup_ins(¨)   swap_ins(;)
add_ins(+)  sub_ins(-)  mul_ins(*)  mulh_ins(∏) div_ins(/)  rem_ins(%)   slt_ins(~) 
xor_ins(^)  and_ins(&)  or_ins(|)   xori_ins(ˆ) andi_ins(ˇ) ori_ins(')   addi_ins(·) 
sll_ins(«)  srl_ins(»)  slli_ins(‹) srli_ins(›) ld_ins(`)   str_ins(,)   slti_ins(˜)
blt_ins(<)  bge_ins(≥)  bne_ins(≠)  beq_ins(=)  aipc_ins(@) ecall_ins(\) emit_ins(#)
begin_ins(˛) end_ins(¸)





		// walk forwards until you find names[name] in the text again, 
		// starting from AFTER this name instance. 










found:
	printf("debug: found name: %s\n", names[name]);
// 	goto advance;
// 	abort();

	if (defining) {
		puts("info: found a name while in the defining mode, treating as a delimiter for a new name...");

		defining = false;
		at = 0;
		nat end = 0;
		nat over_end = 0;
		for (nat i = index; i < text_length; i++) {
			if ((unsigned char) text[i] < 33) continue;
			if (not at) end = i;
			if (text[i] == names[name][at]) at++; else at = 0;
			if (at and at == lengths[name]) { over_end = i + 1; goto found_delimiter; }
		}
		puts("asm: error: no closing delimiter found for name...");
		abort();


	found_delimiter:
		
		puts("found a well formed name!!");
		printf("extends from text @ %llu to %llu...\n", index, end);
		
		char* string = NULL;
		nat length = 0;
		
		for (nat i = index; i < end; i++) {
			if ((unsigned char) text[i] < 33) continue;
			string = realloc(string, length + 1);
			string[length++] = text[i];
		}
		string = realloc(string, length + 1);
		string[length] = 0;
		
		printf("final name string: ");
		puts(string);
		printf("(which was length = %llu)\n", length);
		
		names[name_count] = string;
		lengths[name_count] = length;
		values[name_count++] = arguments[arg_count - 1];
		printf("assigning index to be %llu (was %llu)\n", over_end, index);
		index = over_end;				
		goto advance;
	}

	for (nat i = 0; i < isa_count; i++) {
		if (not strcmp(spelling[i], names[name])) { 
			op = i; goto builtin;
		}
	}

	printf("info: found user-defined name:     "
		"calling \"%s\"!! \n", names[name]
	);

	arguments[arg_count++] = values[name];
	printf("----> pushed: user-defined value %llu onto stack...\n", 
		arguments[arg_count - 1]);

	goto advance;


*/

