#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <llvm-c/ExecutionEngine.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    _U,
    _i,
    _n,
    _s,
    _c,
    _a0, _a,
    _p0, _p,
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
    uint8_t value;
};

struct name {
    size_t sig[256];
    size_t type;
    uint8_t count;
    uint8_t codegen_as;
};

struct frame {
    size_t indicies[2][256];
    size_t count[2];
    struct name owner;
};

struct context {
    size_t at;
    size_t best;
    size_t name_count;
    size_t frame_count;
    struct name* names;
    struct frame* frames;
};

static void represent
(size_t given, char* buffer, size_t limit,
 size_t* at, struct context* context)
{
    if (given > context->name_count) return;
    
    struct name name = context->names[given];
    
    for (size_t i = 0; i < name.count; i++) {
        
        if (*at + 4 >= limit)
            return;
        
        else if (name.sig[i] < 256)
            buffer[(*at)++] = name.sig[i];
        
        else {
            buffer[(*at)++] = '(';
            represent(name.sig[i] - 256, buffer,
                      limit, at, context);
            buffer[(*at)++] = ')';
        }
    }
    if (name.type) {
        buffer[(*at)++] = ' ';
        represent(name.type, buffer,
                  limit, at, context);
    }
}



void save_of_print_source_code(uint8_t* source, size_t length, struct loc t) {
    
    ssize_t line_count = 0;
    char* text = strndup((char*)source, length), **lines = 0;
    while (text) {
        lines = realloc(lines, sizeof(char*) * (line_count + 1));
        lines[line_count++] = strsep(&text, "\n");
    }
    
    ssize_t offsets[] = {-2, -1, 0, 1, 2};
    for (size_t o = 0; o < 5; o++) {
        const ssize_t l = (ssize_t) t.line - 1 + offsets[o];
        if (l >= 0 && l < line_count) {
            printf("\t\x1B[90m%5lu\x1B[0m\x1B[32m  │  \x1B[0m%s\n", l + 1, lines[l]);
            if (!offsets[o]) {
                printf("\t");
                for (int i = 0; i < t.column + 9; i++) putchar(' ');
                printf("\x1B[091m^\x1B[0m\n");
            }
        }
    }
    puts("\n");
    free(lines);
}



void print_source_code(uint8_t* source, size_t length, struct loc t) {
    ssize_t at = 0, target = (ssize_t) t.line - 1;
    char* text = strndup((char*)source, length);
    while (text) {
        char* line = strsep(&text, "\n");
        if (at > target + 2 || at < target - 2) { at++; continue; }
        printf("\t\x1B[90m%5lu\x1B[0m\x1B[32m  │  \x1B[0m%s\n", at+1, line);
        if (at++ == target) {
            putchar('\t');
            for (int i = 0; i < t.column + 9; i++) putchar(' ');
            printf("\x1B[091m^\x1B[0m\n");
        }
    } puts("\n"); free(text);
}

static void parse_error
(uint8_t* given, size_t given_count,
 struct loc* loc, size_t type,
 struct context* C, const char* filename, uint8_t* text, size_t length) {
    
    char type_string[4096] = {0};
    size_t index = 0;
    represent(type, type_string,
              sizeof type_string,  &index, C);
    
    if (C->best == given_count && C->best) {
        C->best--;
        printf("n3zqx2l: %s:%u:%u: \x1B[091merror:\x1B[0m expected %s near %c\n\n",
               filename, loc[C->best].line, loc[C->best].column,
               type_string, given[C->best]);
        print_source_code(text, length, loc[C->best]);
    } else if (C->best < given_count) {
        printf("n3zqx2l: %s:%u:%u: \x1B[091merror:\x1B[0m %s: unresolved %c\n\n",
               filename, loc[C->best].line, loc[C->best].column,
               type_string, given[C->best]);
        print_source_code(text, length, loc[C->best]);
        
    } else
        printf("n3zqx2l: %s: \x1B[091merror:\x1B[0m %s: empty file\n\n",
                filename, type_string);
    
    
}

static void duplicate_context
 (struct context* d, struct context* s) {
    d->at = s->at;
    
    d->name_count = s->name_count;
    d->names = realloc(d->names,
                       sizeof(struct frame)
                       * s->name_count);
    
    memcpy(d->names, s->names,
           sizeof(struct name) * s->name_count);
    
    d->frame_count = s->frame_count;
    d->frames = realloc(d->frames,
                        sizeof(struct frame)
                        * s->frame_count);
    
    memcpy(d->frames, s->frames,
           sizeof(struct frame) * s->frame_count);
}

static void do_intrinsic(struct context *context, const struct expr *sol) {
    if (sol->index == _d) {
        
        size_t f = --context->frame_count;
        
        context->frames[f].owner.type = sol->args[1].index;
        
        context->names
        = realloc(context->names,
                  sizeof(struct name)
                  * (context->name_count + 1));
        context->names[context->name_count++]
        = context->frames[f].owner;
        
        size_t
            m = context->frames[f].owner.count &&
                context->frames[f].owner.sig[0] >= 256,
            b = context->frame_count - 1;
        
        context->frames[b].indicies[m]
            [context->frames[b].count[m]++]
            = context->name_count - 1;
        
    } else if (sol->index == _a || sol->index == _p) {
        
        struct name* owner =
        &context->frames[context->frame_count - 1].owner;
            owner->sig[owner->count++]
                = sol->index == _a
                    ? sol->args[0].value
                    : 256 + context->name_count - 1;
    }
}

static struct expr parse
 (uint8_t* given, size_t length, size_t type, size_t depth, struct context* C) {

    if (depth > 32) {
        return (struct expr) {0};
        
    } else if (type == _s) {
        return (struct expr) {_c, 0, 0, given[C->at++]};
    }
    
    struct context saved = {0};
    duplicate_context(&saved, C);
    
    for (size_t p = 2; p--;) {
        for (size_t f = C->frame_count; f--;) {
            for (size_t i = C->frames[f].count[p]; i--;) {
            
                C->best = fmax(C->at, C->best);
                duplicate_context(C, &saved);
                
                struct expr sol = {C->frames[f].indicies[p][i], 0, 0, 0};
                struct name name = C->names[sol.index];
                
                if (type && name.type != type) goto next;
                
                if (sol.index == _d) C->frames = realloc(C->frames, sizeof(struct frame) * ++C->frame_count);
                
                for (uint8_t s = 0; s < name.count; s++) {
                    if (C->at >= length) {
                        if (s == 1 && p == 1) return *sol.args;
                        else goto next;
                        
                    } else if (name.sig[s] >= 256) {
                        struct expr arg = parse(given, length, C->names[name.sig[s] - 256].type, depth + 1, C);
                        if (!arg.index) goto next;
                        sol.args = realloc(sol.args, sizeof(struct expr) * (sol.count + 1));
                        sol.args[sol.count++] = arg;
                    } else if (name.sig[s] == given[C->at]) C->at++; else goto next;
                }
                do_intrinsic(C, &sol);
                return sol;
                next: if (sol.index == _d) C->frame_count--;
            }
        }
    }
    return (struct expr) {0};
}







static void debug_context(struct context* context) {
    printf("index = %lu, best = %lu\n",
           context->at, context->best);
    
    printf("---- debugging frames: ----\n");
    
    for (size_t m = 0; m < 2; m++) {
        
        printf("mode = %lu\n", m);
        
        for (size_t i = 0; i < context->frame_count; i++) {
            
            printf("\t ----- [m=%lu] FRAME # %lu ---- \n"
                   "\t\tidxs: { ", m, i);
            
            for (size_t j = 0; j < context->frames[i].count[m]; j++)
                printf("%lu ", context->frames[i].indicies[m][j]);
            
            puts("}");
        }
    }
    printf("\nmaster: {\n");
    for (size_t i = 0; i < context->name_count; i++) {
        printf("\t%6lu: ", i);
        char buffer[2048] = {0};
        size_t index = 0;
        represent(i, buffer, sizeof buffer,
                  &index, context);
        printf("%s", buffer);
        puts("\n");
    }
    puts("}");
        
}

static void debug_resolved
(struct expr given, size_t depth, struct context* context) {
    
    char buffer[4096] = {0};
    size_t index = 0;
    
    represent(given.index, buffer, sizeof buffer, &index, context);
    
    for (size_t i = depth; i--;) printf(".   ");
    
    printf("%s : [%lu]", buffer, given.index);
    
    if (given.value)
        printf(" : c=%c", (char) given.value);
    printf("\n");
    
    for (size_t i = 0; i < given.count; i++)
        debug_resolved(given.args[i], depth + 1, context);
    
    for (size_t i = depth; i--;) printf(".   ");
    
    printf("\n");
}

int main(int argc, const char** argv) {
    
    for (int i = 1; i < argc; i++) {
        
        if (argv[i][0] == '-')
            exit(!puts("n3zqx2l version 0.0.0\nn3zqx2l [-] [files]"));
        
        const size_t o = 256;
        
        struct context C = {
            0, 0, _intrin_count, 1,
            calloc(_intrin_count, sizeof(struct name)),
            calloc(1, sizeof(struct frame))
        };
        
        C.frames[0] = (struct frame) {{{
            _U, _s, _c, _i, _n, _a, _p, _d, _k, _j
        },{0}}, {10, 0}, {0}};
        
        C.names[_U] = (struct name) {{'U'}, _U, 1, 0};
        C.names[_i] = (struct name) {{'i'}, _U, 1, 0};
        C.names[_n] = (struct name) {{'n'}, _i, 1, 0};
        C.names[_s] = (struct name) {{'s'}, _i, 1, 0};
        C.names[_c] = (struct name) {{'c'}, _s, 1, 0};
        
        C.names[_a0] = (struct name) {{'0'}, _s, 1, 0};
        C.names[_a] = (struct name)
        {{'a', o+_a0}, _n, 2, 0};
        
        C.names[_p0] = (struct name) {{'0'}, _i, 1, 0};
        C.names[_p] = (struct name)
        {{'p', o+_p0}, _n, 2, 0};
        
        C.names[_d0] = (struct name)
        {{'0'}, _n, 1, 0};
        C.names[_d1] = (struct name)
        {{'0'}, _U, 1, 0};
        C.names[_d] = (struct name)
        {{'d', o+_d0, o+_d1}, _i, 3, 0};

        C.names[_k0] = (struct name)
        {{'0', 0}, _n, 1, 0};
        C.names[_k1] = (struct name)
        {{'0', 0}, _n, 1, 0};
        C.names[_k] = (struct name)
        {{'k', o+_k0, o+_k1}, _n, 3, 0};
        
        C.names[_j0] = (struct name)
        {{'0', 0}, _i, 1, 0};
        C.names[_j1] = (struct name)
        {{'0', 0}, _i, 1, 0};
        C.names[_j] = (struct name)
        {{'j', o+_j0, o+_j1}, _i, 3, 0};
        
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
            }
            if (text[i] == 10) {
                l++;
                c = 1;
            } else {
                c++;
            }
        }
        
        puts("");
        debug_context(&C);
        puts("");
        
        struct expr e = parse(tokens, count, _U, 0, &C);
        
        puts("");
        debug_resolved(e, 0, &C);
        puts("");
        debug_context(&C);
        puts("");
        
        if (!e.index || C.at != count) {
            printf("!e.index = %d\n", !e.index);
            printf("C.at != count = %d, -> [%lu != %lu]\n\n\n", C.at != count, C.at, count);
            parse_error(tokens, count, loc, _U, &C, argv[i], text, length);
        }

        free(tokens);
        free(text);
    }
}

