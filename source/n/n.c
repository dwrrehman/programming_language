#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <llvm-c/ExecutionEngine.h>
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <termios.h>
#include <string.h>

struct token {
    size_t value;
    size_t line;
    size_t column;
};

struct resolved {
    size_t index;
    size_t value;
    size_t count;
    struct resolved* arguments;
};

enum {
    _o, _i, _name,
    _char, _symbol,
    _appchar, _param, _namejoin,
    _declare, _define, _join,
    
    /// alternatively,
    
//    _o, _name,
//    _char, _symbol, _appchar,
//    _declare, _define, _join,
};

enum codegen_type {
    _cg_default,
    _cg_none,
    _cg_macro,
    _cg_function,
    _cg_namespace,
    _cg_variable,
    _cg_structure,
};

struct name {
    size_t type;
    size_t count;
    size_t codegen_as;
    size_t* signature;
    LLVMValueRef llvmdef;
    struct resolved def;
};

struct frame {
    size_t count;
    size_t* indicies;
};

struct context {
    size_t at;
    size_t best;
    size_t errors;
    size_t max_error_count;
    size_t frame_count;
    size_t name_count;
    struct frame* frames;
    struct name* names;
    const char* filename;
};

static void represent(size_t given, char* buffer, size_t limit, size_t* at, struct context* context) {
    if (given > context->name_count) return;
    struct name name = context->names[given];
    for (size_t i = 0; i < name.count; i++) {
        if (*at + 4 >= limit) return;
        else if (name.signature[i] < 256) buffer[(*at)++] = name.signature[i];
        else {
            buffer[(*at)++] = '(';
            represent(name.signature[i] - 256, buffer, limit, at, context);
            buffer[(*at)++] = ')';
        }
    }
    if (name.type) {
        buffer[(*at)++] = ' ';
        represent(name.type, buffer, limit, at, context);
    }
}

static struct resolved resolution_error(struct token* given, size_t given_count, size_t type, struct context* context) {
    char buffer[2048] = {0};
    size_t index = 0;
    represent(type, buffer, sizeof buffer,  &index, context);
    if (context->best < given_count) {
        struct token b = given[context->best];
        printf("n3zqx2l: %s:%lu:%lu: error: %s: unresolved %c\n\n", context->filename, b.line, b.column, buffer, (char) b.value);
    } else printf("n3zqx2l: %s:%d:%d: error: %s: unresolved expression\n\n", context->filename, 1, 1, buffer);
    context->errors++; return (struct resolved) {0};
}

static struct resolved macro_expansion_error(struct resolved solution, struct token* given, size_t given_count, struct context* context) {
    char buffer[2048] = {0};
    size_t index = 0;
    represent(solution.index, buffer, sizeof buffer,  &index, context);
    if (context->best < given_count) {
        struct token b = given[context->best];
        printf("n3zqx2l: %s:%lu:%lu: error: could not expand macro %s without definition\n\n", context->filename, b.line, b.column, buffer);
    } context->errors++; return (struct resolved) {0};
}

static size_t define(struct resolved given, struct context* context, size_t should_pop);

static void construct_signature(struct resolved given, struct name* result, struct context* context) {
    if (given.index == _appchar) {
        result->signature = realloc(result->signature, sizeof(size_t) * (result->count + 1));
        result->signature[result->count++] = given.arguments[0].value;
    } else if (given.index == _param) {
        result->signature = realloc(result->signature, sizeof(size_t) * (result->count + 1));
        result->signature[result->count++] = 256 + define(given.arguments[0], context, 1);
    } else for (size_t i = 0; i < given.count; i++) construct_signature(given.arguments[i], result, context);
}

static size_t construct_type(struct resolved type) {
    if (!type.count) return type.index;      /// Base case, i think...      for when we have no arguments. its just a straight type, like "int".
    
    
    /// This is more complicated than simply getting an index.
    /// think,  "vector<int>".  we need to possibly instantiate a new type, right? by running this function, right?
    return type.index; /// TEMP
}

static size_t define(struct resolved given, struct context* context, size_t should_pop) {
    size_t f = context->frame_count - 1, new_index = context->name_count;
    context->frames[f].indicies = realloc(context->frames[f].indicies, sizeof(struct frame) * (context->frames[f].count + 1));
    context->frames[f].indicies[context->frames[f].count++] = new_index; ///TODO: call insertion-back instead
        
    context->frames = realloc(context->frames, sizeof(struct frame) * (context->frame_count + 1));
    context->frames[context->frame_count++] = (struct frame){0};
    struct name new = {0};
    new.type = construct_type(given.arguments[1]);
    construct_signature(given.arguments[0], &new, context);
    context->names = realloc(context->names, sizeof(struct name) * (new_index + 1));
    context->names[context->name_count++] = new;
    if (should_pop) context->frame_count--;
    return new_index;
}

static void replace_in_tree(size_t find, size_t replace, struct resolved* tree) {
    if (tree->index == find) tree->index = replace;
    for (size_t i = 0; i < tree->count; i++) replace_in_tree(find, replace, &tree->arguments[i]);
}

static struct resolved duplicate(struct resolved given) {
    struct resolved new = given;
    new.arguments = malloc(given.count * sizeof(struct resolved));
    for (size_t i = 0; i < given.count; i++) new.arguments[i] = duplicate(given.arguments[i]);
    return new;
}

static size_t is_defined(struct context* context, size_t tofind) {
    for (size_t f = 0; f < context->frame_count; f++)
        for (size_t i = 0; i < context->frames[f].count; i++)
            if (context->frames[f].indicies[i] == tofind) return 1;
    return 0;
}

static struct resolved expand_macro(struct name name, struct resolved solution, struct token* given, size_t given_count, struct context* context) {
    if (!name.def.index) return macro_expansion_error(solution, given, given_count, context);
    struct resolved def = duplicate(name.def);
    for (size_t i = 0, s = 0; s < solution.count; s++) {
        while (i < name.count && name.signature[i] < 256) i++;
        replace_in_tree(name.signature[i++], solution.arguments[s].index, &def);
    } return def;
}

static struct resolved resolve_at(struct token* given, size_t given_count, size_t type, size_t depth, struct context* context) {
    if (depth > 128 || context->errors > context->max_error_count) return (struct resolved) {0};
    if (type == _symbol) return (struct resolved) {_char, given[context->at++].value, 0, 0};
    size_t saved = context->at;
    for (size_t f = context->frame_count; f--;) {
        for (size_t i = context->frames[f].count; i--; ) {
            if (context->errors > context->max_error_count) return (struct resolved) {0};
            context->best = fmax(context->at, context->best);
            context->at = saved;
            struct resolved solution = {context->frames[f].indicies[i], 0, 0, 0};
            struct name name = context->names[solution.index];
            if (name.type != type && is_defined(context, name.type)) continue;
            
            for (size_t s = 0; s < name.count; s++) {
                if (context->errors > context->max_error_count) return (struct resolved) {0};
                if (context->at >= given_count) { if (solution.count == 1 && s == 1) return solution.arguments[0]; else goto next; }
                else if (name.signature[s] >= 256) {
                    struct resolved argument = resolve_at(given, given_count, context->names[name.signature[s] - 256].type, depth + 1, context);
                    if (context->errors > context->max_error_count) return (struct resolved) {0};
                    if (!argument.index) goto next;
                    solution.arguments = realloc(solution.arguments, sizeof(struct resolved) * (solution.count + 1));
                    solution.arguments[solution.count++] = argument;
                } else if (name.signature[s] == given[context->at].value) context->at++; else goto next;
                if ((solution.index == _define || solution.index == _declare) && solution.count == 2) define(solution, context, solution.index == _declare);
            }
            if (name.codegen_as == _cg_macro) solution = expand_macro(name, solution, given, given_count, context);
            if (solution.index == _define) { context->frame_count--; context->names[context->name_count - 1].def = solution.arguments[2]; }
            return solution; next: if (solution.index == _define) context->frame_count--;
        }
    } return resolution_error(given, given_count, type, context);
}














/// ------------------- debug and testing framework ----------------------------

static void debug_context(struct context* context, size_t context_cnp, size_t context_cfp) {
    printf("index = %lu, best = %lu\n", context->at, context->best);
    printf("cnp = %lu, cfp = %lu\n", context_cnp, context_cfp);
    printf("---- debugging frames: ----\n");
    for (size_t i = 0; i < context->frame_count; i++) {
        printf("\t ----- FRAME # %lu ---- \n\t\tidxs: { ", i);
        for (size_t j = 0; j < context->frames[i].count; j++) printf("%lu ", context->frames[i].indicies[j]);
        puts("}");
    }
    printf("\nmaster: {\n");
    for (size_t i = 0; i < context->name_count; i++) {
        printf("\t%6lu: ", i);
        char buffer[2048] = {0};
        size_t index = 0;
        represent(i, buffer, sizeof buffer, &index, context);
        printf("%s", buffer);
        puts("\n");
    }
    puts("}");
}

static void prep(size_t depth) { for (size_t i = depth; i--;) printf(".   "); }

static void debug_resolved(struct resolved given, size_t depth, struct context* context) {
    char buffer[4096] = {0};
    size_t index = 0;
    represent(given.index, buffer, sizeof buffer, &index, context);
    prep(depth); printf("%s : [%lu]", buffer, given.index);
    if (given.value) printf(" : c=%c", (char) given.value);
    printf("\n");
    for (size_t i = 0; i < given.count; i++) {
        debug_resolved(given.arguments[i], depth + 1, context);
    }
    prep(depth); printf("\n");
}

static struct termios terminal = {0};

static inline void restore_terminal() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal) < 0) perror("tcsetattr(STDIN_FILENO, TCSAFLUSH, &terminal))");
}

static inline void configure_terminal() {
    if (tcgetattr(STDIN_FILENO, &terminal) < 0) perror("tcgetattr(STDIN_FILENO, &terminal)");
    atexit(restore_terminal);
    struct termios raw = terminal;
    raw.c_lflag &= ~(ECHO | ICANON);
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) < 0) perror("tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw)");
}

static inline char get_character() {
    char c = 0;
    fflush(stdout);
    const ssize_t n = read(STDIN_FILENO, &c, 1);
    if (n < 0) {
        printf("n < 0 : ");
        perror("read(STDIN_FILENO, &c, 1) syscall");
        abort();
    } else if (n == 0) {
        printf("n == 0 : ");
        perror("read(STDIN_FILENO, &c, 1) syscall");
        abort();
    } else return c;
}

static size_t read_number() {
    printf(".");
    size_t n = get_character() - '0'; printf("%c", (char)(n + '0'));
    size_t m = get_character() - '0'; printf("%c", (char)(m + '0'));
    return 10 * n + m;
}

static void resolve_file_in_context(const char* filename, struct context* context) {
    printf("t: ");
    size_t expected_type = read_number();
    if (expected_type >= context->name_count) {
        printf("error: expected type not in range! aborting.\n");
        return;
    }
    
    FILE* file = fopen(filename, "r");
    if (!file) {
        fprintf(stderr, "n: %s: ", filename);
        perror("error");
        return;
    }
    
    fseek(file, 0, SEEK_END);
    size_t length = ftell(file);
    char* text = malloc(length * sizeof(char));
    struct token* tokens = malloc(length * sizeof(struct token));
    fseek(file, 0, SEEK_SET);
    fread(text, sizeof(char), length, file);
    fclose(file);
    
    size_t token_count = 0, line = 1, column = 1;
    for (size_t i = 0; i < length; i++) {
        if (text[i] > 32) tokens[token_count++] = (struct token){text[i], line, column};
        if (text[i] == 10) { line++; column = 1; } else column++;
    }
    context->filename = filename;
    context->at = context->best = context->errors = 0;
    struct resolved resolved = resolve_at(tokens, token_count, expected_type, 0, context);
    puts("");
    debug_resolved(resolved, 0, context);
    puts("\n");
    if (!resolved.index || context->at != token_count) {
        printf("\n\n\tRESOLUTION ERROR\n\n");
    }
    
    free(tokens);
    free(text);
}

static void resolve_string_in_context(struct context* context) {
    printf("t: ");
    size_t expected_type = read_number();
    if (expected_type >= context->name_count) {
        printf("error: expected type not in range! aborting.\n");
        return;
    }
    const size_t max_string_length = 4096;
    char* text = malloc(max_string_length * sizeof(char));
    printf("text: ");
    restore_terminal();
    fgets(text, max_string_length, stdin);
    configure_terminal();
    
    const size_t length = strlen(text);
    struct token* tokens = malloc(length * sizeof(struct token));
    size_t token_count = 0, line = 1, column = 1;
    for (size_t i = 0; i < length; i++) {
        if (text[i] > 32) tokens[token_count++] = (struct token){text[i], line, column};
        if (text[i] == 10) { line++; column = 1; } else column++;
    }
    
    context->filename = "<string>";
    context->at = context->best = context->errors = 0;
    struct resolved resolved = resolve_at(tokens, token_count, expected_type, 0, context);
    puts("");
    debug_resolved(resolved, 0, context);
    puts("");
    if (!resolved.index || context->at != token_count) {
        printf("\n\tRESOLUTION ERROR\n\n");
    }
    
    free(tokens);
    free(text);
}

void clear_screen() {
    printf("\033[1;1H\033[2J");
}

int main(int argc, const char** argv) {
    
    configure_terminal();
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            exit(!puts("n3zqx2l version 0.0.0\nn3zqx2l [-] [files]"));
        }
        
        struct context context = {0};
        size_t context_cnp = 0;
        size_t context_cfp = 0;
        context.max_error_count = 7;
        
        printf("the CSR terminal. type 'h' for help.\n");
        while (1) {
            printf(":");
            int c = get_character(); printf("%c", c);
            if (c == 'q') break;
            else if (c == 'h') {
                printf("\n"
                       "q : quit                  h : this help menu\n"
                       "s : push signature        e : add empty entry\n"
                       "f : add empty frame       d : display context\n"
                       "c : app-char              a : app-par\n"
                       "j : incr cnp              i : decr cnp\n"
                       "k : incr cfp              l : decr cfp\n"
                       "t : set type              u : pop sig symbol\n"
                       "v : pop tops index        n : decr nc\n"
                       "m : decr fc               ; : clear screen\n"
                       "w : resolve string        r : resolve file\n"
                       "\n");
            
            } else if (c == 's') {
                size_t f = context_cfp;
                context.frames[f].indicies = realloc(context.frames[f].indicies, sizeof(size_t) * (context.frames[f].count + 1));
                context.frames[f].indicies[context.frames[f].count++] = context_cnp;
                
            } else if (c == 'e') {
                context.names = realloc(context.names, sizeof(struct name) * (context.name_count + 1));
                context.names[context.name_count++] = (struct name) {0};
                
            } else if (c == 'f') {
                context.frames = realloc(context.frames, sizeof(struct name) * (context.frame_count + 1));
                context.frames[context.frame_count++] = (struct frame) {0};
            
            } else if (c == 'c') {
                char c = get_character(); printf("%c", c);
                size_t n = context_cnp;
                context.names[n].signature = realloc(context.names[n].signature, sizeof(size_t) * (context.names[n].count + 1));
                context.names[n].signature[context.names[n].count++] = c;
            
            } else if (c == 'a') {
                const size_t n = context_cnp;
                context.names[n].signature = realloc(context.names[n].signature, sizeof(size_t) * (context.names[n].count + 1));
                context.names[n].signature[context.names[n].count++] = read_number() + 256;
                
            } else if (c == 't') context.names[context_cnp].type = read_number();
            else if (c == 'j') context_cnp++;
            else if (c == 'i') context_cnp--;
            else if (c == 'k') context_cfp++;
            else if (c == 'l') context_cfp--;
            else if (c == 'u') context.names[context_cnp].count--;
            else if (c == 'v') context.frames[context_cfp].count--;
            else if (c == 'n') context.name_count--;
            else if (c == 'm') context.frame_count--;
            else if (c == ';') clear_screen();
            else if (c == 'd') debug_context(&context, context_cnp, context_cfp);
            else if (c == 'r') resolve_file_in_context(argv[i], &context);
            else if (c == 'w') resolve_string_in_context(&context);
            else if (c == '\n') continue;
            else printf("\nn: error: unrecognized command: %c\n", c);
        }
    }
    restore_terminal();
}
