// started 202405094.232703: by dwrr
// simple risc-v like language, with minimal terse syntax.

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

static const nat ct_array_count = 1 << 16;
static const nat ct_memory_count = 1 << 16;

enum language_ISA {
	null_ins,
	size_ins,  sign_ins, on_ins,   off_ins,  push_ins, dup_ins,  swap_ins, 
	add_ins,   sub_ins,  mul_ins,  mulh_ins, div_ins,  rem_ins,  slt_ins,
	xor_ins,   and_ins,  or_ins,   xori_ins, andi_ins, ori_ins,  addi_ins,
	sll_ins,   srl_ins,  slli_ins, srli_ins, ld_ins,   str_ins,  slti_ins,
	blt_ins,   bge_ins,  bne_ins,  beq_ins,  aipc_ins, ecal_ins, emit_ins,
	begin_ins, end_ins,  eof_ins,  attr_ins, rt_ins, ct_ins, 
	jal_ins, jalr_ins,

	isa_count
}; 

static const char* ins_spelling[] = {
	"_null_",
	"…", "±", "•", "°", "/", "¨", ";",
	"+", "-", "*", "!", "÷", "%", "~",
	"^", "&", "|", "ˆ", "ˇ", "'", "·",
	"«", "»", "‹", "›", "`", ",", "˜",
	"<", "≥", "≠", "=", "@", "\\", "#",
	"˛", "¸", "$", ":", 
	"_RT_", "_CT_",     //  rename these four. 
	"_JAL_", "_RET_",   // 
};

// missing: text of strings, including files, and deleting names/contexts

/*
how to specify constants in the language:  (only little endian binary constants)

•°•°°••°•/ / °°•/ ·      setting register 4 to the value (5 + 32 + 64 + 256)

*/

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

//static nat byte_count = 0;
//static u8* bytes = NULL;

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

static nat 	word_size = 0,
		is_signed = 0, 
		is_compiletime = 0, 
		skip = 0;


static void print_files(void) {
	printf("here are the current files used in the program: (%lld files) { \n", file_count);
	for (nat i = 0; i < file_count; i++) {
		printf("\t file #%-8lld :   name = \"%-20s\", .start = %-8lld, .size = %-8lld\n",
			i, files[i].name, files[i].start, files[i].count);
	}
	puts("}");
}


static void print_stack(nat* stack_i, nat* stack_f, nat* stack_o, nat stack_count) {
	printf("current stack: (%lld entries) { \n", stack_count);
	for (nat i = 0; i < stack_count; i++) {
		printf("\t entry #%-8lld :   name = \"%-20s\", i = %-8lld, f = %-8lld, o = %-8lld / %lld\n", 
			i, files[stack_f[i]].name, stack_i[i], stack_f[i], stack_o[i], files[stack_f[i]].count);
	}
	puts("}");
}

static void print_arguments(void) {
	printf("\narguments[]: { \n");
	for (nat i = 0; i < arg_count; i++) {
		printf("\targuments[%llu] = { %llu  :  (.start=%llu,.count=%llu)} \n", 
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
			i, ins[i].a[0], ins_spelling[ins[i].a[0]], ins[i].size
		);
		for (nat a = 0; a < 3; a++) printf("%llu ", (nat) ins[i].a[a + 1]);
		
		printf("} -- [found @ %llu]\n", ins[i].start);

	}
	puts("}");
}

static void print_error(const char* reason, nat spot, nat __attribute__((unused)) spot2) {

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
				//printf("file %s begins at %llu!\n", files[f].name, index);
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
	fprintf(stderr, "\033[1masm: %s:%lld:%lld:", filename ? filename : "(top-level)", location, (nat)-1);
	fprintf(stderr, " \033[1;31merror:\033[m \033[1m%s\033[m\n", reason);
}

static char* read_file(const char* name, nat* out_length, nat here) {
	int d = open(name, O_RDONLY | O_DIRECTORY);
	if (d >= 0) { close(d); errno = EISDIR; goto read_error; }
	const int file = open(name, O_RDONLY, 0);
	if (file < 0) { 
		read_error:;
		char reason[4096] = {0};
		snprintf(reason, sizeof reason, "%s: \"%s\"", strerror(errno), name);
		print_error(reason, here, 0);
		fprintf(stderr, "asm: \033[31;1merror:\033[0m %s: \"%s\"\n", strerror(errno), name); 
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




static void push_name(const char* new, const nat length) {

	nat spot = 0;
	for (; spot < name_count; spot++) 
		if (length >= lengths[spot]) break;

	memmove(lengths + spot + 1, lengths + spot,         sizeof(nat) * (name_count - spot));
	memmove(values  + spot + 1, values  + spot,         sizeof(nat) * (name_count - spot));
	memmove(names   + spot + 1, names   + spot, sizeof(const char*) * (name_count - spot));
	
	lengths[spot] = length;
	names[spot] = strndup(new, length);
	values[spot] = arg_count ? arguments[arg_count - 1] : 0;
	name_count++;
}



int main(int argc, const char** argv) {

	if (argc != 2) exit(puts("asm: \033[31;1merror:\033[0m usage: ./asm <source.s>"));

	for (nat i = 0; i < isa_count; i++) 
		push_name(ins_spelling[i], strlen(ins_spelling[i]));
	

	const char* filename = argv[1];
	text_length = 0;
	text = read_file(filename, &text_length, 0);

	puts("read file: ");
	fwrite(text, 1, text_length, stdout);

	array = calloc(ct_array_count, sizeof(nat));
	array[2] = (nat) (void*) calloc(ct_memory_count, sizeof(nat));

	files[file_count].name = filename;
	files[file_count].count = text_length;
	files[file_count++].start = 0;

	nat literal = 0, bit = 1;

	{ nat index = 0, at = 0, save = 0, op = 0, max = 0;
begin:	if (op >= name_count) goto error;
	if (at == lengths[op]) goto found;
	if (index >= text_length) goto done;
	if ((unsigned char) text[index] < 33) goto nextc;
	if (names[op][at] != text[index]) goto fail;
	at++; goto nextc; 

found: 	
	for (nat i = 0; i < isa_count; i++) 
		if (not strcmp(ins_spelling[i], names[op])) { op = i; goto builtin; }
	puts("internal error: found user defined name!");
	abort();
	// process "names[op]" as a regular call to a user-defined name!;
	goto advance;
builtin:

	if (op == null_ins) {}
	else if (op == eof_ins) goto done;
	else if (op == rt_ins) is_compiletime = false;
	else if (op == ct_ins) is_compiletime = true;
	else if (op == sign_ins) is_signed = true;
	else if (op == on_ins) { literal += bit; bit <<= 1; } 
	else if (op == off_ins) bit <<= 1;
	else if (op == push_ins) { arguments[arg_count++] = literal; literal = 0; bit = 1; }
	else if (op == size_ins) word_size = arg_count ? arguments[arg_count - 1] : 0;
	else if (op == begin_ins) { /* skip forwards index until you find end_ins. */ }

	else if (not is_compiletime) {
		printf("info: pushing runtime instruction %llu(\"%s\")...\n", op, ins_spelling[op]);
		struct instruction new = {0};
		new.a[0] = (u32) op;
		new.start = index;
		for (nat i = 1; i < 4; i++) {
			new.a[i] = (u32) (arg_count > i - 1 ? arguments[arg_count - 1 - (i - 1)] : 0);
			printf(" ... argument #%llu : u32 = %u\n", i, new.a[i]);
		}
		new.size = 4;
		ins = realloc(ins, sizeof(struct instruction) * (ins_count + 1));
		ins[ins_count++] = new;

	} else {
		nat e = op;
		nat a0 = arg_count > 0 ? arguments[arg_count - 1] : 0;
		nat a1 = arg_count > 1 ? arguments[arg_count - 2] : 0;
		nat a2 = arg_count > 2 ? arguments[arg_count - 3] : 0;
		

		printf("------> info: CT: EXECUTING: op = %llu (\"%s\"), "
			"args={a0:%llu, a1:%llu, a2:%llu}\n", 
			op, ins_spelling[op], a0, a1, a2
		);

		     if (e == add_ins)	array[a0] = array[a1] + array[a2]; 
		else if (e == addi_ins)	array[a0] = array[a1] + a2; 

		else if (e == sub_ins)	array[a0] = array[a1] - array[a2]; 
		else if (e == mul_ins)	array[a0] = array[a1] * array[a2]; 
		else if (e == div_ins)	array[a0] = array[a1] / array[a2]; 
		else if (e == rem_ins)	array[a0] = array[a1] % array[a2]; 

		else if (e == and_ins)	array[a0] = array[a1] & array[a2]; 
		else if (e == andi_ins)	array[a0] = array[a1] & a2; 

		else if (e == or_ins)	array[a0] = array[a1] | array[a2]; 
		else if (e == ori_ins)	array[a0] = array[a1] | a2; 

		else if (e == xor_ins)	array[a0] = array[a1] ^ array[a2]; 
		else if (e == xori_ins)	array[a0] = array[a1] ^ a2; 

		else if (e == slt_ins)	array[a0] = array[a1] < array[a2]; 
		else if (e == slti_ins)	array[a0] = array[a1] < a2;

		else if (e == sll_ins)	array[a0] = array[a1] << array[a2]; 
		else if (e == slli_ins)	array[a0] = array[a1] << a2;

		else if (e == srl_ins)	array[a0] = array[a1] >> array[a2]; 
		else if (e == srli_ins)	array[a0] = array[a1] >> a2;

		else if (e == ld_ins) {
			if (word_size == 0) array[a0] = *( u8*)(array[a1] + a2); 
			if (word_size == 1) array[a0] = *(u16*)(array[a1] + a2); 
			if (word_size == 2) array[a0] = *(u32*)(array[a1] + a2); 
			if (word_size == 3) array[a0] = *(nat*)(array[a1] + a2); 
		}
		else if (e == str_ins) {
			if (word_size == 0) *( u8*)(array[a0] + a1) = ( u8)array[a2]; 
			if (word_size == 1) *(u16*)(array[a0] + a1) = (u16)array[a2]; 
			if (word_size == 2) *(u32*)(array[a0] + a1) = (u32)array[a2]; 
			if (word_size == 3) *(nat*)(array[a0] + a1) = (nat)array[a2];
		}

		else if (e == blt_ins) { 
			arg_count -= 3; 
			if (array[a0]  < array[a1]) { if (array[a2]) index = array[a2]; else skip = a2; } 
		} else if (e == bge_ins)  { 
			arg_count -= 3; 
			if (array[a0] >= array[a1]) { if (array[a2]) index = array[a2]; else skip = a2; } 
		} else if (e == beq_ins)  { 
			arg_count -= 3; 
			if (array[a0] == array[a1]) { if (array[a2]) index = array[a2]; else skip = a2; } 
		} else if (e == bne_ins)  { 
			arg_count -= 3; 
			if (array[a0] != array[a1]) { if (array[a2]) index = array[a2]; else skip = a2; } 

		} else if (e == jal_ins) {
			arg_count -= 2; 
			array[a0] = index;
			if (array[a1]) index = array[a1]; else skip = a1;

		} else if (e == jalr_ins) {
			arg_count -= 3; 
			array[a0] = index;
			index = array[a1] + a2;
			is_compiletime = false;


		} else { 
			printf("internal error: found unimplemented "
				"operation: %llu (\"%s\")...\n", 
				op, ins_spelling[op]
			);
			printf("asm: error: unknown compiletime instruction = %llu \n", op); 
			print_error("encountered unknown compiletime instruction", index, 0); 
			abort(); 
		}
		is_signed = false;
	}

advance: save = index; op = 0; at = 0; goto begin;
fail: 	op++; index = save; at = 0; goto begin;
nextc:	index++; if (index > max) max = index; goto begin; 
error:	print_error("unresolved symbol", save, max); exit(1); }



done:;
	puts("DONE: finished assembling program.");
	print_dictionary();
	print_instructions();
	return 0;
}













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



*/

