//
//  analysis.cpp
//  language
//
//  Created by Daniel Rehman on 1901314.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "analysis.hpp"


#include "builtins.hpp"
#include "debug.hpp"
#include "helpers.h"
#include "lists.hpp"
#include "llvm/IR/Verifier.h"
#include "llvm/Target/TargetMachine.h"

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

bool matches(expression given, expression& signature, size_t& index, const size_t depth, const size_t max_depth, state& state, flags flags) {
    if (not signature.type or not expressions_match(*given.type, signature.type)) return false;
    
    for (auto& symbol : signature.symbols) {         
        
        if (subexpression(symbol) and subexpression(given.symbols[index])) { // found a subexpression.
            symbol.subexpression = res(given.symbols[index].subexpression, state, flags);
            if (symbol.subexpression.erroneous) return false;
            index++;
        } else if (subexpression(symbol)) {
            symbol.subexpression = csr(given, index, depth + 1, max_depth, state, flags);
            if (symbol.subexpression.erroneous) return false;
            
        } else if (not are_equal_identifiers(symbol, given.symbols[index])) { 
            return false;
        } else index++;
    } return true;
}

expression csr(expression given, size_t& index, const size_t depth, const size_t max_depth, state& state, flags flags) {
    if (index >= given.symbols.size() or not given.type or depth > max_depth) return failure;
    if (found_unit_expression(given)) return parse_unit_expression(given, index);  
    else if (found_llvm_string(given, index)) return parse_llvm_string(given, given.symbols[index].llvm.literal.value, index, state, flags); 
    
    size_t saved = index;
    for (auto signature_index : state.stack.top()) {
        index = saved;
        if (matches(given, state.stack.lookup(signature_index), index, depth, max_depth, state, flags)) 
            return state.stack.lookup(signature_index);
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
    } 
    return solution;
}

static void interpret_file_as_llvm_string(const struct file &file, state &state) { // test, by allowing some llvm random string to be parsed into the file:
    llvm::SMDiagnostic errors;
    if (parse_llvm_string_as_function(file.text, state, errors)) {
        std::cout << "success.\n";
        
    } else {
        std::cout << "failure.\n";
        errors.print("llvm string program:", llvm::errs());
        abort();
    }
}

std::unique_ptr<llvm::Module> analyze(translation_unit unit, llvm::LLVMContext& context, struct file file) {
    
    srand((unsigned)time(nullptr));
    llvm::IRBuilder<> builder(context);
    auto module = llvm::make_unique<llvm::Module>(file.name, context);
    auto triple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(triple);    
    auto main_function = create_main(builder, context, module);    
    declare_donothing(builder, module);
    
    auto dl = llvm::DataLayout(module.get());
    module->setDataLayout(dl);
    
    bool error = false;
    symbol_table stack = {};       // init with      file.name's path,      and builtins.
    flags flags = {};
    translation_unit_data data = {file, module.get(), builder}; 
    state state = {stack, data, error};
    
    print_stack({builtins});    

    auto& body = unit.list.expressions;    
    std::vector<expression> parsed_body = {};
    for (auto expression : body) {        
        auto type = unit_type;        //TODO: fix me!   use sig idx master lookup method of typing stuff.
        expression.type = &type;  
        
        auto solution = res(expression, state, flags.generate_code().at_top_level());
        if (solution.erroneous) error = true;
        else parsed_body.push_back(solution);            
    }
    body = parsed_body;
    
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
