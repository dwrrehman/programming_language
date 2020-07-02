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
    size_t begin;
    size_t done;
    struct resolved* args;
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


struct resolved duplicate(const struct resolved r) {
    struct resolved s = {
        r.index, r.count, r.begin, r.done,
        calloc(r.count, sizeof(struct resolved))
    };
    
    for (size_t i = 0; i < r.count; i++)
        s.args[i] = duplicate(r.args[i]);
    
    return s;
}

struct resolved resolve
(const uint8_t* given, const size_t end, struct resolved* stack,
 size_t count, const size_t depth, struct resolved sol, struct context* context)
{
    if (depth >= 15)
        return (struct resolved){0};
    
    const struct name name = context->names[sol.index - 256];
    
    while (sol.done < name.length) {
        const size_t c = name.signature[sol.done];
        
        if (c < 256) {
            if (c != given[sol.begin]) return (struct resolved){0};
            sol.begin++; sol.done++;
            context->best = sol.begin > context->best ? sol.begin : context->best;
            
        } else {
            
            struct resolved* extended = calloc(count + 1, sizeof(struct resolved));
            for (size_t i = 0; i < count; i++) extended[i] = stack[i];
            extended[count++] = sol;
            
            for (size_t i = 0; i < context->name_count; i++) {
                const struct resolved parent =
                    resolve(given, end, extended, count, depth + 1,
                        (struct resolved){256 + i, 0, sol.begin, 0, 0}, context);
                if (parent.index) return parent;
            }
            
            return (struct resolved){0};
        }
    }
    if (count == 0) {
        if (sol.begin == end) return sol;
        else return (struct resolved){0};
        
    } else {
        struct resolved top = duplicate(stack[--count]);
        top.begin = sol.begin;
        top.args = realloc(top.args, sizeof(struct resolved) * (top.count + 1));
        top.args[top.count++] = sol;
        top.done++;
        
        return resolve(given, end, stack, count, depth, top, context);
    }
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

void debug_resolved(struct resolved given, size_t depth, struct context* context) {
    char buffer[4096] = {0}; size_t index = 0;
    represent(given.index, buffer, sizeof buffer, &index, context);
    for (size_t i = depth; i--;) printf(".   ");
    if (given.index < 256) printf("(%s) :: %c:i=%lu:b=%lu:sd=%lu:c=%lu\n", buffer, (char) given.index, given.index, given.begin, given.done, given.count);
    else printf("(%s) :: i=%lu:b=%lu:sd=%lu:c=%lu\n", buffer, given.index, given.begin, given.done, given.count);
    for (size_t i = 0; i < given.count; i++) {
        for (size_t i = depth + 1; i--;) printf(".   ");
        printf("#%lu: \n", i);
        debug_resolved(given.args[i], depth + 1, context);
    }
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
        
        size_t count = lex(text, tokens,
                           loc, st.st_size);
        
        struct resolved ast =
            resolve(tokens, count, 0, 0, 0,
                (struct resolved){256 + 5, 0, 0, 0, 0},
                &context);
        
        debug_context(&context);
        debug_resolved(ast, 0, &context);
        
        if (!ast.index && count) {
            if (context.best == count)
                context.best--;
            fprintf(stderr,
                    "n: %s:%u:%u: error: unresolved %c\n\n",
                    argv[a],
                    loc[2 * context.best],
                    loc[2 * context.best + 1],
                    tokens[context.best]);
        } else if (!ast.index && !count) {
            fprintf(stderr, "n: %s: error: unresolved empty file\n\n", argv[a]);
        }
        
        destroy(ast);
        free(loc);
        free(tokens);
        munmap(text, st.st_size);
    }
}

