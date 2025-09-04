#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <iso646.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <ctype.h>
#include <stdnoreturn.h>

// a simple c-like language compiler / byte-code interpreter, just for fun.
// written by dwrr  on 202403273.011738
/*


ir generated:
-------

	set b c
	add b a
	blt x y label
		..stuff..
	at label
	nop
	and x y z



B:
---------

	b = c + a
	if (x < y) {
		..stuff..
	}
	nop
	x = y & z
	

C:
---------

	unsigned int b = c + a;
	if (x < y) {
		..stuff..;
	}
	nop;
	char x = y & z;







other example of the syntax:


at my cool thingy
	define my x = 1
	if x < 0001 then
		repeat
			set @(p + my x) = 0
			set my x = my x + 1
		while my x < 0001
		do done with everything
	else
		set my x = 0
		do my cool thingy
	end

at done with everything
	set my x = 11111
	eoi




highlighths:
--------------------------


	- no functions
	- labels can contain spaces
	- only binary literals
	- no compound operators, only   
		+ addition
		- subtraction
		* multiply
		/ division
		% remainder 
		& and
		| or
		~ xor
		^ shift up
		! shift down
		() parenthesis for precedence
		@ load/store
		< set to 1 if less than, else 0


	only certain gramatically correct statements:

		[your comment here]
		define X = Y
		set @(X) = Y
		set X = Y
		if C then ...code... else ...code... end
		repeat ...code... while C
		do L
		at L
		eoi


*/

typedef uint64_t nat;

static const char* keywords[] = {
	"eoi", 
	"at", "do", "while", "repeat", 
	"if", "then", "else", "end", 
	"set", "define",
};

static const nat keyword_count = sizeof keywords / sizeof(char*);

static const char* operators = "<+-*/%&|~^!()@";

enum token_type {
	null_token, number_token, name_token,

	eoi_token, 
	at_token, do_token, while_token, repeat_token, 
	if_token, then_token, else_token, end_token, 
	set_token, define_token,

	less_token, 
	add_token, subtract_token, 
	multiply_token, divide_token, modulo_token, 
	and_token, or_token, eor_token,
	shiftup_token, shiftdown_token, 
	open_paren_token, close_paren_token, 
	deref_token,

	token_type_count
};

static const char* token_spelling[token_type_count] = {
	"null_token", "number_token", "name_token",

	"eoi_token", 
	"at_token", "do_token", "while_token", "repeat_token", 
	"if_token", "then_token", "else_token", "end_token", 
	"set_token", "define_token",

	"less_token", 
	"add_token", "subtract_token", 
	"multiply_token", "divide_token", "modulo_token", 
	"and_token", "or_token", "eor_token",
	"shiftup_token", "shiftdown_token", 
	"open_paren_token", "close_paren_token", 
	"deref_token", 
	
};

enum node_type {
	null_node, name_list_node,
	do_node, declaration_node,
	number_node, parens_node, name_node, 
	statement_list_node, block_node, 
	labeled_statement_node, label_node,
	sum_node, product_node, nor_node, 
	lvalue_node, deref_node, address_node,
	node_type_count
};

static const char* node_spelling[node_type_count] = {
	"null_node",  "name_list_node",
	"do_node", "declaration_node",
	"number_node", "parens_node", "name_node", 
	"statement_list_node", "block_node", 
	"labeled_statement", "label_node",
	"sum_node", "product_node", "nor_node", 
	"lvalue_node", "deref_node", "address_node", 
};

static int colors[] = {
	34, 57, 179, 120, 189, 37, 45, 67, 79, 145, 156, 81, 122, 111, 
	99, 136, 50, 60, 42, 144, 149, 180, 172, 161, 199, 123, 145, 
};

struct token {
	nat type;
	nat value;
	nat location;
	const char* name;
};

struct tree_node {
	nat type;
	nat start;
	nat end;
	nat count;
	struct tree_node** children;
};

static bool is_delimiter(char c) { 
	return isspace(c) or strchr(operators, c);
}

static void print_tokens(struct token* tokens, const nat token_count) {
	
	printf("printing all tokens: (%llu tokens): {\n\n", token_count);
	for (nat i = 0; i < token_count; i++) {

		printf(" @%-6llu: token(.type = \033[38;5;%dm%-20s\033[0m ",
			tokens[i].location, 
			colors[tokens[i].type], 
			token_spelling[tokens[i].type]
		);

		if (tokens[i].type == name_token) 
			printf(".name = \"\033[32m%s\033[0m\" ", tokens[i].name);

		if (tokens[i].type == number_token) 
			printf(".value = \033[31m%llu\033[0m ", tokens[i].value);

		puts("\n");
	}
	puts("}");
}

static void print_string_index(const char* text, nat length, nat given_index) {
	const int index = (int) given_index;
	const int radius = 14;
	putchar('\t');

	for (int offset = -radius; offset < radius; offset++) {

		if (not offset) printf("\033[7;31m");
		if (index + offset >= 0 and index + offset < (int) length) {
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
		if (index + offset >= 0 and index + offset < (int) length) 
			putchar(' ');
	}
	printf("\033[32;1m^\033[0m"); 
	puts("");
}

static nat lex(struct token** given_tokens, char* text, nat length) {
	
	struct token* tokens = NULL;
	nat token_count = 0;

	for (nat i = 0; i < length; ) {

		printf("processing %lluth character, \"%c\"(%d)...\n", i, text[i], text[i]);

		if ((unsigned char) text[i] < 33) { i++; goto next_char; }

		const char* is_operator = strchr(operators, text[i]);
		if (is_operator) {
			nat type = less_token + (nat)(is_operator - operators);
			tokens = realloc(tokens, sizeof(struct token) * (token_count + 1));
			tokens[token_count++] = (struct token) { .type = type, .location = i };
			i++; goto next_char;
		} 

		for (nat k = 0; k < keyword_count; k++) {

			const nat keyword_length = strlen(keywords[k]);

			if (i + keyword_length <= length and not strncmp(text + i, keywords[k], keyword_length) and 
			    (is_delimiter(text[i + keyword_length]))
			) {
				tokens = realloc(tokens, sizeof(struct token) * (token_count + 1));
				tokens[token_count++] = (struct token) { .type = eoi_token + k, .location = i };
				i += keyword_length;
				goto next_char;
			}
		}

		const nat start = i;
		nat t = i;
		while (not is_delimiter(text[t])) t++;
		const nat end = t;

		if (end == start) abort();

		const char* string = strndup(text + start, end - start);
		const nat string_length = strlen(string);
		
		nat r = 0, s = 1;
		for (nat j = 0; j < string_length; j++) {
			if (string[j] == '0') s <<= 1; 
			else if (string[j] == '1') { r += s; s <<= 1; }
			else goto push_name;
		}
	
		puts("pushing number token...");
		tokens = realloc(tokens, sizeof(struct token) * (token_count + 1));
		tokens[token_count++] = (struct token) { 
			.type = number_token,
			.value = r,
			.location = i,
		};
		//printf("number: advancing %llu characters...\n", string_length);
		i += string_length;
		goto next_char;

	push_name:
		puts("pushing name token...");
		tokens = realloc(tokens, sizeof(struct token) * (token_count + 1));
		tokens[token_count++] = (struct token) { 
			.type = name_token,
			.name = string,
			.location = i,
		};
		//printf("name: advancing %llu characters...\n", string_length);
		i += string_length;
	next_char:;
	}
	*given_tokens = tokens;
	return token_count;
}


static nat max_at = 0;
static char* file_text = NULL;
static nat file_length = 0;

static void print_error(const char* reason, const char* filename, nat offset, nat max) {
	fprintf(stderr, "compiler: %s:%llu:%llu: \033[31;1merror:\033[0m \033[1m%s\033[0\n", filename, offset, max, reason);
	print_string_index(file_text, file_length, max);
}

static nat terminal(nat type, nat at, struct token* tokens, nat count) {
	printf("in %s(%s, %llu)...\n", __func__, token_spelling[type], at);
	if (at > max_at) max_at = at;
	if (at == count) return 0;
	if (tokens[at].type != type) return 0;
	return 1;
}

static struct tree_node* name_list(nat at, struct token* tokens, nat count) {
	printf("in %s(%llu)...\n", __func__, at);
	//puts("trying first name in list...");
	nat start = at;
	nat r = terminal(name_token, at, tokens, count);
	if (not r) return NULL;
	
	at += r;
	//printf("advancing at by %llu, to be %llu...\n", r, at);

try_more_names:;
	//puts("trying more names in list...");
	r = terminal(name_token, at, tokens, count);
	if (not r) { 
		//puts("could not recognize name..."); 
		goto done; }
	//puts("success! trying next name...");
	at += r;
	//printf("advancing at by %llu, to be %llu...\n", r, at);
	goto try_more_names;
done:;
	struct tree_node* new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = name_list_node;
	return new;
}


static struct tree_node* term(nat at, struct token* tokens, nat count);


//lvalue:
//	name_list
//	at term

static struct tree_node* lvalue(nat at, struct token* tokens, nat count) {

	printf("in %s(%llu)...\n", __func__, at);
	nat start = at;
	puts("trying to recognize lvalue name list...");

	nat r = terminal(set_token, at, tokens, count);
	if (not r) goto error;
	at += r;

	struct tree_node* names = name_list(at, tokens, count);
	if (not names) goto try_at_expr;
	at = names->end;

	struct tree_node* new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = lvalue_node;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = names;
	return new;

try_at_expr:
	at = start;

	puts("trying to recognize at term lvalue...");

	r = terminal(at_token, at, tokens, count);
	if (not r) goto error;
	at += r;

	struct tree_node* subexpr = term(at, tokens, count);
	if (not subexpr) goto error;
	at = subexpr->end;

	new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = deref_node;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = subexpr;
	return new;
error:
	return NULL;	
}

static struct tree_node* sum_expression(nat at, struct token* tokens, nat count);

//term:
//	number
//	name_list
//	# name_list
//	( sub_expression )
//	at term



static struct tree_node* term(nat at, struct token* tokens, nat count) { 

	printf("in %s(%llu)...\n", __func__, at);
	nat start = at;
	puts("trying to recognize number...");

	nat r = terminal(number_token, at, tokens, count);
	if (not r) goto try_name_list;
	at += r;

	struct tree_node* new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = number_node;
	return new;

try_name_list:
	at = start;

	puts("trying to recognize rvalue name list...");

	struct tree_node* names = name_list(at, tokens, count);
	if (not names) goto try_addr_name_list;
	at = names->end;

	new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = name_node;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = names;
	return new;


try_addr_name_list:
	at = start;

	puts("trying to recognize address name list...");

	r = terminal(pound_token, at, tokens, count);
	if (not r) goto try_parens;
	at += r;

	names = name_list(at, tokens, count);
	if (not names) goto try_parens;
	at = names->end;

	new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = address_node;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = names;
	return new;

try_parens:
	at = start;

	puts("trying to recognize parens...");

	r = terminal(open_paren_token, at, tokens, count);
	if (not r) goto try_at_expr;
	at += r;

	struct tree_node* subexpr = sum_expression(at, tokens, count);
	if (not subexpr) goto try_at_expr;
	at = subexpr->end;

	r = terminal(close_paren_token, at, tokens, count);
	if (not r) goto try_at_expr;
	at += r;

	new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = parens_node;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = subexpr;
	return new;

try_at_expr:
	at = start;

	puts("trying to recognize at term lvalue...");

	r = terminal(at_token, at, tokens, count);
	if (not r) goto error;
	at += r;

	subexpr = term(at, tokens, count);
	if (not subexpr) goto error;
	at = subexpr->end;

	new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = deref_node;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = subexpr;
	return new;

error:
	return NULL;	
}


//nor_expression:
//	term [* term]...

static struct tree_node* nor_expression(nat at, struct token* tokens, nat count) {

	printf("in %s(%llu)...\n", __func__, at);
	nat start = at;
	puts("trying to recognize mul expression...");

	struct tree_node* new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->type = nor_node;
	
	struct tree_node* t = term(at, tokens, count);
	if (not t) goto error;
	at = t->end;

	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = t;

	nat save = at;

try_more_terms:;
	nat r = terminal(tilde_token, at, tokens, count);
	if (not r) goto done;
	at += r;

	t = term(at, tokens, count);
	if (not t) { at = save; goto done; }
	at = t->end;

	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = t;

done:	new->end = at;
	return new;
error:
	print_error("expected mul expression", "test.txt", tokens[start].location, 
			max_at < count ? tokens[max_at].location : file_length
	);
	return NULL;
}


//mul_expression:
//	nor_expression [* nor_expression]...
// 	


static struct tree_node* mul_expression(nat at, struct token* tokens, nat count) {

	printf("in %s(%llu)...\n", __func__, at);
	nat start = at;
	puts("trying to recognize mul expression...");

	struct tree_node* new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->type = product_node;
	
	struct tree_node* t = nor_expression(at, tokens, count);
	if (not t) goto error;
	at = t->end;

	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = t;

	nat save = at;

	nat r = terminal(slash_token, at, tokens, count);
	if (not r) goto try_mod;
	at += r;

	t = nor_expression(at, tokens, count);
	if (not t) { at = save; goto try_mod; }
	at = t->end;

	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = t;
	goto done;

try_mod:
	r = terminal(percent_token, at, tokens, count);
	if (not r) goto try_more_terms;
	at += r;

	t = nor_expression(at, tokens, count);
	if (not t) { at = save; goto try_more_terms; }
	at = t->end;

	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = t;
	goto done;

try_more_terms:;
	nat r = terminal(star_token, at, tokens, count);
	if (not r) goto done;
	at += r;

	t = nor_expression(at, tokens, count);
	if (not t) { at = save; goto done; }
	at = t->end;

	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = t;
	goto try_more_terms;

done:	new->end = at;
	return new;
error:
	print_error("expected mul expression", "test.txt", tokens[start].location, 
			max_at < count ? tokens[max_at].location : file_length
	);
	return NULL;
}

static struct tree_node* sum_expression(nat at, struct token* tokens, nat count) {

	printf("in %s(%llu)...\n", __func__, at);
	nat start = at;
	puts("trying to recognize sum expression...");

	struct tree_node* new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->type = sum_node;
	
	struct tree_node* t = mul_expression(at, tokens, count);
	if (not t) goto error;
	at = t->end;

	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = t;
	
	nat save = at;

	nat r = terminal(minus_token, at, tokens, count);
	if (not r) goto try_more_terms;
	at += r;

	t = mul_expression(at, tokens, count);
	if (not t) { at = save; goto try_more_terms; }
	at = t->end;

	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = t;
	goto done;


try_more_terms:;
	r = terminal(plus_token, at, tokens, count);
	if (not r) goto done;
	at += r;

	t = mul_expression(at, tokens, count);
	if (not t) { at = save; goto done; }
	at = t->end;

	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = t;
	goto try_more_terms;

done:	new->end = at;
	return new;
error:
	print_error("expected sum expression", "test.txt", tokens[start].location, 
			max_at < count ? tokens[max_at].location : file_length
	);
	return NULL;
}



//statement:
//	do name_list
//	nat name_list = sum_expression
//	set name_list = sum_expression

static struct tree_node* statement(nat at, struct token* tokens, nat count) {

	printf("in %s(%llu)...\n", __func__, at);
	nat start = at;
	puts("trying to recognize do statement...");

	nat r = terminal(do_token, at, tokens, count);
	if (not r) goto try_declaration;
	at += r;

	struct tree_node* names = name_list(at, tokens, count);
	if (not names) goto try_declaration;
	at = names->end;

	struct tree_node* new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = do_node;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = names;
	return new;

try_declaration: 
	at = start;
	puts("trying to recognize declaration statement...");

	r = terminal(nat_token, at, tokens, count);
	if (not r) goto try_assignment;
	at += r;

	names = name_list(at, tokens, count);
	if (not names) goto try_assignment;
	at = names->end;

	r = terminal(equal_token, at, tokens, count);
	if (not r) goto try_assignment;
	at += r;

	struct tree_node* value = sum_expression(at, tokens, count);
	if (not value) goto try_assignment;
	at = value->end;

	new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = declaration_node;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = names;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = value;
	return new;

try_assignment: 
	at = start;

	puts("trying to recognize assignment statement...");

	r = terminal(set_token, at, tokens, count);
	if (not r) goto try_assignment;
	at += r;

	struct tree_node* left = lvalue(at, tokens, count);
	if (not left) goto error;
	at = left->end;

	r = terminal(equal_token, at, tokens, count);
	if (not r) goto error;
	at += r;

	value = sum_expression(at, tokens, count);
	if (not value) goto error;
	at = value->end;

	new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = declaration_node;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = left;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = value;
	return new;
error:
	return NULL;
}

//labeled_statement:
//	( name_list ) statement
//	statement

static struct tree_node* labeled_statement(nat at, struct token* tokens, nat count) {

	printf("in %s(%llu)...\n", __func__, at);
	nat start = at;
	puts("trying to recognize labeled statement...");

	nat r = terminal(open_paren_token, at, tokens, count);
	if (not r) goto try_without;
	at += r;

	struct tree_node* names = name_list(at, tokens, count);
	if (not names) goto try_without;
	at = names->end;

	nat r = terminal(close_paren_token, at, tokens, count);
	if (not r) goto try_without;
	at += r;

	struct tree_node* s = statement(at, tokens, count);
	if (not s) goto try_without;
	at = s->end;

	struct tree_node* new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = labeled_statement_node;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = names;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = s;
	return new;

try_without: 
	at = start;
	puts("trying to recognize without-label labeled statement...");

	s = statement(at, tokens, count);
	if (not s) goto error;
	at = s->end;

	new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = labeled_statement_node;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = s;
	return new;
error:
	return NULL;
}

//statement_list:
//	[labeled_statement]...
//	

static struct tree_node* statement_list(nat at, struct token* tokens, nat count) {

	printf("in %s(%llu)...\n", __func__, at);

	struct tree_node* new = calloc(1, sizeof(struct tree_node));
	new->start = at;
	new->type = statement_list_node;

try_more:;
	struct tree_node* s = labeled_statement(at, tokens, count);
	if (not s) goto done;

	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = s;

	at = s->end;
	goto try_more;
done:;
	new->end = at;
	return new;
}

//block:
//	{ statement_list }
//	labeled_statement

static struct tree_node* block(nat at, struct token* tokens, nat count) {
	printf("in %s(%llu)...\n", __func__, at);
	nat start = at;
	puts("trying to recognize curly statement list...");

	nat r = terminal(open_curl_token, at, tokens, count);
	if (not r) goto try_single;
	at += r;

	struct tree_node* list = statement_list(at, tokens, count);
	if (not list) goto try_single;
	at = list->end;

	r = terminal(close_curl_token, at, tokens, count);
	if (not r) goto try_single;
	at += r;

	struct tree_node* new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = block_node;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = list;
	return new;

try_single: 
	at = start;
	puts("trying to recognize a single statement...");

	struct tree_node* s = labeled_statement(at, tokens, count);
	if (not s) goto error;
	at = s->end;

	new = calloc(1, sizeof(struct tree_node));
	new->start = start;
	new->end = at;
	new->type = block_node;
	new->children = realloc(new->children, sizeof(struct tree_node*) * (new->count + 1));
	new->children[new->count++] = s;
	return new;

error:
	print_error("expected block", "test.txt", tokens[start].location, 
			max_at < count ? tokens[max_at].location : file_length
	);
	return NULL;
}

static struct tree_node* parse(nat at, struct token* tokens, nat count) {
	printf("in %s(%llu)...\n", __func__, at);
	return block(at, tokens, count);
}

static void print_syntax_tree(struct tree_node* this, nat depth) {

	if (not this) { printf("node[(pointer is null)]\n\n"); return; } 

	for (nat i = 0; i < depth; i++) printf(".\t");
	printf("node[.type=\033[38;5;%dm%s\033[0m, .start=%llu, .end=%llu, .count=%llu]\n\n", 
		colors[this->type], node_spelling[this->type], this->start, this->end, this->count
	);
	
	for (nat i = 0; i < this->count; i++) {
		print_syntax_tree(this->children[i], depth + 1);
	}
}

int main(void) {

	char* text = strdup(
	"{ \
	nat hello = ((001 + 0) * 0001) 		\
	@hello = hello + 1 			\
	@(1010101) = #hello + 00010101 			\
	my name: do my name 			\
 	} "
	);

	const nat text_length = strlen(text);
	struct token* tokens = NULL;
	const nat token_count = lex(&tokens, text, text_length);
	print_tokens(tokens, token_count);

	file_text = text; file_length = text_length;
	struct tree_node* tree = parse(0, tokens, token_count);
	if (tree and tree->end != token_count) {
		print_error("input not fully used", "test.txt", tokens[tree->start].location, 
			max_at < token_count ? tokens[max_at].location : file_length
		);
	} else {
		puts("success: all tokens have been consumed!");
	}
	puts("AST:");
	print_syntax_tree(tree, 0);
}





/*

print_error("expected statement", "test.txt", tokens[start].location, 
		max_at < count ? tokens[max_at].location : file_length
);








here are the valid statements in the language:


"statement_list" entity must be recognized at the top level. 


statement_list:
	statement...

name_list:
	name...

code:
	{ statement_list }
	
	statement


statement:
	name_list : raw_statement

	raw_statement


raw_statement:
	repeat code while condition

	if expression then code else code

	if expression then code

	do name_list

	nat name_list = slt_expression

	lvalue = slt_expression

slt_expression:
	sum_expression < sum_expression
	sum_expression

sum_expression:
	mul_expression + mul_expression + mul_expression ...
	mul_expression - mul_expression
	mul_expression

mul_expression:
	nor_expression * nor_expression * nor_expression ...
	nor_expression / nor_expression
	nor_expression % nor_expression
	nor_expression

nor_expression
	term ~ term
	term

term:
	number
	( slt_expression )
	name_list
	% name_list                                             <--- address_of operator.

lvalue:
	* sum_expression
	name_list
	




*/


// text file for testing lexer:
//"nat=while (hello) {nat stuff = 001}"









			/*if (false) {
				printf("nat keyword_length = %llu...\n", keyword_length);
				printf("not strncmp(text + i, keywords[k], keyword_length) = %d\n", not strncmp(text + i, keywords[k], keyword_length));
				printf("i + keyword_length >= length = %d\n", i + keyword_length >= length);
				printf("text[i + keyword_length] = %c\n", text[i + keyword_length]);
				printf("is_delimiter(text[i + keyword_length]) = %d\n", is_delimiter(text[i + keyword_length]));
				getchar();
			}*/





		//else {
			//printf("character %c is not an operator...\n", text[i]);
		//}






			//printf("error state: at %llu (%c)...\n", i, text[i]);
			//print_string_index(text, length, i);
			//
		//}





