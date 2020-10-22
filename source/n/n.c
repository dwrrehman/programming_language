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
    size_t index;
    size_t parent;
    size_t begin;
    size_t done;
};

struct node {
    size_t index;
    size_t count;
    struct node* args;
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


void debug_list(struct unit* m, size_t size, size_t head, size_t tail) {
    puts("MEMORY: {\n");
    for (size_t i = 0; i < size; i++) {
        printf("   %5lu    %c%c   i:%-5lu    p:%-5lu    d:%-5lu    b:%-5lu   \n",
               i, i == head ? 'H' : ' ', i == tail ? 'T' : ' ',
                m[i].index, m[i].parent, m[i].done, m[i].begin);
    }
    puts("}\n");
}

void debug_string(const char* message, uint8_t* input, size_t count, size_t at) {
    printf("\n       %15s:  ", message);
    for (size_t i = 0; i < count; i++)
        printf(i == at ? "[%c] " : "%c ", input[i]);
    printf(" (%lu) \n", at);
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

static inline size_t parse(uint8_t* input, size_t length,
                           struct unit* list, size_t size,
                           struct context* context) {
    
    size_t head = 0, tail = 0;
    list[tail++] = (struct unit) {
        .parent = size,
        .begin = 0,
        .index = 0,
        .done = 0,
    };
    
    while (head != tail) {
        size_t at = head, done = 0, begin = list[head].begin;
        while (at != size) {
            struct name name = context->names[list[at].index];
            while (done < name.length) {
                const size_t c = name.signature[done++];
                if (c >= 256) {
                    for (size_t arg = 1; arg < context->name_count; arg++) {
                        if (tail == size) break;
                        list[tail++] = (struct unit) {
                            .index = arg,
                            .parent = at,
                            .done = done,
                            .begin = begin
                        };
                    }
                    goto next;
                } else if (c != input[begin]) {
                    list[head].index = -1;
                    goto next;
                }
                begin++;
                if (begin > context->best) context->best = begin;
            }
            done = list[at].done;
            at = list[at].parent;
        }
        if (begin == length) return head + 1;
        else next: head++;
    }
    return 0;
}



struct node construct(size_t head, struct unit* list, size_t start) {
    
    struct node root = {0};
    
    return root;
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
            "_",
            "join__",
            "hello",
            0
        };
        
        for (size_t i = 0; names[i]; i++)
            push_signature(names[i], &context);
        
        size_t count = lex(text, tokens, locations, st.st_size);
        size_t head = parse(tokens, count, memory, memory_size, &context);
        struct node root = construct(head, memory, 0);
        
        if (!head) debug_string("error at", tokens, count, context.best);
        else {
            debug_list(memory, head, 0,0);
            printf("head = %lu\n", head);
        }
        
        free(memory);
        free(locations);
        free(tokens);
        munmap(text, st.st_size);
    }
}
