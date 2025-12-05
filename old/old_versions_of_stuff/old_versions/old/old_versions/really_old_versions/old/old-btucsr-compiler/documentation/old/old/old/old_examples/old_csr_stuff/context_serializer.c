#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

enum codegen {
    none, _macro, _function, _struct, _variable, _global,
};

struct resolved {
    size_t index;
    size_t count;
    size_t begin;
    size_t done;
    struct resolved* args;
};

struct name {
    size_t* signature;
    size_t length;
    size_t type;
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

typedef uint16_t block;
const size_t block_size = 2;

size_t add_signature(block* s, block l, block type, block codegen_as, block precedence, struct context* c) {
        
    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
    
    c->names[c->name_count].type = type;
    c->names[c->name_count].codegen_as = codegen_as;
    c->names[c->name_count].precedence = precedence;
    c->names[c->name_count].length = l;
    c->names[c->name_count].signature = calloc(l, sizeof(size_t));
    
    for (size_t i = 0; i < l; i++)
        c->names[c->name_count].signature[i] = s[i];
        
    c->indicies = realloc(c->indicies, sizeof(size_t) * (c->index_count + 1));
    c->indicies[c->index_count++] = c->name_count;
    
    return c->name_count++ + 256;
}

static void serialize_context_to_file(const struct context *context, const char *path) {
    FILE* out = fopen(path, "w");
    if (!out) { perror("fopen"); exit(1); }
    
    const block count = (block) context->name_count;
    fwrite(&count, sizeof(block), 1, out);
    
    for (block i = 0; i < count; i++) {
        
        block
        n = (block) context->names[i].type; fwrite(&n, sizeof(block), 1, out);
        n = (block) context->names[i].codegen_as; fwrite(&n, sizeof(block), 1, out);
        n = (block) context->names[i].precedence; fwrite(&n, sizeof(block), 1, out);
        n = (block) context->names[i].length; fwrite(&n, sizeof(block), 1, out);
        
        for (size_t s = 0; s < n; s++) {
            const block c = (block) context->names[i].signature[s];
            fwrite(&c, sizeof(block), 1, out);
        }
    }
    fclose(out);
}

void main3(){
    
    const char* path = "/Users/deniylreimn/Documents/art/c/sandboxes/sandbox/sandbox/init.ni";
    
    struct context context = {0};
    context.frames = calloc(context.frame_count = 1, sizeof(size_t));
    
    size_t o = add_signature((block[]){'o'}, 1, 0, none, 0, &context);
    size_t i = add_signature((block[]){'i'}, 1, o, none, 0, &context);
    size_t c = add_signature((block[]){'c'}, 1, i, none, 0, &context);
    size_t j = add_signature((block[]){'j', i, i}, 1, i, none, 0, &context);
        
    serialize_context_to_file(&context, path);
    
    int file = open(path, O_RDONLY);
    if (file < 0) {
        perror("open: error");
        exit(1);
    }
    
    struct stat st;
    if (stat(path, &st) < 0) {
        perror("stat: error");
        exit(1);
    }
    
    uint8_t* text = NULL;
    if (st.st_size && (text = mmap(0, st.st_size, PROT_READ, MAP_SHARED, file, 0)) == MAP_FAILED) {
        perror("mmap: error");
        exit(1);
    }
    
    close(file);
        
    if (!text) exit(1);
    
    context.name_count = *(block*)text;
    context.names = calloc(context.name_count, sizeof(struct name));
    text += block_size;
    printf("reading %lu signatures...\n", context.name_count);
    
    for (int i = 0; i < context.name_count; i++) {
                
        context.names[i].type = *(block*)text; text += block_size;
        context.names[i].codegen_as = *(block*)text; text += block_size;
        context.names[i].precedence = *(block*)text; text += block_size;
        context.names[i].length = *(block*)text; text += block_size;
        
        context.names[i].signature = calloc(context.names[i].length, sizeof(size_t));
        
        printf("read: length = %lu, cg = %lu, prec = %lu, type = %lu\n",
               context.names[i].length, context.names[i].codegen_as,
               context.names[i].precedence, context.names[i].type);
                
        for (size_t s = 0; s < context.names[i].length; s++) {
            context.names[i].signature[s] = *(block*)text; text += block_size;
            printf("read: %lu\n", context.names[i].signature[s]);
        }
    }
}
