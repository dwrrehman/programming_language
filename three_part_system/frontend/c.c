//front end for language
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <iso646.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <stdnoreturn.h>

typedef uint64_t nat;

static nat text_length = 0;
static char* text = NULL;

static char* load_file(const char* filename, nat* length) {
	int file = open(filename, O_RDONLY);
	if (file < 0) { 
		printf("compiler frontend: error: could not open '%s': %s\n", 
			filename, strerror(errno)
		); 
		exit(1);
	}
	*length = (nat) lseek(file, 0, SEEK_END);
	lseek(file, 0, SEEK_SET);
	char* s = calloc(*length + 2, 1);
	read(file, s, *length);
	close(file);
	return s;
}

static void print_nats(nat* array, nat count) {
	printf("(%llu) { ", count);
	for (nat i = 0; i < count; i++) {
		printf("%lld ", array[i]);
	}
	printf("}\n");
}

static void print_string_index(nat given_index) {
	const int index = (int) given_index;
	const int radius = 30;
	putchar('\t');

	for (int offset = -radius; offset < radius; offset++) {

		if (not offset) printf("\033[7;33m");
		if (index + offset >= 0 and index + offset < (int) text_length) {
			const nat at = (nat) (index + offset);
			if (text[at] == 10) printf("\033[7mN\033[0m");
			else if (text[at] == 9) printf("\033[7mT\033[0m");
			else putchar(text[at]);
		}
		if (not offset) printf("\033[0m");
	}
	printf("\n\t");
	for (int offset = -radius; offset < radius; offset++) {
		if (not offset) break;
		if (index + offset >= 0 and index + offset < (int) text_length) 
			putchar(' ');
	}
	printf("\033[32;1m^\033[0m"); 
	puts("");
}

struct statement {
	nat type;
	char* name;
	nat value;
	nat left;
	nat right;
};

enum statement_type {
	eoi_,
	emit_,
	do_,
	at_,
	del_,
	set_,
	store_,
	define_,
	else_,
	end_,
	repeat_,
	ifthen_,
	while_,	
	name_,
	number_,
	add_,
	sub_,
	mul_,
	div_,
	rem_,
	eor_,
	and_,
	or_,
	si_,
	sd_,
	lt_,
	eq_,
	statement_type_count
};

static char* statement_spelling[statement_type_count] = {
	"eoi",
	"emit",
	"do",
	"at",
	"del",
	"set",
	"store",
	"define",
	"else",
	"end",
	"repeat",
	"ifthen",
	"while",
	"name",
	"number",
	"add",
	"sub",
	"mul",
	"div",
	"rem",
	"eor",
	"and",
	"or",
	"si",
	"sd",
	"lt",
	"eq",

};

static bool is_looking_at(const char* recognize, const nat at) {
	const nat n = strlen(recognize);
	for (nat i = 0; i < n; i++) 
		if (text[at + i] != recognize[i]) return false;
	return isspace(text[at + n]);
}

//static bool is_looking_at_symbol(const char* recognize, const nat at) {
//	const nat n = strlen(recognize);
//	for (nat i = 0; i < n; i++) 
//		if (text[at + i] != recognize[i]) return false;
//	return true;
//}

static void print_program(struct statement* program, nat program_count) {
	puts("printing the program:");
	for (nat i = 0; i < program_count; i++) {
		printf("%5llu: statement "
			"{ .type = %s, .name = \"%s\", "
			".value = %llu, .left = %llu, .right = %llu }\n", 
			i, 
			statement_spelling[program[i].type],
			program[i].name, program[i].value, program[i].left, program[i].right
		);
	}
}

int main(int argc, const char** argv) {

	if (argc != 2) return puts("compiler frontend: error: no input file given");

	text = load_file(argv[1], &text_length);
	text[text_length++] = '\n';

	struct statement program[4096] = {0};
	nat program_count = 0, at = 0;

#define skipspace   while (at < text_length and isspace(text[at])) at++;

	skipspace;
	while (at < text_length) {

		if (text[at] == '(') {
			bool comment = 1;
			while (comment and at < text_length) {
				if (text[at] == '(') comment++;
				if (text[at] == ')') comment--;
				at++;
			}
			if (comment) printf("syntax error: unterminated comment\n");
			skipspace;
			continue;
		}

		if (is_looking_at("del", at)) {
			at += 4; skipspace;
			program[program_count++].type = del_;
			parsename:; const nat begin = at;
			while (at < text_length and not isspace(text[at])) at++;
			program[program_count - 1].name = strndup(text + begin, at - begin);
			skipspace;
			continue;
		}


		if (is_looking_at("emit", at)) {
			at += 5; skipspace;
			program[program_count++].type = emit_;
			goto parsename;
		} 

		if (is_looking_at("do", at)) {
			at += 3; skipspace;
			program[program_count++].type = do_;
			goto parsename;
		} 

		if (is_looking_at("at", at)) {
			at += 3; skipspace;
			program[program_count++].type = at_;
			goto parsename;
		} 

		if (is_looking_at("eoi", at)) {
			at += 4; skipspace
			program[program_count++].type = eoi_;
			continue;
		}

		if (is_looking_at("end", at)) {
			at += 4; skipspace
			program[program_count++].type = eoi_;
			continue;
		}

		if (is_looking_at("else", at)) {
			at += 5; skipspace
			program[program_count++].type = eoi_;
			continue;
		}

		if (is_looking_at("repeat", at)) {
			at += 7; skipspace
			program[program_count++].type = repeat_;
			continue;
		}

		if (is_looking_at("define", at)) {
			at += 7; skipspace;
			program[program_count++].type = define_;
			{ parsenameexpr:; const nat begin = at;
			while (at < text_length and not isspace(text[at])) at++;
			program[program_count - 1].name = strndup(text + begin, at - begin);
			skipspace;
			if (text[at] != '=') {
				puts("expected '=' in statement: ");
				goto error;
			} at++; skipspace; } 
			{ puts("info: we have to parse the expression now!!! : ");
			nat stack[4096] = {0};
			nat arg_count[4096] = {0};
			nat stack_count = 1;
			stack[0] = program_count - 1;
			arg_count[0] = 1;

		el:
			if (at >= text_length) goto done_expression;
			if (not stack_count) goto done_expression;

			puts("NEW ITERATION OF EL");
			printf("stack:"); print_nats(stack, stack_count);
			printf("argcount:"); print_nats(arg_count, stack_count);
			print_program(program, program_count);
			//getchar();

			if (arg_count[stack_count - 1] == 2) program[stack[stack_count - 1]].right = program_count;
			else if (arg_count[stack_count - 1] == 1) program[stack[stack_count - 1]].left = program_count;
			else if (arg_count[stack_count - 1] == 0) { stack_count--; goto el; } 
			arg_count[stack_count - 1]--;

			if (text[at] == '+') {
				program[program_count++].type = add_;
				push_expr: at++; skipspace;
				stack[stack_count] = program_count - 1;
				arg_count[stack_count++] = 2; } 
			else if (text[at] == '-') { program[program_count++].type = sub_; goto push_expr; } 
			else if (text[at] == '*') { program[program_count++].type = mul_; goto push_expr; } 
			else if (text[at] == '/') { program[program_count++].type = div_; goto push_expr; } 
			else if (text[at] == '%') { program[program_count++].type = rem_; goto push_expr; } 
			else if (text[at] == '~') { program[program_count++].type = eor_; goto push_expr; } 
			else if (text[at] == '|') { program[program_count++].type = or_;  goto push_expr; } 
			else if (text[at] == '&') { program[program_count++].type = and_; goto push_expr; } 
			else if (text[at] == '^') { program[program_count++].type = si_;  goto push_expr; } 
			else if (text[at] == '!') { program[program_count++].type = sd_;  goto push_expr; } 
			else if (text[at] == '<') { program[program_count++].type = lt_;  goto push_expr; } 
			else if (text[at] == '=') { program[program_count++].type = eq_;  goto push_expr; } 

			else if (isdigit(text[at])) {
				const nat begin = at;
				while (at < text_length and isdigit(text[at])) at++; 
				program[program_count++].type = number_;
				program[program_count - 1].name = strndup(text + begin, at - begin);
				skipspace;				
			} else {
				const nat begin = at;
				while (at < text_length and 
					(isalnum(text[at]) or 
					text[at] == '_')) at++;
				program[program_count++].type = name_;
				program[program_count - 1].name = strndup(text + begin, at - begin);
				skipspace;
			} goto el; }
		done_expression:;
			puts("done parsing expression.");
			puts("COMPLETED RECURSIVE DESCENT");
			printf("completed expression tree:");
			print_program(program, program_count);
			continue;
		}
		if (is_looking_at("set", at)) {
			at += 4; skipspace;
			program[program_count++].type = set_;
			goto parsenameexpr; 
		}
	error: 
		puts("parse error"); 
		print_string_index(at);
		exit(1);
	}

	for (nat i = 0; i < program_count; i++) {
		const nat op = program[i].type;
		if (op == emit_ or op == number_) {
			program[i].value = (nat) atoi(program[i].name);

		} else {

		}
	}

	puts("\n\n\n\n\n");
	print_program(program, program_count);
}





























































/*




			if (arg_count[stack_count - 1] == 1) {
				//puts("found argcount 1, adding right node!");
				//program[stack[stack_count - 1]].right = program_count - 1;

			} else if (arg_count[stack_count - 1] == 0) {
				puts("popping off stack because argcount was 0, adding left as well...");
				//program[stack[stack_count - 1]].left = program_count - 1;
				stack_count--;
				goto el;
			} 

*/




































































































































/*


#define indent \
	nat begin = program[i + 1], length = 0; i += 2; \
	for (nat t = begin; t < text_length and not isspace(text[t]); t++) length++;

	puts("parse success!"); 
	for (nat i = 0; i < node_count; ) {

		if (nodes[i] == del_ins) {
			indent; printf("\tdel <%.*s>\n", (int) length, text + begin);

		} else if (nodes[i] == do_ins) {
			indent; printf("\tdo <%.*s>\n", (int) length, text + begin);

		} else if (nodes[i] == at_ins) {
			indent; printf("\tat <%.*s>\n", (int) length, text + begin);

		} else if (nodes[i] == eoi_ins) {
			indent; printf("\teoi\n"); i++;
		}
	}
	exit(0);
}

*/






/*

	struct node nodes[4096] = {0};
	nat state[4096] = {0};
	nat progress[4096] = {0};
	nat stack_count = 0;
	nat at = i;
	nodes[stack_count] = (struct node) {.type = ret_node, .count = 0, .args = NULL};
	state[stack_count] = block_node;
	progress[stack_count++] = 0;
	char c = 0;






struct token {
	nat type;
	nat value;
	nat location;
	char* name;
};



*/












/*

1202512044.192626

FACT: we only need to actually use LL1 recursive descent for expressions!!
	and its kinda like way easier than usual, because each thing is only one character!

	




(
 a program to compute the 
 number of prime numbers 
 less than a given number, 
 limit.
)

define limit = 100000
define count = 0
define i = 0

repeat
	define j = 2
	at innerloop
		if < j i then else do prime end
		define r = % i j
		if = r 0 then do composite end del r
		set j = + j 1 del j
		do innerloop del innerloop
at prime 
	set count = + count 1
at composite
	set i = + i 1
	while < i limit del i



	if = limit limit then 
		set limit = 0
		if < i limit then 	
			set i = 0	
		else 
			set i = + i 1
		end
	else 
		set limit = + limit 1
	end

eoi








----------------------

if cond then        (if cond != 0)
	code1
else 
	code2
end

----------------------

li zero 0
...

lt 0 cond else0

	(...block-body...)

do end0
at else0

	(...block-body...)

at end0


----------------------------




if found:        emit this:
-------------------------------

define x = E      def x  set x e
set x = E         set x e 
set @E = E        stb e1 e1
if E then         lt 0 e else0
while E           lt 0 e loop0


do x              do x
at x              at x
del x             del x
emit N            emit N
else              do end0 at else0
end               at end0
repeat            at loop0
eoi







*/



