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
    size_t ind;
    size_t type;
    size_t index;
    size_t begin;
    size_t done;
    size_t count;
    struct unit* args;
};

struct name {
    size_t length;
    size_t type;
    size_t* signature;
    size_t codegen_as;
    size_t precedence;
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

enum intrinsics {
    intrin_root = 0,
    intrin_init = 1,
    
    intrin_pop = 2,
    intrin_push = 3,
    intrin_char = 4,          // goes last in the symbol table always.
    intrin_param = 5,
    intrin_join = 6, // (init) (init)
    intrin_decl = 7, // (root)
};


static inline void print_vector(size_t* v, size_t length) {
    printf("{ ");
    for (size_t i = 0; i < length; i++) {
        printf("%lu ", v[i]);
    }
    printf("}\n");
}

static inline void debug_tree(struct unit tree, size_t d, struct context* context) {
    for (size_t i = 0; i < d; i++)
        printf(".   ");
    
    struct name name = context->names[tree.index];
    
    for (size_t i = 0; i < name.length; i++) {
        if (name.signature[i] < 256)
            printf("%c", (char) name.signature[i]);
        else printf(" (%lu) ", name.signature[i]);            
    }
    printf(" :: [ind=%ld, index=%lu : type=%lu]\n\n", tree.ind, tree.index, tree.type);
    for (size_t i = 0; i < tree.count; i++)
        debug_tree(tree.args[i], d + 1, context);
}

static inline void debug_context(struct context* context) {
    printf("---------------- context --------------\n");
    printf("best = %lu\n", context->best);
    
    printf("-------- names --------\n");
    printf("name_count = %lu\n", context->name_count);
    for (size_t i = 0; i < context->name_count; i++) {
        printf("----- [%lu] ----- \n", i);
        printf("\t type = %lu\n", context->names[i].type);
        printf("\t precedence = %lu\n", context->names[i].precedence);
        printf("\t codegen_as = %lu\n", context->names[i].codegen_as);
        printf("\t length = %lu\n", context->names[i].length);
        printf("\t signature:     ");
        for (size_t s = 0; s < context->names[i].length; s++) {
            const size_t c = context->names[i].signature[s];
            if (c >= 256) {
                printf("(%lu) ", c - 256);
            } else {
                printf("%c ", (char) c);
            }
        }
        printf("\n\n");
    }
    printf("-------------------\n\n");
        
    printf("---------- indicies --------\n");
    printf("index_count = %lu\n", context->index_count);
    printf("indicies:     { ");
    for (size_t i = 0; i < context->index_count; i++) {
        printf("%lu ", context->indicies[i]);
    }
    printf("}\n\n");
    
    printf("---------- frames --------\n");
    printf("frame_count = %lu\n", context->frame_count);
    printf("frames:     { ");
    for (size_t i = 0; i < context->frame_count; i++) {
        printf("%lu ", context->frames[i]);
    }
    printf("}\n\n");
    
    printf("---------- owners --------\n");
    printf("(owner)frame_count = %lu\n", context->frame_count);
    for (size_t i = 0; i < context->frame_count; i++) {
        printf("----- owner [frame=%lu] ----- \n", i);
        printf("\t type = %lu\n", context->owners[i].type);
        printf("\t precedence = %lu\n", context->owners[i].precedence);
        printf("\t codegen_as = %lu\n", context->owners[i].codegen_as);
        printf("\t length = %lu\n", context->owners[i].length);
        printf("\t signature:     ");
        for (size_t s = 0; s < context->owners[i].length; s++) {
            const size_t c = context->owners[i].signature[s];
            if (c >= 256) {
                printf("(%lu) ", c - 256);
            } else {
                printf("%c ", (char) c);
            }
        }
        printf("\n\n");
    }
    printf("-------------------\n\n");
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

static inline void do_intrinsic(struct context* context, struct unit* stack,
                                size_t top) {
    
    size_t index = stack[top].index;
    
    if (index == intrin_pop) {
        if (!context->frame_count) {
            printf("error: no more stack frames to pop.\n");
            abort();
        }
        context->index_count = context->frames[--context->frame_count];
        
    } else if (index == intrin_push) {
        context->frames = realloc(context->frames, sizeof(size_t) * (context->frame_count + 1));
        context->frames[context->frame_count] = context->index_count;
        
        context->owners = realloc(context->owners, sizeof(struct name) * (context->frame_count + 1));
        context->owners[context->frame_count++] = (struct name) {
            .length = 0,
            .type = 0,
            .signature = NULL,
            .codegen_as = 0,
            .precedence = 0
        };
    }
    
    else if (index == intrin_decl) {
        
        debug_context(context);
        
//        abort();
        
        if (!context->frame_count) {
            printf("error: no stack frames to declare signature into.\n");
            abort();
        }
        
        if (stack[top].count) context->owners[context->frame_count].type = stack[top].args[0].index;
        context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
        context->names[context->name_count] = context->owners[context->frame_count];
        
        size_t i = context->index_count;
        while (i-- > context->frames[context->frame_count - 1])
            if (context->names[context->name_count].length
                >=
                context->names[context->indicies[i]].length) break;
        i++;
        
        printf("insertion point (i) = %lu\n", i);
        
        context->indicies = realloc(context->indicies,
                                    sizeof(size_t) * (context->index_count + 1));

        printf("before insertion: ");
        print_vector(context->indicies, context->index_count);
        
        memmove(context->indicies + i + 1, context->indicies + i,
               sizeof(size_t) * (context->index_count - i));
        
        printf("after move: ");
        print_vector(context->indicies, context->index_count);
        printf("inserting: %lu\n", context->name_count);

        context->indicies[i] = context->name_count++;
        context->index_count++;
        
        printf("after insertion: ");
        print_vector(context->indicies, context->index_count);
                
        printf("updating stack IND indicies:\n");
        
        for (size_t stack_level = 0; stack_level <= top; stack_level++)
            if (i <= stack[stack_level].ind)
                stack[stack_level].ind++;
    }
    
    else if (index == intrin_param) {
        if (!context->frame_count) {
            printf("error: cannot add param from top level stack frame.\n");
            abort();
        }
        context->owners[context->frame_count - 1].signature
        = realloc(context->owners[context->frame_count - 1].signature, sizeof(size_t)
                  * (context->owners[context->frame_count - 1].length + 1));
        context->owners[context->frame_count - 1]
        .signature[context->owners[context->frame_count - 1].length++] = 256 + context->owners[context->frame_count].type;
    }
}

static inline void push_literal(size_t c, struct context* context) {
    context->owners[context->frame_count - 1].signature
    = realloc(context->owners[context->frame_count - 1].signature, sizeof(size_t)
              * (context->owners[context->frame_count - 1].length + 1));
    context->owners[context->frame_count - 1]
    .signature[context->owners[context->frame_count - 1].length++] = c;
}

static inline size_t parse
(uint8_t* input, size_t length, size_t type, struct unit* stack, size_t max, struct context* context) {
    struct name name;
    size_t top = 0, done = 0, begin = 0;
    *stack = (struct unit) {context->index_count, type, 0, 0, 0, 0, NULL};
_0:
    if (!stack[top].ind--) {
        if (!top) return 1;
        if (stack[top].type == intrin_init && begin < length && context->frame_count) {
            begin = stack[top].begin;
            stack[top].index = intrin_char;
            push_literal(input[begin], context);
            if (begin++ > context->best) context->best = begin;
            goto _2;
        }
        top--;
        goto _3;
    }
    done = 0; begin = stack[top].begin;
_1:
    stack[top].index = context->indicies[stack[top].ind];
    name = context->names[stack[top].index];
    
    if (stack[top].type && stack[top].type != name.type)
        goto _3;
    
    while (done < name.length) {
        size_t c = name.signature[done++];
        if (c >= 256 && top + 1 < max) {
            stack[top].args = realloc(stack[top].args, sizeof(struct unit) * (++(stack[top].count)));
            stack[++top] = (struct unit) {context->index_count, c - 256, 0, begin, done, 0, 0};
            goto _0;
        }
        if (c != input[begin])
            goto _3;
        if (++begin > context->best)
            context->best = begin;
    }
_2:
    if (top) {
        do_intrinsic(context, stack,top);
        stack[top - 1].args[stack[top - 1].count - 1] = stack[top];
        done = stack[top--].done;
        goto _1;
    }
    if (begin == length) return 0;
_3:
    free(stack[top].args);
    stack[top].args = NULL;
    stack[top].count = 0;
    goto _0;
}










static inline void construct_a_context(struct context* context) {
 
    context->best = 0;
    context->name_count = 0;
    context->index_count = 0;
    context->frame_count = 0;
    
    context->owners = realloc(context->owners, sizeof(struct name) * (context->frame_count + 1));
    context->owners[context->frame_count] = (struct name) {0};
    
    context->frames = realloc(context->frames, sizeof(size_t) * (context->frame_count + 1));
    context->frames[context->frame_count++] = context->index_count;
    
    context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
    context->names[context->name_count].type = intrin_root;
    context->names[context->name_count].precedence = 0;
    context->names[context->name_count].codegen_as = 0;
    context->names[context->name_count].length = 4;
    context->names[context->name_count].signature = calloc
    (context->names[context->name_count].length, sizeof(size_t));
    context->names[context->name_count].signature[0] = 'r';
    context->names[context->name_count].signature[1] = 'o';
    context->names[context->name_count].signature[2] = 'o';
    context->names[context->name_count].signature[3] = 't';
    context->name_count++;
        
    context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
    context->names[context->name_count].type = intrin_root;
    context->names[context->name_count].precedence = 0;
    context->names[context->name_count].codegen_as = 0;
    context->names[context->name_count].length = 4;
    context->names[context->name_count].signature = calloc(context->names[context->name_count].length, sizeof(size_t));
    context->names[context->name_count].signature[0] = 'i';
    context->names[context->name_count].signature[1] = 'n';
    context->names[context->name_count].signature[2] = 'i';
    context->names[context->name_count].signature[3] = 't';
    context->name_count++;
    
    context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
    context->names[context->name_count].type = intrin_init;
    context->names[context->name_count].precedence = 0;
    context->names[context->name_count].codegen_as = 0;
    context->names[context->name_count].length = 3;
    context->names[context->name_count].signature = calloc(context->names[context->name_count].length, sizeof(size_t));
    context->names[context->name_count].signature[0] = 'p';
    context->names[context->name_count].signature[1] = 'o';
    context->names[context->name_count].signature[2] = 'p';
    context->name_count++;
    
    context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
    context->names[context->name_count].type = intrin_init;
    context->names[context->name_count].precedence = 0;
    context->names[context->name_count].codegen_as = 0;
    context->names[context->name_count].length = 4;
    context->names[context->name_count].signature = calloc(context->names[context->name_count].length, sizeof(size_t));
    context->names[context->name_count].signature[0] = 'p';
    context->names[context->name_count].signature[1] = 'u';
    context->names[context->name_count].signature[2] = 's';
    context->names[context->name_count].signature[3] = 'h';
    context->name_count++;
    
    
    context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
    context->names[context->name_count].type = intrin_init;
    context->names[context->name_count].precedence = 0;
    context->names[context->name_count].codegen_as = 0;
    context->names[context->name_count].length = 4;
    context->names[context->name_count].signature = calloc(context->names[context->name_count].length, sizeof(size_t));
    context->names[context->name_count].signature[0] = 'c';
    context->names[context->name_count].signature[1] = 'h';
    context->names[context->name_count].signature[2] = 'a';
    context->names[context->name_count].signature[3] = 'r';
    context->name_count++;
    
    
    context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
    context->names[context->name_count].type = intrin_init;
    context->names[context->name_count].precedence = 0;
    context->names[context->name_count].codegen_as = 0;
    context->names[context->name_count].length = 5;
    context->names[context->name_count].signature = calloc(context->names[context->name_count].length, sizeof(size_t));
    context->names[context->name_count].signature[0] = 'p';
    context->names[context->name_count].signature[1] = 'a';
    context->names[context->name_count].signature[2] = 'r';
    context->names[context->name_count].signature[3] = 'a';
    context->names[context->name_count].signature[4] = 'm';
    context->name_count++;
    
    context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
    context->names[context->name_count].type = intrin_init;
    context->names[context->name_count].precedence = 0;
    context->names[context->name_count].codegen_as = 0;
    context->names[context->name_count].length = 6;
    context->names[context->name_count].signature = calloc(context->names[context->name_count].length, sizeof(size_t));
    context->names[context->name_count].signature[0] = 'j';
    context->names[context->name_count].signature[1] = 'o';
    context->names[context->name_count].signature[2] = 'i';
    context->names[context->name_count].signature[3] = 'n';
    context->names[context->name_count].signature[4] = intrin_init + 256;
    context->names[context->name_count].signature[5] = intrin_init + 256;
    context->name_count++;
    
    context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
    context->names[context->name_count].type = intrin_init;
    context->names[context->name_count].precedence = 0;
    context->names[context->name_count].codegen_as = 0;
    context->names[context->name_count].length = 5;
    context->names[context->name_count].signature = calloc(context->names[context->name_count].length, sizeof(size_t));
    context->names[context->name_count].signature[0] = 'd';
    context->names[context->name_count].signature[1] = 'e';
    context->names[context->name_count].signature[2] = 'c';
    context->names[context->name_count].signature[3] = 'l';
    context->names[context->name_count].signature[4] = intrin_root + 256;
    context->name_count++;
    
    
    
    context->indicies = realloc(context->indicies, sizeof(size_t) * (context->index_count + 1));
    context->indicies[context->index_count++] = intrin_pop;

    context->indicies = realloc(context->indicies, sizeof(size_t) * (context->index_count + 1));
    context->indicies[context->index_count++] = intrin_root;
    
    context->indicies = realloc(context->indicies, sizeof(size_t) * (context->index_count + 1));
    context->indicies[context->index_count++] = intrin_init;
    
    
    context->indicies = realloc(context->indicies, sizeof(size_t) * (context->index_count + 1));
    context->indicies[context->index_count++] = intrin_push;
    
    context->indicies = realloc(context->indicies, sizeof(size_t) * (context->index_count + 1));
    context->indicies[context->index_count++] = intrin_char;
    
    context->indicies = realloc(context->indicies, sizeof(size_t) * (context->index_count + 1));
    context->indicies[context->index_count++] = intrin_param;
    
    
    context->indicies = realloc(context->indicies, sizeof(size_t) * (context->index_count + 1));
    context->indicies[context->index_count++] = intrin_decl;
    
    
    context->indicies = realloc(context->indicies, sizeof(size_t) * (context->index_count + 1));
    context->indicies[context->index_count++] = intrin_join;
    
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
        const size_t root_type = intrin_root;
        
        struct context context = {0};
        construct_a_context(&context);
        debug_context(&context);
        
        uint8_t* tokens = malloc(sizeof(uint8_t) * st.st_size);
        uint16_t* locations = malloc(sizeof(uint16_t) * st.st_size * 2);
        struct unit* memory = malloc(sizeof(struct unit) * memory_size);
        
        const size_t count = lex(text, tokens, locations, st.st_size);
        const size_t error = parse(tokens, count, root_type, memory, memory_size, &context);
        
        debug_context(&context);
        
        if (error) {
            if (!count) printf("%s: error: unresolved empty file\n", argv[a]);
            else if (context.best == count) printf("%s: error: unresolved EOF\n", argv[a]);
            else
                printf("%s: %u:%u: error: unresolved \"%c\"\n",
                       argv[a],
                       locations[2 * context.best],
                       locations[2 * context.best + 1],
                       tokens[context.best]);            
        }
        
        debug_tree(*memory, 0, &context);
        
        free(memory);
        free(locations);
        free(tokens);
        munmap(text, st.st_size);
    }
}
