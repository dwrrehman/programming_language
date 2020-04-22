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

struct name {
    size_t type;
    size_t count;
    size_t* signature;
    struct resolved def;
    LLVMValueRef llvmdef;
};

struct frame {
    size_t count;
    size_t* indicies;
};

struct context {
    size_t at;
    size_t best;
    size_t frame_count;
    size_t count;
    
    size_t cnp;
    size_t cfp;
    
    struct frame* frames;
    struct name* names;
};

static void represent(size_t given, char* buffer, size_t limit, size_t* at, struct context* context) {
    if (given > context->count) return;
    struct name name = context->names[given];
    for (size_t i = 0; i < name.count; i++) {
        if (*at + 3 >= limit) return;
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

static void print_error(struct token* given, size_t given_count, size_t type, struct context* context, const char* filename) {
     ///TODO: we probably want to have a limiter on the number of errors that we are going to give. thats seems important.
    char buffer[2048] = {0}; size_t index = 0;
    represent(type, buffer, sizeof buffer,  &index, context);
    if (context->best < given_count) {
        struct token b = given[context->best];
        printf("n3zqx2l: %s:%lu:%lu: error: %s: unresolved %c\n\n", filename, b.line, b.column, buffer, (char) b.value);
    } else printf("n3zqx2l: %s:%d:%d: error: %s: unresolved expression\n\n", filename, 1, 1, buffer);
}

enum {
    _o, _define,
};

static void do_intrinsic(struct resolved solution, struct context* context) {
//    if (solution.index == _appchar) {
//        const size_t n = context->cnp;
//        context->names[n].signature = realloc(context->names[n].signature, sizeof(size_t) * (context->names[n].count + 1));
//        context->names[n].signature[context->names[n].count++] = solution.arguments[0].value;
//
////    } else if (solution.index == _apppar) {
////
////        const size_t n = context->cnp;
////        context->names[n].signature = realloc(context->names[n].signature, sizeof(size_t) * (context->names[n].count + 1));
////        context->names[n].signature[context->names[n].count++] = solution.arguments[0].index + 256;
//
//    } else if (solution.index == _push) {
//
//        /// "Quick and Dirty" solution:    (incorrect solution)
//        /// ... we really should be doing a sorted-insertion, according to a wacky comparison function, to make it so
//        /// forehead-heavy signatures are last, and signatures are sorted smallest first. very important.
//
//        const size_t f = context->cfp;
//        context->frames[f].indicies = realloc(context->frames[f].indicies, sizeof(size_t) * (context->frames[f].count + 1));
//        context->frames[f].indicies[context->frames[f].count++] = context->cnp;
//    }
}


static void evaluate(struct resolved solution, struct context* context) {
    do_intrinsic(solution, context);
    for (size_t i = 0; i < solution.count; i++) evaluate(solution.arguments[i], context);
}

static void define(struct resolved solution, struct context* context) {
    if (solution.index == _define) {

        evaluate(solution, context);

        // evalulate the name.      ///DONE --> not neccesary.
        
        // evaluate the type.       /// NECCESSARY
        
        // define a random name, of that type.
        
        // thats it.
    }
}

static struct resolved resolve_at(struct token* given, size_t given_count, size_t type, struct context* context, size_t depth, const char* filename) {
    if (depth > 128) return (struct resolved) {0};
    
    size_t saved = context->at;
    for (size_t f = context->frame_count; f--;) {
        struct frame frame = context->frames[f];
        
        for (size_t i = frame.count; i--; ) {
            context->best = fmax(context->at, context->best);
            context->at = saved;
 
            struct resolved solution = {frame.indicies[i], 0, 0, 0};
            struct name name = context->names[solution.index];
            
            if (name.type != type) continue;
            
            for (size_t s = 0; s < name.count; s++) {
                
                if (context->at >= given_count && solution.count == 1 && s == 1) return solution.arguments[0];
                
                else if (context->at >= given_count) goto next;
                
                else if (name.signature[s] > 255) {
                    struct resolved argument = resolve_at(given, given_count, context->names[name.signature[s] - 256].type, context, depth + 1, filename);
                    if (!argument.index) goto next;
                    solution.arguments = realloc(solution.arguments, sizeof(struct resolved) * (solution.count + 1));
                    solution.arguments[solution.count++] = argument;
                    
                } else if (name.signature[s] == given[context->at].value) context->at++;
                else goto next;
                
            }
            // right here, we want to expand the definition of a macro, if it is a macro.
            //do_intrinsic(solution, context);
            return solution;
            next: continue;
        }
    }
//    if (type == _c) return (struct resolved) {_c, given[context->at++].value, 0, 0};
    print_error(given, given_count, type, context, filename);
    return (struct resolved) {0};
}

static void debug_context(struct context* context) {
    printf("index = %lu, best = %lu\n", context->at, context->best);
    printf("cnp = %lu, cfp = %lu\n", context->cnp, context->cfp);
    printf("---- debugging frames: ----\n");
    for (size_t i = 0; i < context->frame_count; i++) {
        printf("\t ----- FRAME # %lu ---- \n\t\tidxs: { ", i);
        for (size_t j = 0; j < context->frames[i].count; j++) printf("%lu ", context->frames[i].indicies[j]);
        puts("}");
    }
    printf("\nmaster: {\n");
    for (size_t i = 0; i < context->count; i++) {
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
    if (expected_type >= context->count) {
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
    
    context->at = context->best = 0;
    struct resolved resolved = resolve_at(tokens, token_count, expected_type, context, 0, filename);
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
    if (expected_type >= context->count) {
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
    
    context->at = context->best = 0;
    struct resolved resolved = resolve_at(tokens, token_count, expected_type, context, 0, "<string>");
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
    
//    LLVMLinkInInterpreter();
//
//    const char* path = "/Users/deniylreimn/Documents/projects/n3zqx2l/examples/test.ll";
//
//    LLVMModuleRef module = LLVMModuleCreateWithName("init.n");
//
//    LLVMAddFunction(module, "hi", LLVMFunctionType(LLVMInt32Type(), 0, 0, 0));
//    LLVMAddFunction(module, "fwef", LLVMFunctionType(LLVMInt32Type(), 0, 0, 0));
//
//    LLVMModuleRef m = LLVMModuleCreateWithName("temp.n");
//    LLVMMemoryBufferRef buffer; char* out = NULL;
//    if (LLVMCreateMemoryBufferWithContentsOfFile(path, &buffer, &out) ||
//        LLVMParseIRInContext(LLVMGetGlobalContext(), buffer, &m, &out) ||
//        LLVMLinkModules2(module, m))
//        printf("llvm: error: %s\n", out);
//
//    puts(LLVMPrintModuleToString(module));
//
//    LLVMExecutionEngineRef engine = NULL;
//    if (LLVMCreateExecutionEngineForModule(&engine, module, &out)) {
//        printf("llvm: error: %s\n", out);
//    }
//
//    const char* name = "hello";
//
//    LLVMValueRef f = NULL;
//    if (LLVMFindFunction(engine, name, &f)) {
//        printf("llvm: error: could not find function %s to run\n", name);
//    }
//
//    LLVMRunFunction(engine, f, 0, NULL);
//
//    exit(0);
    
    configure_terminal();
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') exit(!puts("n3zqx2l version 0.0.0\nn3zqx2l [-] [files]"));
        struct context context = {0};
        
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
                size_t f = context.cfp;
                context.frames[f].indicies = realloc(context.frames[f].indicies, sizeof(size_t) * (context.frames[f].count + 1));
                context.frames[f].indicies[context.frames[f].count++] = context.cnp;
                
            } else if (c == 'e') {
                context.names = realloc(context.names, sizeof(struct name) * (context.count + 1));
                context.names[context.count++] = (struct name) {0};
                
            } else if (c == 'f') {
                context.frames = realloc(context.frames, sizeof(struct name) * (context.frame_count + 1));
                context.frames[context.frame_count++] = (struct frame) {0};
            
            } else if (c == 'c') {
                char c = get_character(); printf("%c", c);
                size_t n = context.cnp;
                context.names[n].signature = realloc(context.names[n].signature, sizeof(size_t) * (context.names[n].count + 1));
                context.names[n].signature[context.names[n].count++] = c;
            
            } else if (c == 'a') {
                const size_t n = context.cnp;
                context.names[n].signature = realloc(context.names[n].signature, sizeof(size_t) * (context.names[n].count + 1));
                context.names[n].signature[context.names[n].count++] = read_number() + 256;
                
            } else if (c == 't') context.names[context.cnp].type = read_number();
            else if (c == 'j') context.cnp++;
            else if (c == 'i') context.cnp--;
            else if (c == 'k') context.cfp++;
            else if (c == 'l') context.cfp--;
            else if (c == 'u') context.names[context.cnp].count--;
            else if (c == 'v') context.frames[context.cfp].count--;
            else if (c == 'n') context.count--;
            else if (c == 'm') context.frame_count--;
            else if (c == ';') clear_screen();
            else if (c == 'd') debug_context(&context);
            else if (c == 'r') resolve_file_in_context(argv[i], &context);
            else if (c == 'w') resolve_string_in_context(&context);
            else if (c == '\n') continue;
            else printf("\nn: error: unrecognized command: %c\n", c);
        }
    }
    restore_terminal();
}
