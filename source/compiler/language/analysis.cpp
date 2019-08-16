//
//  analysis.cpp
//  language
//
//  Created by Daniel Rehman on 1901314.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

/*
 
 
 
 
 -------------------------------whats missing?------------------------------------------
 
 

 -----> code gen for abstractions  [IMPORTANT]
 
 -----> compiletime evaluation  [IMPORTANT]
 
 
 -----> scope/func references:  [IMPORTANT]
     
                _abstraction_x_x
 
                _application_x_x

 
 -----> interpret the _define and _undefine and _disclose intrins.   [IMPORTANT]
 
 
        
----->  complete the FDI algorithm in CSR  [IMPORTANT]
 
 
 
 ------->  figure out how to implement NSS [IMPORTANT]
 
 
 
 
 
 
 
 
 
 -----> write a custom stack class, 
 
        which keep tracks if something was defined in the current scope or a parent scope. 

 
 
 */

#include "analysis.hpp"
#include "parser.hpp"
#include "nodes.hpp"
#include "lists.hpp"
#include "builtins.hpp"
#include "debug.hpp"
#include "helpers.h"


/*
expression adp(expression given, size_t &pointer, size_t saved, state& state, flags flags) {
    auto def = preliminary_parse_abstraction(given, pointer);        
    parse_abstraction(def, state, flags);
    auto abstraction_type = generate_abstraction_type_for(def);
    prune_extraneous_subexpressions(*abstraction_type);
    
    if ((expressions_match(*given.type, *abstraction_type) or expressions_match(*expected, infered_type)) or flags.is_at_top_level) {
        if (expressions_match(*expected, infered_type)) expected = abstraction_type;
        expression result = {{definition}, abstraction_type};
        result.erroneous = state.error;
        if (not result.erroneous and flags.is_at_top_level) {
            *result.type = unit_type;
            add_signature_to_symbol_table(definition.call_signature, state.stack);
        }
        return result;
    } else {
        delete abstraction_type;
        return {true};
    }
}

abstraction_definition adp(expression given, size_t pointer, state& state, flags flags) {
    clean(given.body);
    state.stack.push_back(state.stack.back());
    parse_signature(given, state, flags);
    parse_return_type(given, state, flags);
    parse_abstraction_body(given, state, flags);
    state.stack.pop_back();
}

*/


expression parse_abstraction_definition(expression given, size_t& index, state& state, flags flags) {
    return {};
}


////TODO: this algorithm left out subexpressions. we need to add that back in using "res()".

bool matches(expression given, expression& signature, size_t& index, const size_t depth, const size_t max_depth, state& state, flags flags) {
    if (not signature.type or not expressions_match(*given.type, signature.type)) return false;
    for (auto& element : signature.symbols) {
        ///TODO: check to see if the current given node is a subexpression. this means we need to call "res()".
        if (subexpression(element)) {
            element.subexpression = csr(given, index, depth + 1, max_depth, state, flags);
            if (element.subexpression.erroneous) return false;        
        } else if (not are_equal_identifiers(element, given.symbols[index])) return false;
        else index++;
    } return true;
}

expression csr(expression given, size_t& index, const size_t depth, const size_t max_depth, state& state, flags flags) {
    if (index >= given.symbols.size() or not given.type or depth > max_depth) return failure;
    size_t saved = index;
    for (auto signature : state.stack.top()) {
        index = saved;
        if (matches(given, signature, index, depth, max_depth, state, flags)) return signature;
    }
    
    if (found_abstraction_definition(given, index)) return parse_abstraction_definition(given, index, state, flags);
    
    return failure;
}

expression res(expression given, state& state, flags flags) {
    expression solution {};    
    for (size_t max_depth = 0; max_depth <= max_expression_depth; max_depth++) {
        size_t pointer = 0;
        solution = csr(given, pointer, 0, max_depth, state, flags);
        if (not solution.erroneous and pointer == given.symbols.size()) break;
    } return solution;
}

std::unique_ptr<llvm::Module> analyze(translation_unit unit, llvm::LLVMContext& context, struct file file) {
    
    srand((unsigned)time(nullptr));
    llvm::IRBuilder<> builder(context);
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    auto triple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(triple);
    auto main_function = create_main(builder, context, module);
    
    llvm::Function* donothing = llvm::Intrinsic::getDeclaration(module.get(), llvm::Intrinsic::donothing);        
    builder.CreateCall(donothing);
    
    
    bool error = false;
    stack stack = {};
    astack astack = {}; //init with file.name's path.
    flags flags = {};
    translation_unit_data data = {file, module.get(), builder};          // get rid of main_function in this data structure.
    state state = {stack, astack, data, error};
    
    
    
    
    
    // test, by allowing some llvm random string to be parsed into the file:
    llvm::SMDiagnostic errors;
    if (parse_llvm_string_as_function(file.text, state, errors)) {
        std::cout << "success.\n";
        
    } else {
        std::cout << "failure.\n";
        errors.print("llvm string program:", llvm::errs());
        abort();
    }
    
    

    //    auto& body = unit.list.expressions;
    //    std::vector<expression> parsed_body = {};
    //    for (auto expression : body) {
    //        auto solution = resolve(expression, unit_type, state, flags.generate_code().at_top_level());
    //        if (solution.erroneous) error = true;
    //        else parsed_body.push_back(solution);
    //    } body = parsed_body;

    
    
    
    
    if (llvm::verifyFunction(*main_function)) append_return_0_statement(builder, context);
    if (llvm::verifyModule(*module, &llvm::errs())) error = true;
    if (debug) print_translation_unit(unit, file);    
    
    if (debug) {
        std::cout << "emitting the following LLVM: \n";
        module->print(llvm::errs(), NULL); // temp
    }
    
    if (error) { throw "analysis error"; }
    else { return module; }
}







//    if (contains_top_level_runtime_statement(body)) found_main = true; 
//    else if (found_main) main_function->eraseFromParent();



//////////////////////////////////////////////////////


//    auto& body = unit.list.expressions;
//    std::vector<expression> parsed_body = {};
//    for (auto expression : body) {
//        auto solution = resolve(expression, unit_type, state, flags.generate_code().at_top_level());
//        if (solution.erroneous) error = true;
//        else parsed_body.push_back(solution);
//    } body = parsed_body;




//    else if (flags.should_allow_undefined_signatures) {
//        //fdi.symbols.push_back(given.symbols[pointer++]);
//        //return fdi;       //// ?      is this right?
//    }
