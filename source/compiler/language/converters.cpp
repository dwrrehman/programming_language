//
//  converters.cpp
//  language
//
//  Created by Daniel Rehman on 1908213.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#include "converters.hpp"

#include "nodes.hpp"
#include "lexer.hpp"
#include "builtins.hpp"
#include "analysis_ds.hpp"
#include "converters.hpp"
#include "helpers.hpp"
#include "symbol_table.hpp"

#include <string>
#include <vector>

std::string expression_to_string(expression given, symbol_table& stack) {  // unimplemented
    std::string result = "(";
    size_t i = 0;
    for (auto symbol : given.symbols) {
        if (symbol.type == symbol_type::identifier) result += symbol.identifier.name.value;
        else if (symbol.type == symbol_type::subexpression) {
            //result += "(" + expression_to_string(symbol.expressions, stack) + ")";
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
        result += " " + expression_to_string(stack.master[given.type].signature, stack);  
    }
    return result;
}



//    std::cout << "STR TO EXP TAIL: received: {\n";
//    print_expression_list_line(list);
//    std::cout << "}\n";

//    // for parsing the parameter list in the main signature:
//    for (auto& element : signature.symbols) {
//        if (subexpression(element)) {
//            auto subexpressions = filter_subexpressions(element.subexpression); 
//            
//            auto result = string_to_expression_tail(subexpressions, state, flags); 
//            if (result.erroneous) state.error = true;
//            else element.subexpression = result;
//        }
//    }
////    if (signature.symbols.empty() and type == intrin::type) 
////        return intrin::unit;
//    
//
//auto result = resolved_expression;
//std::cout << "STRING TO EXPRESSION PRODUCED: ";
//print_expression(result, 0);
//std::cout << "\n";



//    std::cout << "string = \"" << given << "\"\n";




// unimpelented: WIP:

size_t string_to_expression_tail(std::vector<expression> list, state& state, flags flags) { // returns an index to the correct into the stack.
    if (list.empty()) return intrin::infered;
    if (list.size() == 1 and expressions_match(list[0], type_type)) return intrin::type;
    
    auto current = list.front();
    list.erase(list.begin());
    
    auto resolved_expression = traverse(current, state, flags);
    
    if (resolved_expression.error) state.error = true;
    resolved_expression.type = string_to_expression_tail(list, state, flags.dont_allow_undefined().not_at_top_level().not_parsing_a_type());
    
    return 0; // temp
}

expression string_to_expression(std::string given, state& state, flags flags) {    
    struct file file = {"<llvm string symbol>", given};
    start_lex(file);
    auto e = parse_expression(file, false, false);    
    auto subexpressions = filter_subexpressions(e);    
    auto signature = subexpressions.front();    
    
    subexpressions.erase(subexpressions.begin());        
    signature.type = string_to_expression_tail(subexpressions, state, flags);
    
    return signature;
}

expression convert_raw_llvm_symbol_to_expression(std::string id, llvm::Value* value, symbol_table& stack, file_data& data, flags flags) { 
    if (id[0] == '(') {
        bool error = false;
        state state = {stack, data, error};
        return string_to_expression(id, state, flags);
    } else {
        expression e {};
        e.llvm_type = value->getType();
        e.type = intrin::typeless;
        e.symbols = {symbol{id}};
        return e;
    }
}
