//
//  converters.hpp
//  language
//
//  Created by Daniel Rehman on 1908213.
//  Copyright © 2019 Daniel Rehman. All rights reserved.
//

#ifndef converters_hpp
#define converters_hpp

#include "analysis_ds.hpp"

expression convert_raw_llvm_symbol_to_expression(const std::string& id, llvm::Value* value, struct symbol_table& stack, struct program_data& data);

std::string expression_to_string(const expression& given, struct symbol_table& stack);

expression string_to_expression(const std::string& given, state& state);


#endif /* converters_hpp */
