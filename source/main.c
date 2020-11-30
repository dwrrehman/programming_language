#include <llvm-c/Core.h>
#include <llvm-c/IRReader.h>
#include <llvm-c/Linker.h>
#include <llvm-c/ExecutionEngine.h>
#include <llvm-c/Target.h>
#include <llvm-c/Analysis.h>
#include <llvm-c/BitWriter.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iso646.h>

typedef int nat;

struct unit {
    LLVMValueRef value;
    struct unit* args;
    nat begin;
    nat type;
    nat ind;
    nat index;
    nat done;
    nat count;
};

struct name {
    struct unit definition;
    nat* signature;
    char* llvm_name;
    nat type;
    nat length;
    nat codegen_as;
    nat precedence;
};

struct context {
    nat* frames;
    nat* indicies;
    struct name* names;
    struct name* owners;
    nat frame_count;
    nat index_count;
    nat name_count;
    nat _padding;
};

enum codegen_type { cg_local_variable, cg_global_variable, cg_function, cg_struct };

enum intrinsics {
    intrin_root = 256,///  root -> (0)
    intrin_init, ///       init -> root
    intrin_char, ///       char -> init
    intrin_join, ///       join (init) (init) -> init
    intrin_decl, ///       decl (name: char) (type: (0)) -> init
    intrin_eval, ///       eval (type: (0)) (expression: type) -> comp(type)
    intrin_comp, ///       comp (type: init) -> init
    // ------------------------
    intrin_param, intrin_define__arg0, intrin_define,
};



static inline void print_vector(nat* v, nat length) { // temp
    printf("{ ");
    for (nat i = 0; i < length; i++)
        printf("%d ", v[i]);
    printf("}\n");
}

static inline void debug_tree(struct unit tree, size_t d, struct context* context) { // temp
    for (size_t i = 0; i < d; i++)
        printf(".   ");
    
    if (tree.index >= 256) {
        struct name name = context->names[tree.index - 256];

        for (nat i = 0; i < name.length; i++) {
            if (name.signature[i] < 256)
                printf("%c", (char) name.signature[i]);
            else printf(" (%d) ", name.signature[i]);
        }
        
    } else {
        printf("CHARACTER{%c::%u}", (char) tree.index, tree.index);
    }
    
    printf(" :: [ind=%d, index=%d : type=%d, begin=%d, done=%d, count=%d]\n\n", tree.ind, tree.index, tree.type, tree.begin, tree.done, tree.count);
    for (nat i = 0; i < tree.count; i++)
        debug_tree(tree.args[i], d + 1, context);
}

static inline void debug_context(struct context* context) { // temp
    printf("---------------- context --------------\n");
    printf("-------- names --------\n");
    printf("name_count = %d\n", context->name_count);
    for (nat i = 0; i < context->name_count; i++) {
        printf("----- [%d] ----- \n", i);
        printf("\t type = %d\n", context->names[i].type);
        printf("\t precedence = %d\n", context->names[i].precedence);
        printf("\t codegen_as = %d\n", context->names[i].codegen_as);
        printf("\t length = %d\n", context->names[i].length);
        printf("\t signature:     ");
        for (nat s = 0; s < context->names[i].length; s++) {
            nat c = context->names[i].signature[s];
            if (c >= 256) {
                printf("(%d) ", c);
            } else {
                printf("%c ", (char) c);
            }
        }
        printf("\n\n");
        debug_tree(context->names[i].definition, 0, context);
    }
    printf("-------------------\n\n");
    
    printf("---------- indicies --------\n");
    printf("index_count = %d\n", context->index_count);
    printf("indicies:     ");
    print_vector(context->indicies, context->index_count);
    printf("\n");

    printf("---------- frames --------\n");
    printf("frame_count = %d\n", context->frame_count);
    printf("frames:     ");
    print_vector(context->frames, context->frame_count);
    printf("\n");

    printf("---------- owners --------\n");
    printf("(owner)frame_count = %d\n", context->frame_count);
    for (nat i = 0; i < context->frame_count; i++) {
        printf("----- owner [frame=%d] ----- \n", i);
        printf("\t type = %d\n", context->owners[i].type);
        printf("\t precedence = %d\n", context->owners[i].precedence);
        printf("\t codegen_as = %d\n", context->owners[i].codegen_as);
        printf("\t length = %d\n", context->owners[i].length);
        printf("\t signature:     ");
        for (nat s = 0; s < context->owners[i].length; s++) {
            nat c = context->owners[i].signature[s];
            if (c >= 256) {
                printf("(%d) ", c);
            } else {
                printf("%c ", (char) c);
            }
        }
        printf("\n\n");
    }
    printf("-------------------\n\n");
}


static inline char* translate(struct name name) { // temp
    
    nat* signature, length, type;
    signature = name.signature;
    length = name.length;
    type = name.type;
    
//    print_vector(signature, length);
        
    char* result = NULL;
    nat result_length = 0;
    nat result_capacity = 0;
    
    for (nat i = 0; i < length; i++) {
        const nat c = signature[i];
        
        if (c < 256) {
            if (result_length + 1 >= result_capacity) result = realloc(result, sizeof(char) * (result_capacity = 2 * (result_capacity + 1)));
            
            result[result_length++] = c;
        } else {
            
            if (result_length + 1 >= result_capacity) result = realloc(result, sizeof(char) * (result_capacity = 2 * (result_capacity + 1)));
            result[result_length++] = '\x1D';
            
            nat extra = snprintf(NULL, 0, "%u", c);
            if (result_length + extra >= result_capacity) result = realloc(result, sizeof(char) * (result_capacity = 2 * (result_capacity + extra)));
            result_length += sprintf(result + result_length, "%u", c);
            
            if (result_length + 1 >= result_capacity) result = realloc(result, sizeof(char) * (result_capacity = 2 * (result_capacity + 1)));
            result[result_length++] = '\x1E';
        }
    }
    
    if (result_length + 1 >= result_capacity) result = realloc(result, sizeof(char) * (result_capacity = 2 * (result_capacity + 1)));
    result[result_length++] = '\x1F';
    
    nat extra = snprintf(NULL, 0, "%u", type);
    if (result_length + extra >= result_capacity) result = realloc(result, sizeof(char) * (result_capacity = 2 * (result_capacity + extra)));
    result_length += sprintf(result + result_length, "%u", type);
        
    if (result_length + 1 >= result_capacity) result = realloc(result, sizeof(char) * (result_capacity = 2 * (result_capacity + 1)));
    result[result_length++] = '\0';
    
//    printf("result length = %u,  result capacity = %u\n", result_length, result_capacity);
//    printf("\"%s\"\n", result);
    
    return result;
}


static inline void construct_a_context(struct context* c) { // temp
    
    c->name_count = 0;
    c->index_count = 0;
    c->frame_count = 0;
    
    c->owners = realloc(c->owners, sizeof(struct name) * (c->frame_count + 1));
    c->owners[c->frame_count] = (struct name) {0};
    
    c->frames = realloc(c->frames, sizeof(nat) * (c->frame_count + 1));
    c->frames[c->frame_count++] = c->index_count;
    
    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
    c->names[c->name_count].type = 0;
    c->names[c->name_count].precedence = 0;
    c->names[c->name_count].codegen_as = 0;
    c->names[c->name_count].length = 4;
    c->names[c->name_count].signature = calloc
    (c->names[c->name_count].length, sizeof(nat));
    c->names[c->name_count].signature[0] = 'r';
    c->names[c->name_count].signature[1] = 'o';
    c->names[c->name_count].signature[2] = 'o';
    c->names[c->name_count].signature[3] = 't';
    c->names[c->name_count].definition = (struct unit) {0};
    c->names[c->name_count].llvm_name = translate(c->names[c->name_count]);
    c->name_count++;
    
    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
    c->names[c->name_count].type = intrin_root;
    c->names[c->name_count].precedence = 0;
    c->names[c->name_count].codegen_as = 0;
    c->names[c->name_count].length = 4;
    c->names[c->name_count].signature = calloc(c->names[c->name_count].length, sizeof(nat));
    c->names[c->name_count].signature[0] = 'i';
    c->names[c->name_count].signature[1] = 'n';
    c->names[c->name_count].signature[2] = 'i';
    c->names[c->name_count].signature[3] = 't';
    c->names[c->name_count].definition = (struct unit) {0};
    c->names[c->name_count].llvm_name = translate(c->names[c->name_count]);
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
//    c->names[c->name_count].llvm_name = translate(c->names[c->name_count]);
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
//    c->names[c->name_count].llvm_name = translate(c->names[c->name_count]);
//    c->name_count++;
        
    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
    c->names[c->name_count].type = intrin_init;
    c->names[c->name_count].precedence = 0;
    c->names[c->name_count].codegen_as = 0;
    c->names[c->name_count].length = 4;
    c->names[c->name_count].signature = calloc(c->names[c->name_count].length, sizeof(nat));
    c->names[c->name_count].signature[0] = 'c';
    c->names[c->name_count].signature[1] = 'h';
    c->names[c->name_count].signature[2] = 'a';
    c->names[c->name_count].signature[3] = 'r';
    c->names[c->name_count].definition = (struct unit) {0};
    c->names[c->name_count].llvm_name = translate(c->names[c->name_count]);
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
//    c->names[c->name_count].llvm_name = translate(c->names[c->name_count]);
//    c->name_count++;
    
    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
    c->names[c->name_count].type = intrin_init;
    c->names[c->name_count].precedence = 0;
    c->names[c->name_count].codegen_as = 0;
    c->names[c->name_count].length = 6;
    c->names[c->name_count].signature = calloc(c->names[c->name_count].length, sizeof(nat));
    c->names[c->name_count].signature[0] = 'j';
    c->names[c->name_count].signature[1] = 'o';
    c->names[c->name_count].signature[2] = 'i';
    c->names[c->name_count].signature[3] = 'n';
    c->names[c->name_count].signature[4] = intrin_init;
    c->names[c->name_count].signature[5] = intrin_init;
    c->names[c->name_count].definition = (struct unit) {0};
    c->names[c->name_count].llvm_name = translate(c->names[c->name_count]);
    c->name_count++;
    
    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
    c->names[c->name_count].type = intrin_init;
    c->names[c->name_count].precedence = 0;
    c->names[c->name_count].codegen_as = 0;
    c->names[c->name_count].length = 6;
    c->names[c->name_count].signature = calloc(c->names[c->name_count].length, sizeof(nat));
    c->names[c->name_count].signature[0] = 'd';
    c->names[c->name_count].signature[1] = 'e';
    c->names[c->name_count].signature[2] = 'c';
    c->names[c->name_count].signature[3] = 'l';
    c->names[c->name_count].signature[4] = intrin_init;
    c->names[c->name_count].signature[5] = 0;
    c->names[c->name_count].definition = (struct unit) {0};
    c->names[c->name_count].llvm_name = translate(c->names[c->name_count]);
    c->name_count++;
        
    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
    c->names[c->name_count].type = intrin_init;
    c->names[c->name_count].precedence = 0;
    c->names[c->name_count].codegen_as = 0;
    c->names[c->name_count].length = 5;
    c->names[c->name_count].signature = calloc(c->names[c->name_count].length, sizeof(nat));
    c->names[c->name_count].signature[0] = 'e';
    c->names[c->name_count].signature[1] = 'v';
    c->names[c->name_count].signature[2] = 'a';
    c->names[c->name_count].signature[3] = 'l';
    c->names[c->name_count].signature[4] = intrin_init;
    c->names[c->name_count].definition = (struct unit) {0};
    c->names[c->name_count].llvm_name = translate(c->names[c->name_count]);
    c->name_count++;
        
    c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
    c->names[c->name_count].type = intrin_init;
    c->names[c->name_count].precedence = 0;
    c->names[c->name_count].codegen_as = 0;
    c->names[c->name_count].length = 5;
    c->names[c->name_count].signature = calloc(c->names[c->name_count].length, sizeof(nat));
    c->names[c->name_count].signature[0] = 'c';
    c->names[c->name_count].signature[1] = 'o';
    c->names[c->name_count].signature[2] = 'm';
    c->names[c->name_count].signature[3] = 'p';
    c->names[c->name_count].signature[4] = intrin_init;
    c->names[c->name_count].definition = (struct unit) {0};
    c->names[c->name_count].llvm_name = translate(c->names[c->name_count]);
    c->name_count++;
    
    c->indicies = realloc(c->indicies, sizeof(nat) * (c->index_count + 1));
    c->indicies[c->index_count++] = intrin_root;
    
    c->indicies = realloc(c->indicies, sizeof(nat) * (c->index_count + 1));
    c->indicies[c->index_count++] = intrin_init;
    
    c->indicies = realloc(c->indicies, sizeof(nat) * (c->index_count + 1));
    c->indicies[c->index_count++] = intrin_char;
    
//    c->indicies = realloc(c->indicies, sizeof(size_t) * (c->index_count + 1));
//    c->indicies[c->index_count++] = intrin_param;

    
    c->indicies = realloc(c->indicies, sizeof(nat) * (c->index_count + 1));
    c->indicies[c->index_count++] = intrin_join;
    
    c->indicies = realloc(c->indicies, sizeof(nat) * (c->index_count + 1));
    c->indicies[c->index_count++] = intrin_decl;
    
    c->indicies = realloc(c->indicies, sizeof(nat) * (c->index_count + 1));
    c->indicies[c->index_count++] = intrin_eval;
    
    c->indicies = realloc(c->indicies, sizeof(nat) * (c->index_count + 1));
    c->indicies[c->index_count++] = intrin_comp;
}



// --------------------------------------------------------------------------------------------------------------------------




int main(int argc, const char** argv) {
    
    LLVMLinkInInterpreter(); //LLVMLinkInJIT();
    //    LLVMInitializeNativeTarget();
    
    LLVMModuleRef module = LLVMModuleCreateWithName("main.n");
    
    for (nat i = 1; i < argc; i++) {
        
        const char* ext = strrchr(argv[i], '.');
        if (not ext) abort();
        
        else if (not strcmp(ext, ".n")) {
            
            struct stat file_data = {0};
            int file = open(argv[i], O_RDONLY);
            
            if (file < 0 or stat(argv[i], &file_data) < 0) {
                fprintf(stderr, "n: %s: ", argv[i]);
                perror("error");
                continue;
            }
        
            nat length = (nat) file_data.st_size;
            int8_t* text = mmap(0, length, PROT_READ, MAP_SHARED, file, 0);
            
            if (text == MAP_FAILED) {
                fprintf(stderr, "n: %s: ", argv[i]);
                perror("error");
                continue;
            } else close(file);
                                                
            struct context context = {0};
            construct_a_context(&context);
            
            struct unit program = {0};
            struct name name = {0};
            nat begin = 0, best = 0, top = 0, done = 0, line = 1, column = 1;
            char* out = NULL;
            
            LLVMModuleRef new = LLVMModuleCreateWithName(argv[i]);
            LLVMBuilderRef builder = LLVMCreateBuilder();
            {
                LLVMTypeRef join_param_list[] = { LLVMInt32Type(), LLVMInt32Type() };
                LLVMValueRef join_function = LLVMAddFunction(new, context.names[intrin_join - 256].llvm_name, LLVMFunctionType(LLVMInt32Type(), join_param_list, 2, 0));
                LLVMBasicBlockRef join_entry = LLVMAppendBasicBlock(join_function, "entry");
                LLVMPositionBuilderAtEnd(builder, join_entry);
                LLVMValueRef join_result = LLVMBuildAdd(builder, LLVMGetParam(join_function, 0), LLVMGetParam(join_function, 1), "");
                LLVMBuildRet(builder, join_result);
            }
            {
                LLVMTypeRef* char_param_list = NULL;
                LLVMValueRef char_function = LLVMAddFunction(new, context.names[intrin_char - 256].llvm_name, LLVMFunctionType(LLVMInt32Type(), char_param_list, 0, 0));
                LLVMBasicBlockRef char_entry = LLVMAppendBasicBlock(char_function, "entry");
                LLVMPositionBuilderAtEnd(builder, char_entry);
                LLVMValueRef char_result = LLVMConstInt(LLVMInt32Type(), 234, 0);
                LLVMBuildRet(builder, char_result);
            }
            
            {
                LLVMValueRef print_hello_function = LLVMAddFunction(new, "print_hello", LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0));
            }
            
    
            {
                LLVMTypeRef main_param_list[] = { LLVMInt32Type(), LLVMPointerType(LLVMPointerType(LLVMInt8Type(), 0), 0)};
                LLVMValueRef main_function = LLVMAddFunction(new, "main", LLVMFunctionType(LLVMInt32Type(), main_param_list, 2, 0));
                LLVMBasicBlockRef main_entry = LLVMAppendBasicBlock(main_function, "entry");
                LLVMPositionBuilderAtEnd(builder, main_entry);
            }
            
            
            
//            LLVMValueRef tmp = LLVMBuildAdd(builder, LLVMGetParam(sum, 0), LLVMGetParam(sum, 1), "tmp");
//            LLVMBuildRet(builder, tmp);
            
    
            while (begin < length && text[begin] <= ' ') {
                if (text[begin] == '\n') { line++; column = 1; } else column++;
                begin++; if (begin > best) best = begin;
            }

            const nat stack_size = 65536;
            struct unit* stack = malloc(sizeof(struct unit) * stack_size);
            memset(stack, 0, sizeof(struct unit));
            stack[0].ind = context.index_count;
            stack[0].type = intrin_init;
        _0:
            if (not stack[top].ind) {
                if (not top) goto _4;
                if (stack[top].type == intrin_char) {
                    begin = stack[top].begin;
                    stack[top].index = text[begin];
                    column++;
                    begin++;
                    if (begin > best) best = begin;
                    while (begin < length && text[begin] <= ' ') {
                        if (text[begin] == '\n') { line++; column = 1; } else column++;
                        begin++; if (begin > best) best = begin;
                    }
                    goto _2;
                }
                top--;
                goto _3;
            }
            stack[top].ind--;
            done = 0;
            begin = stack[top].begin;
        _1:
            stack[top].index = context.indicies[stack[top].ind];
            name = context.names[stack[top].index - 256];
            
            if (stack[top].type != intrin_root and
                stack[top].type != name.type) goto _3;
            
            while (done < name.length) {
                nat c = name.signature[done];
                done++;
                if (c >= 256 and top + 1 < stack_size) {
                    stack[top].count++;
                    stack[top].args = realloc(stack[top].args, sizeof(struct unit) * stack[top].count);
                    top++;
                    stack[top] = (struct unit){0};
                    stack[top].ind = context.index_count;
                    stack[top].type = c;
                    stack[top].done = done;
                    stack[top].begin = begin;
                    goto _0;
                }
                if (c != text[begin]) goto _3;
                column++;
                begin++;
                if (begin > best) best = begin;
                while (begin < length && text[begin] <= ' ') {
                    if (text[begin] == '\n') { line++; column = 1; } else column++;
                    begin++; if (begin > best) best = begin;
                }
            }
        _2:
            if (stack[top].index == intrin_eval) {
                
//                LLVMModuleRef copy = LLVMCloneModule(new);
//                puts(LLVMPrintModuleToString(copy));
//                LLVMExecutionEngineRef engine = NULL;
//                if (LLVMCreateExecutionEngineForModule(&engine, copy, &out)) {
//                    printf("llvm: error: %s\n", out);
//                    abort();
//                }
//                LLVMValueRef f = NULL;
//                if (LLVMFindFunction(engine,"", &f)) {
//                    printf("llvm: error: could not find function\n");
//                    abort();
//                }
//                LLVMGenericValueRef args[] = {
//                    LLVMCreateGenericValueOfInt(LLVMInt32Type(), 999, 0),
//                    LLVMCreateGenericValueOfInt(LLVMInt32Type(), 34, 0)
//                };
//                LLVMGenericValueRef res = LLVMRunFunction(engine, f, 2, args);
//                printf("%d\n", (int)LLVMGenericValueToInt(res, 0));
//                LLVMDisposeGenericValue(res);
//                LLVMDisposeExecutionEngine(engine);
                
            } else if (stack[top].index == intrin_join) {
                
                unsigned count = stack[top].count;
                LLVMValueRef* arguments = malloc(sizeof(LLVMValueRef) * count);
                for (unsigned a = 0; a < count; a++)
                    arguments[a] = stack[top].args[a].value;
                LLVMValueRef function = LLVMGetNamedFunction(new, context.names[intrin_join - 256].llvm_name);
                LLVMValueRef result = LLVMBuildCall(builder, function, arguments, count, "");
                stack[top].value = result;
                                
            } else if (stack[top].index == intrin_char) {
                
                unsigned count = stack[top].count;
                LLVMValueRef* arguments = malloc(sizeof(LLVMValueRef) * count);
                for (unsigned a = 0; a < count; a++)
                    arguments[a] = stack[top].args[a].value;
                LLVMValueRef function = LLVMGetNamedFunction(new, context.names[intrin_char - 256].llvm_name);
                LLVMValueRef result = LLVMBuildCall(builder, function, arguments, count, "");
                stack[top].value = result;
            }
            
            if (top) {
                stack[top - 1].args[stack[top - 1].count - 1] = stack[top];
                done = stack[top].done;
                top--;
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
        
            // temp
            unsigned count = 0;
            LLVMValueRef* arguments = NULL;
            LLVMValueRef function = LLVMGetNamedFunction(new, "print_hello");
            LLVMBuildCall(builder, function, arguments, count, "");
            
            
            // temp
//            LLVMValueRef main_result = LLVMConstInt(LLVMInt32Type(), 0, 0);
            LLVMBuildRet(builder, program.value);
        
//            debug_context(&context);
//            debug_tree(program, 0, &context);
                            
            if (not program.index) {
                printf("%s: %u:%u: error: unresolved %c\n",
                       argv[i], line, column,
                       best == length ? ' ' : text[best]);
                exit(1);
            } else {
//                printf("---> compilation successful.\n");
            }
                        
            if (LLVMVerifyModule(new, LLVMPrintMessageAction, &out) or
                LLVMLinkModules2(module, new)) {
                printf("llvm: error: %s\n", out);
                abort();
            }
            
            LLVMDisposeMessage(out);
            munmap(text, file_data.st_size);
            
        } else if (not strcmp(ext, ".ll")) {
            
            LLVMModuleRef new = NULL;
            LLVMMemoryBufferRef buffer;
            char* out = NULL;
            if (LLVMCreateMemoryBufferWithContentsOfFile(argv[i], &buffer, &out) or
                LLVMParseIRInContext(LLVMGetGlobalContext(), buffer, &new, &out) or
                LLVMVerifyModule(new, LLVMPrintMessageAction, &out) or
                LLVMLinkModules2(module, new)) {
                printf("llvm: error: %s\n", out);
                abort();
            }
            
        } else {
            printf("%s: unknown file type with extension: \"%s\"\n", argv[i], ext);
            abort();
        }
    }
//    puts(LLVMPrintModuleToString(module));
    
    char* out = NULL;

    LLVMExecutionEngineRef engine = NULL;
    if (LLVMCreateExecutionEngineForModule(&engine, module, &out)) {
        printf("llvm: error: %s\n", out);
        abort();
    }
    LLVMValueRef main_function = NULL;
    if (LLVMFindFunction(engine,"main", &main_function)) {
        printf("llvm: error: could not find function\n");
        abort();
    }
    
    
    unsigned int main_argc = 0;
    const char* const* main_argv = NULL;
    const char* const* main_envp = NULL;
            
    int exit_code = LLVMRunFunctionAsMain(engine, main_function, main_argc, main_argv, main_envp);
    //    printf("Program ended with exit code: %d\n", exit_code);
    LLVMDisposeExecutionEngine(engine);
    return exit_code;
}


















































//
//
//
//
//     LLVMTypeRef param_types[] = { LLVMInt32Type(), LLVMInt32Type() };
//     LLVMValueRef sum = LLVMAddFunction(new, "sum", LLVMFunctionType(LLVMInt32Type(), param_types, 2, 0));
//
//     LLVMBasicBlockRef entry = LLVMAppendBasicBlock(sum, "entry");
//
//     LLVMBuilderRef builder = LLVMCreateBuilder();
//     LLVMPositionBuilderAtEnd(builder, entry);
//     LLVMValueRef tmp = LLVMBuildAdd(builder, LLVMGetParam(sum, 0), LLVMGetParam(sum, 1), "tmp");
//     LLVMBuildRet(builder, tmp);
//
//     char *error = NULL;
//     LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
//     LLVMDisposeMessage(error);
//
//     LLVMExecutionEngineRef engine;
//     error = NULL;
//
//     LLVMLinkInJIT();
//
//     LLVMInitializeNativeTarget();


//
//void test_llvm_stuff() {
//
//    LLVMLinkInInterpreter();
//
//    const char* path = "/Users/deniylreimn/Documents/projects/language/examples/test.ll";
//
//
//    LLVMModuleRef module = LLVMModuleCreateWithName("init.n");
//    LLVMAddFunction(module, "hi", LLVMFunctionType(LLVMInt32Type(), 0, 0, 0));
//    LLVMAddFunction(module, "fwef", LLVMFunctionType(LLVMInt32Type(), 0, 0, 0));
//
//
//    LLVMModuleRef m = LLVMModuleCreateWithName("temp.n");
//    LLVMMemoryBufferRef buffer; char* out = NULL;
//    if (LLVMCreateMemoryBufferWithContentsOfFile(path, &buffer, &out) ||
//        LLVMParseIRInContext(LLVMGetGlobalContext(), buffer, &m, &out) ||
//        LLVMLinkModules2(module, m)) {
//        printf("llvm: error: %s\n", out);
//        abort();
//    }
//
//    puts(LLVMPrintModuleToString(module));
//
//    LLVMExecutionEngineRef engine = NULL;
//    if (LLVMCreateExecutionEngineForModule(&engine, module, &out)) {
//        printf("llvm: error: %s\n", out);
//        abort();
//    }
//
//    const char* name = "main";
//
//    LLVMValueRef f = NULL;
//    if (LLVMFindFunction(engine, name, &f)) {
//        printf("llvm: error: could not find function %s to run\n", name);
//        abort();
//    }
//
//    long long x = strtoll("999", NULL, 10);
//    long long y = strtoll("1", NULL, 10);
//
//    LLVMGenericValueRef args[] = {
//        LLVMCreateGenericValueOfInt(LLVMInt32Type(), x, 0),
//        LLVMCreateGenericValueOfInt(LLVMInt32Type(), y, 0)
//    };
//
//    LLVMGenericValueRef res = LLVMRunFunction(engine, f, 2, args);
//    printf("%d\n", (int)LLVMGenericValueToInt(res, 0));
//
//    LLVMDisposeExecutionEngine(engine);
//}


// struct unit {
//     struct unit* args;
//     i32 begin; // ranges in file length
//     i32 type; // ranges in context length
//     i32 ind; // ranges in context length
//     i32 index; // ranges in context length
//     i32 done; // ranges in signature length   /// i16?
//     i32 count; // ranges in parameter count  or signature length.  /// i16?
// };
 
//
//static inline size_t evaluate_intrinsic(struct context* c, struct unit* stack, i32 top) {
//
//    if (stack[top].index == intrin_decl) {
//
//        if (stack[top].count) c->owners[c->frame_count - 1].type = stack[top].args[0].index;
//        c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
//        c->names[c->name_count] = c->owners[c->frame_count - 1];
//
//        i32 i = c->frames[c->frame_count - 1];
//        while (i-- > c->frames[c->frame_count - 2])
//            if (c->names[c->name_count].length >= c->names[c->indicies[i]].length) break;
//        i++;
//
//        c->indicies = realloc(c->indicies, sizeof(size_t) * (c->index_count + 1));
//        memmove(c->indicies + i + 1, c->indicies + i, sizeof(size_t) * (c->index_count - i));
//        c->indicies[i] = c->name_count++;
//        c->index_count++;
//
//        for (i32 s = 0; s <= top; s++)
//            if (i <= stack[s].ind) stack[s].ind++;
//
//        c->frames[c->frame_count - 1]++;
//    }
//
//    else if (stack[top].index == intrin_param) {
//            struct name* this = c->owners + c->frame_count - 1;
//            this->signature = realloc(this->signature, sizeof(i32) * (this->length + 1));
//            this->signature[this->length++] = this[1].type;
//
//    } else if (stack[top].index == intrin_define) c->names[c->name_count - 1].definition = stack[top].args[0];
//
//    return 0;
//}
//
//
//


/*
 
 -I/usr/local/Cellar/llvm/10.0.1_1/include -std=c++14 -stdlib=libc++   -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS
 
 -L/usr/local/Cellar/llvm/10.0.1_1/lib -Wl,-search_paths_first -Wl,-headerpad_max_install_names
 -lLLVMInterpreter -lLLVMCodeGen -lLLVMScalarOpts -lLLVMInstCombine -lLLVMAggressiveInstCombine -lLLVMBitWriter -lLLVMExecutionEngine -lLLVMTarget -lLLVMRuntimeDyld -lLLVMLinker -lLLVMTransformUtils -lLLVMAnalysis -lLLVMProfileData -lLLVMObject -lLLVMTextAPI -lLLVMMCParser -lLLVMMC -lLLVMDebugInfoCodeView -lLLVMDebugInfoMSF -lLLVMIRReader -lLLVMBitReader -lLLVMAsmParser -lLLVMCore -lLLVMRemarks -lLLVMBitstreamReader -lLLVMBinaryFormat -lLLVMSupport -lLLVMDemangle
 -lm -lz -lcurses -llibxml2.tbd
 
 */





//                        LLVMModuleRef module = LLVMModuleCreateWithName("init.n");
//                        LLVMAddFunction(module, "hi", LLVMFunctionType(LLVMInt32Type(), 0, 0, 0));
//                        LLVMAddFunction(module, "fwef", LLVMFunctionType(LLVMInt32Type(), 0, 0, 0));
                    
                    


