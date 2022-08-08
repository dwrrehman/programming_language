

















		// // printf("checking given type expression, for instance in type table....\n");
		// // struct expression e = this->args[1];
		// // debug_program(&e, 0, context);			
		
		// // printf("starting type table member equiv checks...\n");
		
		// nat its_type = 0;

		// while (its_type < context->type_count) {

		// 	// printf("checking equivalence with: \n");
		// 	// debug_program(context->types + its_type, 0, context);			

		// 	if (trees_equal(this->args + 1, context->types + its_type)) {
		// 		// printf("TYPES EQUAL = TRUE\n");
		// 		break;
		// 	} else {
		// 		// printf("TYPES EQUAL = FALSE\n");			
		// 		its_type++;
		// 	}
		// }

		// // if (its_type == context->type_count) {
		// // 	printf("type not found: ----> NEW TYPE will be generated.\n");
			
		// // } else {
		// // 	printf("type found: ----> EXISTING TYPE will be used: at index %d\n", its_type);
		// // }













	    // if (not strcmp(argv[i], "--top")) {
     //            if (i + 1 < argc) top_level_type = atoi(argv[++i]);
     //            else {
     //                fprintf(stderr, "compiler: error0: argument not supplied for option --top <nat>\n");
     //                error_count++;
     //            }
     //            continue;
     //        } 
	








// if (not error_count and action_type == action_context) serialize_context(&context, output_name);




   // if (stack[top].index == intrin_define) {

		// const nat type = stack[top].args[0].index;
		// struct name new = {0};


		// new.type = type; 

		// // how do we set the signature?

	 //       const nat f = context.frame_count - 1;

	 //       context.names = realloc(context.names, sizeof(struct name) * (size_t)(context.name_count + 1));
	 //       context.names[context.name_count] = new_name;

	 //       nat place = context.frames[f];
	 //       while (place-- > context.frames[f])
	 //           if (new_name.length >= context.names[context.indicies[place]].length) break;
	 //       place++;
	 //       context.indicies = realloc(context.indicies, sizeof(size_t) * (size_t)(context.index_count + 1));
	 //       memmove(context.indicies + place + 1, context.indicies + place, 
		// 		sizeof(size_t) * (size_t)(context.index_count - place));

	 //       context.indicies[place] = context.name_count++;
	 //       context.index_count++;
	 //       for (nat s = 0; s <= top; s++) if (place <= stack[s].ind) stack[s].ind++;

	 //       context.frames[f]++;
	    // }






























// ----------------------------------------------------







/*

-syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX11.0.sdk 

"ld64.lld "
                    "-L/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib "
                    "-macosx_version_min 11.0 -sdk_version 11.0 "
                    "-lSystem -lc "
                    "-o %s %s", output_name, emit_filename);



 "/usr/bin/ld" -demangle -lto_library /usr/local/Cellar/llvm/11.0.0/lib/libLTO.dylib 

-dynamic -arch x86_64 -platform_version macos 11.0.0 0.0.0 

-syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX11.0.sdk 

-o a.out 
object.o 

-lSystem /usr/local/Cellar/llvm/11.0.0/lib/clang/11.0.0/lib/darwin/libclang_rt.osx.a

*/


//     LLVMTypeRef param_types[] = { LLVMInt32Type(), LLVMInt32Type() };
//     LLVMValueRef sum = LLVMAddFunction(new, "sum", LLVMFunctionType(LLVMInt32Type(), param_types, 2, 0));
//
//     LLVMBasicBlockRef entry = LLVMAppendBasicBlock(sum, "entry");
//     LLVMBuilderRef builder = LLVMCreateBuilder();
//     LLVMPositionBuilderAtEnd(builder, entry);
//     LLVMValueRef tmp = LLVMBuildAdd(builder, LLVMGetParam(sum, 0), LLVMGetParam(sum, 1), "tmp");
//     LLVMBuildRet(builder, tmp);
//     char *error = NULL;
//     LLVMVerifyModule(mod, LLVMAbortProcessAction, &error);
//     LLVMDisposeMessage(error);
//     LLVMExecutionEngineRef engine;
//     error = NULL;
//     LLVMLinkInJIT();
//     LLVMInitializeNativeTarget();
// struct unit {
//     struct unit* args;
//     i32 begin; // ranges in file length
//     i32 type; // ranges in context length
//     i32 ind; // ranges in context length
//     i32 index; // ranges in context length
//     i32 done; // ranges in signature length   /// i16?
//     i32 count; // ranges in parameter count  or signature length.  /// i16?
// };
//static inline size_t evaluate_intrinsic(struct context* c, struct unit* stack, i32 top) {
//    if (stack[top].index == intrin_decl) {
//        if (stack[top].count) c->owners[c->frame_count - 1].type = stack[top].args[0].index;
//        c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
//        c->names[c->name_count] = c->owners[c->frame_count - 1];
//        i32 i = c->frames[c->frame_count - 1];
//        while (i-- > c->frames[c->frame_count - 2])
//            if (c->names[c->name_count].length >= c->names[c->indicies[i]].length) break;
//        i++;
//        c->indicies = realloc(c->indicies, sizeof(size_t) * (c->index_count + 1));
//        memmove(c->indicies + i + 1, c->indicies + i, sizeof(size_t) * (c->index_count - i));
//        c->indicies[i] = c->name_count++;
//        c->index_count++;
//        for (i32 s = 0; s <= top; s++)
//            if (i <= stack[s].ind) stack[s].ind++;
//
//        c->frames[c->frame_count - 1]++;
//    }
//    else if (stack[top].index == intrin_param) {
//            struct name* this = c->owners + c->frame_count - 1;
//            this->signature = realloc(this->signature, sizeof(i32) * (this->length + 1));
//            this->signature[this->length++] = this[1].type;
//
//    } else if (stack[top].index == intrin_define) c->names[c->name_count - 1].definition = stack[top].args[0];
//    return 0;
//}


/*

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

*/

// const char* target_name = LLVMGetTargetName(target);
        
        // debug:
        // printf("\n\ntarget = %s\ntriple: %s :: name: %s :: features: %s\n\n", target_name, triple, name, features);
        
// const char* target_name = LLVMGetTargetName(target);


        // printf("count = %d\n", main_argc);
        // for (int i = 0; i < main_argc; i++) {
        //     printf("\t - \"%s\"\n", main_argv[i]);
        // }

// {
            //     LLVMTypeRef join_param_list[] = { LLVMInt32Type(), LLVMInt32Type() };
            //     LLVMValueRef join_function = LLVMAddFunction(new, context.names[intrin_join - 256].llvm_name, LLVMFunctionType(LLVMInt32Type(), join_param_list, 2, 0));
            //     LLVMBasicBlockRef join_entry = LLVMAppendBasicBlock(join_function, "entry");
            //     LLVMPositionBuilderAtEnd(builder, join_entry);
            //     LLVMValueRef join_result = LLVMBuildAdd(builder, LLVMGetParam(join_function, 0), LLVMGetParam(join_function, 1), "");
            //     LLVMBuildRet(builder, join_result);
            // }
            // {
            //     LLVMTypeRef* char_param_list = NULL;
            //     LLVMValueRef char_function = LLVMAddFunction(new, context.names[intrin_char - 256].llvm_name, LLVMFunctionType(LLVMInt32Type(), char_param_list, 0, 0));
            //     LLVMBasicBlockRef char_entry = LLVMAppendBasicBlock(char_function, "entry");
            //     LLVMPositionBuilderAtEnd(builder, char_entry);
            //     LLVMValueRef char_result = LLVMConstInt(LLVMInt32Type(), 1, 0);
            //     LLVMBuildRet(builder, char_result);
            // }            
            // {
            //     LLVMAddFunction(new, "print_hello", LLVMFunctionType(LLVMInt32Type(), NULL, 0, 0));
            //     //  LLVMValueRef print_hello_function = 
            // }  









/*

if (stack[top].index == intrin_eval) {
            
               LLVMModuleRef copy = LLVMCloneModule(new);
               puts(LLVMPrintModuleToString(copy));
               LLVMExecutionEngineRef engine = NULL;
               if (LLVMCreateExecutionEngineForModule(&engine, copy, &llvm_error_message)) {
                   printf("llvm: error: %s\n", llvm_error_message);
                   error_count++;
                   continue;
               }
               LLVMValueRef f = NULL;
               if (LLVMFindFunction(engine,"", &f)) {
                   printf("llvm: error: could not find function\n");
                   error_count++;
                   continue;
               }

               LLVMGenericValueRef args[] = {
                   LLVMCreateGenericValueOfInt(LLVMInt32Type(), 999, 0),
                   LLVMCreateGenericValueOfInt(LLVMInt32Type(), 34, 0)
               };
               LLVMGenericValueRef res = LLVMRunFunction(engine, f, 2, args);
               printf("%d\n", (int)LLVMGenericValueToInt(res, 0));
               LLVMDisposeGenericValue(res);
               LLVMDisposeExecutionEngine(engine);               
 
            } else if (stack[top].index == intrin_join) {                
                // unsigned count = (unsigned) stack[top].count;
                // LLVMValueRef* arguments = malloc(sizeof(LLVMValueRef) * count);
                // for (unsigned a = 0; a < count; a++)
                //     arguments[a] = stack[top].args[a].value;
                // LLVMValueRef function = LLVMGetNamedFunction(new, context.names[intrin_join - 256].llvm_name);
                // LLVMValueRef result = LLVMBuildCall(builder, function, arguments, count, "");
                // stack[top].value = result;
            
            } else if (stack[top].index == intrin_char) {                
                // unsigned count = (unsigned) stack[top].count;
                // LLVMValueRef* arguments = malloc(sizeof(LLVMValueRef) * count);
                // for (unsigned a = 0; a < count; a++)
                //     arguments[a] = stack[top].args[a].value;
                // LLVMValueRef function = LLVMGetNamedFunction(new, context.names[intrin_char - 256].llvm_name);
                // LLVMValueRef result = LLVMBuildCall(builder, function, arguments, count, "");
                // stack[top].value = result;

	    } else if (stack[top].index == intrin_decl) {

		// declare the signature into the current frame...? 
		// or only when eval'd?


            } else if (stack[top].index == intrin_unreachable) {
                LLVMBuildUnreachable(builder);

            } else if (stack[top].index == intrin_branch) {
                // LLVMBasicBlockRef block = 
                // LLVMBuildBr(builder, block);
            }



*/


		// unsigned count = 0;
            // LLVMValueRef* arguments = NULL;
            // LLVMValueRef function = LLVMGetNamedFunction(new, "print_hello");
            // LLVMBuildCall(builder, function, arguments, count, "");            




    // do optimizations:
    // LLVMPassManagerRef pass_manageer = LLVMCreateFunctionPassManagerForModule(module);




// } else if () {
    
    //     LLVMTargetRef target = NULL;  
    //     LLVMCodeGenOptLevel optimization_level = LLVMCodeGenLevelDefault;    
    //     LLVMCodeGenFileType output_filetype = LLVMAssemblyFile;        
    //     const char* triple = LLVMGetDefaultTargetTriple();
    //     const char* name = LLVMGetHostCPUName();
    //     const char* features = LLVMGetHostCPUFeatures();

    //     if (LLVMGetTargetFromTriple(triple, &target, &llvm_error_message)) {
    //         printf("error: get target from triple failed: %s\n", llvm_error_message);
    //         LLVMDisposeMessage(llvm_error_message);
    //         abort();
    //     }    
    //     LLVMTargetMachineRef target_machine = LLVMCreateTargetMachine
    //         (target, triple, name, features, optimization_level, LLVMRelocDefault, LLVMCodeModelDefault);

    //     if (LLVMTargetMachineEmitToFile(target_machine, module, (char*) (intptr_t) output_name, output_filetype, &llvm_error_message)) {
    //         printf("error: target machine mit to file failed: %s\n", llvm_error_message);
    //         LLVMDisposeMessage(llvm_error_message);
    //         abort();
    //     } 




    // } else if (action_type == action_generate_executable) {
    
    //     LLVMTargetRef target = NULL;

    //     size_t object_filename_size = strlen(output_name) + 2 + 1;
    //     char* object_filename = calloc(object_filename_size, sizeof(char));

    //     strncpy(object_filename, output_name, object_filename_size);
    //     strncat(object_filename, ".o", object_filename_size);
    
    //     LLVMCodeGenOptLevel optimization_level = LLVMCodeGenLevelDefault;    
    //     LLVMCodeGenFileType output_filetype = LLVMObjectFile;
        
    //     const char* triple = LLVMGetDefaultTargetTriple();
    //     const char* name = LLVMGetHostCPUName();
    //     const char* features = LLVMGetHostCPUFeatures();

    //     if (LLVMGetTargetFromTriple(triple, &target, &llvm_error_message)) {
    //         fprintf(stderr, "llvm: error: get target from triple failed: %s\n", llvm_error_message);
    //         LLVMDisposeMessage(llvm_error_message);
    //         abort();
    //     }
        
    //     LLVMTargetMachineRef target_machine = LLVMCreateTargetMachine
    //         (target, triple, name, features, optimization_level, LLVMRelocDefault, LLVMCodeModelDefault);
        
    //     if (LLVMTargetMachineEmitToFile(target_machine, module, object_filename, output_filetype, &llvm_error_message)) {
    //         fprintf(stderr, "llvm: error: target machine mit to file failed: %s\n", llvm_error_message);
    //         LLVMDisposeMessage(llvm_error_message);
    //         abort();
    //     }
        
    //     char string[4096] = {0};
    //     snprintf(string, sizeof string, 
    //                 "ld64.lld "
    //                 "-L/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib "
    //                 "-macosx_version_min 11.0 "
    //                 "-sdk_version 11.0 "
    //                 "-lSystem -lc "
    //                 "-o %s %s", output_name, object_filename);
    //     system(string);
    //     remove(object_filename);
    //     free(object_filename);        



            
            // {
            //     LLVMTypeRef main_param_list[] = { LLVMInt32Type(), LLVMPointerType(LLVMPointerType(LLVMInt8Type(), 0), 0)};
            //     LLVMValueRef main_function = LLVMAddFunction(new, "main", LLVMFunctionType(LLVMInt32Type(), main_param_list, 2, 0));
            //     LLVMBasicBlockRef main_entry = LLVMAppendBasicBlock(main_function, "entry");
            //     LLVMPositionBuilderAtEnd(builder, main_entry);
            // }
//            LLVMValueRef tmp = LLVMBuildAdd(builder, LLVMGetParam(sum, 0), LLVMGetParam(sum, 1), "tmp");
//            LLVMBuildRet(builder, tmp);




// LLVMValueRef main_result = LLVMConstInt(LLVMInt32Type(), 0, 0);
                // LLVMBuildRet(builder, main_result);
            







// static inline char* translate(struct name name) { // temp
    
//     nat* signature, length, type;
//     signature = name.signature;
//     length = name.length;
//     type = name.type;
//     char* result = NULL;
//     nat result_length = 0;
//     nat result_capacity = 0;    
//     for (nat i = 0; i < length; i++) {
//         const nat c = signature[i];
        
//         if (c < 256) {
//             if (result_length + 1 >= result_capacity) result = realloc(result, sizeof(char) * (size_t) (result_capacity = 2 * (result_capacity + 1)));
            
//             result[result_length++] = (char) c;
//         } else {            
//             if (result_length + 1 >= result_capacity) result = realloc(result, sizeof(char) * (size_t) (result_capacity = 2 * (result_capacity + 1)));
//             result[result_length++] = '\t';
            
//             nat extra = snprintf(NULL, 0, "%u", c);
//             if (result_length + extra >= result_capacity) result = realloc(result, sizeof(char) * (size_t) (result_capacity = 2 * (result_capacity + extra)));
//             result_length += sprintf(result + result_length, "%u", c);
            
//             if (result_length + 1 >= result_capacity) result = realloc(result, sizeof(char) * (size_t) (result_capacity = 2 * (result_capacity + 1)));
//             result[result_length++] = '\n';
//         }
//     }

//     if (result_length + 1 >= result_capacity) result = realloc(result, sizeof(char) * (size_t) (result_capacity = 2 * (result_capacity + 1)));
//     result[result_length++] = ' ';
    
//     nat extra = snprintf(NULL, 0, "%u", type);
//     if (result_length + extra >= result_capacity) result = realloc(result, sizeof(char) * (size_t) (result_capacity = 2 * (result_capacity + extra)));
//     result_length += sprintf(result + result_length, "%u", type);
        
//     if (result_length + 1 >= result_capacity) result = realloc(result, sizeof(char) * (size_t) (result_capacity = 2 * (result_capacity + 1)));
//     result[result_length++] = '\0';
    
//     return result;
// }





// {
//     struct context context = {0};
//     construct_a_context(&context);
//     debug_context(&context);
//     serialize_context(&context, "context.ctx");
//     printf("serialized the context! exiting...\n");    
// }

// {
//     struct context context = {0};    
//     load_context(&context, "context.ctx");
//     printf("loaded the context! printing...\n");
//     debug_context(&context);   
// }













    // if (argc == 1) { 
    //    struct context context = {0};
    //    load_context(&context, "context.ctx");
    //    debug_context(&context);
    //    serialize_context(&context, "context.ctx");
    //    exit(0);
    // }
    




    // intrin_define, ///       decl (name: char) (type: (0)) -> init
    
    // intrin_eval, ///       eval (type: (0)) (expression: type) -> comp(type)
    // intrin_comp, ///       comp (type: init) -> init
    // intrin_unreachable, /// temp
    // intrin_label, /// temp
    // intrin_branch, /// temp    
    // intrin_param, 
    // intrin_define__arg0, 
    // intrin_define,



// static inline void load_context(struct context* context, const char* path) {

//     // todo: make this realloc the context, ie, extend it, not obliterate it. 
    
//     struct stat file_data = {0};
//     int file = open(path, O_RDONLY);            
//     if (file < 0 or stat(path, &file_data) < 0) {
//         fprintf(stderr, "compiler: load_context: error: %s: ", path);
//         perror("open/stat");
//         return;
//     }
    
//     nat* text = mmap(0, (size_t) file_data.st_size, PROT_READ, MAP_SHARED, file, 0);
//     if (text == MAP_FAILED) {
//         fprintf(stderr, "compiler: load_context: serror: %s: ", path);
//         perror("mmap");
//         return;
//     }
//     close(file);
    
//     nat count = *text++;
//     context->name_count = count; 
//     context->index_count = count;
//     context->frame_count = 1;
//     context->names = calloc((size_t) count, sizeof(struct name));
//     context->indicies = calloc((size_t) count, sizeof(nat));    
//     context->frames = calloc(1, sizeof(nat));
    
//     for (nat i = 0; i < count; i++) {               
//         context->names[i].type = *text++;
//         context->names[i].codegen_as = *text++;
//         context->names[i].precedence = *text++;
//         context->names[i].length = *text++;        
//         context->names[i].signature = calloc((size_t) context->names[i].length, sizeof(nat));
//         for (nat s = 0; s < context->names[i].length; s++) 
// 	    context->names[i].signature[s] = *text++;
//         context->indicies[i] = i + 256;
//     }   
// }


// static inline void serialize_context(struct context* context, const char* path) { // inline into the trailing section of the .n file parse.
 
//     FILE* out = fopen(path, "w");
//     if (not out) { 
// 	fprintf(stderr, "compiler: serialize_context: error: %s: ", path);
// 	perror("fopen"); 
// 	exit(1); 
//     }
//     fwrite(&context->name_count, sizeof(nat), 1, out);
//     for (nat i = 0; i < context->name_count; i++) {
//         fwrite(&context->names[i].type, sizeof(nat), 1, out);
//         fwrite(&context->names[i].codegen_as, sizeof(nat), 1, out);
//         fwrite(&context->names[i].precedence, sizeof(nat), 1, out);        
//         fwrite(&context->names[i].length, sizeof(nat), 1, out);        
//         for (nat s = 0; s < context->names[i].length; s++) 
// 	    fwrite(context->names[i].signature + s, sizeof(nat), 1, out);
//     }
//     fclose(out);
// }



            // if (not strcmp(argv[i], "--max-depth")) {
            //     if (i + 1 < argc) stack_size = atoi(argv[++i]);
            //     else {
            //         fprintf(stderr, "compiler: error0: argument not supplied for option --max-depth <nat>\n");
            //         error_count++;
            //     }
            //     continue;
            // } 



    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_root;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_type;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_unit;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_join;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_name;
 
    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_define;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_param;




    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_init;
    
    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_char;
    
    
    
    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_decl;
    
    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_eval;
    
    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_comp;








//static inline size_t evaluate_intrinsic(struct context* c, struct unit* stack, i32 top) {
//    if (stack[top].index == intrin_decl) {
//        if (stack[top].count) c->owners[c->frame_count - 1].type = stack[top].args[0].index;
//        c->names = realloc(c->names, sizeof(struct name) * (c->name_count + 1));
//        c->names[c->name_count] = c->owners[c->frame_count - 1];
//        i32 i = c->frames[c->frame_count - 1];
//        while (i-- > c->frames[c->frame_count - 2])
//            if (c->names[c->name_count].length >= c->names[c->indicies[i]].length) break;
//        i++;
//        c->indicies = realloc(c->indicies, sizeof(size_t) * (c->index_count + 1));
//        memmove(c->indicies + i + 1, c->indicies + i, sizeof(size_t) * (c->index_count - i));
//        c->indicies[i] = c->name_count++;
//        c->index_count++;
//        for (i32 s = 0; s <= top; s++)
//            if (i <= stack[s].ind) stack[s].ind++;
//
//        c->frames[c->frame_count - 1]++;
//    }
//    else if (stack[top].index == intrin_param) {
//            struct name* this = c->owners + c->frame_count - 1;
//            this->signature = realloc(this->signature, sizeof(i32) * (this->length + 1));
//            this->signature[this->length++] = this[1].type;
//
//    } else if (stack[top].index == intrin_define) c->names[c->name_count - 1].definition = stack[top].args[0];
//    return 0;
//}




    // intrin_root,
    // intrin_type,
    // intrin_unit,
    // intrin_join, 
    // intrin_name, 

    // intrin_A,
    // intrin_B,
    // intrin_C,
    // intrin_D,
    // intrin_E,
    // intrin_F,
    // intrin_G,
    // intrin_H,
    // intrin_I,
    // intrin_J,
    // intrin_K,
    // intrin_L,
    // intrin_M,
    // intrin_N,
    // intrin_O,
    // intrin_P,
    // intrin_Q,
    // intrin_R,
    // intrin_S,
    // intrin_T,
    // intrin_U,
    // intrin_V,
    // intrin_W,
    // intrin_X,
    // intrin_Y,
    // intrin_Z,

    // intrin_stop,
    // intrin_define,
    // intrin_param,


// llvm_function, 
    // llvm_struct,
    // llvm_local_variable, 
    // llvm_global_variable, 
    // llvm_parameter















//     c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_undef;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 4;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'r';
//     c->names[c->name_count].signature[1] = 'o';
//     c->names[c->name_count].signature[2] = 'o';
//     c->names[c->name_count].signature[3] = 't';
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


//     c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_root;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 4;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 't';
//     c->names[c->name_count].signature[1] = 'y';
//     c->names[c->name_count].signature[2] = 'p';
//     c->names[c->name_count].signature[3] = 'e';    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;
    
//     c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_type;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 4;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'u';
//     c->names[c->name_count].signature[1] = 'n';
//     c->names[c->name_count].signature[2] = 'i';
//     c->names[c->name_count].signature[3] = 't';
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


//     c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_unit;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 6;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'j';
//     c->names[c->name_count].signature[1] = 'o';
//     c->names[c->name_count].signature[2] = 'i';
//     c->names[c->name_count].signature[3] = 'n';
//     c->names[c->name_count].signature[4] = intrin_unit;
//     c->names[c->name_count].signature[5] = intrin_unit;
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;

//     c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_type;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 4;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'n';
//     c->names[c->name_count].signature[1] = 'a';
//     c->names[c->name_count].signature[2] = 'm';
//     c->names[c->name_count].signature[3] = 'e';
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;















//     c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'A';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;

//     c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'B';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


//     c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'C';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


//     c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'D';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


//     c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'E';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


//     c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'F';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;

// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'G';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;

// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'H';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;

// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'I';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;

// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'J';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;

// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'K';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;

// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'L';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;

// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'M';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'N';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'O';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'P';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'Q';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'R';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'S';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'T';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'U';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'V';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'W';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'X';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'Y';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


// c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 2;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'Z';
//     c->names[c->name_count].signature[1] = intrin_name;    
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;






















//     c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 0;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//  //    	c->names[c->name_count].signature[0] = 's';    
// 	// c->names[c->name_count].signature[1] = 't';    
// 	// c->names[c->name_count].signature[2] = 'o';    
// 	// c->names[c->name_count].signature[3] = 'p';
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


//     c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_unit;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 8;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'd';
//     c->names[c->name_count].signature[1] = 'e';
//     c->names[c->name_count].signature[2] = 'f';
//     c->names[c->name_count].signature[3] = 'i';
//     c->names[c->name_count].signature[4] = 'n';
//     c->names[c->name_count].signature[5] = 'e';
//     c->names[c->name_count].signature[6] = intrin_name;
//     c->names[c->name_count].signature[7] = intrin_type;
//     c->names[c->name_count].definition = (struct unit) {0};    
//     c->name_count++;


//     c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
//     c->names[c->name_count].type = intrin_name;
//     c->names[c->name_count].precedence = 0;
//     c->names[c->name_count].codegen_as = 0;
//     c->names[c->name_count].length = 7;
//     c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
//     c->names[c->name_count].signature[0] = 'p';
//     c->names[c->name_count].signature[1] = 'a';
//     c->names[c->name_count].signature[2] = 'r';
//     c->names[c->name_count].signature[3] = 'a';
//     c->names[c->name_count].signature[4] = 'm';
//     c->names[c->name_count].signature[5] = intrin_unit;
//     c->names[c->name_count].signature[6] = intrin_name;
//     c->names[c->name_count].definition = (struct unit) {0};
//     // c->names[c->name_count].llvm_name = translate(c->names[c->name_count]);
//     c->name_count++;


    // c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
    // c->names[c->name_count].type = intrin_init;
    // c->names[c->name_count].precedence = 0;
    // c->names[c->name_count].codegen_as = 0;
    // c->names[c->name_count].length = 5;
    // c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
    // c->names[c->name_count].signature[0] = 'e';
    // c->names[c->name_count].signature[1] = 'v';
    // c->names[c->name_count].signature[2] = 'a';
    // c->names[c->name_count].signature[3] = 'l';
    // c->names[c->name_count].signature[4] = intrin_init;
    // c->names[c->name_count].definition = (struct unit) {0};
    // // c->names[c->name_count].llvm_name = translate(c->names[c->name_count]);
    // c->name_count++;
        
    // c->names = realloc(c->names, sizeof(struct name) * (size_t) (c->name_count + 1));
    // c->names[c->name_count].type = intrin_init;
    // c->names[c->name_count].precedence = 0;
    // c->names[c->name_count].codegen_as = 0;
    // c->names[c->name_count].length = 5;
    // c->names[c->name_count].signature = calloc((size_t) c->names[c->name_count].length, sizeof(nat));
    // c->names[c->name_count].signature[0] = 'c';
    // c->names[c->name_count].signature[1] = 'o';
    // c->names[c->name_count].signature[2] = 'm';
    // c->names[c->name_count].signature[3] = 'p';
    // c->names[c->name_count].signature[4] = intrin_init;
    // c->names[c->name_count].definition = (struct unit) {0};
    // // c->names[c->name_count].llvm_name = translate(c->names[c->name_count]);
    // c->name_count++;

    
    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_stop;
    


    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_A;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_B;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_C;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_D;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_E;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_F;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_G;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_H;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_I;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_J;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_K;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_L;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_M;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_N;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_O;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_P;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_Q;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_R;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_S;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_T;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_U;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_V;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_W;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_X;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_Y;

    // c->indicies = realloc(c->indicies, sizeof(nat) * (size_t) (c->index_count + 1));
    // c->indicies[c->index_count++] = intrin_Z;






    // if (tree.index >= 256) {
    //     struct name name = context->names[tree.index - 256];

    //     for (nat i = 0; i < name.length; i++) {
    //         if (name.signature[i] < 256)
    //             printf("%c", (char) name.signature[i]);
    //         else printf(" (arg=%d) ", name.signature[i]);
    //     }





    // for (size_t i = 0; i < d; i++) printf(".   ");
    // printf("type: ");
    // debug_tree(tree->type, d + 1, context);
    // puts("");


// if (argc == 1) {
	// 	struct context context = {0};
	// 	// load_context(&context, "context.ctx");
	// 	construct_context(&context);
	// 	debug_context(&context);
	// 	exit(0);
	// }





//TODO: we also need to load in the starting context.. 
		// which we can actually tweak in the compiler. later on, we will be able to specify a 
		// init-context file, which is done for all files. so thats cool. 