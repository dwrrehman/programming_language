#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <llvm-c/ExecutionEngine.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    _U, _i, _n, _s, _c,
    _a0, _a, _p0, _p,
    _d0, _d1, _d,
    _k0, _k1, _k,
    _j0, _j1, _j,
    _intrin_count,
};

enum {
    _cg_default,
    _cg_none,
    _cg_macro,
    _cg_function,
    _cg_namespace,
    _cg_variable,
    _cg_structure
};

struct loc {
    uint16_t line;
    uint16_t column;
};

struct expr {
    size_t index;
    size_t count;
    struct expr* args;
    size_t value;
    size_t total;
};

struct name {
    size_t sig[256]; // i think we should make signatures unlimited length.
    size_t type;
    size_t count;
    size_t codegen_as;
    size_t precedence;
};

struct context {
    size_t best;
    size_t frame_count;
    size_t index_count;
    size_t name_count;
    size_t* frames;
    size_t* indicies;
    struct name* names;
    struct name* owners;
};

static void represent(size_t given, char* buffer, size_t limit, size_t* at, struct context* context) {
    if (given > context->name_count) return;
    struct name name = context->names[given];
    for (size_t i = 0; i < name.count; i++) {
        if (*at + 4 >= limit) return;
        if (name.sig[i] < 256) buffer[(*at)++] = name.sig[i];
        else {
            buffer[(*at)++] = '(';
            represent(name.sig[i] - 256, buffer, limit, at, context);
            buffer[(*at)++] = ')';
        }
    } if (name.type) {
        buffer[(*at)++] = ' ';
        represent(name.type, buffer, limit, at, context);
    }
}

void print_source_code(uint8_t* source, size_t length, struct loc t) {
    const ssize_t target = (ssize_t) t.line - 1;
    char* copy = strndup((char*)source, length), *text = copy;
    for (ssize_t at = 0; text; at++) {
        char* line = strsep(&text, "\n");
        if (at > target + 2) break; else if (at < target - 2) continue;
        printf("   \x1B[90m%5lu\x1B[0m\x1B[32m  │  \x1B[0m%s\n", at + 1, line);
        if (at == target) printf("%*c\x1B[091m^\x1B[0m\n", t.column + 12, ' ');
    } puts("\n"); free(copy);
}

static void parse_error(uint8_t* given, size_t given_count, struct loc* loc, size_t type, struct context* C, const char* filename, uint8_t* text, size_t length) {
    char type_string[4096] = {0}; size_t index = 0;
    represent(type, type_string, sizeof type_string,  &index, C);
    if (!given_count) { printf("n3zqx2l: %s: \x1B[091merror:\x1B[0m %s: unresolved empty expression\n\n", filename, type_string); return; }
    else if (C->best == given_count) C->best--;
    printf("n3zqx2l: %s:%u:%u: \x1B[091merror:\x1B[0m %s: unresolved %c\n\n", filename, loc[C->best].line, loc[C->best].column, type_string, given[C->best]);
    print_source_code(text, length, loc[C->best]);
}

static struct expr parse(uint8_t* given, size_t begin, size_t end, size_t type, size_t max_depth, struct context* C) {
    for (size_t depth = 0; depth <= max_depth; depth++) {
        for (size_t i = C->index_count; i--;) {
            struct expr sol = {C->indicies[i], 0, 0, 0, 0};
            struct name name = C->names[sol.index];
            if (type && name.type != type) goto next;
            for (uint8_t s = 0; s < name.count; s++) {
                if (begin + sol.total >= end) goto next;
                if (name.sig[s] >= 256) {
                    struct expr arg = parse(given, begin + sol.total, end, C->names[name.sig[s] - 256].type, depth + 1, C);
                    if (!arg.index) goto next;
                    sol.total += arg.total;
                    sol.args = realloc(sol.args, sizeof(struct expr) * (sol.count + 1));
                    sol.args[sol.count++] = arg;
                } else if (name.sig[s] == given[begin + sol.total]) sol.total++; else goto next;
            }
            return sol; next: C->best = fmax(begin + sol.total, C->best);
        }
    } return (struct expr) {0};
}

//                    for (size_t e = end + 1; e--;) {
//                        if (arg.index) break;
//                    }

//            if (begin + sol.total == end)

static void debug_context(struct context* context) {
    printf("[best = %lu]\n", context->best);
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
    } puts("}");
}

static void debug_resolved(struct expr given, size_t depth, struct context* context) {
    char buffer[4096] = {0}; size_t index = 0;
    represent(given.index, buffer, sizeof buffer, &index, context);
    for (size_t i = depth; i--;) printf(".   ");
    printf("%s : [%lu]{%lu}", buffer, given.index, given.total);
    if (given.value) printf(" : c=%c", (char) given.value);
    printf("\n");
    for (size_t i = 0; i < given.count; i++) debug_resolved(given.args[i], depth + 1, context);
    for (size_t i = depth; i--;) printf(".   ");
    printf("\n");
}

int main(int argc, const char** argv) {
    if (argc == 1) exit(!!printf("n: \x1B[091merror:\x1B[0m no input files\n"));
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') exit(!puts("n3zqx2l version 0.0.1\nn3zqx2l [-] [files]"));
        struct context C = {
            0, 1, 10, _intrin_count,
            calloc(1, sizeof(size_t)), calloc(10, sizeof(size_t)),
            calloc(_intrin_count, sizeof(struct name)), calloc(1, sizeof(struct name)),
        };
        memcpy(C.indicies, (size_t[]) {_j,_U,_s,_c,_i,_n,_a,_p,_d,_k}, 10 * sizeof(size_t));
        C.names[_U] = (struct name) {{'U'}, _U, 1, 0, 0};
        C.names[_i] = (struct name) {{'i'}, _U, 1, 0, 0};
        C.names[_n] = (struct name) {{'n'}, _i, 1, 0, 0};
        C.names[_s] = (struct name) {{'s'}, _i, 1, 0, 0};
        C.names[_c] = (struct name) {{'c'}, _s, 1, 0, 0};
        C.names[_a0] = (struct name) {{'0'}, _s, 1, 0, 0};
        C.names[_a] = (struct name) {{'a', 256+_a0}, _n, 2, 0, 0};
        C.names[_p0] = (struct name) {{'0'}, _i, 1, 0, 0};
        C.names[_p] = (struct name) {{'p', 256+_p0}, _n, 2, 0, 0};
        C.names[_d0] = (struct name) {{'0'}, _n, 1, 0, 0};
        C.names[_d1] = (struct name) {{'0'}, _U, 1, 0, 0};
        C.names[_d] = (struct name) {{'d', 256+_d0, 256+_d1}, _i, 3, 0, 0};
        C.names[_k0] = (struct name) {{'0', 0}, _n, 1, 0, 0};
        C.names[_k1] = (struct name) {{'0', 0}, _n, 1, 0, 0};
        C.names[_k] = (struct name) {{'k', 256+_k0, 256+_k1}, _n, 3, 0, 0};
        C.names[_j0] = (struct name) {{'0', 0}, _i, 1, 0, 0};
        C.names[_j1] = (struct name) {{'0', 0}, _i, 1, 0, 0};
        C.names[_j] = (struct name) {{/*'j',*/ 256+_j0, 256+_j1}, _i, 2, 0, 65536};
        FILE* file = fopen(argv[i], "r");
        if (!file) {
            fprintf(stderr, "n: %s: ", argv[i]);
            perror("error");
            continue;
        }
        fseek(file, 0, SEEK_END);
        size_t length = ftell(file), count = 0;
        uint8_t *text = malloc(length), *tokens = malloc(length);
        struct loc* loc = malloc(length * sizeof(struct loc));
        fseek(file, 0, SEEK_SET);
        fread(text, 1, length, file);
        fclose(file);
        
        uint16_t l = 1, c = 1;
        for (size_t i = 0; i < length; i++) {
            if (text[i] > 32) {
                tokens[count] = text[i];
                loc[count++] = (struct loc){l, c};
            } if (text[i] == 10){l++; c = 1;} else c++;
        }
        
        printf("parsing...\n");
        puts(""); debug_context(&C); puts("");
        
        struct expr e = parse(tokens, 0, count, _U, 5, &C);
        
        puts(""); debug_resolved(e, 0, &C);
        puts(""); debug_context(&C); puts("");
        if (!e.index) parse_error(tokens, count, loc, _U, &C, argv[i], text, length);
        free(tokens);
        free(text);
    }
}







































//static void duplicate_context(struct context* d, struct context* s) {
////    d->at = s->at;
//    d->name_count = s->name_count;
//    d->names = realloc(d->names, sizeof(struct frame) * s->name_count);
//    memcpy(d->names, s->names, sizeof(struct name) * s->name_count);
//    d->frame_count = s->frame_count;
//    d->frames = realloc(d->frames, sizeof(struct frame) * s->frame_count);
//    memcpy(d->frames, s->frames, sizeof(struct frame) * s->frame_count);
//}






//static void do_intrinsic(struct context *context, const struct expr *sol) {
//    if (sol->index == _d) {
//
//        size_t f = --context->frame_count;
//
//        context->frames[f].owner.type = sol->args[1].index;
//
//        context->names
//        = realloc(context->names,
//                  sizeof(struct name)
//                  * (context->name_count + 1));
//        context->names[context->name_count++]
//        = context->frames[f].owner;
//
//        size_t
//            m = context->frames[f].owner.count &&
//                context->frames[f].owner.sig[0] >= 256,
//            b = context->frame_count - 1;
//
//        context->frames[b].indicies
//            [context->frames[b].count++]
//            = context->name_count - 1;
//
//    } else if (sol->index == _a || sol->index == _p) {
//
//        struct name* owner =
//        &context->frames[context->frame_count - 1].owner;
//            owner->sig[owner->count++]
//                = sol->index == _a
//                    ? sol->args[0].value
//                    : 256 + context->name_count - 1;
//    }
//}




//                if (sol.index == _d) C->frame_count--;

//                do_intrinsic(C, &sol);


//                if (sol.index == _d) C->frames = realloc(C->frames, sizeof(struct frame) * ++C->frame_count);



//struct frame {
//    size_t indicies[256];   // i think we should make this:   size_t* indicies;
//    size_t count;
//    struct name owner;
//};
