#include <stdio.h>    // a compiler written in c, for my programming language 
#include <iso646.h>          // made by daniel warren riaz rehman.
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>//     useful for inspecing the object file:        otool -tvVhlL object.o  
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdint.h>
#include <mach/vm_prot.h> 
#include <mach-o/loader.h>

typedef uint8_t uc;

enum { bytes_limit = 4096 };
static size_t size = 0;
static unsigned char bytes[bytes_limit] = {0};

enum {
	rax, rcx, rdx, rbx, rsp, rbp, rsi, rdi, 
	r8,  r9,  r10, r11, r12, r13, r14, r15,
	register_count,
};

enum {
	indirect,
	indirect_disp8,
	indirect_disp32,
	direct
};

enum {
	scale_1,
	scale_2,
	scale_4,
	scale_8,
};

static inline void emit(uc byte) {
	bytes[size++] = byte;
}

static inline void emit4(int value) {
	bytes[size++] = value & 0xff;
	bytes[size++] = (value >> 8) & 0x0ff;
	bytes[size++] = (value >> 16) & 0x0ff;
	bytes[size++] = (value >> 24) & 0x0ff;
}

static inline void triple(uc mod, uc rx, uc rm) {
	emit((uc)(rm & 7) | (uc)((rx & 7) << 3) | (uc)(mod << 6));
}


/////////////// MOD RM BYTE functions /////////////////


static inline void emit_direct(uc rx, uc rm) {
	triple(direct, rx, rm);
}

static inline void emit_indirect(uc rx, uc base) {
	triple(indirect, rx, base);
}

static inline void emit_indirect_disp8(uc rx, uc base, uc displacement) {
	triple(indirect_disp8, rx, base);
	emit(displacement);
}

static inline void emit_indirect_disp32(uc rx, uc base, int displacement) {
	triple(indirect_disp32, rx, base);
	emit4(displacement);
}



/////////////// RIP relative ////////////////


static inline void emit_indirect_rip_relative(uc rx, int displacement) {
	triple(indirect, rx, rbp);
	emit4(displacement);
}

static inline void emit_indirect_pure_displacement(uc rx, int displacement) {
	triple(indirect, rx, rsp);
	triple(scale_1, rsp, rbp);
	emit4(displacement);
}


//////////////// SIB BYTE functions ////////////////////


static inline void emit_indexed_indirect(uc rx, uc base, uc index, uc scale) {
	triple(indirect, rx, rsp);
	triple(scale, index, base);
}

static inline void emit_indexed_indirect_disp8(uc rx, uc base, uc index, uc scale, uc displacement) {
	triple(indirect_disp8, rx, rsp);
	triple(scale, index, base);
	emit(displacement);
}

static inline void emit_indexed_indirect_disp32(uc rx, uc base, uc index, uc scale, int displacement) {
	triple(indirect_disp32, rx, rsp);
	triple(scale, index, base);
	emit4(displacement);
}




/////////////// OP CODES ////////////////////


static inline void emit_rex(uc rx, uc base, uc index) {
	emit(0x48 | (uc)(base >> 3) | (uc)((index >> 3) << 1) | (uc)((rx >> 3) << 2));
}

static inline void emit_mov_register() { emit(0x8B); } // also "LOAD", verbosely.
static inline void emit_mov_memory()   { emit(0x89); } // also "STORE", verbosely.

static inline void emit_add_register() { emit(0x03); }
static inline void emit_add_memory()   { emit(0x01); }

static inline void emit_xor_register() { emit(0x33); }
static inline void emit_xor_memory()   { emit(0x31); }

static inline void emit_inc_register() { emit(0xff); }

static inline void emit_nop1() { emit(0x90); }
static inline void emit_nop2() { emit(0x66); emit(0x90); }
static inline void emit_nop3() { emit(0x0f); emit(0x1f); emit(0x00); }
static inline void emit_nop4() { emit(0x0f); emit(0x1f); emit(0x40); emit(0x00); }
static inline void emit_nop5() { emit(0x0f); emit(0x1f); emit(0x44); emit(0x00); emit(0x00); }
static inline void emit_nop6() { emit(0x66); emit(0x0f); emit(0x1f); emit(0x44); emit(0x00); emit(0x00); }
static inline void emit_nop7() { emit(0x0f); emit(0x1f); emit(0x80); emit(0x00); emit(0x00); emit(0x00); emit(0x00); }
static inline void emit_nop8() { emit(0x0f); emit(0x1f); emit(0x84); emit(0x00); emit(0x00); emit(0x00); emit(0x00); emit(0x00); }
static inline void emit_nop9() { emit(0x66); emit(0x0f); emit(0x1f); emit(0x84); emit(0x00); emit(0x00); emit(0x00); emit(0x00); emit(0x00); }
static inline void emit_nop10() { emit(0x66); emit(0x2e); emit(0x0f); emit(0x1f); emit(0x84); emit(0x00); emit(0x00); emit(0x00); emit(0x00); emit(0x00); }

static inline void emit_nop_size(int n) {
	if (n == 1) emit_nop1();
	if (n == 2) emit_nop2();
	if (n == 3) emit_nop3();
	if (n == 4) emit_nop4();
	if (n == 5) emit_nop5();
	if (n == 6) emit_nop6();
	if (n == 7) emit_nop7();
	if (n == 8) emit_nop8();
	if (n == 9) emit_nop9();
	if (n == 10) emit_nop10();
}

static inline void align8() {
	int byte_count = 0;
	while ((size + byte_count) % 8) byte_count++;
	emit_nop_size(byte_count);
}

/////////////////////////// DEBUG  //////////////////////////////////


static inline void print_vector(int* v, int l) {
	printf("{ ");
	for (int i = 0; i < l; i++) {
		printf("%d ", v[i]);
	}
	printf("}\n");
}


static inline void dumphex(uc* ibytes, size_t byte_count) {
	for (size_t i = 0; i < byte_count; i++) {
		if (!(i % 8)) printf("\n");
		printf("%02x ", ibytes[i]);
	}
	printf("\n");
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
}






////////////////////////////// CODE GENERATION //////////////////////////

static inline int is(const char* string, const char* input, int start) {
	int ii = start;
	while (input[ii] != ';' or *string != ';') {
		if (input[ii] != *string) return 0;
		ii++;
		string++;
	}
	return 1;
}

static inline int is_type(const char* string, const char* input, int start) {
	int ii = start;
	while (input[ii] != ':' or *string != ':') {
		if (input[ii] != *string) return 0;
		ii++;
		string++;
	}
	return 1;
}

static inline int get(int arg, const char* input, int* output) {
	int r = 0x0F0F0F0F;
	int start = output[output[arg] + 2];
	if (is("register:r0;", input, start)) r = 0;
	else if (is("register:r1;", input, start)) r = 1;
	else if (is("register:r2;", input, start)) r = 2;
	else if (is("register:r3;", input, start)) r = 3;
	else if (is("register:r4;", input, start)) r = 4;
	else if (is("register:r5;", input, start)) r = 5;
	else if (is("register:r6;", input, start)) r = 6;
	else if (is("register:r7;", input, start)) r = 7;
	else if (is("register:r8;", input, start)) r = 8;
	else if (is("register:r9;", input, start)) r = 9;
	else if (is("register:r10;", input, start)) r = 10;
	else if (is("register:r11;", input, start)) r = 11;
	else if (is("register:r12;", input, start)) r = 12;
	else if (is("register:r13;", input, start)) r = 13;
	else if (is("register:r14;", input, start)) r = 14;
	else if (is("register:r15;", input, start)) r = 15;
	else abort();
	return r;
}

static inline int scratch_alloc(int* S) {
	// bitset(bitarray *s, index bit r);

	for (int i = 0; i < register_count; i++) {
		if (i == rsp) continue;
		if (i == rbp) continue;
		if (i == r12) continue;
		if (i == r13) continue;

		if (not S[i]) {
			S[i] = 1;
			return i;
		}
	}

	printf("out of registers\n");
	abort();
}

static inline void scratch_free(const int r, int* S) {
	// bitclear(bitarray *s, index bit r);
	
	if (not S[r]) {
		printf("attempt to free unused register");
		abort(); 
	}
	
	S[r] = 0;
}

// static inline int scratch_name(const int r) {
// 	//      if (r == rax_r) return rax;
// 	// else if (r == rcx_r) return rcx;
// 	// else if (r == rdx_r) return rdx;
// 	// else if (r == rbx_r) return rbx;
// 	// else if (r == rsp_r) return rsp;
// 	// else if (r == rbp_r) return rbp;
// 	// else abort();
// 	return r;
// }

int main(const int argc, const char** argv) {
	

	if (argc != 2) return printf("usage: ./compiler <input>\n");
	const int limit = 8192, ctm_limit = 4096,
		args_limit = 64, ctr_limit = 16;
	int* output = malloc(limit * sizeof(int));
	memset(output, 0x0F, limit * sizeof(int));
	int index = 0, top = 0, begin = 0, done = 0;
	int var = 0, length = 0, where = 0, best = 0;
	struct stat file_data = {0};
	int file = open(argv[1], O_RDONLY);
	if (file < 0 or stat(argv[1], &file_data) < 0) { perror("open"); exit(3); }
	length = (int) file_data.st_size;
	char* input = not length ? 0 : mmap(0, (size_t) length, PROT_READ, MAP_SHARED, file, 0);
	if (input == MAP_FAILED) { perror("mmap"); exit(4); }
	close(file);
	if (not length) goto error;
i0: 	if (begin >= length) goto i3;
	if (input[begin] == 59) goto i3;
	if (input[begin] != 92) goto i2;
i1: 	begin++;
	if (begin >= length) goto i2;
	if ((uc)input[begin] < 33) goto i1;
i2: 	begin++;
	if (begin >= length) goto i0;
	if ((uc)input[begin] < 33) goto i2;
	goto i0;
i3: 	begin++;
	if (begin >= length) goto i4;
	if ((uc)input[begin] < 33) goto i3;	
i4:	if (top + 7 >= limit) goto error;
	output[top] = limit;
	output[top + 2] = 0;
	output[top + 3] = 0;
	output[top + 5] = 0;
	output[top + 6] = begin;
	top += 4;
	best = begin;
_0:  	var = output[top + 1];
	if (var) goto _1;
	goto _3;
_1:	var = output[var + 3];
_2: 	var++;
	if ((uc)input[var] < 33) goto _2;
	if (input[var] == 58) goto _16;
_3:	if (done >= length) goto _35;
	if (var >= length) goto _35;
	if (input[done] != 58) goto _3_;
	if (input[var] == 58) goto _8;
_3_: 	if (input[done] != input[var]) goto _35;
	if (input[done] != 92) goto _6;
_4: 	done++; 
	if ((uc)input[done] < 33) goto _4;
_5: 	var++;
	if ((uc)input[var] < 33) goto _5;
_6:	var++;
	if (var >= length) goto _7;
	if ((uc)input[var] < 33) goto _6;
_7: 	done++; 
	if (done >= length) goto _3;
	if ((uc)input[done] < 33) goto _7;
	goto _3;
_8:	done++;
	if ((uc)input[done] < 33) goto _8;
	begin = output[top + 2];
_9:	if (input[done] == 59) goto _21;
	if (input[done] != 58) goto _10;
	if (top + 7 >= limit) goto error;
	output[top] = index;
	output[top + 3] = done;
	output[top + 5] = top;
	output[top + 6] = begin;
	top += 4;
	index = 0;
	done = 0;
	goto _0;
_10:	if (input[done] != 92) goto _12;
_11: 	done++;
	if ((uc)input[done] < 33) goto _11;
_12:	if (begin >= length) goto _28;
	if (input[done] != input[begin]) goto _28;
_13: 	begin++;
	if (begin >= length) goto _14;
	if ((uc)input[begin] < 33) goto _13;
_14: 	done++;
	if ((uc)input[done] < 33) goto _14;
	if (begin <= best) goto _15; 
	best = begin; 
	where = done;
_15:	goto _9;
_16:	index = limit;
_17:	if (begin >= length) goto _20;
	if (input[begin] == 59) goto _20;
	if (input[begin] != 92) goto _19;
_18: 	begin++;
	if (begin >= length) goto _19;
	if ((uc)input[begin] < 33) goto _18;
_19: 	begin++;
	if (begin >= length) goto _19_;
	if ((uc)input[begin] < 33) goto _19;
_19_:	goto _17;
_20:	begin++;
	if (begin >= length) goto _20_;
	if ((uc)input[begin] < 33) goto _20;
_20_:	if (begin <= best) goto _21; 
	best = begin;
	where = done;
_21:	output[top] = index;
	output[top + 3] = done;
	var = output[top + 1];
	if (not var) goto _27;
	if (top + 7 >= limit) goto error;
	top += 4;
	output[top + 1] = output[var + 1];
	output[top + 2] = begin;
	index = output[var];
	done = output[var + 3];
_22: 	done++;
	if ((uc)input[done] < 33) goto _22;
_23:	if (input[done] == 58) goto _26;
	if (input[done] != 92) goto _25;
_24: 	done++; 
	if ((uc)input[done] < 33) goto _24;
_25: 	done++;
	if ((uc)input[done] < 33) goto _25;
	goto _23;
_26:	done++;
	if ((uc)input[done] < 33) goto _26;
	goto _9;
_27:	if (begin == length) goto success;
_28:	if (index == limit) goto _34;
	var = output[index + 2];
_29:	if (input[var] == 58) goto _32;
	if (input[var] != 92) goto _31;
_30: 	var++;
	if ((uc)input[var] < 33) goto _30;
_31: 	var++;
	if ((uc)input[var] < 33) goto _31;
	goto _29;
_32:	var++;
	if ((uc)input[var] < 33) goto _32; 
	if (input[var] == 59) goto _33;
	if (input[var] == 58) goto _33;
	if (var == done) goto _35; 
	goto _32;
_33:	if (var == done) goto _35;
_34:	if (not top) goto error;
	top -= 4;
	index = output[top];
	done = output[top + 3];
	goto _28;
_35:	index += 4;
	if (index >= top) goto _34;
	if (output[index] != limit) goto _35;
	done = output[index + 2];
	goto _0;

success: top += 4;
	puts("success: compile successful."); 
	debug("success", input, output, length, begin, top, index, done);

	int this = 0, next = 0, count = 0, skip = 0;
	int* args = malloc(args_limit * sizeof(int));
	size_t* ctr = malloc(ctr_limit * sizeof(size_t));
	
	size_t* memory = malloc(ctm_limit * sizeof(size_t));
	memset(memory, 0x0F, ctm_limit * sizeof(size_t));
	memset(ctr, 0x0F, ctr_limit * sizeof(size_t));

	int* state = malloc(register_count * sizeof(int));
	memset(state, 0, register_count * sizeof(int));
	
	printf("\n---------------parsing output as tree:----------------\n\n");
	
code:	if (this >= top) goto out;
	if (output[this] == limit) {
		printf(" %10d : %10di %10dp %10db %10dd   : UDS :   ", 
			this, output[this + 0], output[this + 1], output[this + 2], output[this + 3]);
		int s = output[this + 2];
		while (input[s] != ';') {
			putchar(input[s]);
			s++;
		}
		printf("\n");
		goto move;
	}
	if (input[output[this + 3]] != 59) goto move;
	printf("\n\n\n------------------------- %d ---------------------------\n", this);
	printf(" %10d : %10di %10dp %10db %10dd   :   ", 
		this, output[this + 0], output[this + 1], output[this + 2], output[this + 3]);
	int s = output[output[this] + 2];
	while (input[s] != ';') {
		putchar(input[s]);
		s++;
	}
	printf("\n");
	next = this;
	count = 0;
next_child:
	index = output[next];
	if (index == limit) goto first;
	done = output[next + 3];
	var = output[index + 2];
fail:	if (input[var] == 58) goto more;
	if (input[var] != 92) goto jj;
kk: 	var++;
	if ((uc)input[var] < 33) goto kk;
jj: 	var++;
	if ((uc)input[var] < 33) goto jj;
	goto fail;
more:	var++;
	if ((uc)input[var] < 33) goto more;
	if (input[var] == 59) goto check;
	if (input[var] == 58) goto check;
	goto more;
check:	if (var == done) goto first;
	args[count++] = next - 4;
	next = output[next - 3];
	goto next_child;
first:;
	printf("\n    (index=%d) : parsed %d arguments : ", index, count);
	print_vector(args, count);
	printf("\n");

	int start = output[output[this] + 2];

	if (skip) {
		if (is("unit:at:label::unit:;", input, start) and output[args[count - 1]] == skip) {
			output[output[args[count - 1]] + 3] = args[count - 2];
			skip = 0;
			this = args[count - 2];
			goto code;
		}
		goto move;
	}

	if (is("unit:at:label::unit:;", input, start)) {
		output[output[args[count - 1]] + 3] = args[count - 2];

	} else if (is("unit:if:register:<:register:,:label:;", input, start)) {
		int left = get(args[count - 1], input, output);
		int right = get(args[count - 2], input, output);
		if (ctr[left] < ctr[right]) goto branch;

	} else if (is("unit:if:register:=:register:,:label:;", input, start)) {
		int left = get(args[count - 1], input, output);
		int right = get(args[count - 2], input, output);
		if (ctr[left] == ctr[right]) {
		branch:	if (output[output[args[count - 3]] + 3]) {
				this = output[output[args[count - 3]] + 3];
				goto code;
			}
			skip = output[args[count - 3]];
		}
	} else if (is("unit:increment:register:;", input, start)) 
		ctr[get(args[count - 1], input, output)]++;
	else if (is("unit:decrement:register:;", input, start)) 
		ctr[get(args[count - 1], input, output)]--;
	else if (is("unit:zero:register:;", input, start)) 
		ctr[get(args[count - 1], input, output)] = 0;
	else if (is("unit:copy:register:,:register:;", input, start)) 
		ctr[get(args[count - 1], input, output)] = ctr[get(args[count - 2], input, output)];
	else if (is("unit:add:register:,:register:;", input, start)) 
		ctr[get(args[count - 1], input, output)] += ctr[get(args[count - 2], input, output)];
	else if (is("unit:subtract:register:,:register:;", input, start)) 
		ctr[get(args[count - 1], input, output)] -= ctr[get(args[count - 2], input, output)];
	else if (is("unit:multiply:register:,:register:;", input, start)) 
		ctr[get(args[count - 1], input, output)] *= ctr[get(args[count - 2], input, output)];
	else if (is("unit:divide:register:,:register:;", input, start)) 
		ctr[get(args[count - 1], input, output)] /= ctr[get(args[count - 2], input, output)];
	else if (is("unit:modulo:register:,:register:;", input, start)) 
		ctr[get(args[count - 1], input, output)] %= ctr[get(args[count - 2], input, output)];
	else if (is("unit:xor:register:,:register:;", input, start)) 
		ctr[get(args[count - 1], input, output)] ^= ctr[get(args[count - 2], input, output)];
	else if (is("unit:and:register:,:register:;", input, start)) 
		ctr[get(args[count - 1], input, output)] &= ctr[get(args[count - 2], input, output)];
	else if (is("unit:or:register:,:register:;", input, start)) 
		ctr[get(args[count - 1], input, output)] |= ctr[get(args[count - 2], input, output)];
	else if (is("unit:store:register:,:register:;", input, start)) 
		memory[ctr[get(args[count - 1], input, output)]] = ctr[get(args[count - 2], input, output)];
	else if (is("unit:load:register:,:register:;", input, start)) 
		ctr[get(args[count - 1], input, output)] = memory[ctr[get(args[count - 2], input, output)]];
	
	else if (is("unit:nop;", input, start)) {
		bytes[size++] = 0x90;

	} else if (is("reg64:(:reg64:+:reg64:);", input, start)) {

		uc dest = (uc)output[args[count - 1] + 2];
		uc source = (uc)output[args[count - 2] + 2];
		
		printf("BEFORE: state = ");
		print_vector(state, register_count);

		if (source != dest) scratch_free((int)source, state);
		output[this + 2] = (int)dest;

		emit_rex(dest, source, 0);
		emit_add_register();
		emit_direct(dest, source);

		printf("AFTER: state = ");
		print_vector(state, register_count);

	} else if (is("unit:xor:reg64:,:reg64:;", input, start)) {

		uc dest = (uc)output[args[count - 1] + 2];
		uc source = (uc)output[args[count - 2] + 2];
		
		printf("BEFORE: state = ");
		print_vector(state, register_count);

		if (source != dest) scratch_free((int)source, state);
		output[this + 2] = (int)dest;

		emit_rex(dest, source, 0);
		emit_xor_register();
		emit_direct(dest, source);

		printf("AFTER: state = ");
		print_vector(state, register_count);



	} else if (is("unit:inc:reg64:;", input, start)) {

		uc dest = (uc)output[args[count - 1] + 2];
		
		printf("BEFORE: state = ");
		print_vector(state, register_count);

		output[this + 2] = (int)dest;

		emit_rex(dest, 0, 0);
		emit_inc_register();
		emit_direct(dest, 0);

		printf("AFTER: state = ");
		print_vector(state, register_count);

	} else if (is("unit:new::;", input, start)) {

		printf("BEFORE: state = ");
		print_vector(state, register_count);
	
		uc r = (uc) scratch_alloc(state);
		output[args[count - 1] + 3] = (int)r;

		printf("AFTER: state = ");
		print_vector(state, register_count);

	} else if (is("unit:discard:reg64:;", input, start)) {

		printf("BEFORE: state = ");
		print_vector(state, register_count);

		int r = output[args[count - 1] + 2]; 
		printf("calling: scratch_free(%d)\n", r);
		scratch_free(r, state);
	
		printf("AFTER: state = ");
		print_vector(state, register_count);

	} else if (is_type("reg64:", input, start)) { // else if, index != limit:
		output[this + 2] = output[index + 3];

		printf("REG USE: state = ");
		print_vector(state, register_count);
	}

move: 	this += 4;
	goto code;
out:;

	align8();

	const int number_of_sections = 1;

	struct mach_header_64 header = {0};	
	struct segment_command_64 command = {0};
	struct section_64 section = {0};


	header.magic = MH_MAGIC_64;
	header.cputype = (int)CPU_TYPE_X86 | (int)CPU_ARCH_ABI64;
	header.cpusubtype = (int)CPU_SUBTYPE_I386_ALL | (int)CPU_SUBTYPE_LIB64;
	header.filetype = MH_OBJECT;
	header.ncmds = 1;
	header.sizeofcmds = 0;
	header.flags = MH_NOUNDEFS | MH_SUBSECTIONS_VIA_SYMBOLS;
	
	command.cmd = LC_SEGMENT_64;
	command.cmdsize = sizeof(struct segment_command_64) + sizeof(struct section_64) * number_of_sections;

	header.sizeofcmds += command.cmdsize;

	strncpy(command.segname, "__TEXT", 16);
	command.vmsize = sizeof header + sizeof command + sizeof section * number_of_sections + size;
	command.vmaddr = 0;
	command.fileoff = 0;
	command.filesize = sizeof header + sizeof command + sizeof section * number_of_sections + size;
	command.maxprot = VM_PROT_ALL;
	command.initprot = VM_PROT_ALL;
	command.nsects = number_of_sections;
	
	strncpy(section.sectname, "__text", 16);
	strncpy(section.segname, "__TEXT", 16);
	section.addr = 0;
	section.size = size;
	section.offset = sizeof header + sizeof command + sizeof section * number_of_sections;
	section.align = 3;
	section.reloff = 0;
	section.nreloc = 0;

	printf("\ndebugging header bytes:\n------------------------\n");
	dumphex((void*) &header, sizeof(header));

	printf("\ndebugging command bytes:\n------------------------\n");
	dumphex((void*) &command, sizeof(command));

	printf("\ndebugging section bytes:\n------------------------\n");
	dumphex((void*) &section, sizeof(section));

	printf("\ndebugging bytes bytes:\n------------------------\n");
	dumphex((void*) bytes, size);
	
	printf("\n\n--> outputting %zd bytes to output file...\n\n", size);

	int out_file = open("object.o", O_WRONLY | O_CREAT);
	if (out_file < 0) { perror("open"); exit(4); }

	write(out_file, &header, sizeof header);
	write(out_file, &command, sizeof command);
	write(out_file, &section, sizeof section);
	write(out_file, bytes, size);

	close(out_file);

	printf("DEBUG: ctr:\n{\n");
	for (int i = 0; i < ctr_limit; i++) {
		if (ctr[i] != 0x0F0F0F0F0F0F0F0F) 
			printf("\tr%d = %zu\n", i, ctr[i]);
	}
	printf("}\n");

	printf("DEBUG: memory:\n{\n");
	for (int i = 0; i < ctm_limit; i++) {
		if (memory[i] != 0x0F0F0F0F0F0F0F0F) 
			printf("\t[%d] = %zu\n", i, memory[i]);
	}
	printf("}\n");
	goto clean_up;

error:; 
	int at = 0, line = 1, column = 1, wat = 0, wline = 1, wcolumn = 1;
	while (at < best and at < length) {
		if (input[at++] != 10) { column++; } 
		else { line++; column = 1; }
	}
	while (wat < where and wat < length) {
		if (input[wat++] != 10) { wcolumn++; } 
		else { wline++; wcolumn = 1; }
	}
	fprintf(stderr, "%u %u %u %u %u %u %u %u\n", 
			line, column, at, wline, wcolumn, wat, top, limit);
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



