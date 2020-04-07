#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

struct expression {
    size_t value; // a token value, ie a character.
    size_t count;
    struct expression* symbols;
};


///TODO: get rid of me!
static size_t text_index = 0;
///TODO: must be set to 0 before each call to parse.




/**
 static e p(t &s, F &f) {
     e l{};
     t S = s;
     t t = n(s, f);
     while (t.v && t.v != ')') {
         if (t.v == '(') {
             e e = p(s, f);
             if (n(s, f).v != ')')
                 printf("n3zqx2l: %s:%ld:%ld: error: expected )\n\n",
                        f.n, t.l, t.c);
             e.t.l = t.l, e.t.c = t.c, e.t.v = 0;
             l.s.push_back(e);
         } else l.s.push_back({{}, t});
         if (t.v) t = n(S = s, f);
     }
     s = S;
     return l;
 }
 
 */





struct expression parse(const char* name, const char* text) {
    struct expression list = {0};
    
    while (isspace(text[text_index++]));
    size_t token = text[text_index++];
    
    while (token && token != ')') {
        if (token == '(') {
            struct expression expr = parse(text, name);
            while (isspace(text[text_index++]));
            
            if (text[text_index++] != ')')
                printf("n3zqx2l: %s: error: expected )\n\n", name);
            
            expr.value = 0;
            list.symbols = realloc(list.symbols, sizeof(struct expression) * (list.count));
            list.symbols[list.count++] = expr;
            
        } else {
            //        l.s.push_back({{}, t});
            list.symbols = realloc(list.symbols, sizeof(struct expression) * (list.count));
            list.symbols[list.count++] = (struct expression){token, 0, NULL};
        }
        
//            t = n(S = s, f);
        while (isspace(text[text_index++]));
        token = text[text_index++];
                
    }
    return (struct expression){0, 0, NULL};
}

///TODO: look up the oroginal code for parsing LISP s-expressions.








char* open_file(const char* filename) {
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("n: %s: error: %s\n", filename, strerror(errno));
        return NULL;
    }
    fseek(file, 0, 2);
    const size_t length = ftell(file);
    char* buffer = (char*) calloc(length + 1, sizeof(char));
    fseek(file, 0, 0);
    fread(buffer, sizeof(char), length, file);
    fclose(file);
    return buffer;
}

static void print_expression(struct expression e) {
    if (e.value) printf("%c", (char) e.value);
    else {
        printf("(");
        for (size_t i = 0; i < e.count; i++) {
            print_expression(e.symbols[i]);
        }
        printf(")");
    }
}

int main(int argc, const char** argv) {
    
    const char* filename = "/Users/deniylreimn/Documents/art/c/projects/n/n/test.n";
    const char* text = open_file(filename);
    puts(text);
    struct expression ast = parse(filename, text);
    print_expression(ast);
    puts("");
}

