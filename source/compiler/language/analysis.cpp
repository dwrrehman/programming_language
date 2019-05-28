//
//  analysis.cpp
//  language
//
//  Created by Daniel Rehman on 1901314.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "analysis.hpp"
#include "parser.hpp"
#include "nodes.hpp"
#include "lists.hpp"
#include "builtins.hpp"
#include "debug.hpp"

#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/IR/Verifier.h"
#include "llvm/ADT/Optional.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/ModuleSummaryIndex.h"
#include "llvm/AsmParser/Parser.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Transforms/Utils/FunctionComparator.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include "llvm/AsmParser/Parser.h"


#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <system_error>
#include <utility>
#include <vector>
#include <sstream>
#include <iostream>

/*----------------------- KNOWN BUGS ------------------------

 (int) {}
 (g) int {}
 (hello (x)) {
    hello x
    hello g
 }

 ---> succeeds, but doesnt fill in the infered type for x, in the call signature for hello, only in the call to hello using g.





------------------------- TODOs ----------------------------



                want something to do?



    x: 0. implement LLVM types.;

    x: 0.1. allow llvm interaction with variables, back and forth.;

    1. implement FDI for CSR.

    2. implement _application_N_N

    3. implement "_define" and "_undefine" / "_undefine all"

    4. implement _abstraction_N_N

    5. implement NSS for ADP.parse_signature

    6. implement user-defined precedence and associavity




    N. implement the better error printing/handling system.





 might be useful:








 _precedence N <_expression>

 _associativity N <_expression>









 ------------------------------ actions: -----------------------------         (all return void.)


 _define <_signature> as <_expression> in <_application>

 _undefine <_signature> in <_application>

 _undefine all in <_application>


 _disclose all in <_expression> into <_application>

 _disclose <_expression> from <_expression> into <_application>


 ------------------------------ handles: -----------------------------

 _abstraction_N_N
 _application_N_N
 */








///////////// debug functions ////////////////////


static void display(llvm::Module *module, std::string message) {
    std::cout << "-------- "<< message << " -------\n";
    module->print(llvm::errs(), nullptr);
    std::cout << "-----------------------------------------\n";
    std::cout << "ok? ";
    std::string str = "";
    //std::cin >> str;
}

static void print_bbs(llvm::Function *function) {
    std::cout << "this function has: " << function->getBasicBlockList().size() << " basic blocks.\n";
    std::cout << "printing bbs: \n\n\n";
    for (auto& bb: function->getBasicBlockList()) {
        std::cout << "heres a bb: \n";
        bb.print(llvm::errs());
        std::cout << "done printing bb.\n\n";
        std::cout << "[that bb had " << bb.getInstList().size() << " instructions.]\n";
    }
    std::cout << "done printing bbs.\n";
}

static void verify(llvm::Function &f, std::string functionname) {
    if (llvm::verifyFunction(f)) {
        std::cout << "verification of "<< functionname<<" failed.\n";
        f.print(llvm::errs());
        
    } else {
        std::cout << functionname << " VERIFICATION SUCCESS\n";
        f.print(llvm::errs());
    }
}


///////////// end of debug functions ////////////////////














bool expressions_match(expression first, expression second);
expression csr(std::vector<std::vector<expression>>& stack, const expression given, const size_t depth, const size_t max_depth, size_t& pointer, struct expression*& expected, bool can_define_new_signature, bool is_at_top_level, bool is_parsing_type, llvm::Module* module, struct file file, llvm::Function* function, llvm::IRBuilder<>& builder, bool should_generate_code);
bool adp(abstraction_definition& given, std::vector<std::vector<expression>>& stack, llvm::Module* module, struct file file, llvm::Function* function, llvm::IRBuilder<>& builder, bool should_generate_code);
expression resolve(std::vector<std::vector<expression>>& stack, expression given, expression& expected_solution_type, bool can_define_new_signature, bool is_at_top_level, bool is_parsing_type, llvm::Module* module, struct file file, llvm::Function* function, llvm::IRBuilder<>& builder, bool should_generate_code);




bool symbols_match(symbol first, symbol second) {
    if (first.type == symbol_type::subexpression and second.type == symbol_type::subexpression and expressions_match(first.subexpression, second.subexpression)) return true;
    else if (first.type == symbol_type::identifier and second.type == symbol_type::identifier and first.identifier.name.value == second.identifier.name.value) return true;
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

    else if (first.type and second.type and expressions_match(*first.type, *second.type)) return true;
    else return false;
}

void prune_extraneous_subexpressions(expression& given) {
    while (given.symbols.size() == 1
           and given.symbols[0].type == symbol_type::subexpression
           and given.symbols[0].subexpression.symbols.size()) {
        auto save = given.symbols[0].subexpression.symbols;
        given.symbols = save;
    }
    for (auto& symbol : given.symbols)
        if (symbol.type == symbol_type::subexpression) prune_extraneous_subexpressions(symbol.subexpression);
}

std::vector<symbol> filter_subexpressions(expression call_signature) {
    std::vector<symbol> result = {};
    for (auto element : call_signature.symbols) {
        if (element.type == symbol_type::subexpression) {
            result.push_back(element);
        }
    } return result;
}

expression* generate_abstraction_type_for(abstraction_definition def) {
    auto type = new expression();        ///TODO: free this at some point.
    type->was_allocated = true;
    auto parameter_list = filter_subexpressions(def.call_signature);
    if (parameter_list.empty()) {
        *type = def.return_type;
        return type;
    }
    for (auto parameter : parameter_list) {
        expression t = type_type;
        if (parameter.subexpression.type) t = *parameter.subexpression.type;
        type->symbols.push_back(t);
    }
    type->symbols.push_back({def.return_type});
    type->type = &type_type;
    return type;
}

void clean(block& body) {
    block result = {};
    for (auto expression : body.list.expressions) {
        if (not expression.symbols.empty()) result.list.expressions.push_back(expression);
    } body = result;
}

static void parse_abstraction_body(bool &error, abstraction_definition &given, std::vector<std::vector<expression>> &stack, llvm::Module* module, struct file file, llvm::Function* function, llvm::IRBuilder<>& builder, bool should_generate_code) {
    stack.back().push_back(given.call_signature);
    auto& body = given.body.list.expressions;
    if (body.size()) {
        std::vector<expression> parsed_body = {};
        for (size_t i = 0; i < body.size() - 1; i++) {
            auto type = unit_type;
            auto solution = resolve(stack, body[i], type, false, true, false, module, file, function, builder, should_generate_code);
            if (solution.erroneous) {
                std::cout << "n3zqx2l: adp-csr: fake error: Could not parse expression!\n"; // TODO: print an error (IN CSR!) of some kind!

                auto type = infered_type;
                auto actual = resolve(stack, body[i], type, false, true, false, module, file, function, builder, should_generate_code);

                std::cout << "the actual type was: \n";
                print_expression_line(type);
                std::cout << "\n\n";

                std::cout << "was expecting: \n";
                print_expression_line(unit_type);
                std::cout << "\n\n";

                error = true;
                continue;
            }
            parsed_body.push_back(solution);
        }
        auto solution = resolve(stack, body[body.size() - 1], given.return_type, false, true, false, module, file, function, builder, should_generate_code);
        if (solution.erroneous) {
            std::cout << "n3zqx2l: adp-csr: fake error: Could not parse return expression!\n"; // TODO: print an error (IN CSR!) of some kind!

            auto type = infered_type;
            auto actual = resolve(stack, body[body.size() - 1], type, false, true, false, module, file, function, builder, should_generate_code);

            std::cout << "the actual type was: \n";
            print_expression_line(type);
            std::cout << "\n\n";

            std::cout << "was expecting: \n";
            print_expression_line(given.return_type);
            std::cout << "\n\n";

            error = true;
        }
        parsed_body.push_back(solution);
        given.body.list.expressions = parsed_body;
    } else if (expressions_match(given.return_type, infered_type)) {
        given.return_type = unit_type;
    }
    if (given.call_signature.type) *given.call_signature.type = given.return_type;
}

static void parse_return_type(abstraction_definition &given, std::vector<std::vector<expression> > &stack, bool& error, llvm::Module* module, struct file file, llvm::Function* function, llvm::IRBuilder<>& builder, bool should_generate_code) {
    ///TODO: this function needs to evaluate its return type, at compiletime.
    if (given.return_type.symbols.size()) {
        auto type = infered_type;
        given.return_type = resolve(stack, given.return_type, type, true, false, true, module, file, function, builder, should_generate_code);
        if (given.return_type.erroneous) {
            std::cout << "n3zqx2l: adp-csr: fake error: Could not parse type expression.\n"; // TODO: print an error (IN CSR!) of some kind!
            error = true;
        }
        if (given.return_type.type and expressions_match(*given.return_type.type, unit_type) and given.return_type.symbols.empty()) given.return_type = unit_type;
        else if (given.return_type.type and expressions_match(*given.return_type.type, none_type)) given.return_type = none_type;
    } else given.return_type = infered_type;
    given.call_signature.type = new expression();
    *given.call_signature.type = given.return_type;
    given.call_signature.type->was_allocated = true;
}

static void parse_signature(abstraction_definition &given, std::vector<std::vector<expression>>& stack, bool& error, llvm::Module* module, struct file file, llvm::Function* function, llvm::IRBuilder<>& builder, bool should_generate_code) {
    expression result = {};
    auto call = given.call_signature.symbols;
    for (size_t i = 0; i < call.size(); i++) {
        if (call[i].type == symbol_type::subexpression) {
            auto sub = call[i].subexpression;
            abstraction_definition definition = {};
            size_t pointer = 0;
            if (sub.symbols.empty()) continue;
            else if (sub.symbols[pointer].type == symbol_type::subexpression) {
                definition.call_signature = sub.symbols[pointer++].subexpression;
                while (pointer < sub.symbols.size()) {
                    definition.return_type.symbols.push_back(sub.symbols[pointer++]);
                }
                parse_signature(definition, stack, error, module, file, function, builder, should_generate_code);
                parse_return_type(definition, stack, error, module, file, function, builder, should_generate_code);
                auto parameter_type = generate_abstraction_type_for(definition);
                expression parameter = {definition.call_signature.symbols, parameter_type};
                result.symbols.push_back({parameter});
                stack.back().push_back(parameter);
            } else {
                auto parameter_type = new expression();
                *parameter_type = infered_type;
                parameter_type->was_allocated = true;
                expression parameter = {sub.symbols, parameter_type};
                result.symbols.push_back({parameter});
                stack.back().push_back(parameter);
            }
        } else if (call[i].type == symbol_type::identifier) {
            result.symbols.push_back(call[i]);

        } else { //TODO: add additional cases for the other symbol types. (like strings, etc.)
            error = true;
        }
    }
    given.call_signature = result;
}






std::string expression_to_string(expression given) {
    std::string result = "(";
    size_t i = 0;
    for (auto symbol : given.symbols) {
        if (symbol.type == symbol_type::identifier) result += symbol.identifier.name.value;
        else if (symbol.type == symbol_type::subexpression) {
            result += expression_to_string(symbol.subexpression);
        }
        if (i < given.symbols.size() - 1) result += " ";
        i++;
    }
    result += ")";
    if (given.llvm_type) {
        std::string type = "";
        given.llvm_type->print(llvm::raw_string_ostream(type) << "");
        result += " " + type;
    } else if (given.type) {
        result += " " + expression_to_string(*given.type);
    }
    return result;
}

expression string_to_expression(std::string given, llvm::LLVMContext& context, llvm::Module* module, std::vector<std::vector<expression>>& stack, llvm::Function* function, llvm::IRBuilder<>& builder, bool should_generate_code) {
    struct file file = {};
    file.name = "<llvm string symbol>";
    file.text = given;
    start_lex(file);
    auto full = parse_expression(file, false, false);
    
    expression signature = full.symbols.front().subexpression;
//    expression type = full;                
//    type.symbols.erase(type.symbols.begin());
//    
//    std::vector<expression> nested_types = {};    
//    for (auto s : type.symbols) {
//        if (s.type == symbol_type::subexpression) nested_types.push_back(s.subexpression);                 
//    }
//    
//    // debug:
//    std::cout << "there are " << nested_types.size()  << " nested types...\n";
//    std::cout << "printing them: {\n";
//    for (auto e : nested_types) {
//        std::cout << "\texpression:      ";
//        print_expression_line(e);
//        std::cout << "\n";        
//    }
//    std::cout << "}\n";

    std::cout << "parsed signature: ";     
    print_expression_line(signature);
    std::cout << "\n";
    
    auto signatures_type = infered_type;  // just a temporary hack.
    
    auto result = resolve(stack, signature, signatures_type, false, false, false, module, file, function, builder, should_generate_code);
    
    if (result.erroneous) {
        std::cout << "I DOING SOMETHING WRONG....\n";
        std::cout << "heres the stack currently...\n";
        
        print_stack(stack);
        
        std::cout << "\n\n\n\n";
    }
    return result;
}









bool adp(abstraction_definition& given, std::vector<std::vector<expression>>& stack, llvm::Module* module, struct file file, llvm::Function* function, llvm::IRBuilder<>& builder, bool should_generate_code) {
    clean(given.body); //TODO: move me into the corrector code.
    bool error = false;
    stack.push_back(stack.back());
    parse_signature(given, stack, error, module, file, function, builder, should_generate_code);
    parse_return_type(given, stack, error, module, file, function, builder, false);
    parse_abstraction_body(error, given, stack, module, file, function, builder, should_generate_code);


///    we need to determine the llvm types, from the signature of the function, (or rather the gener abs type.
    
//  useful:
//    std::vector<llvm::Type*> parameters = {/*llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()*/};
//    llvm::FunctionType* main_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(module->getContext()), parameters, false);
//    llvm::Function* function = llvm::Function::Create(main_type, llvm::Function::ExternalLinkage, "main", module);
//    builder.SetInsertPoint(llvm::BasicBlock::Create(module->getContext(), "entry", function));


    
    std::cout << "DEBUG:::::\n";
    std::cout << "testing the string to expression func: \n";
    
    std::cout << "the signature that we parsed for this ad, is: \n\t";
    print_expression_line(given.call_signature);
    std::cout << "\n";
    
    auto string_version = expression_to_string(given.call_signature);
    std::cout << "heres it stringified : \n\n\t";
    std::cout << string_version;    
    std::cout << "\n\n";
    
    auto back_again = string_to_expression(string_version, module->getContext(), module, stack, function, builder, should_generate_code);
    std::cout << "..and back to an expression : \n\n\t";
    print_expression_line(back_again);    
    std::cout << "\n\n";
    
    

    stack.pop_back();
    return error;
}

bool contains_a_block_starting_from(size_t begin, std::vector<symbol> list) {
    for (; begin < list.size(); begin++)
        if (list[begin].type == symbol_type::block) return true;
    return false;
}








//TODO: we need a flag in the resolve and csr function, which says that we DO want to generate code, or not.





std::string random_string() {
    static int num = 0;
    std::stringstream stream;
    stream << std::hex << rand();
    return std::string(stream.str()) + std::to_string(num++);
}


llvm::Type* parse_llvm_string_as_type(std::string given, llvm::Module* module, struct file file, llvm::SMDiagnostic& errors, std::vector<std::vector<expression>>& stack) {
    return llvm::parseType(given, errors, *module);
}




bool parse_llvm_string_as_instruction(std::string given, llvm::Module* module, struct file file, llvm::SMDiagnostic& errors, std::vector<std::vector<expression>>& stack, llvm::Function* function, llvm::IRBuilder<>& builder) {

    std::string body = "";
    function->print(llvm::raw_string_ostream(body) << "");

    const size_t bb_count = function->getBasicBlockList().size();

    body.pop_back(); // delete the newline;
    body.pop_back(); // delete the close brace.
    body += given + "\nunreachable\n}\n";

    const std::string current_name = function->getName();
    function->setName("_anonymous_" + random_string());

    llvm::MemoryBufferRef reference(body, "<llvm-string>");
    llvm::ModuleSummaryIndex my_index(true);

    if (llvm::parseAssemblyInto(reference, module, &my_index, errors)) {
        function->setName(current_name);
        return false;
    } else {
        auto& made = module->getFunctionList().back();
        made.getBasicBlockList().back().back().eraseFromParent();
        if (bb_count != made.getBasicBlockList().size()) made.getBasicBlockList().back().eraseFromParent();
        function->getBasicBlockList().clear();
        builder.SetInsertPoint(llvm::BasicBlock::Create(module->getContext(), "entry", function));
        builder.CreateUnreachable();
        auto& insert_before_point = function->getBasicBlockList().back().back();
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
        function->getBasicBlockList().back().back().eraseFromParent();      // delete the trailing unreachable.
        made.eraseFromParent();
        function->setName(current_name);
        return true;
    }
}

bool parse_llvm_string_as_function(std::string given, llvm::Module* module, struct file file, llvm::SMDiagnostic& errors, std::vector<std::vector<expression>>& stack) {
    llvm::MemoryBufferRef reference(given, "<llvm-string>");
    llvm::ModuleSummaryIndex my_index(true);
    if (llvm::parseAssemblyInto(reference, module, &my_index, errors)) {
        return false;
    }
    return true;
}

static expression parse_llvm_string(const struct file &file, const expression &given, bool is_at_top_level, bool is_parsing_type, const std::basic_string<char> &llvm_string, llvm::Module *module, size_t &pointer, std::vector<std::vector<expression> > &stack, llvm::Function* function, llvm::IRBuilder<>& builder) {
    if (is_at_top_level and not is_parsing_type) {

        llvm::SMDiagnostic instruction_errors;
        llvm::SMDiagnostic function_errors;

         if (parse_llvm_string_as_function(llvm_string, module, file, function_errors, stack)) {
            expression solution = {};
            solution.erroneous = false;
            solution.type = &unit_type;
            solution.symbols = {};
            symbol s = {};
            s.type = symbol_type::llvm_literal;
            s.llvm = given.symbols[pointer++].llvm;
            solution.symbols.push_back(s);
            return solution;

        } else if (parse_llvm_string_as_instruction(llvm_string, module, file, instruction_errors, stack, function, builder)) {
            expression solution = {};
            solution.erroneous = false;
            solution.type = &unit_type;
            solution.symbols = {};
            symbol s = {};
            s.type = symbol_type::llvm_literal;
            s.llvm = given.symbols[pointer++].llvm;
            solution.symbols.push_back(s);
            return solution;

        } else {
            // print error, assuming instruction, as well as for function.

            std::cout << "ins: llvm: "; // TODO: make this have color!
            instruction_errors.print(file.name.c_str(), llvm::errs()); // temp

            std::cout << "func: llvm: "; // TODO: make this have color!
            function_errors.print(file.name.c_str(), llvm::errs());

            // temp, when we print the errors,
            //we are actually just going to extract
            //the error data from the extractor methods
            //of the diagnostic error,

            //and then simply format it our self,
            //in our print_llvm_error(...) function.

            return {true};
        }

    } else if (is_parsing_type and not is_at_top_level) {

        llvm::SMDiagnostic type_errors;

        if (auto llvm_type = parse_llvm_string_as_type(llvm_string, module, file, type_errors, stack)) {

            expression solution = {};
            solution.erroneous = false;
            solution.llvm_type = llvm_type;

            solution.type = &type_type;
            solution.symbols = {};
            symbol s = {};
            s.type = symbol_type::llvm_literal;
            s.llvm = given.symbols[pointer++].llvm;
            solution.symbols.push_back(s);
            return solution;

        } else {
            std::cout << "llvm: "; // TODO: make this have color!
            type_errors.print(file.name.c_str(), llvm::errs()); // temp, see above block comment.
            return {true};
        }
    } else {
        std::cout << "unexpected llvm string here.\n"; // TODO: make this a proper error.
        return {true};
    }
}

bool add_signature_to_symbol_table(expression new_signature, std::vector<std::vector<expression>>& stack) {

    stack.back().push_back(new_signature); //TODO: should we do anything else?

    // yes:

        //TODO: merge and/xor override signatures. if it already exists.
        // we will need to do a find if.

        // give an error, and return false, if the signatures cannot be overriden/merged, and it already exists.
        // given an error if the type that the signature is, is the "_none" type. (you cannot instantiate the none type.

    return false;
}

expression csr(std::vector<std::vector<expression>>& stack, const expression given,
               const size_t depth, const size_t max_depth, size_t& pointer, struct expression*& expected,
               bool can_define_new_signature, bool is_at_top_level, bool is_parsing_type,
               llvm::Module* module, struct file file, llvm::Function* function, llvm::IRBuilder<>& builder, bool should_generate_code) {

    if (depth > max_depth) return {true};
    if (!expected) return {true};
    if (given.symbols.empty() or (given.symbols.size() == 1
                                  and given.symbols[0].type == symbol_type::subexpression
                                  and given.symbols[0].subexpression.symbols.empty())) {
        if (given.symbols.size() == 1
            and given.symbols[0].type == symbol_type::subexpression
            and given.symbols[0].subexpression.symbols.empty()) pointer++;
        if (expressions_match(*expected, infered_type)) expected = &unit_type;
        if (expressions_match(*expected, unit_type)) return unit_type;
        else return {true};
    } else if (pointer < given.symbols.size() and given.symbols[pointer].type == symbol_type::llvm_literal) {
        auto llvm_string = given.symbols[pointer].llvm.literal.value;
        return parse_llvm_string(file, given, is_at_top_level, is_parsing_type, llvm_string, module, pointer, stack, function, builder);
    }

    const auto saved = pointer;
    for (auto& signature : stack.back()) {
        if (not expressions_match(*expected, infered_type) and signature.type and not expressions_match(*signature.type, infered_type) and not expressions_match(*expected, *signature.type)) continue;
        expression solution = {};
        pointer = saved;
        auto failed = false;
        for (auto& element : signature.symbols) {
            if (pointer >= given.symbols.size()) { failed = true; break; }
            if (element.type == symbol_type::subexpression) {
                auto& TEMP = element.subexpression.type;
                auto subexpression = csr(stack, given, depth + 1, max_depth, pointer, TEMP, /*bool can_define_new_signature, bool is_at_top_level, bool is_parsing_type*/ can_define_new_signature, false, is_parsing_type, module, file, function, builder, should_generate_code);
                if (subexpression.erroneous) {
                    pointer = saved;
                    if (given.symbols[pointer].type == symbol_type::subexpression) {
                        size_t local_pointer = 0, current_depth = 0;
                        expression subexpression = {};
                        while (current_depth <= max_expression_depth) {
                            local_pointer = 0;
                            auto& TEMP = element.subexpression.type;
                            subexpression = csr(stack, given.symbols[pointer].subexpression, 0, current_depth, local_pointer, TEMP, /*bool can_define_new_signature, bool is_at_top_level, bool is_parsing_type*/ can_define_new_signature, false, is_parsing_type, module, file, function, builder, should_generate_code);
                            if (subexpression.erroneous or local_pointer < given.symbols[pointer].subexpression.symbols.size()) {
                                current_depth++;
                            } else break;
                        }
                        if (subexpression.erroneous or local_pointer < given.symbols[pointer].subexpression.symbols.size()) { failed = true; break; }
                        solution.symbols.push_back({subexpression});
                        pointer++; continue;
                    } else { failed = true; break; }
                } solution.symbols.push_back({subexpression});
            } else if (element.identifier.name.value == given.symbols[pointer].identifier.name.value
                       and given.symbols[pointer].type == symbol_type::identifier
                       and element.type == symbol_type::identifier) {
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
    if (given.symbols[pointer].type == symbol_type::subexpression and contains_a_block_starting_from(pointer + 1, given.symbols)) {
        abstraction_definition definition = {};
        definition.call_signature = given.symbols[pointer++].subexpression;
        while (given.symbols[pointer].type != symbol_type::block) {
            definition.return_type.symbols.push_back(given.symbols[pointer++]);
        } definition.body = given.symbols[pointer++].block;
        bool adp_error = adp(definition, stack, module, file, function, builder, should_generate_code);

        auto abstraction_type = generate_abstraction_type_for(definition);
        prune_extraneous_subexpressions(*abstraction_type);

        if ((expressions_match(*expected, *abstraction_type) or expressions_match(*expected, infered_type)) or is_at_top_level) {
            if (expressions_match(*expected, infered_type)) expected = abstraction_type;
            expression result = {{definition}, abstraction_type};
            result.erroneous = adp_error;
            if (not result.erroneous and is_at_top_level) {
                *result.type = unit_type;
                stack.back().push_back(definition.call_signature); ///TODO: make this go through a central function, "add_signature_to_symbol_table(sig, stack)".
            }
            return result;
        } else {
            pointer = saved;
            delete abstraction_type;
            return {true};
        }
    }
    return {true};
}

expression resolve(std::vector<std::vector<expression>>& stack, expression given, expression& expected_solution_type, bool can_define_new_signature, bool is_at_top_level, bool is_parsing_type, llvm::Module* module, struct file file, llvm::Function* function, llvm::IRBuilder<>& builder, bool should_generate_code) {
    auto& list = stack.back();
    std::sort(list.begin(), list.end(), [](auto a, auto b) { return a.symbols.size() > b.symbols.size(); });
    prune_extraneous_subexpressions(given);

    size_t pointer = 0, max_depth = 0;
    expression solution = {};
    while (max_depth <= max_expression_depth) {
        pointer = 0;
        auto solution_type_copy = expected_solution_type;
        solution.type = &solution_type_copy;
        solution = csr(stack, given, 0, max_depth, pointer, solution.type, can_define_new_signature, is_at_top_level, is_parsing_type, module, file, function, builder, should_generate_code);
        if (solution.erroneous or pointer < given.symbols.size() or not solution.type) { max_depth++; }
        else break;
    }
    if (pointer < given.symbols.size() or not solution.type) solution.erroneous = true;
    if (not solution.erroneous and expressions_match(solution, unit_type)) solution.type = &unit_type;
    if (expressions_match(expected_solution_type, infered_type) and solution.type) expected_solution_type = *solution.type;
    return solution;
}

void wrap_into_main(translation_unit& unit) {
    auto main_body = unit.list;
    expression main_call_signature = {{{"main", false}}};
    expression main_return_type = {{{"i32", false}}};
    abstraction_definition main_abstraction = {main_call_signature, main_return_type, {main_body}};
    symbol main_symbol = {main_abstraction};
    expression top_level_expression = {{main_symbol}};
    unit.list.expressions.clear();
    unit.list.expressions.push_back(top_level_expression);
}

bool contains_top_level_statements(std::vector<expression> list) {
    for (auto e : list) if (not (e.symbols.size() == 1 and e.symbols[0].type == symbol_type::abstraction_definition)) return true;
    return false;
}

std::unique_ptr<llvm::Module> analyze(translation_unit unit, llvm::LLVMContext& context, struct file file) {

    auto module = llvm::make_unique<llvm::Module>(file.name, context);

    srand((unsigned)time(nullptr));

    static bool found_main = false;
    bool error = false;
    wrap_into_main(unit);
    std::vector<std::vector<expression>> stack = {builtins};

    auto type = type_type;
    auto& main = unit.list.expressions[0].symbols[0].abstraction;
    auto& body = main.body.list.expressions;

    std::cout << "starting with stack frame: \n";
    print_stack(stack);
    
    llvm::IRBuilder<> builder(context);
    auto triple = llvm::sys::getDefaultTargetTriple();
    module->setTargetTriple(triple);
    
    std::vector<llvm::Type*> parameters = {llvm::Type::getInt32Ty(context), llvm::Type::getInt8PtrTy(context)->getPointerTo()};
    llvm::FunctionType* main_type = llvm::FunctionType::get(llvm::Type::getInt32Ty(context), parameters, false);
    llvm::Function* main_function = llvm::Function::Create(main_type, llvm::Function::ExternalLinkage, "main", module.get());
    builder.SetInsertPoint(llvm::BasicBlock::Create(context, "entry", main_function));
    bool should_generate_code = true;

    if (body.size()) {
        std::vector<expression> parsed_body = {};
        for (size_t i = 0; i < body.size(); i++) {
            auto type = unit_type;
            auto solution = resolve(stack, body[i], type, false, true, false, module.get(), file, main_function, builder, should_generate_code);
            if (solution.erroneous) {
                std::cout << "n3zqx2l: csr: fake error: Could not parse expression!\n"; // TODO: print an error (IN CSR!) of some kind.

                error = true;
                continue;
            }
            parsed_body.push_back(solution);
        }
        main.body.list.expressions = parsed_body;
        if (contains_top_level_statements(parsed_body)) {
            if (found_main) {
                std::cout << "n3zqx2l: fake error: cannot have multiple files with top level statements.\n"; // TODO: print an error of some kind!
            } else {
                file.is_main = true;
                found_main = true;
            }
        }
    }

    if (llvm::verifyFunction(*main_function)) {
        std::cout << "verify function error: main: trying \"adding implicit return value of 0\"...\n";
        llvm::Value* value = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), 0);
        builder.CreateRet(value); 
    }

    //std::string the_message = "";
    //auto stream = &(llvm::raw_string_ostream(the_message) << "");
    if (llvm::verifyModule(*module, &llvm::errs())) {
        std::cout << "the module is broken. lemme fix it.";
        std::cout << "heres the error: \n";
        // std::cout << the_message;
        std::cout << "\n";
        error = true;
    } else {
        std::cout << "the module is ok!\n";
    }

    if (debug) {
        std::cout << "----------------- analyzer ---------------------\n";
        print_translation_unit(unit, file);
    }

    std::cout << "final stack is now: \n";
    print_stack(stack);

    module->print(llvm::errs(), nullptr);

    if (error) {
        std::cout << "\n\n\tCSR ERROR\n\n\n\n";
        throw "analysis error";
    } else {
        std::cout << "\n\n\tsuccess.\n\n\n";
        return module;
    }
}











/*


---------- this is a very imporant algorithm. --------------- (code generating a call expression.)

llvm::Value* CallExprAST::codegen() {
    // Look up the name in the global module table.
    llvm::Function *CalleeF = TheModule->getFunction(Callee);
    if (!CalleeF) {
        std::cout << Callee << ": ";
        return LogErrorV("Unknown function referenced");
    }
    // If argument mismatch error.
    if (CalleeF->arg_size() != Args.size()) {
        std::cout << "Expected " << Args.size() << " arguments, but found " << CalleeF->arg_size() << ".\n";
        return LogErrorV("Incorrect # arguments passed");
    }
    std::vector<llvm::Value*> ArgsV;
    for (unsigned i = 0, e = (unsigned) Args.size(); i != e; ++i) {
        ArgsV.push_back(Args[i]->codegen());
        if (!ArgsV.back()) return nullptr;
    }

    return Builder.CreateCall(CalleeF, ArgsV, "calltmp");
}

the problem with doing this in my language, however, is that in our
 expressions we might have abstraction definitions in expressions, and
 we dont really know what to pass in for that.

    i think the only sensible thing to do is to essentially delete that parameter,
 and then simply have that function be defined on a more global scale, so that
 it can still be accessed, but we arent passing it into another function. maybe
 someone else has a better idea.








llvm::Function* PrototypeAST::codegen() {
    // Make the function type:  double(double,double) etc.
    std::vector<llvm::Type*> parameters(arguments.size(), llvm::Type::getDoubleTy(context));
    llvm::FunctionType* type = llvm::FunctionType::get(llvm::Type::getDoubleTy(context), parameters, false);
    llvm::Function* function = llvm::Function::Create(type, llvm::Function::ExternalLinkage, Name, TheModule.get());

    size_t i = 0;
    for (auto &argument : function->args()) argument.setName(arguments[i++]);

    return function;
}




llvm::Function* codegen() {

    // First, check for an existing function from a previous 'extern' declaration.
    llvm::Function* function = TheModule->getFunction(Proto->getName());
    if (!function && !(function = Proto->codegen())) return nullptr;

    // Create a new basic block to start insertion into.
    llvm::BasicBlock* block = llvm::BasicBlock::Create(context, "entry", function, builder, should_generate_code);
    Builder.SetInsertPoint(block);

    // Record the function arguments in the NamedValues map.
    NamedValues.clear();
    for (auto &Arg : function->args()) NamedValues[Arg.getName()] = &Arg;

    if (llvm::Value* return_value = Body->codegen()) {
        // Finish off the function.
        Builder.CreateRet(return_value);

        // Validate the generated code, checking for consistency.
        verifyFunction(*function);

        return function;
    }

    // Error reading body, remove function.
    function->eraseFromParent();
    return nullptr;
}



 llvm_function->copyAttributesFrom(const Function *Src)       used for transfering function attributes.


*/



//    std::string the_message = "";
//    auto stream = &(llvm::raw_string_ostream(the_message) << "");
//    if (llvm::verifyModule(*module, stream)) {
//        std::cout << "the module is broken. lemme fix it.";
//        std::cout << "heres the error: \n";
//        std::cout << the_message;
//        std::cout << "\n";
//        error = true;
//    } else {
//        std::cout << "the module is ok!\n";
//    }







//        function->getBasicBlockList().back();
//        builder.SetInsertPoint(llvm::BasicBlock::Create(module->getContext(), "entry", function));
//        display(module, "delete everything in function");
//        verify(*function, "function after deletion");
//std::cout << "going to transfer instruction now...\n";
