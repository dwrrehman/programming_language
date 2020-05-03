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
//    _f0, _f1, _f2, _f,
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

struct location {
    uint16_t line;
    uint16_t column;
};

struct resolved {
    size_t index;
    size_t count;
    struct resolved* args;
    uint8_t value;
};

struct name {
    size_t signature[256];
    size_t type;
    uint8_t count;
    uint8_t codegen_as;
};

struct frame {
    size_t indicies[2][256];
    size_t count[2];
    struct name name;
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
        
        else if (name.signature[i] < 256)
            buffer[(*at)++] = name.signature[i];
        
        else {
            buffer[(*at)++] = '(';
            represent(name.signature[i] - 256, buffer,
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

static void parse_error
(uint8_t* given, size_t given_count,
 struct location* loc, size_t type,
 struct context* C, const char* filename) {
    
    char type_string[4096] = {0};
    size_t index = 0;
    represent(type, type_string,
              sizeof type_string,  &index, C);
    
    if (C->best == given_count && C->best) {
        C->best--;
        printf("n3zqx2l: %s:%u:%u: error: expected %s near %c\n\n",
               filename, loc[C->best].line,
               loc[C->best].column,
               type_string, given[C->best]); }
    else if (C->best < given_count)
        printf("n3zqx2l: %s:%u:%u: error: %s: unresolved %c\n\n",
               filename, loc[C->best].line,
               loc[C->best].column,
               type_string, given[C->best]);
    else
        printf("n3zqx2l: %s: error: %s: empty file\n\n",
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
           sizeof(struct name)
           * s->name_count);
    
    d->frame_count = s->frame_count;
    d->frames = realloc(d->frames,
                        sizeof(struct frame)
                        * s->frame_count);
    
    memcpy(d->frames, s->frames,
           sizeof(struct frame)
           * s->frame_count);
}

static struct resolved resolve
 (uint8_t* given, size_t count, size_t type,
  size_t depth, struct context* context)
{
    if (depth > 32)
        return (struct resolved) {0};
    
    if (context->at < count && type == _s)
        return (struct resolved) {
            _c, 0, 0, given[context->at++]
        };
    
    struct context saved = {0};
    duplicate_context(&saved, context);
    
    for (size_t p = 2; p--;) {
        for (size_t f = context->frame_count; f--;) {
            for (size_t i = context->frames[f].count[p]; i--; ) {
                
                context->best = fmax(context->at, context->best);
                duplicate_context(context, &saved);
                
                struct resolved sol = {
                    context->frames[f].indicies[p][i],
                    0, 0, 0
                };
                
                struct name name = context->names[sol.index];
                
                if (type && name.type != type)
                    continue;
                
                if (sol.index == _d)
                    context->frames = realloc(context->frames,
                                              sizeof(struct frame) *
                                              ++context->frame_count);
                
                for (uint8_t s = 0; s < name.count; s++) {
                    
                    if (context->at >= count) {
                        if (p == 1 && s == 1)
                            return *sol.args;
                        else
                            goto next;
                    }
                    
                    else if (name.signature[s] >= 256) {
                        struct resolved argument
                        = resolve(given, count,
                                  context->names
                                  [name.signature[s] - 256].type,
                                  depth + 1, context);
                        
                        if (!argument.index)
                            goto next;
                        
                        sol.args = realloc(sol.args,
                                           sizeof(struct resolved)
                                           * (sol.count + 1));
                        sol.args[sol.count++] = argument;
                        
                    } else if (name.signature[s] == given[context->at]) {
                        context->at++; depth = 0;
                        
                    } else
                        goto next;
                }
                
                if (sol.index == _d) {
                    
                    size_t f = --context->frame_count;
                    
                    context->frames[f].name.type
                        = sol.args[1].index;
                    
                    context->names
                    = realloc(context->names,
                              sizeof(struct name)
                              * (context->name_count + 1));
                    context->names[context->name_count++]
                    = context->frames[f].name;
                    
                    size_t
                    m = context->frames[f].name.count &&
                        context->frames[f]
                            .name.signature[0] >= 256,
                        b = context->frame_count - 1;
                    
                    context->frames[b].indicies
                    [m][context->frames[b].count[m]++]
                        = context->name_count - 1;
                    
                } else if (sol.index == _a || sol.index == _p)
                    
                    context->frames
                    [context->frame_count - 1]
                        .name.signature
                    [context->frames
                     [context->frame_count - 1]
                        .name.count++]
                    = sol.index == _a
                        ? sol.args[0].value
                        : 256 + context->name_count - 1;
                
                return sol;
                
            next:
                if (sol.index == _d)
                    context->frame_count--;
            }
        }
    }
    return (struct resolved) {0};
}


static void debug_context
 (struct context* context) {
    printf("index = %lu, best = %lu\n",
           context->at, context->best);
    
    printf("---- debugging frames: ----\n");
    
    for (size_t m = 0; m < 2; m++) {
        
        printf("mode = %lu\n", m);
        
        for (size_t i = 0; i < context->frame_count; i++) {
            
            printf("\t ----- [m=%lu] FRAME # %lu ---- \n"
                   "\t\tidxs: { ",
                   m, i);
            
            for (size_t j = 0; j <
                 context->frames[i].count[m]; j++)
                printf("%lu ", context->frames[i]
                    .indicies[m][j]);
            
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

static void prep(size_t depth) {
    for (size_t i = depth; i--;)
        printf(".   ");
}

static void debug_resolved
(struct resolved given,
 size_t depth,
 struct context* context)
{
    char buffer[4096] = {0};
    size_t index = 0;
    represent(given.index, buffer,
              sizeof buffer, &index, context);
    
    prep(depth);
    printf("%s : [%lu]", buffer, given.index);
    
    if (given.value)
        printf(" : c=%c", (char) given.value);
    
    printf("\n");
    
    for (size_t i = 0; i < given.count; i++)
        
        debug_resolved(given.args[i],
                       depth + 1, context);
    
    prep(depth);
    printf("\n");
}




static void lex
(size_t *count,
 size_t length,
 struct location* loc,
 uint8_t* text,
 uint8_t* tokens)
{
    uint16_t
        l = 1,
        c = 1;
    
    for (size_t i = 0; i < length; i++) {
        
        if (text[i] > 32) {
            tokens[*count] = text[i];
            loc[(*count)++]
            = (struct location){l, c};
        }
        
        if (text[i] == 10) {
            l++;
            c = 1;
            
        } else {
            c++;
        }
    }
}

int main(int argc, const char** argv) {
    
    for (int i = 1; i < argc; i++) {
        
        if (argv[i][0] == '-')
            exit(!puts("n3zqx2l version 0.0.0\n"
                       "n3zqx2l [-] [files]"));
        
        const size_t o = 256;
        
        struct context C = {
            0, 0,
            _intrin_count, 1,
            calloc(_intrin_count,
                   sizeof(struct name)),
            calloc(1,
                   sizeof(struct frame))};
        
        C.frames[0] = (struct frame) {{{
            _U, _s, _c, _i, _n, _a, _p, _d, _k, _j
        },{0}}, {10, 0}, {0}};
        
        C.names[_U] = (struct name) {{'U'}, _U, 1, 0};
        C.names[_i] = (struct name) {{'i'}, _U, 1, 0};
        C.names[_n] = (struct name) {{'n'}, _i, 1, 0};
        C.names[_s] = (struct name) {{'s'}, _i, 1, 0};
        C.names[_c] = (struct name) {{'c'}, _s, 1, 0};
        
        C.names[_a0] = (struct name) {{'0'}, _s, 1, 0};
        C.names[_a] = (struct name) {{'a', o+_a0}, _n, 2, 0};
        
        C.names[_p0] = (struct name) {{'0'}, _i, 1, 0};
        C.names[_p] = (struct name) {{'p', o+_p0}, _n, 2, 0};
        
        C.names[_d0] = (struct name) {{'0'}, _n, 1, 0};
        C.names[_d1] = (struct name) {{'0'}, _U, 1, 0};
        C.names[_d] = (struct name) {{'d', o+_d0, o+_d1}, _i, 3, 0};

        C.names[_k0] = (struct name) {{'0', 0}, _n, 1, 0};
        C.names[_k1] = (struct name) {{'0', 0}, _n, 1, 0};
        C.names[_k] = (struct name) {{'k', o+_k0, o+_k1}, _n, 3, 0};
        
        C.names[_j0] = (struct name) {{'0', 0}, _i, 1, 0};
        C.names[_j1] = (struct name) {{'0', 0}, _i, 1, 0};
        C.names[_j] = (struct name) {{'j', o+_j0, o+_j1}, _i, 3, 0};
        
        FILE* file = fopen(argv[i], "r");
        
        if (!file) {
            fprintf(stderr, "n: %s: ", argv[i]);
            perror("error");
            continue;
        }
        
        fseek(file, 0, SEEK_END);
        
        size_t
            length = ftell(file),
            count = 0;
        
        uint8_t
            *text = malloc(length),
            *tokens = malloc(length);
        
        struct location* loc
            = malloc(length *
                     sizeof(struct location));
        
        fseek(file, 0, SEEK_SET);
        fread(text, 1, length, file);
        fclose(file);
        
        lex(&count, length, loc, text, tokens);
        
        puts("");
        debug_context(&C);
        puts("");
        
        struct resolved resolved
            = resolve(tokens, count, _U, 0, &C);
        
        puts("");
        debug_resolved(resolved, 0, &C);
        
        puts("");
        debug_context(&C);
        puts("");
        
        if (!resolved.index || C.at != count)
            
            parse_error(tokens, count,
                        loc, _U, &C, argv[i]);
        
        free(tokens);
        free(text);
    }
}

///......................................................... <- 60 columns.
