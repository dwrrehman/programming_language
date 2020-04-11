#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <stdio.h>
#include <stdlib.h>

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
    struct resolved sig;
    size_t type;
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
    if (given >= context->count || *at >= limit) return;
    struct name name = context->names[given];
    if (name.sig.value) buffer[(*at)++] = name.sig.value;
    else for (size_t i = 0; i < name.sig.count; i++) {
        represent(name.sig.arguments[i].index, buffer, limit, at, context);
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
        
        context->best = fmax(context->at, context->best);
        context->at = saved;
        
        struct resolved solution = {top.indicies[i], 0, 0, 0};
        struct name name = context->names[solution.index];
        
        if (name.type != type) continue;
        
        if (name.sig.value) { // (c) sig
            if (name.sig.value != given[context->at].value) continue;
            context->at++;
            
        } else if (name.sig.index == 4) { // Decl-param(sig)(type) sig
            ///IS THIS RIGHTTT?????!!?
            if (context->at >= given_count) {
                if (solution.count == 1 /*&& s == 1  ie, this is the first signature element. */)
                    return solution.arguments[0];
                else goto next;
            } /// where do we put thiss!??!
            /// call match_signature() on name.signature.arguments[0];
            struct resolved argument = resolve_at(given, given_count, name.sig.arguments[1].index, context, depth + 1, filename);
            
            if (!argument.index) goto next;
            
            solution.arguments = realloc(solution.arguments, sizeof(struct resolved) * (solution.count + 1));
            solution.arguments[solution.count++] = argument;
            return argument; ///????
        }
//        for (each argument to s) {
//            sol = resolve(given, type, context, depth);
//            if (!sol.index) return 0; /// continue to thenext signature.
//        }
        return solution;
        next: continue; /// we might not need a goto statement at all with this change!
    }
    
    if (type == 3) return (struct resolved) {3, given[context->at++].value, 0, 0};
    
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
        
        struct context context = {0};
        context.frames = calloc(context.frame_count = 1, sizeof(struct frame));
        size_t type = 0;
        struct resolved resolved = resolve_at(tokens, token_count, type, &context, 0, argv[i]);
        
        debug_context(&context);
        debug_resolved(resolved);
        if (!resolved.index) {
            printf("\n\n\tRESOLUTION ERROR\n\n");
        }
        
        free(context.frames);
        free(context.names);
        free(tokens);
        free(text);
    }
}
