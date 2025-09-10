// a simple c-like language compiler / byte-code interpreter, just for fun.
// written by dwrr  on 202403273.011738
// rewritten on 1202509092.005051 by dwrr

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

static const char* keywords[] = {
	"eoi", 
	"at", "do", "while", "repeat", 
	"if", "then", "else", "end", 
	"set", "define",
};

static const nat keyword_count = sizeof keywords / sizeof(char*);

static const char* operators = "<=+-*/%&|~^!()@";

enum token_type {
	null_token, number_token, name_token,

	eoi_token, 
	at_token, do_token, while_token, repeat_token, 
	if_token, then_token, else_token, end_token, 
	set_token, define_token,

	less_token, equal_token,
	add_token, subtract_token, 
	multiply_token, divide_token, remainder_token, 
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

	"less_token", "equal_token", 
	"add_token", "subtract_token", 
	"multiply_token", "divide_token", "remainder_token", 
	"and_token", "or_token", "eor_token",
	"shiftup_token", "shiftdown_token", 
	"open_paren_token", "close_paren_token", 
	"deref_token", 
	
};

enum node_type {
	null_node, 
	name_node, 
	identifier_node,
	number_node, 
	sum_node, 
	product_node, 
	subtraction_node,
	division_node,
	remainder_node,
	load_node,
	at_node,
	do_node,
	define_node,
	assignment_node,
	store_node,
	expression_node, 
	if_node,
	repeat_node,
	statement_node,
	statement_list_node, 
	node_type_count
};

static const char* node_spelling[node_type_count] = {
	"null_node", 
	"name_node", 
	"identifier_node",
	"number_node", 
	"sum_node", 
	"product_node", 
	"subtraction_node",
	"division_node",
	"remainder_node",
	"load_node",
	"at_node",
	"do_node",
	"define_node",
	"assignment_node",
	"store_node",
	"expression_node", 
	"if_node",
	"repeat_node",
	"statement_node",
	"statement_list_node", 
};

struct token {
	nat type;
	nat value;
	nat location;
	char* name;
};

struct node {
	nat type;
	nat count;
	nat begin;
	nat end;
	nat* children;
};


static const nat error = (nat) -1;

static nat text_length = 0;
static char* text = NULL;

static struct token* tokens = NULL;
static nat token_count = 0;

static nat max_at = 0;

static nat node_count = 0;
static struct node nodes[65536] = {0};
static nat pointers[65536] = {0};
static nat* heap = pointers;


static bool is_delimiter(char c) { 
	return isspace(c) or strchr(operators, c);
}

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
	char* s = calloc(*length + 1, 1);
	read(file, s, *length);
	close(file);
	return s;
}

static void print_tokens(void) {	
	printf("printing all tokens: (%llu tokens): {\n\n", token_count);
	for (nat i = 0; i < token_count; i++) {
		printf("#%llu: @%-6llu: token(.type = %-20s ",
			i, tokens[i].location, 
			token_spelling[tokens[i].type]
		);
		if (tokens[i].type == name_token) 
			printf(".name = \"%s\" ", tokens[i].name);
		if (tokens[i].type == number_token) 
			printf(".value = %llu ", tokens[i].value);
		puts("\n");
	}
	puts("}");
}

static void print_nats(nat* array, nat count) {
	printf("(%llu) { ", count);
	for (nat i = 0; i < count; i++) {
		printf("%lld ", array[i]);
	}
	printf("}");
}


static void print_nodes(void) {
	puts("nodes:");
	for (nat i = 0; i < node_count; i++) {
		printf("#%llu: node[.type=%s, .begin = %llu, .end = %llu, .count=%llu, .children = {",
			i, node_spelling[nodes[i].type], 
			nodes[i].begin,
			nodes[i].end,
			nodes[i].count
		);
		print_nats(nodes[i].children, nodes[i].count);
		puts("} ];");
	}
	puts("");
}



static void print_syntax_tree(nat this, nat depth) {
	for (nat i = 0; i < depth; i++) printf(".\t");
	printf("node[.type=%s, .count=%llu]\n\n",
		node_spelling[nodes[this].type], 
		nodes[this].count
	);
	for (nat i = 0; i < nodes[this].count; i++) 
		print_syntax_tree(nodes[this].children[i], depth + 1);
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


static void print_token_position(nat at) {

	puts("token position:");
	for (nat i = 0; i < token_count; i++) {
		if (i % 3 == 0) puts("");
		if (i == at) printf("\033[32;1m");
		printf("  %20s  ", token_spelling[tokens[i].type]);
		if (i == at) printf("\033[0m");

	}
	puts("\n");
}


static nat terminal(nat type, nat at) {
	printf("in %s(%s, %llu)...\n", __func__, token_spelling[type], at);
	print_token_position(at);
	if (at > max_at) max_at = at;
	if (at == token_count) return error;
	if (tokens[at].type != type) return error;
	return at + 1;
}

static nat identifier(nat at) {
	printf("in %s(%llu)...\n", __func__, at);
	print_token_position(at);

	nat begin = at;
	nat r = terminal(name_token, at);
	if (r == error) return error;
	at = r;
more:
	r = terminal(name_token, at);
	if (r == error) goto done;
	at = r; goto more;
done:
	nodes[node_count++] = (struct node) {
		.type = identifier_node, 
		.begin = begin, 
		.end = at
	};
	return node_count - 1;
}

static nat number(nat at) {
	printf("in %s(%llu)...\n", __func__, at);
	print_token_position(at);

	nat begin = at;
	nat r = terminal(number_token, at);
	if (r == error) return error;
	at = r;

	nodes[node_count++] = (struct node) {
		.type = number_node, 
		.begin = begin, 
		.end = at
	};
	return node_count - 1;
}



static nat expression(nat at);

static nat binary_expression(nat at) {
	printf("in %s(%llu)...\n", __func__, at);
	print_token_position(at);

	nat begin = at, e = 0, f = 0, op = 0, r = 0;

	e = expression(at);
	if (e == error) return error;
	at = nodes[e].end;

	op = add_token; 	r = terminal(op, at); if (r != error) { at = r; goto second; } 
	op = subtract_token; 	r = terminal(op, at); if (r != error) { at = r; goto second; } 
	op = multiply_token; 	r = terminal(op, at); if (r != error) { at = r; goto second; } 
	op = divide_token; 	r = terminal(op, at); if (r != error) { at = r; goto second; } 
	op = remainder_token; 	r = terminal(op, at); if (r != error) { at = r; goto second; } 
	op = less_token; 	r = terminal(op, at); if (r != error) { at = r; goto second; } 
	op = equal_token; 	r = terminal(op, at); if (r != error) { at = r; goto second; } 

	return error;

second:
	f = expression(at);
	if (f == error) return error;
	at = nodes[f].end;

	heap[0] = op;
	heap[1] = e;
	heap[2] = f;
	nodes[node_count++] = (struct node) {
		.type = expression_node, 
		.begin = begin, 
		.end = at,
		.count = 3,
		.children = heap,
	};
	heap += 3;
	return node_count - 1;
}


static nat expression(nat at) {
	printf("in %s(%llu)...\n", __func__, at);
	print_token_position(at);

	nat begin = at, e = 0;

	nat r = terminal(open_paren_token, at);
	if (r == error) goto next;
	at = r;

	e = binary_expression(at);
	if (e != error) goto next;
	at = nodes[e].end;

	r = terminal(close_paren_token, at);
	if (r == error) return error;
	at = r;
	goto success;

next:
	e = identifier(at);
	if (e != error) {
		at = nodes[e].end;
		goto success;
	}

	e = number(at);
	if (e != error) {
		at = nodes[e].end;
		goto success;
	}

	return error;

success:
	heap[0] = e;
	nodes[node_count++] = (struct node) {
		.type = expression_node, 
		.begin = begin, 
		.end = at,
		.count = 1,
		.children = heap,
	};
	heap += 1;
	return node_count - 1;
}

static nat assignment_statement(nat at) {
	printf("in %s(%llu)...\n", __func__, at);
	print_token_position(at);

	nat begin = at;
	nat r = terminal(set_token, at);
	if (r == error) return error;
	at = r;

	nat destination = identifier(at);
	if (destination == error) return error;
	at = nodes[destination].end;

	r = terminal(equal_token, at);
	if (r == error) return error;
	at = r;

	nat source = expression(at);
	if (source == error) return error;
	at = nodes[source].end;	

	heap[0] = destination;
	heap[1] = source;

	nodes[node_count++] = (struct node) {
		.type = assignment_node, 
		.begin = begin, 
		.end = at,
		.count = 2,
		.children = heap,
	};
	heap += 2;
	return node_count - 1;
}

static nat define_statement(nat at) {
	printf("in %s(%llu)...\n", __func__, at);
	print_token_position(at);

	nat begin = at;
	nat r = terminal(define_token, at);
	if (r == error) return error;
	at = r;

	nat destination = identifier(at);
	if (destination == error) return error;
	at = nodes[destination].end;

	r = terminal(equal_token, at);
	if (r == error) return error;
	at = r;

	nat source = expression(at);
	if (source == error) return error;
	at = nodes[source].end;	

	heap[0] = destination;
	heap[1] = source;

	nodes[node_count++] = (struct node) {
		.type = define_node, 
		.begin = begin, 
		.end = at,
		.count = 2,
		.children = heap,
	};
	heap += 2;
	return node_count - 1;
}

static nat statement_list(nat at);

static nat repeat_statement(nat at) {
	printf("in %s(%llu)...\n", __func__, at);
	print_token_position(at);

	nat begin = at;

	nat r = terminal(repeat_token, at);
	if (r == error) return error;
	at = r;

	nat block = statement_list(at);
	if (block == error) return error;
	at = nodes[block].end;

	r = terminal(while_token, at);
	if (r == error) return error;
	at = r;

	nat condition = expression(at);
	if (condition == error) return error;
	at = nodes[condition].end;	

	heap[0] = block;
	heap[1] = condition;
	nodes[node_count++] = (struct node) {
		.type = repeat_node, 
		.begin = begin, 
		.end = at,
		.count = 2,
		.children = heap,
	};
	heap += 2;
	return node_count - 1;
}



static nat statement(nat at) {
	printf("in %s(%llu)...\n", __func__, at);
	print_token_position(at);

	nat begin = at, s = 0;

	s = repeat_statement(at);
	if (s != error) {
		at = nodes[s].end;
		goto success;	
	}

	s = define_statement(at);
	if (s != error) {	
		at = nodes[s].end;
		goto success;
	}

	s = assignment_statement(at);
	if (s != error) {	
		at = nodes[s].end;
		goto success;
	}

	return error;

success:
	heap[0] = s;
	nodes[node_count++] = (struct node) {
		.type = statement_node, 
		.begin = begin, 
		.end = at,
		.count = 1,
		.children = heap,
	};
	heap += 1;
	return node_count - 1;	
}

static nat statement_list(nat at) {
	printf("in %s(%llu)...\n", __func__, at);
	print_token_position(at);

	nat array[4096] = {0};
	nat begin = at, count = 0;
next:;
	nat s = statement(at);
	if (s == error) goto done;
	at = nodes[s].end;
	array[count++] = s;
	goto next;
done:
	for (nat i = 0; i < count; i++) {
		heap[i] = array[i];
	}
	nodes[node_count++] = (struct node) {
		.type = statement_list_node, 
		.begin = begin, 
		.end = at,
		.count = count,
		.children = heap
	};
	heap += count;
	return node_count - 1;
}



int main(int argc, const char** argv) {
	if (argc != 2) return puts("compiler frontend: error: no input file given");
	text = load_file(argv[1], &text_length);
	
	for (nat i = 0; i < text_length; ) {
		if (text[i] == '[') {
			bool comment = 1;
			while (comment and i < text_length) {
				if (text[i] == '[') comment++;
				if (text[i] == ']') comment--;
				i++;
			}
			if (comment) printf("syntax error: unterminated comment\n");
			continue;
		}

		printf("processing character text[%llu], \"%c\"(%d)...\n", i, text[i], text[i]);
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
			if (i + keyword_length <= text_length and not strncmp(text + i, keywords[k], keyword_length) and 
			    (is_delimiter(text[i + keyword_length]))
			) {
				tokens = realloc(tokens, sizeof(struct token) * (token_count + 1));
				tokens[token_count++] = (struct token) { .type = eoi_token + k, .location = i };
				i += keyword_length;
				goto next_char;
			}
		}

		nat start = i;
		nat t = i;
		while (not is_delimiter(text[t])) t++;
		const nat end = t;
		if (end == start) abort();

		char* string = strndup(text + start, end - start);
		nat string_length = strlen(string);

		nat r = 0, s = 1;
		for (nat j = 0; j < string_length; j++) {
			if (string[j] == '0') s <<= 1; 
			else if (string[j] == '1') { r += s; s <<= 1; }
			else goto push_name;
		}	

		tokens = realloc(tokens, sizeof(struct token) * (token_count + 1));
		tokens[token_count++] = (struct token) { 
			.type = number_token,
			.value = r,
			.location = i,
		};

		i += string_length;
		goto next_char;
	push_name:
		tokens = realloc(tokens, sizeof(struct token) * (token_count + 1));
		tokens[token_count++] = (struct token) { 
			.type = name_token,
			.name = string,
			.location = i,
		};	
		i += string_length;
		next_char:;
	}
	print_tokens();

	const nat root = statement_list(0);
	if (root == error) {
		printf("compiler: syntax parsing error: error at token: %llu\n", max_at);
		if (max_at < token_count) print_string_index(tokens[max_at].location);
		else puts("[could not print error token position...]");
		abort();
	}

	if (nodes[root].end != token_count) {
		printf("compiler: syntax parsing error: error at token: %llu\n", max_at);
		if (max_at < token_count) print_string_index(tokens[max_at].location);
		else puts("[could not print error token position...]");
		abort();
	}

	print_syntax_tree(root, 0);
	puts("");
	print_nodes();
	puts("");
	printf("root = %llu\n", root);
	puts("");
	print_nats(pointers, heap + 10 - pointers);
	puts("");
}
































































/**/


















/*


static struct tree_node* term(nat at, struct token* tokens, nat count);

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

	r = terminal(name_token, at, tokens, count);
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



static struct tree_node* statement_list(nat at, struct token* tokens, nat count) {
	printf("in %s(%llu)...\n", __func__, at);

	nat start = at;

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

*/





















/*


static void print_error(const char* reason, const char* filename, nat offset, nat max) {
	printf("compiler: %s:%llu:%llu: \033[31;1merror:\033[0m \033[1m%s\033[0\n", 
		filename, offset, max, reason
	);
	print_string_index(file_text, file_length, max);
}


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

*/




/*	char* text = strdup(
	"{ \
	nat hello = ((001 + 0) * 0001) 		\
	@hello = hello + 1 			\
	@(1010101) = #hello + 00010101 			\
	my name: do my name 			\
 	} "
	);
*/









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





