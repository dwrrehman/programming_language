#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <llvm-c/ExecutionEngine.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { _U, _i };
enum { _cg_default, _cg_none, _cg_macro, _cg_function, _cg_namespace, _cg_variable, _cg_structure };

struct location {
    uint16_t line;
    uint16_t column;
};

struct resolved {
    size_t index;
    size_t count;
    struct resolved* arguments;
    uint8_t value;
};

struct name {
    uint16_t signature[256];
    size_t type;
    uint8_t count;
    uint8_t codegen_as;
};

struct frame {
    uint16_t indicies[512];
    uint16_t count;
};

struct context {
    size_t at;
    size_t best;
    size_t name_count;
    size_t frame_count;
    struct name* names;
    struct frame* frames;
};

static void represent(size_t given, char* buffer, size_t limit, size_t* at, struct context* context) {
    if (given > context->name_count) return;
    struct name name = context->names[given];
    for (size_t i = 0; i < name.count; i++) {
        if (*at + 4 >= limit) return;
        else if (name.signature[i] < 256) buffer[(*at)++] = name.signature[i];
        else {
            buffer[(*at)++] = '(';
            represent(name.signature[i] - 256, buffer, limit, at, context);
            buffer[(*at)++] = ')';
        }
    }
    if (name.type) {
        buffer[(*at)++] = ' ';
        represent(name.type, buffer, limit, at, context);
    }
}

static void parse_error(uint8_t* given, size_t given_count, struct location* locations, size_t type, struct context* context, const char* filename) {
    char buffer[4096] = {0};
    size_t index = 0;
    represent(type, buffer, sizeof buffer,  &index, context);
    if (context->best < given_count) {
        uint8_t b = given[context->best];
        struct location l = locations[context->best];
        printf("n3zqx2l: %s:%u:%u: error: %s: unresolved %c\n\n", filename, l.line, l.column, buffer, b);
    } else if (context->best == given_count && context->best) {
        uint8_t b = given[context->best - 1];
        struct location l = locations[context->best - 1];
        printf("n3zqx2l: %s:%u:%u: error: %s: unresolved expression near %c\n\n", filename, l.line, l.column, buffer, b);
    } else printf("n3zqx2l: %s:%u:%u: error: %s: unresolved empty expression\n\n", filename, 0, 0, buffer);
}

static struct resolved resolve(uint8_t* given, size_t given_count, size_t type, struct context* context) {
    struct context saved = *context; /// MAKE THIS A DEEP COPY!!!
    for (size_t f = context->frame_count; f--;) {
        for (uint16_t i = context->frames[f].count; i--; ) {
            context->best = fmax(context->at, context->best);
            context->at = saved.at;
            
            struct resolved solution = {context->frames[f].indicies[i], 0, 0, 0};
            struct name name = context->names[solution.index];
            if (name.type != type) continue;
            for (uint8_t s = 0; s < name.count; s++) {
                if (context->at >= given_count) goto next;
                else if (name.signature[s] >= 256) {
                    struct resolved argument = resolve(given, given_count, context->names[name.signature[s] - 256].type, context);
                    if (!argument.index) goto next;
                    solution.arguments = realloc(solution.arguments, sizeof(struct resolved) * (solution.count + 1));
                    solution.arguments[solution.count++] = argument;
                } else if (name.signature[s] == given[context->at]) context->at++;
                else goto next;
            }
            if (context->at != given_count) goto next; ///TODO: WIP
            return solution; next: continue;
        }
    } return (struct resolved) {0};
}
static void debug_context(struct context* context);
static void prep(size_t depth);
static void debug_resolved(struct resolved given, size_t depth, struct context* context);

static void resolve_file_in_context(const char* filename, struct context* context) {
    FILE* file = fopen(filename, "r");
    if (!file) { fprintf(stderr, "n: %s: ", filename); perror("error"); return; }
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file), count = 0;
    uint8_t *text = malloc(length), *tokens = malloc(length);
    struct location* loc = malloc(length * sizeof(struct location));
    fseek(file, 0, SEEK_SET);
    fread(text, 1, length, file);
    fclose(file);
    uint16_t l = 1, c = 1;
    for (size_t i = 0; i < length; i++) {
        if (text[i] > 32) {tokens[count] = text[i]; loc[count++] = (struct location){l, c};}
        if (text[i] == 10) {l++; c = 1;} else c++;
    }
    const size_t type = _i; // TEMP
    struct resolved resolved = resolve(tokens, count, type, context);
    puts(""); debug_resolved(resolved, 0, context); puts("\n");
    if (!resolved.index) {
        parse_error(tokens, count, loc, type, context, filename);
        printf("\n\n\tRESOLUTION ERROR\n\n");
    }
    free(tokens);
    free(text);
}

int main(int argc, const char** argv) {
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') exit(!puts("n3zqx2l version 0.0.0\nn3zqx2l [-] [files]"));
        struct context context = {0, 0, 6, 1, calloc(6, sizeof(struct name)), calloc(1, sizeof(struct frame))};
        context.frames[0] = (struct frame) {{0, 1, 2, 5}, 4};
        context.names[0] = (struct name) {{'U', 0}, _U, 1, 0};
        context.names[1] = (struct name) {{'i', 0}, _U, 1, 0};
        context.names[2] = (struct name) {{'n', 0}, _i, 1, 0};
        context.names[3] = (struct name) {{'a', 0}, _i, 1, 0};
        context.names[4] = (struct name) {{'b', 0}, _i, 1, 0};
        context.names[5] = (struct name) {{'j', 256+3, 256+4}, _i, 3, 0};
        resolve_file_in_context(argv[i], &context);
        debug_context(&context);
    }
}














































static void debug_context(struct context* context) {
    printf("index = %lu, best = %lu\n", context->at, context->best);
    printf("---- debugging frames: ----\n");
    for (size_t i = 0; i < context->frame_count; i++) {
        printf("\t ----- FRAME # %lu ---- \n\t\tidxs: { ", i);
        for (size_t j = 0; j < context->frames[i].count; j++) printf("%u ", context->frames[i].indicies[j]);
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

static void prep(size_t depth) { for (size_t i = depth; i--;) printf(".   "); }

static void debug_resolved(struct resolved given, size_t depth, struct context* context) {
    char buffer[4096] = {0};
    size_t index = 0;
    represent(given.index, buffer, sizeof buffer, &index, context);
    prep(depth); printf("%s : [%lu]", buffer, given.index);
    if (given.value) printf(" : c=%c", (char) given.value);
    printf("\n");
    for (size_t i = 0; i < given.count; i++) {
        debug_resolved(given.arguments[i], depth + 1, context);
    }
    prep(depth); printf("\n");
}

