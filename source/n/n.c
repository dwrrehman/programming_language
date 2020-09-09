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
    size_t done;      // done and index can be merged.
    size_t parent;
    size_t queue_next;
    size_t count;
    size_t args[64];
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


static void print_resolved_data(struct resolved given) {
    printf("   :   { i:%-5lu b:%-5lu d:%-5lu p:%-5lu qn:%-5lu cnt:%-5lu } : [ ",
           given.index, given.begin, given.done, given.parent, given.queue_next, given.count);
    for (size_t i = 0; i < given.count; i++) printf("%lu ", given.args[i]);
    printf("]\n");
}

static void debug_resolved(struct resolved* memory, struct context* context, size_t r, size_t depth) {
    const struct resolved given = memory[r];
    for (size_t i = depth; i--;) printf(".   ");
    int printed = 0;
    if (r) {
        if (given.index >= context->name_count) printf("ERROR IN SIG INDEX");
        else for (size_t s = 0; s < context->names[given.index].length; s++) {
            size_t c = context->names[given.index].signature[s];
            if (c >= 256) printf("_"); else printf("%c", (char) c);
            printed++;
        }
    } else {
            printf("?");
            printed++;
        }
    for (int j = 0; j < abs(30 - printed); j++) printf(" ");
    print_resolved_data(given);
    for (size_t i = 0; i < given.count; i++) {
        for (size_t i = depth + 1; i--;) printf(".   ");
        printf("#%lu: \n", i);
        debug_resolved(memory, context, given.args[i], depth + 2);
    }
}

static void debug_context(struct context* context) {
    printf("\n----- context names: ------ \n{\n");
    for (size_t i = 0; i < context->name_count; i++) {
        printf("\t%6lu  :    ", i);
        struct name name = context->names[i];
        for (size_t s = 0; s < name.length; s++)
            printf("%c",
                   name.signature[s] < 256
                    ? (char) name.signature[s]
                    : '_');
        puts("\n");
    }
    puts("}\n");
}

static void debug_memory(struct resolved* m, struct context* context,
                         size_t head, size_t tail, size_t next, size_t count) {
    const int max_name_length = 32;
    printf("-------- memory -----------------------------------------------------------------------\n");
    for (size_t i = 0; i < count; i++) {
        const struct resolved given = m[i];
        printf("%5lu     %c%c%c   :   ", i, i == head ? 'H' : ' ', i == tail ? 'T' : ' ', i == next ? 'N' : ' ');
        int printed = 0;
        if (given.index < context->name_count) {
            for (size_t s = 0; s < context->names[given.index].length; s++) {
                size_t c = context->names[given.index].signature[s];
                if (c >= 256) printf("_"); else printf("%c", (char) c);
                printed++;
            }
        } else {
            printf("////");
            printed += 4;
        }
        for (int j = 0; j < abs(max_name_length - printed); j++) printf(" ");
        if (given.index < context->name_count) print_resolved_data(given);
        else printf("\n");
    }
    printf("-------------------------------------------------------------------------------------------\n");
}

static void display_signature(size_t* signature, size_t at, size_t length) {
    printf("\n         signature: ");
    for (size_t i = 0; i < length + 1; i++)
        if (i < length) printf(i == at ? "[%c] " : "%c ",
                               (signature[i] < 256) ? (char) signature[i] : '_');
        else printf(i == at ? "[%c] " : "%c ", '\0');
    puts("\n");
}

static void display_string_at_char(uint8_t* input, size_t length, size_t at) {
    printf("\n            string:  ");
    for (size_t i = 0; i < length; i++)
        printf(i == at ? "[%c] " : "%c ", input[i]);
    puts("\n");
}

static void print_result(uint8_t* tokens, size_t length, struct resolved* memory, size_t solution, struct context* context,
                         const char* filename, uint16_t* locations) {
        
    printf("\n\n\n\nsolution = %lu\n", solution);
    debug_resolved(memory, context, solution, 0);
    printf("\n\n\n\n");
    
    if (!length) fprintf(stderr, "n: %s: error: unresolved empty file\n\n", filename);
    else if (!solution) {
        size_t b = context->best;        
        if (b == length) {
            b--;
            fprintf(stderr, "n: %s:%u:%u: error: unresolved end of expression\n\n", filename, locations[2 * b], locations[2 * b + 1]);
            display_string_at_char(tokens, length, b);
        } else {
             fprintf(stderr, "n: %s:%u:%u: error: unresolved %c\n\n", filename, locations[2 * b], locations[2 * b + 1], tokens[b]);
             display_string_at_char(tokens, length, b);
        }
    } else printf("\n\t [parse successful]\n\n");
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
                while (me.done < name.length) {
                    size_t c = name.signature[me.done++];
                    if (c >= 256 && next + 2 < size) {
                        struct resolved child = {.begin = me.begin, .parent = next + 1};
                        me.args[me.count++] = next;  // this is definitely wrong.
                        memory[tail].queue_next = next; tail = next;
                        memory[next++] = child;
                        memory[next++] = me;     // this seems wrong.
                    }
                    if (c >= 256 || c != (size_t) given[me.begin]) goto skip;
                    me.begin++;
                    if (me.begin > context->best) context->best = me.begin;
                }
                size_t p = me.parent;
                if (!p) { if (me.begin == length) return at; else break; }
                memory[p].begin = me.begin;
                memory[p].args[memory[p].count - 1] = next;
                if (next + 1 < size) memory[next++] = me;
            } skip: continue;
        }
        memory[head].index = 0;
    }
    return 0;
}



















int main(int argc, const char** argv) {
    for (int a = 1; a < argc; a++) {
        
        uint8_t* text = NULL;
        struct stat st = {0};
        int file = open(argv[a], O_RDONLY);
        if (file < 0 || stat(argv[a], &st) < 0 ||
            (text = mmap(0, st.st_size, PROT_READ,
                         MAP_SHARED, file, 0))
            == MAP_FAILED) {
            fprintf(stderr, "n: %s: ", argv[a]);
            perror("error"); continue;
        } else close(file);
        
        const size_t memory_size = 1024;
        
        uint8_t* tokens = malloc(sizeof(uint8_t) * st.st_size);
        uint16_t* locations = malloc(sizeof(uint16_t) * st.st_size * 2);
        struct resolved* memory = malloc(sizeof(struct resolved) * memory_size);
        memset(memory, 0, sizeof(struct resolved) * 2);
        
        struct context context = {0};
        const char* names[] = {
            "-",
            "join__",
            "john",
            "+__",
            "_iscool",
            "print_",
            "3",
            "5",
            "(_)",
            "hello__",
        0};
        
        for (size_t i = 0; names[i]; i++) push_signature(names[i], &context);
        debug_context(&context);
        
        size_t count = lex(text, tokens, locations, st.st_size);
        size_t solution = parse(tokens, count, memory_size, memory, &context);
        print_result(tokens, count, memory, solution, &context, argv[a], locations);
        debug_memory(memory, &context, 0, 0, 0, memory_size);
        
        for (size_t i = 0; i < context.name_count; i++)
            free(context.names[i].signature);
        free(context.names);
                
        free(memory);
        free(locations);
        free(tokens);
        munmap(text, st.st_size);
    }
}
