#include <llvm-c/Core.h>           // a n3zqx2l compiler
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <llvm-c/ExecutionEngine.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct res {
    size_t index;
    size_t count;
    size_t total;
    struct res* args;
};

struct ent {
    size_t type;
    size_t count;
    size_t* sig;
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
    struct ent* names;
    struct ent* owners;
};

static inline void represent
(size_t given, char* buffer, size_t limit,
 size_t* at, struct context* context) {
    
    if (given < 256) {
        buffer[(*at)++] = given;
        return;
    } else given -= 256;
    
    if (given >= context->name_count) return;
    
    struct ent s = context->names[given];
    
    for (size_t i = 0; i < s.count; i++)
        represent(s.sig[i], buffer, limit, at, context);
    
    if (!s.type) return;
    
    buffer[(*at)++] = '\n';
    
    represent(s.type, buffer, limit, at, context);
}

static inline void print_source_code
(uint8_t* source, size_t length,
 uint16_t line, uint16_t column) {
    
    ssize_t target = (ssize_t) line - 1;
    char* copy = strndup((char*)source, length), *text = copy;
    
    for (ssize_t at = 0; text; at++) {
        char* line = strsep(&text, "\n");
        
        if (at > target + 2) break;
        else if (at < target - 2) continue;
        
        printf("   \x1B[90m%5lu\x1B[0m\x1B[32m  │  \x1B[0m%s\n", at + 1, line);
        
        if (at == target)
            printf("%*c\x1B[091m^\x1B[0m\n", column + 12, ' ');
    }
    puts("\n");
    free(copy);
}

static inline void parse_error
(uint8_t* given, size_t given_count,
 uint16_t* lines, uint16_t* columns,
 size_t type, struct context* C,
 const char* filename,
 uint8_t* text, size_t length) {
    
    if (!given_count) return;
    
    else if (C->best == given_count) C->best--;
    
    char type_string[4096] = {0};
    size_t index = 0;
    represent(type, type_string, sizeof type_string, &index, C);
    
    printf("n3zqx2l: %s:%u:%u: \x1B[091merror:\x1B[0m %s: unresolved %c\n\n",
           filename, lines[C->best], columns[C->best], type_string, given[C->best]);
    
    print_source_code(text, length, lines[C->best], columns[C->best]);
}

static inline struct res parse
(uint8_t* given,
 size_t begin, size_t end,
 size_t type, size_t max_depth,
 struct context* C)
{
    return (struct res) {0};
}

void debug_context(struct context* context) {
    printf("\n[best = %lu]\n", context->best);
    printf("---- debugging frames: ----\n");
    for (size_t i = 0; i < context->frame_count; i++)
        printf("\tframe # %lu  bp = %lu\n", i, context->frames[i]);
    printf("\n---- debugging indicies: ----\n");
    printf("\t\tidxs: { ");
    for (size_t i = 0; i < context->index_count; i++)
        printf("%lu ", context->indicies[i]);
    printf("}\n\n----- master: ------ \n{\n");
    for (size_t i = 0; i < context->name_count; i++) {
        char buffer[4096] = {0}; size_t index = 0;
        represent(i, buffer, sizeof buffer, &index, context);
        printf("\t%6lu: %s\n\n", i, buffer);
    } puts("}\n");
}

void debug_resolved(struct res given, size_t depth, struct context* context) {
    char buffer[4096] = {0}; size_t index = 0;
    represent(given.index, buffer, sizeof buffer, &index, context);
    for (size_t i = depth; i--;) printf(".   ");
    printf("%s:%lu:%lu:%c\n", buffer, given.index, given.total, (char) given.index);
    for (size_t i = 0; i < given.count; i++) debug_resolved(given.args[i], depth + 1, context);
    puts("");
}

void destroy(struct res r) {
    for (size_t i = 0; i < r.count; i++) destroy(r.args[i]);
    free(r.args);
}

int main(int argc, const char** argv) {
    for (int i = 1; i < argc; i++) {
        
        FILE* file = fopen(argv[i], "r");
        if (!file) {
            perror(argv[i]);
            continue;
        }
        
        fseek(file, 0, 2);
        size_t length = ftell(file), token_count = 0;
        
        uint8_t
            * text = malloc(length),
            * tokens = malloc(length);
        
        fseek(file, 0, 0);
        fread(text, 1, length, file);
        fclose(file);
        
        uint16_t
            * lines = malloc(length * 2),
            * columns = malloc(length * 2),
            line = 1,
            column = 1;
        
        for (size_t j = 0; j < length; j++) {
            if (text[j] > 32) {
                lines[token_count] = line;
                columns[token_count] = column;
                tokens[token_count++] = text[j];
            }
            
            if (text[j] == 10) {
                line++;
                column = 1;
            } else column++;
        }
        
        struct context context = {0};
        struct res result = parse(tokens, 0, token_count, 256, 5, &context);
        
        if (!result.index)
            parse_error(tokens, token_count, lines, columns, 256, &context, argv[i], text, length);
        
        debug_resolved(result, 0, &context);
        debug_context(&context);
        
        destroy(result);
        free(lines);
        free(columns);
        free(tokens);
        free(text);
    }
}

























    //    for (size_t depth = 0; depth <= max_depth; depth++) {
    //        for (size_t i = C->index_count; i--;) {
//            struct res sol = {C->indicies[i], 0, 0, 0};
//            struct ent name = C->names[sol.index];
//            if (type && name.type != type) goto next;
//            for (uint8_t s = 0; s < name.count; s++) {
//                if (begin + sol.total >= end) goto next;
//                if (name.sig[s] >= 256) {
//                    struct res arg = parse(given, begin + sol.total, end, C->names[name.sig[s] - 256].type, depth + 1, C);
//                    if (!arg.index) goto next;
//                    sol.total += arg.total;
//                    sol.args = realloc(sol.args, sizeof(struct res) * (sol.count + 1));
//                    sol.args[sol.count++] = arg;
//                } else if (name.sig[s] == given[begin + sol.total]) sol.total++; else goto next;
//            } return sol; next: C->best = begin + sol.total > C->best ? begin + sol.total : C->best;
//        }
//    }
