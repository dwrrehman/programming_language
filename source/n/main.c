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
    size_t id;
    size_t value;
    size_t count;
    struct resolved* arguments;
};

struct name {
    size_t length;
    struct resolved* data;
};

struct frame {
    size_t count;
    size_t* indicies;
};

struct context {
    size_t at;
    size_t best;
    size_t frame_count;
    struct frame* frames;
    size_t name_count;
    struct name* names;
    const char* filename;
    size_t UNUSED;
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

static void represent(struct resolved given, char* buffer, size_t limit, size_t* at, struct context* context) {
    struct name name = context->names[given.id];
    for (size_t i = 2, a = 0; i < name.length; i++) {
        if (*at + 2 >= limit) return;
        else if (name.data[i].value) buffer[(*at)++] = name.data[i].value;
        else {
            buffer[(*at)++] = '(';
            represent(given.count ? given.arguments[a++] : name.data[i], buffer, limit, at, context);
            buffer[(*at)++] = ')';
        }
    }
    if (name.data[0].id) {
        buffer[(*at)++] = ' ';
        represent(name.data[0], buffer, limit, at, context);
    }
}

static struct resolved resolve(struct expression given, struct resolved type, struct context* context, size_t max_depth);

static struct resolved resolve_at(struct expression given, struct resolved type, struct context* context, size_t depth, size_t max_depth) {
    if (depth > max_depth) return (struct resolved) {0};
    else if (context->at < given.count && !given.symbols[context->at].value) return resolve(given.symbols[context->at++], type, context, max_depth);

    size_t saved = context->at;
    struct frame top = context->frames[context->frame_count - 1];
    
    for (size_t i = 0; i < top.count; i++) {
        context->best = fmax(context->at, context->best); context->at = saved;
        struct resolved solution = {top.indicies[i], 0, 0, 0};
        struct name name = context->names[solution.id];
        
///        if (/* doesnt type check */0) continue;
        
        for (size_t s = 2; s < name.length; s++) {
            if (context->at >= given.count) {
                if (solution.count == 1 && s == 3) return solution.arguments[0]; else goto next;
            }
            if (!name.data[s].value) {
                struct resolved argument = resolve_at(given, context->names[name.data[s].id].data[0], context, depth + 1, max_depth);
                if (!argument.id) goto next; /// if (/* doesnt type check */0) goto next;
                solution.arguments = realloc(solution.arguments, sizeof(struct resolved) * (solution.count + 1));
                solution.arguments[solution.count++] = argument;
            } else if (name.data[s].value != given.symbols[context->at].value) goto next; else context->at++;
        }
        return solution;
        next: continue;
    }
    return (struct resolved) {0};
}

static struct resolved resolve(struct expression given, struct resolved type, struct context* context, size_t max_depth) {
    struct context sub = *context; sub.at = sub.best = 0;
    struct resolved solution = resolve_at(given, type, &sub, 0, max_depth);
    if (sub.at < given.count) solution.id = 0;
    
    if (!solution.id) {
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
    printf("\n---- debugging frames: ----\n");
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
        represent((struct resolved){i, 0, 0, 0}, buffer, sizeof buffer, &index, context);
        printf("%s", buffer);
        memset(buffer, 0, sizeof buffer);
        index = 0;
        represent(context->names[i].data[1], buffer, sizeof buffer, &index, context);
        if (context->names[i].data[1].id) printf("      %s", buffer);
        puts("\n");
    }
    puts("}");
}
  

int main(const int argc, const char** argv) {
    size_t max_depth = 20;
    for (int i = 1; i < argc; i++) {
        if (!strcmp("version", argv[i])) exit(!puts("n: 0.0.3"));
        
        char* text = open_file(argv[i]);
        if (!text) continue;
        size_t at = 0;
        uint32_t line = 1, column = 1;
        struct expression expression = parse(argv[i], text, &at, &line, &column);
        debug_expression(expression); puts("\n");
        
        struct name zero = {0};
        zero.data = calloc(zero.length = 3, sizeof(struct resolved));
        zero.data[2].value = 'o';
        
        struct name one = {0};
        one.data = calloc(one.length = 3, sizeof(struct resolved));
        one.data[2].value = 'i';
        
        struct frame init = {0};
        init.indicies = calloc(init.count = 2, sizeof(size_t));
        init.indicies[0] = 0;
        init.indicies[1] = 1;
        
        struct context context = {0};
        context.names = calloc(context.name_count = 2, sizeof(struct name));
        context.names[0] = zero;
        context.names[1] = one;
        context.frames = calloc(context.frame_count = 1, sizeof(struct frame));
        context.frames[0] = init;
        context.filename = argv[i];
             
        struct resolved resolved = resolve(expression, (struct resolved) {1, 0, 0, 0}, &context, max_depth);
        
        debug_context(&context);
        char buffer[2048] = {0}; size_t index = 0;
        represent(resolved, buffer, sizeof buffer, &index, &context);
        
        printf("printing solution: \n\n\t%s\n\n", buffer);
        
        if (!resolved.id) {
            printf("\nRESOLUTION ERROR\n");
        }
        
    }
}
