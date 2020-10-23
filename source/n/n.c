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

struct unit {
    size_t type;
    size_t index;
    size_t begin;
    size_t done;
    size_t count;
    struct unit* args;
};

struct name {
    size_t* signature;
    size_t length;
    size_t type;
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


static inline void debug_list(struct unit* m, size_t size, size_t head, size_t tail) {
    puts("MEMORY: {\n");
    for (size_t i = 0; i < size; i++) {
        printf("   %5lu    %c%c   i:%-5lu    b:%-5lu     d:%-5lu    c:%-5lu    a:%p   \n",
               i, i == head ? 'H' : ' ', i == tail ? 'T' : ' ',
                m[i].index, m[i].done, m[i].begin, m[i].count, (void*) m[i].args);
    }
    puts("}\n");
}

static inline void debug_string(const char* message, uint8_t* input, size_t count, size_t at) {
    printf("\n       %15s:  ", message);
    for (size_t i = 0; i < count; i++)
        printf(i == at ? "[%c] " : "%c ", input[i]);
    printf(" (%lu) \n", at);
}

static inline void debug_tree(struct unit tree, size_t d) {
    for (size_t i = 0; i < d; i++)
        printf(".   ");
    printf("[index = %lu]\n", tree.index);
    for (size_t i = 0; i < tree.count; i++)
        debug_tree(tree.args[i], d + 1);
}

static void push_signature(const char* string, struct context* context) {
    const size_t n = context->name_count, length = strlen(string);
    context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
    context->names[n].signature = calloc(length, sizeof(size_t));
    for (size_t i = 0; i < length; i++)
        context->names[n].signature[i] = string[i] != '_' ? string[i] : 256;
    context->names[n].length = length;
    context->name_count++;
}

static inline size_t lex(uint8_t* text, uint8_t* tokens, uint16_t* locations, size_t length) {
    size_t count = 0;
    uint16_t l = 1, c = 1;
    for (size_t i = 0; i < length; i++) {
        if (text[i] > 32) {
            locations[2 * count] = l;
            locations[2 * count + 1] = c;
            tokens[count++] = text[i];
        }
        if (text[i] == 10) {
            l++;
            c = 1;
        } else c++;
    }
    return count;
}

static inline size_t parse
(uint8_t* input, size_t length, size_t type, struct unit* list, size_t max, struct context* context) {
    struct name name;
    size_t at = 0;
    *list = (struct unit) {type, 0, 0, 0, 0, NULL};
_0:
    if (list[at].index >= context->name_count) {
        if (at--) goto _2; return 1;
    }
    size_t done = 0, begin = list[at].begin;
_1:
    name = context->names[list[at].index];
    while (done < name.length) {
        size_t c = name.signature[done++];
        if (c >= 256 && at + 1 < max) {
            list[at].args = realloc(list[at].args, sizeof(struct unit) * (++(list[at].count)));
            list[++at] = (struct unit) {c, 0, begin, done, 0, 0};
            goto _0;
        } else if (c != input[begin]) goto _2;
        else if (++begin > context->best) context->best = begin;
    }
    if (at) {
        list[at - 1].args[list[at - 1].count - 1] = list[at];
        done = list[at--].done;
        goto _1;
    } else if (begin == length) return 0;
_2:
    list[at].index++;
    free(list[at].args);
    list[at].args = NULL;
    list[at].count = 0;
    goto _0;
}

int main(int argc, const char** argv) {
    for (int a = 1; a < argc; a++) {
        
        uint8_t* text = NULL;
        struct stat st = {0};
        int file = open(argv[a], O_RDONLY);
        if (file < 0 || stat(argv[a], &st) < 0 ||
            (text = mmap(0, st.st_size, PROT_READ, MAP_SHARED, file, 0)) == MAP_FAILED) {
            fprintf(stderr, "n: %s: ", argv[a]);
            perror("error"); continue;
        } else close(file);
        
        const size_t memory_size = 128;
        
        uint8_t* tokens = malloc(sizeof(uint8_t) * st.st_size);
        uint16_t* locations = malloc(sizeof(uint16_t) * st.st_size * 2);
        struct unit* memory = malloc(sizeof(struct unit) * memory_size);
        
        struct context context = {0};
        
        const char* names[] = {
            "init",
            "hello",
            "join__",
        0};
        
        const size_t type = 0;
        
        for (size_t i = 0; names[i]; i++)
            push_signature(names[i], &context);
                
        size_t count = lex(text, tokens, locations, st.st_size);
        size_t error = parse(tokens, count, type, memory, memory_size, &context);
        
        if (error) {
            if (context.best == count) context.best--;
            printf("%s: %u:%u: error: unresolved \"%c\"\n",
                   argv[a],
                   locations[2 * context.best],
                   locations[2 * context.best + 1],
                   tokens[context.best]);
        }
        
        debug_tree(*memory, 0);
        
        free(memory);
        free(locations);
        free(tokens);
        munmap(text, st.st_size);
    }
}
