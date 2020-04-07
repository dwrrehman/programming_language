#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

struct e {
    size_t value; // a token value, ie a character.
    size_t c;
    struct e* s;
};


// 14 lines so far, we can do better!
/// now its 9 lines long. definiely better.       i think its near the optimal, i think.

static size_t i = 0; ///TODO: get rid of me!

struct e parse(const char* f, const char* t) {
    struct e e = {0};
    while (t[i] && t[i] != ')') {
        if (isspace(t[i++])) continue;
        e.s = realloc(e.s, sizeof(struct e) * (e.c + 1));
        if (t[i - 1] == '(') {
            e.s[e.c++] = parse(f, t);
            if (!t[i] || t[i++] != ')') printf("n3zqx2l: %s: error: expected )\n\n", f);
        } else e.s[e.c++].value = t[i - 1];
    } return e;
}

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

static void print_expression(struct e e) {
    if (e.value) printf("%c", (char) e.value);
    else {
        printf("(");
        for (size_t i = 0; i < e.c; i++) {
            print_expression(e.s[i]);
        }
        printf(")");
    }
}

int main(int argc, const char** argv) {
    const char* filename = "/Users/deniylreimn/Documents/art/c/projects/n/n/test.n";
    const char* text = open_file(filename);
    struct e ast = parse(filename, text);
    
    print_expression(ast); puts("");
        
    if (text[i]) {
        printf("error: unexpected closing paren\n");
        exit(1);    
    }
    
}

////eventually, ill need:
//#include <llvm-c/ExecutionEngine.h>
//#include <llvm-c/Target.h>
