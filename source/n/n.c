//..........................................................
#include <llvm-c/Core.h>
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

static inline size_t lex
 (uint8_t* text, uint8_t* tokens,
  uint16_t* loc, long length) {
     
    size_t count = 0;
    uint16_t l = 1, c = 1;
    
    for (long i = 0; i < length; i++) {
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
    return count;
}

static inline struct resolved parse
 (uint8_t* given, size_t begin,
  size_t end, size_t type, size_t max_depth,
  struct context* C) {
    

    return (struct resolved) {0};
}

static inline void represent
(size_t given, char* buffer, size_t limit,
 size_t* at, struct context* context) {
    
    if (given < 256) {
        buffer[(*at)++] = given;
        return;
    } else given -= 256;
    if (given >= context->name_count) return;
    
    buffer[(*at)++] = ' ';
    struct name s = context->names[given];
    for (size_t i = 0; i < s.count; i++)
        represent(s.sig[i], buffer,
                  limit, at, context);
    
    if (!s.type) return;
    
    buffer[(*at)++] = ' ';
    represent(s.type, buffer,
              limit, at, context);
}

void debug_context(struct context* context) {
    printf("\n[best = %lu]\n", context->best);
    printf("---- debugging frames: ----\n");
    for (size_t i = 0; i < context->frame_count; i++)
        printf("\tframe # %lu  bp = %lu\n",
               i, context->frames[i]);
    printf("\n---- debugging indicies: ----\n");
    printf("\t\tidxs: { ");
    for (size_t i = 0; i < context->index_count; i++)
        printf("%lu ", context->indicies[i]);
    printf("}\n\n----- master: ------ \n{\n");
    for (size_t i = 0; i < context->name_count; i++) {
        char buffer[4096] = {0}; size_t index = 0;
        represent(i, buffer, sizeof buffer,
                  &index, context);
        printf("\t%6lu: %s\n\n", i, buffer);
    } printf("}\n\n");
}

void debug_resolved
(struct resolved given,
 size_t depth,
 struct context* context) {
    char buffer[4096] = {0}; size_t index = 0;
    represent(given.index, buffer,
              sizeof buffer, &index, context);
    for (size_t i = depth; i--;) printf(".   ");
    printf("%s:%lu:%lu:%c\n", buffer,
           given.index, given.total,
           (char) given.index);
    for (size_t i = 0; i < given.count; i++)
        debug_resolved(given.args[i],
                       depth + 1, context);
    printf("\n");
}

void destroy(struct resolved r) {
    for (size_t i = 0; i < r.count; i++)
        destroy(r.args[i]);
    free(r.args);
}

int main(int argc, const char** argv) {
    for (int a = 1; a < argc; a++) {
        
        uint8_t* text;
        struct stat st;
        int file = open(argv[a], O_RDONLY);
        if (file < 0 ||
            stat(argv[a], &st) < 0 ||
            (text = mmap(0, st.st_size,
                         PROT_READ, MAP_SHARED,
                         file,0))
            == MAP_FAILED) {
            fprintf(stderr, "n: %s: ", argv[a]);
            perror("error"); continue;
        } else close(file);
        
        struct context context = {0};
        uint8_t* tokens = malloc(st.st_size);
        uint16_t* loc = malloc(4 * st.st_size);
        
        size_t count = lex(text, tokens,
                           loc, st.st_size);
        
        struct resolved ast = parse(tokens, 0, count,
                                    256, 5, &context);
        
        if (!ast.index && count) {
            if (context.best == count)
                context.best--;
            fprintf(stderr,
                    "n: %s:%u:%u: error: unresolved %c\n\n",
                    argv[a],
                    loc[2 * context.best],
                    loc[2 * context.best + 1],
                    tokens[context.best]);
        }
        
        debug_resolved(ast, 0, &context);
        debug_context(&context);
        
        destroy(ast);
        free(loc);
        free(tokens);
        munmap(text, st.st_size);
    }
}
