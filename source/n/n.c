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
(uint8_t* input, size_t length, size_t type, struct unit* stack, size_t max, struct context* context) {
    struct name name;
    size_t top = 0;
    *stack = (struct unit) {type, 0, 0, 0, 0, NULL};
_0:
    if (stack[top].index >= context->index_count) {
        if (top--) goto _2;
        return 1;
    }
    size_t done = 0, begin = stack[top].begin;
_1:
    name = context->names[context->indicies[stack[top].index]];
    if (stack[top].type &&
        stack[top].type != name.type)
        goto _2;
    
    while (done < name.length) {
        size_t c = name.signature[done++];
        if (c >= 256 && top + 1 < max) {
            stack[top].args = realloc(stack[top].args, sizeof(struct unit) * (++(stack[top].count)));
            stack[++top] = (struct unit) {c - 256, 0, begin, done, 0, 0};
            goto _0;
        }
        if (c != input[begin])
            goto _2;
        if (++begin > context->best)
            context->best = begin;
    }
    if (top) {
        stack[top - 1].args[stack[top - 1].count - 1] = stack[top];
        done = stack[top--].done;
        goto _1;
    }
    if (begin == length)
        return 0;
_2:
    stack[top].index++;
    free(stack[top].args);
    stack[top].args = NULL;
    stack[top].count = 0;
    goto _0;
}

/**
    constructs the following context:
 ----------------------------------------------------------
 
 
      [0]  root              : (typeless)      the root type. this is the universe type, as as such doesnt have a type.
   
      [1]  init                  : root           the initial unit type.
 
      [2]  push_char (c)
     
      [2]  param (init)             : init     signifies a param. important, i think...
 
      [2]  decl (init) (root)       : init              extends the context. also acts as the param indicator.
                                               all signatures are not given a definition.
 
      [3]  join (init) (init)       : init              allows for multiple statements,
                                               and multi-character signatures.
                                               the analogy in lisp, is "(progn ...)".
              
[builtin]  <char>             : init              turns an unresolved char into resolved signature.
 
 
 
 ------------------------------------------------------------
 
 */
static inline void construct_a_context(struct context* context) {
    
    const char* root = "root"; size_t index_of_root = 0;
    const char* init = "init"; size_t index_of_init = 1;
    const char* temp = "temp"; size_t index_of_temp = 2;
    const char* decl = "decl_"; size_t index_of_decl = 3;
    const char* join = "join__";  size_t index_of_join = 4;
    
    context->best = 0;
    context->name_count = 0;
            
    context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
    context->names[context->name_count].type = -1;
    context->names[context->name_count].precedence = 0;
    context->names[context->name_count].codegen_as = 0;
    context->names[context->name_count].length = strlen(root);
    context->names[context->name_count].signature = calloc
    (context->names[context->name_count].length, sizeof(size_t));
    for (size_t i = 0; i < context->names[context->name_count].length; i++) {
        context->names[context->name_count].signature[i] = root[i];
    }
    context->name_count++;
        
    context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
    context->names[context->name_count].type = index_of_root;
    context->names[context->name_count].precedence = 0;
    context->names[context->name_count].codegen_as = 0;
    context->names[context->name_count].length = strlen(init);
    context->names[context->name_count].signature = calloc
    (context->names[context->name_count].length, sizeof(size_t));
    for (size_t i = 0; i < context->names[context->name_count].length; i++) {
        context->names[context->name_count].signature[i] = init[i];
    }
    context->name_count++;
    
    context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
    context->names[context->name_count].type = index_of_init;
    context->names[context->name_count].precedence = 0;
    context->names[context->name_count].codegen_as = 0;
    context->names[context->name_count].length = strlen(temp);
    context->names[context->name_count].signature = calloc
    (context->names[context->name_count].length, sizeof(size_t));
    for (size_t i = 0; i < context->names[context->name_count].length; i++) {
        context->names[context->name_count].signature[i] = temp[i];
    }
    context->name_count++;
    
    context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
    context->names[context->name_count].type = index_of_init;
    context->names[context->name_count].precedence = 0;
    context->names[context->name_count].codegen_as = 0;
    context->names[context->name_count].length = strlen(decl);
    context->names[context->name_count].signature = calloc
    (context->names[context->name_count].length, sizeof(size_t));
    for (size_t i = 0; i < context->names[context->name_count].length; i++) {
        context->names[context->name_count].signature[i] =
        decl[i] == '_' ? 256 + index_of_init : decl[i];
    }
    context->name_count++;
    
    context->names = realloc(context->names, sizeof(struct name) * (context->name_count + 1));
    context->names[context->name_count].type = index_of_init;
    context->names[context->name_count].precedence = 0;
    context->names[context->name_count].codegen_as = 0;
    context->names[context->name_count].length = strlen(join);
    context->names[context->name_count].signature = calloc
    (context->names[context->name_count].length, sizeof(size_t));
    for (size_t i = 0; i < context->names[context->name_count].length; i++) {
        context->names[context->name_count].signature[i] =
        join[i] == '_' ? 256 + index_of_init : join[i];
    }
    context->name_count++;
    
    context->index_count = 0;
    
    context->indicies = realloc(context->indicies, sizeof(size_t) * (context->index_count + 1));
    context->indicies[context->index_count++] = index_of_root;
    
    context->indicies = realloc(context->indicies, sizeof(size_t) * (context->index_count + 1));
    context->indicies[context->index_count++] = index_of_init;
    
    context->indicies = realloc(context->indicies, sizeof(size_t) * (context->index_count + 1));
    context->indicies[context->index_count++] = index_of_temp;
      
    context->indicies = realloc(context->indicies, sizeof(size_t) * (context->index_count + 1));
    context->indicies[context->index_count++] = index_of_decl;
    
    context->indicies = realloc(context->indicies, sizeof(size_t) * (context->index_count + 1));
    context->indicies[context->index_count++] = index_of_join;
    
}

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
                printf("(%lu) ", c);
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
        const size_t root_type = 0;
        
        struct context context = {0};
        construct_a_context(&context);
        debug_context(&context);

        
        uint8_t* tokens = malloc(sizeof(uint8_t) * st.st_size);
        uint16_t* locations = malloc(sizeof(uint16_t) * st.st_size * 2);
        struct unit* memory = malloc(sizeof(struct unit) * memory_size);
                
        const size_t count = lex(text, tokens, locations, st.st_size);
        const size_t error = parse(tokens, count, root_type, memory, memory_size, &context);
        
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
