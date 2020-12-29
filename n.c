#include <iso646.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef int32_t nat;

static inline void compile(uint8_t* input, nat length) {
	

	nat** args = malloc(sizeof(nat*) * 64);
	nat* counts = malloc(sizeof(nat) * 64);
	nat count 0;


	nat begin = 0, top = 0;

	nat* stack = malloc(sizeof(nat) * 1024);

_0:
	if (not stack[top].ind) {
		if (not top--) goto _3; else goto _2;
	}
	stack[top].ind--;
	done = 0;
	begin = stack[top].begin;
_1:
	struct abstraction name = context->names[index];
	
	while (done < name.length) {
		nat c = name.syntax[done++];
		if (c >= 256) {
			stack[++top] = begin;
			goto _0;
		}
		if (begin >= length or c != text[begin]) goto _2;
		begin++;		
	}	
	
	if (top) {
		done = stack[top--].done;
		stack[top].data.args.push(stack[top + 1].data);
		goto _1;
	}

	if (begin == length) goto _3;
_2:
	stack[top].data.args.clear();
	goto _0;
_3:;
	return;
}

int main(int argc, const char** argv) {
	if (argc < 2) return 1; 
	else compile((uint8_t*) (intptr_t) (argv[1]), (nat) strlen(argv[i]));
}