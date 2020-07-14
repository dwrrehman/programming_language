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
    size_t index; // 2
    size_t begin; // 4
    size_t done; // 2
    size_t parent; // 2
    size_t queue_next; // 2
    size_t count; // 2
    size_t* args; // 2???
};

struct name {
    size_t type;
    size_t length;
    size_t* signature;
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

static inline size_t lex(uint8_t* text, uint8_t* tokens, uint16_t* loc, size_t length) {
    
    size_t count = 0;
    uint16_t l = 1, c = 1;
    
    for (size_t i = 0; i < length; i++) {
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

static inline size_t csr(const uint8_t* given, size_t end, struct resolved* m, struct context* context) {
    
    return 0;
}

static inline void represent
(size_t given, char* buffer, size_t limit,
 size_t* at, struct context* context) {
    if (given < 256) {
        buffer[(*at)++] = given;
        return;
    } else given -= 256;
    if (given >= context->name_count) return;
    struct name s = context->names[given];
    for (size_t i = 0; i < s.length; i++) {
        const size_t c = s.signature[i];
        if (c < 256) {
            buffer[(*at)++] = c;
        } else {
            buffer[(*at)++] = ' ';
            buffer[(*at)++] = '(';
            represent(c, buffer, limit, at, context);
            buffer[(*at)++] = ')';
            buffer[(*at)++] = ' ';
        }
    }
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
    printf("}\n\n----- names: ------ \n{\n");
    for (size_t i = 0; i < context->name_count; i++) {
        char buffer[4096] = {0}; size_t index = 0;
        represent(i + 256, buffer, sizeof buffer,
                  &index, context);
        printf("\t%6lu: %s\n\n", i, buffer);
    } printf("}\n\n");
}

int main(int argc, const char** argv) {
    for (int a = 1; a < argc; a++) {
        
        uint8_t* text;
        struct stat st;
        int file = open(argv[a], O_RDONLY);
        if (file < 0 ||
            stat(argv[a], &st) < 0 ||
            (text = mmap(0, st.st_size, PROT_READ,
                         MAP_SHARED, file, 0))
            == MAP_FAILED) {
            fprintf(stderr, "n: %s: ", argv[a]);
            perror("error"); continue;
        } else close(file);
        
        struct context context = {0};
        
        context.best = 0;
        context.name_count = 6;
        context.names = calloc(context.name_count, sizeof(struct name));
        context.name_count = 0;
        
        // _      param type.
        context.names[context.name_count].length = 1;
        context.names[context.name_count].signature = calloc(1, sizeof(size_t));
        context.names[context.name_count].signature[0] = '_';
        context.name_count++;
        
        // hello
        context.names[context.name_count].length = 5;
        context.names[context.name_count].signature = calloc(5, sizeof(size_t));
        context.names[context.name_count].signature[0] = 'h';
        context.names[context.name_count].signature[1] = 'e';
        context.names[context.name_count].signature[2] = 'l';
        context.names[context.name_count].signature[3] = 'l';
        context.names[context.name_count].signature[4] = 'o';
        context.name_count++;
        
        // my (x) is (y)
        context.names[context.name_count].length = 6;
        context.names[context.name_count].signature = calloc(6, sizeof(size_t));
        context.names[context.name_count].signature[0] = 'm';
        context.names[context.name_count].signature[1] = 'y';
        context.names[context.name_count].signature[2] = 256;
        context.names[context.name_count].signature[3] = 'i';
        context.names[context.name_count].signature[4] = 's';
        context.names[context.name_count].signature[5] = 256;
        context.name_count++;
        
        // (x) empty
        context.names[context.name_count].length = 6;
        context.names[context.name_count].signature = calloc(6, sizeof(size_t));
        context.names[context.name_count].signature[0] = 256;
        context.names[context.name_count].signature[1] = 'e';
        context.names[context.name_count].signature[2] = 'm';
        context.names[context.name_count].signature[3] = 'p';
        context.names[context.name_count].signature[4] = 't';
        context.names[context.name_count].signature[5] = 'y';
        context.name_count++;
        
        // (x) (y)
        context.names[context.name_count].length = 2;
        context.names[context.name_count].signature = calloc(2, sizeof(size_t));
        context.names[context.name_count].signature[0] = 256;
        context.names[context.name_count].signature[1] = 256;
        context.name_count++;
        
        // (x)
        context.names[context.name_count].length = 1;
        context.names[context.name_count].signature = calloc(1, sizeof(size_t));
        context.names[context.name_count].signature[0] = 256;
        context.name_count++;

        uint8_t* tokens = malloc(st.st_size);
        uint16_t* loc = malloc(4 * st.st_size);
        struct resolved* m = malloc(sizeof(struct resolved) * 65536);
        
        size_t count = lex(text, tokens, loc, st.st_size);
        size_t r = csr(tokens, count, m, &context);
        
        debug_context(&context);
        
        if (!m[r].index && count) {
            if (context.best == count) context.best--;
            fprintf(stderr,
                    "n: %s:%u:%u: error: unresolved %c\n\n",
                    argv[a],
                    loc[2 * context.best],
                    loc[2 * context.best + 1],
                    tokens[context.best]);
        } else if (!m[r].index && !count) {
            fprintf(stderr, "n: %s: error: unresolved empty file\n\n", argv[a]);
        }
                
        free(m);
        free(loc);
        free(tokens);
        munmap(text, st.st_size);
    }
}

