//
//  helpers.h
//  language
//
//  Created by Daniel Rehman on 1905315.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef helpers_h
#define helpers_h


#include "parser.hpp"

#include "llvm/IR/LLVMContext.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/ValueSymbolTable.h"

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
    if (subexpression(first) and subexpression(second) and expressions_match(first.subexpression, second.subexpression)) return true;
    else if (are_equal_identifiers(first, second)) return true;
    else if (first.type == symbol_type::llvm_literal and second.type == symbol_type::llvm_literal) return true;
    else return false;
}

bool expressions_match(expression first, expression second) {
    if (first.symbols.size() != second.symbols.size()) return false;
    for (size_t i = 0; i < first.symbols.size(); i++) {
        if (not symbols_match(first.symbols[i], second.symbols[i])) return false;
    }
    if (first.erroneous or second.erroneous) return false;
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

void clean(block& body) {
    block result = {};
    for (auto expression : body.list.expressions) 
        if (not expression.symbols.empty()) result.list.expressions.push_back(expression);
    body = result;
}

void prune_extraneous_subexpressions(expression& given) {
    while (given.symbols.size() == 1 
           and subexpression(given.symbols[0])
           and given.symbols[0].subexpression.symbols.size()) {
        auto save = given.symbols[0].subexpression.symbols;
        given.symbols = save;
    }
    for (auto& symbol : given.symbols)
        if (subexpression(symbol)) prune_extraneous_subexpressions(symbol.subexpression);
}

std::vector<expression> filter_subexpressions(expression given) {
    std::vector<expression> subexpressions = {};    
    for (auto element : given.symbols) 
        if (subexpression(element)) subexpressions.push_back(element.subexpression);    
    return subexpressions;
}

abstraction_definition preliminary_parse_abstraction(const expression &given, size_t &index) {
    abstraction_definition definition = {};
    definition.call_signature = given.symbols[index++].subexpression;
    while (given.symbols[index].type != symbol_type::block) {
        definition.return_type.symbols.push_back(given.symbols[index++]);
    } definition.body = given.symbols[index++].block;
    return definition;
}

expression* generate_abstraction_type_for(abstraction_definition def) {
//    auto type = new expression();
//    type->was_allocated = true;
//    auto parameter_list = filter_subexpressions(def.call_signature);
//    if (parameter_list.empty()) {
//        *type = def.return_type;
//        return type;
//    }
//    for (auto parameter : parameter_list) {
//        expression t = type_type;
//        if (parameter.type) t = *parameter.type;
//        ////type->symbols.push_back(t);              ///TODO: update me.
//    }
//    type->symbols.push_back({def.return_type});
//    type->type = &type_type;
//    return type;
    return nullptr;
}

bool contains_a_block_starting_from(size_t begin, std::vector<symbol> list) {
    for (; begin < list.size(); begin++)
        if (list[begin].type == symbol_type::block) return true;
    return false;
}

static bool found_abstraction_definition(expression &given, size_t &index) {
    return subexpression(given.symbols[index]) and contains_a_block_starting_from(index + 1, given.symbols);
}

bool contains_top_level_runtime_statement(std::vector<expression> list) { //TODO: fix me, to consider only runtime statements.
    //for (auto e : list) if (not (e.symbols.size() == 1 and e.symbols[0].type == symbol_type::abstraction_definition)) return true;
    return false;
}

void append_return_0_statement(llvm::IRBuilder<> &builder, llvm::LLVMContext &context) {
    llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
    builder.CreateRet(value);
}

static void declare_donothing(llvm::IRBuilder<> &builder, const std::unique_ptr<llvm::Module> &module) {
    llvm::Function* donothing = llvm::Intrinsic::getDeclaration(module.get(), llvm::Intrinsic::donothing);        
    builder.CreateCall(donothing); // TODO: TEMP
}

bool found_unit_expression(const expression &given) {
    return given.symbols.empty() 
    or (given.symbols.size() == 1 
        and subexpression(given.symbols[0]) 
        and given.symbols[0].subexpression.symbols.empty());
}

expression parse_unit_expression(expression& given, size_t& index) {   ////TODO: this is bad. fix this. 
//    if (given.symbols.size() == 1
//        and subexpression(given.symbols[0])
//        and given.symbols[0].subexpression.symbols.empty()) index++;
//    if (expressions_match(*given.type, infered_type)) given.type = &unit_type;
//    if (expressions_match(*given.type, unit_type)) return unit_type;
//    else 
    return failure;
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




























//////////////////////////// SYMBOL TABLE CONVERTERS ./////////////////////////////











expression string_to_expression_tail(std::vector<expression> list, state& state, flags flags) {
    
    /*
    if (list.empty()) return {};
    if (list.size() == 1 and expressions_match(list.back(), type_type)) return type_type;          
    auto signature = list.front();
    
    for (auto& element : signature.symbols) {
        if (subexpression(element)) {
            auto subexpressions = filter_subexpressions(element.subexpression); 
            
            auto result = string_to_expression_tail(subexpressions, state, flags);
            if (result.erroneous) state.error = true;
            else element.subexpression = result;
        }
    }
    list.erase(list.begin());
    auto type = string_to_expression_tail(list, state, flags
                                          .dont_allow_undefined()
                                          .not_at_top_level()
                                          .not_parsing_a_type());
    if (signature.symbols.empty() and expressions_match(type, type_type)) return unit_type;    
    auto result = resolve(signature, type, state, flags);    
    if (result.erroneous) state.error = true;    
    return result;
     */
    return {};
}

expression string_to_expression(std::string given, state& state, flags flags, bool& error) {
    struct file file = {"<llvm string symbol>", given};
    start_lex(file);
    return string_to_expression_tail(filter_subexpressions(parse_expression(file, false, false)), state, flags); 
}



























/////////////////////////////////////// PARSE LLVM STRING ///////////////////////////////////////////





llvm::Type* parse_llvm_string_as_type(std::string given, state& state, llvm::SMDiagnostic& errors) {
    return llvm::parseType(given, errors, *state.data.module);
}



/////TODO:
//////          rewrite this code by replacing the use of the "unreachable" instruction" with a call to "llvm.donothing()". this makes way more sense.

bool parse_llvm_string_as_instruction(std::string given, llvm::Function* function, state& state, llvm::SMDiagnostic& errors) {
    
    std::string body = "";
    //function->print(llvm::raw_string_ostream(body) << "");
    
    
    //const size_t bb_count = function->getBasicBlockList().size();
//    
//    body.pop_back(); // delete the newline;
//    body.pop_back(); // delete the close brace.
//    body += given + "\nunreachable\n}\n";
//    
    /*
    const std::string current_name = data.function->getName();
    data.function->setName("_anonymous_" + random_string());
    
    llvm::MemoryBufferRef reference(body, "<llvm-string>");
    llvm::ModuleSummaryIndex my_index(true);
    
    if (llvm::parseAssemblyInto(reference, data.module, &my_index, errors)) {
        data.function->setName(current_name);
        return false;
    } else {
        auto& made = data.module->getFunctionList().back();
        made.getBasicBlockList().back().back().eraseFromParent();
        if (bb_count != made.getBasicBlockList().size()) made.getBasicBlockList().back().eraseFromParent();
        data.function->getBasicBlockList().clear();
        data.builder.SetInsertPoint(llvm::BasicBlock::Create(data.module->getContext(), "entry", data.function));
        data.builder.CreateUnreachable();
        auto& insert_before_point = data.function->getBasicBlockList().back().back();
        for (auto& bb : made.getBasicBlockList()) {
            llvm::ValueToValueMapTy vmap;
            for (auto& inst: bb.getInstList()) {
                auto* new_inst = inst.clone();
                new_inst->setName(inst.getName());
                new_inst->insertBefore(&insert_before_point);
                vmap[&inst] = new_inst;
                llvm::RemapInstruction(new_inst, vmap, llvm::RF_NoModuleLevelChanges | llvm::RF_IgnoreMissingLocals);
            }
        }
        data.function->getBasicBlockList().back().back().eraseFromParent();      // delete the trailing unreachable.
        made.eraseFromParent();
        data.function->setName(current_name);
        return true;
    }*/
    return false;
}

bool parse_llvm_string_as_function(std::string given, state& state, llvm::SMDiagnostic& errors) {
    llvm::MemoryBufferRef reference(given, "<llvm-string>");
    llvm::ModuleSummaryIndex my_index(true);
    return !llvm::parseAssemblyInto(reference, state.data.module, &my_index, errors);        
}

static expression parse_llvm_string(const expression &given, std::string llvm_string, size_t &pointer, state& state, flags flags) {
    
    if (flags.is_at_top_level and not flags.is_parsing_type) {
        
        llvm::SMDiagnostic instruction_errors;
        llvm::SMDiagnostic function_errors;
        
        if (parse_llvm_string_as_function(llvm_string, state, function_errors)) {
            expression solution = {};
            solution.erroneous = false;
            solution.type = 3;
            solution.symbols = {};
            symbol s = {};
            s.type = symbol_type::llvm_literal;
            s.llvm = given.symbols[pointer++].llvm;
            solution.symbols.push_back(s);
            return solution;
            
        } else if (parse_llvm_string_as_instruction(llvm_string, NULL, state, instruction_errors)) {
            expression solution = {};
            solution.erroneous = false;
            solution.type = 3;
            solution.symbols = {};
            symbol s = {};
            s.type = symbol_type::llvm_literal;
            s.llvm = given.symbols[pointer++].llvm;
            solution.symbols.push_back(s);
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
            solution.erroneous = false;
            solution.llvm_type = llvm_type;
            solution.type = 1;
            solution.symbols = {};
            symbol s = {};
            s.type = symbol_type::llvm_literal;
            s.llvm = given.symbols[pointer++].llvm;
            solution.symbols.push_back(s);
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































////////////////// OLD CODE /////////////////////////// 


/*

bool add_signature_to_symbol_table(expression new_signature, stack& stack) {    
    stack.back().push_back(new_signature); //TODO: should we do anything else?
    return false;
}

static void parse_abstraction_body(abstraction_definition &given, state& state, flags flags) {        
    add_signature_to_symbol_table(given.call_signature, state.stack);
    auto& body = given.body.list.expressions;
    if (body.size()) {
        std::vector<expression> parsed_body = {};
        for (size_t i = 0; i < body.size() - 1; i++) {
            
            auto type = unit_type;
            auto solution = resolve(body[i], type, state, flags
                                    .dont_allow_undefined()
                                    .at_top_level()
                                    .not_parsing_a_type());
            
            if (solution.erroneous) state.error = true;                
            else parsed_body.push_back(solution);
        }
        
        auto solution = resolve(body[body.size() - 1], given.return_type, state, flags
                                .dont_allow_undefined()
                                .at_top_level()
                                .not_parsing_a_type());        
        if (solution.erroneous) state.error = true;
        else parsed_body.push_back(solution);
        given.body.list.expressions = parsed_body;
        
    } else if (expressions_match(given.return_type, infered_type)) given.return_type = unit_type;
    else if (not expressions_match(given.return_type, unit_type)) state.error = true;    
    if (given.call_signature.type) *given.call_signature.type = given.return_type;
}

static void parse_return_type(abstraction_definition &given, state& state, flags flags) {
    
    if (given.return_type.symbols.size()) {
        auto type = infered_type;
        
        given.return_type = resolve(given.return_type, type, state, flags
                                    .allow_undefined()
                                    .not_at_top_level()
                                    .parsing_a_type());
        if (given.return_type.erroneous) state.error = true;
        
        if (given.return_type.type and expressions_match(*given.return_type.type, unit_type) and given.return_type.symbols.empty()) given.return_type = unit_type;
        else if (given.return_type.type and expressions_match(*given.return_type.type, none_type)) given.return_type = none_type;
    } else given.return_type = infered_type;
    
    given.call_signature.type = new expression();
    *given.call_signature.type = given.return_type;
    given.call_signature.type->was_allocated = true;
}

static void push_infered_parameter(expression &result, stack &stack, const expression &sub) {
    auto parameter_type = new expression();
    *parameter_type = infered_type;
    parameter_type->was_allocated = true;
    expression parameter = {sub.symbols, parameter_type};
    result.symbols.push_back({parameter});
    stack.back().push_back(parameter);
}

void parse_signature(abstraction_definition &given, state& state, flags flags);

static void push_typed_parameter(state& state, flags flags, expression &result, expression &sub) {
    abstraction_definition definition = {};
    size_t pointer = 0;
    definition.call_signature = sub.symbols[pointer++].subexpression;
    while (pointer < sub.symbols.size()) definition.return_type.symbols.push_back(sub.symbols[pointer++]);                
    parse_signature(definition, state, flags); 
    parse_return_type(definition, state, flags); 
    auto parameter_type = generate_abstraction_type_for(definition);
    expression parameter = {definition.call_signature.symbols, parameter_type};
    result.symbols.push_back({parameter});
    stack.back().push_back(parameter);
}

void parse_signature(abstraction_definition &given, state& state, flags flags) {
    expression result = {};
    auto call = given.call_signature.symbols;
    for (size_t i = 0; i < call.size(); i++) {
        if (subexpression(call[i])) {
            auto sub = call[i].subexpression;
            if (sub.symbols.empty()) continue;
            else if (subexpression(sub.symbols.front()))
                push_typed_parameter(state, flags, result, sub);
            else push_infered_parameter(result, state.stack, sub);
            
        } else if (identifier(call[i])) {
            result.symbols.push_back(call[i]);
        } else error = true;        
    }
    given.call_signature = result;
}

void parse_abstraction(abstraction_definition& given, state& state, flags flags) {
    clean(given.body);
    state.stack.push_back(state.stack.back());
    parse_signature(given, state, flags);
    parse_return_type(given, state, flags);
    parse_abstraction_body(given, state, flags);
    state.stack.pop_back();
}

static void use_csr1(symbol &element, expression &fdi, state& state, flags flags, const expression &given, size_t &local_pointer, size_t &pointer, expression &subexpression) {
    subexpression = expression {};
    local_pointer = 0; size_t current_depth = 0;    
    while (current_depth <= max_expression_depth) {
        local_pointer = 0;
        auto& type = element.subexpression.type;
        flags.should_allow_undefined_signatures = type and expressions_match(*type, signature_type);
        subexpression = csr(given.symbols[pointer].subexpression, type, fdi, 0, current_depth, local_pointer, state, flags.not_at_top_level());
        if (subexpression.erroneous or local_pointer < given.symbols[pointer].subexpression.symbols.size()) {
            current_depth++;
        } 
        else break;
    }
}
// call it "descend()"?
static void use_csr2(expression &expected_type, expression &fdi, state& state, flags flags, const expression &given, size_t &pointer, expression &solution) {
    solution = expression {};
    pointer = 0; size_t max_depth = 0;
    while (max_depth <= max_expression_depth) {
        pointer = 0;
        auto copy = expected_type;
        solution.type = &copy;
        flags.should_allow_undefined_signatures = expressions_match(*solution.type, signature_type);
        solution = csr(given, solution.type, fdi, 0, max_depth, pointer, state, flags);
        if (solution.erroneous or pointer < given.symbols.size() or not solution.type) { 
            max_depth++;
        }
        else break;
    }
}

expression parse_new_abstraction_definition( expression *&expected, state& state, flags flags, const expression &given, size_t &pointer, unsigned long saved) {
    abstraction_definition definition = preliminary_parse_abstraction(given, pointer);        
    parse_abstraction(definition, state, flags);
    auto abstraction_type = generate_abstraction_type_for(definition);
    prune_extraneous_subexpressions(*abstraction_type);
    
    if ((expressions_match(*expected, *abstraction_type) or expressions_match(*expected, infered_type)) or flags.is_at_top_level) {
        if (expressions_match(*expected, infered_type)) expected = abstraction_type;
        expression result = {{definition}, abstraction_type};
        result.erroneous = state.error;
        if (not result.erroneous and flags.is_at_top_level) {
            *result.type = unit_type;
            add_signature_to_symbol_table(definition.call_signature, state.stack);
        }
        return result;
    } else {
        pointer = saved;
        delete abstraction_type;
        return {true};
    }
}

expression csr(const expression& given, expression*& expected, expression& fdi, const size_t depth, const size_t max_depth, size_t& pointer, state& state, flags flags) {    
    if (depth > max_depth or not expected) return {true};
    if (found_unit_expression(given)) return parse_unit_expression(expected, given, pointer);
    else if (found_llvm_string(given, pointer)) return parse_llvm_string(given, given.symbols[pointer].llvm.literal.value, pointer, state, flags); 
    
    const auto saved = pointer;
    for (auto& signature : state.stack.back()) {
        
        if (not expressions_match(*expected, infered_type) and signature.type
            and not expressions_match(*signature.type, infered_type) 
            and not expressions_match(*expected, *signature.type)) continue;
        
        expression solution = {};
        pointer = saved;
        auto failed = false;
        for (auto& element : signature.symbols) {
            if (pointer >= given.symbols.size()) { failed = true; break; }
            if (subexpression(element)) {
                auto& type = element.subexpression.type;
                flags.should_allow_undefined_signatures = type and expressions_match(*type, signature_type);                
                expression parsed_subexpression = csr(given, type, fdi, depth + 1, max_depth, pointer, state, flags.not_at_top_level());                
                if (parsed_subexpression.erroneous) {
                    pointer = saved;
                    if (subexpression(given.symbols[pointer])) {                                                
                        size_t local_pointer;
                        expression subexpression = {};
                        use_csr1(data, element, error, fdi, flags, given, local_pointer, pointer, stack, subexpression);                                                
                        if (subexpression.erroneous or local_pointer < given.symbols[pointer].subexpression.symbols.size()) { failed = true; break; }
                        solution.symbols.push_back({subexpression});
                        pointer++; continue;
                    } else { failed = true; break; }
                } solution.symbols.push_back({parsed_subexpression});
            } else if (element.identifier.name.value == given.symbols[pointer].identifier.name.value
                       and identifier(given.symbols[pointer]) and identifier(element)) {
                solution.symbols.push_back(element);
                pointer++;
            } else { failed = true; break; }
        } if (!failed) {
            if (not expressions_match(*expected, infered_type) and signature.type
                and expressions_match(*signature.type, infered_type)) *signature.type = *expected;
            else if (expressions_match(*expected, infered_type)) expected = signature.type;
            if (signature.type) solution.type = signature.type;
            else solution.type = &type_type;
            return solution;
        }
    }
    
    if (subexpression(given.symbols[pointer]) and contains_a_block_starting_from(pointer + 1, given.symbols))
        return parse_new_abstraction_definition(data, error, expected, flags, given, pointer, saved, stack);
    
    else if (flags.should_allow_undefined_signatures and pointer < given.symbols.size()) {
        fdi.symbols.push_back(given.symbols[pointer++]);
        return fdi;       //// ?      is this right?
    }
    return {true};
}

expression resolve(expression given, expression& expected_type, state& state, flags flags) {    
    std::sort(state.stack.back().begin(), state.stack.back().end(), [](auto a, auto b) { return a.symbols.size() > b.symbols.size(); });
    prune_extraneous_subexpressions(given);             
    expression solution = {}, fdi = {};
    size_t pointer;
    use_csr2(data, error, expected_type, fdi, flags, given, pointer, solution, stack);
    if (pointer < given.symbols.size() or not solution.type) solution.erroneous = true;
    if (not solution.erroneous and expressions_match(solution, unit_type)) solution.type = &unit_type;
    if (expressions_match(expected_type, infered_type) and solution.type) expected_type = *solution.type;
    return solution;
}









*/






////////////////////// debuggers //////////////////////////// 







#endif /* helpers_h */
