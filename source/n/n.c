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
    size_t begin;
    size_t done;
    size_t parent;
    size_t queue_next;
    size_t count;
    size_t args[64];    // FIX ME
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
            l++; c = 1;
        } else c++;
    }
    return count;
}

static inline size_t parse(uint8_t* given, size_t length, size_t size,
                           struct resolved* memory, struct context* context) {
     for (size_t next = 2, head = 1, tail = 1; head; head = memory[head].queue_next) {
         for (; memory[head].index < context->name_count; memory[head].index++) {
             for (size_t at = head; at; at = memory[at].parent) {
                 struct resolved me = memory[at];
                 struct name name = context->names[me.index];
                 for (; me.done < name.length; ) {
                     size_t c = name.signature[me.done++];
                     if (c >= 256 && next + 2 < size) {
                         struct resolved child = {.begin = me.begin, .parent = next + 1};
                         me.args[me.count++] = next;
                         memory[tail].queue_next = next; tail = next;
                         memory[next++] = child; memory[next++] = me;
                     }
                     if (c >= 256 || c != (size_t) given[me.begin]) goto skip;
                     me.begin++; context->best = me.begin > context->best ? me.begin : context->best;
                 }
                 size_t p = me.parent;
                 if (!p) {
                     if (me.begin == length) return at; else break;
                 }
                 memory[p].begin = me.begin;
                 memory[p].args[memory[p].count - 1] = next;
                 if (next + 1 < size) memory[next++] = me;
             } skip: continue;
         }
     }
     return 0;
 }

int main(int argc, const char** argv) {
    for (int a = 1; a < argc; a++) {
        
        uint8_t* text = NULL;
        struct stat st = {0};
        int file = open(argv[a], O_RDONLY);
        if (file < 0 ||
            stat(argv[a], &st) < 0 ||
            (text = mmap(0, st.st_size, PROT_READ,
                         MAP_SHARED, file, 0))
            == MAP_FAILED) {
            fprintf(stderr, "n: %s: ", argv[a]);
            perror("error"); continue;
        } else close(file);
        
        struct context context = {0};      // load_context(&context);
        
        uint8_t* tokens = malloc(sizeof(uint8_t) * st.st_size);
        uint16_t* loc = malloc(sizeof(uint16_t) * st.st_size * 2);
        struct resolved* memory = malloc(sizeof(struct resolved) * 65536);
        memset(memory, 0, sizeof(struct resolved) * 2);
        
        size_t count = lex(text, tokens, loc, st.st_size);
        size_t solution = parse(tokens, count, 256, memory, &context);
        
        printf("r = %lu\n", solution);
            
        if (!count) fprintf(stderr, "n: %s: error: unresolved empty file\n\n", argv[a]);
        else if (!solution) {
            if (context.best == count) context.best--;
            fprintf(stderr,
                    "n: %s:%u:%u: error: unresolved %c\n\n", argv[a],
                    loc[2 * context.best], loc[2 * context.best + 1], tokens[context.best]);
        }
        
        free(memory);
        free(loc);
        free(tokens);
        munmap(text, st.st_size);
    }
}
