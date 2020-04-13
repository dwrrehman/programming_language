#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>


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
    struct name name = context->names[given];
    for (size_t i = 0; i < name.count; i++) {
        if (*at + 2 >= limit) return;
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
    _o, _i, _c, _appchar, _apppar, _join, _push, _new,
    _declare_function,
    
};

static void do_intrinsic(struct resolved solution, struct context* context) {
    if (solution.index == _appchar) {
        const size_t n = context->cnp;
        context->names[n].signature = realloc(context->names[n].signature, sizeof(size_t) * (context->names[n].count + 1));
        context->names[n].signature[context->names[n].count++] = solution.arguments[0].value;
        
//    } else if (solution.index == _apppar) {
//
//        const size_t n = context->cnp;
//        context->names[n].signature = realloc(context->names[n].signature, sizeof(size_t) * (context->names[n].count + 1));
//        context->names[n].signature[context->names[n].count++] = solution.arguments[0].index + 256;
        
    } else if (solution.index == _push) {
                        
        /// "Quick and Dirty" solution:    (incorrect solution)
        /// ... we really should be doing a sorted-insertion, according to a wacky comparison function, to make it so
        /// forehead-heavy signatures are last, and signatures are sorted smallest first. very important.
        
        const size_t f = context->cfp;
        context->frames[f].indicies = realloc(context->frames[f].indicies, sizeof(size_t) * (context->frames[f].count + 1));
        context->frames[f].indicies[context->frames[f].count++] = context->cnp;
    }
}


static void evaluate(struct resolved solution, struct context* context) {
    do_intrinsic(solution, context);
    for (size_t i = 0; i < solution.count; i++) evaluate(solution.arguments[i], context);
}

static void define(struct resolved solution, struct context* context) {
    if (solution.index == _declare_function) {
        
        
        
        evaluate(solution, context);
        
        
        //evalulate the name.
        //evaluate the type.
        // define the name, of that type.
        // thats it.
        
        
        
        
    }
}

static struct resolved resolve_at(struct token* given, size_t given_count, size_t type, struct context* context, size_t depth, const char* filename) {
    if (depth > 128) return (struct resolved) {0};
    size_t saved = context->at;
    for (size_t f = context->frame_count; f--; ) {
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
                } else if (name.signature[s] != given[context->at].value) goto next;
                else context->at++; //TODO: make depth = 0; here.
            } //do_intrinsic(solution, context);
            return solution;
            next: continue;
        }
    }
    if (type == _c) return (struct resolved) {_c, given[context->at++].value, 0, 0};
    print_error(given, given_count, type, context, filename);
    return (struct resolved) {0};
}

static void debug_context(struct context* context) {
    printf("index = %lu, best = %lu\n", context->at, context->best);
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

static void debug_resolved(struct resolved given, size_t depth) {
    prep(depth); printf(" [%lu]:%c:{ ", given.index, (char) given.value);
    for (size_t i = 0; i < given.count; i++) {
        prep(depth+1); printf("ARG(%lu)[", i);
        debug_resolved(given.arguments[i], depth + 1);
        prep(depth+1);printf("], ");
    }
    prep(depth); printf(" } ");
}

static void do_csr_with_context(const char** argv, struct context* context, int i) {
    printf("performing CSR:\n");
    size_t expected_type = 0;
    printf("expected type: ");
    scanf("%lu", &expected_type);
    if (expected_type >= context->count) {
        printf("error: expected type not in range! trying 0 instead.\n");
        expected_type = 0;
    }
    
    printf("reading in file again...\n");
    FILE* file = fopen(argv[i], "r");
    if (!file) {
        fprintf(stderr, "n: %s: ", argv[i]);
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
    struct resolved resolved = resolve_at(tokens, token_count, expected_type, context, 0, argv[i]);
    debug_resolved(resolved, 0);
    
    if (!resolved.index) {
        printf("\n\n\tRESOLUTION ERROR\n\n");
    }
    
    free(tokens);
    free(text);
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

int main(int argc, const char** argv) {
        
    configure_terminal();
    
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') exit(!puts("n3zqx2l version 0.0.0\nn3zqx2l [-] [files]"));
        
        struct context context = {0};
        
        printf("the CSR terminal. used for debugging purposes. type 'h' for help.\n");
        
        while (1) {
            
            printf(": ");
            int c = get_character();
            
            if (c == 'q') break;
            
            else if (c == 'h') {
                printf(
                       "a : add new name\n"
                       "c : add new name\n"
                       "p : add new name\n"
                       "s : print state\n"
                       "u : push new frame index\n"
                       "o : pop last frame index\n"
                       "o : pop last frame index\n"
                       "o : pop last frame index\n"
                       "o : pop last frame index\n"
                       "d : pop last name\n"
                       "h : this help menu\n"
                       "q : quit terminal\n"
                       );
                
            } else if (c == 'a') {
                
                struct name n = {0};
                printf("adding signature\n");
                
                printf("type: ");
//                scanf("%lu", &n.type);
            
                printf("give symbols.\n for a parameter, give (, "
                       "then type a number, then type ).\n to "
                       "terminate the signature, type ;\n");
                while (c != ';') {
                    
                    printf("symbol: ");
                    c = get_character();
                    printf("received: %d\n", c);
                    
                    if (c == '(') {
                        
                        struct resolved param = {0};
                        
                        printf("index: ");
                        scanf("%lu", &param.index);
                        
                        printf("close paren: ");
                        get_character();
                        
                        n.signature
                        = realloc(n.signature,
                                  sizeof(size_t)
                                  * (n.count + 1));
                        n.signature[n.count++] = 0;
                        
                    } else if (c != '\n' && c != ';') {
                        n.signature
                        = realloc(n.signature,
                                  sizeof(size_t)
                                  * (n.count + 1));
                        n.signature[n.count++] = 0;
                    }
                }
                printf("signature terminated. adding now.");
                context.names
                = realloc(context.names,
                          sizeof(struct name) *
                          (context.count + 1));
                context.names[context.count++] = n;
                printf("added.\n");
                
            } else if (c == 's') {
                
                printf("printing context: \n\n");
                debug_context(&context);
                
            } else if (c == 'd') {
                printf("decrementing name count..\n");
                context.count--;
            } else if (c == 'g') {
                
                printf("decrementing stack frame count.\n");
                context.frame_count--;
            } else if (c == 'y') {
                
                printf("push empty new frame.\n");

                context.frames = realloc(context.frames,
                                         sizeof(struct frame) *
                                         (context.frame_count + 1));
                context.frames[context.frame_count++]
                = (struct frame) {0};
                
            } else if (c == 'o') {
                
                printf("decrementing top stack frame indicies count..\n");
                context.frames[context.frame_count - 1].count--;
                                            
            } else if (c == 'u') {
                
                size_t integer = 0;
//                printf("index to push: ");
//                scanf("%lu", &integer);
                
                context.frames[context.frame_count - 1].indicies =
                realloc(context.frames[context.frame_count - 1].indicies,
                        sizeof(size_t) *
                        (context.frames[context.frame_count - 1].count + 1)
                        );
                context.frames[context.frame_count - 1].
                indicies[context.frames[context.frame_count - 1].count++]
                = integer;
                
                
                
                
            } else if (c == 'f') {
                
                do_csr_with_context(argv, &context, i);
                
                
                
            } else if (c == '\n') {
                // do nothing.
            } else {
                printf("ERROR: unrecognized command: %c\n", c);
            }
        }
        printf("terminated terminal.\n");
        
        free(context.frames);
        free(context.names);
        
    }
    
    restore_terminal();
}
