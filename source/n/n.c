#include <llvm-c/Core.h>           // a n3zqx2l compiler
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <llvm-c/ExecutionEngine.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct resolved {
    size_t index;
    size_t count;
    size_t total;
    struct resolved* args;
};

struct name {
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
    struct name* names;
    struct name* owners;
};

static inline void represent
(size_t given, char* buffer, size_t limit,
 size_t* at, struct context* context) {
    if (given < 256) {
        buffer[(*at)++] = given;
        return;
    } else given -= 256;
    if (given >= context->name_count) return;
    struct name s = context->names[given];
    for (size_t i = 0; i < s.count; i++)
        represent(s.sig[i], buffer, limit, at, context);
    if (!s.type) return;
    buffer[(*at)++] = '\n';
    represent(s.type, buffer, limit, at, context);
}

static inline struct resolved resolve (uint8_t* given, size_t begin, size_t end, size_t type, size_t max_depth, struct context* C) {
    return (struct resolved) {0};
}

void debug_context(struct context* context) {
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

void debug_resolved(struct resolved given, size_t depth, struct context* context) {
    char buffer[4096] = {0}; size_t index = 0;
    represent(given.index, buffer, sizeof buffer, &index, context);
    for (size_t i = depth; i--;) printf(".   ");
    printf("%s:%lu:%lu:%c\n", buffer, given.index, given.total, (char) given.index);
    for (size_t i = 0; i < given.count; i++) debug_resolved(given.args[i], depth + 1, context);
    puts("");
}

void destroy(struct resolved r) {
    for (size_t i = 0; i < r.count; i++)
        destroy(r.args[i]);
    free(r.args);
}

int main(int argc, const char** argv) {
    for (int a = 1; a < argc; a++) {
        
        struct stat st;
        uint8_t *text, *tokens;
        int f = open(argv[a], O_RDONLY);
        
        if (f < 0 || stat(argv[a], &st) < 0 ||
            (text = mmap(0, st.st_size, 1, 1, f, 0))
            == MAP_FAILED) {
            fprintf(stderr, "n3zqx2l: %s: ", argv[a]);
            perror("error"); continue;
        } else close(f);
        
        tokens = malloc(st.st_size);
        uint16_t* loc = malloc(4 * st.st_size), l = 1, c = 1;
        size_t count = 0;
        
        for (size_t i = 0; i < (size_t) st.st_size; i++) {
            if (text[i] > 32) {
                loc[2 * count] = l;
                loc[2 * count + 1] = c;
                tokens[count++] = text[i];
            }
            if (text[i] == 10) {
                l++;
                c = 1;
            } else c++;
        }
        
        struct context context = {0};        
        struct resolved result = resolve(tokens, 0, count, 256, 5, &context);
        
        if (!result.index && count) {
            if (context.best == count) context.best--;
            fprintf(stderr,"n3zqx2l: %s:%u:%u: error: unresolved %c\n\n",
                    argv[a], loc[2 * context.best],
                    loc[2 * context.best + 1], tokens[context.best]);
        }
        
        debug_resolved(result, 0, &context);
        debug_context(&context);
        
        destroy(result);
        free(loc);
        free(tokens);
        munmap(text, st.st_size);
    }
}

