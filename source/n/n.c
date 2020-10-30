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

static const size_t stack_size = 1024;
static const size_t top_level_type = 0;

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
    struct unit definition;
};

struct context {
    size_t frame_count;
    size_t index_count;
    size_t name_count;
    size_t* frames;
    size_t* indicies;
    struct name* names;
    struct name* owners;
};

enum intrinsics {
    
    intrin_root = 256,
    intrin_init,
    intrin_char,
//    intrin_compiletime, ///   ct (i)
//    intrin_eval, ///         eval (T: init) (e: T) -> compiletime<T>
   
    intrin_join, ///       join (i) (i) -> i
    intrin_decl, ///       decl (name: char) (type: (0)) -> i
    
    
    
    
    
    
    
    
    
    
    
    
    // ------------------------
    /// UD SIGS:
    intrin_param,
    intrin_macro,
    intrin_define__arg0,
    intrin_define,
};

enum codegen_type {
    cg_function,
    cg_struct,
    cg_variable,
    cg_macro,
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

    
    if (tree.index >= 256) {
        struct name name = context->names[tree.index - 256];

        for (size_t i = 0; i < name.length; i++) {
            if (name.signature[i] < 256)
                printf("%c", (char) name.signature[i]);
            else printf(" (%lu) ", name.signature[i]);
        }
    } else {
        printf("CHARACTER{%c}", (char) tree.index);
    }
    
    
    printf(" :: [ind=%ld, index=%lu : type=%lu, begin=%lu, done=%lu, count=%lu]\n\n", tree.ind, tree.index, tree.type, tree.begin, tree.done, tree.count);
    for (size_t i = 0; i < tree.count; i++)
        debug_tree(tree.args[i], d + 1, context);
}

static inline void debug_context(struct context* context) {
    printf("---------------- context --------------\n");
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
        debug_tree(context->names[i].definition, 0, context);
    }
    printf("-------------------\n\n");

    printf("---------- indicies --------\n");
    printf("index_count = %lu\n", context->index_count);
    printf("indicies:     ");
    print_vector(context->indicies, context->index_count);
    printf("\n");

    printf("---------- frames --------\n");
    printf("frame_count = %lu\n", context->frame_count);
    printf("frames:     ");
    print_vector(context->frames, context->frame_count);
    printf("\n");

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
                printf("(%lu) ", c);
            } else {
                printf("%c ", (char) c);
            }
        }
        printf("\n\n");
    }
    printf("-------------------\n\n");
}

static inline void push_literal(size_t push, struct context* c) {
    c->owners[c->frame_count - 1].signature = realloc
    (c->owners[c->frame_count - 1].signature,
     sizeof(size_t) * (c->owners[c->frame_count - 1].length + 1));
    c->owners[c->frame_count - 1].signature
    [c->owners[c->frame_count - 1].length++] = push;
}

static inline struct unit duplicate(struct unit this) {
    struct unit dup = this;
    dup.args = malloc(sizeof(struct unit) * this.count);
    for (size_t i = 0; i < this.count; i++)
        dup.args[i] = duplicate(this.args[i]);
    return dup;
}

static inline void replace_all_occurences(size_t parameter, struct unit argument, struct unit* tree) {
    for (size_t i = 0; i < tree->count; i++)
        replace_all_occurences(parameter, argument, tree->args + i);
    if (tree->index == parameter) {
        tree->index = argument.index;
        tree->count = argument.count;
        tree->args = argument.args;
    }
}

static inline void expand_macro(struct context* c, struct unit* stack, size_t top) {
    struct name function = c->names[stack[top].index - 256];
    if (function.codegen_as == cg_macro) {
        struct unit new = duplicate(function.definition);
        const size_t arity = stack[top].count;
        for (size_t a = 0; a < arity; a++) {
            struct unit argument = stack[top].args[a];
            size_t parameter = stack[top].index - arity + a;
            replace_all_occurences(parameter, argument, &new);
        }
        stack[top].index = new.index;
        stack[top].args = new.args;
        stack[top].count = new.count;
    }
}

static inline size_t do_intrinsic(struct context* c, struct unit* stack, size_t top) {
    
    const size_t index = stack[top].index;
//
//    if (index == intrin_pop) {
//        c->index_count = c->frames[--c->frame_count];
//
//    } else if (index == intrin_push) {
//
//        c->frames = realloc(c->frames, sizeof(size_t) * (c->frame_count + 1));
//        c->frames[c->frame_count] = c->index_count;
//        c->owners = realloc(c->owners, sizeof(struct name) * (c->frame_count + 1));
//        c->owners[c->frame_count++] = (struct name) {0};
//
//    } else
//        
    if (index == intrin_decl) {
        
        if (stack[top].count) c->owners[c->frame_count - 1].type = stack[top].args[0].index;
        c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
        c->names[c->name_count] = c->owners[c->frame_count - 1];
        
        size_t i = c->frames[c->frame_count - 1];
        while (i-- > c->frames[c->frame_count - 2])
            if (c->names[c->name_count].length >= c->names[c->indicies[i]].length) break;
        i++;
        
        c->indicies = realloc(c->indicies, sizeof(size_t) * (c->index_count + 1));
        memmove(c->indicies + i + 1, c->indicies + i, sizeof(size_t) * (c->index_count - i));
        c->indicies[i] = c->name_count++;
        c->index_count++;
        
        for (size_t s = 0; s <= top; s++)
            if (i <= stack[s].ind) stack[s].ind++;
        
        c->frames[c->frame_count - 1]++;
    }
    
    else if (index == intrin_param) {
        push_literal(c->owners[c->frame_count].type, c);
        
    } else if (index == intrin_define) {
        
        size_t
            expected = c->names[c->name_count - 1].type,
            actual = c->names[stack[top].args[0].index].type;
        
        if (expected != actual) {
            printf("error: attached definition does not match return type: %lu != %lu\n", expected, actual);
            return 2;
            ///TODO: make this error impossible.
        }
                
        c->names[c->name_count - 1].definition = stack[top].args[0];
        
    } else if (index == intrin_macro) {
        c->names[c->name_count - 1].codegen_as = cg_macro;
    }
    return 0;
}

static inline struct unit compile(const char* filename, uint8_t* input, size_t length, struct context* context) {
    
    struct unit program = {0};
    struct name name = {0};
    size_t top = 0, done = 0, begin = 0, best = 0, line = 1, column = 1;
    
    while (begin < length && input[begin] <= ' ') {
        if (input[begin] == '\n') { line++; column = 1; } else column++;
        begin++;
    }

    struct unit* stack = malloc(sizeof(struct unit) * stack_size);
    stack[0] = (struct unit) {context->index_count, top_level_type, 0, 0, 0, 0, NULL};

_0:
    if (!stack[top].ind--) {
        if (!top) goto _4;
        if (stack[top].type == intrin_char && begin < length && context->frame_count) {
            begin = stack[top].begin;
            stack[top].index = input[begin];
            column++;
            if (++begin > best) best = begin;
            while (begin < length && input[begin] <= ' ') {
                if (input[begin] == '\n') { line++; column = 1; } else column++;
                if (++begin > best) best = begin;
            }
            goto _2;
        }
        top--;
        goto _3;
    }
    done = 0; begin = stack[top].begin;
_1:
    stack[top].index = context->indicies[stack[top].ind];
    name = context->names[stack[top].index - 256];
    
    if (stack[top].type && stack[top].type != name.type) ///TODO: make equality of trees.
        goto _3;
    
    while (done < name.length) {
        size_t c = name.signature[done++];
        if (c >= 256 && top + 1 < stack_size) {
            stack[top].args = realloc(stack[top].args, sizeof(struct unit) * (++(stack[top].count)));
            stack[++top] = (struct unit) {context->index_count, c, 0, begin, done, 0, 0};
            goto _0;
        }
        if (c != input[begin]) goto _3;
        column++;
        if (++begin > best) best = begin;
        while (begin < length && input[begin] <= ' ') {
            if (input[begin] == '\n') { line++; column = 1; } else column++;
            if (++begin > best) best = begin;
        }
    }
_2:
    if (top) {
        expand_macro(context, stack, top);
//        if (stack[top].index == intrin_eval) {
//            evaluate_intrinsic(context, stack, top)) goto _3;
//        }
        stack[top - 1].args[stack[top - 1].count - 1] = stack[top];
        done = stack[top--].done;
        goto _1;
    }
    if (begin == length) {
        program = stack[top];
        goto _4;
    }
_3:
    free(stack[top].args);
    stack[top].args = NULL;
    stack[top].count = 0;
    goto _0;
_4:
    free(stack);
    if (!program.index)
        printf("%s: %lu:%lu: error: unresolved %c\n",
               filename, line, column,
               best == length ? ' ' : input[best]);
    return program;
}

static inline void construct_a_context(struct context* c) {
    
    c->name_count = 0;
    c->index_count = 0;
    c->frame_count = 0;
    
    c->owners = realloc(c->owners, sizeof(struct name) * (c->frame_count + 1));
    c->owners[c->frame_count] = (struct name) {0};
    
    c->frames = realloc(c->frames, sizeof(size_t) * (c->frame_count + 1));
    c->frames[c->frame_count++] = c->index_count;
    
    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
    c->names[c->name_count].type = 0;
    c->names[c->name_count].precedence = 0;
    c->names[c->name_count].codegen_as = 0;
    c->names[c->name_count].length = 4;
    c->names[c->name_count].signature = calloc
    (c->names[c->name_count].length, sizeof(size_t));
    c->names[c->name_count].signature[0] = 'r';
    c->names[c->name_count].signature[1] = 'o';
    c->names[c->name_count].signature[2] = 'o';
    c->names[c->name_count].signature[3] = 't';
    c->names[c->name_count].definition = (struct unit) {0};
    c->name_count++;
    
    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
    c->names[c->name_count].type = intrin_root;
    c->names[c->name_count].precedence = 0;
    c->names[c->name_count].codegen_as = 0;
    c->names[c->name_count].length = 4;
    c->names[c->name_count].signature = calloc(c->names[c->name_count].length, sizeof(size_t));
    c->names[c->name_count].signature[0] = 'i';
    c->names[c->name_count].signature[1] = 'n';
    c->names[c->name_count].signature[2] = 'i';
    c->names[c->name_count].signature[3] = 't';
    c->names[c->name_count].definition = (struct unit) {0};
    c->name_count++;
    
//    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
//    c->names[c->name_count].type = intrin_init;
//    c->names[c->name_count].precedence = 0;
//    c->names[c->name_count].codegen_as = 0;
//    c->names[c->name_count].length = 3;
//    c->names[c->name_count].signature = calloc(c->names[c->name_count].length, sizeof(size_t));
//    c->names[c->name_count].signature[0] = 'p';
//    c->names[c->name_count].signature[1] = 'o';
//    c->names[c->name_count].signature[2] = 'p';
//    c->names[c->name_count].definition = (struct unit) {0};
//    c->name_count++;
//
//    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
//    c->names[c->name_count].type = intrin_init;
//    c->names[c->name_count].precedence = 0;
//    c->names[c->name_count].codegen_as = 0;
//    c->names[c->name_count].length = 4;
//    c->names[c->name_count].signature = calloc(c->names[c->name_count].length, sizeof(size_t));
//    c->names[c->name_count].signature[0] = 'p';
//    c->names[c->name_count].signature[1] = 'u';
//    c->names[c->name_count].signature[2] = 's';
//    c->names[c->name_count].signature[3] = 'h';
//    c->names[c->name_count].definition = (struct unit) {0};
//    c->name_count++;
        
    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
    c->names[c->name_count].type = intrin_init;
    c->names[c->name_count].precedence = 0;
    c->names[c->name_count].codegen_as = 0;
    c->names[c->name_count].length = 4;
    c->names[c->name_count].signature = calloc(c->names[c->name_count].length, sizeof(size_t));
    c->names[c->name_count].signature[0] = 'c';
    c->names[c->name_count].signature[1] = 'h';
    c->names[c->name_count].signature[2] = 'a';
    c->names[c->name_count].signature[3] = 'r';
    c->names[c->name_count].definition = (struct unit) {0};
    c->name_count++;
    
//    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
//    c->names[c->name_count].type = intrin_init;
//    c->names[c->name_count].precedence = 0;
//    c->names[c->name_count].codegen_as = 0;
//    c->names[c->name_count].length = 5;
//    c->names[c->name_count].signature = calloc(c->names[c->name_count].length, sizeof(size_t));
//    c->names[c->name_count].signature[0] = 'p';
//    c->names[c->name_count].signature[1] = 'a';
//    c->names[c->name_count].signature[2] = 'r';
//    c->names[c->name_count].signature[3] = 'a';
//    c->names[c->name_count].signature[4] = 'm';
//    c->names[c->name_count].definition = (struct unit) {0};
//    c->name_count++;
    
    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
    c->names[c->name_count].type = intrin_init;
    c->names[c->name_count].precedence = 0;
    c->names[c->name_count].codegen_as = 0;
    c->names[c->name_count].length = 6;
    c->names[c->name_count].signature = calloc(c->names[c->name_count].length, sizeof(size_t));
    c->names[c->name_count].signature[0] = 'j';
    c->names[c->name_count].signature[1] = 'o';
    c->names[c->name_count].signature[2] = 'i';
    c->names[c->name_count].signature[3] = 'n';
    c->names[c->name_count].signature[4] = intrin_init;
    c->names[c->name_count].signature[5] = intrin_init;
    c->names[c->name_count].definition = (struct unit) {0};
    c->name_count++;
    
    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
    c->names[c->name_count].type = intrin_init;
    c->names[c->name_count].precedence = 0;
    c->names[c->name_count].codegen_as = 0;
    c->names[c->name_count].length = 5;
    c->names[c->name_count].signature = calloc(c->names[c->name_count].length, sizeof(size_t));
    c->names[c->name_count].signature[0] = 'd';
    c->names[c->name_count].signature[1] = 'e';
    c->names[c->name_count].signature[2] = 'c';
    c->names[c->name_count].signature[3] = 'l';
    c->names[c->name_count].signature[4] = intrin_root;
    c->names[c->name_count].definition = (struct unit) {0};
    c->name_count++;
    
    c->indicies = realloc(c->indicies, sizeof(size_t) * (c->index_count + 1));
    c->indicies[c->index_count++] = intrin_root;
    
    c->indicies = realloc(c->indicies, sizeof(size_t) * (c->index_count + 1));
    c->indicies[c->index_count++] = intrin_init;
    
    c->indicies = realloc(c->indicies, sizeof(size_t) * (c->index_count + 1));
    c->indicies[c->index_count++] = intrin_char;
    
//    c->indicies = realloc(c->indicies, sizeof(size_t) * (c->index_count + 1));
//    c->indicies[c->index_count++] = intrin_param;
//
    c->indicies = realloc(c->indicies, sizeof(size_t) * (c->index_count + 1));
    c->indicies[c->index_count++] = intrin_decl;
    
    c->indicies = realloc(c->indicies, sizeof(size_t) * (c->index_count + 1));
    c->indicies[c->index_count++] = intrin_join;
}

int main(int argc, const char** argv) {
    for (int i = 1; i < argc; i++) {    
        const char* ext = strrchr(argv[i], '.');
        if (!ext) abort();
        
        else if (!strcmp(ext, ".n")) {
            
            uint8_t* text = NULL;
            struct stat st = {0};
            int file = open(argv[i], O_RDONLY);
            if (file < 0 || stat(argv[i], &st) < 0 ||
                (text = mmap(0, st.st_size, PROT_READ,
                             MAP_SHARED, file, 0)) == MAP_FAILED) {
                fprintf(stderr, "n: %s: ", argv[i]);
                perror("error");
                continue;
            } else close(file);
            
            struct context context = {0};
            construct_a_context(&context);
            debug_context(&context);
            struct unit program = compile(argv[i], text, st.st_size, &context);
            debug_context(&context);
            debug_tree(program, 0, &context);
            munmap(text, st.st_size);
            
        } else if (!strcmp(ext, ".ll")) {
            printf("found a .ll file.\n");
            abort();
            
        } else {
            printf("unknown extension.\n");
            abort();
        }
    }
}
