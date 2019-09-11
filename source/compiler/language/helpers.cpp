//
//  helpers.cpp
//  language
//
//  Created by Daniel Rehman on 1908213.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "helpers.hpp"

#include "analysis_ds.hpp"
#include "parser.hpp"
#include "builtins.hpp"
#include "symbol_table.hpp"
#include "lists.hpp"
#include "error.hpp"

#include "llvm/IR/LLVMContext.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <cstdlib>
#include <iostream>
#include <sstream>


////////////////// comparisons ///////////////////////////

bool expressions_match(expression first, expression second);

bool subexpression(const symbol& s) {
    return s.type == symbol_type::subexpression;
}
bool identifier(const symbol& s) {
    return s.type == symbol_type::identifier;
}

bool are_equal_identifiers(const symbol &first, const symbol &second) {
    return identifier(first) and identifier(second) 
    and first.identifier.name.value == second.identifier.name.value;
}

bool symbols_match(symbol first, symbol second) {
    if (subexpression(first) and subexpression(second) 
        and expressions_match(first.expressions.list[0], 
                              second.expressions.list[0])) return true;  /// FIX ME: this code is ugly, and is only checking for if the FIRST expressions  in the expression lists are equal.
    else if (are_equal_identifiers(first, second)) return true;
    else if (first.type == symbol_type::llvm_literal and second.type == symbol_type::llvm_literal) return true;
    else return false;
}

bool expressions_match(expression first, expression second) {
    if (first.error or second.error) return false;    
    if (first.symbols.size() != second.symbols.size()) return false;
    for (size_t i = 0; i < first.symbols.size(); i++) 
        if (not symbols_match(first.symbols[i], second.symbols[i])) return false;    
    if (!first.type and !second.type) return true;
    
    if (first.llvm_type and second.llvm_type) {
        std::string first_llvm_type = "", second_llvm_type = "";
        first.llvm_type->print(llvm::raw_string_ostream(first_llvm_type) << "");
        second.llvm_type->print(llvm::raw_string_ostream(second_llvm_type) << "");
        return first_llvm_type == second_llvm_type;
    } else if (first.llvm_type or second.llvm_type) return false;
    
    else if (first.type == second.type) return true;
    else return false;
}








////////////////////////////////// General helpers ////////////////////////////////

std::string random_string() {
    static int num = 0;
    std::stringstream stream;
    stream << std::hex << rand();
    return std::string(stream.str()) + std::to_string(num++);
}


void print(std::vector<std::string> v) {
    std::cout << "[ ";
    for (auto i : v) {
        std::cout << "\"" << i << "\", ";
    }
    std::cout << "]";
}

void prune_extraneous_subexpressions(expression& given) { // unimplemented
    while (given.symbols.size() == 1 
           and subexpression(given.symbols[0])
           and given.symbols[0].subexpression.symbols.size()) {
        auto save = given.symbols[0].subexpression.symbols;
        given.symbols = save;
    }
    for (auto& symbol : given.symbols)
        if (subexpression(symbol)) prune_extraneous_subexpressions(symbol.subexpression);
}


std::vector<expression> filter_subexpressions(expression given) { // unimplemented
    std::vector<expression> subexpressions = {};    
    for (auto element : given.symbols) 
        if (subexpression(element)) subexpressions.push_back(element.subexpression);    
    return subexpressions;
    
    return {};
}

//abstraction_definition generate_abstraction_definition(const expression &given, size_t &index) {
//    abstraction_definition definition = {};
//    definition.call_signature = given.symbols[index++].subexpression;
//    while (given.symbols[index].type != symbol_type::list) {
//        definition.return_type.symbols.push_back(given.symbols[index++]);
//    } definition.body = given.symbols[index++].list;
//    return definition;
//}

//size_t generate_type_for(abstraction_definition definition) {
//    size_t type = 0;
//    auto parameter_list = filter_subexpressions(definition.call_signature);
//    if (parameter_list.empty()) {
//        type = definition.return_type;
//        return type;
//    }
//    for (auto parameter : parameter_list) {
//        expression t = type_type;
//        if (parameter.type) t = *parameter.type;
//        type->symbols.push_back(t);              
//    }
//    type->symbols.push_back({definition.return_type});
//    type->type = &type_type;
//    return type;
//    return 0;
//}

void append_return_0_statement(llvm::IRBuilder<> &builder, llvm::LLVMContext &context) {
    llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    builder.CreateRet(value);
}

void declare_donothing(llvm::IRBuilder<> &builder, const std::unique_ptr<llvm::Module> &module) {
    llvm::Function* donothing = llvm::Intrinsic::getDeclaration(module.get(), llvm::Intrinsic::donothing);        
    builder.CreateCall(donothing); // TODO: TEMP
}

bool found_llvm_string(const expression &given, size_t &pointer) {
    return pointer < given.symbols.size() and given.symbols[pointer].type == symbol_type::llvm_literal;
}

llvm::Function* create_main(llvm::IRBuilder<>& builder, llvm::LLVMContext& context, const std::unique_ptr<llvm::Module>& module) {
    std::vector<llvm::Type*> state = {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()};
    llvm::FunctionType* main_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), state, false);
    llvm::Function* main_function = llvm::Function::Create(main_type, llvm::Function::ExternalLinkage, "main", module.get());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main_function));    
    return main_function;
}















/////////////////////////////////////// PARSE LLVM STRING ///////////////////////////////////////////


llvm::Type* parse_llvm_string_as_type(std::string given, state& state, llvm::SMDiagnostic& errors) {
    return llvm::parseType(given, errors, *state.data.module);
}


bool parse_llvm_string_as_instruction(std::string given, llvm::Function* original, state& state, llvm::SMDiagnostic& errors) {
    
    std::string body = "";
    original->print(llvm::raw_string_ostream(body) << "");    
    body.pop_back(); // delete the newline
    body.pop_back(); // delete the close brace
    body += given + "\n"
        "call void @llvm.donothing()" "\n" 
        "unreachable" "\n" 
    "}" "\n";    
    
    const std::string current_name = original->getName();
    original->setName("_anonymous_" + random_string());
            
    llvm::MemoryBufferRef reference(body, "<llvm-string>");
    llvm::ModuleSummaryIndex my_index(true);
    if (llvm::parseAssemblyInto(reference, state.data.module, &my_index, errors)) {        
        original->setName(current_name);
        return false; 
            
    } else {        
        auto& temporary = state.data.module->getFunctionList().back();                
        auto& original_blocks = original->getBasicBlockList();
        auto& temporary_blocks = temporary.getBasicBlockList();        
        temporary_blocks.back().back().eraseFromParent();           // erase the unreachable ins
        temporary_blocks.back().back().eraseFromParent();           // erase the donothing() call.
        
        if (original_blocks.size() != temporary_blocks.size()) temporary_blocks.back().eraseFromParent();
        
        original_blocks.clear();  
        
        
        
        
//        for (auto& block : temporary_blocks) {            
//            std::string block_name = block.getName();            
//            state.data.builder.SetInsertPoint(llvm::BasicBlock::Create(state.data.module->getContext(), block_name, original));                
//            llvm::ValueToValueMapTy value_map;            
//            for (auto& instruction: block.getInstList()) {                
//                auto* copy = instruction.clone();
//                copy->setName(instruction.getName());                                
//                state.data.builder.Insert(copy);
//                value_map[&instruction] = copy;
//                llvm::RemapInstruction(copy, value_map, llvm::RF_NoModuleLevelChanges | llvm::RF_IgnoreMissingLocals);                                                                    
//            }
//        }    
        
        
        std::cout << "1: original state: ::::::::::::::: \n";        
        original->print(llvm::errs());
        std::cout << "::::::::::::::: \n";
        
        std::cout << "2: temporary state: ::::::::::::::: \n";        
        temporary.print(llvm::errs());
        std::cout << "::::::::::::::: \n";
        
        
        
        
        temporary.setName("_unused_" + random_string());
        original->setName(current_name);
    
        
        llvm::ValueToValueMapTy value_map;
        
        value_map.insert({original, &temporary});
        
        //value_map[original] = &temporary;
        //value_map[&temporary] = original;
        
        //llvm::RemapFunction(*original, value_map);        
        llvm::RemapFunction(temporary, value_map);
        
        
        
        
//        
//        
//        llvm::Function *F;
//        llvm::Value *V = F;
//        llvm::ValueToValueMapTy VMap;
//        auto *Clone = llvm::CloneFunction(F, VMap);
//        // V2 represents essentially the same register as V,
//        // except it's in Clone instead of F
//        llvm::Value *V2 = VMap[V];
//        
        
        
        
        
        
        
        

        
        std::cout << "1: original state: ::::::::::::::: \n";        
        original->print(llvm::errs());
        std::cout << "::::::::::::::: \n";
        
        std::cout << "2: temporary state: ::::::::::::::: \n";        
        temporary.print(llvm::errs());
        std::cout << "::::::::::::::: \n";

        return true;
     }
}

bool parse_llvm_string_as_function(std::string given, state& state, llvm::SMDiagnostic& errors) {
    llvm::MemoryBufferRef reference(given, "<llvm-string>");
    llvm::ModuleSummaryIndex my_index(true);
    return !llvm::parseAssemblyInto(reference, state.data.module, &my_index, errors);        
}

expression parse_llvm_string(const expression &given, llvm::Function* function, std::string llvm_string, size_t& pointer, state& state, flags flags) {
    
    if (flags.is_at_top_level and not flags.is_parsing_type) {
        
        llvm::SMDiagnostic instruction_errors;
        llvm::SMDiagnostic function_errors;
        
        if (parse_llvm_string_as_function(llvm_string, state, function_errors)) {
            expression solution = {};            
            solution.type = intrin::unit;
            solution.symbols = {{given.symbols[pointer++].llvm}};
            return solution;
            
        } else if (parse_llvm_string_as_instruction(llvm_string, function, state, instruction_errors)) { 
            expression solution = {};            
            solution.type = intrin::unit;
            solution.symbols = {{given.symbols[pointer++].llvm}};
            return solution;
            
        } else {
            std::cout << "ins: llvm: "; // TODO: make this have color!
            instruction_errors.print(state.data.file.name.c_str(), llvm::errs()); // temp
            std::cout << "func: llvm: "; // TODO: make this have color!
            function_errors.print(state.data.file.name.c_str(), llvm::errs());
            return failure;
        }
        
    } else if (flags.is_parsing_type and not flags.is_at_top_level) {
        
        llvm::SMDiagnostic type_errors;
        
        if (auto llvm_type = parse_llvm_string_as_type(llvm_string, state, type_errors)) {            
            expression solution = {};
            solution.llvm_type = llvm_type;
            solution.type = intrin::type;
            solution.symbols = {{given.symbols[pointer++].llvm}};
            return solution;
            
        } else {
            std::cout << "llvm: "; // TODO: make this have color!
            type_errors.print(state.data.file.name.c_str(), llvm::errs()); 
            return failure;
        }
    } else {        
        return failure;
    }
}



bool found_unit_value_expression(const expression& given) { 
    return given.symbols.empty() or 
        (given.symbols.size() == 1 and subexpression(given.symbols[0]) 
         and given.symbols[0].subexpression.symbols.empty());
}

expression parse_unit_expression(expression& given, size_t& index, state& state) {    
    if (given.symbols.size() == 1
        and subexpression(given.symbols[0])
        and given.symbols[0].subexpression.symbols.empty()) index++;
    if (given.type == intrin::infered) given.type = intrin::unit;
    if (given.type == intrin::unit) return state.stack.lookup(intrin::empty);
    else return failure;
} 






bool matches(expression given, llvm::Function* function, expression& signature, size_t& index, const size_t depth, 
             const size_t max_depth, state& state, flags flags) {
    if (given.type != signature.type and given.type != intrin::infered) return false;
    for (auto& symbol : signature.symbols) {        
        if (subexpression(symbol) and subexpression(given.symbols[index])) {
            symbol.expressions = traverse(given.symbols[index].expressions, function, state, flags);
            if (symbol.expressions.error) return false;
            index++;
        } else if (subexpression(symbol)) {
            //symbol.expressions = csr(given, index, depth + 1, max_depth, state, flags); /// i think this should be csr_single().
            if (symbol.expressions.error) return false;            
        } else if (not are_equal_identifiers(symbol, given.symbols[index])) return false;
        else index++;
    } return true;
}


expression csr_single(expression given, llvm::Function* function, size_t& index, const size_t depth, const size_t max_depth, state& state, flags flags) {
    if (index >= given.symbols.size() or not given.type or depth > max_depth) return failure; 
    if (found_llvm_string(given, index)) {
//        std::cout << "we got here! :)\n";
//        //std::cin.get();
        return parse_llvm_string(given, function, given.symbols[index].llvm.literal.value, index, state, flags);
    }
    size_t saved = index;
    for (auto signature_index : state.stack.top()) {
        index = saved;
        auto signature = state.stack.lookup(signature_index);
        if (matches(given, function, signature, index, depth, max_depth, state, flags)) return signature;
    }
    return failure;
}

expression_list csr(expression_list given, llvm::Function* function,size_t& index, const size_t depth, const size_t max_depth, state& state, flags flags) {
    for (auto& e : given.list) e = csr_single(e, function, index, depth, max_depth, state, flags);
    return given;
}

inline static void sort_top_stack_by_largest_signature(state& state) {           
    std::sort(state.stack.top().begin(), state.stack.top().end(), [&](nat a, nat b) {
        return state.stack.lookup(a).symbols.size() > state.stack.lookup(b).symbols.size(); 
    });
}

expression_list traverse(expression_list given, llvm::Function* function, state& state, flags flags) {
    sort_top_stack_by_largest_signature(state);
    
    expression_list solution {};
    for (size_t max_depth = 0; max_depth <= max_expression_depth; max_depth++) {
        size_t pointer = 0;
        solution = csr(given, function, pointer, 0, max_depth, state, flags); 
        //if (not solution.error and pointer == given.symbols.size()) break;       // i think we need to look through all expressions, and see if any have an error.
    }
    return solution; 
}

static void prepare_expressions(expression_list& given) {
    for (auto& e : given.list) {        
        e.type = intrin::unit; 
        prune_extraneous_subexpressions(e); 
    }
}

expression_list resolve(expression_list given, llvm::Function* function, state& state, flags flags) {         
    prepare_expressions(given);
    auto saved = given;
    given = traverse(given, function, state, flags);
    
    state.error = state.error or given.error;     
    if (given.error) 
        for (auto& e : saved.list) 
            print_error_message(state.data.file.name, "could not resolve expression: \n\n" + expression_to_string(e, state.stack) + "\n\n", 0, 0);
    
    return given;
}

// debug tool:

void interpret_file_as_llvm_string(const struct file &file, state &state) { // test, by allowing some llvm random string to be parsed into the file:
    llvm::SMDiagnostic errors;
    if (parse_llvm_string_as_function(file.text, state, errors)) {
        std::cout << "success.\n";
        
    } else {
        std::cout << "failure.\n";
        errors.print("llvm string program:", llvm::errs());
        abort();
    }
}

