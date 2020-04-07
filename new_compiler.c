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

static size_t i = 0; ///TODO: get rid of me!

struct expression parse(const char* name, const char* text) {
    struct expression list = {0};
    while (text[i]) {
        while (isspace(text[i])) i++;
        const size_t t = text[i++];
        if (!t || t == ')') break;
        struct expression e = {0};
        if (t == '(') {
            e = parse(name, text);
            while (isspace(text[i])) i++;
            if (!text[i] || text[i++] != ')') printf("n3zqx2l: %s: error: expected )\n\n", name);
        } else e.value = t;
        list.symbols = realloc(list.symbols, sizeof(struct expression) * (list.count + 1));
        list.symbols[list.count++] = e;
    } i--; return list;
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
    struct expression ast = parse(filename, text);
    print_expression(ast);
    puts("");
}

