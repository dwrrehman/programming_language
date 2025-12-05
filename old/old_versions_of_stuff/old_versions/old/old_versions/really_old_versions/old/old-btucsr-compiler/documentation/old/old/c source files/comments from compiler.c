





/*







		stack[8 * top + 0] = index_count;
			stack[8 * top + 1] = done;
			stack[8 * top + 1] = c;
			stack[8 * top + 1] = begin;
			stack[8 * top + 1] = index_count;
			stack[8 * top + 1] = index_count;
			stack[8 * top + 1] = index_count;
			stack[8 * top + 1] = index_count;
stack[0] = index_count;
	stack[1] = 0;
	stack[2] = 257;
	stack[3] = begin;
	stack[4] = 0;
	stack[5] = context_count;
	stack[6] = 0;
	stack[7] = 0;
			stack[top].begin = begin;
			
			stack[top].type = c;
			stack[top].done = done;

















		// if (not *new) {
		// 	i16 i = 128 * program[S * second];
		// 	context[i + context[i] + 1] = 0;

	if (index == 3 or index == 4) {

		if (index_count == 32767) { reason = "context limit exceeded (32767)"; goto error; }

		i8* new = context + S * index_count; 
		*new = 0;

		i16 e = stack_data[S * top + 2];
		i8 i = 0;
		while (i < program[S * e + 1]) {
			i8 c = (i8) program[S * program[S * e + 2 + i++]];
			if (*new == S - 1) { reason = "signature limit exceeded (127)"; goto error; }
			if (c != 127) new[++*new] = c; else break;
		}


		{
			i8* n = new;
			printf(" ---> found (length=%d) [ ", *n);
			for (int j = 0; j < *n; j++) {
				i8 c = n[j + 1];
				if ((u8)c < 33) printf(" (%d) ", c);
				else putchar(c);
			}
			printf(" ]\n\n");
		}

		--*new; i32 place = index_count;
		while (place and *new < context[S * indicies[place - 1]]) place--;
		memmove(indicies + place + 1, indicies + place, sizeof(i16) * (size_t) (index_count - place));
		indicies[place] = (i16) index_count;
		for (i16 s = 0; s <= top; s++) if (place <= stack[s].ind) stack[s].ind++;
		macros[index_count++] = 0; // program[S * second] ? second : 

	}






	// if (macros[index]) {
	
	// 	memcpy(stack_data + S * top, program[macros[index]], count);


	// 	i16 definition = macros[index]; // macro definition.
	// 	// stack_data[S * top + 2] // call.args0 is here!!

	// 	i16 call_count = stack_data + S * top  + 1;


	// 	i16 call_index = index;
	// 	i16* call = malloc(call_count * 2);
		
	
	// 	// memcpy(stack_data + S * top; // arg count for the call expression.

	// 	i16 stack_count = top + 1;

	// 	stack[stack_count++].ind = (i16) definition;

	// 	while (stack_count > top + 1) {
	// 		i16 e = stack[--stack_count].ind;
	// 		i16 def_index = program[S * e];

	// 		if (def_index >= call_index - call_count and def_index < call_index) { 
				
	// 			*out = call.args[call.count - (call.index - def.index)];

	// 		for (i16 i = program[S * e + 1]; i--;) 
	// 			stack[stack_count++].ind = program[S * e + 2 + i];
	// 	}

	// }


// printf("stack_count=%d | (expr=%d) : looking at %d (%.*s) (count=%d)\n", 
		// 	stack_count, e, index, context[S * index], context + S * index + 1, program[64 * e + 1]);


		if (def.index >= call.index - call.count and 
		    def.index <  call.index) { 

			*out = call.args[call.count - (call.index - def.index)];

		} else {
			*out = def;
			out->args = calloc((size_t) def.count, sizeof(struct expression));

			for (nat i = 0; i < def.count; i++) 
				copy_replace(def.args[i], call, out->args + i, context, stack, top);
		}

static inline void expand_macro(struct context* context, struct stack_element* stack, nat top) {
	struct expression call = stack[top].data;	
	copy_replace(context->names[call.index].def, call, &stack[top].data, context, stack, top);
}






const char* spellings[] = {
			"\0\1",   // should be at index 127
  			"\4name\1", // stay
			"\5data0\1", // stay
			"\5decl\3\2", // after y256
			"\5def\3\2\2", // after 256
			"\6join\2\2\2", // after 256
			"\5data8\1", // stay
			"\6data16\1", // stay
			"\3nop\2",   // after 256.
			"\41(\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1\1)\3", // should at index. 255
			"\0\0", "\0\0", "\0\0", "\0\0",
			"\0\0", "\0\0", "\0\0", "\0\0",
			"\0\0", "\0\0", "\0\0", "\0\0", 
			"\0\0", "\0\0", "\0\0", "\0\0",
			"\0\0", "\0\0", "\0\0", "\0\0",
			"\0\0", "\0\0", "\0\0",
		NULL};








// static inline void print_program(i16* program, i16 p, int depth, i8* context) { // debug
// 	for (int i = 0; i < depth; i++)  printf(".   ");
// 	i16 ind = program[64 * p];
// 	i16 count = program[64 * p + 1];
// 	printf("[i=%d] : (c=%d) : %.*s\n\n", ind, count, 
// 		context[128 * ind], context + 128 * ind + 1);
// 	for (i16 i = 0; i < count; i++) 
// 		print_program(program, program[64 * p + i + 2], depth + 1, context);
// }




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














printf("\n--------- program: -------- \n");
	for (int i = 0; i < program_count; i++) {
		i16* e = program + S * i;
		printf("%d | index=%d : \"%.*s\", count=%d, [ ", i, *e, context[S * *e + 0], context + S * *e + 1, e[1]);
		for (int j = 0; j < e[1]; j++) 
			printf("%d ", e[j + 2]);
		printf("]\n");
	}
	printf("\n--------- context: -------- \n");
	printf("indicies = (%d){ ", index_count);
	for (int i = 0; i < index_count; i++) 
		printf("%d ", indicies[i]);
	printf("}\n");
	for (int i = 0; i < index_count; i++) {
		i8* n = context + S * i;	
		printf("%d | (length=%d) [ ", i, *n);
		for (int j = 0; j < *n + 2; j++) {
			i8 c = n[j];
			if ((u8)c < 33) printf(" (%d) ", c);
			else putchar(c);
		}
		printf(" ] \n");
		if (macros[i]) {
			printf("MACRO DEF: \n");
			print_program(program, macros[i], 0, context);
			printf("END MACRO\n");
		}
	}
	printf("-----------------------------\n\n");
	if (program_count) 
		print_program(program, (i16) program_count - 1, 0, context);












if (candidate) {
		i8* n = context + S * candidate;
		printf("candidate: ");
		for (i8 s = 0; s < *n + 1; s++) {
			i8 c = n[s + 1];
			if ((u8)c < 33) printf(" (%d) ", c);
			else putchar(c);
		}
		puts("");
	}

	for (i32 b = line > 2 ? line - 2 : 0, e = line + 2, i = 0, l = 1, c = 1; i < length + 1; i++) {
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
	printf("\n\n");















 		0:  14 0   
		2:  14 0 
	4:  6 2 0 2 

		8:  14 0 
		10: 14 0 
	12: 6 2 8 10 

	16: 6 2 8 12       // WRONG

		20: 14 0 
		22: 14 0 
	24: 6 2 20 22 

	28: 6 2 20 24 














		0: 14 0 
		2: 14 0 
	4: 6 2 0 2 

		8: 14 0 
		10 14 0 
	12: 6 2 8 10 

	16: 6 2 8 12














*/



