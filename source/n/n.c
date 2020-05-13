#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <llvm-c/ExecutionEngine.h>
#include <stdio.h>
#include <stdlib.h>   // n3zqx2l
#include <string.h>

struct res {
    size_t index;
    size_t count;
    size_t total;
    struct res* args;
};

struct ent {
    size_t type;
    size_t count;
    size_t* sig;
    uint32_t codegen_as;
    uint32_t precedence;
};

struct context {
    size_t best;
    size_t frame_count;
    size_t index_count;
    size_t name_count;
    size_t* frames;
    size_t* indicies;
    struct ent* names;
    struct ent* owners;
};

static void represent(size_t given, char* buffer, size_t limit, size_t* at, struct context* context) {
    if (given < 256) { buffer[(*at)++] = given; return; } else given -= 256;
    if (given >= context->name_count) return;
    struct ent s = context->names[given];
    for (size_t i = 0; i < s.count; i++) represent(s.sig[i], buffer, limit, at, context);
    if (!s.type) return; buffer[(*at)++] = '\n';
    represent(s.type, buffer, limit, at, context);
}

void print_source_code(uint8_t* source, size_t length, uint16_t line, uint16_t column) {
    const ssize_t target = (ssize_t) line - 1;
    char* copy = strndup((char*)source, length), *text = copy;
    for (ssize_t at = 0; text; at++) {
        char* line = strsep(&text, "\n");
        if (at > target + 2) break; else if (at < target - 2) continue;
        printf("   \x1B[90m%5lu\x1B[0m\x1B[32m  │  \x1B[0m%s\n", at + 1, line);
        if (at == target) printf("%*c\x1B[091m^\x1B[0m\n", column + 12, ' ');
    } puts("\n"); free(copy);
}

static void parse_error(uint8_t* given, size_t given_count, uint16_t* lines, uint16_t* columns, size_t type, struct context* C, const char* filename, uint8_t* text, size_t length) {
    char type_string[4096] = {0}; size_t index = 0;
    represent(type, type_string, sizeof type_string,  &index, C);
    if (!given_count) { printf("n3zqx2l: %s: \x1B[091merror:\x1B[0m %s: unresolved empty expression\n\n", filename, type_string); return; }
    else if (C->best == given_count) C->best--;
    printf("n3zqx2l: %s:%u:%u: \x1B[091merror:\x1B[0m %s: unresolved %c\n\n", filename, lines[C->best], columns[C->best], type_string, given[C->best]);
    print_source_code(text, length, lines[C->best], columns[C->best]);
}

static struct res parse(uint8_t* given, size_t begin, size_t end, size_t type, size_t max_depth, struct context* C) {
    for (size_t depth = 0; depth <= max_depth; depth++) {
        for (size_t i = C->index_count; i--;) {
            
            struct res sol = {C->indicies[i], 0, 0, 0};
            struct ent name = C->names[sol.index];
            
            if (type && name.type != type) goto next;
            
            for (uint8_t s = 0; s < name.count; s++) {
                if (begin + sol.total >= end) goto next;
                if (name.sig[s] >= 256) {
                    struct res arg = parse(given, begin + sol.total, end, C->names[name.sig[s] - 256].type, depth + 1, C);
                    if (!arg.index) goto next;
                    sol.total += arg.total;
                    sol.args = realloc(sol.args, sizeof(struct res) * (sol.count + 1));
                    sol.args[sol.count++] = arg;
                } else if (name.sig[s] == given[begin + sol.total]) sol.total++; else goto next;
            }
            return sol; next: C->best = fmax(begin + sol.total, C->best);
        }
    } return (struct res) {0};
}

static void debug_context(struct context* context) {
    printf("\n[best = %lu]\n", context->best);
    printf("---- debugging frames: ----\n");
    for (size_t i = 0; i < context->frame_count; i++)
        printf("\tframe # %lu  bp = %lu\n", i, context->frames[i]);
    printf("\n---- debugging indicies: ----\n");
    printf("\t\tidxs: { ");
    for (size_t i = 0; i < context->index_count; i++)
        printf("%lu ", context->indicies[i]);
    printf("}\n\n----- master: ------ \n{\n");
    for (size_t i = 0; i < context->name_count; i++) {
        char buffer[4096] = {0}; size_t index = 0;
        represent(i, buffer, sizeof buffer, &index, context);
        printf("\t%6lu: %s\n\n", i, buffer);
    } puts("}\n");
}

static void debug_resolved(struct res given, size_t depth, struct context* context) {
    char buffer[4096] = {0}; size_t index = 0;
    represent(given.index, buffer, sizeof buffer, &index, context);
    for (size_t i = depth; i--;) printf(".   ");
    printf("%s:%lu:%lu:%c\n", buffer, given.index, given.total, (char) given.index);
    for (size_t i = 0; i < given.count; i++) debug_resolved(given.args[i], depth + 1, context);
    puts("");
}

int main(int argc, char** argv) {
    if (argc == 1) return !printf("n: \x1B[091merror:\x1B[0m no input files\n");
    for (int i = 1; i < argc; i++) {
        
        if (argv[i][0] == '-') return !puts("n: 0.0.1\nn [-v] [files]");
        
        struct context C = {0};
//            0, 1, 0, 0,
//            calloc(1, sizeof(size_t)), calloc(0, sizeof(size_t)),
//            calloc(0, sizeof(struct ent)), calloc(1, sizeof(struct ent)),
//        };
        
        printf("parsing...\n");
        debug_context(&C);
        
        FILE* file = fopen(argv[i], "r");
        if (!file) {
            fprintf(stderr, "n: %s: ", argv[i]);
            perror("error");
            continue;
        }
        
        fseek(file, 0, SEEK_END);
        size_t L = ftell(file), count = 0;
        uint8_t *text = malloc(L), *tokens = malloc(L);
        uint16_t *lines = malloc(L*2), *columns = malloc(L*2);
        fseek(file, 0, SEEK_SET);
        fread(text, 1, L, file);
        fclose(file);
        
        uint16_t l = 1, c = 1;
        for (size_t i = 0; i < L; i++) {
            if (text[i] > 32) { lines[count] = l; columns[count] = c; tokens[count++] = text[i]; }
            if (text[i] == 10) { l++; c = 1; } else c++;
        }
        
        struct res e = parse(tokens, 0, count, 256, 5, &C);
        if (!e.index) parse_error(tokens, count, lines, columns, 256, &C, argv[i], text, L);
        debug_resolved(e, 0, &C);
        debug_context(&C);
        free(tokens);
        free(text);
    }
}




















































//        memcpy(C.indicies, (size_t[]) {_j,_U,_s,_c,_i,_n,_a,_p,_d,_k}, 10 * sizeof(size_t));
//        C.names[_U] = (struct ent) {{'U'}, _U, 1, 0, 0};
//        C.names[_i] = (struct ent) {{'i'}, _U, 1, 0, 0};
//        C.names[_n] = (struct ent) {{'n'}, _i, 1, 0, 0};
//        C.names[_s] = (struct ent) {{'s'}, _i, 1, 0, 0};
//        C.names[_a0] = (struct ent) {{'0'}, _s, 1, 0, 0};
//        C.names[_a] = (struct ent) {{'a', 256+_a0}, _n, 2, 0, 0};
//        C.names[_p0] = (struct ent) {{'0'}, _i, 1, 0, 0};
//        C.names[_p] = (struct ent) {{'p', 256+_p0}, _n, 2, 0, 0};
//        C.names[_d0] = (struct ent) {{'0'}, _n, 1, 0, 0};
//        C.names[_d1] = (struct ent) {{'0'}, _U, 1, 0, 0};
//        C.names[_d] = (struct ent) {{'d', 256+_d0, 256+_d1}, _i, 3, 0, 0};
//        C.names[_k0] = (struct ent) {{'0', 0}, _n, 1, 0, 0};
//        C.names[_k1] = (struct ent) {{'0', 0}, _n, 1, 0, 0};
//        C.names[_k] = (struct ent) {{'k', 256+_k0, 256+_k1}, _n, 3, 0, 0};
//        C.names[_j0] = (struct ent) {{'0', 0}, _i, 1, 0, 0};
//        C.names[_j1] = (struct ent) {{'0', 0}, _i, 1, 0, 0};
//        C.names[_j] = (struct ent) {{/*'j',*/ 256+_j0, 256+_j1}, _i, 2, 0, 65536};
        
//enum { _U = 256, _i, _n, _s, _c, _a0, _a, _p0, _p, _d0, _d1, _d, _k0, _k1, _k, _j0, _j1, _j, _intrin_count };
//enum { _cg_default, _cg_none, _cg_macro, _cg_function, _cg_namespace, _cg_variable, _cg_structure };
