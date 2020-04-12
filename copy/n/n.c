#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <stdio.h>
#include <stdlib.h>

enum {
    _o,
    _i,
    _c,
    _appchar,
    _apppar,
    _join,
    _incr,
    _decr
};

struct token {
    size_t value;
    size_t line;
    size_t column;
};

struct resolved {
    size_t index;
    size_t value;
    size_t count;
    struct resolved* arguments;
};

struct name {
    size_t type;
    size_t count;
    struct resolved* signature; ///This doesnt need to be a resolved... it can just be a index or a value?       use the top bit to see whether this is a index or a value.   or simply offset all indicies by 256, so that any index less than 256 is known to be a value. i think thats the better way to do things. cool. maybe later.
};

struct frame {
    size_t count;
    size_t* indicies;
};

struct context {
    size_t at;
    size_t best;
    size_t frame_count;
    size_t count;
    struct frame* frames;
    struct name* names;
};

static void represent(size_t given, char* buffer, size_t limit, size_t* at, struct context* context) {
    struct name name = context->names[given];
    for (size_t i = 0; i < name.count; i++) {
        if (*at + 2 >= limit) return;
        else if (name.signature[i].value) buffer[(*at)++] = name.signature[i].value;
        else {
            buffer[(*at)++] = '(';
            represent(name.signature[i].index, buffer, limit, at, context);
            buffer[(*at)++] = ')';
        }
    }
    if (name.type) {
        buffer[(*at)++] = ' ';
        represent(name.type, buffer, limit, at, context);
    }
}

static void print_error(struct token* given, size_t given_count, size_t type, struct context* context, const char* filename) {
     ///TODO: we probably want to have a limiter on the number of errors that we are going to give. thats seems important.
    char buffer[2048] = {0}; size_t index = 0;
    represent(type, buffer, sizeof buffer,  &index, context);
    if (context->best < given_count) {
        struct token b = given[context->best];
        printf("n3zqx2l: %s:%lu:%lu: error: %s: unresolved %c\n\n", filename, b.line, b.column, buffer, (char) b.value);
    } else printf("n3zqx2l: %s:%d:%d: error: %s: unresolved expression\n\n", filename, 1, 1, buffer);
}

static struct resolved resolve_at(struct token* given, size_t given_count, size_t type, struct context* context, size_t depth, const char* filename) {
    if (depth > 128) return (struct resolved) {0};
    size_t saved = context->at;
    struct frame top = context->frames[context->frame_count - 1];
    for (size_t i = 0; i < top.count; i++) {
        context->best = fmax(context->at, context->best); context->at = saved;
        struct resolved solution = {top.indicies[i], 0, 0, 0};
        struct name name = context->names[solution.index];
        if (name.type != type) continue;
        
        for (size_t s = 0; s < name.count; s++) {
            if (context->at >= given_count) {
                if (solution.count == 1 && s == 1)
                    return solution.arguments[0];
                else goto next;
            }
            if (!name.signature[s].value) {
                struct resolved argument = resolve_at(given, given_count, context->names[name.signature[s].index].type, context, depth + 1, filename);
                if (!argument.index) goto next;
                solution.arguments = realloc(solution.arguments, sizeof(struct resolved) * (solution.count + 1));
                solution.arguments[solution.count++] = argument;
            } else if (name.signature[s].value != given[context->at].value) goto next;
            else context->at++;
        }
        return solution;
        next: continue;
    }
    if (type == _c) return (struct resolved) {_c, given[context->at++].value, 0, 0};
    
    print_error(given, given_count, type, context, filename);
    
    return (struct resolved) {0};
}

static void debug_context(struct context* context) {
    printf("index = %lu, best = %lu\n", context->at, context->best);
    printf("---- debugging frames: ----\n");
    for (size_t i = 0; i < context->frame_count; i++) {
        printf("\t ----- FRAME # %lu ---- \n\t\tidxs: { ", i);
        for (size_t j = 0; j < context->frames[i].count; j++) printf("%lu ", context->frames[i].indicies[j]);
        puts("}");
    }
    printf("\nmaster: {\n");
    for (size_t i = 0; i < context->count; i++) {
        printf("\t%6lu: ", i);
        char buffer[2048] = {0};
        size_t index = 0;
        represent(i, buffer, sizeof buffer, &index, context);
        printf("%s", buffer);
        puts("\n");
    }
    puts("}");
}

static void debug_resolved(struct resolved given) {
    printf(" [%lu]:%c:{ ", given.index, (char) given.value);
    for (size_t i = 0; i < given.count; i++) {
        printf("ARG(%lu)[", i);
        debug_resolved(given.arguments[i]);
        printf("], ");
    }
    printf(" } ");
}

int main(int argc, const char** argv) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') exit(!puts("n3zqx2l version 0.0.0\nn3zqx2l [-] [files]"));
        
        struct context context = {0};
        context.frames = calloc(context.frame_count = 1, sizeof(struct frame));
        
        printf("the CSR terminal. used for debugging purposes. type 'h' for help.\n");
        
        while (1) {
            
            printf(": ");
            int c = getchar();
            
            if (c == 'q') break;
            
            else if (c == 'h') {
                printf("a : add new name\n"
                       "s : print state\n"
                       "u : push new frame index\n"
                       "o : pop last frame index\n"
                       "d : pop last name\n"
                       "h : this help menu\n"
                       "q : quit terminal\n"
                       );
                
            } else if (c == 'a') {
                
                struct name n = {0};
                printf("adding signature\n");
                
                printf("type: ");
                scanf("%lu", &n.type);
            
                printf("give symbols.\n for a parameter, give (, then type a number, then type ).\n to terminate the signature, type ;\n");
                while (c != ';') {
                    
                    printf("symbol: ");
                    c = getchar();
                    printf("received: %d\n", c);
                    
                    if (c == '(') {
                        
                        struct resolved param = {0};
                        
                        printf("index: ");
                        scanf("%lu", &param.index);
                        
                        printf("close paren: ");
                        getchar();
                        
                        n.signature = realloc(n.signature, sizeof(struct resolved) * (n.count + 1));
                        n.signature[n.count++] = param;
                        
                    } else if (c != '\n' && c != ';') {
                        n.signature = realloc(n.signature, sizeof(struct resolved) * (n.count + 1));
                        n.signature[n.count++].value = c;
                    }
                }
                printf("signature terminated. adding now.");
                context.names = realloc(context.names, sizeof(struct name) * (context.count + 1));
                context.names[context.count++] = n;
                printf("added.\n");
                
            } else if (c == 's') {
                
                printf("printing context: \n\n");
                debug_context(&context);
                
            } else if (c == 'd') {
                printf("decrementing name count..\n");
                context.count--;
                
            } else if (c == 'o') {
                
                printf("decrementing stack frame indicies count..\n");
                context.frames[0].count--;
                
            } else if (c == 'u') {
                
                size_t integer = 0;
                printf("index to push: ");
                scanf("%lu", &integer);
                
                context.frames[0].indicies = realloc(context.frames[0].indicies, sizeof(size_t) * (context.frames[0].count + 1));
                context.frames[0].indicies[context.frames[0].count++] = integer;
            } else if (c == 'f') {
                
                printf("performing CSR:\n");
                size_t expected_type = 0;
                printf("expected type: ");
                scanf("%lu", &expected_type);
                if (expected_type >= context.count) {
                    printf("error: expected type not in range! trying 0 instead.\n");
                    expected_type = 0;
                }
                
                printf("reading in file again...\n");
                FILE* file = fopen(argv[i], "r");
                if (!file) {
                    fprintf(stderr, "n: %s: ", argv[i]);
                    perror("error");
                    continue;
                }
                
                fseek(file, 0, SEEK_END);
                size_t length = ftell(file);
                char* text = malloc(length * sizeof(char));
                struct token* tokens = malloc(length * sizeof(struct token));
                fseek(file, 0, SEEK_SET);
                fread(text, sizeof(char), length, file);
                fclose(file);
                
                size_t token_count = 0, line = 1, column = 1;
                for (size_t i = 0; i < length; i++) {
                    if (text[i] > ' ') tokens[token_count++] = (struct token){text[i], line, column};
                    if (text[i] == '\n') { line++; column = 1; } else column++;
                }
                
                context.at = context.best = 0;
                size_t type = 0;
                
                struct resolved resolved = resolve_at(tokens, token_count, type, &context, 0, argv[i]);
                debug_resolved(resolved);
                
                if (!resolved.index) {
                    printf("\n\n\tRESOLUTION ERROR\n\n");
                }
                
                free(tokens);
                free(text);
                                                
            } else if (c == '\n') {
                // do nothing.
            } else {
                printf("ERROR: unrecognized command: %c\n", c);
            }
        }
        printf("terminated terminal.\n");
        
        free(context.frames);
        free(context.names);
        
    }
}
