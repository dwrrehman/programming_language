#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

struct expression {
    size_t value;
    size_t count;
    struct expression* symbols;
    uint32_t line;
    uint32_t column;
};

struct resolved {
    size_t index;
    size_t value;
    size_t count;
    struct resolved* arguments;
};

struct name {
    size_t type;
    struct resolved signature;
    size_t length; /// Do we need this?
};

struct frame {
    size_t count;
    size_t* indicies;
};

struct context {
    size_t at;
    size_t best;
    size_t frame_count;
    size_t name_count;
    size_t UNUSED;
    struct frame* frames;
    struct name* names;
    const char* filename;
};

static char* open_file(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (!file) { fprintf(stderr, "n: %s: ", filename); perror("error"); return NULL; }
    fseek(file, 0, SEEK_END);
    const size_t length = ftell(file);
    char* buffer = (char*) calloc(length + 1, sizeof(char));
    fseek(file, 0, SEEK_SET);
    fread(buffer, sizeof(char), length, file);
    fclose(file);
    return buffer;
}

static struct expression parse(const char* filename, const char* text, size_t* at, uint32_t* line, uint32_t* column) {
    struct expression result = {0};
    while (text[*at] && text[*at] != ')') {
        if (text[*at] == '\n') { ++*line; *column = 1;} else ++*column;
        if (isspace(text[(*at)++])) continue;
        uint32_t l = *line, c = *column - 1;
        result.symbols = realloc(result.symbols, sizeof(struct expression) * (result.count + 1));
        if (text[*at - 1] == '(') {
            result.symbols[result.count] = parse(filename, text, at, line, column);
            result.symbols[result.count].line = l; result.symbols[result.count++].column = c;
            if (!text[*at] || text[(*at)++] != ')') fprintf(stderr, "n3zqx2l: %s:%u:%u: error: expected )\n\n", filename, l, c); ++*column;
        } else result.symbols[result.count++] = (struct expression){text[*at - 1], 0, 0, l, c};
    } return result;
}

static void represent(size_t given, char* buffer, size_t limit, size_t* at, struct context* context) {
    if (given >= context->name_count) return;
    struct name name = context->names[given];
//    for (size_t i = 0; i < name.length; i++) {
//        if (*at + 2 >= limit) return;
//        else if (name.signature[i].value) buffer[(*at)++] = name.signature[i].value;
//        else {
//            buffer[(*at)++] = '(';
//            represent(name.signature[i].index, buffer, limit, at, context);
//            buffer[(*at)++] = ')';
//        }
//    }
    if (name.type) {
        buffer[(*at)++] = ' ';
        represent(name.type, buffer, limit, at, context);
    }
}

static struct resolved resolve(struct expression, size_t, struct context*);

static struct resolved resolve_at(struct expression given, size_t type, struct context* context, size_t depth) {
    if (depth > 128) return (struct resolved) {0};
    else if (context->at < given.count && !given.symbols[context->at].value) return resolve(given.symbols[context->at++], type, context);
    
    size_t saved = context->at;
    struct frame top = context->frames[context->frame_count - 1];
    
    for (size_t i = 0; i < top.count; i++) {
        context->best = fmax(context->at, context->best); context->at = saved;
        struct resolved solution = {top.indicies[i], 0, 0, 0};
        struct name name = context->names[solution.index];
        
        if (name.type != type) continue;
        
        if (name.signature.index == 3) { // (c) sig
            if (name.signature.value != given.symbols[context->at].value) continue;
            context->at++;
        }

        if (name.signature.index == 4) { // Decl-param(sig)(type) sig
            
            ///IS THIS RIGHTTT?????!!?
            if (context->at >= given.count) {
                if (solution.count == 1 && s == 1) return solution.arguments[0]; else goto next;
            } /// where do we put thiss!??!
            
            
            /// call match_signature() on name.signature.arguments[0];
            
            struct resolved argument = resolve_at(given, name.signature.arguments[1].index, context, depth + 1);
            
            if (!argument.index) goto next;
            
            solution.arguments = realloc(solution.arguments, sizeof(struct resolved) * (solution.count + 1));
            solution.arguments[solution.count++] = argument;
            
            return argument; ///????
        }
                
        for (each argument to s) {
            sol = resolve(given, type, context, depth);
            if (!sol.index) return 0; /// continue to thenext signature.
        }
                
        return solution;
        
        next: continue; /// we might not need a goto statement at all with this change!
    }
    /// I THINK HERE
    
    /// is where we should give errors. always.
    /// we dont need a seperate resolve() function for printing errors anymore.
    
    
    return (struct resolved) {0};
}

/*

static struct resolved resolve_at(struct expression given, size_t type, struct context* context, size_t depth) {
    if (depth > 128) return (struct resolved) {0};
    else if (context->at < given.count && !given.symbols[context->at].value) return resolve(given.symbols[context->at++], type, context);
    
    size_t saved = context->at;
    struct frame top = context->frames[context->frame_count - 1];
    
    for (size_t i = 0; i < top.count; i++) {
        context->best = fmax(context->at, context->best); context->at = saved;
        struct resolved solution = {top.indicies[i], 0, 0, 0};
        struct name name = context->names[solution.index];
        
        if (name.type != type) continue;
        
        for (size_t s = 0; s < name.length; s++) {
            if (context->at >= given.count) {
                if (solution.count == 1 && s == 1) return solution.arguments[0]; else goto next;
            }
            if (!name.signature[s].value) {
                struct resolved argument = resolve_at(given, context->names[name.signature[s].index].type, context, depth + 1);
                if (!argument.index) goto next;
                solution.arguments = realloc(solution.arguments, sizeof(struct resolved) * (solution.count + 1));
                solution.arguments[solution.count++] = argument;
            } else if (name.signature[s].value != given.symbols[context->at].value) goto next; else context->at++;
        }
        
        return solution;
        next: continue;
    }
    return (struct resolved) {0};
}

*/

static struct resolved resolve(struct expression given, size_t type, struct context* context) {
    struct context sub = *context; sub.at = sub.best = 0;
    struct resolved solution = resolve_at(given, type, &sub, 0);
    if (sub.at < given.count) solution.index = 0;
    
    if (!solution.index) {
        struct expression b = sub.best < given.count ? given.symbols[sub.best] : given;
        char buffer[2048] = {0}; size_t index = 0;
        represent(type, buffer, sizeof buffer,  &index, context);
        printf("n3zqx2l: %s:%u:%u: error: %s: unresolved %s%c\n\n", context->filename, b.line, b.column, buffer, b.value ? "symbol " : "expression", (char) b.value);
    }
    return solution;
}


static void debug_expression(struct expression e) {
    if (e.value) printf(" %c[%u,%u] ", (char) e.value, e.line, e.column);
    else {
        printf(" ( ");
        for (size_t i = 0; i < e.count; i++) debug_expression(e.symbols[i]);
        printf(" ) [%u,%u] ", e.line, e.column);
    }
}

static void debug_context(struct context* context) {
    printf("filename = %s\n", context->filename);
    printf("index = %lu, best = %lu\n", context->at, context->best);
    printf("---- debugging frames: ----\n");
    for (size_t i = 0; i < context->frame_count; i++) {
        printf("\t ----- FRAME # %lu ---- \n\t\tidxs: { ", i);
        for (size_t j = 0; j < context->frames[i].count; j++) printf("%lu ", context->frames[i].indicies[j]);
        puts("}");
    }
    printf("\nmaster: {\n");
    for (size_t i = 0; i < context->name_count; i++) {
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

int main(const int argc, const char** argv) {
    for (int i = 1; i < argc; i++) {
        if (!strcmp("version", argv[i])) exit(!puts("n3zqx2l: 0.0.3"));
        if (!strcmp("usage", argv[i])) exit(!puts("n3zqx2l version/usage [files]"));
                
        char* text = open_file(argv[i]);
        if (!text) continue;
        size_t at = 0;
        uint32_t line = 1, column = 1;
        struct expression expression = parse(argv[i], text, &at, &line, &column);
        debug_expression(expression); puts("\n");
                        
        struct context context = {0};
        context.frames = calloc(context.frame_count = 1, sizeof(struct frame));
        context.filename = argv[i];
        context.at = context.best = 0;

        
        // n.signature = realloc(n.signature, sizeof(struct resolved) * (n.length + 1));
        
        // n.signature = realloc(n.signature, sizeof(struct resolved) * (n.length + 1));
        // n.signature[n.length++].value = c;
        
         context.names = realloc(context.names, sizeof(struct name) * (context.name_count + 1));
         context.names[context.name_count++] = (struct name){0};

        
         context.frames[0].indicies = realloc(context.frames[0].indicies, sizeof(size_t) * (context.frames[0].count + 1));
         context.frames[0].indicies[context.frames[0].count++] = 0;
        
        debug_context(&context);
        struct resolved resolved = resolve(expression, 0, &context);
        debug_resolved(resolved);
        if (!resolved.index) printf("\nRESOLUTION ERROR\n");
    }
}
